#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	uint8_t pad_buttons; // D-pad and 4 buttons mapped to 8-bit Kempston Joystick port layout
	int16_t pointer_delta_x;
	int16_t pointer_delta_y;
	uint8_t pointer_buttons;
	bool pointer_buttons_changed;
} gamepad_state_t;

typedef struct 
{
	bool (*match)(uint16_t vendor_id, uint16_t product_id);
	bool (*init)(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t descr_len);
	bool (*poll)(USB_HUB_PORT *pUsbDevice, gamepad_state_t* gamepad_state);
} custom_handler_t;

custom_handler_t* match(uint16_t vendor_id, uint16_t product_id);

void process_handler(custom_handler_t* handler, USB_HUB_PORT *pUsbDevice);