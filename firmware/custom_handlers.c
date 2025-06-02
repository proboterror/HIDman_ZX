#include "usbhost.h"
#include "usbdef.h"

#include "custom_handlers.h"

#include "ps3.h"
#include "xbox360_usb.h"
#include "xbox_one.h"

custom_handler_t handlers[] = 
{
	{
		ps3_usb_match,
		ps3_usb_init,
		ps3_usb_poll,
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