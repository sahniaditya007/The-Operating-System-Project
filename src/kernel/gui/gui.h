#pragma once

#include <stdint.h>
#include <stdbool.h>

// GUI Colors (VGA palette)
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GRAY    7
#define COLOR_DARK_GRAY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

typedef struct {
    int x, y, width, height;
} Rect;

typedef struct {
    int x, y;
} Point;

// Application IDs
typedef enum {
    APP_NONE = 0,
    APP_DESKTOP,
    APP_CLOCK,
    APP_CALENDAR,
    APP_NOTEPAD,
    APP_SNAKE,
    APP_SETTINGS,
    APP_SYSMON
} AppID;

#define CURSOR_WIDTH 3
#define CURSOR_HEIGHT 3

// GUI state
typedef struct {
    AppID current_app;
    int mouse_x, mouse_y;
    int old_mouse_x, old_mouse_y;
    bool mouse_clicked;
    bool mouse_was_clicked;
    uint32_t ticks;
    uint8_t cursor_buffer[CURSOR_WIDTH * CURSOR_HEIGHT];
} GUIState;

// GUI functions
void gui_init(void);
void gui_update(void);
void gui_draw_string(int x, int y, const char* str, uint8_t color);
void gui_draw_window(int x, int y, int width, int height, const char* title);
void gui_draw_button(Rect rect, const char* label, bool hover);
bool gui_is_point_in_rect(Point p, Rect r);
bool gui_button_clicked(Rect rect);

// Application interfaces
void app_desktop_init(void);
void app_desktop_update(void);
void app_desktop_draw(void);

void app_clock_init(void);
void app_clock_update(void);
void app_clock_draw(void);

void app_calendar_init(void);
void app_calendar_update(void);
void app_calendar_draw(void);

void app_notepad_init(void);
void app_notepad_update(void);
void app_notepad_draw(void);

void app_snake_init(void);
void app_snake_update(void);
void app_snake_draw(void);

void app_settings_init(void);
void app_settings_update(void);
void app_settings_draw(void);

void app_sysmon_init(void);
void app_sysmon_update(void);
void app_sysmon_draw(void);

void gui_switch_app(AppID app);
