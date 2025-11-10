#include <stdio.h>
#include <string.h>

#include "simulator.h"

unsigned calcular_shift_bits(int tamanho_pagina_bytes) {
    unsigned s, tmp;
    tmp = tamanho_pagina_bytes;
    s = 0;

    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }
    return s;
}

// Algoritmo de Substituicao random 
int substituicao_random(Frame* mem_fisica, Config* config) {
    return random() % config->total_frames; }

// Algoritmo de Substituicao LRU
int substituicao_LRU(Frame* mem_fisica, Config* config) {
    long oldest = -1;
    int victim = -1;

    for (int i = 0; i < config->total_frames; i++) {
        if (victim == -1 || mem_fisica[i].last_access_time < oldest) {
            oldest = mem_fisica[i].last_access_time;
            victim = i;
        }
    }
    return victim;
}

// Algoritmo de Substituicao LFU
int substituicao_LFU(Frame* mem_fisica, Config* config) {
    long min_freq = -1;
    long oldest = -1;
    int victim = -1;

    for (int i = 0; i < config->total_frames; i++) {
        long current_freq = mem_fisica[i].frequency_count;
        long current_time = mem_fisica[i].last_access_time;


        if (victim == -1) {
            min_freq = current_freq;
            oldest = current_time;
            victim = i;
        }
        else if (current_freq < min_freq) {
            min_freq = current_freq;
            oldest = current_time;
            victim = i;
        }
        else if (current_freq == min_freq) {
            if (current_time < oldest) {
                oldest = current_time;
                victim = i;
            }
        }
    }
    return victim;
}

// Algoritmo de Substituicao MFU
int substituicao_MFU(Frame* mem_fisica, Config* config) {
    long max_freq = -1;
    int victim = -1;
    long oldest = -1;

    for (int i = 0; i < config->total_frames; i++) {
        long current_freq = mem_fisica[i].frequency_count;
        long current_time = mem_fisica[i].last_access_time;

        if (victim == -1) {
            max_freq = current_freq;
            oldest = current_time;
            victim = i;
        }
        else if (current_freq > max_freq) {
            max_freq = current_freq;
            oldest = current_time;
            victim = i;
        }
        else if (current_freq == max_freq) {
            if (current_time < oldest) {
                oldest = current_time;
                victim = i;
            }
        }
    }
    return victim;
}


int seleciona_quadro_vitima(
    Frame* mem_fisica,
    Config* config,
    char* alg_nome
) {
    if (strcmp(alg_nome, "random") == 0) {
        return substituicao_random(mem_fisica, config); }

    else if (strcmp(alg_nome, "lru") == 0) {
        return substituicao_LRU(mem_fisica, config); } 
    
    else if (strcmp(alg_nome, "lfu") == 0) {
        return substituicao_LFU(mem_fisica, config); } 
    
    else if (strcmp(alg_nome, "mfu") == 0) {
        return substituicao_MFU(mem_fisica, config); } 

    return 0;
}

// Funções de gerenciamento de paginas


void simular_acesso(
    Frame* memoria_fisica, 
    Config* config,
    Stats* stats,
    unsigned page_number, 
    char rw, 
    long current_time
) {
    for (int i = 0; i < config->total_frames; i++) {
        if (memoria_fisica[i].page_id == page_number) {
            memoria_fisica[i].last_access_time = current_time;
            memoria_fisica[i].frequency_count++;
            
            if (rw == 'W') {
                memoria_fisica[i].dirty_bit = true;
            }
            
            if (config->debug_mode) {
                printf("DEBUG: [Acesso %ld] HIT - Página %u no Quadro %d\n", 
                       current_time, page_number, i);
            }
            return;
        }
    }

    stats->paginas_lidas++; // Equivalente ao page fault

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
            return;
        }
    }

    // Substituição
    int victim_frame = seleciona_quadro_vitima(memoria_fisica, config, 
        config->algoritmo_nome);

    if (memoria_fisica[victim_frame].dirty_bit) {
        stats->paginas_escritas++;
    }

    if (config->debug_mode) {
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u (suja: %d)\n", 
               current_time, memoria_fisica[victim_frame].page_id, 
               memoria_fisica[victim_frame].dirty_bit);
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u alocada no Quadro %d\n", 
               current_time, page_number, victim_frame);
    }

    memoria_fisica[victim_frame].page_id = page_number;
    memoria_fisica[victim_frame].last_access_time = current_time;
    memoria_fisica[victim_frame].dirty_bit = (rw == 'W');
    memoria_fisica[victim_frame].frequency_count = 1;
}