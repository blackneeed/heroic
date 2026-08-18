#ifndef PORTIO_H
#define PORTIO_H
#include <stdint.h>

void outb(uint16_t port, uint8_t value);
void inb(uint16_t port);
#endif