#include <stdint.h>

#include "type.h"
#include "system.h" // For DEBUGOUT
#include "usbdef.h"
#include "usbhost.h"
#include "usbll.h"

#include "custom_handlers.h"

/*
Sony DualShock 4 controller driver

Features:
D-pad and Square / Cross / Circle / Triangle buttons mapped as 8-bit Kempston Joystick buttons.
Touchpad touch movement mapped as pointing device / mouse X/Y delta.
Touchpad second touch mapped as mouse left button, touchpad click as right mouse button.

Experimental:
Controller left analog stick mapped to pointer delta (covert axis range from 0..255 to -128..255, apply deadzone / treshold of 16 and scale axis by 16)
L1/R1 buttons mapped as mouse left / right buttons.

Driver tracks and reports if mapped pointing device buttons state changed.

Reference:
https://www.psdevwiki.com/ps4/DS4-USB
https://controllers.fandom.com/wiki/Sony_DualShock_4
https://github.com/felis/USB_Host_Shield_2.0/PS4USB.h
https://github.com/Slamy/Yaumataca/src/handlers/hid_ps4.cpp
https://github.com/torvalds/linux/blob/master/drivers/hid/hid-sony.c

VendorID: 0x054C
ProductID: 0x05C4

Device Descriptor :
12 01 00 02 00 00 00 40 4C 05 C4 05 00 01 01 02
00 01

Config Descriptor :
09 02 29 00 01 01 00 C0 FA 09 04 00 00 02 03 00
00 00 09 21 11 01 00 01 22 E3 01 07 05 84 03 40
00 05 07 05 03 03 40 00 05

Interface 0 Report Descriptor :
05 01 09 05 A1 01 85 01 09 30 09 31 09 32 09 35
15 00 26 FF 00 75 08 95 04 81 02 09 39 15 00 25
07 35 00 46 3B 01 65 14 75 04 95 01 81 42 65 00
05 09 19 01 29 0E 15 00 25 01 75 01 95 0E 81 02
06 00 FF 09 20 75 06 95 01 15 00 25 7F 81 02 05
01 09 33 09 34 15 00 26 FF 00 75 08 95 02 81 02
06 00 FF 09 21 95 36 81 02 85 05 09 22 95 1F 91
02 85 04 09 23 95 24 B1 02 85 02 09 24 95 24 B1
02 85 08 09 25 95 03 B1 02 85 10 09 26 95 04 B1
02 85 11 09 27 95 02 B1 02 85 12 06 02 FF 09 21
95 0F B1 02 85 13 09 22 95 16 B1 02 85 14 06 05
FF 09 20 95 10 B1 02 85 15 09 21 95 2C B1 02 06
80 FF 85 80 09 20 95 06 B1 02 85 81 09 21 95 06
B1 02 85 82 09 22 95 05 B1 02 85 83 09 23 95 01
B1 02 85 84 09 24 95 04 B1 02 85 85 09 25 95 06
B1 02 85 86 09 26 95 06 B1 02 85 87 09 27 95 23
B1 02 85 88 09 28 95 22 B1 02 85 89 09 29 95 02
B1 02 85 90 09 30 95 05 B1 02 85 91 09 31 95 03
B1 02 85 92 09 32 95 03 B1 02 85 93 09 33 95 0C
B1 02 85 A0 09 40 95 06 B1 02 85 A1 09 41 95 01
B1 02 85 A2 09 42 95 01 B1 02 85 A3 09 43 95 30
B1 02 85 A4 09 44 95 0D B1 02 85 A5 09 45 95 15
B1 02 85 A6 09 46 95 15 B1 02 85 F0 09 47 95 3F
B1 02 85 F1 09 48 95 3F B1 02 85 F2 09 49 95 0F
B1 02 85 A7 09 4A 95 01 B1 02 85 A8 09 4B 95 01
B1 02 85 A9 09 4C 95 08 B1 02 85 AA 09 4E 95 01
B1 02 85 AB 09 4F 95 39 B1 02 85 AC 09 50 95 39
B1 02 85 AD 09 51 95 0B B1 02 85 AE 09 52 95 01
B1 02 85 AF 09 53 95 02 B1 02 85 B0 09 54 95 3F
B1 02 85 B1 09 55 95 02 B1 02 85 B2 09 56 95 02
B1 02 C0
Length: 483 bytes
*/

#define PS4_VID 0x054C  // Sony Corporation
#define PS4_PID_V1 0x05c4 // DualShock 4 V1 (CUH-ZCT1x)
#define PS4_PID_V2 0x09cc // DualShock 4 V2 / Slim / Pro (CUH-ZCT2x) PS4_PID_SLIM

// DualShock 4 USB HID report descriptor input endpoint max packet size (DS4_INPUT_REPORT_USB_SIZE)
#define EP_MAXPKTSIZE 64

