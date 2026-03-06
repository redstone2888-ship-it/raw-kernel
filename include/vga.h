#pragma once

// ==== VGA text mode constants ====
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// ==== Cursor/Color ====
extern int cursor_x;
extern int cursor_y;
extern unsigned char color; // White on black by default

// ==== VGA functions ====
void clear_screen(void);
void move_cursor(void);
void scroll(void);
void putchar(char c);
void print_text(const char* str);
void itoa(int n, char* buf);
void printf_v(const char* fmt, int value);
void printf(const char* fmt, ...);
void println(const char* fmt, ...);

// ==== Low-level port I/O (needed by move_cursor) ====
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}