[bits 16]

section .stage2_main

global stage2_main
stage2_main:
    ; switch to pm32

    xor ax, ax
    mov es, ax
    mov di, 0x7E00
    mov cx, 256 ; words = 512 bytes
    rep stosw

    ; 0x7e00-0x8000 is now a usable buffer for e820
    ; es is already zero
    mov di, 0x7e00
    xor ebx, ebx
    mov edx, 0x534D4150
    mov eax, 0x0000E820
    mov ecx, 24
    int 0x15

    .loop:
    cmp ebx, 0
    je .memory_map_end ; if either ebx is zero or the carry is set the memory map is finished
    jc .memory_map_end

    xor ch, ch
    add di, cx

    cmp cl, 24 ; if cl is already 24 we dont need to add 4
    je .loop2

    mov dword [di], 1

    add di, 4
    add cl, 4
    .loop2:
    add di, cx
    mov eax, 0x0000E820
    int 0x15

    .memory_map_end:

    cli

    ; TODO: implement better A20 enabling.
    in al, 0x92
    or al, 2
    out 0x92, al

    lgdt [gdt32]

    mov eax, cr0 
    or al, 1
    mov cr0, eax
    ; set pe bit

    jmp 0x08:protected_mode

section .data

gdt32_entries:
    dq 0 ; null desc
    dq 0x00CF9A000000FFFF ; code desc
    dq 0x00CF92000000FFFF ; data desc
gdt32:
    dw $ - gdt32_entries - 1
    dd gdt32_entries

[bits 32]

section .text

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pushfd                               ;Save EFLAGS
    pushfd                               ;Store EFLAGS
    xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                ;Load stored EFLAGS (with ID bit inverted)
    pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax,[esp]                        ;eax = whichever bits were changed
    popfd                                ;Restore original EFLAGS
    and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
    jz .NoLongMode

    mov eax, 0x80000000    ; Set the A-register to 0x80000000.
    cpuid                  ; CPU identification.
    cmp eax, 0x80000001    ; Compare the A-register with 0x80000001.
    jb .NoLongMode         ; It is less, there is no long mode.

    mov eax, 0x80000001    ; Set the A-register to 0x80000001.
    cpuid                  ; CPU identification.
    test edx, 1 << 29      ; Test if the LM-bit, which is bit 29, is set in the D-register.
    jz .NoLongMode         ; They aren't, there is no long mode.

    mov edi, 0x1000    ; Set the destination index to 0x1000.
    mov cr3, edi       ; Set control register 3 to the destination index.
    xor eax, eax       ; Nullify the A-register.
    mov ecx, 4096      ; Set the C-register to 4096.
    rep stosd          ; Clear the memory.
    mov edi, cr3       ; Set the destination index to control register 3.

    mov DWORD [edi], 0x2003      ; Set the uint32_t at the destination index to 0x2003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x3003      ; Set the uint32_t at the destination index to 0x3003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.
    mov DWORD [edi], 0x4003      ; Set the uint32_t at the destination index to 0x4003.
    add edi, 0x1000              ; Add 0x1000 to the destination index.

    mov ebx, 0x00000003          ; Set the B-register to 0x00000003.
    mov ecx, 512                 ; Set the C-register to 512.
    
.SetEntry:
    mov DWORD [edi], ebx         ; Set the uint32_t at the destination index to the B-register.
    add ebx, 0x1000              ; Add 0x1000 to the B-register.
    add edi, 8                   ; Add eight to the destination index.
    loop .SetEntry               ; Set the next entry.

    mov eax, cr4                 ; Set the A-register to control register 4.
    or eax, 1 << 5               ; Set the PAE-bit, which is the 6th bit (bit 5).
    mov cr4, eax                 ; Set control register 4 to the A-register.

    mov ecx, 0xC0000080          ; Set the C-register to 0xC0000080, which is the EFER MSR.
    rdmsr                        ; Read from the model-specific register.
    or eax, 1 << 8               ; Set the LM-bit which is the 9th bit (bit 8).
    wrmsr                        ; Write to the model-specific register.
    
    mov eax, cr0                 ; Set the A-register to control register 0.
    or eax, 1 << 31              ; Set the PG-bit, which is the 32nd bit (bit 31).
    mov cr0, eax                 ; Set control register 0 to the A-register.

    lgdt [gdt64]
    jmp 0x08:long_mode
.NoLongMode:
    mov byte [0xb8000], 'n'
    mov byte [0xb8002], 'o'
    mov byte [0xb8004], ' '
    mov byte [0xb8006], 'l'
    mov byte [0xb8008], 'o'
    mov byte [0xb800a], 'n'
    mov byte [0xb800c], 'g'
    mov byte [0xb800e], ' '
    mov byte [0xb8010], 'm'
    mov byte [0xb8012], 'o'
    mov byte [0xb8014], 'd'
    mov byte [0xb8016], 'e'
    mov byte [0xb8018], ' '
    mov byte [0xb801a], 's'
    mov byte [0xb801c], 'u'
    mov byte [0xb801e], 'p'
    mov byte [0xb8020], 'p'
    mov byte [0xb8022], 'o'
    mov byte [0xb8024], 'r'
    mov byte [0xb8026], 't'
    mov byte [0xb8028], '!'
    jmp $

gdt64_entries:
    dq 0 ; null desc
    dq 0x00AF9A000000FFFF ; code desc
    dq 0x00AF92000000FFFF ; data desc
gdt64:
    dw $ - gdt64_entries - 1
    dd gdt64_entries
    
[bits 64]

extern stage2_cmain

long_mode:
    jmp stage2_cmain