// Global shared interrupt unsafe unguarded USB packet receive buffer.
// Drivers must (static) assert actual max packet size reported by descriptor <= EP_MAXPKTSIZE
// Note: duplicates ReceiveDataBuffer[512] in usbhost.c
uint8_t g_shared_packed_buffer[EP_MAXPKTSIZE]; // General purpose buffer for input data

enum Direction { // C11: enum struct/class Direction: uint8_t
	North = 0,
	NorthEast,
	East,
	SouthEast,
	South,
	SouthWest,
	West,
	NorthWest,
	None = 8 // Warning: global scope symbol
};

struct TouchPointData { // touch_point
	uint8_t Index : 7; // This counter increments every time a finger touches the touchpad.
	uint8_t NotTouching : 1;
	//uint16_t FingerX : 12; // To decode, read 24 bits as little-endian: lower half is x, upper half is y.
	//uint16_t FingerY : 12;
	uint8_t X_low; // Lower 8 bits of X (X[7:0])
	uint8_t XY_mid; // Upper 4 bits of X (X[11:8]) | Lower 4 bits of Y (Y[3:0])
	uint8_t Y_high; // Upper 8 bits of Y (Y[11:4])
};

//static_assert(sizeof(struct TouchPointData) == 4, "TouchPointData invalid alignment")

struct TouchData {
	uint8_t Timestamp;
	struct TouchPointData TouchPoints[2];
};

//static_assert(sizeof(struct TouchData) == 9, "TouchData invalid alignment");

typedef struct {
		uint8_t report_id;
/*0  */ uint8_t LeftStickX;
/*1  */ uint8_t LeftStickY;
/*2  */ uint8_t RightStickX;
/*3  */ uint8_t RightStickY;
/*4.0*/ enum Direction DPad : 4;
/*4.4*/ uint8_t ButtonSquare : 1;
/*4.5*/ uint8_t ButtonCross : 1;
/*4.6*/ uint8_t ButtonCircle : 1;
/*4.7*/ uint8_t ButtonTriangle : 1;
/*5.0*/ uint8_t ButtonL1 : 1;
/*5.1*/ uint8_t ButtonR1 : 1;
/*5.2*/ uint8_t ButtonL2 : 1;
/*5.3*/ uint8_t ButtonR2 : 1;
/*5.4*/ uint8_t ButtonShare : 1;
/*5.5*/ uint8_t ButtonOptions : 1;
/*5.6*/ uint8_t ButtonL3 : 1;
/*5.7*/ uint8_t ButtonR3 : 1;
/*6.0*/ uint8_t ButtonHome : 1;
/*6.1*/ uint8_t ButtonPad : 1;
/*6.2*/ uint8_t Counter : 6; // always 0 on USB, counts up with some skips on BT
/*7  */ uint8_t TriggerLeft;
/*8  */ uint8_t TriggerRight;
//------------------------------------
/* 9  */ uint16_t Timestamp; // in 5.33us units?
/*11  */ uint8_t Temperture;
/*12  */ int16_t AngularVelocityX;
/*14  */ int16_t AngularVelocityZ;
/*16  */ int16_t AngularVelocityY;
/*18  */ int16_t AccelerometerX;
/*20  */ int16_t AccelerometerY;
/*22  */ int16_t AccelerometerZ;
/*24  */ uint8_t ExtData[5]; // range can be set by EXT device
/*29  */ uint8_t PowerPercent : 4; // 0x00-0x0A or 0x01-0x0B if plugged int
/*29.4*/ uint8_t PluggedPowerCable : 1;
/*29.5*/ uint8_t PluggedHeadphones : 1;
/*29.6*/ uint8_t PluggedMic : 1;
/*29,7*/ uint8_t PluggedExt : 1;
/*30.0*/ uint8_t UnkExt1 : 1; // ExtCapableOfExtraData?
/*30.1*/ uint8_t UnkExt2 : 1; // ExtHasExtraData?
/*30.2*/ uint8_t NotConnected : 1; // Used by dongle to indicate no controller
/*30.3*/ uint8_t Unk1 : 5;
/*31  */ uint8_t Unk2; // unused?
/*32  */ uint8_t TouchCount;
//------------------------------------
		struct TouchData Touches[3]; // Current and two previous touches track and location
		uint8_t Pad[3];
} ps4_hid_report_t;

//static_assert(sizeof(ps4_hid_report_t) == 64, "ps4_hid_report_t invalid alignment")

uint8_t g_pointer_buttons; // ToDo: move to USB_HUB_PORT device state.

bool ps4_usb_match(uint16_t vendor_id, uint16_t product_id)
{
	return (vendor_id == PS4_VID) && ((product_id == PS4_PID_V1) || (product_id == PS4_PID_V2));
}

// X uses lower 8 bits from X_low and upper 4 bits from the lower nibble of XY_mid
#define TOUCH_POINT_X(point) (uint16_t)point->X_low | (((uint16_t)(point->XY_mid & 0x0F)) << 8);
// Y uses lower 4 bits from the upper nibble of XY_mid and upper 8 bits from Y_high
#define TOUCH_POINT_Y(point) (((uint16_t)(point->XY_mid >> 4)) & 0x0F) | (((uint16_t)point->Y_high) << 4);

