/*
	mouse.c
	
    Keeps track of a particular mouse output channel
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ch559.h"
#include "mouse.h"
#include "ps2_mouse.h"

__xdata MOUSE OutputMice;

void InitMice(void)
{
    memset(&OutputMice, 0x00, sizeof(OutputMice));
}

void MouseMove(int32_t DeltaX, int32_t DeltaY, int32_t DeltaZ)
{
    MOUSE *m = &OutputMice;
    m->DeltaX += DeltaX;
    m->DeltaY += DeltaY;
    m->DeltaZ += DeltaZ;
    m->NeedsUpdating = true;
}

void GetMouseAxisUpdate(MOUSE *m, int16_t* Axis, int16_t* Value, int16_t Min, int16_t Max)
{	
	// assume update value won't exceed min/max limit
	*Value = *Axis;
	
	// but if it does then cap to limit 
	if (*Value < Min)
	{
		*Value = Min;
		m->NeedsUpdating = true;
	}
	else if (*Value > Max)
	{
		*Value = Max;
		m->NeedsUpdating = true;
	}	
	
	// decrease delta by update value (delta is zeroed if limits were not exceeded, otherwise we have leftovers)
	// note that we can do power of two downscaling just by two bit shifts, no floating point maths needed
	*Axis -= *Value;
}

bool GetMouseUpdate(int16_t Min, int16_t Max, int16_t *X, int16_t *Y, int16_t *Z, uint8_t *Buttons)
{
    MOUSE *m = &OutputMice;
	
	if (m->NeedsUpdating)
    {
        // assume it doesn't need updating after this, but can change if deltas exceeds min/max limit
        m->NeedsUpdating = false;
		
		// get deltas for x and y (notice downscaling, this is for ps2 mouse resolution but would work with serial as well)
		GetMouseAxisUpdate(m, &m->DeltaX, X, Min, Max);
		GetMouseAxisUpdate(m, &m->DeltaY, Y, Min, Max);
		
		// get delta for z also for ps2 intellimouse 
		GetMouseAxisUpdate(m, &m->DeltaZ, Z, -8, 7);

		// get buttons
        *Buttons = m->Buttons;

        return true;
    }
    else
        return false;
}

void MouseSet(uint8_t Button, uint8_t value)
{
    MOUSE *m = &OutputMice;
    if (value)
        m->Buttons |= 1 << Button;
    else
        m->Buttons &= ~(1 << Button);
    m->NeedsUpdating = 1;
}

void HandleMouse(void)
{
	int16_t X, Y, Z;
	uint8_t byte1, byte2, byte3, byte4;
	uint8_t Buttons;

	if (GetMouseUpdate(-255, 255, &X, &Y, &Z, &Buttons))
	{
		// ps2 is inverted compared to USB
		Y = -Y;

		// TODO: construct bytes from real state
		byte1 = 0b00001000 |			   //bit3 always set
				((Y >> 10) & 0b00100000) | // Y sign bit
				((X >> 11) & 0b00010000) | // X sign bit
				(Buttons & 0x07);

		byte2 = (X & 0xFF);
		byte3 = (Y & 0xFF);
		byte4 = (-Z & 0xFF);

		mouse_set_state(byte1, byte2, byte3, byte4);
	}
}