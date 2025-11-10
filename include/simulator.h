#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define INVALID_PAGE (unsigned)(-1)
#define INVALID_FRAME -1

typedef enum {
    PT_DENSE,
    PT_HIERARCHICAL_2,
    PT_HIERARCHICAL_3,
    PT_INVERTED
} PageTableType;

typedef struct {
    int frame_number;
    bool bit_validade;
} PageTableEntry;

// --- Tabela Densa ---
typedef struct {
    PageTableEntry *entries;
    long total_entries;
} DensePageTable;

// Tabela Hierárquica de 2 Níveis
// Nível 2 (L2) - Aponta para os quadros
typedef struct {
    PageTableEntry *entries;
    long total_entries;
} PageTableL2;

typedef struct {
    PageTableL2 **l2_tables;
    long total_entries;
} Hierarchical2Table;

// Tabela Hierárquica de 3 Níveis
// Nível 3 (L3) - Aponta para os quadros
typedef struct {
    PageTableEntry *entries;
    long total_entries;
} PageTableL3;

// Nível 2 (L2) - Aponta para tabelas L3
typedef struct {
    PageTableL3 **l3_tables;
    long total_entries;
} PageTableL2_H3;

// Nível 1 (L1) - Aponta para tabelas L2
typedef struct {
    PageTableL2_H3 **l2_tables;
    long total_entries;
} Hierarchical3Table;

//  Tabela Invertida - HASH 
typedef struct InvertedPageTableEntry {
    unsigned page_id;
    int frame_number;
    struct InvertedPageTableEntry* next;
} InvertedPageTableEntry;

typedef struct {
    InvertedPageTableEntry **buckets;
    long num_buckets;
} InvertedTable;


// AUXILIARES
typedef struct Config Config;
typedef struct {
    PageTableType type;
    void* data;
    Config* config;
} PageTable;

struct Config {
    char* algoritmo_nome;
    char* arquivo_log_nome;
    int page_size_kb;
    int mem_size_kb;
    PageTableType tabela_id;
    bool debug_mode;

    int shift_bits;
    int total_frames;
    unsigned long max_adress_space;

    int l1_bits;
    int l2_bits;
    unsigned l2_mask_h2;

    int l1_bits_h3;
    int l2_bits_h3;
    int l3_bits;
    unsigned l2_mask_h3;
    unsigned l3_mask_h3;
};

typedef struct {
    unsigned page_id;
    bool dirty_bit;
    long last_access_time;
    int frequency_count;
} Frame;

typedef struct {
    long total_acessos;
    long paginas_lidas;
    long paginas_escritas;
} Stats;

int calcular_shift_bits(int page_size_bytes);

PageTable* pt_init(Config *config);
void pt_free(PageTable *pt);
int pt_find_frame(PageTable* pt, unsigned page_number);
void pt_update_mapping(PageTable* pt, unsigned page_number, int frame_number);
void pt_invalidate_mapping(PageTable* pt, unsigned page_number);

void simular_acesso(
    PageTable* pt,
    Frame* memoria_fisica,
    Config* config,
    Stats* stats,
    unsigned page_number,
    char rw,
    long current_time
);

int tratar_page_fault(
    PageTable* pt,
    Frame* memoria_fisica,
    Config* config,
    Stats* stats,
    unsigned page_number,
    char rw,
    long current_time
);

int seleciona_quadro_vitima(
    Frame* memoria_fisica,
    Config* config,
    const char* algoritmo_nome
);

#endif