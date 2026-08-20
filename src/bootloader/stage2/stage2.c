#include <panic.h>
#include <idt.h>
#include <vga.h>
#include <pic.h>
#include <stdio.h>
#include <mmap.h>
#include <boot_protocol_structure.h>
#include <rsdp.h>

heroic_boot_protocol_data_t* boot_prot_data = (heroic_boot_protocol_data_t*)0x7f00;

void stage2_cmain() {
    vga_text_mode_clear();
    pic_mask_all(true);
    idt_init();

    // start filling in the boot prot structure

    boot_prot_data->magic = HEROIC_BOOT_PROTOCOL_MAGIC;
    boot_prot_data->version = HEROIC_BOOT_PROTOCOL_VERSION;
    boot_prot_data->size = sizeof(heroic_boot_protocol_data_t);

    mmap_init(boot_prot_data);
    find_rsdp(boot_prot_data);

    printf("end of heroic stage2");
    hcf();
}