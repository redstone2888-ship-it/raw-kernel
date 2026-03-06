CC = i686-elf-gcc
LD = i686-elf-ld

CFLAGS = -m32 -ffreestanding -nostdlib -fno-builtin -Wall -Wextra -Iinclude
LDFLAGS = -T linker.ld -nostdlib

KERNEL = kernel.bin
ISO = myos.iso

# 🔥 автоматически находим ВСЕ .c
SRC := $(shell find kernel drivers fs lib -name "*.c")
OBJ := $(SRC:.c=.o)

all: build

# ===== Сборка =====
build: $(KERNEL) iso

$(KERNEL): $(OBJ) linker.ld
	$(LD) -m elf_i386 $(LDFLAGS) $(OBJ) -o $(KERNEL)

# ===== правило компиляции любого .c → .o =====
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ===== ISO =====
iso: $(KERNEL)
	rm -rf iso
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/

	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "RawOS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '  boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg

	i686-elf-grub-mkrescue -o $(ISO) iso

# ===== Запуск =====
run: build
	qemu-system-i386 -cdrom $(ISO) -m 256M -serial stdio

# ===== Очистка =====
clean:
	rm -rf $(OBJ) $(KERNEL) $(ISO) iso

re: clean build