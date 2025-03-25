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

__xdata int16_t Ps2MouseScalingTable[] = {-9, -6, -3, -1, -1, 0, 1, 1, 3, 6, 9};

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
    m->NeedsUpdating = 1;
}


void GetMouseAxisUpdate(MOUSE *m, int16_t* Axis, int16_t* Value, int16_t Min, int16_t Max, uint8_t Downscale) 
{	
	// assume update value won't exceed min/max limit
	*Value = *Axis >> Downscale;
	
	// but if it does then cap to limit 
	if (*Value < Min)
	{
		*Value = Min;
		m->NeedsUpdating = 1;
	}
	else if (*Value > Max)
	{
		*Value = Max;
		m->NeedsUpdating = 1;
	}	
	
	// decrease delta by update value (delta is zeroed if limits were not exceeded, otherwise we have leftovers)
	// note that we can do power of two downscaling just by two bit shifts, no floating point maths needed
	*Axis -= *Value << Downscale;	
}

uint8_t GetMouseUpdate(int16_t Min, int16_t Max, int16_t *X, int16_t *Y, int16_t *Z, uint8_t *Buttons, bool Accelerate, uint8_t Downscale)
{
    MOUSE *m = &OutputMice;
	
	if (m->NeedsUpdating)
    {
        // assume it doesn't need updating after this, but can change if deltas exceeds min/max limit
        m->NeedsUpdating = 0;
		
		// get deltas for x and y (notice downscaling, this is for ps2 mouse resolution but would work with serial as well)
		GetMouseAxisUpdate(m, &m->DeltaX, X, Min, Max, Downscale);
		GetMouseAxisUpdate(m, &m->DeltaY, Y, Min, Max, Downscale);
		
		// get delta for z also for ps2 intellimouse 
		GetMouseAxisUpdate(m, &m->DeltaZ, Z, -8, 7, 0);

		// get buttons
        *Buttons = m->Buttons;
		
		// apply acceleration (this is for ps2 mouse 2:1 scaling support but in theory could be used with serial mouse)
		if (Accelerate)
		{
			*X = (abs(*X) < 6 ? Ps2MouseScalingTable[(*X)+5] : (*X)*2);
			*Y = (abs(*Y) < 6 ? Ps2MouseScalingTable[(*Y)+5] : (*Y)*2);
		}
		
        return 1;
    }
    else
        return 0;
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

__xdata MOUSE *ps2Mouse = &OutputMice;

void HandleMouse(void) {
	
		int16_t X, Y, Z;
		uint8_t byte1, byte2, byte3, byte4;
		uint8_t Buttons;

		{
			if (GetMouseUpdate(-255, 255, &X, &Y, &Z, &Buttons, false/*(ps2Mouse->Ps2Scaling==MOUSE_PS2_SCALING_2X)*/, 0/*(3-ps2Mouse->Ps2Resolution)*/))
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


				{
					byte4 = (-Z & 0xFF);
					mouse_set_state(byte1, byte2, byte3, byte4);
				}
			}
		}
}
