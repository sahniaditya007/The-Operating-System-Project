#include "mouse.h"
#include <arch/i686/irq.h>
#include <arch/i686/io.h>
#include <debug.h>
#include <arch/i686/pic.h>
#include <arch/i686/vga_gfx.h>
#include <arch/i686/ps2.h>

#define PS2_DATA_PORT 0x60

static uint8_t mouse_cycle = 0;
static int8_t mouse_byte[3];
int32_t mouse_x = 0;
int32_t mouse_y = 0;
bool mouse_left_button = false;
bool mouse_right_button = false;

void draw_cursor(int x, int y, uint8_t color) {
    // Draw a visible arrow-like cursor (Windows 1 style)
    // Draw a small crosshair cursor that's more visible
    if (x < 318 && y < 198) {
        // Main cursor body (2x2 square)
        vga_put_pixel(x, y, color);
        vga_put_pixel(x + 1, y, color);
        vga_put_pixel(x, y + 1, color);
        vga_put_pixel(x + 1, y + 1, color);
        // Add outline for visibility
        vga_put_pixel(x + 2, y, 0); // Black outline
        vga_put_pixel(x, y + 2, 0);
    }
}

static void mouse_handler(Registers* regs) {
    (void)regs;
    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = (int8_t)i686_inb(PS2_DATA_PORT);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = (int8_t)i686_inb(PS2_DATA_PORT);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = (int8_t)i686_inb(PS2_DATA_PORT);
            // Update mouse position (mouse_byte[1] and [2] are signed deltas)
            int32_t new_x = mouse_x + mouse_byte[1];
            int32_t new_y = mouse_y - mouse_byte[2]; // Y is inverted
            
            // Clamp to screen bounds
            if (new_x < 0) new_x = 0;
            if (new_x > 319) new_x = 319;
            if (new_y < 0) new_y = 0;
            if (new_y > 199) new_y = 199;
            
            mouse_x = new_x;
            mouse_y = new_y;
            
            // Update button states (bit 0 = left, bit 1 = right)
            mouse_left_button = (mouse_byte[0] & 0x01) != 0;
            mouse_right_button = (mouse_byte[0] & 0x02) != 0;
            
            log_debug("Mouse", "x=%d, y=%d, buttons=%x", mouse_x, mouse_y, mouse_byte[0]);
            mouse_cycle = 0;
            break;
    }
}

void mouse_install() {
    log_debug("Mouse", "Initializing mouse...");

    ps2_flush_output_buffer();

    if (!ps2_send_controller_command(0xA8)) {
        log_debug("Mouse", "ERROR: Failed to enable auxiliary device.");
    } else {
        log_debug("Mouse", "Auxiliary device enabled.");
    }

    ps2_configure_port(PS2_PORT_MOUSE, true, true);

    // Set default settings
    if (!ps2_write_mouse_data(0xF6) || !ps2_expect_ack()) {
        log_debug("Mouse", "WARNING: Failed to set default settings.");
    } else {
        log_debug("Mouse", "Default settings set.");
    }

    // Enable the mouse
    if (!ps2_write_mouse_data(0xF4) || !ps2_expect_ack()) {
        log_debug("Mouse", "WARNING: Failed to enable mouse device.");
    } else {
        log_debug("Mouse", "Mouse enabled.");
    }

    i686_IRQ_RegisterHandler(12, mouse_handler);
    const PICDriver* pic = i686_IRQ_GetDriver();
    if (pic) {
        // IRQ12 lives on the slave PIC, so ensure the cascade line (IRQ2) is unmasked
        pic->Unmask(2);
        pic->Unmask(12);
    }
    log_debug("Mouse", "Mouse driver installed.");
}
