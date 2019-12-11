#pragma once

#include <cstdlib>

// initializes the button GPIO pins
void btn_init();
// determine whether that particular button is pressed
bool btn_pressed(size_t num);

#define NUM_BTNS 5

#define BTN_A 0
#define BTN_B 1
#define BTN_LEFT 2
#define BTN_RIGHT 3
#define BTN_DOWN 4