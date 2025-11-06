#include <stdio.h>
#include <stdlib.h>
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
            
            if (config->debug_mode) {
                printf("DEBUG: [Acesso %ld] FAULT (Livre) - Página %u alocada no Quadro %d\n", 
                       current_time, page_number, i);
            }
            return;
        }
    }

    // TODO: Implementar lógica para decidir o quadro que será usado para rodar o algoritmo de substituição
    int victim_frame = 0;

    if (memoria_fisica[victim_frame].dirty_bit) {
        stats->paginas_escritas++;
    }

    if (config->debug_mode) {
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u (suja: %d) removida do Quadro %d\n", 
               current_time, memoria_fisica[victim_frame].page_id, 
               memoria_fisica[victim_frame].dirty_bit);
        printf("DEBUG: [Acesso %ld] FAULT (Subst) - Página %u alocada no Quadro %d\n", 
               current_time, page_number, victim_frame);
    }

    memoria_fisica[victim_frame].page_id = page_number;
    memoria_fisica[victim_frame].last_access_time = current_time;
    memoria_fisica[victim_frame].dirty_bit = (rw == 'W');
}