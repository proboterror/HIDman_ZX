#pragma once

// Sony DualShock 3 USB driver

#include <stdint.h>
#include <stdbool.h>

bool ps3_usb_match(uint16_t vendor_id, uint16_t product_id);
bool ps3_usb_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t len);
bool ps3_usb_poll(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state);