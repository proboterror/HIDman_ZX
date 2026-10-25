#include "usbhost.h"
#include "usbdef.h"

#include "custom_handlers.h"

#include "ps3.h"
#include "ps4.h"
#include "ps5.h"
#include "xbox360_usb.h"
#include "xbox_one.h"

#include "keyboardled.h"
#include "kempston_joy.h"
#include "mouse.h"

custom_handler_t handlers[] = 
{
	{
		ps3_usb_match,
		ps3_usb_init,
		ps3_usb_poll,
	},
	{
		ps4_usb_match,
		ps4_usb_init,
		ps4_usb_poll,
	},
	{
		ps5_usb_match,
		ps5_usb_init,
		ps5_usb_poll,
	},
	{
		xbox360_usb_match,
		xbox360_usb_init,
		xbox360_usb_poll,
	},
	{
		xbox_one_match,
		xbox_one_init,
		xbox_one_poll,
	}
};

custom_handler_t* match(uint16_t vendor_id, uint16_t product_id)
{
	for(uint8_t i = 0; i < sizeof(handlers) / sizeof(custom_handler_t); i++)
	{
		if(handlers[i].match(vendor_id, product_id))
			return &handlers[i];
	}

	return NULL;
}

void process_handler(custom_handler_t* handler, USB_HUB_PORT *pUsbDevice)
{
	gamepad_state_t gamepad_state = {0,0,0,0, false};

	if (handler->poll(pUsbDevice, &gamepad_state))
	{
		kempston_joy_set(gamepad_state.pad_buttons);

		if(gamepad_state.pad_buttons)
			setLED(false);
		
		if(gamepad_state.pointer_delta_x || gamepad_state.pointer_delta_y )
			MouseMove(gamepad_state.pointer_delta_x, gamepad_state.pointer_delta_y, 0);

		const uint8_t buttons = gamepad_state.pointer_buttons;

		if(gamepad_state.pointer_buttons_changed)
		{
			MouseSet(0, buttons & 0x01);
			MouseSet(1, buttons & 0x02);
		}
	}
}