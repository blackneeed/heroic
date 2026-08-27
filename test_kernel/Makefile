.PHONY: compile clean

compile:
	gcc -ffreestanding -mno-red-zone -fshort-wchar -c main.c -o main.o
	ld -T linker.ld -static -Bsymbolic -nostdlib -o main.elf main.o

clean:
	rm -f main.o main.elf