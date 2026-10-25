#include "type.h"
#include "system.h" // For DEBUGOUT
#include "usbdef.h"
#include "usbhost.h"
#include "usbll.h"

#include "custom_handlers.h"

/*
Sony PS5 DualSense / DualSense Edge controller driver

Features:
D-pad and Square / Cross / Circle / Triangle buttons mapped as 8-bit Kempston Joystick buttons.
Touchpad touch movement mapped as pointing device / mouse X/Y delta.
Touchpad second touch mapped as mouse left button, touchpad click as right mouse button.

Experimental:
Controller left analog stick mapped to pointer delta (covert axis range from 0..255 to -128..255, apply deadzone / treshold of 16 and scale axis by 16)
L1/R1 buttons mapped as mouse left / right buttons.

Driver tracks and reports if mapped pointing device buttons state changed.

Reference:
https://controllers.fandom.com/wiki/Sony_DualSense
https://github.com/felis/USB_Host_Shield_2.0/PS5USB.h
https://github.com/torvalds/linux/blob/master/drivers/hid/hid-sony.c

VendorID: 0x054C
ProductID: 0x0CE6

Device Descriptor :
12 01 00 02 00 00 00 40 4C 05 E6 0C 00 01 01 02
00 01

Config Descriptor :
09 02 E3 00 04 01 00 C0 FA 09 04 00 00 00 01 01
00 00 0A 24 01 00 01 49 00 02 01 02 0C 24 02 01
01 01 06 04 33 00 00 00 0C 24 06 02 01 01 03 00
00 00 00 00 09 24 03 03 01 03 04 02 00 0C 24 02
04 02 04 03 01 00 00 00 00 09 24 06 05 04 01 03
00 00 09 24 03 06 01 01 01 05 00 09 04 01 00 00
01 02 00 00 09 04 01 01 01 01 02 00 00 07 24 01
01 01 01 00 0B 24 02 01 04 02 10 01 80 BB 00 09
05 01 09 88 01 01 00 00 07 25 01 01 00 00 00 09
04 02 00 00 01 02 00 00 09 04 02 01 01 01 02 00
00 07 24 01 06 01 01 00 0B 24 02 01 02 02 10 01
80 BB 00 09 05 82 05 C4 00 01 00 00 07 25 01 00
00 00 00 09 04 03 00 02 03 00 00 00 09 21 11 01
00 01 22 11 01 07 05 84 03 40 00 04 07 05 03 03
40 00 04

Interface 0 Report Descriptor :
05 01 09 05 A1 01 85 01 09 30 09 31 09 32 09 35
09 33 09 34 15 00 26 FF 00 75 08 95 06 81 02 06
00 FF 09 20 95 01 81 02 05 01 09 39 15 00 25 07
35 00 46 3B 01 65 14 75 04 95 01 81 42 65 00 05
09 19 01 29 0F 15 00 25 01 75 01 95 0F 81 02 06
00 FF 09 21 95 0D 81 02 06 00 FF 09 22 15 00 26
FF 00 75 08 95 34 81 02 85 02 09 23 95 2F 91 02
85 05 09 33 95 28 B1 02 85 08 09 34 95 2F B1 02
85 09 09 24 95 13 B1 02 85 0A 09 25 95 1A B1 02
85 20 09 26 95 3F B1 02 85 21 09 27 95 04 B1 02
85 22 09 40 95 3F B1 02 85 80 09 28 95 3F B1 02
85 81 09 29 95 3F B1 02 85 82 09 2A 95 09 B1 02
85 83 09 2B 95 3F B1 02 85 84 09 2C 95 3F B1 02
85 85 09 2D 95 02 B1 02 85 A0 09 2E 95 01 B1 02
85 E0 09 2F 95 3F B1 02 85 F0 09 30 95 3F B1 02
85 F1 09 31 95 3F B1 02 85 F2 09 32 95 0F B1 02
85 F4 09 35 95 3F B1 02 85 F5 09 36 95 03 B1 02
C0
Length: 273
*/

