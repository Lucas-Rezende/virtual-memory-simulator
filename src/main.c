#include "simulator.h"
#include <time.h>

void set_table_type(Config* config, const char* table_name) {
    if (strcmp(table_name, "densa") == 0) {
        config->tabela_id = PT_DENSE;
    } else if (strcmp(table_name, "h2") == 0) {
        config->tabela_id = PT_HIERARCHICAL_2;
    } else if (strcmp(table_name, "h3") == 0) {
        config->tabela_id = PT_HIERARCHICAL_3;
    } else if (strcmp(table_name, "invertida") == 0) {
        config->tabela_id = PT_INVERTED;
    } else {
        fprintf(stderr, "Erro: Tipo de tabela '%s' desconhecido.\n", table_name);
        fprintf(stderr, "Use: densa, h2, h3, ou invertida\n");
        exit(EXIT_FAILURE);
    }
}

// PARSING
void parse_arguments(int argc, char* argv[], Config* config) {
    const char* USAGE_STRING = "Uso: ./simulador <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_memoria_kb> <tabela> [debug]\n";

    if (argc < 6) {
        fprintf(stderr, "Erro: Argumentos insuficientes.\n");
        fprintf(stderr, "%s", USAGE_STRING);
        exit(EXIT_FAILURE);
    }

    config->algoritmo_nome = argv[1];
    config->arquivo_log_nome = argv[2];
    config->page_size_kb = atoi(argv[3]);
    config->mem_size_kb = atoi(argv[4]);
    set_table_type(config, argv[5]);

    config->debug_mode = false;
    if (argc == 7 && strcmp(argv[6], "debug") == 0) {
        config->debug_mode = true;
    }

    if (config->page_size_kb <= 0 || config->mem_size_kb <= 0) {
        fprintf(stderr, "Erro: Tamanho da página e da memória devem ser > 0.\n");
        exit(EXIT_FAILURE);
    }
    if (config->mem_size_kb < config->page_size_kb) {
        fprintf(stderr, "Erro: Memória total deve ser maior que o tamanho da página.\n");
        exit(EXIT_FAILURE);
    }
}

void print_final_report(Config* config, Stats* stats) {
    printf("--- Resultaldos ---\n");
    printf("Total de acessos: %ld\n", stats->total_acessos);
    printf("Páginas lidas: %ld\n", stats->paginas_lidas);
    printf("Porcentagem páginas lidas: %.2f%%\n",
           (double)stats->paginas_lidas * 100.0 / stats->total_acessos);
    printf("Páginas escritas: %ld\n", stats->paginas_escritas);
    printf("Porcentagem páginas escritas: %.2f%%\n",
           (double)stats->paginas_escritas * 100.0 / stats->total_acessos);
}

int main(int argc, char *argv[]) {
    Config config;
    Stats stats = {0, 0, 0};

    parse_arguments(argc, argv, &config);

    srandom((unsigned int)time(NULL));

    config.shift_bits = calcular_shift_bits(config.page_size_kb * 1024);
    config.total_frames = (config.mem_size_kb * 1024) / (config.page_size_kb * 1024);

    int total_page_bits = 32 - config.shift_bits;
    
    config.l2_bits = 10;
    config.l1_bits = total_page_bits - config.l2_bits;
    if (config.l1_bits <= 0) {
        config.l1_bits = 1;
        config.l2_bits = total_page_bits - config.l1_bits;
    }
    config.l2_mask_h2 = (1 << config.l2_bits) - 1;

    config.l3_bits = 10;
    config.l2_bits_h3 = 10;
    config.l1_bits_h3 = total_page_bits - config.l2_bits_h3 - config.l3_bits;
    if (config.l1_bits_h3 <= 0) {
        config.l1_bits_h3 = 1;
        config.l2_bits_h3 = (total_page_bits - 1) / 2;
        config.l3_bits = total_page_bits - 1 - config.l2_bits_h3;
    }
    config.l3_mask_h3 = (1 << config.l3_bits) - 1;
    config.l2_mask_h3 = (1 << config.l2_bits_h3) - 1;

    FILE *f_in = fopen(config.arquivo_log_nome, "r");
    if (f_in == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", config.arquivo_log_nome);
        return 1;
    }

    Frame *memoria_fisica = (Frame *) malloc(config.total_frames * sizeof(Frame));
    if (memoria_fisica == NULL) {
        fprintf(stderr, "Erro ao alocar memória física\n");
        fclose(f_in);
        return 1;
    }
    for (int i = 0; i < config.total_frames; i++) {
        memoria_fisica[i].page_id = INVALID_PAGE;
        memoria_fisica[i].dirty_bit = false;
        memoria_fisica[i].last_access_time = 0;
        memoria_fisica[i].frequency_count = 0;
    }

    PageTable *pt = pt_init(&config);
    if (pt == NULL) {
        fprintf(stderr, "Erro ao inicializar tabela de páginas\n");
        free(memoria_fisica);
        fclose(f_in);
        return 1;
    }

    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", config.arquivo_log_nome);
    printf("Tamanho da memória: %d KB\n", config.mem_size_kb);
    printf("Tamanho das páginas: %d KB\n", config.page_size_kb);
    printf("Técnica de reposição: %s\n", config.algoritmo_nome);
    printf("Tipo de Tabela: %s\n", argv[5]);


    unsigned addr;
    char rw;
    long current_time = 0;

    while (fscanf(f_in, "%x %c", &addr, &rw) == 2) {
        current_time++;
        stats.total_acessos++;
        
        unsigned page_number = addr >> config.shift_bits;

        simular_acesso(pt, memoria_fisica, &config, &stats, page_number, rw, current_time);
    }

    fclose(f_in);
    print_final_report(&config, &stats);

    pt_free(pt);
    free(memoria_fisica);
    
    return 0;
}