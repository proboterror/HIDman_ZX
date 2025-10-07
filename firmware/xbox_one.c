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
[MS-GIPUSB]: Gaming Input Protocol (GIP) Universal Serial Bus (USB) Extension
https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-gipusb/15ad7aff-5ede-4fec-b047-9ddc6686973b

GIP protocol implementation:
https://github.com/medusalix/xone/blob/master/bus/protocol.c

Reverse engeneered:
https://github.com/quantus/xbox-one-controller-protocol
https://github.com/felis/USB_Host_Shield_2.0/blob/master/XBOXONE.cpp
https://github.com/Slamy/Yaumataca/blob/main/src/handlers/bare_xbox_one.cpp
https://github.com/Ryzee119/tusb_xinput/blob/master/xinput_host.c
https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c
*/

/*
Source: own dumps https://github.com/proboterror/HIDman_ZX

Xbox One Controller Model 1537

Device Descriptor :
12 01 00 02 FF 47 D0 40 5E 04 D1 02 03 02 01 02
03 01

DeviceClass:255
VendorID:45e
ProductID:2d1
bcdDevice:2030x045E 0x02D1 0x0203
Config Descriptor :
09 02 60 00 03 01 00 A0 FA 09 04 00 00 02 FF 47
D0 00 07 05 01 03 40 00 04 07 05 81 03 40 00 04
09 04 01 00 00 FF 47 D0 00 09 04 01 01 02 FF 47
D0 00 07 05 02 01 E4 00 01 07 05 82 01 E4 00 01
09 04 02 00 00 FF 47 D0 00 09 04 02 01 02 FF 47
D0 00 07 05 03 02 40 00 00 07 05 83 02 40 00 00


Xbox Series S/X Controller Model 1914

Device Descriptor :
12 01 00 02 FF 47 D0 40 5E 04 12 0B 01 05 01 02
03 01

DeviceClass:255
VendorID:45e
ProductID:b12
bcdDevice:5010x045E 0x0B12 0x0501
Config Descriptor :
09 02 77 00 03 01 00 A0 FA 09 04 00 00 02 FF 47
D0 00 07 05 02 03 40 00 04 07 05 82 03 40 00 04
09 04 00 01 02 FF 47 D0 00 07 05 02 03 40 00 04
07 05 82 03 40 00 02 09 04 01 00 00 FF 47 D0 00
09 04 01 01 02 FF 47 D0 00 07 05 03 01 E4 00 01
07 05 83 01 40 00 01 09 04 02 00 00 FF 47 D0 00
09 04 02 01 02 FF 47 D0 00 07 05 04 02 40 00 00
07 05 84 02 40 00 00

See also:
https://github.com/DJm00n/ControllersInfo
*/

// PID and VID of the different versions of the controller - see:
// https://github.com/torvalds/linux/blob/master/drivers/input/joystick/xpad.c

static const struct
{
	uint16_t vid;
	uint16_t pid;
}
supported_devices[] =
{
	{0x045E, 0x02D1}, // Microsoft X-Box One pad
	{0x045E, 0x02DD}, // Microsoft X-Box One pad (Firmware 2015)
	{0x045E, 0x02E3}, // Microsoft X-Box One Elite pad
	{0x045E, 0x02EA}, // Microsoft X-Box One S pad
	{0x045E, 0x0B00}, // Microsoft X-Box One Elite 2 pad
	{0x045E, 0x0B0A}, // Microsoft X-Box One Adaptive Controller
	{0x045E, 0x0B12}, // Microsoft Xbox Series S|X Controller / Microsoft X-Box Core Controller (Model 1914)

	{0x2dc8, 0x2000}, // 8BitDo Pro 2 Wired Controller fox Xbox
	{0x2dc8, 0x200f} // 8BitDo Ultimate 3-mode Controller for Xbox
};