#define PS5_VID 0x054C // Sony Corporation
#define PS5_PID 0x0CE6 // PS5 DualSense Controller
#define PS5_PID_2 0x0DF2 // DualSense Edge

// DualSense USB HID report descriptor input endpoint max packet size (DS_INPUT_REPORT_USB_SIZE)
#define EP_MAXPKTSIZE 64

extern uint8_t g_shared_packed_buffer[EP_MAXPKTSIZE]; // General purpose buffer for input data

enum Direction {
	North = 0,
	NorthEast,
	East,
	SouthEast,
	South,
	SouthWest,
	West,
	NorthWest,
	None = 8
};

struct TouchPointData {
/*0.0*/ uint8_t Index : 7;
/*0.7*/ uint8_t NotTouching : 1;
/*1.0*/ //uint32_t FingerX : 12; // To decode, read 24 bits as little-endian: lower half is x, upper half is y.
/*2.4*/ //uint32_t FingerY : 12;
		uint8_t X_low; // Lower 8 bits of X (X[7:0])
		uint8_t XY_mid; // Upper 4 bits of X (X[11:8]) | Lower 4 bits of Y (Y[3:0])
		uint8_t Y_high; // Upper 8 bits of Y (Y[11:4])
};

//static_assert(sizeof(struct TouchPointData) == 4, "TouchPointData invalid alignment")

struct TouchData {
/*0*/ struct TouchPointData TouchPoints[2];
/*8*/ uint8_t Timestamp;
};

//static_assert(sizeof(struct TouchData) == 9, "TouchData invalid alignment");

typedef struct
{
		uint8_t ReportID; // 0x01 ReportIn01USB 
/*0  */ uint8_t LeftStickX;
/*1  */ uint8_t LeftStickY;
/*2  */ uint8_t RightStickX;
/*3  */ uint8_t RightStickY;
/*4  */ uint8_t TriggerLeft;
/*5  */ uint8_t TriggerRight;
/*6  */ uint8_t SeqNo; // always 0x01 on BT
/*7.0*/ enum Direction DPad : 4;
/*7.4*/ uint8_t ButtonSquare : 1;
/*7.5*/ uint8_t ButtonCross : 1;
/*7.6*/ uint8_t ButtonCircle : 1;
/*7.7*/ uint8_t ButtonTriangle : 1;
/*8.0*/ uint8_t ButtonL1 : 1;
/*8.1*/ uint8_t ButtonR1 : 1;
/*8.2*/ uint8_t ButtonL2 : 1;
/*8.3*/ uint8_t ButtonR2 : 1;
/*8.4*/ uint8_t ButtonCreate : 1;
/*8.5*/ uint8_t ButtonOptions : 1;
/*8.6*/ uint8_t ButtonL3 : 1;
/*8.7*/ uint8_t ButtonR3 : 1;
/*9.0*/ uint8_t ButtonHome : 1;
/*9.1*/ uint8_t ButtonPad : 1;
/*9.2*/ uint8_t ButtonMute : 1;
/*9.3*/ uint8_t UNK1 : 1; // appears unused
/*9.4*/ uint8_t ButtonLeftFunction : 1; // DualSense Edge
/*9.5*/ uint8_t ButtonRightFunction : 1; // DualSense Edge
/*9.6*/ uint8_t ButtonLeftPaddle : 1; // DualSense Edge
/*9.7*/ uint8_t ButtonRightPaddle : 1; // DualSense Edge
/*10 */ uint8_t UNK2; // appears unused
/*11 */ uint32_t UNK_COUNTER; // Linux driver calls this reserved, tools leak calls the 2 high bytes "random"
/*15 */ int16_t AngularVelocityX;
/*17 */ int16_t AngularVelocityZ;
/*19 */ int16_t AngularVelocityY;
/*21 */ int16_t AccelerometerX;
/*23 */ int16_t AccelerometerY;
/*25 */ int16_t AccelerometerZ;
/*27 */ uint32_t SensorTimestamp;
/*31 */ int8_t Temperature; // reserved2 in Linux driver
/*32 */ struct TouchData TouchData;
// The rest of report structure skipped
} ps5_hid_report_t;

