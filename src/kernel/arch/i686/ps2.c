#include "ps2.h"
#include "io.h"

#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64
#define PS2_CMD_PORT     0x64

#define PS2_STATUS_OUTPUT_FULL 0x01
#define PS2_STATUS_INPUT_FULL  0x02

static bool ps2_wait_for_input_ready(void) {
    uint32_t timeout = 1000000;
    while (timeout--) {
        if ((i686_inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) == 0) {
            return true;
        }
    }
    return false;
}

static bool ps2_wait_for_output_ready(void) {
    uint32_t timeout = 1000000;
    while (timeout--) {
        if (i686_inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
            return true;
        }
    }
    return false;
}

void ps2_flush_output_buffer(void) {
    while (i686_inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) {
        (void)i686_inb(PS2_DATA_PORT);
    }
}

bool ps2_send_controller_command(uint8_t command) {
    if (!ps2_wait_for_input_ready()) {
        return false;
    }
    i686_outb(PS2_CMD_PORT, command);
    return true;
}

bool ps2_write_data(uint8_t data) {
    if (!ps2_wait_for_input_ready()) {
        return false;
    }
    i686_outb(PS2_DATA_PORT, data);
    return true;
}

bool ps2_write_mouse_data(uint8_t data) {
    if (!ps2_send_controller_command(0xD4)) {
        return false;
    }
    return ps2_write_data(data);
}

uint8_t ps2_read_data(void) {
    if (!ps2_wait_for_output_ready()) {
        return 0;
    }
    return i686_inb(PS2_DATA_PORT);
}

uint8_t ps2_get_command_byte(void) {
    if (!ps2_send_controller_command(0x20)) {
        return 0;
    }
    return ps2_read_data();
}

void ps2_set_command_byte(uint8_t command_byte) {
    if (!ps2_send_controller_command(0x60)) {
        return;
    }
    ps2_write_data(command_byte);
}

void ps2_configure_port(PS2Port port, bool enable_irq, bool enable_clock) {
    uint8_t command = ps2_get_command_byte();

    if (port == PS2_PORT_KEYBOARD) {
        if (enable_irq) command |= 0x01;
        else command &= ~0x01;

        if (enable_clock) command &= ~(1 << 4);
        else command |= (1 << 4);
    } else { // PS2_PORT_MOUSE
        if (enable_irq) command |= 0x02;
        else command &= ~0x02;

        if (enable_clock) command &= ~(1 << 5);
        else command |= (1 << 5);
    }

    ps2_set_command_byte(command);
}

void ps2_enable_translation(bool enable) {
    uint8_t command = ps2_get_command_byte();
    if (enable) command |= (1 << 6);
    else command &= ~(1 << 6);
    ps2_set_command_byte(command);
}

bool ps2_expect_ack(void) {
    return ps2_read_data() == 0xFA;
}
