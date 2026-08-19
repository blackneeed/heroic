#include <boot_protocol_structure.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void find_rsdp(heroic_boot_protocol_data_t* boot_prot_data) {
    boot_prot_data->rsdp = 0;
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Warray-bounds"
    uint64_t ebda_address = (uint64_t)(*(volatile uint16_t*)(uintptr_t)0x40E) << 4;
    #pragma GCC diagnostic pop

    dprintf("attempting to find root system description pointer in the extended bios data area located at 0x%p\r\n", ebda_address);
    for (uint64_t i = 0; i + 8 <= 1024; i += 16) {
        if (!memcmp((char*)(ebda_address + i), "RSDP PTR", 8)) {
            dprintf("found root system description pointer at 0x%p\r\n", ebda_address + i);
            boot_prot_data->rsdp = ebda_address + i;
            return;
        }
    }

    dprintf("attempting to find root system description pointer in the bios expansions area located at 0x%p\r\n", 0xe0000);
    for (uint64_t addr = 0xE0000; addr + 8 <= 0xFFFFF; addr += 16) {
        if (!memcmp((char*)addr, "RSD PTR ", 8)) {
            dprintf("found root system description pointer at 0x%p\r\n", addr);
            boot_prot_data->rsdp = addr;
            return;
        }
    }

    dprintf("didn't find the root system description pointer\r\n");
}