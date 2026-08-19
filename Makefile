BUILD=build
OBJ=obj
SRC=src

STAGE2_ASM_SOURCES=$(shell find $(SRC)/bootloader/stage2 -name '*.asm')
STAGE2_ASM_OBJECTS=$(patsubst $(SRC)/bootloader/stage2/%.asm, $(OBJ)/bootloader/stage2/%.asm.o, $(STAGE2_ASM_SOURCES))

STAGE2_C_SOURCES=$(shell find $(SRC)/bootloader/stage2 -name '*.c')
STAGE2_C_OBJECTS=$(patsubst $(SRC)/bootloader/stage2/%.c, $(OBJ)/bootloader/stage2/%.c.o, $(STAGE2_C_SOURCES))

.PHONY: run clean

run: $(BUILD)/floppy.img
	qemu-system-x86_64 -fda $(BUILD)/floppy.img --no-shutdown --no-reboot

run_debug: $(BUILD)/floppy.img
	qemu-system-x86_64 -fda $(BUILD)/floppy.img --no-shutdown --no-reboot -d cpu_reset,guest_errors,int -monitor stdio

clean:
	rm -rf build obj floppy.img

$(BUILD)/floppy.img: clean $(BUILD)/bootloader/stage1/boot.bin $(BUILD)/bootloader/stage2/stage2.bin
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader/stage1/boot.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/bootloader/stage2/stage2.bin "::stage2.bin"
	cp $(BUILD)/floppy.img floppy.img

$(BUILD)/bootloader/stage1/boot.bin: $(SRC)/bootloader/stage1/boot.asm
	mkdir -p $(BUILD)/bootloader/stage1
	nasm -f bin $(SRC)/bootloader/stage1/boot.asm -o $(BUILD)/bootloader/stage1/boot.bin

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
