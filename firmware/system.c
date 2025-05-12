
#include "type.h"
#include "ch559.h"
#include "system.h"
#include "settings.h"
#include "scancode.h"

__xdata volatile uint16_t SoftWatchdog = 0;

FunctionReference runBootloader = (FunctionReference)0xF400;

/*******************************************************************************
 * Function Name  : mDelayus(UNIT16 n)
 * Description    : us延时函数
 * Input          : UNIT16 n
 * Output         : None
 * Return         : None
 *******************************************************************************/ 
void mDelayuS(UINT16 n) // 以uS为单位延时
{
	while (n)
	{				// total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
		++SAFE_MOD; // 2 Fsys cycles, for higher Fsys, add operation here
#ifdef FREQ_SYS
#if FREQ_SYS >= 14000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 16000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 18000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 20000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 22000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 24000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 26000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 28000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 30000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 32000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 34000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 36000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 38000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 40000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 42000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 44000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 46000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 48000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 50000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 52000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 54000000
		++SAFE_MOD;
#endif
#if FREQ_SYS >= 56000000
		++SAFE_MOD;
#endif
#endif
		--n;
	}
}

/*******************************************************************************
 * Function Name  : mDelayms(UNIT16 n)
 * Description    : ms延时函数
 * Input          : UNIT16 n
 * Output         : None
 * Return         : None
 *******************************************************************************/
void mDelaymS(UINT16 n) // 以mS为单位延时
{
	// reset watchdog, as this function is only used by USB routines that sometimes take a while to return
	SoftWatchdog = 0;

	while (n)
	{
		mDelayuS(1000);
		--n;
	}

	SoftWatchdog = 0;
}


int putcharserial(int c)
{
	while (!TI)
		;
	TI = 0;
	SBUF = c & 0xFF;
	return c;
}


/**
 * stdio printf directed to UART0 and/or keyboard port using putchar and getchar
 */

int putchar(int c)
{
	if (HMSettings.SerialDebugOutput){
		while (!TI)
			;
		TI = 0;
		SBUF = c & 0xFF;
	}
	
	return c;
}

int getchar(void)
{
	while (!RI)
		;
	RI = 0;
	return SBUF;
}

void GPIOInit(void)
{
	// P0 are are ZX Kempston mouse registers DI data output.
	P0_DIR = 0xFF;
	PORT_CFG &= ~bP0_OC; // push pull
	P0_PU = 0x00;		 // no pullups
	P0 = 0x00;			 // default pin states

	// 1.0, 1.1, 1.2 are ZX Kempston mouse registers MX, MY, MKEY strobe output,
	// 1.3, 1.4, 1.5 are CH446Q ZX keyboard matrix DATA, SCLK, STROBE output,
	// 1.6 are Kempston joystick JOY strobe output,
	// 1.7 are keyboard activity LED output.
	P1_DIR = 0xFF;
	PORT_CFG &= ~bP1_OC; // push pull
	P1_PU = 0x00;		 // no pullups
	P1 = 0b00000000;	 // default pin states

	// port2 setup
	P2_DIR = 0b00000111; // 2.0, 2.1, 2.2 are GOTEK buttons outputs
	PORT_CFG |= bP2_OC;  // open collector
	P2_PU = 0x00;		 // no pullups
	P2 = 0b00000111;	 // GOTEK buttons off by default (i.e. high)

	// port3 setup
	P3_DIR = 0b00000010; // 6 is switch, 2,3,4,5 are PS/2 mouse/keyboard i/o, 1 is UART0 TXD
	PORT_CFG |= bP3_OC;	 // open collector
	P3_PU = 0b01000010;	 // pullup on 1 for TXD, 6 for switch, PS/2 use external pullups
	P3 = 0b01111110;	 // default pin states

	// port4 setup
	P4_DIR = 0b00010100; // 4.0 is RXD, 4.2 is Blue LED, 4.3 is MOUSE DATA (actually input, since we're faking open drain), 4.4 is TXD, 4.6 is SWITCH
	P4_PU = 0b01000000;	 // pullup on switch
	P4_OUT = 0b00000100; // LEDs off (i.e. HIGH), MOUSE DATA low (since it's switched by toggling input on and off, i.e. faking open drain)
}

void ClockInit(void)
{

	SAFE_MOD = 0x55;
	SAFE_MOD = 0xAA;

#if defined(OSC_EXTERNAL)
	CLOCK_CFG |= bOSC_EN_XT;
	CLOCK_CFG &= ~bOSC_EN_INT;
#endif

	CLOCK_CFG &= ~MASK_SYS_CK_DIV;

	CLOCK_CFG |= 6;
	PLL_CFG = (24 << 0) | (6 << 5);
}

// Ports P0-P3 support pure input, push-pull output and standard bi-direction modes,
// P4 supports pure input and push-pull output modes.
// More efficient preprocessor implementation sample:
// https://github.com/wagiminator/MCU-Templates/blob/main/CH559/template/src/gpio.h
void pinMode(unsigned char port, unsigned char pin, unsigned char mode)
{
	static uint8_t *dir[] = {&P0_DIR, &P1_DIR, &P2_DIR, &P3_DIR};
	static uint8_t *pu[] = {&P0_PU, &P1_PU, &P2_PU, &P3_PU};

	switch (mode)
	{
	case PIN_MODE_INPUT: //Input only, no pull up
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_INPUT_PULLUP: //Input only, pull up
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] |= 1 << pin;
		break;
	case PIN_MODE_OUTPUT: //Push-pull output, high and low level strong drive
		PORT_CFG &= ~(bP0_OC << port);
		*dir[port] |= 1 << pin;
		break;
	case PIN_MODE_OUTPUT_OPEN_DRAIN: //Open drain output, no pull-up, support input (P0-P3 only)
		PORT_CFG |= (bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_OUTPUT_OPEN_DRAIN_2CLK: //Open-drain output, no pull-up, only drives 2 clocks high when the transition output goes from low to high (P0-P3 only)
		PORT_CFG |= (bP0_OC << port);
		*dir[port] |= 1 << pin;
		*pu[port] &= ~(1 << pin);
		break;
	case PIN_MODE_INPUT_OUTPUT_PULLUP: //Weakly bidirectional (standard 51 mode), open drain output, with pull-up (P0-P3 only)
		PORT_CFG |= (bP0_OC << port);
		*dir[port] &= ~(1 << pin);
		*pu[port] |= 1 << pin;
		break;
	case PIN_MODE_INPUT_OUTPUT_PULLUP_2CLK: //Quasi-bidirectional (standard 51 mode), open-drain output, with pull-up, when the transition output is low to high, only drives 2 clocks high (P0-P3 only)
		PORT_CFG |= (bP0_OC << port);
		*dir[port] |= 1 << pin;
		*pu[port] |= 1 << pin;
		break;
	default:
		break;
	}
}
