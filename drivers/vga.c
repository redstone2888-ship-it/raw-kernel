/*
 * vga.c - The VGA text mode driver
 * 
 * Copyright (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 * 
 * Handles screen output in text mode:
 *  - Printing characters and strings
 *  - Cursor movement
 *  - Screen scrolling
 *  - Clearing the screen
 * 
 * Not recommended to delete - the kernel not be able to dispay text/chars 
 * to screen.
 */

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Current cursor position
int cursor_x = 0;
int cursor_y = 0;
unsigned char color = 0x0F; // White on black

#include <stdarg.h>

// putchar prototype
void putchar(char c); 

void print_text(const char* str) {
    while (*str) {
        putchar(*str);
        str++;
    }
}

// itoa for nums
void itoa(int n, char* buf) {
    int i = 0;
    int is_negative = 0;
    if (n == 0) { buf[i++] = '0'; buf[i] = 0; return; }
    if (n < 0) { is_negative = 1; n = -n; }
    char temp[10]; int j = 0;
    while (n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
    if (is_negative) buf[i++] = '-';
    while (j > 0) buf[i++] = temp[--j];
    buf[i] = 0;
}

// Experimental function
void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[32];

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 'd') {
                int val = va_arg(args, int);
                itoa(val, buf);
                print_text(buf);
            } else if (*fmt == 's') {
                char* s = va_arg(args, char*);
                print_text(s);
            }
            fmt++;
        } else {
            putchar(*fmt++);
        }
    }

    va_end(args);
}

// println with formating
void println(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // Everything is the same as printf, but we add '\n'
    char buf[32];
    const char* p = fmt;
    while (*p) {
        if (*p == '%') {
            p++;
            if (*p == 'd') {
                int val = va_arg(args, int);
                itoa(val, buf);
                print_text(buf);
            } else if (*p == 's') {
                char* s = va_arg(args, char*);
                print_text(s);
            }
            p++;
        } else {
            putchar(*p++);
        }
    }
    putchar('\n');

    va_end(args);
}

// Clear the entire screen
void clear_screen(void) {
    volatile unsigned short* vga = (unsigned short*)VGA_MEMORY;

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = ((unsigned short)color << 8) | ' ';
    }

    cursor_x = 0;
    cursor_y = 0;
}
// Write a byte to the specified port
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// move vga cursor to the current position
void move_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// scroll the screen up by one line
void scroll() {
    volatile unsigned short* vga = (unsigned short*)VGA_MEMORY;

    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
        vga[i] = vga[i + VGA_WIDTH];
    }

    for (int i = VGA_WIDTH * (VGA_HEIGHT - 1);
         i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = ((unsigned short)color << 8) | ' ';
    }
}

// put character to the screen
void putchar(char c) {
    volatile unsigned short* vga = (unsigned short*)VGA_MEMORY;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    }
    else {
        vga[cursor_y * VGA_WIDTH + cursor_x] =
            ((unsigned short)color << 8) | c;

        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        scroll();
        cursor_y = VGA_HEIGHT - 1;
    }

    move_cursor();
}
