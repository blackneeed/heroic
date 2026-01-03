BUILD=build
SRC=src

.PHONY: run
run: $(BUILD)/stage1.bin
	qemu-system-x86_64 $(BUILD)/stage1.bin

$(BUILD)/stage1.bin: $(SRC)/stage1.asm
	mkdir -p $(BUILD)
	nasm -f bin $(SRC)/stage1.asm -o $(BUILD)/stage1.bin