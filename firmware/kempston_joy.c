#include <stdint.h>

#include "ch559.h"
#include "type.h"
#include "system.h"
#include "gpio.h"

#define DI_PORT 0
// 1.6 are Kempston joystick JOY strobe output
#define JOY_PORT 1
#define JOY_PIN 6

SBIT(MJOY, PADR(JOY_PORT), JOY_PIN);

#define DI_BUS_SET_DELAY 4
#define REGISTER_SET_DELAY 4

#define _delay_us mDelayuS

volatile __data uint8_t joy_state = 0x00;

void kempston_joy_init()
{
	// Assume DI_PORT set to PIN_MODE_OUTPUT in ps2_mouse_init_registers()

	pinMode(JOY_PORT, JOY_PIN, PIN_MODE_OUTPUT);
}

void kempston_joy_set(uint8_t state)
{
	joy_state = state;
}

void kempston_joy_update()
{
	// Set value to Kempston Joystick controller register.
	// Data written to register on rising_edge.
	// Assume buttons register bits active level is high.
	MJOY = 0;
	PORT(DI_PORT) = joy_state;
	_delay_us(DI_BUS_SET_DELAY);
	MJOY = 1;
	_delay_us(REGISTER_SET_DELAY);
	MJOY = 0;
}
