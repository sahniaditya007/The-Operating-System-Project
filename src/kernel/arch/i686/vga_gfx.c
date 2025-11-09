#include "vga_gfx.h"
#include <arch/i686/io.h>

static uint8_t* g_vga_buffer = (uint8_t*)0xA0000;

void vga_set_mode(int mode) {
    // BIOS video interrupts (int 0x10) are not available in protected mode.
    // Setting VGA mode should be done either in the bootloader (real mode)
    // or by directly programming VGA registers here. For now, this is a no-op
    // to avoid faulting on vector 0x10 (which maps to x87 exception in our IDT).
    (void)mode;
}

// Wait for vertical retrace to prevent flickering
void vga_wait_vsync(void) {
    // Wait for vertical retrace to end (bit 3 = 0)
    while (i686_inb(0x3DA) & 0x08);
    // Wait for vertical retrace to start (bit 3 = 1)
    while (!(i686_inb(0x3DA) & 0x08));
}

void vga_put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) {
        return;
    }
    g_vga_buffer[y * 320 + x] = color;
}

void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    // For now, only horizontal and vertical lines are supported
    if (x0 == x1) { // Vertical line
        for (int y = y0; y <= y1; y++) {
            vga_put_pixel(x0, y, color);
        }
    } else if (y0 == y1) { // Horizontal line
        for (int x = x0; x <= x1; x++) {
            vga_put_pixel(x, y0, color);
        }
    }
}

void vga_draw_rect(int x, int y, int width, int height, uint8_t color) {
    vga_draw_line(x, y, x + width - 1, y, color);
    vga_draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);
    vga_draw_line(x, y, x, y + height - 1, color);
    vga_draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void vga_fill_rect(int x, int y, int width, int height, uint8_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            vga_put_pixel(x + j, y + i, color);
        }
    }
}

uint8_t vga_get_pixel(int x, int y) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) {
        return 0;
    }
    return g_vga_buffer[y * 320 + x];
}
