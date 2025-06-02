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
https://github.com/felis/USB_Host_Shield_2.0/blob/master/XBOXUSB.cpp
https://www.partsnotincluded.com/understanding-the-xbox-360-wired-controllers-usb-data/
https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c
ToDo XBox One:
https://github.com/quantus/xbox-one-controller-protocol

VendorID: 045E
ProductID: 028E

Device Descriptor :
12 01 00 02 FF FF FF 08 5E 04 8E 02 14 01 01 02

Config Descriptor :
09 02 99 00 04 01 00 A0 FA 09 04 00 00 02 FF 5D
01 00 11 21 00 01 01 25 81 14 00 00 00 00 13 01
08 00 00 07 05 81 03 20 00 04 07 05 01 03 20 00
08 09 04 01 00 04 FF 5D 03 00 1B 21 00 01 01 01
82 40 01 02 20 16 83 00 00 00 00 00 00 16 03 00
00 00 00 00 00 07 05 82 03 20 00 02 07 05 02 03
20 00 04 07 05 83 03 20 00 40 07 05 03 03 20 00
10 09 04 02 00 01 FF 5D 02 00 09 21 00 01 01 22
84 07 00 07 05 84 03 20 00 10 09 04 03 00 00 FF
FD 13 04 06 41 00 01 01 03
*/

/*
https://gist.github.com/DJm00n/a6bbcb810879daa9354dee4a02a6b34e#file-descriptordump_controller-xbox-360-wired-model-x854237-001-txt
DescriptorDump_Controller (Xbox 360 Wired Model X854237-001).txt
Information for device Controller (VID=0x045E PID=0x028E):

Connection Information:
------------------------------
Device current bus speed: FullSpeed
Device supports USB 1.1 specification
Device supports USB 2.0 specification
Device address: 0x000F
Current configuration value: 0x01
Number of open pipes: 7

Device Descriptor:
------------------------------
0x12	bLength
0x01	bDescriptorType
0x0200	bcdUSB
0xFF	bDeviceClass      (Vendor specific)
0xFF	bDeviceSubClass   
0xFF	bDeviceProtocol   
0x08	bMaxPacketSize0   (8 bytes)
0x045E	idVendor
0x028E	idProduct
0x0114	bcdDevice
0x01	iManufacturer
0x02	iProduct     
0x03	iSerialNumber
0x01	bNumConfigurations

Configuration Descriptor:
------------------------------
0x09	bLength
0x02	bDescriptorType
0x0099	wTotalLength   (153 bytes)
0x04	bNumInterfaces
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
0x5D	bInterfaceSubClass   
0x01	bInterfaceProtocol   
0x00	iInterface

Unknown Descriptor:
------------------------------
0x11	bLength
0x21	bDescriptorType
Hex dump: 
0x11 0x21 0x00 0x01 0x01 0x25 0x81 0x14 0x00 0x00 
0x00 0x00 0x13 0x01 0x08 0x00 0x00 

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x81	bEndpointAddress  (IN endpoint 1)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x04	bInterval         (4 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x01	bEndpointAddress  (OUT endpoint 1)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x08	bInterval         (8 frames)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x01	bInterfaceNumber
0x00	bAlternateSetting
0x04	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x5D	bInterfaceSubClass   
0x03	bInterfaceProtocol   
0x00	iInterface

Unknown Descriptor:
------------------------------
0x1B	bLength
0x21	bDescriptorType
Hex dump: 
0x1B 0x21 0x00 0x01 0x01 0x01 0x82 0x40 0x01 0x02 
0x20 0x16 0x83 0x00 0x00 0x00 0x00 0x00 0x00 0x16 
0x03 0x00 0x00 0x00 0x00 0x00 0x00 

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x82	bEndpointAddress  (IN endpoint 2)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x02	bInterval         (2 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x02	bEndpointAddress  (OUT endpoint 2)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x04	bInterval         (4 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x83	bEndpointAddress  (IN endpoint 3)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x40	bInterval         (64 frames)

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x03	bEndpointAddress  (OUT endpoint 3)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x10	bInterval         (16 frames)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x02	bInterfaceNumber
0x00	bAlternateSetting
0x01	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0x5D	bInterfaceSubClass   
0x02	bInterfaceProtocol   
0x00	iInterface

Unknown Descriptor:
------------------------------
0x09	bLength
0x21	bDescriptorType
Hex dump: 
0x09 0x21 0x00 0x01 0x01 0x22 0x84 0x07 0x00 

Endpoint Descriptor:
------------------------------
0x07	bLength
0x05	bDescriptorType
0x84	bEndpointAddress  (IN endpoint 4)
0x03	bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
0x0020	wMaxPacketSize    (1 x 32 bytes)
0x10	bInterval         (16 frames)

Interface Descriptor:
------------------------------
0x09	bLength
0x04	bDescriptorType
0x03	bInterfaceNumber
0x00	bAlternateSetting
0x00	bNumEndPoints
0xFF	bInterfaceClass      (Vendor specific)
0xFD	bInterfaceSubClass   
0x13	bInterfaceProtocol   
0x04	iInterface

Unknown Descriptor:
------------------------------
0x06	bLength
0x41	bDescriptorType
Hex dump: 
0x06 0x41 0x00 0x01 0x01 0x03 

Microsoft OS Descriptor is not available. Error code: 0x0000001F

String Descriptor Table
--------------------------------
Index  LANGID  String
0x00   0x0000  
0x01   0x0000  "©Microsoft Corporation"
0x02   0x0000  "Controller"
0x03   0x0000  "04B229A"
0x04   0x0000  "Xbox Security Method 3, Version 1.00, © 2005 Microsoft Corporation. All rights reserved."

------------------------------

Connection path for device: 
USB xHCI Compliant Host Controller
Root Hub
Controller (VID=0x045E PID=0x028E) Port: 2

Running on: Windows 10 or greater (Build Version 18363)

Brought to you by TDD v2.15.0, Jun  8 2020, 17:18:07
*/

