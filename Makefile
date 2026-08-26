hello:
	gcc -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c src/main.c -o main.o
	ld -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o main.o -o main.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --output-target pei-x86-64 --subsystem=10 main.so main.efi
	dd if=/dev/zero of=uefi.img bs=512 count=93750
	parted uefi.img -s -a minimal mklabel gpt
	parted uefi.img -s -a minimal mkpart EFI FAT32 2048s 93716s
	parted uefi.img -s -a minimal toggle 1 boot
	dd if=/dev/zero of=/tmp/part.img bs=512 count=91669
	mformat -i /tmp/part.img -h 32 -t 32 -n 64 -c 1
	mmd -i /tmp/part.img ::/EFI
	mmd -i /tmp/part.img ::/EFI/BOOT
	mcopy -i /tmp/part.img main.efi ::/EFI/BOOT/BOOTX64.EFI
	dd if=/tmp/part.img of=uefi.img bs=512 count=91669 seek=2048 conv=notrunc

	qemu-system-x86_64 -cpu qemu64 \
	-drive if=pflash,format=raw,unit=0,file=ovmf-code-x86_64.fd,readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf-vars-x86_64.fd \
	-net none \
	-drive file=uefi.img,if=ide