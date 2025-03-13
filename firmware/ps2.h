#pragma once

#include <stdint.h>

void InitPS2Ports(void);

bool SendKeyboard(const uint8_t *chunk);

void PressKey(uint8_t currchar);
void ReleaseKey(uint8_t currchar);