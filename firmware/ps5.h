#pragma once

// Sony PS5 DualSense USB driver

#include <stdint.h>
#include <stdbool.h>

bool ps5_usb_match(uint16_t vendor_id, uint16_t product_id);
bool ps5_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len);
bool ps5_usb_poll(USB_HUB_PORT *pUsbDevice, gamepad_state_t* gamepad_state);