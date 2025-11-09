#pragma once

#include <stdint.h>
#include <stdbool.h>

extern int32_t mouse_x;
extern int32_t mouse_y;
extern bool mouse_left_button;
extern bool mouse_right_button;

void mouse_install();
void draw_cursor(int x, int y, uint8_t color);
