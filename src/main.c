#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "simulator.h"

int parse_arguments(int argc, char *argv[], Config *config) {
    if (argc != ARGS_ESPERADOS && argc != ARGS_COM_DEBUG) {
        fprintf(stderr, "Uso: ./simulador <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_memoria_kb> [debug]\n");
        fprintf(stderr, "[1]: lru, random, lfu, etc\n");
        fprintf(stderr, "[2]: Tamanho da página em memória, em KB");
        fprintf(stderr, "[3]: Tamanho da memória, em KB");
        fprintf(stderr, "[4]: Tamanho total da memória física disponível para o processo, em KB");
        return 1;
    }

    config->algoritmo_nome = argv[1];
    config->arquivo_log = argv[2];
    config->tamanho_pagina_kb = atoi(argv[3]);
    config->tamanho_memoria_kb = atoi(argv[4]);
    config->debug_mode = (argc == ARGS_COM_DEBUG && strcmp(argv[5], "debug") == 0);

    // OBS: Talvez não sejam realmente problemáticos, mas por via das dúvidas adicionei essa verificação,
    // se não for realmente útil, pode remover. 
    if (config->tamanho_pagina_kb < 2 || config->tamanho_pagina_kb > 64) {
        fprintf(stderr, "Error: Tamanho da página (%d KB) fora do range razoável (2-64 KB)\n",
                config->tamanho_pagina_kb);
        return 1;
    }
    if (config->tamanho_memoria_kb < 128 || config->tamanho_memoria_kb > 16384) {
        fprintf(stderr, "Erro: Tamanho da memória (%d KB) fora do range razoável (128-16384 KB)\n",
                config->tamanho_memoria_kb);
        return 1;
    }

    config->shift_bits = calcular_shift_bits(config->tamanho_pagina_kb * 1024);
    config->total_frames = config->tamanho_memoria_kb / config->tamanho_pagina_kb;
    
    return 0;
}

// Inspirado no exemplo do doc, mas pode ser válido alterar para o relatório.
void print_final_report(Config* config, Stats* stats) {
    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", config->arquivo_log);
    printf("Tamanho da memória: %d KB\n", config->tamanho_memoria_kb);
    printf("Tamanho das páginas: %d KB\n", config->tamanho_pagina_kb);
    printf("Técnica de reposição: %s\n", config->algoritmo_nome);
    printf("Total de acessos: %ld\n", stats->total_acessos);
    printf("Páginas lidas: %ld\n", stats->paginas_lidas);
    printf("Páginas escritas: %ld\n", stats->paginas_escritas);
}

int main(int argc, char *argv[]) {
    Config config;
    Stats stats = {0, 0, 0};

    if (parse_arguments(argc, argv, &config) != 0) {
        return ERROR;
    }

    // TODO: Add lógica do que deve ser feito a depender do algoritmo escolhido no input

    FILE *f_in = fopen(config.arquivo_log, "r");
    if (!f_in) {
        fprintf(stderr, "Erro ao abrir o arquivo de log: %s\n", config.arquivo_log);
        return 1;
    }

    if (config.debug_mode) {
        printf("DEBUG: Algoritmo: %s, Arquivo: %s, Página: %d KB, Memória: %d KB\n",
               config.algoritmo_nome, config.arquivo_log, 
               config.tamanho_pagina_kb, config.tamanho_memoria_kb);
        printf("DEBUG: Total de quadros: %d, Shift: %u bits\n", 
               config.total_frames, config.shift_bits);
    }

    Frame *memoria_fisica = (Frame *) malloc(config.total_frames * sizeof(Frame));
    if (memoria_fisica == NULL) {
        fprintf(stderr, "Erro: Falha ao alocar memória física (%d quadros)\n", config.total_frames);
        fclose(f_in);
        return 1;
    }

    for (int i = 0; i < config.total_frames; i++) {
        memoria_fisica[i].page_id = INVALID_PAGE;
        memoria_fisica[i].dirty_bit = false;
        memoria_fisica[i].last_access_time = 0;
    }

    unsigned addr;
    char rw;

    if (config.debug_mode) {
        printf("DEBUG: Iniciando processamento do arquivo de log...\n");
    }

    while (fscanf(f_in, "%x %c", &addr, &rw) == 2) {
        stats.total_acessos++;
        //unsigned page_number = addr >> config.shift_bits;

        // TODO: Lógica de acesso à memória e simulação
    }

    fclose(f_in);

    print_final_report(&config, &stats);

    free(memoria_fisica);
    return 0;
}