[org 0x7c00]

jmp short main
db 0
bpb:
.oem_name: db "MSWIN4.1"
.bytes_per_sector: dw 512
.sectors_per_cluster: db 1
.reserved_sectors_count: dw 65
.fat_count: db 2
.root_dir_entry_count: dw 0xE0
.total_sectors: dw 2880
.media: db 0xF0
.sectors_per_fat: dw 9
.sectors_per_track: dw 18
.heads: dw 2
.hidden_sectors: dd 0
.total_sectors_32: dd 0

; ebpb for fat12&16
.drive_number: db 0
db 0
.boot_signature: db 0x29
.volume_id: dd 0
.volume_label: db "HEROIC     "
.file_system_type: db "FAT12   "

main:
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax

    mov sp, 0x7c00
    mov [bpb.drive_number], dl

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
    div word [bpb.sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector

    xor dx, dx                          ; dx = 0
    div word [bpb.heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
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
    mov dl, [bpb.drive_number]
    pusha
    int 0x13
    popa
    jc error
    ret

error:
    mov si, error_string
    call print_string
    jmp $

error_string: db "stage1 fail!", 0x0D, 0x0A, 0
stage2_name: db "STAGE2  BIN"

times 510-($-$$) db 0
db 0x55, 0xaa
