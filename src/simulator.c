#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <simulator.h>

int main(int argc, char *argv[]) {
    ReasonableRange pages_and_frames_range = {2, 64};
    ReasonableRange available_physic_memory_range = {128, 16384};
    bool is_reasonable_value = true;
    char *inputFile = argv[2];
    FILE *f_in = fopen(inputFile, "r");

    if (argc != 5) {
        fprintf(stderr, "Uso: simulador lru arquivo.log [1] [2]");
        fprintf(stderr, "[1]: Tamanho de cada página/quadro de memória, em KB");
        fprintf(stderr, "[2]: Tamanho total da memória física disponível para o processo, em KB");
        return 1;
    } else if (!f_in) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", inputFile);
            return 1;
    }

    /* if ((argv[3] >= pages_and_frames_range.lower_value && argv[3] <= pages_and_frames_range.upper_value) ||
        (argv[4] >= available_physic_memory_range.lower_value && argv[4] <= available_physic_memory_range.upper_value))
    {
        is_reasonable_value = false;
    } */
    
    return 0;
}