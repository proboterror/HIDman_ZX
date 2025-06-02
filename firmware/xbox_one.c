#include <stdint.h>
#include <stdbool.h>

#include "type.h"
#include "ch559.h"
#include "system.h"

#include "defs.h"

#include "usbdef.h"
#include "usbhost.h"
#include "usbll.h"

/*
Reference:
https://github.com/quantus/xbox-one-controller-protocol
https://github.com/felis/USB_Host_Shield_2.0/blob/master/XBOXONE
https://github.com/Slamy/Yaumataca/blob/main/src/handlers/bare_xbox_one.cpp

ToDo: Xbox Series X|S Controllers
https://github.com/medusalix/xone

Also
[MS-GIPUSB]: Gaming Input Protocol (GIP) Universal Serial Bus (USB) Extension
https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-gipusb/e7c90904-5e21-426e-b9ad-d82adeee0dbc
*/

/*
https://gist.github.com/DJm00n/a6bbcb810879daa9354dee4a02a6b34e#file-descriptordump_controller-xbox-one-model-1537-txt
DescriptorDump_Controller (Xbox One Model 1537).txt

Information for device Controller (VID=0x045E PID=0x02D1):

Connection Information:
------------------------------
Device current bus speed: FullSpeed
Device supports USB 1.1 specification
Device supports USB 2.0 specification
Device address: 0x0017
Current configuration value: 0x01
Number of open pipes: 2

Device Descriptor:
------------------------------
0x12	bLength
0x01	bDescriptorType
0x0200	bcdUSB
0xFF	bDeviceClass      (Vendor specific)
0x47	bDeviceSubClass   
0xD0	bDeviceProtocol   
0x40	bMaxPacketSize0   (64 bytes)
0x045E	idVendor
0x02D1	idProduct
0x0203	bcdDevice
0x01	iManufacturer   "Microsoft"
0x02	iProduct        "Controller"
0x03	iSerialNumber   "7EED86C35B04"
0x01	bNumConfigurations

Configuration Descriptor:
------------------------------
0x09	bLength
0x02	bDescriptorType
0x0060	wTotalLength   (96 bytes)
0x03	bNumInterfaces
0x01	bConfigurationValue
0x00	iConfiguration
0xA0	bmAttributes   (Bus-powered Device, Remote-Wakeup)
0xFA	bMaxPower      (500 mA)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x00	bInterfaceNumber
0x00	bAlternateSetting
0x02	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x47	bInterfaceSubClass   
0xD0	bInterfaceProtocol   
0x00	iInterface

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x01	bEndpointAddress  (OUT endpoint 1)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0040	wMaxPacketSize    (1 x 64 bytes)
0x04	bInterval         (4 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x81	bEndpointAddress  (IN endpoint 1)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0040	wMaxPacketSize    (1 x 64 bytes)
0x04	bInterval         (4 frames)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x01	bInterfaceNumber
0x00	bAlternateSetting
0x00	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x47	bInterfaceSubClass   
0xD0	bInterfaceProtocol   
0x00	iInterface

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x01	bInterfaceNumber
0x01	bAlternateSetting
0x02	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x47	bInterfaceSubClass   
0xD0	bInterfaceProtocol   
0x00	iInterface

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x02	bEndpointAddress  (OUT endpoint 2)
0x01	bmAttributes      (Transfer: Isochronous / Synch: None / Usage: Data)
0x00E4	wMaxPacketSize    (1 x 228 bytes)
0x01	bInterval         (1 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x82	bEndpointAddress  (IN endpoint 2)
0x01	bmAttributes      (Transfer: Isochronous / Synch: None / Usage: Data)
0x00E4	wMaxPacketSize    (1 x 228 bytes)
0x01	bInterval         (1 frames)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x02	bInterfaceNumber
0x00	bAlternateSetting
0x00	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x47	bInterfaceSubClass   
0xD0	bInterfaceProtocol   
0x00	iInterface

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x02	bInterfaceNumber
0x01	bAlternateSetting
0x02	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x47	bInterfaceSubClass   
0xD0	bInterfaceProtocol   
0x00	iInterface

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x03	bEndpointAddress  (OUT endpoint 3)
0x02	bmAttributes      (Transfer: Bulk / Synch: None / Usage: Data)
0x0040	wMaxPacketSize    (64 bytes)
0x00	bInterval         

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x83	bEndpointAddress  (IN endpoint 3)
0x02	bmAttributes      (Transfer: Bulk / Synch: None / Usage: Data)
0x0040	wMaxPacketSize    (64 bytes)
0x00	bInterval         

Microsoft OS Descriptor:
------------------------------
0x12	bLength
0x03	bDescriptorType
Hex dump: 
0x12 0x03 0x4D 0x00 0x53 0x00 0x46 0x00 0x54 0x00 
0x31 0x00 0x30 0x00 0x30 0x00 0x90 0x00 

String Descriptor Table
--------------------------------
Index  LANGID  String
0x00   0x0000  0x0409 
0x01   0x0409  "Microsoft"
0x02   0x0409  "Controller"
0x03   0x0409  "7EED86C35B04"

------------------------------

Connection path for device: 
USB xHCI Compliant Host Controller
Root Hub
Controller (VID=0x045E PID=0x02D1) Port: 1

Running on: Windows 10 or greater (Build Version 18363)

Brought to you by TDD v2.15.0, Jun  8 2020, 17:18:07
*/

