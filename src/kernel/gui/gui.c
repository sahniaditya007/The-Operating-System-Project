#include "gui.h"
#include <arch/i686/vga_gfx.h>
#include <arch/i686/mouse.h>
#include <arch/i686/keyboard.h>
#include <util/font.h>
#include <boot/bootparams.h>
#include <string/string.h>

extern BootParams* g_BootParams;

static GUIState g_gui_state = {0};

// Windows 1 color palette
#define WIN_BG_COLOR        COLOR_LIGHT_GRAY  // Desktop background
#define WIN_WINDOW_BG       COLOR_LIGHT_GRAY  // Window background
#define WIN_TITLE_BG        COLOR_DARK_GRAY   // Title bar
#define WIN_BORDER_LIGHT    COLOR_WHITE       // Light border (3D effect)
#define WIN_BORDER_DARK     COLOR_BLACK       // Dark border (3D effect)
#define WIN_BUTTON_BG       COLOR_LIGHT_GRAY  // Button background
#define WIN_BUTTON_HOVER    COLOR_WHITE       // Button hover
#define WIN_TASKBAR_BG      COLOR_DARK_GRAY   // Taskbar background

// Helper function: Draw string
void gui_draw_string(int x, int y, const char* str, uint8_t color) {
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            y += 9;
            cx = x;
        } else {
            font_draw_char(cx, y, *str, color);
            cx += 8;
        }
        str++;
    }
}

// Helper: int to string
static void int_to_str(int num, char* buf, int bufsize) {
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    int negative = 0;
    if (num < 0) {
        negative = 1;
        num = -num;
    }
    
    int i = 0;
    while (num > 0 && i < bufsize - 1) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if (negative && i < bufsize - 1) {
        buf[i++] = '-';
    }
    
    buf[i] = '\0';
    
    // Reverse
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = tmp;
    }
}

// Windows 1 style window with 3D borders
void gui_draw_window(int x, int y, int width, int height, const char* title) {
    // Draw 3D border effect (Windows 1 style)
    // Light border (top and left)
    vga_draw_line(x, y, x + width - 1, y, WIN_BORDER_LIGHT);
    vga_draw_line(x, y, x, y + height - 1, WIN_BORDER_LIGHT);
    // Dark border (bottom and right)
    vga_draw_line(x, y + height - 1, x + width - 1, y + height - 1, WIN_BORDER_DARK);
    vga_draw_line(x + width - 1, y, x + width - 1, y + height - 1, WIN_BORDER_DARK);
    
    // Window background
    vga_fill_rect(x + 1, y + 1, width - 2, height - 2, WIN_WINDOW_BG);
    
    // Title bar (Windows 1 style - dark gray)
    vga_fill_rect(x + 1, y + 1, width - 2, 12, WIN_TITLE_BG);
    // Title bar text
    gui_draw_string(x + 4, y + 3, title, COLOR_WHITE);
    
    // Close button (small square in title bar)
    int close_x = x + width - 12;
    int close_y = y + 2;
    // Button with 3D effect
    vga_fill_rect(close_x, close_y, 8, 8, WIN_BUTTON_BG);
    vga_draw_line(close_x, close_y, close_x + 7, close_y, WIN_BORDER_LIGHT);
    vga_draw_line(close_x, close_y, close_x, close_y + 7, WIN_BORDER_LIGHT);
    vga_draw_line(close_x, close_y + 7, close_x + 7, close_y + 7, WIN_BORDER_DARK);
    vga_draw_line(close_x + 7, close_y, close_x + 7, close_y + 7, WIN_BORDER_DARK);
    // X mark
    vga_put_pixel(close_x + 2, close_y + 2, COLOR_BLACK);
    vga_put_pixel(close_x + 5, close_y + 2, COLOR_BLACK);
    vga_put_pixel(close_x + 3, close_y + 4, COLOR_BLACK);
    vga_put_pixel(close_x + 4, close_y + 4, COLOR_BLACK);
    vga_put_pixel(close_x + 2, close_y + 5, COLOR_BLACK);
    vga_put_pixel(close_x + 5, close_y + 5, COLOR_BLACK);
}

