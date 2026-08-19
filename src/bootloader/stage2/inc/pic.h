#ifndef PIC_H
#define PIC_H
#include <stdbool.h>
#include <stdint.h>

#define PIC1_CMD 0x20
#define PIC1_DAT 0x21
#define PIC2_CMD 0xA0
#define PIC2_DAT 0xA1

#define PIC_EOI 0x20
#define PIC_INIT 0x10
#define PIC_ICW4_PRESENT 0x01
#define PIC_8086_MODE 0x01

void pic_init();
void pic_remap(uint8_t o1, uint8_t o2);
void pic_mask(uint8_t irq, bool mask);
void pic_mask_all(bool mask);
#endif