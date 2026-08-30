#ifndef BOOT_PROTOCOL_STRUCTURE
#define BOOT_PROTOCOL_STRUCTURE
#include <stdint.h>

#define HEROIC_BOOT_PROTOCOL_MAGIC 0x8585AEAEEAEA5858ULL

typedef enum {
    reserved_memory_type,
    efi_loader_code,
    efi_loader_data,
    efi_boot_services_code,
    efi_boot_services_data,
    efi_runtime_services_code,
    efi_runtime_services_data,
    conventional_memory,
    unusable_memory,
    acpi_reclaimable,
    acpi_memory_nvs,
    memory_mapped_io,
    memory_mapped_io_port_space,
    pal_code,
    persistent_memory,
    unaccepted_memory_type,
    max_memory_type
} heroic_memory_type;

#define HEROIC_MEMORY_UC            0x0000000000000001
#define HEROIC_MEMORY_WC            0x0000000000000002
#define HEROIC_MEMORY_WT            0x0000000000000004
#define HEROIC_MEMORY_WB            0x0000000000000008
#define HEROIC_MEMORY_UCE           0x0000000000000010
#define HEROIC_MEMORY_WP            0x0000000000001000
#define HEROIC_MEMORY_RP            0x0000000000002000
#define HEROIC_MEMORY_XP            0x0000000000004000
#define HEROIC_MEMORY_RO            0x0000000000020000
#define HEROIC_MEMORY_NV            0x0000000000008000
#define HEROIC_MEMORY_RUNTIME       0x8000000000000000
#define HEROIC_MEMORY_MORE_RELIABLE 0x0000000000010000
#define HEROIC_MEMORY_SP            0x0000000000040000
#define HEROIC_MEMORY_CPU_CRYPTO    0x0000000000080000
#define HEROIC_MEMORY_HOT_PLUGGABLE 0x0000000000100000
#define HEROIC_MEMORY_ISA_VALID     0x4000000000000000
#define HEROIC_MEMORY_ISA_MASK      0x0FFFF00000000000

typedef struct {
    uint32_t type;
    uint32_t _pad;
    uint64_t phys_addr;
    uint64_t _reserved;
    uint64_t page_count;
    uint64_t attributes;
} heroic_memory_descriptor;

typedef struct _heroic_boot_protocol_data {
    uint64_t magic;
    uint64_t size;

    uint64_t memory_map;
    uint64_t memory_map_size;
    uint64_t memory_map_descriptor_size;
    
    uint64_t hhdm;
} heroic_boot_protocol_data_t;

#endif