#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stdlib.h>

#define ERROR -1 
#define INVALID_PAGE -1 
#define ARGS_ESPERADOS 5
#define ARGS_COM_DEBUG 6

typedef struct {
    char* algoritmo_nome;
    char* arquivo_log;
    int tamanho_pagina_kb;
    int tamanho_memoria_kb;
    bool debug_mode;
    
    int total_frames;
    unsigned shift_bits;
} Config;

typedef struct {
    int page_id;            
    bool dirty_bit;         
    long last_access_time;
} Frame;

typedef struct {
    long total_acessos;
    long paginas_lidas; // Equivalente a Page Faults
    long paginas_escritas;
} Stats;


unsigned calcular_shift_bits(int tamanho_pagina_bytes);

void simular_acesso(
    Frame* memoria_fisica, 
    Config* config,
    Stats* stats,
    unsigned page_number, 
    char rw, 
    long current_time
);

// TODO: Aqui seriam adicionados os eventuais algoritmos e algum método auxiliar para decidir qual
// seria usado, dependendo do input.

#endif