// Windows 1 style button
void gui_draw_button(Rect rect, const char* label, bool hover) {
    uint8_t bg_color = hover ? WIN_BUTTON_HOVER : WIN_BUTTON_BG;
    
    // Button with 3D effect
    vga_fill_rect(rect.x, rect.y, rect.width, rect.height, bg_color);
    // Light border (top and left)
    vga_draw_line(rect.x, rect.y, rect.x + rect.width - 1, rect.y, WIN_BORDER_LIGHT);
    vga_draw_line(rect.x, rect.y, rect.x, rect.y + rect.height - 1, WIN_BORDER_LIGHT);
    // Dark border (bottom and right)
    vga_draw_line(rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1, WIN_BORDER_DARK);
    vga_draw_line(rect.x + rect.width - 1, rect.y, rect.x + rect.width - 1, rect.y + rect.height - 1, WIN_BORDER_DARK);
    
    int text_x = rect.x + (rect.width - strlen(label) * 8) / 2;
    int text_y = rect.y + (rect.height - 8) / 2;
    gui_draw_string(text_x, text_y, label, COLOR_BLACK);
}

// Check if point in rect
bool gui_is_point_in_rect(Point p, Rect r) {
    return p.x >= r.x && p.x < r.x + r.width &&
           p.y >= r.y && p.y < r.y + r.height;
}

// Check if button was clicked
bool gui_button_clicked(Rect rect) {
    Point mouse = {g_gui_state.mouse_x, g_gui_state.mouse_y};
    return gui_is_point_in_rect(mouse, rect) && 
           g_gui_state.mouse_clicked && !g_gui_state.mouse_was_clicked;
}

void gui_switch_app(AppID app) {
    g_gui_state.current_app = app;
    
    // Initialize the new app
    switch (app) {
        case APP_DESKTOP: app_desktop_init(); break;
        case APP_CLOCK: app_clock_init(); break;
        case APP_CALENDAR: app_calendar_init(); break;
        case APP_NOTEPAD: app_notepad_init(); break;
        case APP_SNAKE: app_snake_init(); break;
        case APP_SETTINGS: app_settings_init(); break;
        case APP_SYSMON: app_sysmon_init(); break;
        default: break;
    }
}

// ======================
// DESKTOP APPLICATION
// ======================
void app_desktop_init(void) {
    // Windows 1 style desktop - light gray background
    vga_fill_rect(0, 0, 320, 200, WIN_BG_COLOR);
}

void app_desktop_update(void) {
    // Check app buttons (Windows 1 style) — unified grid for consistent alignment
    const int margin_x = 10;      // left/right margin
    const int top_y = 30;         // grid top (leave room for title)
    const int btn_w = 70;         // uniform width for all buttons (wide enough for long labels)
    const int btn_h = 30;         // uniform height
    const int gap_x = 10;         // horizontal gap between buttons
    const int gap_y = 10;         // vertical gap between rows

    // Column x-positions
    const int col0 = margin_x;
    const int col1 = margin_x + (btn_w + gap_x);
    const int col2 = margin_x + 2 * (btn_w + gap_x);
    const int col3 = margin_x + 3 * (btn_w + gap_x);

    // Row y-positions
    const int row0 = top_y;
    const int row1 = top_y + btn_h + gap_y;

    Rect clock_btn    = (Rect){col0, row0, btn_w, btn_h};
    Rect calendar_btn = (Rect){col1, row0, btn_w, btn_h};
    Rect notepad_btn  = (Rect){col2, row0, btn_w, btn_h};
    Rect snake_btn    = (Rect){col3, row0, btn_w, btn_h};
    Rect settings_btn = (Rect){col0, row1, btn_w, btn_h};
    Rect sysmon_btn   = (Rect){col1, row1, btn_w, btn_h};
    
    if (gui_button_clicked(clock_btn)) gui_switch_app(APP_CLOCK);
    if (gui_button_clicked(calendar_btn)) gui_switch_app(APP_CALENDAR);
    if (gui_button_clicked(notepad_btn)) gui_switch_app(APP_NOTEPAD);
    if (gui_button_clicked(snake_btn)) gui_switch_app(APP_SNAKE);
    if (gui_button_clicked(settings_btn)) gui_switch_app(APP_SETTINGS);
    if (gui_button_clicked(sysmon_btn)) gui_switch_app(APP_SYSMON);
}

