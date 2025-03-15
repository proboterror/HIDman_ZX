#ifndef	__SYSTEM_H__
#define __SYSTEM_H__


//#define	 FREQ_SYS	48000000ul	  //系统主频48MHz

#ifndef  BUAD_RATE
#ifdef DEBUG
#define  BUAD_RATE  115200ul
#else
#define  BUAD_RATE  115200ul
#endif
#endif

void CfgFsys(void);                        //CH559时钟选择和配置
void mDelayuS(UINT16 n);              // 以uS为单位延时
void mDelaymS(UINT16 n);              // 以mS为单位延时

#include <stdio.h>


void initClock(void);
unsigned char UART0Receive(void);
void UART0Send(unsigned char b);
int putcharserial(int c);

#define st(x)      do { x } while (__LINE__ == -1)

#define HAL_ENABLE_INTERRUPTS()         st( EA = 1; )
#define HAL_DISABLE_INTERRUPTS()        st( EA = 0; )
#define HAL_INTERRUPTS_ARE_ENABLED()    (EA)

typedef unsigned char halIntState_t;
#define HAL_ENTER_CRITICAL_SECTION(x)   st( x = EA;  HAL_DISABLE_INTERRUPTS(); )
#define HAL_EXIT_CRITICAL_SECTION(x)    st( EA = x; )
#define HAL_CRITICAL_STATEMENT(x)       st( halIntState_t _s; HAL_ENTER_CRITICAL_SECTION(_s); x; HAL_EXIT_CRITICAL_SECTION(_s); )


typedef void(* __data FunctionReference)(void);
extern FunctionReference runBootloader;

void GPIOInit(void);
void ClockInit(void);

//Input only, no pull up
#define PIN_MODE_INPUT 0
//Input only, pull up
#define PIN_MODE_INPUT_PULLUP 1
//Push-pull output, high and low level strong drive
#define PIN_MODE_OUTPUT 2
//Open drain output, no pull-up, support input
#define PIN_MODE_OUTPUT_OPEN_DRAIN 3
//Open-drain output, no pull-up, only drives 2 clocks high when the transition output goes from low to high
#define PIN_MODE_OUTPUT_OPEN_DRAIN_2CLK 4
//Weakly bidirectional (standard 51 mode), open drain output, with pull-up
#define PIN_MODE_INPUT_OUTPUT_PULLUP 5
//Quasi-bidirectional (standard 51 mode), open-drain output, with pull-up, when the transition output is low to high, only drives 2 clocks high
#define PIN_MODE_INPUT_OUTPUT_PULLUP_2CLK 6

void pinMode(unsigned char port, unsigned char pin, unsigned char mode);

extern __xdata volatile uint16_t SoftWatchdog;

#define DEBUGOUT(...) { printf(__VA_ARGS__);}
#endif

