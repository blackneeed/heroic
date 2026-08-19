#include <panic.h>
#include <idt.h>
#include <vga.h>
#include <pic.h>

void stage2_cmain() {
    vga_text_mode_clear();
    pic_mask_all(true);
    idt_init();
    vga_text_mode_print_string("heroic stage2 initialized\r\n");
    hcf();
}