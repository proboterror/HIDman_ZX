#pragma once

#include "usbhost.h"
#include "defs.h"
#include "type.h"

bool ParseReport(__xdata INTERFACE *interface, uint32_t len, __xdata uint8_t *report);