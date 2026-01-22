cat > README.md << 'EOF'

Простая 32-битная ОС, использующая загрузчик GRUB м написанная на C для обучения.

- Загрузка через GRUB/Multiboot
- Вывод текста в VGA режиме (80x25)
- Базовые функции: clear_screen, print_text, delay
- Поддержка цветов текста

```bash
make