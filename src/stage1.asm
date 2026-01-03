[org 0x7c00]

main:
    xor ax, ax
    mov ds, ax
    mov ss, ax

    mov sp, 0x7c00

    mov si, hello_string
    call print_string

    jmp $

print_string:
    mov ah, 0x0e
    .loop:
    lodsb
    test al, al
    jz .end
    int 0x10
    jmp .loop
    .end:
    ret

hello_string: db "Hello World!", 0x0A, 0x0D, 0

times 510-($-$$) db 0
db 0x55, 0xaa