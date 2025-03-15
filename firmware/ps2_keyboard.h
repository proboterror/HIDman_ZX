#pragma once

#include <stdint.h>
#include <stdbool.h>

#define KBD_BUFFER_SIZE 16

void ps2_keyboard_init(void);
// Returns PS/2 keyboard set 2 multi-byte make / break scan code and code length from buffer.
// Length = 0 if buffer is empty.
// Note: PAUSE scan code processed as E1,14; 77; E1,F0,14; F0,77 three codes (interfere with NUM (0x77) scan code).
uint8_t ps2_get_raw_code(uint8_t *code_0, uint8_t *code_1, uint8_t *code_2);
// Expected data format: parameters count, followed by up to 3 (8 for Pause/Break) parameters data bytes (PS/2 Set 2 make/break code).
bool ps2_add_raw_code(const uint8_t *data);