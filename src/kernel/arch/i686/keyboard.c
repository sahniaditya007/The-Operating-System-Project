#include "keyboard.h"
#include "io.h"
#include "irq.h"
#include <debug.h>
#include <arch/i686/ps2.h>

#define KEYBOARD_DATA_PORT   0x60

static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

#define KEY_BUFFER_SIZE 64
static KeyEvent key_buffer[KEY_BUFFER_SIZE];
static int key_buffer_read = 0;
static int key_buffer_write = 0;
static bool shift_pressed = false;

static void keyboard_handler(Registers* regs) {
    (void)regs;
    
    uint8_t scancode = i686_inb(KEYBOARD_DATA_PORT);
    
    // Handle key release (scancode with bit 7 set)
    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == KEY_LSHIFT) {
            shift_pressed = false;
        }
        return;
    }
    
    // Handle shift key
    if (scancode == KEY_LSHIFT) {
        shift_pressed = true;
        return;
    }
    
    // Convert scancode to ASCII
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }
    }
    
    // Add to key buffer
    int next_write = (key_buffer_write + 1) % KEY_BUFFER_SIZE;
    if (next_write != key_buffer_read) {
        key_buffer[key_buffer_write].pressed = true;
        key_buffer[key_buffer_write].ascii = ascii;
        key_buffer[key_buffer_write].scancode = scancode;
        key_buffer_write = next_write;
    }
}

void keyboard_install(void) {
    log_debug("Keyboard", "Installing keyboard driver...");

    // Ensure the controller is ready to generate interrupts for port 1
    ps2_flush_output_buffer();
    ps2_enable_translation(true);
    ps2_configure_port(PS2_PORT_KEYBOARD, true, true);

    key_buffer_read = 0;
    key_buffer_write = 0;
    shift_pressed = false;

    // Enable keyboard scanning so that key presses are reported
    if (!ps2_write_data(0xF4) || !ps2_expect_ack()) {
        log_debug("Keyboard", "WARNING: Failed to enable keyboard scanning");
    }

    i686_IRQ_RegisterHandler(1, keyboard_handler);
    
    const PICDriver* pic = i686_IRQ_GetDriver();
    if (pic) {
        pic->Unmask(1);
        log_debug("Keyboard", "Keyboard IRQ unmasked");
    } else {
        log_debug("Keyboard", "ERROR: PIC driver not available!");
    }
    
    log_debug("Keyboard", "Keyboard driver installed.");
}

bool keyboard_has_key(void) {
    return key_buffer_read != key_buffer_write;
}

KeyEvent keyboard_get_key(void) {
    KeyEvent event = {0};
    
    if (keyboard_has_key()) {
        event = key_buffer[key_buffer_read];
        key_buffer_read = (key_buffer_read + 1) % KEY_BUFFER_SIZE;
    }
    
    return event;
}

char keyboard_get_char(void) {
    KeyEvent event = keyboard_get_key();
    return event.ascii;
}
