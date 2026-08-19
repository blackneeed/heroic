#include <stdint.h>
#include <stdbool.h>
#include <portio.h>
#include <pic.h>
#include <stdio.h>

void pic_init() {
    pic_remap(32, 40);
    pic_mask_all(false);
}

void pic_remap(uint8_t o1, uint8_t o2) {
    outb(PIC1_CMD, PIC_INIT | PIC_ICW4_PRESENT);
    outb(PIC2_CMD, PIC_INIT | PIC_ICW4_PRESENT);
    outb(PIC1_DAT, o1);
    outb(PIC2_DAT, o2);
    outb(PIC1_DAT, 4); // irq2 - slave pic
    outb(PIC2_DAT, 2);
    outb(PIC1_DAT, PIC_8086_MODE);
    outb(PIC2_DAT, PIC_8086_MODE);
}

void pic_mask(uint8_t irq, bool mask) {
    uint16_t port = PIC1_DAT;
    if (irq >= 8) { port = PIC2_DAT; irq -= 8; }

    if (mask) outb(port, inb(port) | ~(1 << irq));
    else outb(port, inb(port) & (1 << irq));
}

void pic_mask_all(bool mask) {
    uint8_t val = 0;
    if (mask) val = 0xFF;
    outb(PIC1_DAT, val);
    outb(PIC2_DAT, val);
    if (mask) dprintf("disabled programmable interrupt controller\r\n");
    else dprintf("enabled programmable interrupt controller\r\n");
}