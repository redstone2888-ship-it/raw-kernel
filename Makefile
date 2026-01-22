all:
	gcc -m32 -ffreestanding -c kernel.c -o kernel.o
	ld -m elf_i386 -T linker.ld kernel.o -o kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/
	cp -r boot/grub iso/boot/
	grub-mkrescue -o myos.iso iso
	qemu-system-i386 -cdrom myos.iso
