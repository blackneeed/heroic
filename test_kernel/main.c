#include "boot_protocol.h"

void debug_print(const char* str) {
    for (int i = 0; str[i]; i++) {
        asm volatile ("outb %0, $0xe9" : : "a"((unsigned char)str[i]));
    }
}

void kstart(heroic_boot_protocol_data_t* boot_prot) {
    debug_print("Hello, world from the kernel loaded by heroic!\r\n");
    for (;;) asm volatile ("hlt");
}