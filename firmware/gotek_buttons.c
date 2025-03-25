#include <stdint.h>

#include "ch559.h"
#include "type.h"
#include "system.h"
#include "gpio.h"

#include "gotek_buttons.h"

#define GOTEK_BUTTONS_PORT 2
#define GOTEK_LEFT_PIN 0
#define GOTEK_RIGHT_PIN 1
#define GOTEK_SELECT_PIN 2

SBIT(GOTEK_LEFT, PADR(GOTEK_BUTTONS_PORT), GOTEK_LEFT_PIN);
SBIT(GOTEK_RIGHT, PADR(GOTEK_BUTTONS_PORT), GOTEK_RIGHT_PIN);
SBIT(GOTEK_SELECT, PADR(GOTEK_BUTTONS_PORT), GOTEK_SELECT_PIN);

void gotek_buttons_init()
{
	// Set control pins to output (push pull).
	// Note: CH559 / i8051 port mode set for all port bits.
	pinMode(GOTEK_BUTTONS_PORT, GOTEK_LEFT_PIN, PIN_MODE_OUTPUT);
	pinMode(GOTEK_BUTTONS_PORT, GOTEK_RIGHT_PIN, PIN_MODE_OUTPUT);
	pinMode(GOTEK_BUTTONS_PORT, GOTEK_SELECT_PIN, PIN_MODE_OUTPUT);

	// GOTEK buttons are pulled up (0 if pressed).
	GOTEK_LEFT = GOTEK_RIGHT = GOTEK_SELECT = 1;
}

void set_gotek_buttons(const uint8_t buttons)
{
	GOTEK_LEFT = !(buttons & GOTEK_BUTTON_LEFT);
	GOTEK_RIGHT = !(buttons & GOTEK_BUTTON_RIGHT);
	GOTEK_SELECT = !(buttons & GOTEK_BUTTON_SELECT);
}