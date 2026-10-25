#include <stdint.h>

#include "ch559.h"
#include "type.h"
#include "system.h"
#include "gpio.h"
#include "defs.h"

#include "scancode.h"
#include "usbhidkeys.h"
#include "ps2_keyboard.h"

#include "sega_pad.h"

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
	MJOY = 0;
}

void kempston_joy_set(uint8_t state)
{
	// Kempston Joystick to HID mapping.
	// Conversion chain: HID key code / state => PS/2 key make / break code => PS/2 keyboard buffer => zx_keyboard_update => CH446Q_set
	// Also can be mapped to much cheaper zx_keyboard => CH446Q_set
	static const uint8_t mapping[8] = {
		KEY_RIGHT,
		KEY_LEFT,
		KEY_DOWN,
		KEY_UP,
		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D
	};

	uint8_t changed = state ^ joy_state;

	joy_state = state;

	for(uint8_t i = 0; i < 8; i++)
	{
		if(changed & 0x01)
		{
			const uint8_t hid_code = mapping[i];
			const bool pressed = state & 0x01;

			if(pressed)
				ps2_add_raw_code(HIDtoSET2_Make[hid_code]);
			else
				ps2_add_raw_code(HIDtoSET2_Break[hid_code]);
		}

		changed >>= 1;
		state >>= 1;
	}
}

void kempston_joy_update()
{
	// Set value to Kempston Joystick controller register.
	// Data written to register on rising_edge.
	// Assume buttons register bits active level is high.
	MJOY = 0;
	PORT(DI_PORT) = joy_state | get_sega_pad_state();
	_delay_us(DI_BUS_SET_DELAY);
	MJOY = 1;
	_delay_us(REGISTER_SET_DELAY);
	MJOY = 0;
}
