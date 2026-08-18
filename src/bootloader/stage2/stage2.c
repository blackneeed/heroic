#include <panic.h>
#include <idt.h>

void stage2_cmain() {
    idt_init();
    asm volatile ("int $0x80");
    hcf();
}