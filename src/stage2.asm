[org 0x7e00]

times 2048-($-$$) db 0

jmp stub

stub:
    mov si, hello
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

hello: db "hallo stage2 make your komputer no go kaput!", 0x0D, 0x0A, 0