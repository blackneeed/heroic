void kernel_start() {
    __asm__ volatile ("outb %0, $0xe9" : : "a"((unsigned char)'h'));
    asm volatile ("cli");
    for (;;) asm volatile ("hlt");
}