#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ch559.h"
#include "usbhost.h"
#include "uart.h"
#include "ps2protocol.h"
#include "parsedescriptor.h"
#include "mouse.h"
#include "pwm.h"
#include "keyboardled.h"
#include "settings.h"
#include "system.h"
#include "ps2_keyboard.h"
#include "zx_keyboard.h"


uint8_t UsbUpdateCounter = 0;


void EveryMillisecond(void) {

	// Soft watchdog is to get around the fact that the real watchdog runs too fast
	SoftWatchdog++;
	if (SoftWatchdog > 5000) {
		// if soft watchdog overflows, just go into an infinite loop and we'll trigger the real watchdog
		DEBUGOUT("Soft overflow\n");
		while(1);
	}

	// otherwise, reset the real watchdog
	WDOG_COUNT = 0x00;
	// every 4 milliseconds (250hz), check one or the other USB port (so each gets checked at 125hz)
	if (UsbUpdateCounter == 4)
		s_CheckUsbPort0 = TRUE;
	else if (UsbUpdateCounter == 8){
		s_CheckUsbPort1 = TRUE;
		UsbUpdateCounter = 0;
	}

	UsbUpdateCounter++;

	// Turn current LED on if we haven't seen any activity in a while
	if (LEDDelayMs) {
		LEDDelayMs--;
	} else {
			SetPWM1Dat(0x00);
			SetPWM2Dat(0x00);
			T3_FIFO_L = 0;
			T3_FIFO_H = 0;

			// blue
			T3_FIFO_L = 0xFF;
			T3_FIFO_H = 0;
	}
}



// timer should run at 48MHz divided by (0xFFFF - (TH0TL0))
// i.e. 60khz
void mTimer0Interrupt(void) __interrupt(INT_NO_TMR0)
{
	static uint8_t msDiv = 0;
	if (++msDiv == 60) {
		msDiv = 0;
		EveryMillisecond();
	}

}

// With SDCC prototypes for the interrupts must be visible in the context of the main() function.
void ext0_interrupt(void) __interrupt(INT_NO_INT0);

int main(void)
{
	bool WatchdogReset = 0;

	// Watchdog happened, go to "safe mode"
	if (!(PCON & bRST_FLAG0) && (PCON & bRST_FLAG1)){
		WatchdogReset = 1;
	}

	GPIOInit();

	//delay a bit, without using the builtin functions
	UsbUpdateCounter = 255;
	while (--UsbUpdateCounter);

#if defined(OSC_EXTERNAL)
	if (!(P3 & (1 << 4))) runBootloader();
#endif

	ClockInit();
    mDelaymS(500);   
	
	InitUART0();

	InitUsbData();
	InitUsbHost();

	InitPWM();

	ps2_keyboard_init();
	zx_keyboard_init();

	// timer0 setup
	TMOD = (TMOD & 0xf0) | 0x02; // mode 1 (8bit auto reload)
	TH0 = 0xBD;					 // 60khz

	TR0 = 1; // start timer0
	ET0 = 1; //enable timer0 interrupt;

	EA = 1;	 // enable all interrupts

	if (WatchdogReset) DEBUGOUT("Watchdog reset detected (%x), entering safemode\n", PCON);

	InitSettings();

	// enable watchdog
	WDOG_COUNT = 0x00;
	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;
	GLOBAL_CFG |= bWDOG_EN;

	WDOG_COUNT = 0x00;

	DEBUGOUT("ok\n");

	// main loop
	while (1)
	{
		// reset watchdog
		SoftWatchdog = 0;

		ProcessUsbHostPort();
		HandleMouse();
		
		zx_keyboard_update();
	}
}
