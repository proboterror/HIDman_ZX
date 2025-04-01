#include "ch559.h"
#include "type.h"
#include "gpio.h"

#include "keyboardled.h"

__xdata uint8_t LEDDelayMs = 0;

UINT8 GetKeyboardLedStatus(void)
{
	return 0x00;
}

// Keyboard / mouse activity LED connected to P1.6 pin, push-pull mode.
void setLED(bool state)
{
	if(state)
		high(1, 7);
	else
	{
		low(1, 7);
		LEDDelayMs = 33;
	}
}