#include <stdio.h>
#include <stdlib.h>
#include "simulator.h" // Inclui o cabeçalho do seu simulador

// Função para reinicializar o estado do simulador para uma nova execução
void resetar_simulador(Simulador *sim) {
    sim->tempo_atual = 0;
    sim->total_acessos = 0;
    sim->page_faults = 0;

    // Reinicializa a memória física
    for (int i = 0; i < sim->memoria.num_frames; i++) {
        sim->memoria.frames[i] = -1;       // Indica que o frame está vazio
        sim->memoria.tempo_carga[i] = 0;   // Zera o tempo de carga do frame
    }

    // Reinicializa as tabelas de página de todos os processos
    for (int i = 0; i < sim->num_processos; i++) {
        for (int j = 0; j < sim->processos[i].num_paginas; j++) {
            sim->processos[i].tabela_paginas[j].presente = 0; // Página não está na memória física
            sim->processos[i].tabela_paginas[j].frame = -1;   // Nenhum frame associado
        }
    }
}

// Função para liberar toda a memória alocada pelo simulador
void finalizar_simulador(Simulador *sim) {
    // Libera a memória das tabelas de página de cada processo
    for (int i = 0; i < sim->num_processos; i++) {
        free(sim->processos[i].tabela_paginas);
    }
    // Libera a memória do array de processos
    free(sim->processos);
    // Libera a memória dos frames da memória física
    free(sim->memoria.frames);
    // Libera a memória dos tempos de carga dos frames
    free(sim->memoria.tempo_carga);
    // Libera a memória da estrutura principal do simulador
    free(sim);
}

// Função principal para executar o simulador de paginação de memória
int main() {
    printf("Simulador de Paginação de Memória\n");

    // Inicializa o simulador com 4 KB por página e 16 KB de memória física
    Simulador *sim = inicializar_simulador(4096, 16384);

    for (int i = 0; i < 3; i++) {
        criar_processo(sim, 16384);
    }
    printf("Criados %d processos, cada um com 4 páginas\n", sim->num_processos);

    // --- Execução da simulação com o algoritmo FIFO ---
    printf("\nExecutando simulação com FIFO...\n");
    resetar_simulador(sim); 
    executar_simulacao(sim, 0); 

    // --- Execução da simulação com o algoritmo Random ---
    printf("\nExecutando simulação com Random...\n");
    resetar_simulador(sim); 
    executar_simulacao(sim, 3); 

    // Libera toda a memória
    finalizar_simulador(sim);

    return 0; // Retorna 0 para indicar sucesso
}