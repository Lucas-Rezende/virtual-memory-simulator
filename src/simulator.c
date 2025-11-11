#include "simulator.h"

int calcular_shift_bits(int page_size_bytes) {
    int s = 0;
    unsigned tmp = (unsigned)page_size_bytes;
    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }
    return s;
}

// ALGORITMOS
int random_page(int total_frames) {
    return random() % total_frames;
}

int lru_page(Frame* memoria_fisica, int total_frames) {
    int victim = 0;
    long min_time = LONG_MAX;
    for (int i = 0; i < total_frames; i++) {
        if (memoria_fisica[i].last_access_time < min_time) {
            min_time = memoria_fisica[i].last_access_time;
            victim = i;
        }
    }
    return victim;
}

int lfu_page(Frame* memoria_fisica, int total_frames) {
    int victim = 0;
    int min_freq = INT_MAX;
    for (int i = 0; i < total_frames; i++) {
        if (memoria_fisica[i].frequency_count < min_freq) {
            min_freq = memoria_fisica[i].frequency_count;
            victim = i;
        }
    }
    return victim;
}

int mfu_page(Frame* memoria_fisica, int total_frames) {
    int victim = 0;
    int max_freq = 0;
    for (int i = 0; i < total_frames; i++) {
        if (memoria_fisica[i].frequency_count > max_freq) {
            max_freq = memoria_fisica[i].frequency_count;
            victim = i;
        }
    }
    return victim;
}

int seleciona_quadro_vitima(Frame* memoria_fisica, Config* config, const char* algoritmo_nome) {
    if (strcmp(algoritmo_nome, "random") == 0) {
        return random_page(config->total_frames);
    }
    if (strcmp(algoritmo_nome, "lru") == 0) {
        return lru_page(memoria_fisica, config->total_frames);
    }
    if (strcmp(algoritmo_nome, "lfu") == 0) {
        return lfu_page(memoria_fisica, config->total_frames);
    }
    if (strcmp(algoritmo_nome, "mfu") == 0) {
        return mfu_page(memoria_fisica, config->total_frames);
    }
    
    fprintf(stderr, "Aviso: Algoritmo '%s' desconhecido, usando 'random'.\n", algoritmo_nome);
    config->algoritmo_nome = "random";
    return random_page(config->total_frames);
}


// TABELAS DE PÁGINA

// Tabela Densa
DensePageTable* init_dense_table(Config* config) {
    DensePageTable* dense_pt = (DensePageTable*) malloc(sizeof(DensePageTable));
    if (!dense_pt) return NULL;

    config->max_adress_space = 1UL << (32 - config->shift_bits);
    dense_pt->total_entries = config->max_adress_space;

    dense_pt->entries = (PageTableEntry*) calloc(dense_pt->total_entries, sizeof(PageTableEntry));
    if (!dense_pt->entries) {
        free(dense_pt);
        return NULL;
    }
    
    if (config->debug_mode) {
        printf("DEBUG: Tabela Densa inicializada com %ld entradas\n", dense_pt->total_entries);
    }
    return dense_pt;
}

// Tabela Hierárquica 2 Níveis
Hierarchical2Table* init_hierarchical_2_table(Config* config) {
    Hierarchical2Table* h2_table = (Hierarchical2Table*) malloc(sizeof(Hierarchical2Table));
    if (!h2_table) return NULL;

    h2_table->total_entries = 1L << config->l1_bits;
    h2_table->l2_tables = (PageTableL2**) calloc(h2_table->total_entries, sizeof(PageTableL2*));
    if (!h2_table->l2_tables) {
        free(h2_table);
        return NULL;
    }

    if (config->debug_mode) {
        printf("DEBUG: Tabela Hierárq. 2 Níveis inicializada.\n");
        printf("DEBUG: L1 bits: %d (%ld entradas), L2 bits: %d\n", config->l1_bits, h2_table->total_entries, config->l2_bits);
    }
    return h2_table;
}

// Tabela Hierárquica 3 Níveis
Hierarchical3Table* init_hierarchical_3_table(Config* config) {
    Hierarchical3Table* h3_table = (Hierarchical3Table*) malloc(sizeof(Hierarchical3Table));
    if (!h3_table) return NULL;

    h3_table->total_entries = 1L << config->l1_bits_h3;
    h3_table->l2_tables = (PageTableL2_H3**) calloc(h3_table->total_entries, sizeof(PageTableL2_H3*));
    if (!h3_table->l2_tables) {
        free(h3_table);
        return NULL;
    }
    
    if (config->debug_mode) {
        printf("DEBUG: Tabela Hierárq. 3 Níveis inicializada.\n");
        printf("DEBUG: L1 bits: %d (%ld entradas), L2 bits: %d, L3 bits: %d\n", 
            config->l1_bits_h3, h3_table->total_entries, config->l2_bits_h3, config->l3_bits);
    }
    return h3_table;
}

