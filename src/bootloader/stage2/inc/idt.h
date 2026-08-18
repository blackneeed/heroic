#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define INTERRUPT_GATE 0x8E

typedef struct _idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

typedef struct _idt_entry {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
} __attribute__((packed)) idt_entry_t;

void lidt(idtr_t* descriptor);
void idt_init();
#endif