void app_desktop_draw(void) {
    // Windows 1 style desktop
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR); // Desktop area (leave space for taskbar)
    
    // Center the title for better alignment
    const char* title = "The Operating System Project";
    int title_x = (320 - (int)strlen(title) * 8) / 2;
    if (title_x < 0) title_x = 0;
    gui_draw_string(title_x, 8, title, COLOR_BLACK);
    
    Point mouse = {g_gui_state.mouse_x, g_gui_state.mouse_y};
    
    // App buttons (unified grid to fix misalignment)
    const int margin_x = 10;
    const int top_y = 30;
    const int btn_w = 70;
    const int btn_h = 30;
    const int gap_x = 10;
    const int gap_y = 10;

    const int col0 = margin_x;
    const int col1 = margin_x + (btn_w + gap_x);
    const int col2 = margin_x + 2 * (btn_w + gap_x);
    const int col3 = margin_x + 3 * (btn_w + gap_x);
    const int row0 = top_y;
    const int row1 = top_y + btn_h + gap_y;

    Rect clock_btn    = (Rect){col0, row0, btn_w, btn_h};
    Rect calendar_btn = (Rect){col1, row0, btn_w, btn_h};
    Rect notepad_btn  = (Rect){col2, row0, btn_w, btn_h};
    Rect snake_btn    = (Rect){col3, row0, btn_w, btn_h};
    Rect settings_btn = (Rect){col0, row1, btn_w, btn_h};
    Rect sysmon_btn   = (Rect){col1, row1, btn_w, btn_h};
    
    gui_draw_button(clock_btn, "Clock", gui_is_point_in_rect(mouse, clock_btn));
    gui_draw_button(calendar_btn, "Calendar", gui_is_point_in_rect(mouse, calendar_btn));
    gui_draw_button(notepad_btn, "Notepad", gui_is_point_in_rect(mouse, notepad_btn));
    gui_draw_button(snake_btn, "Snake", gui_is_point_in_rect(mouse, snake_btn));
    gui_draw_button(settings_btn, "Settings", gui_is_point_in_rect(mouse, settings_btn));
    gui_draw_button(sysmon_btn, "Monitor", gui_is_point_in_rect(mouse, sysmon_btn));
    
    // Draw taskbar (Windows 1 style)
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// CLOCK APPLICATION
// ======================
static uint32_t clock_hours = 12;
static uint32_t clock_minutes = 0;
static uint32_t clock_seconds = 0;

void app_clock_init(void) {
    clock_seconds = (g_gui_state.ticks / 18) % 60;
    clock_minutes = (g_gui_state.ticks / 1080) % 60;
    clock_hours = (g_gui_state.ticks / 64800) % 24;
}

void app_clock_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    if (gui_button_clicked(close_btn)) {
        gui_switch_app(APP_DESKTOP);
    }
}

