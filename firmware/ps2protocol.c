/*
	ps2protocol.c
	
	Handles the higher-level parts of the PS/2 protocol
	HID conversion, responding to host commands
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "ch559.h"
#include "usbhost.h"
#include "ps2protocol.h"
#include "mouse.h"
#include "keyboardled.h"
#include "system.h"
#include "scancode.h"
#include "ps2_keyboard.h"

#define SetKey(key,report) (report->KeyboardKeyMap[key >> 3] |= 1 << (key & 0x07))

#define GetOldKey(key,report) (report->oldKeyboardKeyMap[key >> 3] & (1 << (key & 0x07)))

__code uint8_t bitMasks[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1F, 0x3F, 0x7F, 0xFF};

void processSeg(__xdata HID_SEG *currSeg, __xdata HID_REPORT *report, __xdata uint8_t *data)
{
	bool make = 0;
	uint8_t tmp = 0;
	uint16_t cnt, endbit;
	uint8_t *currByte;
	uint8_t pressed = 0;
	int16_t tmpl;

	if (currSeg->InputType == MAP_TYPE_BITFIELD)
	{

		endbit = currSeg->startBit + currSeg->reportCount;
		tmp = currSeg->OutputControl;
		for (cnt = currSeg->startBit; cnt < endbit; cnt++)
		{

			pressed = 0;

			// find byte
			currByte = data + ((cnt) >> 3);

			// find bit
			if (*currByte & (0x01 << (cnt & 0x07)))
				pressed = 1;

			if (currSeg->OutputChannel == MAP_KEYBOARD)
			{
				if (pressed)
				{
					SetKey(tmp, report);
					if (!GetOldKey(tmp, report)) {
						report->keyboardUpdated = 1;
					}
				}
				else
				{
					
					if (GetOldKey(tmp, report)) {
						report->keyboardUpdated = 1;
					}
				}
			}
			else
			{
				switch (tmp)
				{
				case MAP_MOUSE_BUTTON1:
					MouseSet(0, pressed);
					break;
				case MAP_MOUSE_BUTTON2:
					MouseSet(1, pressed);
					break;
				case MAP_MOUSE_BUTTON3:
					MouseSet(2, pressed);
					break;
				case MAP_MOUSE_BUTTON4:
					MouseSet(3, pressed);
					break;
				case MAP_MOUSE_BUTTON5:
					MouseSet(4, pressed);
					break;
				}
			}

			tmp++;
		}
	}
	else if (currSeg->InputType) //i.e. not MAP_TYPE_NONE
	{
		uint32_t value = 0;

		// bits may be across any byte alignment
		// so do the neccesary shifting to get it to all fit in a uint32_t
		int8_t shiftbits = -(currSeg->startBit % 8);
		uint8_t startbyte = currSeg->startBit / 8;

        currByte = data + startbyte;
        
		while(shiftbits < currSeg->reportSize) {
        
			if (shiftbits < 0)
				value |= ((uint32_t)(*currByte)) >> (uint32_t)(-shiftbits);
			else
				value |= ((uint32_t)(*currByte)) << (uint32_t)shiftbits;
            
            currByte++;
			shiftbits += 8;
		}

		// if it's a signed integer we need to extend the sign
		// todo, actually determine if it is a signed int... look at logical max/min fields in descriptor
		if (currSeg->InputParam & INPUT_PARAM_SIGNED)
			value = SIGNEX(value, currSeg->reportSize - 1);

		

		//old way, not significantly faster anymore
		//currByte = data + (currSeg->startBit >> 3);
		//currSeg->value = ((*currByte) >> (currSeg->startBit & 0x07)) // shift bits so lsb of this seg is at bit zero
		//				 & bitMasks[currSeg->reportSize];			 // mask off the bits according to seg size

		//printf("x:%lx\n", currSeg->value);

		if (currSeg->OutputChannel == MAP_KEYBOARD)
			report->keyboardUpdated = 1;

		if (currSeg->InputType == MAP_TYPE_THRESHOLD_ABOVE && value > currSeg->InputParam)
		{
			make = 1;
		}
		else if (currSeg->InputType == MAP_TYPE_THRESHOLD_BELOW && value < currSeg->InputParam)
		{
			make = 1;
		}
		else if (currSeg->InputType == MAP_TYPE_EQUAL && value == currSeg->InputParam)
		{
			make = 1;
		}

		else
		{
			make = 0;
		}
		// hack for mouse, as it needs to explicity switch on and off
		// this needs rewritten
		if (currSeg->OutputChannel == MAP_MOUSE && currSeg->InputType == MAP_TYPE_THRESHOLD_ABOVE) {
			switch (currSeg->OutputControl)
				{
				case MAP_MOUSE_BUTTON1:
					MouseSet(0, value);
					break;
				case MAP_MOUSE_BUTTON2:
					MouseSet(1, value);
					break;
				case MAP_MOUSE_BUTTON3:
					MouseSet(2, value);
					break;
				case MAP_MOUSE_BUTTON4:
					MouseSet(3, value);
					break;
				case MAP_MOUSE_BUTTON5:
					MouseSet(4, value);
					break;
				}
		}
		else if (make)
		{
			if (currSeg->OutputChannel == MAP_KEYBOARD)
			{
				SetKey(currSeg->OutputControl, report);
			}
		}
		else if (currSeg->InputType == MAP_TYPE_SCALE)
		{
			if (currSeg->OutputChannel == MAP_MOUSE)
			{

				#define DEADZONE 1

				switch (currSeg->OutputControl)
				{
				// TODO scaling
				case MAP_MOUSE_X:
					if (currSeg->InputParam == INPUT_PARAM_SIGNED_SCALEDOWN){

						tmpl = ((int8_t)((value + 8) >> 4) - 0x08);

						// deadzone
						if (tmpl <= -DEADZONE) tmpl+= DEADZONE;
						else if (tmpl >= DEADZONE) tmpl-= DEADZONE;
						else tmpl = 0;
						
						MouseMove(tmpl, 0, 0);
					}
					else
						MouseMove((int32_t)value, 0, 0);

					break;
				case MAP_MOUSE_Y:
					if (currSeg->InputParam == INPUT_PARAM_SIGNED_SCALEDOWN) {

						tmpl = ((int8_t)((currSeg->value + 8) >> 4) - 0x08);

						// deadzone
						if (tmpl <= -DEADZONE) tmpl+= DEADZONE;
						else if (tmpl >= DEADZONE) tmpl-= DEADZONE;
						else tmpl = 0;

						MouseMove(0, tmpl, 0);
					}
					else
						MouseMove(0, (int32_t)value, 0);

					break;
				case MAP_MOUSE_WHEEL:

					MouseMove(0, 0, (int32_t)value);

					break;
				}
			}
		}
		else if (currSeg->InputType == MAP_TYPE_ARRAY)
		{
			if (currSeg->OutputChannel == MAP_KEYBOARD)
			{
				SetKey(value, report);
			}
		}
	}
}

bool BitPresent(uint8_t *bitmap, uint8_t bit)
{
	if (bitmap[bit >> 3] & (1 << (bit & 0x07)))
		return 1;
	else
		return 0;
}
 
bool ParseReport(__xdata INTERFACE *interface, uint32_t len, __xdata uint8_t *report)
{
	__xdata HID_REPORT *descReport;
	__xdata LinkedList *currSegNode;

	// Turn off LEDs for a while
	setLED(false);

	if (interface->usesReports)
	{
		// first byte of report will be the report number
		descReport = (__xdata HID_REPORT *)ListGetData(interface->Reports, report[0]);
	}
	else
	{
		descReport = (__xdata HID_REPORT *)ListGetData(interface->Reports, 0);
	}
	
	if (descReport == NULL) {
		DEBUGOUT("Invalid report\n");
		return 0;
	}

	// sanity check length - smaller is no good
	if (len < descReport->length)
	{
		DEBUGOUT("report too short - %lu < %u\n", len, descReport->length);
		return 0;
	}

	currSegNode = descReport->HidSegments;

	// clear key map as all pressed keys should be present in report
	memset(descReport->KeyboardKeyMap, 0, 32);

	while (currSegNode != NULL)
	{
		processSeg((__xdata HID_SEG *)(currSegNode->data), descReport, report);
		currSegNode = currSegNode->next;
	}

	if(descReport->keyboardUpdated)
	{
		// for each byte in the report
		for (uint8_t d = 0; d < 32; d++) 
		{
			// XOR to see if any bits are different
			uint8_t xorred = descReport->KeyboardKeyMap[d] ^ descReport->oldKeyboardKeyMap[d];

			if (xorred) {

				for (uint8_t c = 0; c < 8; c++)
				{
					if (xorred & (1 << c)) 
					{
						uint8_t hidcode = (d << 3) | c;

						if (descReport->KeyboardKeyMap[d] & (1 << c)) // set in current but not prev
						{
							{
								//DEBUGOUT("\nSendn %x\n", hidcode);
								// Make

								ps2_add_raw_code(HIDtoSET2_Make[hidcode]);
							}
						}
						else // set in prev but not current
						{
							{
								// break

								//DEBUGOUT("\nBreakn %x\n", hidcode);

								// Pause has no break for some reason
								if (hidcode == 0x48)
									continue;

									ps2_add_raw_code(HIDtoSET2_Break[hidcode]);

							}
						}
					}
					
				}
			}
		}
		memcpy(descReport->oldKeyboardKeyMap, descReport->KeyboardKeyMap, 32);
		descReport->keyboardUpdated = 0;
	}

	return 1;
}