/*
// Xbox 360 USB Controller HID Report Descriptor
const uint8_t xbox360_hid_descriptor[] = 
{
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    
    // Buttons (2 bytes)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x10,        //   Usage Maximum (Button 16)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Triggers (2 bytes)
    0x05, 0x02,        //   Usage Page (Simulation Controls)
    0x09, 0xC5,        //   Usage (Brake)
    0x09, 0xC4,        //   Usage (Accelerator)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Joysticks (8 bytes)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x35,        //     Usage (Rz)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x04,        //     Report Count (4)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    
    0xC0,              //   End Collection
    
    // Vibration output (if supported)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x91, 0x02,        //     Output (Data,Var,Abs)
    
    0xC0,              //   End Collection
    
    0xC0               // End Collection
};
*/

#define XBOX_VID 0x045E // Microsoft Corporation
#define XBOX_WIRED_PID  0x028E // Microsoft 360 Wired controller
//0x028f "Microsoft X-Box 360 pad v2"

// Data Xbox 360 taken from descriptors
#define XBOX_360_EP_MAXPKTSIZE 32 // max size for data via USB

// Names we give to the 3 Xbox360 pipes
#define XBOX_CONTROL_PIPE 0
#define XBOX_INPUT_PIPE 1
#define XBOX_OUTPUT_PIPE 2

#define XBOX_MAX_ENDPOINTS 3

/*
	The joystick values are signed 16-bit integers with neutral position at 0.
	Trigger values are unsigned 8-bit integers (0 = released, 255 = fully pressed).
	The actual USB report is 20 bytes long (including the 2-byte header).
*/
// ToDo: Ensure byte-aligned packing, SDCC pack structures by default.
typedef struct
{
	// Report header
	uint8_t report_id;    // Always 0x00 for input reports
	uint8_t report_size;  // Size of the report (20 bytes)

	// Digital buttons (1 = pressed, 0 = released)
	uint8_t dpad_up     : 1;
	uint8_t dpad_down   : 1;
	uint8_t dpad_left   : 1;
	uint8_t dpad_right  : 1;
	uint8_t start       : 1;
	uint8_t back        : 1;
	uint8_t left_thumb  : 1;
	uint8_t right_thumb : 1;

	// Main buttons
	uint8_t lb     : 1;
	uint8_t rb     : 1;
	uint8_t xbox   : 1;
	uint8_t unused : 1;
	uint8_t a      : 1;
	uint8_t b      : 1;
	uint8_t x      : 1;
	uint8_t y      : 1;

	// Analog triggers (0-255)
	uint8_t left_trigger;
	uint8_t right_trigger;

	// Joysticks (-32768 to 32767)
	int16_t left_stick_x;
	int16_t left_stick_y;
	int16_t right_stick_x;
	int16_t right_stick_y;

	// Reserved/vibration feedback
	uint8_t reserved[4]; // Typically unused in input reports
} xbox360_usb_report;

uint8_t xbox360_read_buf[XBOX_360_EP_MAXPKTSIZE]; // General purpose buffer for input data

bool xbox360_usb_match(uint16_t vendor_id, uint16_t product_id)
{
	return (vendor_id == XBOX_VID) && (product_id == XBOX_WIRED_PID);
}

