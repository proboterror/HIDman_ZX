/*
	ps2.c
	
	Handles the low-level parts of the PS/2 protocol
	I.e. buffers and bit-bang state machine
*/

#include <stdint.h>
#include "ch559.h"
#include "data.h"
#include "system.h"
#include "scancode.h"

void InitPS2Ports(void)
{
}

bool SendKeyboard(const uint8_t *chunk)
{
	// reset watchdog timer, this routine blocks. It shouldn't really
	SoftWatchdog = 0;
	
	TR0 = 0; //disable timer0  so send is not disabled while we're in the middle of buffer shuffling

	TR0 = 1; // re-enable timer interrupt
	return 0;
}


void PressKey(uint8_t currchar)
{
    while (!SendKeyboard(HIDtoSET2_Make[ASCIItoHID[currchar]]));
}

void ReleaseKey(uint8_t currchar)
{
    while (!SendKeyboard(HIDtoSET2_Break[ASCIItoHID[currchar]]));
}