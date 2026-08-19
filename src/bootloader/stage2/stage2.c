#include <panic.h>
#include <idt.h>
#include <vga.h>
#include <pic.h>
#include <stdio.h>

void stage2_cmain() {
    vga_text_mode_clear();
    pic_mask_all(true);
    idt_init();
    printf("end of heroic stage2");
    hcf();
}