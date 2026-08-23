#include <stdint.h>
#include <idt.h>
#include <stdio.h>
#include <interrupts.h>

idtr_t idtr;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256];

void lidt(idtr_t* descriptor) {
    asm volatile ("lidt %0" : : "m"(*descriptor));
}

void idt_set(uint16_t vector, void* isr) {
    idt_entry_t* desc = &idt[vector];
    uint64_t addr = (uint64_t)isr;

    desc->offset_1 = addr & 0xFFFF;
    desc->offset_2 = (addr >> 16) & 0xFFFF;
    desc->offset_3 = addr >> 32;
    desc->selector = 0x08;
    desc->ist = 0;
    desc->type_attributes = INTERRUPT_GATE;
    desc->zero = 0;
}

void idt_init() {
    idtr.base = (uint64_t)&idt[0];
    idtr.limit = (uint16_t)(sizeof(idt_entry_t) * 256) - 1;

    for (uint16_t vec = 0; vec < 256; vec++) {
        idt_set(vec, isr_stub_table[vec]);
    }

    lidt(&idtr);
    dprintf("idtr 0x%p loaded\r\n", &idtr);
    enable_interrupts();
    dprintf("interrupts enabled\r\n");
}