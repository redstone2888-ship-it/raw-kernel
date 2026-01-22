#!/bin/bash
cd "$(dirname "$0")"  # Переходим в директорию скрипта

echo "=== 1. Компиляция ==="
# Проверяем что файлы существуют
ls -la kernel.c linker.ld

# Компиляция
gcc -m32 -ffreestanding -nostdlib -fno-builtin -c kernel.c -o kernel.o

echo "=== 2. Линковка ==="
# Линковка
ld -m elf_i386 -T linker.ld -nostdlib kernel.o -o kernel.bin  # <-- ТОЧКА, а не подчёркивание!

echo "=== 3. Проверка kernel.bin ==="
file kernel.bin
readelf -h kernel.bin

echo "=== 4. Подготовка ISO ==="
# Очищаем и создаём структуру
rm -rf iso
mkdir -p iso/boot/grub

# Копируем kernel.bin (с точкой!)
cp kernel.bin iso/boot/

# Создаём grub.cfg
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=0
set default=0

menuentry "RawOSS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

echo "=== 5. Проверяем что в папке iso ==="
tree iso/
ls -la iso/boot/

echo "=== 6. Создаём ISO ==="
grub-mkrescue -o myos.iso iso

echo "=== 7. Проверяем ISO ==="
xorriso -indev myos.iso -ls 2>/dev/null

echo "=== 8. Запуск ==="
qemu-system-i386 -cdrom myos.iso -m 256M -serial stdio#!/bin/bash
cd "$(dirname "$0")"