/*
 [MS-GIPUSB] 2.2.2.4.1 SupportedInSystemCommands
 1 Protocol Control. Every device MUST support.
 2 Hello Device. Every device MUST support. Even though this is sent before the metadata
   exchange, this MUST still be listed in the metadata
 3 Status Device. Every device MUST support.
 4 Metadata Response.
   Every device MUST support. Even though the device obviously supports
   the metadata exchange (or the system would not look at this field), this
   MUST still be listed in the metadata.
 6 Security Exchange. Every device MUST support.
 7 Key Input Every device with an Xbox button MUST support this. This message is
   used for indicated VKEY presses, and the Xbox button is in the form of a VKEY (0x5B or VK_LWIN)

[MS-GIPUSB] 2.2.2.4.2 SupportedOutSystemCommands
 1 Protocol Control. Every device MUST support.
 4 Metadata Request
   Every device MUST support. Even though the device obviously supports
   the metadata exchange (or the system would not look at this field), this
   MUST still be listed in the metadata.
 5 Set Device State. Every device MUST support.
 6 Security Exchange. Every device MUST support.
 10 LED Commands Devices with a guide button MUST support this command.
*/

// [MS-GIPUSB] 3.1.5.4 Message Summary

enum gip_command
{
	GIP_CMD_ACK = 0x01, // Protocol Control. Bidirectional. ACKs transfers and indicates receive buffer status.
	GIP_CMD_ANNOUNCE =  0x02, // Hello Device. Upstream. Device sends to Host when communication is established every 500 ms until Host responds.
	GIP_CMD_STATUS = 0x03, // Status Device. Upstream. Device state report: battery level, battery type, and so on.
	GIP_CMD_IDENTIFY= 0x04, // Metadata Request. Downstream. Request for primary device's metadata.
							// Metadata Response. Upstream. Single or multipacket response that contains primary device's metadata.
	GIP_CMD_POWER = 0x05, // Set Device State. Downstream.
/*
[MS-GIPUSB] 3.1.5.5.5 Set Device State Command
Host to device message used to set the state of the device.

Table 39: Downstream GIP Message  Set Device State Command
──────┬────────────┬────────────┬───────────────────────────────────────────────────────
Offset│Value       │Name        │Description
──────┼────────────┼────────────┼───────────────────────────────────────────────────────
0     │0x05        │GIP         │Command Data Class, Command 5.
      │            │MessageType │
1     │0x20        │GIP Flags   │Single packet system message to primary device. No
      │            │            │acknowledgement required.
2     │Incrementing│GIP Sequence│Wrapping counter for Command Data Class that is
      │            │ID          │incremented each time Command packet is sent. 0x00
      │            │            │is reserved.
3     │0x01        │GIP Payload │Length of payload data: 1 byte.
      │            │Length      │
4     │Variable    │State       │New device state. See Device States table for details.
──────┴────────────┴────────────┴───────────────────────────────────────────────────────

Table 40: Device States
──────┬────────┼─────────────────────────────────────────────────────────────────────────────
State │Name    │Description
──────┼────────┼─────────────────────────────────────────────────────────────────────────────
0x00  │Start   │GIP device SHOULD transition to the GIP Active State. See State Machine for
      │        │details on device behavior section 3.1.1.
0x01  │Stop    │GIP device SHOULD transition to the GIP Idle State. See State Machine for
      │        │details on device behavior.
0x02  │        │Not used. Formerly Standby.
0x03  │Full    │Resets idle user timer to prevent wireless devices from turning off. Not
      │power   │relevant to USB wired-only devices which SHOULD ignore this.
0x04  │Off     │GIP device SHOULD transition to the GIP Off State. See State Machine for
      │        │details on device behavior.
0x05  │Quiesce │This is used by the host to signal to a device when the focus changes from
      │        │one app to another or from an app to the dashboard. This is triggered when
      │        │the Guide button on the device is pressed to open the guide. The device
      │        │SHOULD clear all motor states, FFB equations and memory, and so on. The
      │        │app, with the help of the appropriate device library, is expected to restore
      │        │any required motor state, FFB state, and so on, to the device when the app
      │        │resumes.
0x06  │Reserved│Reserved.
0x07  │Reset   │Full device reset. The device SHOULD cleanly tear down the GIP stack as if it
      │        │were shutting down. The device SHOULD then reinitialize everything as it
      │        │does on power up. Before resetting, the device MUST send a status message
      │        │that indicates the Device is Powering Off.
──────┴────────┴─────────────────────────────────────────────────────────────────────────────
*/
	GIP_CMD_VIRTUAL_KEY = 0x07, // Guide Button Status. Upstream. Guide button status on the device.
	GIP_CMD_RUMBLE = 0x09, // Direct Motor Command. Downstream. Directly controls vibration and impulse motors.
	GIP_CMD_LED = 0x0A, // Guide Button LED. Downstream. Command Control Guide button LED intensity.
	GIP_CMD_INPUT = 0x20 // Gamepad Input Report. Upstream. Buttons, Triggers, Thumbstick, and so on data.
};