// PID and VID of the different versions of the controller - see:
// https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c

// Official controllers
#define XBOX_VID1 0x045E      // Microsoft Corporation
#define XBOX_ONE_PID1 0x02D1  // Microsoft X-Box One pad
#define XBOX_ONE_PID2 0x02DD  // Microsoft X-Box One pad (Firmware 2015)
#define XBOX_ONE_PID3 0x02E3  // Microsoft X-Box One Elite pad
#define XBOX_ONE_PID4 0x02EA  // Microsoft X-Box One S pad
#define XBOX_ONE_PID13 0x0B0A // Microsoft X-Box One Adaptive Controller
#define XBOX_ONE_PID14 0x0B12 // Microsoft X-Box Core Controller

// Xbox One data taken from descriptors
#define XBOX_ONE_EP_MAXPKTSIZE 64 // Max size for data via USB

// Names we give to the 3 XboxONE pipes
#define XBOX_ONE_CONTROL_PIPE 0
#define XBOX_ONE_OUTPUT_PIPE 1
#define XBOX_ONE_INPUT_PIPE 2

#define XBOX_ONE_MAX_ENDPOINTS 3

typedef struct
{
	uint8_t type;
	uint8_t const_0;
	uint16_t id;

	bool sync : 1;
	bool dummy1 : 1; // Always 0.
	bool start : 1;
	bool back : 1;

	bool a : 1;
	bool b : 1;
	bool x : 1;
	bool y : 1;

	bool dpad_up : 1;
	bool dpad_down : 1;
	bool dpad_left : 1;
	bool dpad_right : 1;

	bool bumper_left : 1;
	bool bumper_right : 1;
	bool stick_left_click : 1;
	bool stick_right_click : 1;

	uint16_t trigger_left;
	uint16_t trigger_right;

	int16_t stick_left_x;
	int16_t stick_left_y;
	int16_t stick_right_x;
	int16_t stick_right_y;
} xbox_one_report;

uint8_t xbox_one_read_buf[XBOX_ONE_EP_MAXPKTSIZE]; // General purpose buffer for input data
uint8_t g_cmdCounter = 0;
uint8_t g_pollInterval = 0;

bool xbox_one_match(uint16_t vid, uint16_t pid)
{
	return (vid == XBOX_VID1) &&
		(pid == XBOX_ONE_PID1 || pid == XBOX_ONE_PID2 || pid == XBOX_ONE_PID3 || pid == XBOX_ONE_PID4 ||
		pid == XBOX_ONE_PID13 || pid == XBOX_ONE_PID14);
}

bool xbox_one_parse_report(void* report, uint8_t length, uint8_t* gamepad_state)
{
	(void)length;

	const uint8_t TYPE_BUTTON_DATA =0x20;

	const xbox_one_report* r = (xbox_one_report*)report;

	if (r->type == TYPE_BUTTON_DATA)
	{
		uint8_t state = 0;

		state |= r->dpad_left << MAP_KEMPSTON_L;
		state |= r->dpad_down << MAP_KEMPSTON_D;
		state |= r->dpad_right << MAP_KEMPSTON_R;
		state |= r->dpad_up << MAP_KEMPSTON_U;

		state |= r->a << MAP_KEMPSTON_BUTTON1;
		state |= r->b << MAP_KEMPSTON_BUTTON2;
		state |= r->x << MAP_KEMPSTON_BUTTON3;
		state |= r->y << MAP_KEMPSTON_BUTTON4;

		*gamepad_state = state;

		return true;
	}

	return false;
}

bool xbox_one_poll(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state)
{
	// Get dummy device interface 0
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	ENDPOINT* endpoints = currInt->Endpoint;

	// ToDo: take account on g_pollInterval. Main loop checks USB devices every 8 ms.

	uint16_t len = 0;
	uint8_t result = TransferReceive(&endpoints[XBOX_ONE_INPUT_PIPE], xbox_one_read_buf, &len, 0);
	if (result == ERR_SUCCESS)
	{
		return xbox_one_parse_report(xbox_one_read_buf, len, gamepad_state);
	}

	return false;
}

