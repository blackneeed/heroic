BUILD=build
OBJ=obj
SRC=src

STAGE2_ASM_SOURCES=$(shell find $(SRC)/bootloader/stage2 -name '*.asm')
STAGE2_ASM_OBJECTS=$(patsubst $(SRC)/bootloader/stage2/%.asm, $(OBJ)/bootloader/stage2/%.asm.o, $(STAGE2_ASM_SOURCES))

STAGE2_C_SOURCES=$(shell find $(SRC)/bootloader/stage2 -name '*.c')
STAGE2_C_OBJECTS=$(patsubst $(SRC)/bootloader/stage2/%.c, $(OBJ)/bootloader/stage2/%.c.o, $(STAGE2_C_SOURCES))

.PHONY: run_floppy clean

run_floppy: $(BUILD)/floppy.img
	qemu-system-x86_64 -fda $(BUILD)/floppy.img --no-shutdown --no-reboot -m 64M

run_harddisk: $(BUILD)/harddisk.hda
	qemu-system-x86_64 -hda $(BUILD)/harddisk.hda --no-shutdown --no-reboot -m 64M

clean:
	rm -rf build obj

$(BUILD)/harddisk.hda: clean $(BUILD)/bootloader/stage1/boot_harddisk.bin $(BUILD)/bootloader/stage2/stage2.bin
	truncate -s $(shell echo "32 * 1024 * 1024" | bc) $(BUILD)/harddisk.hda
	parted -s $(BUILD)/harddisk.hda mklabel msdos
	parted -s $(BUILD)/harddisk.hda unit s mkpart primary 65s 100%
	parted -s $(BUILD)/harddisk.hda unit s print
	mkfs.fat -F 12 --offset $(shell echo "512 * 65" | bc) $(BUILD)/harddisk.hda
	dd if=$(BUILD)/bootloader/stage1/boot_harddisk.bin of=$(BUILD)/harddisk.hda bs=440 count=1 conv=notrunc
	dd if=$(BUILD)/bootloader/stage2/stage2.bin of=$(BUILD)/harddisk.hda bs=512 seek=1 conv=notrunc

$(BUILD)/floppy.img: clean $(BUILD)/bootloader/stage1/boot_floppy.bin $(BUILD)/bootloader/stage2/stage2.bin
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -R 65 $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader/stage1/boot_floppy.bin of=$(BUILD)/floppy.img bs=512 conv=notrunc
	dd if=$(BUILD)/bootloader/stage2/stage2.bin of=$(BUILD)/floppy.img bs=512 seek=1 conv=notrunc

$(BUILD)/bootloader/stage1/boot_harddisk.bin: $(SRC)/bootloader/stage1/boot_harddisk.asm
	mkdir -p $(BUILD)/bootloader/stage1
	nasm -f bin $(SRC)/bootloader/stage1/boot_harddisk.asm -o $(BUILD)/bootloader/stage1/boot_harddisk.bin

$(BUILD)/bootloader/stage1/boot_floppy.bin: $(SRC)/bootloader/stage1/boot_floppy.asm
	mkdir -p $(BUILD)/bootloader/stage1
	nasm -f bin $(SRC)/bootloader/stage1/boot_floppy.asm -o $(BUILD)/bootloader/stage1/boot_floppy.bin

$(OBJ)/bootloader/stage2/%.asm.o: $(SRC)/bootloader/stage2/%.asm
	mkdir -p $(shell dirname '$@')
	nasm -f elf64 -o "$@" "$<"

$(OBJ)/bootloader/stage2/%.c.o: $(SRC)/bootloader/stage2/%.c
	mkdir -p $(shell dirname '$@')
	gcc -I "$(SRC)/bootloader/stage2/inc" -lgcc -Os -ffunction-sections -fdata-sections -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -mno-avx -mno-80387 -m64 -mno-red-zone -Wall -Wextra -Wpedantic -Werror -c "$<" -o "$@"

$(BUILD)/bootloader/stage2/stage2.elf: $(STAGE2_ASM_OBJECTS) $(STAGE2_C_OBJECTS)
	mkdir -p $(BUILD)/bootloader/stage2
	ld -T"$(SRC)/bootloader/stage2/linker.ld" --gc-sections $(STAGE2_ASM_OBJECTS) $(STAGE2_C_OBJECTS) -o $(BUILD)/bootloader/stage2/stage2.elf -m elf_x86_64

$(BUILD)/bootloader/stage2/stage2.bin: $(BUILD)/bootloader/stage2/stage2.elf
	objcopy -O binary $(BUILD)/bootloader/stage2/stage2.elf $(BUILD)/bootloader/stage2/stage2.bin