#define BIT(n) 1 << n

enum gip_option
{
	GIP_OPT_ACK = BIT(4), // GIP message flags: ACME: This complete message or fragment requires acknowledgement.
	GIP_OPT_INTERNAL = BIT(5), // GIP message flags: System: Single packet system message to primary device. No acknowledgement required.
	GIP_OPT_CHUNK_START = BIT(6),
	GIP_OPT_CHUNK = BIT(7)
};

enum gip_device_state
{
	GIP_PWR_ON = 0x00, // Device state: Start: GIP device SHOULD transition to the GIP Active State.
	GIP_PWR_SLEEP = 0x01, // Device state: Stop: GIP device SHOULD transition to the GIP Idle State.
	GIP_PWR_OFF = 0x04, // Device state: Off: GIP device SHOULD transition to the GIP Off State.
	GIP_PWR_RESET = 0x07 // Device state: Reset: Full device reset.
};

#define GIP_SEQ0 0x00 // Dummy placeholder for GIP Sequence ID

#define GIP_PL_LEN(N) (N) // GIP message payload length encoded with https://en.wikipedia.org/wiki/LEB128 which is a no-op for N < 128

// Xbox One data taken from descriptors
#define XBOX_ONE_EP_MAXPKTSIZE 64 // Max size for data via USB

// Names we give to the 3 XboxONE pipes
#define XBOX_ONE_CONTROL_PIPE 0
#define XBOX_ONE_OUTPUT_PIPE 1
#define XBOX_ONE_INPUT_PIPE 2

#define XBOX_ONE_MAX_ENDPOINTS 3

// See also [MS-GIPUSB] 3.1.5.6.1.1 Gamepad Input Report
// ToDo: Ensure byte-aligned packing, SDCC pack structures by default.
typedef struct
{
	uint8_t gip_message_type;
	uint8_t gip_flags;
	uint8_t gip_sequence_id;
	uint8_t gip_payload_length;

	bool sync : 1; // Reserved
	bool dummy1 : 1; // Always 0 / Keep Alive
	bool start : 1; // Menu
	bool back : 1; // View

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
uint8_t g_cmdCounter = 0; // GIP Sequence ID for outcoming GIP messages
uint8_t g_pollInterval = 0; // Typical polling interval / rate is up to 4 ms / 250 Hz.

// Expect controller to be in blinking LED state on power on / connect to be detected as USB device.
// ToDo: Match behavior with Windows PC, expected init controller in any state.
bool xbox_one_match(uint16_t vid, uint16_t pid)
{
	for(uint8_t i = 0; i < sizeof(supported_devices) / sizeof(supported_devices[0]); i++ )
	{
		if((supported_devices[i].vid == vid) && (supported_devices[i].pid == pid))
			return true;
	}

	return false;
}

bool xbox_one_parse_report(void* report, uint8_t length, uint8_t* gamepad_state)
{
	(void)length; // Expected to be at least 18 bytes (4 bytes header + 14 bytes payload)

	const xbox_one_report* r = (xbox_one_report*)report;

	if (r->gip_message_type == GIP_CMD_INPUT)
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
		// Report sent only on changed gamepad controls state.
		return xbox_one_parse_report(xbox_one_read_buf, len, gamepad_state);
	}

	return false;
}

uint8_t xbox_one_command(USB_HUB_PORT *pUsbDevice, uint8_t* data, uint16_t nbytes)
{
	data[2] = ++g_cmdCounter; // Increment the output command counter. Value 0x00 is reserved.

	// Get dummy device interface 0
	INTERFACE* currInt = (INTERFACE*)(pUsbDevice->Interfaces->data);
	ENDPOINT* endpoints = currInt->Endpoint;

	uint8_t result = TransferSend(&endpoints[XBOX_ONE_OUTPUT_PIPE], data, nbytes, 0);

	return result;
}

