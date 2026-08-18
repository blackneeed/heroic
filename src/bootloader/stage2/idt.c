#include <stdint.h>
#include <idt.h>
#include <interrupts.h>

static idtr_t idtr;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256];

void lidt(idtr_t* descriptor) {
    asm volatile ("lidt %0" : : "m"(descriptor));
}

void idt_set(uint16_t vector, void* isr) {
    idt_entry_t* desc = &idt[vector];

    desc->offset_1 = (uint16_t)((uint64_t)isr & 0xFFFF);
    desc->offset_2 = (uint16_t)(((uint64_t)isr >> 16) & 0xFFFF);
    desc->offset_3 = (uint32_t)((uint64_t)isr >> 32);
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
    enable_interrupts();
}