//static_assert(sizeof(ps5_hid_report_t) == 34, "ps5_hid_report_t invalid alignment")

typedef struct
{
	uint16_t x;
	uint16_t y;
	bool touched;
	uint8_t pointer_buttons;
} ps5_state_t;

ps5_state_t g_state; // ToDo: move to USB_HUB_PORT device state.

bool ps5_usb_match(uint16_t vendor_id, uint16_t product_id)
{
	return (vendor_id == PS5_VID) && ((product_id == PS5_PID) || (product_id == PS5_PID_2));
}

// X uses lower 8 bits from X_low and upper 4 bits from the lower nibble of XY_mid
#define TOUCH_POINT_X(point) (uint16_t)point->X_low | (((uint16_t)(point->XY_mid & 0x0F)) << 8);
// Y uses lower 4 bits from the upper nibble of XY_mid and upper 8 bits from Y_high
#define TOUCH_POINT_Y(point) (((uint16_t)(point->XY_mid >> 4)) & 0x0F) | (((uint16_t)point->Y_high) << 4);

#define STICK_NEUTRAL_VALUE 128

// Convert analog stick axis value from normalised 0..255 range to signed value, applying deadzone treshold and scale.
int8_t compute_stick_delta(uint8_t stick_val)
{
	// stick_val - 128 results in a range of -128 to +127, fitting safely in int8_t
	const int8_t raw_delta = (int8_t)(stick_val - STICK_NEUTRAL_VALUE);

	if (raw_delta > 0)
		return raw_delta >> 4; // Apply deadzone / treshold of 16 and scale axis by 16

	if (raw_delta < 0)
		return -(-raw_delta >> 4); // Note: right bit shift of negative values results in -1 (0xFF)

	return 0;
}

uint8_t ps5_usb_parse_report(void* report, uint8_t length, gamepad_state_t* gamepad_state)
{
	const uint8_t report_id = *(uint8_t*)report;

	if (report_id == 1) // HID axes, buttons and touchpad data (DS_INPUT_REPORT_USB)
	{
		if(length < sizeof(ps5_hid_report_t))
		{
				DEBUGOUT("Invalid report 1");
				return false;
		}

		const ps5_hid_report_t* r = (ps5_hid_report_t*)report;

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

		const struct TouchPointData* touch_point = r->TouchData.TouchPoints;

		const bool touch_active = (touch_point->NotTouching == 0);

		if (touch_active)
		{
			const uint16_t point_x = TOUCH_POINT_X(touch_point);
			const uint16_t point_y = TOUCH_POINT_Y(touch_point);

			if(g_state.touched) // first touch event skipped to calculate delta with previous touch position
			{
				gamepad_state->pointer_delta_x = point_x - g_state.x;
				gamepad_state->pointer_delta_y = point_y - g_state.y;
			}
			else
			{
				g_state.touched = true;
			}

			g_state.x = point_x;
			g_state.y = point_y;
		}
		else
			g_state.touched = false;

		const bool touch_button = r->ButtonPad;
		const bool second_touch = r->TouchData.TouchPoints[1].NotTouching == 0;

		// Touchpad second touch as mouse left button, touchpad click as right mouse button.
		uint8_t pointer_buttons = second_touch | touch_button << 1;

		// Map left analog stick to pointer delta.
		gamepad_state->pointer_delta_x += compute_stick_delta(r->LeftStickX);
		gamepad_state->pointer_delta_y += compute_stick_delta(r->LeftStickY);

		// Map L1 and R1 buttons as mouse left / right buttons.
		pointer_buttons |= r->ButtonL1 | r->ButtonR1 << 1;

		gamepad_state->pointer_buttons = pointer_buttons;
		gamepad_state->pointer_buttons_changed = pointer_buttons ^ g_state.pointer_buttons;

		g_state.pointer_buttons = pointer_buttons;

		return true;
	}

	return false;
}

