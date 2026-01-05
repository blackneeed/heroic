[org 0x7c00]

jmp short main
db 0
bpb:
.oem_name: db "MSWIN4.1"
.bytes_per_sector: dw 512
.sectors_per_cluster: db 1
.reserved_sectors_count: dw 1
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

ROOT_DIR_LBA:
    dw 0

DATA_LBA: dw 0

; dbg
print_uint:
    mov bx, 10          ; divisor
    xor cx, cx          ; digit count = 0

.convert:
    xor dx, dx
    div bx              ; AX = AX / 10, DX = remainder
    push dx             ; save digit
    inc cx
    test ax, ax
    jnz .convert

.print:
    pop dx
    add dl, '0'
    mov ah, 0x0E
    mov al, dl
    int 0x10
    loop .print

    ret
; end dbg

main:
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax

    mov sp, 0x7c00
    mov [bpb.drive_number], dl

    mov al, [bpb.fat_count] ; ah is 0 (first line of main)
    mov bx, [bpb.sectors_per_fat]
    mul bx

    add ax, [bpb.reserved_sectors_count]
    mov [ROOT_DIR_LBA], ax

    mov ax, [bpb.root_dir_entry_count]
    shl ax, 5 ; NOTE: assumes root dir entry count ≤ 2047

    div word [bpb.bytes_per_sector]

    cmp dx, 0
    je .read_root_dir
    add ax, 1

    .read_root_dir:
    mov bx, [ROOT_DIR_LBA]
    add bx, ax
    mov [DATA_LBA], bx

    ; NOTE: we're loading the full root dir at 0x1000 which is ok for 224 root dir entries but not really for 2047
    mov cl, al
    mov ax, [ROOT_DIR_LBA]
    ; es is 0 (first lines of main)
    mov bx, 0x1000
    call read_sectors

    mov di, 0x1000
    .find_file_loop:
    mov si, stage2_name
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je .find_file_match

    add di, 32
    inc bx
    cmp bx, [bpb.root_dir_entry_count]
    jl .find_file_loop

    jmp error
    .find_file_match:
    mov ax, [di + 26]
    push ax
    
    mov cl, [bpb.sectors_per_fat]
    mov ax, [bpb.reserved_sectors_count]
    ; es is 0 (first lines of main)
    mov bx, 0x1000
    call read_sectors

    pop cx

    mov bx, 0x7e00 ; loading it there!
    .read_file_loop:
    mov ax, cx
    sub ax, 2

    push dx
    mul byte [bpb.sectors_per_cluster]
    pop dx

    add ax, [DATA_LBA]

    push cx
    mov cl, byte [bpb.sectors_per_cluster]
    ; es & bx are set (es is zeroed in first lines of main)
    call read_sectors
    pop cx

    mov ax, [bpb.bytes_per_sector]
    mul word [bpb.sectors_per_cluster]
    ; again discarding dx so clusters cant have more than 64k (if im not wrong)
    mov dx, ax ; dx = bytes per cluster

    add bx, dx

    mov ax, cx
    mov dx, 3
    mul dx
    ; r16*r16->r32

    push bx
    mov bx, 2
    div bx
    pop bx

    ; r32/r16->r16

    push ax ; index in fat on stack

    push bx
    mov ax, cx
    mov bx, 2
    div bx
    pop bx

    cmp dx, 0
    jnz .read_file_odd_cluster_num
    .read_file_even_cluster_num:
    pop cx
    push bx
    mov bx, cx
    add bx, 0x1000
    mov cx, word [bx]  
    and cx, 0xFFF
    pop bx
    jmp .read_file_after_recalc_cluster_num
    .read_file_odd_cluster_num:
    pop cx
    push bx
    mov bx, cx
    add bx, 0x1000
    mov cx, word [bx]  
    shr cx, 4
    pop bx
    .read_file_after_recalc_cluster_num:
    cmp cx, 0xFF8
    jae .file_read
    jmp .read_file_loop
    .file_read:

    jmp 0x7e00

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