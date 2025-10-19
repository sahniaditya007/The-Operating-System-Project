#pragma once

#include <stdint.h>

extern int32_t mouse_x;
extern int32_t mouse_y;

void mouse_install();
void draw_cursor(int x, int y, uint8_t color);
