/*
 * keyboard.c - The keyboard driver
 * Copyright (c) 2026, Redstone2888
 * Read LICENSE.txt for details
 * 
 * This file contains keyboard input for kernel:
 *  - Getting key
 *  - Reading line
 * 
 * Without this file, the kernel will not be able 
 * to read keyboard.
 * 
 * Not recommended to delete.
 */

#include <vga.h>
#include <keyboard.h>
#include <asm.h>

// keyboard map
char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,0,0,' '
};

unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd" (port));
    return ret;
}

// get key from keyboard
char getkey() {
    while (1) {
        unsigned char sc = inb(0x60);

        // press
        if (!(sc & 0x80)) {
            char c = keymap[sc];

            while (!(inb(0x60) & 0x80));

            return c;
        }
    }
}

// reading line
void readline(char* buf, int max) {
    int i = 0;

    while (1) {
        char c = getkey();

        if (!c) continue;

        // enter
        if (c == '\n') {
            putchar('\n');
            buf[i] = 0;
            return;
        }

        // backspace
        if (c == '\b') {
            if (i > 0) {
                i--;
                cursor_x--;
                putchar(' ');
                cursor_x--;
            }
            continue;
        }

        // normal key
        if (i < max - 1) {
            buf[i++] = c;
            putchar(c);
        }
    }
}