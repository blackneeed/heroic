#include <panic.h>
#include <idt.h>

void stage2_cmain() {
    *((unsigned char*)0xB8000) = 'c';
    hcf();
    idt_init();
    asm volatile ("int $0x10");
    hcf();
}