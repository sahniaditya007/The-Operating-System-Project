#pragma once

#include <stdint.h>

extern char font8x8_basic[128][8];

void font_draw_char(int x, int y, char c, uint8_t color);
