#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stdlib.h>

#define ERROR -1 
#define INVALID_PAGE -1 
#define ARGS_ESPERADOS 5
#define ARGS_COM_DEBUG 6

// ESTRUTURAS DE TABELAS
typedef enum {
    PT_DENSE,
    PT_HIERARCHICAL,
    PT_INVERTED
} PageTableType;

typedef struct {
    int frame_number;
    bool present;
} PageTableEntry;

typedef struct {
    PageTableEntry *entries;
    long total_entries;
} DensePageTable;

typedef struct {
    PageTableType type;
    union {
        DensePageTable dense;
    } data;
} PageTable;

// ESTRUTURAS ADICIONAIS
typedef struct {
    char* algoritmo_nome;

    PageTableType tabela_id;

    char* arquivo_log;
    int tamanho_pagina_kb;
    int tamanho_memoria_kb;
    bool debug_mode;
    
    int total_frames;
    unsigned shift_bits;

    long max_adress_space;
} Config;

typedef struct {
    int page_id;            
    bool dirty_bit;         
    long last_access_time;
    long frequency_count;
} Frame;

typedef struct {
    long total_acessos;
    long paginas_lidas; // Equivalente a Page Faults
    long paginas_escritas;
} Stats;


unsigned calcular_shift_bits(int tamanho_pagina_bytes);

//PageTable* init(Config *config);
//void free_table(PageTable *pt);

void simular_acesso(
    Frame* memoria_fisica, 
    Config* config,
    Stats* stats,
    unsigned page_number, 
    char rw, 
    long current_time
);

int substituicao_random(Frame* mem_fisica, Config* config);
int substituicao_LRU(Frame* mem_fisica, Config* config);
int substituicao_LFU(Frame* mem_fisica, Config* config);
int substituicao_MFU(Frame* mem_fisica, Config* config);


#endif