void app_clock_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(40, 30, 240, 120, "Clock");
    
    clock_seconds = (g_gui_state.ticks / 18) % 60;
    clock_minutes = (g_gui_state.ticks / 1080) % 60;
    clock_hours = (g_gui_state.ticks / 64800) % 24;
    
    char time_str[32];
    char buf1[8], buf2[8], buf3[8];
    
    int_to_str(clock_hours, buf1, 8);
    int_to_str(clock_minutes, buf2, 8);
    int_to_str(clock_seconds, buf3, 8);
    
    time_str[0] = '\0';
    strcat(time_str, buf1);
    strcat(time_str, ":");
    if (clock_minutes < 10) strcat(time_str, "0");
    strcat(time_str, buf2);
    strcat(time_str, ":");
    if (clock_seconds < 10) strcat(time_str, "0");
    strcat(time_str, buf3);
    
    gui_draw_string(120, 80, time_str, COLOR_BLACK);
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// CALENDAR APPLICATION
// ======================
static int cal_month = 1;
static int cal_year = 2025;

void app_calendar_init(void) {
    cal_month = 1;
    cal_year = 2025;
}

void app_calendar_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    Rect prev_btn = {40, 150, 40, 15};
    Rect next_btn = {240, 150, 40, 15};
    
    if (gui_button_clicked(close_btn)) gui_switch_app(APP_DESKTOP);
    if (gui_button_clicked(prev_btn)) {
        cal_month--;
        if (cal_month < 1) { cal_month = 12; cal_year--; }
    }
    if (gui_button_clicked(next_btn)) {
        cal_month++;
        if (cal_month > 12) { cal_month = 1; cal_year++; }
    }
}

void app_calendar_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(30, 20, 260, 140, "Calendar");
    
    char title[32];
    char month_buf[8], year_buf[8];
    int_to_str(cal_month, month_buf, 8);
    int_to_str(cal_year, year_buf, 8);
    
    title[0] = '\0';
    strcat(title, "Month: ");
    strcat(title, month_buf);
    strcat(title, "/");
    strcat(title, year_buf);
    
    gui_draw_string(100, 60, title, COLOR_BLACK);
    
    // Draw simple grid
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 7; col++) {
            int day = row * 7 + col + 1;
            if (day <= 31) {
                char day_str[4];
                int_to_str(day, day_str, 4);
                gui_draw_string(45 + col * 30, 80 + row * 12, day_str, COLOR_YELLOW);
            }
        }
    }
    
    Point mouse = {g_gui_state.mouse_x, g_gui_state.mouse_y};
    Rect prev_btn = {40, 150, 40, 15};
    Rect next_btn = {240, 150, 40, 15};
    gui_draw_button(prev_btn, "Prev", gui_is_point_in_rect(mouse, prev_btn));
    gui_draw_button(next_btn, "Next", gui_is_point_in_rect(mouse, next_btn));
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// NOTEPAD APPLICATION
// ======================
#define NOTEPAD_MAX_TEXT 256
static char notepad_text[NOTEPAD_MAX_TEXT];
static int notepad_cursor = 0;

void app_notepad_init(void) {
    notepad_text[0] = '\0';
    notepad_cursor = 0;
}

void app_notepad_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    if (gui_button_clicked(close_btn)) gui_switch_app(APP_DESKTOP);
    
    // Handle keyboard input
    if (keyboard_has_key()) {
        KeyEvent key = keyboard_get_key();
        if (key.ascii && notepad_cursor < NOTEPAD_MAX_TEXT - 1) {
            if (key.ascii == '\b') {
                if (notepad_cursor > 0) {
                    notepad_cursor--;
                    notepad_text[notepad_cursor] = '\0';
                }
            } else if (key.ascii == '\n') {
                notepad_text[notepad_cursor++] = '\n';
                notepad_text[notepad_cursor] = '\0';
            } else {
                notepad_text[notepad_cursor++] = key.ascii;
                notepad_text[notepad_cursor] = '\0';
            }
        }
    }
}

