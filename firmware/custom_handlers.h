#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct custom_handler
{
	bool (*match)(uint16_t vendor_id, uint16_t product_id);
	bool (*init)(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t descr_len);
	bool (*poll)(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state);
} custom_handler_t;

custom_handler_t* match(uint16_t vendor_id, uint16_t product_id);
