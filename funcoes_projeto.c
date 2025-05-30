#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "simulator.h"

// Busca um processo na lista de processos pelo PID
Processo* buscar_processo(Simulador *sim, int pid) {
    for (int i = 0; i < sim->num_processos; i++) {
        if (sim->processos[i].pid == pid) return &sim->processos[i];
    }
    return NULL;
}

// Inicializa o simulador
Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria_fisica) {
    Simulador *sim = (Simulador *)malloc(sizeof(Simulador));
    sim->tempo_atual = 0;
    sim->tamanho_pagina = tamanho_pagina;
    sim->tamanho_memoria_fisica = tamanho_memoria_fisica;
    sim->num_processos = 0;
    sim->processos = NULL;

    // Inicializa memória física
    sim->memoria.num_frames = tamanho_memoria_fisica / tamanho_pagina;
    sim->memoria.frames = (int *)calloc(sim->memoria.num_frames, sizeof(int));
    sim->memoria.tempo_carga = (int *)calloc(sim->memoria.num_frames, sizeof(int));
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        sim->memoria.frames[i] = -1; // Frame vazio
        sim->memoria.tempo_carga[i] = 0;
    }

    sim->total_acessos = 0;
    sim->page_faults = 0;
    sim->algoritmo = 0;
    srand(time(NULL)); // Inicializa a semente para random

    return sim;
}

// Cria um novo processo e sua tabela de páginas
Processo* criar_processo(Simulador *sim, int tamanho_processo) {
    sim->processos = (Processo *)realloc(sim->processos, (sim->num_processos + 1) * sizeof(Processo));
    Processo *proc = &sim->processos[sim->num_processos++];

    proc->pid = sim->num_processos;
    proc->tamanho = tamanho_processo;
    proc->num_paginas = (tamanho_processo + sim->tamanho_pagina - 1) / sim->tamanho_pagina;
    proc->tabela_paginas = (Pagina *)calloc(proc->num_paginas, sizeof(Pagina));

    for (int i = 0; i < proc->num_paginas; i++) {
        proc->tabela_paginas[i] = (Pagina){.presente = 0, .frame = -1, .modificada = 0, .referencia = 0, .tempo_carga = 0, .ultimo_acesso = 0};
    }
    return proc;
}

// Traduz endereço virtual para físico
int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual) {
    int pagina, deslocamento;
    extrair_pagina_deslocamento(sim, endereco_virtual, &pagina, &deslocamento);

    int presente = verificar_pagina(sim, pid, pagina);
    if (presente == -1) return -1;

    int frame = presente ? buscar_processo(sim, pid)->tabela_paginas[pagina].frame
                          : carregar_pagina(sim, pid, pagina);
    if (frame == -1) return -1;

    int endereco_fisico = frame * sim->tamanho_pagina + deslocamento;
    sim->total_acessos++;
    sim->tempo_atual++;
    printf("Tempo t=%d: Endereço Virtual (P%d): %d -> Página: %d -> Frame: %d -> Endereço Físico: %d\n",
           sim->tempo_atual, pid, endereco_virtual, pagina, frame, endereco_fisico);
    return endereco_fisico;
}
// Divide um endereço virtual em número de página e deslocamento
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento) {
    *pagina = endereco_virtual / sim->tamanho_pagina;
    *deslocamento = endereco_virtual % sim->tamanho_pagina;
}

// Verifica se a página está presente na memória
int verificar_pagina(Simulador *sim, int pid, int pagina) {
    Processo *proc = buscar_processo(sim, pid);
    if (!proc || pagina >= proc->num_paginas) return -1;
    return proc->tabela_paginas[pagina].presente;
}

// Carrega uma página na memória, se necessário substitui
int carregar_pagina(Simulador *sim, int pid, int pagina) {
    Processo *proc = buscar_processo(sim, pid);
    if (!proc || pagina >= proc->num_paginas) return -1;

    int frame = -1;
    // Verifica se há frame livre
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.frames[i] == -1) {
            frame = i;
            break;
        }
    }

    // Se não há frame livre, faz substituição
    if (frame == -1) {
        frame = sim->algoritmo == 0 ? substituir_pagina_fifo(sim) : substituir_pagina_random(sim);
        printf("Tempo t=%d: [PAGE FAULT] Substituindo página no Frame %d usando algoritmo %s\n", 
               sim->tempo_atual, frame, sim->algoritmo == 0 ? "FIFO" : "Random");
    } else {
        printf("Tempo t=%d: [PAGE FAULT] Carregando Página %d do Processo %d no Frame %d\n", 
               sim->tempo_atual, pagina, pid, frame);
    }

    sim->memoria.frames[frame] = (pid << 16) | pagina;
    sim->memoria.tempo_carga[frame] = sim->tempo_atual;
    proc->tabela_paginas[pagina] = (Pagina){
        .presente = 1, .frame = frame, .tempo_carga = sim->tempo_atual,
        .modificada = 0, .referencia = 0, .ultimo_acesso = 0
    };
    sim->page_faults++;
    return frame;
}

// Algoritmo FIFO para substituição de página
int substituir_pagina_fifo(Simulador *sim) {
    int oldest_frame = 0, min_tempo = sim->memoria.tempo_carga[0];
    for (int i = 1; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.tempo_carga[i] < min_tempo) {
            min_tempo = sim->memoria.tempo_carga[i];
            oldest_frame = i;
        }
    }
    if (sim->memoria.frames[oldest_frame] != -1) {
        int old_pid = sim->memoria.frames[oldest_frame] >> 16;
        int old_page = sim->memoria.frames[oldest_frame] & 0xFFFF;
        Processo *proc = buscar_processo(sim, old_pid);
        if (proc) {
            proc->tabela_paginas[old_page].presente = 0;
            proc->tabela_paginas[old_page].frame = -1;
        }
    }
    return oldest_frame;
}

// Algoritmo Random para substituição de página
int substituir_pagina_random(Simulador *sim) {
    int frame = rand() % sim->memoria.num_frames;
    if (sim->memoria.frames[frame] != -1) {
        int old_pid = sim->memoria.frames[frame] >> 16;
        int old_page = sim->memoria.frames[frame] & 0xFFFF;
        Processo *proc = buscar_processo(sim, old_pid);
        if (proc) {
            proc->tabela_paginas[old_page].presente = 0;
            proc->tabela_paginas[old_page].frame = -1;
        }
    }
    return frame;
}

// Mostra o estado atual da memória física
void exibir_memoria_fisica(Simulador *sim) {
    printf("Tempo t=%d\n", sim->tempo_atual);
    printf("Estado da Memória Física:\n");
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        if (sim->memoria.frames[i] == -1)
            printf("| ----- |\n");
        else {
            int pid = sim->memoria.frames[i] >> 16;
            int page = sim->memoria.frames[i] & 0xFFFF;
            printf("| P%d-%d |\n", pid, page);
        }
    }
}

// Exibe estatísticas gerais da simulação
void exibir_estatisticas(Simulador *sim) {
    printf("\nTotal de acessos à memória: %d\n", sim->total_acessos);
    printf("Total de page faults: %d\n", sim->page_faults);
    float fault_rate = sim->total_acessos > 0 ? (float)sim->page_faults / sim->total_acessos * 100 : 0;
    printf("Taxa de page faults: %.2f%%\n", fault_rate);
    printf("Algoritmo utilizado: %s\n", sim->algoritmo == 0 ? "FIFO" : "Random");
}

