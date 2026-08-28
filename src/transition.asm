bits 64

section .text

global transition_start
global transition_end

transition_start:
mov cr3, rax

mov rdi, rbx
jmp rcx
cli
hlt
transition_end: