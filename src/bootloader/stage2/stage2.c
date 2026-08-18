#include <panic.h>
#include <idt.h>

void stage2_cmain() {
    idt_init();
    hcf();
}