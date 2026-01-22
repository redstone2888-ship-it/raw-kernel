/* kernel.c - Enhanced with basic functions */

/* Multiboot header */
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,             /* Magic number */
    0x00000003,             /* Flags */
    -(0x1BADB002 + 0x00000003) /* Checksum */
};

/* VGA text mode constants */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* Colors */
enum vga_color {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15,
};

/* Current cursor position */
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char color = 0x0F; /* White on black */

/* Make a color byte from foreground and background */
static inline unsigned char make_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

/* Make a VGA entry from character and color */
static inline unsigned short make_vgaentry(char c, unsigned char color) {
    return (unsigned short)c | ((unsigned short)color << 8);
}

/* Simple delay function - busy wait */
void delay(unsigned int cycles) {
    for (volatile unsigned int i = 0; i < cycles * 10000; i++) {
        __asm__ __volatile__("nop");
    }
}

/* Clear the entire screen */
void clear_screen(void) {
    unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;
    
    unsigned short blank = make_vgaentry(' ', color);
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
}

/* Set text color */
void set_color(enum vga_color fg, enum vga_color bg) {
    color = make_color(fg, bg);
}

/* Scroll the screen up by one line */
static void scroll(void) {
    unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;
    
    /* Move all lines up */
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    /* Clear the last line */
    unsigned short blank = make_vgaentry(' ', color);
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

/* Put a single character at current position */
void putchar(char c) {
    unsigned short* vga_buffer = (unsigned short*)VGA_MEMORY;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = make_vgaentry(c, color);
        cursor_x++;
    }
    
    /* Handle line wrap and scroll */
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }
}

/* Print a null-terminated string */
void print_text(const char* str) {
    while (*str) {
        putchar(*str);
        str++;
    }
}

/* Print a string with newline */
void println(const char* str) {
    print_text(str);
    putchar('\n');
}

/* Print a number in decimal */
void print_dec(unsigned int num) {
    char buffer[12];
    int i = 0;
    
    if (num == 0) {
        putchar('0');
        return;
    }
    
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        putchar(buffer[j]);
    }
}

/* Print a number in hexadecimal */
void print_hex(unsigned int num) {
    print_text("0x");
    
    if (num == 0) {
        putchar('0');
        return;
    }
    
    char buffer[9];
    int i = 0;
    
    while (num > 0) {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
        num /= 16;
    }
    
    for (int j = i - 1; j >= 0; j--) {
        putchar(buffer[j]);
    }
}

/* Kernel main function */
void kernel_main(void) {
    /* Initialize */
    set_color(COLOR_WHITE, COLOR_BLACK);
    clear_screen();
    
    /* Welcome message */
    println("==========================================");
    println("           RawOS Kernel v0.1");
    println("==========================================");
    println("");
    
    /* Demo of functions */
    print_text("Initializing system... ");
    delay(100);
    println("OK");
    
    print_text("Memory check... ");
    delay(80);
    println("OK");
    
    print_text("VGA driver... ");
    delay(60);
    println("OK");
    println("");
    
    /* Color demo */
    println("Color test:");
    set_color(COLOR_RED, COLOR_BLACK);
    print_text("Red ");
    set_color(COLOR_GREEN, COLOR_BLACK);
    print_text("Green ");
    set_color(COLOR_BLUE, COLOR_BLACK);
    print_text("Blue ");
    set_color(COLOR_YELLOW, COLOR_BLACK);
    print_text("Yellow ");
    set_color(COLOR_CYAN, COLOR_BLACK);
    print_text("Cyan ");
    set_color(COLOR_MAGENTA, COLOR_BLACK);
    println("Magenta");
    
    set_color(COLOR_WHITE, COLOR_BLACK);
    println("");
    
    /* Number demo */
    print_text("Decimal: ");
    print_dec(12345);
    putchar('\n');
    
    print_text("Hexadecimal: ");
    print_hex(0xDEADBEEF);
    putchar('\n');
    println("");
    
    /* Progress bar demo */
    println("Loading complete!");
    print_text("[");
    for (int i = 0; i < 20; i++) {
        putchar('#');
        delay(30);
    }
    println("] 100%");
    println("");
    
    /* Final message */
    set_color(COLOR_LIGHT_GREEN, COLOR_BLACK);
    println("System ready. Welcome to RawOS!");
    set_color(COLOR_WHITE, COLOR_BLACK);
    println("Type 'help' for commands...");
    
    /* Kernel loop */
    while (1) {
        /* Just idle */
        __asm__ __volatile__("hlt");
    }
}