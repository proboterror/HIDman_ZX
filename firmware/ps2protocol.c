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
#include "system.h"
#include "scancode.h"
#include "ps2_keyboard.h"
#include "kempston_joy.h"

#define SetKey(key,report) (report->KeyboardKeyMap[key >> 3] |= 1 << (key & 0x07))

#define GetOldKey(key,report) (report->oldKeyboardKeyMap[key >> 3] & (1 << (key & 0x07)))

#define map_to_uint8(value, min, max) (uint8_t)((((value - min) * 255) + ((max - min) >> 1)) / (max - min))

void processSeg(__xdata HID_SEG *currSeg, __xdata HID_REPORT *report, __xdata uint8_t *data)
{
	if (currSeg->InputType == MAP_TYPE_BITFIELD)
	{
		const uint16_t endbit = currSeg->startBit + currSeg->reportCount;
		uint8_t tmp = currSeg->OutputControl;
		for (uint16_t cnt = currSeg->startBit; cnt < endbit; cnt++)
		{
			uint8_t pressed = 0;

			// find byte
			const uint8_t *currByte = data + ((cnt) >> 3);

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
			else if (currSeg->OutputChannel == MAP_MOUSE)
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
		uint16_t currentBit = currSeg->startBit;

		for (uint8_t i = 0; i < currSeg->reportSize; i++)
		{
			// Calculate byte position and bit offset within the byte
			const uint8_t shiftbits = currentBit % 8;
			const uint8_t startbyte = currentBit / 8;
	
			// Extract the bit
			const uint8_t bit = (data[startbyte] >> shiftbits) & 0x01;
	
			// Add the bit to the result
			value |= (uint32_t)bit << i;
	
			currentBit++;
		}

		const bool sign = currSeg->logicalMinimum < 0;
		// if it's a signed integer we need to extend the sign
		if (sign)
		{
			value = SIGNEX(value, currSeg->reportSize - 1);
		}

		if (currSeg->OutputChannel == MAP_KEYBOARD)
			report->keyboardUpdated = 1;

		if (currSeg->OutputChannel == MAP_KEMPSTON)
			report->joystickUpdated = 1;

		bool make = 0;

		// Received HID value should be re-interpreted depending on Logical Minimum / Logical Maximum and Report Size in USB HID report descriptor.
		// Example:
		// Minimum 0xFF (-1), Maximum 0x01 (1), Report Size 0x08: value is signed int8_t in -1..1 range.
		// Minimum 0x81 (-127), Maximum 0x7F (127), Report Size 0x08: value is signed int8_t in -127..127 range.
		// Minimum 0x00 (0), Maximum 0xFF (255), Report Size 0x08: value is signed uint8_t in 0..255 range.

		// Note: Logical Minimum/Maximum can describe 16/32-bit ranges but require extended encoding
		// (0x81 for 16-bit, 0x82 for 32-bit), can be checked with Report Size.

		// Note: InputParam field in JoyPreset user presets is written assuming REPORT_USAGE_X / Y / Z / Rz
		// are unsigned 8-bit values in 0..255 range.
		// In practice they can be signed in -1..1 range for example, or 0..12000 / 0..65535 / -32768..32767 range with 16-bit ReportSize, or even 32-bit.
		
		// Map received value to 0..255 range depending on Logical Minimum / Logical Maximum.
		// ToDo: only map values with currSeg->InputUsage == REPORT_USAGE_X/Y
		if (currSeg->InputType == MAP_TYPE_THRESHOLD_ABOVE)
		{
			if(sign)
			{
				const uint8_t mapped_value = map_to_uint8((int32_t)value, currSeg->logicalMinimum, currSeg->logicalMaximum);

				if (mapped_value > currSeg->InputParam)
					make = 1;
			}
			else
			{

				const uint8_t mapped_value = map_to_uint8(value, currSeg->logicalMinimum, currSeg->logicalMaximum);

				if (mapped_value > currSeg->InputParam)
					make = 1;
			}
		}
		else if (currSeg->InputType == MAP_TYPE_THRESHOLD_BELOW)
		{
			if(sign)
			{
				const uint8_t mapped_value = map_to_uint8((int32_t)value, currSeg->logicalMinimum, currSeg->logicalMaximum);

				if (mapped_value < currSeg->InputParam)
					make = 1;
			}
			else
			{
				const uint8_t mapped_value = map_to_uint8(value, currSeg->logicalMinimum, currSeg->logicalMaximum);

				if (mapped_value < currSeg->InputParam)
					make = 1;
			}
		}
		else if (currSeg->InputType == MAP_TYPE_EQUAL)
		{
			if (value == currSeg->InputParam)
				make = 1;
		}
		
		if (make)
		{
			if (currSeg->OutputChannel == MAP_KEYBOARD)
			{
				SetKey(currSeg->OutputControl, report);
			}

			if (currSeg->OutputChannel == MAP_KEMPSTON)
			{
				report->joystickState |= (1 << currSeg->OutputControl);
			}
		}
		else if (currSeg->InputType == MAP_TYPE_SCALE)
		{
			if (currSeg->OutputChannel == MAP_MOUSE)
			{
				switch (currSeg->OutputControl)
				{
				// TODO scaling
				case MAP_MOUSE_X:
						MouseMove((int32_t)value, 0, 0);
					break;
				case MAP_MOUSE_Y:
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
	// Kempston joystick state bits will be set if Kempston joystick mappings assigned to current report triggered.
	descReport->joystickState = 0;

	while (currSegNode != NULL)
	{
		processSeg((__xdata HID_SEG *)(currSegNode->data), descReport, report);
		currSegNode = currSegNode->next;
	}

	if(descReport->joystickUpdated)
	{
		kempston_joy_set(descReport->joystickState);
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
		descReport->joystickUpdated = 0;
	}

	return 1;
}