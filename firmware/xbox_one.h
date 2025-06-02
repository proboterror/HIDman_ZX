#pragma once

// XBox One Controller USB driver

#include <stdint.h>
#include <stdbool.h>

bool xbox_one_match(uint16_t vendor_id, uint16_t product_id);
bool xbox_one_init(USB_HUB_PORT *pUsbDevice, USB_CFG_DESCR *pCfgDescr, uint16_t descr_len);
bool xbox_one_poll(USB_HUB_PORT *pUsbDevice, uint8_t* gamepad_state);