void app_notepad_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(20, 20, 280, 140, "Notepad");
    
    // Draw text area with 3D border
    vga_fill_rect(25, 35, 270, 100, COLOR_WHITE);
    vga_draw_line(25, 35, 295, 35, WIN_BORDER_LIGHT);
    vga_draw_line(25, 35, 25, 135, WIN_BORDER_LIGHT);
    vga_draw_line(25, 135, 295, 135, WIN_BORDER_DARK);
    vga_draw_line(295, 35, 295, 135, WIN_BORDER_DARK);
    
    gui_draw_string(28, 38, notepad_text, COLOR_BLACK);
    
    // Draw cursor
    int cursor_x = 28 + ((notepad_cursor % 33) * 8);
    int cursor_y = 38 + ((notepad_cursor / 33) * 9);
    vga_draw_line(cursor_x, cursor_y, cursor_x, cursor_y + 7, COLOR_BLACK);
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// SNAKE GAME
// ======================
#define SNAKE_MAX_LENGTH 100
#define SNAKE_GRID_SIZE 10

typedef struct {
    int x, y;
} SnakeSegment;

static SnakeSegment snake_body[SNAKE_MAX_LENGTH];
static int snake_length = 3;
static int snake_dir_x = 1;
static int snake_dir_y = 0;
static int snake_food_x = 15;
static int snake_food_y = 10;
static uint32_t snake_last_move = 0;
static bool snake_game_over = false;

void app_snake_init(void) {
    snake_length = 3;
    snake_body[0].x = 10; snake_body[0].y = 10;
    snake_body[1].x = 9;  snake_body[1].y = 10;
    snake_body[2].x = 8;  snake_body[2].y = 10;
    snake_dir_x = 1;
    snake_dir_y = 0;
    snake_food_x = 15;
    snake_food_y = 10;
    snake_game_over = false;
}

void app_snake_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    if (gui_button_clicked(close_btn)) gui_switch_app(APP_DESKTOP);
    
    // Restart game on 'R' key press when game over
    if (snake_game_over) {
        if (keyboard_has_key()) {
            KeyEvent key = keyboard_get_key();
            if (key.ascii == 'r' || key.ascii == 'R') {
                app_snake_init();
            }
        }
        return;
    }
    
    // Handle keyboard input
    if (keyboard_has_key()) {
        KeyEvent key = keyboard_get_key();
        if (key.scancode == KEY_UP && snake_dir_y == 0) {
            snake_dir_x = 0; snake_dir_y = -1;
        } else if (key.scancode == KEY_DOWN && snake_dir_y == 0) {
            snake_dir_x = 0; snake_dir_y = 1;
        } else if (key.scancode == KEY_LEFT && snake_dir_x == 0) {
            snake_dir_x = -1; snake_dir_y = 0;
        } else if (key.scancode == KEY_RIGHT && snake_dir_x == 0) {
            snake_dir_x = 1; snake_dir_y = 0;
        }
    }
    
    // Move snake every 10 ticks
    if (g_gui_state.ticks - snake_last_move > 10) {
        snake_last_move = g_gui_state.ticks;
        
        // Move body
        for (int i = snake_length - 1; i > 0; i--) {
            snake_body[i] = snake_body[i-1];
        }
        
        // Move head
        snake_body[0].x += snake_dir_x;
        snake_body[0].y += snake_dir_y;
        
        // Check bounds
        if (snake_body[0].x < 0 || snake_body[0].x >= 28 ||
            snake_body[0].y < 0 || snake_body[0].y >= 15) {
            snake_game_over = true;
        }
        
        // Check self collision
        for (int i = 1; i < snake_length; i++) {
            if (snake_body[0].x == snake_body[i].x && 
                snake_body[0].y == snake_body[i].y) {
                snake_game_over = true;
            }
        }
        
        // Check food
        if (snake_body[0].x == snake_food_x && snake_body[0].y == snake_food_y) {
            if (snake_length < SNAKE_MAX_LENGTH) {
                snake_length++;
                // Add new segment at the tail
                snake_body[snake_length - 1] = snake_body[snake_length - 2];
            }
            // Place food in a new random location, avoiding snake body
            int attempts = 0;
            bool food_placed = false;
            do {
                snake_food_x = (g_gui_state.ticks * 7 + snake_length + attempts) % 28;
                snake_food_y = (g_gui_state.ticks * 13 + snake_length * 3 + attempts * 5) % 15;
                
                // Check if food position overlaps with snake
                food_placed = true;
                for (int i = 0; i < snake_length; i++) {
                    if (snake_body[i].x == snake_food_x && snake_body[i].y == snake_food_y) {
                        food_placed = false;
                        break;
                    }
                }
                attempts++;
            } while (!food_placed && attempts < 100); // Safety limit
            
            // If we couldn't find a free spot, just place it randomly
            if (!food_placed) {
                snake_food_x = (g_gui_state.ticks * 7) % 28;
                snake_food_y = (g_gui_state.ticks * 13) % 15;
            }
        }
    }
}

