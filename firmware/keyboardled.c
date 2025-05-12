#include "ch559.h"
#include "type.h"
#include "gpio.h"

#include "keyboardled.h"

__xdata uint8_t LEDDelayMs = 0;

UINT8 GetKeyboardLedStatus(void)
{
	return 0x00;
}

SBIT(LED, PADR(1), 7);

// Keyboard / mouse activity LED connected to P1.7 pin, push-pull mode.
void setLED(bool state)
{
	if(state)
		LED = 1;
	else
	{
		LED = 0;
		LEDDelayMs = 33;
	}
}