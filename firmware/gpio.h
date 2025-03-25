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

// CH559: Pn_DIR Register: 1 makes the corresponding pin an output, and a 0 makes the corresponding pin an input.
#define input(PORT_, PIN_) DIR(PORT_) &= ~(1 << PIN_)
#define output(PORT_, PIN_) DIR(PORT_) |= (1 << PIN_)

// CH559: The Pn Register.
#define low(PORT_, PIN_) PORT(PORT_) &= ~(1 << PIN_)
#define high(PORT_, PIN_) PORT(PORT_) |= (1 << PIN_)
