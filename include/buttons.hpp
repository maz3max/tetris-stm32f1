#pragma once

#include <cstdlib>

// initializes the button GPIO pins
void btn_init();
// determine whether that particular button is pressed
bool btn_pressed (size_t num);

#define NUM_BTNS 5

