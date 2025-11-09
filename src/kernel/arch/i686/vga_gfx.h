#pragma once

#include <stdint.h>

void vga_set_mode(int mode);
void vga_wait_vsync(void);
void vga_put_pixel(int x, int y, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_rect(int x, int y, int width, int height, uint8_t color);
void vga_fill_rect(int x, int y, int width, int height, uint8_t color);
uint8_t vga_get_pixel(int x, int y);
