#include <vga.h>
#include <shed.h>
#include <draw.h>
#include <programs.h>
#include <asm.h>
#include <string.h>

void gui() {
    cli();

    clear_screen();

    // верхняя плашка
    draw_rect(0, 0, 80, 5, make_color(COLOR_WHITE, COLOR_BLUE));
    draw_border(0, 0, 80, 5, make_color(COLOR_WHITE, COLOR_BLUE));
    draw_text_center_at("RawOS v0.2", COLOR_WHITE, COLOR_BLUE, 2);
    for (int i = 0; i < 80; i++) {
        draw_text(i, 0, "=", make_color(COLOR_WHITE, COLOR_BLUE));
        draw_text(i, 4, "=", make_color(COLOR_WHITE, COLOR_BLUE));
    }
    for (int i = 1; i < 4; i++) {
        draw_text(0, i, "|", make_color(COLOR_WHITE, COLOR_BLUE));
        draw_text(79, i, "|", make_color(COLOR_WHITE, COLOR_BLUE));
    }

    // центр
    draw_rect(0, 5, 80, VGA_HEIGHT-8, make_color(COLOR_BLACK, COLOR_LIGHT_GREY));
    // code...

    // низ
    draw_rect(0, VGA_HEIGHT - 3, 80, VGA_HEIGHT-5, make_color(COLOR_WHITE, COLOR_BLUE));
    for (int i = 0; i < VGA_WIDTH; i++) {
        draw_text(i, VGA_HEIGHT-3, "=", make_color(COLOR_WHITE, COLOR_BLUE));
        draw_text(i, VGA_HEIGHT-1, "=", make_color(COLOR_WHITE, COLOR_BLUE));
    }

    draw_text_center("Hello, user...", COLOR_BLACK, COLOR_LIGHT_GREY);

    while(1) hlt();
}