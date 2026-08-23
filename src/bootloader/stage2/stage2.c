#include <panic.h>
#include <idt.h>
#include <vga.h>
#include <pic.h>
#include <stdio.h>
#include <mmap.h>
#include <boot_protocol_structure.h>
#include <rsdp.h>
#include <x86_64.h>

extern uint8_t drive_number;
extern uint16_t debug_1[];
extern uint16_t debug_2[];
extern uint16_t debug_3[];

heroic_boot_protocol_data_t* boot_prot_data = (heroic_boot_protocol_data_t*)0x7f00;

extern void mode_switch_test();

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

    vga_text_mode_clear();
    mode_switch_test();
    printf("Test complete\r\n");
    printf("before first 13h      ax: 0x%x bx: 0x%x cx: 0x%x dx: 0x%x si: 0x%x di: 0x%x bp: 0x%x sp: 0x%x ss: 0x%x ds: 0x%x es: 0x%x cs: 0x%x, flags: 0x%x\r\n", debug_1[0], debug_1[1], debug_1[2], debug_1[3], debug_1[4], debug_1[5], debug_1[6], debug_1[7], debug_1[8], debug_1[9], debug_1[10], debug_1[11], debug_1[12]);
    printf("after first 13h       ax: 0x%x bx: 0x%x cx: 0x%x dx: 0x%x si: 0x%x di: 0x%x bp: 0x%x sp: 0x%x ss: 0x%x ds: 0x%x es: 0x%x cs: 0x%x, flags: 0x%x\r\n", debug_2[0], debug_2[1], debug_2[2], debug_2[3], debug_2[4], debug_2[5], debug_2[6], debug_2[7], debug_2[8], debug_2[9], debug_2[10], debug_2[11], debug_2[12]);
    printf("in second mode switch ax: 0x%x bx: 0x%x cx: 0x%x dx: 0x%x si: 0x%x di: 0x%x bp: 0x%x sp: 0x%x ss: 0x%x ds: 0x%x es: 0x%x cs: 0x%x, flags: 0x%x\r\n", debug_3[0], debug_3[1], debug_3[2], debug_3[3], debug_3[4], debug_3[5], debug_3[6], debug_3[7], debug_3[8], debug_3[9], debug_3[10], debug_3[11], debug_3[12]);

 

    hcf();

    disk_params_t disk_params;

    disk_params.drive_number = drive_number;

    bios_check_edd_presence(&disk_params);
    printf("returned from edd presence\r\n");
    bios_get_disk_geometry(&disk_params);
    printf("returned from geometry\r\n");

    const char* temp = " not";

    if (disk_params.edd_present) {
        temp = "";
    }

    printf("info about disk:\r\n  drive number 0x%x\r\n  edd%s present\r\n  head count %d\r\n  sectors per track %d\r\n", disk_params.drive_number, temp, disk_params.heads, disk_params.sectors_per_track);

    printf("end of heroic stage2");
    hcf();
}