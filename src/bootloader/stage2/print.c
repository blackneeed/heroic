#include <vga.h>
#include <print.h>

void dprint(char* str) {
    #ifdef DEBUG_PRINT
    print(str);
    #endif
}

void print(char* str) {
    vga_text_mode_print_string(str);
}