// Tabela Invertida (Hash)
unsigned long inverted_hash_func(InvertedTable* table, unsigned page_id) {
    return page_id % table->num_buckets;
}

InvertedTable* init_inverted_table(Config* config) {
    InvertedTable* inv_table = (InvertedTable*) malloc(sizeof(InvertedTable));
    if (!inv_table) return NULL;

    inv_table->num_buckets = config->total_frames;
    inv_table->buckets = (InvertedPageTableEntry**) 
        calloc(inv_table->num_buckets, sizeof(InvertedPageTableEntry*));
    
    if (!inv_table->buckets) {
        free(inv_table);
        return NULL;
    }

    if (config->debug_mode) {
        printf("DEBUG: Tabela Invertida (Hash) inicializada com %ld baldes\n", inv_table->num_buckets);
    }
    return inv_table;
}


// GERAIS
PageTable* pt_init(Config *config) {
    PageTable* pt = (PageTable*) malloc(sizeof(PageTable));
    if (!pt) return NULL;
    
    pt->config = config;
    pt->type = config->tabela_id;
    
    switch (config->tabela_id) {
        case PT_DENSE:
            pt->data = (void*) init_dense_table(config);
            break;
        case PT_HIERARCHICAL_2:
            pt->data = (void*) init_hierarchical_2_table(config);
            break;
        case PT_HIERARCHICAL_3:
            pt->data = (void*) init_hierarchical_3_table(config);
            break;
        case PT_INVERTED:
            pt->data = (void*) init_inverted_table(config);
            break;
    }

    if (!pt->data) {
        free(pt);
        return NULL;
    }
    return pt;
}

void pt_free(PageTable *pt) {
    if (!pt) return;
    
    switch (pt->type) {
        case PT_DENSE: {
            DensePageTable* dense_pt = (DensePageTable*) pt->data;
            free(dense_pt->entries);
            free(dense_pt);
            break;
        }
        case PT_HIERARCHICAL_2: {
            Hierarchical2Table* h2_table = (Hierarchical2Table*) pt->data;
            for (long i = 0; i < h2_table->total_entries; i++) {
                if (h2_table->l2_tables[i]) {
                    free(h2_table->l2_tables[i]->entries);
                    free(h2_table->l2_tables[i]);
                }
            }
            free(h2_table->l2_tables);
            free(h2_table);
            break;
        }
        case PT_HIERARCHICAL_3: {
            Hierarchical3Table* h3_table = (Hierarchical3Table*) pt->data;
            for (long i = 0; i < h3_table->total_entries; i++) {
                if (h3_table->l2_tables[i]) {
                    PageTableL2_H3* l2 = h3_table->l2_tables[i];
                    for (long j = 0; j < l2->total_entries; j++) {
                        if (l2->l3_tables[j]) {
                            free(l2->l3_tables[j]->entries);
                            free(l2->l3_tables[j]);
                        }
                    }
                    free(l2->l3_tables);
                    free(l2);
                }
            }
            free(h3_table->l2_tables);
            free(h3_table);
            break;
        }
        case PT_INVERTED: {
            InvertedTable* inv_table = (InvertedTable*) pt->data;
            for (long i = 0; i < inv_table->num_buckets; i++) {
                InvertedPageTableEntry* entry = inv_table->buckets[i];
                while (entry) {
                    InvertedPageTableEntry* temp = entry;
                    entry = entry->next;
                    free(temp);
                }
            }
            free(inv_table->buckets);
            free(inv_table);
            break;
        }
    }
    free(pt);
}

int pt_find_frame(PageTable* pt, unsigned page_number) {
    Config* config = pt->config;

    switch (pt->type) {
        case PT_DENSE: {
            DensePageTable* dense_pt = (DensePageTable*) pt->data;
            if (page_number >= dense_pt->total_entries) return INVALID_FRAME;
            if (dense_pt->entries[page_number].bit_validade) {
                return dense_pt->entries[page_number].frame_number;
            }
            break;
        }
        case PT_HIERARCHICAL_2: {
            Hierarchical2Table* h2_table = (Hierarchical2Table*) pt->data;
            unsigned l1_index = page_number >> config->l2_bits;
            unsigned l2_index = page_number & config->l2_mask_h2;

            PageTableL2* l2_table = h2_table->l2_tables[l1_index];
            if (l2_table && l2_table->entries[l2_index].bit_validade) {
                return l2_table->entries[l2_index].frame_number;
            }
            break;
        }
        case PT_HIERARCHICAL_3: {
            Hierarchical3Table* h3_table = (Hierarchical3Table*) pt->data;
            unsigned l1_index = page_number >> (config->l2_bits_h3 + config->l3_bits);
            unsigned l2_index = (page_number >> config->l3_bits) & config->l2_mask_h3;
            unsigned l3_index = page_number & config->l3_mask_h3;

            PageTableL2_H3* l2_table = h3_table->l2_tables[l1_index];
            if (l2_table) {
                PageTableL3* l3_table = l2_table->l3_tables[l2_index];
                if (l3_table && l3_table->entries[l3_index].bit_validade) {
                    return l3_table->entries[l3_index].frame_number;
                }
            }
            break;
        }
        case PT_INVERTED: {
            InvertedTable* inv_table = (InvertedTable*) pt->data;
            unsigned long index = inverted_hash_func(inv_table, page_number);
            InvertedPageTableEntry* entry = inv_table->buckets[index];
            while (entry) {
                if (entry->page_id == page_number) {
                    return entry->frame_number;
                }
                entry = entry->next;
            }
            break;
        }
    }
    return INVALID_FRAME; // MISS
}

