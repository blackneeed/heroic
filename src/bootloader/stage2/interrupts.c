#include <interrupts.h>
#include <panic.h>
#include <stdio.h>

void disable_interrupts() {
    asm volatile ("cli");
}

void enable_interrupts() {
    asm volatile ("sti");
}

void isr_handler(interrupt_frame_t* frame) {
    if (frame->stub_number < 32) {
        dprintf("Exception occured.\r\n");
        hcf();
    }
}