/*
[MS-GIPUSB] 3.1.5.5.7 LED eButton Command

Table 41: Downstream GIP Message: LED Guide Button Command
──────┬────────────┬───────────────┬──────────────────────────────────────────────────
Offset│Value       │Name           │Description
──────┼────────────┼───────────────┼──────────────────────────────────────────────────
0     │0x0A        │GIP MessageType│Command Data Class, Command 10.
1     │0x20        │GIP Flags      │Single packet system message to primary device. No
      │            │               │acknowledgement required.
2     │Incrementing│GIP Sequence ID│Wrapping counter for Command Data Class that is
      │            │               │incremented each time Command packet is sent.
      │            │               │0x00 is reserved.
3     │0x03        │GIP Payload    │Length of payload data: 3 bytes.
      │            │Length         │
4     │0x00        │Command        │Guide Button LED command.
5     │Variable    │Guide Button   │Guide button LED Pattern to display. See Guide
      │            │LED Pattern    │Button LED Patterns table.
6     │Variable    │Intensity      │Intensity of LED, 0 - 47%. Used on all of the
      │            │               │preceding.
──────┴────────────┴───────────────┴──────────────────────────────────────────────────

Table 42: Guide Button LED Patterns
───────┬───────────────┬───────┬──────────┬───────────────────────────────
Pattern│Name           │On (mS)│Cycle (mS)│Description
───────┼───────────────┼───────┼──────────┼───────────────────────────────
 0x00  │ Off           │-      │∞         │Not implemented by host.
 0x01  │ On            │∞      │∞         │Connected to host via USB.
 0x02  │ Fast Blink    │200    │400       │Not implemented by host.
 0x03  │ Slow Blink    │600    │1,200     │Not implemented by host.
 0x04  │ Charging Blink│3,000  │6,000     │Not implemented by host.
 0x0D  │ Ramp to Level │       │          │
 Others│ Reserved      │-      │-         │-
───────┴───────────────┴───────┴──────────┴───────────────────────────────
*/

enum gip_led_mode
{
	GIP_LED_OFF = 0x00,
	GIP_LED_ON = 0x01,

	GIP_LED_BLINK_FAST = 0x02,
	GIP_LED_BLINK_NORMAL = 0x03,
	GIP_LED_BLINK_SLOW = 0x04,
	GIP_LED_FADE_SLOW = 0x08,
	GIP_LED_FADE_FAST = 0x09,
};

#define GIP_LED_BRIGHTNESS_DEFAULT 20
#define GIP_LED_BRIGHTNESS_MAX 50

void xbox_one_set_led_mode(USB_HUB_PORT *pUsbDevice, enum gip_led_mode led_mode)
{
	uint8_t led_command[] =
	{
		// Header
		GIP_CMD_LED, // GIP MessageType
		GIP_OPT_INTERNAL, // GIP Flags
		GIP_SEQ0 , // GIP Sequence ID
		GIP_PL_LEN(3), // GIP Payload Length
		// Payload
		0x00, // Guide Button LED command
		led_mode, // Guide button LED Pattern to display
		GIP_LED_BRIGHTNESS_DEFAULT // Intensity of LED, 0 - 47%.
	};

	if(xbox_one_command(pUsbDevice, led_command, sizeof(led_command)))
	{
		DEBUGOUT("xbox_one_set_led_mode error");
	}
}

/*
[MS-GIPUSB] 3.1.5.6.1 Direct Motor Command

Table 56: Downstream GIP Message: Direct Motor Command
──────┬────────────┬─────────────────────┬──────────────────────────────────────────────────────
Offset│Value       │Name                 │Description
──────┼────────────┼─────────────────────┼──────────────────────────────────────────────────────
0     │0x09        │GIP MessageType      │Command Data Class, Command 9.
1     │0x00        │GIP Flags            │Not a fragment. Device, not expansion port.
2     │Incrementing│GIP Sequence ID      │Wrapping counter for Command Data Class that is
      │            │                     │incremented each time Command packet is sent.
      │            │                     │0x00 is reserved.
3     │0x09        │GIP Payload Length   │Length of payload data: 9 bytes.
4     │0x00        │Command              │Direct Motor Command.
5     │0x0X        │Motor Bitmap         │[7:4]: Reserved (MUST be zero)
      │            │                     │[3]: Left Impulse Motor
      │            │                     │[2]: Right Impulse Motor
      │            │                     │[1]: Left Vibration Motor
      │            │                     │[0]: Right Vibration Moto
6     │Variable    │Left Impulse Level   │Percentage, 0 - 100% (0x00 to 0x64), of PWM for motor.
7     │Variable    │Right Impulse Level  │Percentage, 0 - 100% (0x00 to 0x64), of PWM for motor.
8     │Variable    │Left Vibration Level │Percentage, 0 - 100% (0x00 to 0x64), of PWM for motor.
9     │Variable    │Right Vibration Level│Percentage, 0 - 100% (0x00 to 0x64), of PWM for motor.
10    │Variable    │Duration             │0    = all motors canceled; levels ignored.
      │            │                     │1    to 255, 10 ms steps
11    │Variable    │Delay                │0    = No delay
      │            │                     │1    to 255, 10 ms steps
12    │Variable    │Repeat               │0    = No repeat, play once
      │            │                     │1    to 255 repeat counts
──────┴────────────┴─────────────────────┴──────────────────────────────────────────────────────
*/

