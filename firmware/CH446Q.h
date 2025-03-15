#pragma once

#include <stdint.h>
#include <stdbool.h>

// Set IO pins to output.
void CH446Q_init();
// Address: bits 6-0: AY2,AY1,AY0,AX3,AX2,AX1,AX0 
void CH446Q_set(uint8_t address, bool value);
// Set all switches to off state.
void CH446Q_reset();