uint8_t xbox_one_command(USB_HUB_PORT *pUsbDevice, uint8_t* data, uint16_t nbytes)
{
	data[2] = g_cmdCounter++; // Increment the output command counter

	// Get dummy device interface 0
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	ENDPOINT* endpoints = currInt->Endpoint;

	uint8_t result = TransferSend(&endpoints[XBOX_ONE_OUTPUT_PIPE], data, nbytes, 0);

	return result;
}

/*
Xbox One Controller USB LED Control Protocol
Needs to be checked / validated
The Xbox One controller uses HID feature reports for configuration. The LED control is handled through output reports:
    Report ID: 0x01 (for USB controllers)
    Report Size: 5 bytes (for LED control)

    Command Structure:
    Byte 0: Report ID (0x01)
    Byte 1: Sequence number (increment for each command)
    Byte 2: Command ID (0x03 for LED control)
    Byte 3: LED mode/pattern
    Byte 4: LED brightness/intensity

LED Modes/Patterns (Byte 3):

LED Brightness (Byte 4):
    0x00-0xFF: Brightness level (0x00 = off, 0xFF = max brightness)
*/

//https://gist.github.com/TheNathannator/c5d3b41a12db739b7ffc3d8d1a87c60a#0x0a-led-control
enum xbox_one_led_mode
{
	LED_OFF = 0x00,
	LED_ON = 0x01,
	// Those values obtained from unverified sources
	LED_BLINK_FAST = 0x02,
	LED_BLINK_NORMAL = 0x03,
	LED_BLINK_SLOW = 0x04
};

void xbox_one_set_led_mode(USB_HUB_PORT *pUsbDevice, enum xbox_one_led_mode led_mode)
{
	const uint8_t LED_BRIGHTNESS = 0xFF;

	uint8_t write_buf[] = 
	{
		0x01, // Report ID
		0x00, // Sequence number, assigned in xbox_one_command()
		0x03, //LED command
		(uint8_t)led_mode,
		LED_BRIGHTNESS // LED brightness
	};
	
	uint8_t rcode = xbox_one_command(pUsbDevice, write_buf, sizeof(write_buf));

	if(rcode != ERR_SUCCESS)
	{
		DEBUGOUT("XBox One error set LED mode");
	}
}

#if 0
https://raw.githubusercontent.com/torvalds/linux/refs/heads/master/drivers/input/joystick/xpad.c

/*
 * starting with xbox one, the game input protocol is used
 * magic numbers are taken from
 * - https://github.com/xpadneo/gip-dissector/blob/main/src/gip-dissector.lua
 * - https://github.com/medusalix/xone/blob/master/bus/protocol.c
 */

#define GIP_CMD_POWER    0x05
...
#define GIP_SEQ0 0x00
...
#define GIP_OPT_INTERNAL 0x20
...
/*
 * length of the command payload encoded with
 * https://en.wikipedia.org/wiki/LEB128
 * which is a no-op for N < 128
 */
#define GIP_PL_LEN(N) (N)

/*
 * payload specific defines
 */
#define GIP_PWR_ON 0x00

/*
 * This packet is required for all Xbox One pads with 2015
 * or later firmware installed (or present from the factory).
 */
static const u8 xboxone_power_on[] = {
	GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(1), GIP_PWR_ON
};

/*
 * This packet is required for Xbox One S (0x045e:0x02ea)
 * and Xbox One Elite Series 2 (0x045e:0x0b00) pads to
 * initialize the controller that was previously used in
 * Bluetooth mode.
 */
static const u8 xboxone_s_init[] = {
	GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, 0x0f, 0x06
};

/*
 * This packet is required to get additional input data
 * from Xbox One Elite Series 2 (0x045e:0x0b00) pads.
 * We mostly do this right now to get paddle data
 */
static const u8 extra_input_packet_init[] = {
	0x4d, 0x10, 0x01, 0x02, 0x07, 0x00
};

/*
 * This specifies the selection of init packets that a gamepad
 * will be sent on init *and* the order in which they will be
 * sent. The correct sequence number will be added when the
 * packet is going to be sent.
 */
static const struct xboxone_init_packet xboxone_init_packets[] = {
	XBOXONE_INIT_PKT(0x0e6f, 0x0165, xboxone_hori_ack_id),
	XBOXONE_INIT_PKT(0x0f0d, 0x0067, xboxone_hori_ack_id),
	XBOXONE_INIT_PKT(0x0000, 0x0000, xboxone_power_on),
	XBOXONE_INIT_PKT(0x045e, 0x02ea, xboxone_s_init),
	XBOXONE_INIT_PKT(0x045e, 0x0b00, xboxone_s_init),
	XBOXONE_INIT_PKT(0x045e, 0x0b00, extra_input_packet_init)
	...
};
#endif

