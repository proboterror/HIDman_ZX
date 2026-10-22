#pragma once

#include <stdint.h>

typedef struct MOUSE {
    // the accumulated X, Y and wheel movements
    // may have been accumulated over several USB polls
    // and may require several packets to send on the output device
    int16_t DeltaX, DeltaY, DeltaZ;

    // bitmap with currently held buttons
    uint8_t Buttons;

    // do we need to send something?
    // set by USB HID processing, cleared by output
    bool NeedsUpdating;
} MOUSE;

void InitMice(void);
void MouseMove(int32_t DeltaX, int32_t DeltaY, int32_t DeltaZ);
void MouseSet(uint8_t Button, uint8_t value);

void HandleMouse(void);