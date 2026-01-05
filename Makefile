BUILD=build
SRC=src

.PHONY: run
run: $(BUILD)/floppy.img
	qemu-system-x86_64 -fda $(BUILD)/floppy.img
	rm -rf $(BUILD)

$(BUILD)/floppy.img: $(BUILD)/stage1.bin $(BUILD)/stage2.bin
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 $(BUILD)/floppy.img
	dd if=$(BUILD)/stage1.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/stage2.bin "::stage2.bin"
	cp $(BUILD)/floppy.img floppy.img

$(BUILD)/stage1.bin: $(SRC)/bootloader/stage1/boot.asm
	mkdir -p $(BUILD)
	nasm -f bin $(SRC)/bootloader/stage1/boot.asm -o $(BUILD)/stage1.bin

$(BUILD)/stage2.bin: $(SRC)/bootloader/stage2/stage2_main.asm
	mkdir -p $(BUILD)
	nasm -f bin $(SRC)/bootloader/stage2/stage2_main.asm -o $(BUILD)/stage2.bin