bool xbox360_usb_parse_report(void* report, uint8_t length, uint8_t* gamepad_state)
{
	(void)length;

	const xbox360_usb_report* r = (xbox360_usb_report*)report;

	if( (r->report_id == 0x00) /*Always 0x00 for input reports*/ && 
		(r->report_size == 20) /*Size of the report (20 bytes)*/ )
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

bool xbox360_usb_poll(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state)
{
	// Get dummy device interface 0
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	ENDPOINT* endpoints = currInt->Endpoint;

	uint16_t len = 0;
	uint8_t result = TransferReceive(&endpoints[XBOX_INPUT_PIPE], xbox360_read_buf, &len, 0);
	if (result == ERR_SUCCESS)
	{
		return xbox360_usb_parse_report(xbox360_read_buf, len, gamepad_state);
	}

	return false;
}

uint8_t xbox360_usb_command(USB_HUB_PORT *pUsbDevice, uint8_t* data, uint16_t nbytes)
{
	static __xdata UINT16 len;

	static __xdata USB_SETUP_REQ SetupReq;
	FillSetupReq(&SetupReq, USB_REQ_TYP_OUT | USB_REQ_TYP_CLASS | USB_REQ_RECIP_INTERF,
		HID_SET_REPORT, HID_REPORT_OUTPUT << 8 , 0x00, nbytes);

	uint8_t result = HostCtrlTransfer(&SetupReq, pUsbDevice->MaxPacketSize0, data, &len);

	return result;
}

/*
https://tattiebogle.net/ProjectRoot/Xbox360Controller/UsbInfo
LED Control
Some control over the LEDs surrounding the XBox button is provided, corresponding to the markings 1, 2, 3 and 4. This is controlled using message type 0x01.
To select a new pattern for the LEDs, send a message of the following form:
0103xx
Where xx is the desired pattern:

Table 2. LED mode
0x00 All off
0x01 All blinking
0x02 1 flashes
0x03 2 flashes
0x04 3 flashes
0x05 4 flashes
0x06 1 on
0x07 2 on
0x08 3 on
0x09 4 on
0x0A Rotating (e.g. 1-2-4-3)
0x0B Blinking*
0x0C Slow blinking*
0x0D Alternating (e.g. 1+4-2+3)

* The previous setting will be used for these (all blinking, or 1, 2, 3 or 4 on).
*/
enum XBox360_LEDMode
{
	LED_OFF = 0x00,
	LED_1_ON = 0x06,
	LED_2_ON = 0x07,
	LED_3_ON = 0x08,
	LED_4_ON = 0x09,
	LED_ROTATING = 0x0A,
	LED_FASTBLINK = 0x0B,
	LED_SLOWBLINK = 0x0C,
	LED_ALTERNATING = 0x0D
};

void xbox360_usb_set_led_mode(USB_HUB_PORT *pUsbDevice, enum XBox360_LEDMode ledMode)
{
	uint8_t led_command[] = { 0x01, 0x03, (uint8_t)ledMode };

	xbox360_usb_command(pUsbDevice, led_command, sizeof(led_command));
}

bool xbox360_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len)
{
	(void)pCfgDescr;
	(void)len;
/*
Actually used in and out endpoints belongs to different interfaces, ignored to simplify.
Xbox 360 Controller USB Config Descriptor:

0x00,        // bInterfaceNumber 0
...
0x05,        // bDescriptorType (Endpoint)
0x01,        // bEndpointAddress (OUT/H2D)
0x03,        // bmAttributes (Interrupt)
0x20, 0x00,  // wMaxPacketSize 32
0x08,        // bInterval 8 (unit depends on device speed)
...
0x01,        // bInterfaceNumber 1
...
0x05,        // bDescriptorType (Endpoint)
0x02,        // bEndpointAddress (OUT/H2D)
0x03,        // bmAttributes (Interrupt)
0x20, 0x00,  // wMaxPacketSize 32
0x04,        // bInterval 4 (unit depends on device speed)
*/
	const UINT8 INTERFACE_NUMBER = 0;

	pUsbDevice->Interfaces = ListAdd(pUsbDevice->Interfaces, sizeof(INTERFACE), INTERFACE_NUMBER);
	static INTERFACE * __xdata currInt;
	currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	InitInterface(currInt);

	pUsbDevice->InterfaceNum++;

	ENDPOINT* endpoints = currInt->Endpoint;

	endpoints[XBOX_CONTROL_PIPE].EndpointAddr = 0;
	endpoints[XBOX_CONTROL_PIPE].MaxPacketSize = pUsbDevice->MaxPacketSize0; // Use Max Packet Size from device descriptor
	endpoints[XBOX_CONTROL_PIPE].EndpointDir = ENDPOINT_OUT;
	endpoints[XBOX_CONTROL_PIPE].TOG = FALSE;

	endpoints[XBOX_INPUT_PIPE].EndpointAddr = 0x01; // XBOX 360 report endpoint
	endpoints[XBOX_INPUT_PIPE].MaxPacketSize = XBOX_360_EP_MAXPKTSIZE;
	endpoints[XBOX_INPUT_PIPE].EndpointDir = ENDPOINT_IN;
	endpoints[XBOX_INPUT_PIPE].TOG = FALSE;

	endpoints[XBOX_OUTPUT_PIPE].EndpointAddr = 0x02; // XBOX 360 output endpoint
	endpoints[XBOX_OUTPUT_PIPE].MaxPacketSize = XBOX_360_EP_MAXPKTSIZE;
	endpoints[XBOX_OUTPUT_PIPE].EndpointDir = ENDPOINT_OUT;
	endpoints[XBOX_OUTPUT_PIPE].TOG = FALSE;

	const uint8_t XBOX360_DEFAULT_DEVICE_CONFIGURATION = 1; 
	SetUsbConfig(pUsbDevice, XBOX360_DEFAULT_DEVICE_CONFIGURATION);

	xbox360_usb_set_led_mode(pUsbDevice, LED_1_ON);

	DEBUGOUT("\nXBox 360 Controller");

	return true;
}