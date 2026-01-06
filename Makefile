BUILD=build
OBJ=obj
SRC=src

STAGE2_ASM_SOURCES=$(shell find $(SRC)/bootloader/stage2 -name '*.asm')
STAGE2_ASM_OBJECTS=$(patsubst $(SRC)/bootloader/stage2/%.asm, $(OBJ)/bootloader/stage2/%.asm.o, $(STAGE2_ASM_SOURCES))

.PHONY: run
run: $(BUILD)/floppy.img
	qemu-system-x86_64 -fda $(BUILD)/floppy.img
	rm -rf $(BUILD) $(OBJ)

$(BUILD)/floppy.img: $(BUILD)/bootloader/stage1/boot.bin $(BUILD)/bootloader/stage2/stage2.bin
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
	nasm -f elf64 -o $@ $<

$(BUILD)/bootloader/stage2/stage2.bin: $(STAGE2_ASM_OBJECTS)
	mkdir -p $(BUILD)/bootloader/stage2
	toolchain/x86_64-elf/bin/x86_64-elf-ld -T"$(SRC)/bootloader/stage2/linker.ld" $(STAGE2_ASM_OBJECTS) -o $(BUILD)/bootloader/stage2/stage2.bin
