#include <panic.h>
#include <idt.h>
#include <vga.h>

void stage2_cmain() {
    vga_text_mode_clear();
    idt_init();
    vga_text_mode_print_string("Welcome from stage2!");
    hcf();
}