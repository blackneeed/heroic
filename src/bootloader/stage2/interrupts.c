#include <interrupts.h>

void disable_interrupts() {
    asm volatile ("cli");
}

void enable_interrupts() {
    asm volatile ("sti");
}

void isr_handler(interrupt_frame_t* frame) {
    if (frame->stub_number == 0x10) {
        *((int8_t*)0xb8000) = (int8_t)'a';
    } else {
        *((int8_t*)0xb8000) = (int8_t)'b';
    }
}