bool ps5_usb_poll(USB_HUB_PORT *pUsbDevice, gamepad_state_t* gamepad_state)
{
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data); // get the only stored HID interface
	ENDPOINT* endpoint = currInt->Endpoint; // get the only stored input endpoint

	uint16_t len = 0;
	uint8_t result = TransferReceive(endpoint, g_shared_packed_buffer, &len, 0);

	if (result == ERR_SUCCESS)
	{
		return ps5_usb_parse_report(g_shared_packed_buffer, len, gamepad_state);
	}

	return false;
}

bool extract_hid_interface_input_endpoint(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	uint16_t totalLen = pCfgDescr->wTotalLengthL | (pCfgDescr->wTotalLengthH << 8);
	if (totalLen > len)
	{
		totalLen = len;
	}

	INTERFACE* currInt = NULL;

	uint8_t* pDescr = (uint8_t *)pCfgDescr;

	pDescr += pCfgDescr->bLength; // skip configuration descriptor
	uint8_t index = pCfgDescr->bLength;

	while (index < totalLen) // process all headers till the end of config descriptor
	{
		const DESCR_HEADER* pDescrHeader = (DESCR_HEADER *)pDescr;

		const uint8_t descrType = pDescrHeader->bDescriptorType;

		if(descrType == USB_DESCR_TYP_INTERF) // Interface descriptor
		{
			USB_ITF_DESCR* pItfDescr = (USB_ITF_DESCR *)pDescr;

			if (pItfDescr->bInterfaceClass == USB_DEV_CLASS_HID) // add only HID interface
			{
				pUsbDevice->Interfaces = ListAdd(pUsbDevice->Interfaces, sizeof(INTERFACE), pItfDescr->bInterfaceNumber);
				currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
				InitInterface(currInt);
				pUsbDevice->InterfaceNum++;
			}
			else
			{
				currInt = NULL; // do not process endpoints from other interfaces
			}
		}
		else if (descrType == USB_DESCR_TYP_ENDP) // Endpoint descriptor
		{
			if(currInt) // process endpoints only from HID interface
			{
				const USB_ENDP_DESCR* pEdpDescr = (USB_ENDP_DESCR *)pDescr;

				if((pEdpDescr->bmAttributes & USB_ENDP_TYPE_MASK) == USB_ENDP_TYPE_INTER) // Interrupt endpoint
				{
					if(pEdpDescr->bEndpointAddress & USB_ENDP_DIR_MASK) // input endpoint
					{
						// store one and only input endpoint from HID interface
						ENDPOINT* endpoint = currInt->Endpoint;

						endpoint->EndpointAddr = pEdpDescr->bEndpointAddress & USB_ENDP_ADDR_MASK;
						endpoint->MaxPacketSize = pEdpDescr->wMaxPacketSizeL | (pEdpDescr->wMaxPacketSizeH << 8);
						endpoint->EndpointDir = ENDPOINT_IN;
						endpoint->TOG = FALSE;

						DEBUGOUT("\nEndpoint: %u\n", endpoint->EndpointAddr );

						currInt->EndpointNum++;
					}
				}
			}
		}

		pDescr += pDescrHeader->bDescLength;
		index += pDescrHeader->bDescLength;
	}

	if( pUsbDevice->InterfaceNum == 0 )
	{
		DEBUGOUT("No HID interface");
		return false;
	}

	currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);

	if( currInt->EndpointNum == 0 )
	{
		DEBUGOUT("No input endpoint");
		return false;
	}

	return true;
}

bool ps5_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	const bool result = extract_hid_interface_input_endpoint(pUsbDevice, pCfgDescr, len);

	if(result)
	{
		SetUsbConfig(pUsbDevice, pCfgDescr->bConfigurationValue);

		g_state.touched = false;
		g_state.pointer_buttons = 0;

		DEBUGOUT("\nSony DualSense");
	}

	return result;
}