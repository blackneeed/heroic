#ifndef MMAP_H
#define MMAP_H
#include <stdint.h>
#include <boot_protocol_structure.h>

#define MMAP_TYPE_FREE 1
#define MMAP_TYPE_RESERVED 2
#define MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MMAP_TYPE_ACPI_NVS 4
#define MMAP_TYPE_BAD 5

typedef struct _memory_map_entry {
    uint64_t address;
    uint64_t length;
    uint32_t type;
    uint32_t extended_attributes;
} __attribute__((packed)) memory_map_entry_t;

void mmap_init(heroic_boot_protocol_data_t* boot_prot_data);
#endif