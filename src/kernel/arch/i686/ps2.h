#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PS2_PORT_KEYBOARD = 0,
    PS2_PORT_MOUSE = 1,
} PS2Port;

void ps2_flush_output_buffer(void);
bool ps2_send_controller_command(uint8_t command);
uint8_t ps2_read_data(void);
bool ps2_write_data(uint8_t data);
bool ps2_write_mouse_data(uint8_t data);
uint8_t ps2_get_command_byte(void);
void ps2_set_command_byte(uint8_t command_byte);
void ps2_configure_port(PS2Port port, bool enable_irq, bool enable_clock);
void ps2_enable_translation(bool enable);
bool ps2_expect_ack(void);
