bits 64

section .text

global kernel_start

extern kstart
kernel_start:
    mov rsp, stack
    mov rbp, rsp

    cli
    cld
    clc

    jmp kstart

section .bss
resb 512
stack: