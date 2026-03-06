#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>
#include <string.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

static volatile uint16_t* const VGA = (uint16_t*)0xB8000;

// here's colors

enum {
    COLOR_BLACK         = 0x0,
    COLOR_BLUE          = 0x1,
    COLOR_GREEN         = 0x2,
    COLOR_CYAN          = 0x3,
    COLOR_RED           = 0x4,
    COLOR_MAGENTA       = 0x5,
    COLOR_BROWN         = 0x6,
    COLOR_LIGHT_GREY    = 0x7,
    COLOR_DARK_GREY     = 0x8,
    COLOR_LIGHT_BLUE    = 0x9,
    COLOR_LIGHT_GREEN   = 0xA,
    COLOR_LIGHT_CYAN    = 0xB,
    COLOR_LIGHT_RED     = 0xC,
    COLOR_PINK          = 0xD,
    COLOR_YELLOW        = 0xE,
    COLOR_WHITE         = 0xF
};

static inline uint8_t make_color(uint8_t fg, uint8_t bg)
{
    return fg | (bg << 4);
}

// core

static inline void draw_cell(int x, int y, char c, uint8_t color)
{
    if (x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;

    VGA[y * VGA_WIDTH + x] = (color << 8) | c;
}

// just drawing pixel
static inline void draw_pixel(int x, int y, uint8_t color)
{
    draw_cell(x, y, ' ', color);
}

// clear screen func
static inline void draw_clear(uint8_t color)
{
    for (int y = 0; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            draw_pixel(x, y, color);
}

// here's rects OMG
static inline void draw_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
            draw_pixel(x + xx, y + yy, color);
}

// just border lool
static inline void draw_border(int x, int y, int w, int h, uint8_t color)
{
    for (int i = 0; i < w; i++) {
        draw_pixel(x + i, y, color);
        draw_pixel(x + i, y + h - 1, color);
    }

    for (int i = 0; i < h; i++) {
        draw_pixel(x, y + i, color);
        draw_pixel(x + w - 1, y + i, color);
    }
}

// drawing text on x, y
static inline void draw_text(int x, int y, const char* str, uint8_t color)
{
    int i = 0;
    while (str[i]) {
        draw_cell(x + i, y, str[i], color);
        i++;
    }
}

// drawing text AT CENTER
static inline void draw_text_center(const char* str, uint8_t fg, uint8_t bg)
{
    int x_centre = (VGA_WIDTH - strlen(str)) / 2;
    int y_centre = VGA_HEIGHT / 2;

    draw_text(x_centre, y_centre, str, make_color(fg, bg));
}

// its too, but with y
static inline void draw_text_center_at(const char* str, uint8_t fg, uint8_t bg, int y)
{
    int x_centre = (VGA_WIDTH - strlen(str)) / 2;
    draw_text(x_centre, y, str, make_color(fg, bg));
}

// drawing character
static inline void draw_char(int x, int y, char c, uint8_t fg, uint8_t bg)
{
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;

    VGA[y * VGA_WIDTH + x] = ((bg << 12) | (fg << 8) | c); 
}

#endif