#pragma once

#include <stdint.h>

void ps2_mouse_init_registers();

void ps2_mouse_update();

void mouse_set_state(uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4);