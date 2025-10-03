#include <stdint.h>

#include "ch559.h"
#include "type.h"
#include "system.h"
#include "gpio.h"

/*
	3-button Sega Mega Drive / Genesis controller interface.

	References:
	https://github.com/jonthysell/SegaController/wiki/How-To-Read-Sega-Controllers

	https://www.raspberryfield.life/2019/03/25/sega-mega-drive-genesis-6-button-xyz-controller/
	https://segaretro.org/Control_Pad_(Mega_Drive)
	https://segaretro.org/Six_Button_Control_Pad_(Mega_Drive)
	https://segaretro.org/File:Genesis_Software_Manual.pdf
*/

// Note: pad expects external pullups on data line.
// ToDo: check if port input 5V tolerant
#define PAD_DATA_PORT P4_IN

#define SELECT_PORT 2
#define SELECT_PIN 3

SBIT(SELECT, PADR(SELECT_PORT), SELECT_PIN);

#define SMD1_DATA_PIN0 0
#define SMD1_DATA_PIN1 1
#define SMD1_DATA_PIN2 2
#define SMD1_DATA_PIN3 3
#define SMD1_DATA_PIN4 4
#define SMD1_DATA_PIN5 5

union smd_pad
{
	struct
	{
		// Note: field bits order intended to match MAP_KEMPSTON_* defines
		uint8_t right : 1,
		left : 1, 
		down : 1, 
		up : 1, 
		b : 1, // Button B as primary button
		a : 1, 
		c : 1, 
		start : 1; 
	};
	uint8_t state;
};

//static_assert(sizeof(smd_state) == sizeof(uint8_t)); 

/*volatile*/ __data union smd_pad pad;

void sega_pad_init()
{
	// Set select pin to output.
	// Note: CH559 / i8051 port mode set for all port bits.
	// Use open collector mode because port shared with GOTEK buttons, enable internal pullup.
	pinMode(SELECT_PORT, SELECT_PIN, PIN_MODE_INPUT_OUTPUT_PULLUP);

	SELECT = 0;

	// pinMode function not applicable to P4 port
	P4_DIR = 0b00000000; // Set bits 0-5 to Sega controller data input
	P4_PU = 0b00111111; // Enable internal pullups for data lines

	pad.state = 0;
}

// Read 3-button Sega Mega Drive / Genesis controller state.
// 6-button controllers additional buttons are ignored.
void sega_pad_update()
{
/*
    Reading the 3-Button Controller:
    - Output LOW to the select pin.
    - Read the six input pins according to the table (LOW = the button is being pressed).
    - Output HIGH to the select pin.
    - Read the six input pins according to the table (LOW = the button is being pressed).

     SEL out D5 in  D4 in  D3 in  D2 in  D1 in  D0 in
     LO      Start  A      0      0      Down   Up    (D2 and D3 == LOW indicate the presence of a controller)
     HI      C      B      Right  Left   Down   Up
*/
	// SELECT expected to be set to low in init / after state read/update

	// Check if a controller is connected
	bool connected = (((PAD_DATA_PORT>>SMD1_DATA_PIN2) & 1) == 0) &&
					 (((PAD_DATA_PORT>>SMD1_DATA_PIN3) & 1) == 0);

	// Read input pins for A and Start
	if(connected)
	{
			pad.a = !((PAD_DATA_PORT>>SMD1_DATA_PIN4) & 1);
			pad.start = !((PAD_DATA_PORT>>SMD1_DATA_PIN5) & 1);
	}

	SELECT = 1;
/*
	Genesis Software Manual (C) 1989 Sega of Japan:
	Genesis Technical Bulletin #27 January 24, 1994:

	1. Warning regarding control pad data reads

	For both the 3 and 6 button pads, the pad data is determined 2usec after TH is modified.
	Therefore, as shown in the sample below, read data from the pad 2usec (4 nop's) after TH is modified.

	Joypad Reads

	Pad data from the 3 and 6 button controllers are read 2usec after TH is modified.
	The wait is necessary because the data in the chip needs time to stabilize after TH is modified.
	If data is read without this wait, there is no guarantee that the data will be correct.

	Moreover, the 2usec time is equivalent to 4 nop, including the 68000's prefetch.
*/
	mDelayuS(2); // Short delay to stabilise outputs in controller.

	if(connected)
	{
		// Read input pins for Up, Down, Left, Right, B, C
		pad.b = !((PAD_DATA_PORT>>SMD1_DATA_PIN4) & 1);
		pad.c = !((PAD_DATA_PORT>>SMD1_DATA_PIN5) & 1);

		pad.up = !((PAD_DATA_PORT>>SMD1_DATA_PIN0) & 1);
		pad.down = !((PAD_DATA_PORT>>SMD1_DATA_PIN1) & 1);
		pad.left = !((PAD_DATA_PORT>>SMD1_DATA_PIN2) & 1);
		pad.right = !((PAD_DATA_PORT>>SMD1_DATA_PIN3) & 1);
	}
	else // No Mega Drive controller is connected
	{
		// Clear current state
		pad.state = 0;
	}

	SELECT = 0;
}

uint8_t get_sega_pad_state()
{
	return pad.state;
}