/*
 8x16 Analog Switch Array Chip CH446Q
 Reference: CH446 Datasheet http://wch.cn

 CH446Q is an 8x16 matrix analog switch chip. CH446Q contains 128 analog switches, which are distributed
 at each cross point of 8x16 signal channel matrices.

 - CH446Q supports 7-bit parallel address input and is compatible with existing similar products.
 - Support serial address shift input to save pins.

 Pins:
 - RST Input External manual reset input, active at high level
 - P/-S Input Address input mode selection: Parallel input mode at high level; serial input mode at low level

 - DAT Input Serial data input and switch data input in serial address mode; On at high level, off at low level
 - STB Input Strobe pulse input, active at high level
 - CS/CK Input Serial clock input in serial address mode, active on rising edge;

 - X0~X15 Analog signal Input/Output Port X of 8x16 matrix analog switch
 - Y0~Y7 Analog signal Input/Output Port Y of 8x16 matrix analog switch

 The following table is the decoding truth table of the 7-bit address ADDR for CH446Q chip and the address table of 128 analog switches.

 Intersection
 Point          ADDR6 ADDR5 ADDR4 ADDR3 ADDR2 ADDR1 ADDR0 Address
 Port Y -        AY2   AY1   AY0   AX3   AX2   AX1   AX0    No.
 Port X
 Y0 - X0          0     0     0     0     0     0     0     00H
 Y0 - X1          0     0     0     0     0     0     1     01H
 Y0 - X2          0     0     0     0     0     1     0     02H
 ······
 Y7 - X14         1     1     1     1     1     1     0     7EH
 Y7 - X15         1     1     1     1     1     1     1     7FH

 The figure below is an example of a serial address input that controls the analog switch with 24H address
 (between Y2 = 010b and X4 = 0100b), first on and then off.

       ─┐   ┌───┐       ┌───┐       ┌────────┐        ┌───┐
 DAT     AY2│AY1│AY0|AX3│AX2│AX1|AX0│ ON/OFF │ ON/OFF │AY2│AY1
       ─┴───┘   └───────┘   └───────┘        └────────┘   └────
           0   1   0   0   1   0   0   ON       OFF      1   0

 CS/SK ──┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┬─────────────────┐ ┌─┐ ┌─
       ──┴─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─────────────────┴─┘ └─┘ 
                                       ┌─┐      ┌─┐
 STB   ────────────────────────────────┘ └──────┘ └────────────

 Serial Address Input
  Control steps in serial address input mode: provide 7-bit address through DAT pin and move into CH446 by
  using 7 rising edges of CS/CK pin, provide data through DAT pin, and provide a high level pulse to STB pin.

  If MCU is connected with CH446 through SPI bus, the bit 7 of a byte of 8-bit data provided by SPI will be
  discarded by CH446, the bits 6-0 of SPI will be used as the address, the serial data output pin of SPI for
  MCU is connected with DAT pin to provide the switch data, and MCU uses an independent pin to control
  STB pin of CH446.

 Interface Timing Parameters, min
 Test Conditions: VDD=5V
  TAS Setup time of DAT input address to CS/CK rising edge 7 nS
  TAH Hold time of DAT input address to CS/CK rising edge 3 nS
  TDS Setup time of DAT input data to STB falling edge 8 nS
  TDH Hold time of DAT input data to STB falling edge 6 nS
  TCS Setup time of CS/CK rising edge to STB rising edge 10 nS
  TCH Hold time of CS/CK rising edge to STB falling edge 7 nS
  TCKL Low level width of CS/CK clock signal 10 nS
  TCKH High level width of CS/CK clock signal 10 nS
  TSTB Width of STB input active high pulse 10 nS

  TSW Execution delay of DAT, STB or RST to analog switch min 5 avg 30 max 70 nS

  PIO implementation:
  https://hackaday.io/project/191238-jumperless/log/222626-the-code-part-3-driving-the-ch446qs
*/

#include "CH446Q.h"

#include "ch559.h"
#include "type.h"
#include "system.h"
#include "gpio.h"

#define CH446Q_DATA_PORT 1
#define CH446Q_DATA_PIN 3

#define CH446Q_SCLK_PORT 1
#define CH446Q_SCLK_PIN 4

#define CH446Q_STROBE_PORT 1
#define CH446Q_STROBE_PIN 5

SBIT(CH446Q_DATA, PADR(CH446Q_DATA_PORT), CH446Q_DATA_PIN);
SBIT(CH446Q_SCLK, PADR(CH446Q_SCLK_PORT), CH446Q_SCLK_PIN);
SBIT(CH446Q_STROBE, PADR(CH446Q_STROBE_PORT), CH446Q_STROBE_PIN);

// Delay timings in us / microseconds
#define CH446Q_CLK_LEN 4 // TCKL / TCKH. Assume TAS / TAH is shorter.
#define CH446Q_CLK_STB_DELAY 4 // TCS
#define CH446Q_STB_LEN 4 // TSTB
#define CH446Q_STB_DAT_HOLD_DELAY 4 // TDH

#define CH446Q_MAX_ADDRESS 0x7F

void CH446Q_init()
{
	// Set control pins to output (push pull).
	// Note: CH559 / i8051 port mode set for all port bits.
	pinMode(CH446Q_DATA_PORT, CH446Q_DATA_PIN, PIN_MODE_OUTPUT);
	pinMode(CH446Q_SCLK_PORT, CH446Q_SCLK_PIN, PIN_MODE_OUTPUT);
	pinMode(CH446Q_STROBE_PORT, CH446Q_STROBE_PIN, PIN_MODE_OUTPUT);

	CH446Q_DATA = CH446Q_SCLK = CH446Q_STROBE = 0;
}

void CH446Q_set(uint8_t address, bool value)
{
	for(int i = 6; i >= 0; i--)
	{
		CH446Q_SCLK = 0;
		const uint8_t address_bit = (address >> i) & 0x01;
		CH446Q_DATA = address_bit;
		mDelayuS(CH446Q_CLK_LEN); // TCKL overlapped with TAS
		// Write address on clock rising edge.
		CH446Q_SCLK = 1;
		mDelayuS(CH446Q_CLK_LEN); // TCKH overlapped with TAH
	}

	// Simplified delay timings, actually TCKH,TCS,TDS,TSTB and TDH,TCH,TAS,TCKL are overlap.
	CH446Q_DATA = value;
	mDelayuS(CH446Q_CLK_STB_DELAY /*- CH446Q_CLK_LEN*/); // TCS
	// Write switch value on strobe high level pulse.
	CH446Q_STROBE = 1;
	mDelayuS(CH446Q_STB_LEN); // TSTB
	CH446Q_STROBE = 0;
	mDelayuS(CH446Q_STB_DAT_HOLD_DELAY); // TDH. Assume TCH - TDH - TCKL <= 0.
}

void CH446Q_reset()
{
	for(uint8_t i = 0; i < CH446Q_MAX_ADDRESS; i++)
		CH446Q_set(i, false);
}