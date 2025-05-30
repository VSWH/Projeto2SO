#ifndef SIMULATOR_H
#define SIMULATOR_H
// Header file que é responsável por declarar structs e métodos

typedef struct {
    int presente;       
    int frame;          
    int modificada;     
    int referencia;     
    int tempo_carga;    
    int ultimo_acesso;  
} Pagina;

typedef struct {
    int pid;                
    int tamanho;            
    int num_paginas;       
    Pagina *tabela_paginas; 
} Processo;

typedef struct {
    int num_frames;     
    int *frames;        
    int *tempo_carga;   
} MemoriaFisica;

typedef struct {
    int tempo_atual;            
    int tamanho_pagina;         
    int tamanho_memoria_fisica; 
    int num_processos;          
    Processo *processos;        
    MemoriaFisica memoria;      
    int total_acessos;          
    int page_faults;            
    int algoritmo;             
} Simulador;

Simulador* inicializar_simulador(int tamanho_pagina, int tamanho_memoria_fisica);
Processo* criar_processo(Simulador *sim, int tamanho_processo);
void extrair_pagina_deslocamento(Simulador *sim, int endereco_virtual, int *pagina, int *deslocamento);
int verificar_pagina(Simulador *sim, int pid, int pagina);
int carregar_pagina(Simulador *sim, int pid, int pagina);
int substituir_pagina_fifo(Simulador *sim);
int substituir_pagina_random(Simulador *sim);
int traduzir_endereco(Simulador *sim, int pid, int endereco_virtual);
void exibir_memoria_fisica(Simulador *sim);
void exibir_estatisticas(Simulador *sim);
int acessar_memoria(Simulador *sim, int pid, int endereco_virtual);
void executar_simulacao(Simulador *sim, int algoritmo);

#endif 