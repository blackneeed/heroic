#include <stddef.h>

size_t strlen(const char* str) {
    size_t len;
    for (len = 0; str[len]; len++) {}
    return len;
}