void pt_update_mapping(PageTable* pt, unsigned page_number, int frame_number) {
    Config* config = pt->config;

    switch (pt->type) {
        case PT_DENSE: {
            DensePageTable* dense_pt = (DensePageTable*) pt->data;
            if (page_number < dense_pt->total_entries) {
                dense_pt->entries[page_number].bit_validade = true;
                dense_pt->entries[page_number].frame_number = frame_number;
            }
            break;
        }
        case PT_HIERARCHICAL_2: {
            Hierarchical2Table* h2_table = (Hierarchical2Table*) pt->data;
            unsigned l1_index = page_number >> config->l2_bits;
            unsigned l2_index = page_number & config->l2_mask_h2;

            if (h2_table->l2_tables[l1_index] == NULL) {
                h2_table->l2_tables[l1_index] = (PageTableL2*) malloc(sizeof(PageTableL2));
                long l2_size = 1L << config->l2_bits;
                h2_table->l2_tables[l1_index]->total_entries = l2_size;
                h2_table->l2_tables[l1_index]->entries = (PageTableEntry*) calloc(l2_size, sizeof(PageTableEntry));
            }
            PageTableL2* l2_table = h2_table->l2_tables[l1_index];
            l2_table->entries[l2_index].bit_validade = true;
            l2_table->entries[l2_index].frame_number = frame_number;
            break;
        }
        case PT_HIERARCHICAL_3: {
            Hierarchical3Table* h3_table = (Hierarchical3Table*) pt->data;
            unsigned l1_index = page_number >> (config->l2_bits_h3 + config->l3_bits);
            unsigned l2_index = (page_number >> config->l3_bits) & config->l2_mask_h3;
            unsigned l3_index = page_number & config->l3_mask_h3;

            if (h3_table->l2_tables[l1_index] == NULL) {
                h3_table->l2_tables[l1_index] = (PageTableL2_H3*) malloc(sizeof(PageTableL2_H3));
                long l2_size = 1L << config->l2_bits_h3;
                h3_table->l2_tables[l1_index]->total_entries = l2_size;
                h3_table->l2_tables[l1_index]->l3_tables = (PageTableL3**) calloc(l2_size, sizeof(PageTableL3*));
            }
            PageTableL2_H3* l2_table = h3_table->l2_tables[l1_index];

            if (l2_table->l3_tables[l2_index] == NULL) {
                l2_table->l3_tables[l2_index] = (PageTableL3*) malloc(sizeof(PageTableL3));
                long l3_size = 1L << config->l3_bits;
                l2_table->l3_tables[l2_index]->total_entries = l3_size;
                l2_table->l3_tables[l2_index]->entries = (PageTableEntry*) calloc(l3_size, sizeof(PageTableEntry));
            }
            PageTableL3* l3_table = l2_table->l3_tables[l2_index];
            l3_table->entries[l3_index].bit_validade = true;
            l3_table->entries[l3_index].frame_number = frame_number;
            break;
        }
        case PT_INVERTED: {
            InvertedTable* inv_table = (InvertedTable*) pt->data;
            unsigned long index = inverted_hash_func(inv_table, page_number);
            
            InvertedPageTableEntry* new_entry = (InvertedPageTableEntry*) malloc(sizeof(InvertedPageTableEntry));
            new_entry->page_id = page_number;
            new_entry->frame_number = frame_number;
            
            new_entry->next = inv_table->buckets[index];
            inv_table->buckets[index] = new_entry;
            break;
        }
    }
}

