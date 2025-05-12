#pragma once

#define GLUE(a, b) a##b
#define PORT(p) GLUE(P, p)
#define DIR(p) GLUE(P##p, _DIR)
#define PADR(n) GLUE(ADDR_P, n)

// CH559 ports adresses.
enum PORT_ADDR
{
	ADDR_P0 = 0x80,
	ADDR_P1 = 0x90,
	ADDR_P2 = 0xA0,
	ADDR_P3 = 0xB0
};

// CH559 / 8051: set SFR pin register
#define low(sfr) sfr = 0
#define high(sfr) sfr = 1
