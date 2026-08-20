[org 0x7c00]

jmp short main
db 0

drive_number: db 0
sectors_per_track: dw 0
heads: dw 0

edd_present: db 0

main:
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax

    mov sp, 0x7c00
    mov [drive_number], dl

    mov ah, 0x41
    mov bx, 0x55aa
    mov dl, [drive_number]
    clc
    int 0x13

    jc .no_edd
    cmp bx, 0xaa55
    jne .no_edd

    mov [edd_present], 1
    jmp .read_file_edd

    .no_edd:
    ; fill the drive geometry in
    mov ah, 8
    mov dl, [drive_number]
    int 0x13
    jc error

    add dh, 1
    mov [heads], dh

    and cl, 0x3f
    mov [sectors_per_track], cl

    .read_file:
    mov bx, 0x8000 ; IMPORTANT: This means a maximum 32 KiB file can be loaded by this!
    mov ax, 1
    .read_file_loop:
    cmp ax, 65
    je .file_read
    mov cl, 1

    push ax
    call read_sectors
    pop ax

    add ax, 1
    add bx, 512
    jmp .read_file_loop
    .read_file_edd:
    clc
    mov si, [dap_packet]
    mov ah, 0x42
    mov dl, [drive_number]
    int 0x13
    jc .no_edd
    .file_read:
    jmp 0:0x8000

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

; Taken from nanobyte-os project
lba_to_chs:

    push ax
    push dx

    xor dx, dx                          ; dx = 0
    div word [sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector

    xor dx, dx                          ; dx = 0
    div word [heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                        ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                          ; dh = head
    mov ch, al                          ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                           ; put upper 2 bits of cylinder in CL

    pop ax
    mov dl, al                          ; restore DL
    pop ax
    ret

; lba: ax
; count: cl
; buffer: es:bx
; destroys dx, ax, cx
read_sectors:
    clc
    push cx
    call lba_to_chs

    pop ax
    mov ah, 0x02
    mov dl, [drive_number]
    pusha
    int 0x13
    jc error
    popa
    ret

error:
    mov si, error_string
    call print_string
    jmp $

error_string: db "stage1 fail!", 0x0D, 0x0A, 0
stage2_name: db "STAGE2  BIN"

dap_packet:
    db 0x10
    db 0
    dw 64 ; sector count
    dw 0x8000 ; offset
    dw 0 ; segment
    dq 1 ; lba

times 440-($-$$) db 0

db 0x55, 0xaa
