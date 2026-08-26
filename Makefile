OBJ=obj
OUT=out
SRC=src
CC=gcc
LD=ld
OBJCOPY=objcopy
QEMU=qemu-system-x86_64
NAME=heroic

OVMF_CODE=ovmf/ovmf-code-x86_64.fd
OVMF_VARS=ovmf/ovmf-vars-x86_64.fd

C_SRC=$(shell find $(SRC) -name '*.c')
C_OBJ=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(C_SRC))

.PHONY: run_harddisk clean

run_harddisk: $(OUT)/$(NAME).img
	qemu-system-x86_64 -cpu qemu64 \
	-drive if=pflash,format=raw,unit=0,file=$(OVMF_CODE),readonly=on \
	-drive if=pflash,format=raw,unit=1,file=$(OVMF_VARS) \
	-net none \
	-drive file=$(OUT)/$(NAME).img \
	-m 512M

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(shell dirname '$@')
	gcc -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c "$<" -o "$@"

$(OUT)/EFI_APP.so: $(C_OBJ)
	mkdir -p $(shell dirname '$@')
	ld -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $^ -o "$@" -lgnuefi -lefi

$(OUT)/EFI_APP.efi: $(OUT)/EFI_APP.so
	mkdir -p $(shell dirname '$@')
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target pei-x86-64 --subsystem=10 "$<" "$@"

$(OUT)/$(NAME).img: $(OUT)/EFI_APP.efi
	mkdir -p $(shell dirname '$@')
	
	dd if=/dev/zero of="$@" bs=512 count=93750
	parted "$@" -s -a minimal mklabel gpt
	parted "$@" -s -a minimal mkpart EFI FAT32 2048s 93716s
	parted "$@" -s -a minimal toggle 1 boot
	dd if=/dev/zero of=$(OUT)/$(NAME)_partition.img bs=512 count=91669
	mformat -i $(OUT)/$(NAME)_partition.img -h 32 -t 32 -n 64 -c 1
	mmd -i $(OUT)/$(NAME)_partition.img ::/EFI
	mmd -i $(OUT)/$(NAME)_partition.img ::/EFI/BOOT
	mcopy -i $(OUT)/$(NAME)_partition.img "$<" ::/EFI/BOOT/BOOTX64.EFI
	dd if=$(OUT)/$(NAME)_partition.img of="$@" bs=512 count=91669 seek=2048 conv=notrunc

clean:
	rm -rf $(OBJ) $(OUT)