#ifndef MMAP_H
#define MMAP_H
#include <stdint.h>

typedef struct _memory_map {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t ext_attr;
} memory_map_t;
#endif