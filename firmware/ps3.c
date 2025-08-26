#include "type.h"
#include "system.h" // For DEBUGOUT
#include "usbdef.h"
#include "usbhost.h"
#include "usbll.h"

/*
Reference:
https://github.com/felis/USB_Host_Shield_2.0/PS3USB.cpp 
https://github.com/Slamy/Yaumataca/src/handlers/hid_ps3.cpp
https://github.com/torvalds/linux/blob/master/drivers/hid/hid-sony.c

VendorID: 0x054C
ProductID: 0x0268

Device Descriptor :
12 01 00 02 00 00 00 40 4C 05 68 02 00 01 01 02
00 01

Config Descriptor :
09 02 29 00 01 01 00 80 FA 09 04 00 00 02 03 00
00 00 09 21 11 01 00 01 22 94 00 07 05 02 03 40
00 01 07 05 81 03 40 00 01

Interface 0 Report Descriptor :
05 01 09 04 A1 01 A1 02 85 01 75 08 95 01 15 00
26 FF 00 81 03 75 01 95 13 15 00 25 01 35 00 45
01 05 09 19 01 29 13 81 02 75 01 95 0D 06 00 FF
81 03 15 00 26 FF 00 05 01 09 01 A1 00 75 08 95
04 35 00 46 FF 00 09 30 09 31 09 32 09 35 81 02
C0 05 01 75 08 95 27 09 01 81 02 75 08 95 30 09
01 91 02 75 08 95 30 09 01 B1 02 C0 A1 02 85 02
75 08 95 30 09 01 B1 02 C0 A1 02 85 EE 75 08 95
30 09 01 B1 02 C0 A1 02 85 EF 75 08 95 30 09 01
B1 02 C0 C0
*/

#define PS3_VID 0x054C  // Sony Corporation
#define PS3_PID 0x0268  // PS3 Controller DualShock 3

/* PS3 data taken from descriptors */
#define EP_MAXPKTSIZE 64 // max size for data via USB

/* Names we give to the 3 ps3 pipes - this is only used for setting the bluetooth address into the ps3 controllers */
#define PS3_CONTROL_PIPE 0
#define PS3_OUTPUT_PIPE 1
#define PS3_INPUT_PIPE 2

uint8_t ps3_readBuf[EP_MAXPKTSIZE]; // General purpose buffer for input data

typedef struct
{
	uint8_t reserved1[2];

	// Byte 2
	uint8_t button_select : 1;
	uint8_t stick_click_left : 1;
	uint8_t stick_click_right : 1;
	uint8_t button_start : 1;
	uint8_t dpad_up : 1;
	uint8_t dpad_right : 1;
	uint8_t dpad_down : 1;
	uint8_t dpad_left : 1;

	// Byte 3
	uint8_t trigger_l2 : 1;
	uint8_t trigger_r2 : 1;
	uint8_t trigger_l1 : 1;
	uint8_t trigger_r1 : 1;

	uint8_t button_triangle : 1;
	uint8_t button_circle : 1;
	uint8_t button_cross : 1;
	uint8_t button_square : 1;

	// Bytes 4,5
	uint8_t reserved3[2];

	// Bytes 6,7,8,9
	uint8_t joy_left_x;
	uint8_t joy_left_y;
	uint8_t joy_right_x;
	uint8_t joy_right_y;
} ps3_hid_report_t;

bool ps3_usb_match(uint16_t vendor_id, uint16_t product_id)
{
	return (vendor_id == PS3_VID) && (product_id == PS3_PID);
}

void ps3_usb_setup_reception(USB_HUB_PORT *pUsbDevice, uint8_t interface)
{
	// Command used to enable the Dualshock 3 and Navigation controller to send data via USB
	static uint8_t enable_command[] = { 0x42, 0x0c, 0x00, 0x00 }; // Special PS3 Controller enable commands

	const uint8_t report_id = 0xF4;

	// Copy from usbhost/SetReport: changed HID_REPORT_OUTPUT to HID_REPORT_FEATURE, added Report ID to value param lower bits
	static __xdata UINT8 s;
	static __xdata UINT16 len;

	static __xdata USB_SETUP_REQ SetupReq;
	FillSetupReq(&SetupReq, USB_REQ_TYP_OUT | USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF,
		HID_SET_REPORT, HID_REPORT_FEATURE << 8 | report_id, interface, sizeof(enable_command));

	s = HostCtrlTransfer(&SetupReq, pUsbDevice->MaxPacketSize0, enable_command, &len);
}

uint8_t ps3_usb_parse_report(void* report, uint8_t length, uint8_t* gamepad_state)
{
	const ps3_hid_report_t* r = (ps3_hid_report_t*)report;

	uint8_t state = 0;
	
	state |= r->dpad_left << MAP_KEMPSTON_L;
	state |= r->dpad_down << MAP_KEMPSTON_D;
	state |= r->dpad_right << MAP_KEMPSTON_R;
	state |= r->dpad_up << MAP_KEMPSTON_U;

	state |= r->button_cross << MAP_KEMPSTON_BUTTON1;
	state |= r->button_circle << MAP_KEMPSTON_BUTTON2;
	state |= r->button_square << MAP_KEMPSTON_BUTTON3;
	state |= r->button_triangle << MAP_KEMPSTON_BUTTON4;

	*gamepad_state = state;

	return true;
}

bool ps3_usb_poll(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state)
{
	// Get dummy device interface 0
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	ENDPOINT* endpoints = currInt->Endpoint;

	uint16_t len = 0;
	uint8_t result = TransferReceive(&endpoints[PS3_INPUT_PIPE], ps3_readBuf, &len, 0);
	DEBUGOUT("%ui ", len);
	if (result == ERR_SUCCESS)
	{
		return ps3_usb_parse_report(ps3_readBuf, len, gamepad_state);
	}

	return false;
}

bool ps3_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	(void)pCfgDescr;
	(void)len;

	const UINT8 INTERFACE_NUMBER = 0;

	pUsbDevice->Interfaces = ListAdd(pUsbDevice->Interfaces, sizeof(INTERFACE), INTERFACE_NUMBER);
	static INTERFACE * __xdata currInt;
	currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	InitInterface(currInt);

	pUsbDevice->InterfaceNum++;

	ENDPOINT* endpoints = currInt->Endpoint;

	// Initialize data structures for endpoints of device
	endpoints[PS3_OUTPUT_PIPE].EndpointAddr = 0x02; // PS3 output endpoint
	endpoints[PS3_OUTPUT_PIPE].MaxPacketSize = EP_MAXPKTSIZE;
	endpoints[PS3_OUTPUT_PIPE].EndpointDir = ENDPOINT_OUT;

	endpoints[PS3_INPUT_PIPE].EndpointAddr = 0x01; // PS3 report endpoint
	endpoints[PS3_INPUT_PIPE].MaxPacketSize = EP_MAXPKTSIZE;
	endpoints[PS3_INPUT_PIPE].EndpointDir = ENDPOINT_IN;

	const uint8_t PS3_DEFAULT_DEVICE_CONFIGURATION = 1; 
	SetUsbConfig(pUsbDevice, PS3_DEFAULT_DEVICE_CONFIGURATION);

	ps3_usb_setup_reception(pUsbDevice, INTERFACE_NUMBER);

	DEBUGOUT("\nSony DualShock 3");

	return true;
}