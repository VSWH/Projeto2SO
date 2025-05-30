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

// Divide um endereço virtual em número de página e deslocamento
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento) {
    *pagina = endereco_virtual / sim->tamanho_pagina;
    *deslocamento = endereco_virtual % sim->tamanho_pagina;
}

