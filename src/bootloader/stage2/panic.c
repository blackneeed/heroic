#include <interrupts.h>
#include <panic.h>

void hcf() {
    disable_interrupts();
    for (;;) halt();
}

void halt() {
    asm volatile ("hlt");
}