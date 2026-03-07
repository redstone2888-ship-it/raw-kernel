AS = nasm
ASFLAGS = -f elf32
CC = i686-elf-gcc
LD = i686-elf-ld

CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -nostdlib

KERNEL = kernel.bin
ISO = myos.iso

SRC_C := $(shell find kernel drivers fs lib -name "*.c")
OBJ_C := $(SRC_C:.c=.o)
BOOT_OBJ = boot/boot.o

OBJ = $(BOOT_OBJ) $(OBJ_C)

all: $(ISO)

$(KERNEL): $(OBJ) linker.ld
	$(LD) -m elf_i386 $(LDFLAGS) $(OBJ) -o $(KERNEL)

boot/boot.o: boot/boot.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

iso: $(KERNEL) disk.img
	rm -rf iso
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/
	cp disk.img iso/boot/

	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "RawOS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '  module /boot/disk.img' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg

	i686-elf-grub-mkrescue -o $(ISO) iso

$(ISO): iso

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 256M -serial stdio

clean:
	rm -rf $(OBJ) $(KERNEL) $(ISO) iso

re: clean all