void app_snake_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(10, 10, 300, 160, "Snake Game");
    
    // Draw game area
    vga_fill_rect(15, 25, 280, 140, COLOR_BLACK);
    
    if (snake_game_over) {
        gui_draw_string(110, 80, "GAME OVER!", COLOR_RED);
    } else {
        // Draw snake
        for (int i = 0; i < snake_length; i++) {
            uint8_t color = (i == 0) ? COLOR_YELLOW : COLOR_GREEN;
            vga_fill_rect(15 + snake_body[i].x * SNAKE_GRID_SIZE,
                         25 + snake_body[i].y * SNAKE_GRID_SIZE,
                         SNAKE_GRID_SIZE - 1, SNAKE_GRID_SIZE - 1, color);
        }
        
        // Draw food
        vga_fill_rect(15 + snake_food_x * SNAKE_GRID_SIZE,
                     25 + snake_food_y * SNAKE_GRID_SIZE,
                     SNAKE_GRID_SIZE - 1, SNAKE_GRID_SIZE - 1, COLOR_RED);
    }
    
    // Draw score
    char score_str[32] = "Score: ";
    char score_num[8];
    int_to_str(snake_length - 3, score_num, 8);
    strcat(score_str, score_num);
    gui_draw_string(220, 170, score_str, COLOR_BLACK);
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// SETTINGS APPLICATION
// ======================
void app_settings_init(void) {
}

void app_settings_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    if (gui_button_clicked(close_btn)) gui_switch_app(APP_DESKTOP);
}

void app_settings_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(30, 30, 260, 120, "Settings");
    
    gui_draw_string(50, 60, "Display Mode: 320x200", COLOR_BLACK);
    gui_draw_string(50, 75, "Colors: 256", COLOR_BLACK);
    gui_draw_string(50, 90, "Input: PS/2 Keyboard", COLOR_BLACK);
    gui_draw_string(50, 105, "Mouse: PS/2 Mouse", COLOR_BLACK);
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// SYSTEM MONITOR APPLICATION
// ======================
void app_sysmon_init(void) {
}

void app_sysmon_update(void) {
    Rect close_btn = {280, 32, 8, 8};
    if (gui_button_clicked(close_btn)) gui_switch_app(APP_DESKTOP);
}