void pt_invalidate_mapping(PageTable* pt, unsigned page_number) {
    if (page_number == INVALID_PAGE) return;
    Config* config = pt->config;

    switch (pt->type) {
        case PT_DENSE: {
            DensePageTable* dense_pt = (DensePageTable*) pt->data;
            if (page_number < dense_pt->total_entries) {
                dense_pt->entries[page_number].bit_validade = false;
            }
            break;
        }
        case PT_HIERARCHICAL_2: {
            Hierarchical2Table* h2_table = (Hierarchical2Table*) pt->data;
            unsigned l1_index = page_number >> config->l2_bits;
            unsigned l2_index = page_number & config->l2_mask_h2;

            PageTableL2* l2_table = h2_table->l2_tables[l1_index];
            if (l2_table) {
                l2_table->entries[l2_index].bit_validade = false;
            }
            break;
        }
        case PT_HIERARCHICAL_3: {
            Hierarchical3Table* h3_table = (Hierarchical3Table*) pt->data;
            unsigned l1_index = page_number >> (config->l2_bits_h3 + config->l3_bits);
            unsigned l2_index = (page_number >> config->l3_bits) & config->l2_mask_h3;
            unsigned l3_index = page_number & config->l3_mask_h3;

            PageTableL2_H3* l2_table = h3_table->l2_tables[l1_index];
            if (l2_table) {
                PageTableL3* l3_table = l2_table->l3_tables[l2_index];
                if (l3_table) {
                    l3_table->entries[l3_index].bit_validade = false;
                }
            }
            break;
        }
        case PT_INVERTED: {
            InvertedTable* inv_table = (InvertedTable*) pt->data;
            unsigned long index = inverted_hash_func(inv_table, page_number);
            
            InvertedPageTableEntry* entry = inv_table->buckets[index];
            InvertedPageTableEntry* prev = NULL;

            while (entry) {
                if (entry->page_id == page_number) {
                    if (prev == NULL) {
                        inv_table->buckets[index] = entry->next;
                    } else {
                        prev->next = entry->next;
                    }
                    free(entry);
                    return;
                }
                prev = entry;
                entry = entry->next;
            }
            break;
        }
    }
}

// SIMULAÇÃO
// Acessos únicos
void simular_acesso(
    PageTable* pt,
    Frame* memoria_fisica, 
    Config* config,
    Stats* stats,
    unsigned page_number, 
    char rw, 
    long current_time
) {
    int frame_number = pt_find_frame(pt, page_number);

    if (frame_number != INVALID_FRAME) {
        // HIT
        if (config->debug_mode) {
            printf("DEBUG: [Acesso %ld] HIT - Página %u no Quadro %d\n", 
                   current_time, page_number, frame_number);
        }

        memoria_fisica[frame_number].last_access_time = current_time;
        memoria_fisica[frame_number].frequency_count++;
        if (rw == 'W') {
            memoria_fisica[frame_number].dirty_bit = true;
        }

    } else {
        // PAGE FAULT
        
        int new_frame_number = tratar_page_fault(
            pt, memoria_fisica, config, stats, 
            page_number, rw, current_time
        );

        pt_update_mapping(pt, page_number, new_frame_number);
    }
}

int tratar_page_fault(
    PageTable* pt,
    Frame* memoria_fisica, 
    Config* config,
    Stats* stats,
    unsigned page_number, 
    char rw, 
    long current_time
) {
    stats->paginas_lidas++;

    for (int i = 0; i < config->total_frames; i++) {
        if (memoria_fisica[i].page_id == INVALID_PAGE) {
            memoria_fisica[i].page_id = page_number;
            memoria_fisica[i].last_access_time = current_time;
            memoria_fisica[i].dirty_bit = (rw == 'W');
            memoria_fisica[i].frequency_count = 1;
            
            if (config->debug_mode) {
                printf("DEBUG: [Acesso %ld] FAULT (Livre) - Página %u alocada no Quadro %d\n", 
                       current_time, page_number, i);
            }
            return i;
        }
    }

    int victim_frame = seleciona_quadro_vitima(memoria_fisica, config, config->algoritmo_nome);
    unsigned victim_page_id = memoria_fisica[victim_frame].page_id;

    if (memoria_fisica[victim_frame].dirty_bit) {
        stats->paginas_escritas++;
    }

    if (config->debug_mode) {
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u (suja: %d) será removida do Quadro %d\n", 
               current_time, victim_page_id, 
               memoria_fisica[victim_frame].dirty_bit, victim_frame);
    }

    pt_invalidate_mapping(pt, victim_page_id);

    memoria_fisica[victim_frame].page_id = page_number;
    memoria_fisica[victim_frame].last_access_time = current_time;
    memoria_fisica[victim_frame].dirty_bit = (rw == 'W');
    memoria_fisica[victim_frame].frequency_count = 1;

    if (config->debug_mode) {
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u alocada no Quadro %d\n", 
               current_time, page_number, victim_frame);
    }

    return victim_frame;
}