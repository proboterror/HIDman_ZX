#pragma once

#include <stdint.h>

#define GOTEK_BUTTON_LEFT 1
#define GOTEK_BUTTON_RIGHT 2
#define GOTEK_BUTTON_SELECT 4

void gotek_buttons_init();

void set_gotek_buttons(const uint8_t buttons);