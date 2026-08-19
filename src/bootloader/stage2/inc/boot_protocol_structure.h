#ifndef BOOT_PROTOCOL_STRUCTURE
#define BOOT_PROTOCOL_STRUCTURE
#include <stdint.h>

#define HEROIC_BOOT_PROTOCOL_MAGIC 0x8585AEAEEAEA5858ULL
#define HEROIC_BOOT_PROTOCOL_VERSION 1

#define HEROIC_BOOT_PROTOCOL_MEMORY_MAP_TYPE_FREE 1
#define HEROIC_BOOT_PROTOCOL_MEMORY_MAP_TYPE_RESERVED 2
#define HEROIC_BOOT_PROTOCOL_MEMORY_MAP_TYPE_ACPI_RECLAIMABLE 3
#define HEROIC_BOOT_PROTOCOL_MEMORY_MAP_TYPE_ACPI_NVS 4
#define HEROIC_BOOT_PROTOCOL_MEMORY_MAP_TYPE_BAD 5

typedef struct _heroic_boot_protocol_memory_map_entry {
    uint64_t address;
    uint64_t length;
    uint64_t type;
} heroic_boot_protocol_memory_map_entry_t;

typedef struct _heroic_boot_protocol_data {
    // DISCLAIMER: The memory range 0x7C00–0x8000 is reserved by Heroic only while this structure and/or it's contents are in use. After the kernel has finished consuming those, this range may be reclaimed as normal physical memory. 
    // This is not highlighted in the memory map.

    uint64_t magic; // Check if this is 0x8585AEAEEAEA5858ULL
    uint64_t version; // Check if this is equal to the HEROIC_BOOT_PROTOCOL_VERSION in the header in use, if not, then you are using an outdated header.
    uint64_t size; // Depends on the version.

    uint64_t memory_map; // Pointer to the first heroic_boot_protocol_memory_map_entry_t.
    uint64_t memory_map_entry_count; // Amount of memory map entries present.

    uint64_t rsdp; // Zero if not found.
} heroic_boot_protocol_data_t;

static_assert(sizeof(heroic_boot_protocol_data_t) < 256);
#endif