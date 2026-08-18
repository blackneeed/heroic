#ifndef VGA_H
#define VGA_H
#include <stdint.h>

void vga_text_mode_set_cursor_pos(uint8_t x, uint8_t y);
void vga_text_mode_print(char character);
void vga_text_mode_print_string(char* str);
void vga_text_mode_clear();

#endif