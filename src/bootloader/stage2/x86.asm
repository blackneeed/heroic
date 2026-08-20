[bits 64]

extern gdt64
extern gdt32
extern stack

section .data

jump_target:
    dd 0
    dw 0x08

idt_rmode:
    dw 0x03FF
    dd 0x00000000

saved_rsp: dq 0
saved_idtr: dq 0

section .text

; inspiration from nanobyte-os
%macro enter_real_mode 0
    [bits 64]
    mov [saved_rsp], rsp
    sidt [saved_idtr]
    cli

    lgdt [gdt32]

    ; disable long mode
    mov ecx, 0xC0000080
    rdmsr
    and eax, ~0x100
    wrmsr

    ; disable paging
    mov rax, cr0
    and rax, ~0x80000000
    mov cr0, rax

    mov dword [jump_target], .pmode32

    jmp far [jump_target]
    
.pmode32:
    [bits 32]

    jmp 0x18:.pmode16
    
.pmode16:
    [bits 16]
    
    ; disable protected mode
    mov eax, cr0
    and al, ~1
    mov cr0, eax
    
    ; far jump to real mode
    jmp 0x00:.rmode
    
.rmode:
    [bits 16]

    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov sp, 0x7C00
    
    lidt [idt_rmode]
    sti
    
%endmacro

%macro enter_long_mode 0

    [bits 16]

    cli

    mov eax, cr0
    or al, 1
    mov cr0, eax

    jmp 0x08:.pmode

.pmode:
    [bits 32]
    
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, cr3
    mov cr3, eax

    lgdt [gdt64]

    jmp 0x08:.lmode

.lmode:
    [bits 64]
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, [saved_rsp]
    lidt [saved_idtr]
    sti
%endmacro