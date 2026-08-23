#include <vga.h>
#include <stdint.h>
#include <stdbool.h>
#include <portio.h>

static int __vga_text_mode_current_x = 0;
static int __vga_text_mode_current_y = 0;
static bool __vga_more_chars = false;

static inline uint16_t __vga_calc_pos(uint8_t x, uint8_t y) {
    return y * 80 + x;
}

void vga_text_mode_set_cursor_pos(uint8_t x, uint8_t y) {
    // 0x3D4 - crtcaddr
    // 0x3D5 - crtcdata
    // 0xF - bits 0-7
    // 0xE - bits 8-15

    outb(0x3D4, 0xF);
    outb(0x3D5, __vga_calc_pos(x, y));
    outb(0x3D4, 0xE);
    outb(0x3D5, __vga_calc_pos(x, y) >> 8);
}

static inline void vga_text_mode_put(char character, uint8_t x, uint8_t y) {
    *(char*)(0xb8000 + (uint64_t)__vga_calc_pos(x, y) * 2) = character;
    *(uint8_t*)(0xb8000 + (uint64_t)__vga_calc_pos(x, y) * 2 + 1) = 0x0F;
}

void vga_text_mode_print(char character) {
    switch (character) {
        case '\b': if (__vga_text_mode_current_x) __vga_text_mode_current_x -= 1; break;
        case '\n':
            __vga_text_mode_current_y++;
            if (__vga_text_mode_current_y >= 25) {
                vga_text_mode_clear();
                __vga_text_mode_current_y = 0;
           }
            break;
        case '\r': __vga_text_mode_current_x = 0; break;
        default:
            vga_text_mode_put(character, __vga_text_mode_current_x, __vga_text_mode_current_y);
            __vga_text_mode_current_x++;
            if (__vga_text_mode_current_x >= 80) {
                __vga_text_mode_current_x = 0;
                __vga_text_mode_current_y++;
                if (__vga_text_mode_current_y >= 25) {
                    vga_text_mode_clear();
                    __vga_text_mode_current_y = 0;
                }
            }
    }


    if (!__vga_more_chars) {
        vga_text_mode_set_cursor_pos(__vga_text_mode_current_x, __vga_text_mode_current_y);
    }
}

void vga_text_mode_print_string(char* str) {
    __vga_more_chars = true;
    for (char *c = str; *c != 0; c++) {
        vga_text_mode_print(*c);
    }

    vga_text_mode_set_cursor_pos(__vga_text_mode_current_x, __vga_text_mode_current_y);
    __vga_more_chars = false;
}

void vga_text_mode_clear() {
    vga_text_mode_set_cursor_pos(0, 0);
    for (uint8_t x = 0; x < 80; x++) {
        for (uint8_t y = 0; y < 25; y++) {
            vga_text_mode_put(' ', x, y);
        }
    }
    __vga_text_mode_current_x = 0;
    __vga_text_mode_current_y = 0;
}