void app_sysmon_draw(void) {
    vga_fill_rect(0, 0, 320, 180, WIN_BG_COLOR);
    gui_draw_window(20, 20, 280, 140, "System Monitor");
    
    char uptime_str[64] = "Uptime: ";
    char ticks_buf[16];
    int_to_str(g_gui_state.ticks / 18, ticks_buf, 16);
    strcat(uptime_str, ticks_buf);
    strcat(uptime_str, "s");
    
    gui_draw_string(30, 50, uptime_str, COLOR_BLACK);
    
    gui_draw_string(30, 65, "Memory Regions:", COLOR_YELLOW);
    
    int y = 80;
    for (int i = 0; i < g_BootParams->Memory.RegionCount && i < 3; i++) {
        char mem_str[64] = "Region ";
        char num_buf[16];
        int_to_str(i, num_buf, 16);
        strcat(mem_str, num_buf);
        strcat(mem_str, ": ");
        
        uint32_t size_kb = g_BootParams->Memory.Regions[i].Length / 1024;
        int_to_str(size_kb, num_buf, 16);
        strcat(mem_str, num_buf);
        strcat(mem_str, " KB");
        
        gui_draw_string(35, y, mem_str, COLOR_BLACK);
        y += 12;
    }
    
    // Draw taskbar
    vga_fill_rect(0, 180, 320, 20, WIN_TASKBAR_BG);
    vga_draw_line(0, 180, 320, 180, WIN_BORDER_LIGHT);
    gui_draw_string(5, 185, "Ready", COLOR_WHITE);
}

// ======================
// MAIN GUI FUNCTIONS
// ======================
void gui_init(void) {
    g_gui_state.current_app = APP_DESKTOP;
    g_gui_state.ticks = 0;
    g_gui_state.mouse_x = 160;
    g_gui_state.mouse_y = 100;
    g_gui_state.old_mouse_x = 160;
    g_gui_state.old_mouse_y = 100;
    app_desktop_init();
}

void gui_update(void) {
    // Wait for vertical retrace before drawing to prevent flickering
    vga_wait_vsync();

    // Restore the screen area at the old cursor position
    for (int y = 0; y < CURSOR_HEIGHT; y++) {
        for (int x = 0; x < CURSOR_WIDTH; x++) {
            vga_put_pixel(g_gui_state.old_mouse_x + x, g_gui_state.old_mouse_y + y, g_gui_state.cursor_buffer[y * CURSOR_WIDTH + x]);
        }
    }

    // Update mouse state
    g_gui_state.mouse_was_clicked = g_gui_state.mouse_clicked;
    g_gui_state.mouse_x = mouse_x;
    g_gui_state.mouse_y = mouse_y;
    
    // Update click state based on actual mouse button
    g_gui_state.mouse_clicked = mouse_left_button;
    
    g_gui_state.ticks++;
    
    // Update current app
    switch (g_gui_state.current_app) {
        case APP_DESKTOP: app_desktop_update(); break;
        case APP_CLOCK: app_clock_update(); break;
        case APP_CALENDAR: app_calendar_update(); break;
        case APP_NOTEPAD: app_notepad_update(); break;
        case APP_SNAKE: app_snake_update(); break;
        case APP_SETTINGS: app_settings_update(); break;
        case APP_SYSMON: app_sysmon_update(); break;
        default: break;
    }
    
    // Draw current app
    switch (g_gui_state.current_app) {
        case APP_DESKTOP: app_desktop_draw(); break;
        case APP_CLOCK: app_clock_draw(); break;
        case APP_CALENDAR: app_calendar_draw(); break;
        case APP_NOTEPAD: app_notepad_draw(); break;
        case APP_SNAKE: app_snake_draw(); break;
        case APP_SETTINGS: app_settings_draw(); break;
        case APP_SYSMON: app_sysmon_draw(); break;
        default: break;
    }
    
    // Save the screen area at the new cursor position
    for (int y = 0; y < CURSOR_HEIGHT; y++) {
        for (int x = 0; x < CURSOR_WIDTH; x++) {
            g_gui_state.cursor_buffer[y * CURSOR_WIDTH + x] = vga_get_pixel(g_gui_state.mouse_x + x, g_gui_state.mouse_y + y);
        }
    }

    // Draw mouse cursor last (so it's always visible)
    draw_cursor(g_gui_state.mouse_x, g_gui_state.mouse_y, COLOR_BLACK);

    // Update old mouse position
    g_gui_state.old_mouse_x = g_gui_state.mouse_x;
    g_gui_state.old_mouse_y = g_gui_state.mouse_y;
}
