#include <string.h>

#include "ps2_keyboard.h"

#include "ch559.h"
#include "type.h"
#include "system.h"

// Code originated from murmulator platform (https://murmulator.ru) PS/2 keyboard driver source code (drivers/ps2/ps2.c) with modifications.

volatile __data uint8_t ps2bufsize = 0;
volatile __data uint8_t ps2buffer[KBD_BUFFER_SIZE] /*= {}*/;

bool ps2_add_raw_code(const uint8_t *data)
{
	uint8_t len = *data;

	if(ps2bufsize + len >= KBD_BUFFER_SIZE)
		return false;

	// interrupt INT0 unsafe, also called from USB interrupt
	while(len--)
	{
		ps2buffer[ps2bufsize++] = *(++data);
	}

	return true;
}

uint8_t ps2_get_raw_code(uint8_t *code_0, uint8_t *code_1, uint8_t *code_2)
{
	uint8_t len = 0;

	if (!ps2bufsize)
		return 0;

	switch (ps2buffer[0])
	{
		case 0xF0: // Key with break(up) code: 0xF0, 0xXX
			len = 2;
		case 0xE0: // Extended key with code: 0xEX, 0xXX
		case 0xE1:
			len = 2;

			if(ps2bufsize > 1)
			{
				if(ps2buffer[1] == 0xF0) // Extended key with break(up) code: 0xEX, 0xF0, 0xXX
					len = 3;
			}
			break;
		default:
			len = 1;
			break;
	}

	if (ps2bufsize < len)
		return 0;

	*code_0 = ps2buffer[0];
	
	if (len > 1)
		*code_1 = ps2buffer[1];

	if (len > 2)
		*code_2 = ps2buffer[2];

	// consider if interupt/thread safe? (totally not)
	for (uint32_t i = len; i < KBD_BUFFER_SIZE; i++)
	{
		ps2buffer[i - len] = ps2buffer[i];
	}

	ps2bufsize -= len;

	return len;
}

SBIT(PS2_KEY_CLOCK, 0xB0, 2); // P3.2
SBIT(PS2_KEY_DATA, 0xB0, 4); // P3.4

// See also PS2KeyRaw library: Arduino PS2 keyboard interface by Paul Carpenter
// https://github.com/techpaul/PS2KeyRaw
void ext0_interrupt(void) __interrupt(INT_NO_INT0)
{
	static __data uint8_t bitcount = 0;
	static __data uint8_t incoming = 0;
	static __data uint8_t parity;
	//static uint32_t prev_ms = 0;

	const uint8_t val = PS2_KEY_DATA;
/*
	// Note: This value wraps roughly every 1 hour 11 minutes and 35 seconds.
	const uint32_t now_ms = time_us_32(); // Warning: originally time in ms (milliseconds), not us (microseconds); time_us_64 assigned to uint32_t does not make sense.
	if (now_ms - prev_ms > 250000) // 250 ms bit receive timeout.
	{
		bitcount = 0;
		incoming = 0;
	}

	//prev_ms = now_ms;
*/
	bitcount++;

	switch( bitcount )
	{
	case 1:  // Start bit
		incoming = 0;
		parity = 0;
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9: // Data bits
		parity += val;        // another one received ?
		incoming >>= 1;       // right shift one place for next bit
		incoming |= ( val ) ? 0x80 : 0;    // or in MSbit
		break;
	case 10: // Parity check
		parity &= 1;          // Get LSB if 1 = odd number of 1's so parity should be 0
		if( parity == val )   // Both same parity error
		  parity = 0xFD;      // To ensure at next bit count clear and discard
		break;
	case 11: // Stop bit
		if( parity >= 0xFD )  // had parity error
		{
			// Should send resend byte command here currently discard
		}
		else                  // Good so save byte in buffer
		{
			if (ps2bufsize < KBD_BUFFER_SIZE)
			{
				ps2buffer[ps2bufsize++] = incoming;
			}
		}
		bitcount = 0;
		break;
	default: // in case of weird error and end of byte reception re-sync
		bitcount = 0;
	}	
}

void ps2_keyboard_init(void)
{
	pinMode(1, 6, PIN_MODE_OUTPUT_OPEN_DRAIN);
	pinMode(3, 2, PIN_MODE_OUTPUT_OPEN_DRAIN);
	pinMode(3, 4, PIN_MODE_OUTPUT_OPEN_DRAIN);

	EX0 = 1; // enable external interrupt INT0 or LED interrupt
	IT0 = 1; // INT0 interrupt type: 0=low level action, 1=falling edge action

	memset(ps2buffer, 0, sizeof(ps2buffer));									   
}