extern int8_t compute_stick_delta(uint8_t stick_val);

uint8_t ps4_usb_parse_report(void* report, uint8_t length, gamepad_state_t* gamepad_state)
{
	const uint8_t report_id = *(uint8_t*)report;

	if (report_id == 1) // HID axes, buttons and touchpad data (DS4_INPUT_REPORT_USB)
	{
		if(length < sizeof(ps4_hid_report_t))
		{
				DEBUGOUT("Invalid report 1");
				return false;
		}

		const ps4_hid_report_t* r = (ps4_hid_report_t*)report;

		uint8_t state = 0; // D-pad and 4 buttons mapped to 8-bit Kempston Joystick port layout

		// Matches enum Direction
		static const uint8_t hat_to_cross[9] = { // __code used to store const data
			1 << MAP_KEMPSTON_U,                       // North,
			1 << MAP_KEMPSTON_U | 1 << MAP_KEMPSTON_R, // NorthEast,
			1 << MAP_KEMPSTON_R,                       // East,
			1 << MAP_KEMPSTON_R | 1 << MAP_KEMPSTON_D, // SouthEast,
			1 << MAP_KEMPSTON_D,                       // South,
			1 << MAP_KEMPSTON_D | 1 << MAP_KEMPSTON_L, // SouthWest,
			1 << MAP_KEMPSTON_L,                       // West,
			1 << MAP_KEMPSTON_L | 1 << MAP_KEMPSTON_U, // NorthWest,
			0                                          // None
		};
		
		const uint8_t hat = r->DPad < 9 ? r->DPad : None;
		state |= hat_to_cross[hat];

		state |= r->ButtonCross << MAP_KEMPSTON_BUTTON1;
		state |= r->ButtonCircle << MAP_KEMPSTON_BUTTON2;
		state |= r->ButtonSquare << MAP_KEMPSTON_BUTTON3;
		state |= r->ButtonTriangle << MAP_KEMPSTON_BUTTON4;

		gamepad_state->pad_buttons = state;

		const struct TouchPointData* touch_point = r->Touches[0].TouchPoints;

		const bool touch_active = (touch_point->NotTouching == 0);

		if (touch_active)
		{
			const struct TouchPointData* last_touch_point = r->Touches[1].TouchPoints;

			const bool touch_was_active = (last_touch_point->NotTouching == 0);

			if(touch_was_active) // first touch event skipped to calculate delta with previous touch position
			{
				const uint16_t point_x = TOUCH_POINT_X(touch_point);
				const uint16_t point_y = TOUCH_POINT_Y(touch_point);

				const uint16_t last_point_x = TOUCH_POINT_X(last_touch_point);
				const uint16_t last_point_y = TOUCH_POINT_Y(last_touch_point);

				gamepad_state->pointer_delta_x = last_point_x - point_x;
				gamepad_state->pointer_delta_y = last_point_y - point_y;
			}
		}

		const bool touch_button = r->ButtonPad;
		const bool second_touch = r->Touches[0].TouchPoints[1].NotTouching == 0;

		// Touchpad second touch as mouse left button, touchpad click as right mouse button.
		uint8_t pointer_buttons = second_touch | touch_button << 1;

		// Map left analog stick to pointer delta
		gamepad_state->pointer_delta_x += compute_stick_delta(r->LeftStickX);
		gamepad_state->pointer_delta_y += compute_stick_delta(r->LeftStickY);

		// Map L1 and R1 buttons as mouse left / right buttons.
		pointer_buttons |= r->ButtonR1 | r->ButtonL1 << 1;

		gamepad_state->pointer_buttons = pointer_buttons;
		gamepad_state->pointer_buttons_changed = pointer_buttons ^ g_pointer_buttons;

		g_pointer_buttons = pointer_buttons;

		return true;
	}

	return false;
}

bool ps4_usb_poll(USB_HUB_PORT *pUsbDevice, gamepad_state_t* gamepad_state)
{
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data); // get the only stored HID interface
	ENDPOINT* endpoint = currInt->Endpoint; // get the only stored input endpoint

	uint16_t len = 0;
	uint8_t result = TransferReceive(endpoint, g_shared_packed_buffer, &len, 0);

	if (result == ERR_SUCCESS)
	{
		return ps4_usb_parse_report(g_shared_packed_buffer, len, gamepad_state);
	}

	return false;
}

extern bool extract_hid_interface_input_endpoint(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len);

bool ps4_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	const bool result = extract_hid_interface_input_endpoint(pUsbDevice, pCfgDescr, len);

	if(result)
	{
		SetUsbConfig(pUsbDevice, pCfgDescr->bConfigurationValue);

		g_pointer_buttons = 0;

		DEBUGOUT("\nSony DualShock 4");
	}

	return result;
}