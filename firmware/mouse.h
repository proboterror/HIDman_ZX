#include <stdint.h>
#ifndef _MOUSE_H_
#define _MOUSE_H_

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

__xdata extern MOUSE OutputMice;

void InitMice(void);
void MouseMove(int32_t DeltaX, int32_t DeltaY, int32_t DeltaZ);
uint8_t GetMouseUpdate(int16_t Min, int16_t Max, int16_t *X, int16_t *Y, int16_t *Z, uint8_t *Buttons, bool Accelerate, uint8_t Downscale);
void MouseSet(uint8_t Button, uint8_t value);

void HandleMouse(void);

#endif