#include <mmap.h>
#include <stdio.h>
#include <boot_protocol_structure.h>

void mmap_init(heroic_boot_protocol_data_t* boot_prot_data) {
    memory_map_entry_t* memory_map_entry = (memory_map_entry_t*)0x7c00;

    (void)boot_prot_data;
    heroic_boot_protocol_memory_map_entry_t entries[20];
    (void)entries;

    uint64_t index = 0;

    for (;;) {
        if (memory_map_entry->address == 0 && memory_map_entry->length == 0 && memory_map_entry->type == 0 && memory_map_entry->extended_attributes == 0) break;

        const char* type_str;

        switch (memory_map_entry->type) {
            case MMAP_TYPE_FREE:
                if (!(memory_map_entry->extended_attributes & 1)) {
                    type_str = "reserved";
                    memory_map_entry->type = MMAP_TYPE_RESERVED;
                } else type_str = "free";
                break;
            case MMAP_TYPE_RESERVED: type_str = "reserved"; break;
            case MMAP_TYPE_ACPI_RECLAIMABLE: type_str = "reclaimable after acpi setup"; break;
            case MMAP_TYPE_ACPI_NVS: type_str = "reserved for acpi nvs"; break;
            case MMAP_TYPE_BAD: type_str = "bad"; break;
            default: type_str = "?"; break;
        }

        dprintf("entry %d: 0x%p-0x%p is %s\r\n", index, memory_map_entry->address, memory_map_entry->address + memory_map_entry->length, type_str);

        // this just usually makes this struct be the structure that the boot protocol uses for mmap entries (uint64_t for type)
        memory_map_entry->extended_attributes = memory_map_entry->type;
        memory_map_entry->type = 0;

        index++;
        memory_map_entry++;
    }

    boot_prot_data->memory_map_entry_count = index;
    boot_prot_data->memory_map = 0x7c00;
}