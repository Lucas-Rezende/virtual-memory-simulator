#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <simulator.h>

unsigned calcular_shift_bits(int tamanho_pagina) {
    unsigned s, page_size, tmp;
    page_size = tamanho_pagina;
    tmp = page_size;
    s = 0;

    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }

    return s;
}

int main(int argc, char *argv[]) {
    if (argc != ARGS_ESPERADOS && argc != ARGS_COM_DEBUG) {
        fprintf(stderr, "Uso: ./simulador <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_memoria_kb> [debug]\n");
        fprintf(stderr, "[1]: lru, random, lfu, etc\n");
        fprintf(stderr, "[2]: Tamanho da página em memória, em KB");
        fprintf(stderr, "[3]: Tamanho da memória, em KB");
        fprintf(stderr, "[4]: Tamanho total da memória física disponível para o processo, em KB");
        return 1;
    }

    char *algoritmo = argv[1];
    char *inputFile = argv[2];
    int tamanho_pagina_kb = atoi(argv[3]);
    int tamanho_memoria_kb = atoi(argv[4]);

    bool debug_mode = false;
    if (argc == ARGS_COM_DEBUG && strcmp(argv[5], "debug") == 0) {
        debug_mode = true;
    }

    FILE *f_in = fopen(inputFile, "r");
    if (!f_in) {
        fprintf(stderr, "Erro ao abrir o arquivo de log: %s\n", inputFile);
        return 1;
    }

    ReasonableRange pages_and_frames_range = {2, 64};
    ReasonableRange available_physic_memory_range = {128, 16384};

    if (tamanho_pagina_kb < pages_and_frames_range.lower_value || 
        tamanho_pagina_kb > pages_and_frames_range.upper_value) {
        
        fprintf(stderr, "Error: Tamanho da página (%d KB) fora do range razoável (%d-%d KB)\n",
                tamanho_pagina_kb, pages_and_frames_range.lower_value, pages_and_frames_range.upper_value);
        fclose(f_in);
        return 1;
    }

    if (tamanho_memoria_kb < available_physic_memory_range.lower_value || 
        tamanho_memoria_kb > available_physic_memory_range.upper_value) {
        
        fprintf(stderr, "Erro: Tamanho da memória (%d KB) fora do range razoável (%d-%d KB)\n",
                tamanho_memoria_kb, available_physic_memory_range.lower_value, available_physic_memory_range.upper_value);
        fclose(f_in);
        return 1;
    }

    /* if ((argv[3] >= pages_and_frames_range.lower_value && argv[3] <= pages_and_frames_range.upper_value) ||
        (argv[4] >= available_physic_memory_range.lower_value && argv[4] <= available_physic_memory_range.upper_value))
    {
        is_reasonable_value = false;
    } */
    
    return 0;
}