bool xbox_one_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	const uint8_t INTERFACE_NUMBER = 0;
	pUsbDevice->Interfaces = ListAdd(pUsbDevice->Interfaces, sizeof(INTERFACE), INTERFACE_NUMBER);

	pUsbDevice->InterfaceNum++;

	static INTERFACE * __xdata currInt;
	currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	InitInterface(currInt);

	ENDPOINT* endpoints = currInt->Endpoint;

	endpoints[XBOX_ONE_CONTROL_PIPE].EndpointAddr = 0;
	endpoints[XBOX_ONE_CONTROL_PIPE].MaxPacketSize = pUsbDevice->MaxPacketSize0; // Use Max Packet Size from device descriptor
	endpoints[XBOX_ONE_CONTROL_PIPE].EndpointDir = ENDPOINT_OUT;
	endpoints[XBOX_ONE_CONTROL_PIPE].TOG = FALSE;

	currInt->EndpointNum++;

	// Assume XBox One Controller have 1 device configuration with id 0x00 (confirmed for Xbox One Model 1537 VID=0x045E PID=0x02D1)
	// Passed USB_CFG_DESCR requested for configuration 0

	// Extract in/out interrupt endpoints
	{
		static uint8_t * __xdata pDescr;
		static DESCR_HEADER * __xdata pDescrHeader;
		static USB_ENDP_DESCR * __xdata pEdpDescr;
	
		uint16_t totalLen = pCfgDescr->wTotalLengthL | (pCfgDescr->wTotalLengthH << 8);
		if (totalLen > len)
		{
			totalLen = len;
		}
	
		pDescr = (uint8_t *)pCfgDescr;
		pDescr += pCfgDescr->bLength;
	
		uint8_t index = pCfgDescr->bLength;
	
		while (index < totalLen)
		{
			pDescrHeader = (DESCR_HEADER *)pDescr;
			const uint8_t descrType = pDescrHeader->bDescriptorType;
	
			if (descrType == USB_DESCR_TYP_ENDP)
			{
				//endpoint descriptor
				pEdpDescr = (USB_ENDP_DESCR *)pDescr;

				if((pEdpDescr->bmAttributes & USB_ENDP_TYPE_MASK) == USB_ENDP_TYPE_INTER) // Interrupt endpoint
				{ 
					const bool is_input = (pEdpDescr->bEndpointAddress & USB_ENDP_DIR_MASK);
					const uint8_t endpoint_index = is_input ? XBOX_ONE_INPUT_PIPE : XBOX_ONE_OUTPUT_PIPE; // Set the endpoint index

					endpoints[endpoint_index].EndpointAddr = pEdpDescr->bEndpointAddress & 0x0F;
					endpoints[endpoint_index].MaxPacketSize = pEdpDescr->wMaxPacketSizeL | (pEdpDescr->wMaxPacketSizeH << 8);
					endpoints[endpoint_index].EndpointDir = is_input ? ENDPOINT_IN : ENDPOINT_OUT; 
					endpoints[endpoint_index].TOG = FALSE;

					if(g_pollInterval < pEdpDescr->bInterval) // Set the polling interval as the largest polling interval obtained from endpoints
						g_pollInterval = pEdpDescr->bInterval;

					currInt->EndpointNum++;
				}
			}

			if(currInt->EndpointNum >= XBOX_ONE_MAX_ENDPOINTS) // All endpoints extracted
				break;

			pDescr += pDescrHeader->bDescLength;
	
			index += pDescrHeader->bDescLength;
		}
	}

	if(currInt->EndpointNum < XBOX_ONE_MAX_ENDPOINTS)
	{
		DEBUGOUT("XBox One error too few endpoints");
		return false;
	}

	const uint8_t XBOX_ONE_DEFAULT_DEVICE_CONFIGURATION = 1; 
	SetUsbConfig(pUsbDevice, XBOX_ONE_DEFAULT_DEVICE_CONFIGURATION);

	// Initialize the controller for input

	g_cmdCounter = 0; // Reset the counter used when sending out the commands

	// This packet is required for all Xbox One pads with 2015 or later firmware installed (or present from the factory).
	// u8 xboxone_power_on[] = { GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(1), GIP_PWR_ON }
	uint8_t xboxone_power_on[] = 
	{
		0x05, 
		0x20, 
		0x00, // Byte 2 is set in xbox_one_command()
		0x01, 
		0x00
	};

	uint8_t result = xbox_one_command(pUsbDevice, xboxone_power_on, sizeof(xboxone_power_on));

	if (result != ERR_SUCCESS)
	{
		DEBUGOUT("XBox One error enable poll");
		return false;
	}

	xbox_one_set_led_mode(pUsbDevice, LED_BLINK_NORMAL);

	DEBUGOUT("\nXBox One Controller");

	return true;
}