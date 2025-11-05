#ifndef SIMULATOR
#define SIMULATOR

#define ARGS_ESPERADOS 5
#define ARGS_COM_DEBUG 6

typedef struct {
    size_t lower_value;
    size_t upper_value;
} ReasonableRange;

/* Since the operating system is managing physical memory, it must be aware of the allocation details of physical memory—which frames are allocated, which frames are available, how many total frames there are, and so on. This information is generally kept in a single, system-wide data structure called a frame table.*/
typedef struct {
    int page_id;            
    bool dirty_bit;         
    long last_access_time;
} Frame;

unsigned calcular_shift_bits(int tamanho_pagina_kb);

#endif