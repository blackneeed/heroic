#include <vga.h>
#include <nanoprintf.h>
#include <stdio.h>
#include <stdarg.h>

static inline void __vga_putc_npf_wrapper(int c, void* ctx) {
    (void)ctx;
    vga_text_mode_print((char)c);
}

int dprintf(const char* restrict format, ...) {
    #ifdef DEBUG_PRINT
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(__vga_putc_npf_wrapper, NULL, format, args);
    va_end(args);
    return ret;
    #else
    return 0;
    #endif
}

int printf(const char* restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = npf_vpprintf(__vga_putc_npf_wrapper, NULL, format, args);
    va_end(args);
    return ret;
}