#define GIP_GP_RUMBLE_MAX 100

enum gip_gamepad_motor
{
	GIP_GP_MOTOR_R = BIT(0),
	GIP_GP_MOTOR_L = BIT(1),
	GIP_GP_MOTOR_RT = BIT(2),
	GIP_GP_MOTOR_LT = BIT(3),
};

int gip_send_rumble(USB_HUB_PORT *pUsbDevice)
{
	uint8_t rumble_command[] = 
	{
		// Header
		GIP_CMD_RUMBLE, // GIP MessageType
		0x00, // GIP Flags
		GIP_SEQ0, // GIP Sequence ID
		GIP_PL_LEN(9), // GIP Payload Length
		// Payload
		0x00, // Direct Motor Command
		0x0F, // motors
		4, // left_trigger
		4, // right_trigger
		0x20, // left
		0x20, // right
		80, // duration
		0, // delay
		0 // repeat
	};

	if(xbox_one_command(pUsbDevice, rumble_command, sizeof(rumble_command)))
	{
		DEBUGOUT("gip_send_rumble error");
	}
}

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

	// Assume XBox One Controller have 1 device configuration with id 0x00
	// (confirmed for Xbox One Controller Model 1537 VID=0x045E PID=0x02D1, Xbox Series S/X Controller Model 1914 VID=0x045E PID=0x0B12)
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
				// Endpoint descriptor
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

			if(currInt->EndpointNum >= XBOX_ONE_MAX_ENDPOINTS) // in/out endpoints extracted
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

	uint8_t gip_power_on[] = {GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(1), GIP_PWR_ON};

	// This packet is required for all Xbox One pads with 2015 or later firmware installed (or present from the factory).
	if (xbox_one_command(pUsbDevice, gip_power_on, sizeof(gip_power_on)))
	{
		DEBUGOUT("gip_power_on error");
		return false;
	}
/*
	This packet is required for Xbox One S (0x045e:0x02ea) and Xbox One Elite Series 2 (0x045e:0x0b00) pads
	to initialize the controller that was previously used in Bluetooth mode.

	static const u8 xboxone_s_init[] = { GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, 0x0f, 0x06 };

	{0x045E, 0x02EA}, // Microsoft X-Box One S pad
	{0x045E, 0x0B00}, // Microsoft X-Box One Elite 2 pad
*/
	uint8_t xboxone_s_init[] = {GIP_CMD_POWER, GIP_OPT_INTERNAL, GIP_SEQ0, GIP_PL_LEN(15), 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (xbox_one_command(pUsbDevice, xboxone_s_init, sizeof(xboxone_s_init)))
	{
		DEBUGOUT("xboxone_s_init error");
		return false;
	}

	uint8_t extra_input_packet_init[] = {0x4d, 0x10, GIP_SEQ0, 0x02, 0x07, 0x00};

	if ((pUsbDevice->VendorID == 0x045E) && (pUsbDevice->ProductID == 0x0B00)) // Microsoft X-Box One Elite 2 pad
	{
		if (xbox_one_command(pUsbDevice, extra_input_packet_init, sizeof(extra_input_packet_init)))
		{
			DEBUGOUT("extra_input_packet_init error");
			return false;
		}
	}

	gip_send_rumble(pUsbDevice);

	DEBUGOUT("\nXBox One Controller");

	return true;
}