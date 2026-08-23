[bits 64]

extern gdt64
extern gdt32
extern stack
extern drive_number
extern idtr

section .data

jump_target:
    dd 0
    dw 0x08

idt_rmode:
    dw 0x03FF
    dq 0x00000000

saved_rsp: dq 0

global debug_1
global debug_2
global debug_3
debug_1:    times 26 db 0
debug_2:    times 26 db 0
debug_3:    times 26 db 0

section .text

; inspiration from nanobyte-os
%macro enter_real_mode 0
    [bits 64]
    mov qword [saved_rsp], rsp
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

    mov sp, stack    
    lidt [idt_rmode]
    
%endmacro

%macro enter_long_mode 0
    [bits 16]

    cli

    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; TODO: implement better A20 enabling.
    in al, 0x92
    or al, 2
    out 0x92, al

    jmp 0x08:.pmode

.pmode:
    [bits 32]
    
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    mov eax, cr3
    mov eax, 0x1000
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

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
    lidt [idtr]
    sti
%endmacro

global bios_get_disk_geometry
bios_get_disk_geometry:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    enter_real_mode
    mov ah, 8
    mov dl, byte [di]
    int 0x13

    jc .exit

    mov ah, 0
    mov dl, byte [di]
    int 0x13

    add dh, 1
    mov [di + 3], dh

    and cl, 0x3f
    mov [di + 2], cl
    .exit:
    enter_long_mode
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret

global bios_check_edd_presence
bios_check_edd_presence:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    enter_real_mode
    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, byte [di]
    clc
    int 0x13

    jc .no_edd
    cmp bx, 0xaa55
    jne .no_edd
    mov [di + 1], 1

    mov ah, 0
    mov dl, byte [di]
    int 0x13

    jmp .exit
    .no_edd:
    mov [di + 1], 0
    .exit:
    enter_long_mode
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret

global mode_switch_test
mode_switch_test:
    enter_real_mode
    mov [debug_1 +  0], ax
    mov [debug_1 +  2], bx
    mov [debug_1 +  4], cx
    mov [debug_1 +  6], dx
    mov [debug_1 +  8], si
    mov [debug_1 + 10], di
    mov [debug_1 + 12], bp
    mov [debug_1 + 14], sp
    mov [debug_1 + 16], ss
    mov [debug_1 + 18], ds
    mov [debug_1 + 20], es
    mov [debug_1 + 22], cs

    push ax
    pushf
    pop ax
    mov [debug_1 + 24], ax
    pop ax

    mov ah, 8
    mov dl, 0x80
    int 0x13

    mov ah, 0
    mov dl, 0x80
    int 0x13

    mov [debug_2 +  0], ax
    mov [debug_2 +  2], bx
    mov [debug_2 +  4], cx
    mov [debug_2 +  6], dx
    mov [debug_2 +  8], si
    mov [debug_2 + 10], di
    mov [debug_2 + 12], bp
    mov [debug_2 + 14], sp
    mov [debug_2 + 16], ss
    mov [debug_2 + 18], ds
    mov [debug_2 + 20], es
    mov [debug_2 + 22], cs
    push ax
    pushf
    pop ax
    mov [debug_2 + 24], ax
    pop ax

    enter_long_mode
    jmp mode_switch_test2

mode_switch_test2:
    enter_real_mode
    mov [debug_3 +  0], ax
    mov [debug_3 +  2], bx
    mov [debug_3 +  4], cx
    mov [debug_3 +  6], dx
    mov [debug_3 +  8], si
    mov [debug_3 + 10], di
    mov [debug_3 + 12], bp
    mov [debug_3 + 14], sp
    mov [debug_3 + 16], ss
    mov [debug_3 + 18], ds
    mov [debug_3 + 20], es
    mov [debug_3 + 22], cs

    mov ah, 8
    mov dl, 0x80
    int 0x13

    mov ah, 0
    mov dl, 0x80
    int 0x13

    push ax
    pushf
    pop ax
    mov [debug_3 + 24], ax
    pop ax

    enter_long_mode
    ret