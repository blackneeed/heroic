.PHONY: compile clean

compile:
	gcc -ffreestanding -mno-red-zone -fshort-wchar -c main.c -o main.o
	nasm -f elf64 main.asm -o main.asm.o
	ld -T linker.ld -static -nostdlib -o main.elf main.o main.asm.o

clean:
	rm -f main.o main.elf main.asm.o