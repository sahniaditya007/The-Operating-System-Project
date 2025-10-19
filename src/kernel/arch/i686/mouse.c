#include "mouse.h"
#include <arch/i686/irq.h>
#include <arch/i686/io.h>
#include <debug.h>
#include <arch/i686/pic.h>
#include <arch/i686/vga_gfx.h>

static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
int32_t mouse_x = 0;
int32_t mouse_y = 0;

void draw_cursor(int x, int y, uint8_t color) {
    vga_put_pixel(x, y, color);
    vga_put_pixel(x + 1, y, color);
    vga_put_pixel(x, y + 1, color);
    vga_put_pixel(x + 1, y + 1, color);
}

void mouse_wait(uint8_t a_type) {
    uint32_t _time_out = 100000;
    if (a_type == 0) {
        while (_time_out--) {
            if ((i686_inb(0x64) & 1) == 1) {
                return;
            }
        }
        return;
    } else {
        while (_time_out--) {
            if ((i686_inb(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    i686_outb(0x64, 0xD4);
    mouse_wait(1);
    i686_outb(0x60, a_write);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return i686_inb(0x60);
}

static void mouse_handler(Registers* regs) {
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_read();
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = mouse_read();
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = mouse_read();
            mouse_x += mouse_byte[1];
            mouse_y -= mouse_byte[2];
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > 319) mouse_x = 319;
            if (mouse_y > 199) mouse_y = 199;
            log_debug("Mouse", "x=%d, y=%d", mouse_x, mouse_y);
            mouse_cycle = 0;
            break;
    }
}

void mouse_install() {
    uint8_t status;

    log_debug("Mouse", "Initializing mouse...");

    // Enable the auxiliary mouse device
    mouse_wait(1);
    i686_outb(0x64, 0xA8);
    log_debug("Mouse", "Auxiliary device enabled.");

    // Enable the interrupts
    mouse_wait(1);
    i686_outb(0x64, 0x20);
    mouse_wait(0);
    status = (i686_inb(0x60) | 2);
    mouse_wait(1);
    i686_outb(0x64, 0x60);
    mouse_wait(1);
    i686_outb(0x60, status);
    log_debug("Mouse", "Interrupts enabled.");

    // Set default settings
    mouse_write(0xF6);
    mouse_read();
    log_debug("Mouse", "Default settings set.");

    // Enable the mouse
    mouse_write(0xF4);
    mouse_read();
    log_debug("Mouse", "Mouse enabled.");

    i686_IRQ_RegisterHandler(12, mouse_handler);
    const PICDriver* pic = i686_IRQ_GetDriver();
    if (pic) {
        pic->Unmask(12);
    }
    log_debug("Mouse", "Mouse driver installed.");
}
