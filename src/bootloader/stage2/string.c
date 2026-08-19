#include <stddef.h>

size_t strlen(const char* str) {
    size_t len;
    for (len = 0; str[len]; len++) {}
    return len;
}

int memcmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) if (s1[i] != s2[i]) return 1;
    return 0;
}