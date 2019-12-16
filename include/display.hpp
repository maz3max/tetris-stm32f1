#pragma once

#define DISPLAY_ROWS 16
#define DISPLAY_COLS 8

// initializes the display GPIO pins
void display_init();

// light up a single dot
void draw_dot(uint8_t row, uint8_t col, bool enable);

// light up a dot on both displays
void double_draw_dot(uint8_t row, uint8_t col, bool enable_top,
                     bool enable_bottom);