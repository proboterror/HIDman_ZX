/*
 ZX BUS Kempston Mouse controller

 Kempston mouse ports:
 #FADF 1111 1010 1101 1111 BUTTONS
 #FBDF 1111 1011 1101 1111 MX
 #FFDF 1111 1111 1101 1111 MY

 Port #FADF: buttons and mouse wheel.
  D0: left button (0 = pressed)
  D1: right button (0 = pressed)
  D2: middle button (0 = pressed)
  D3: reserved (default 1)
  D4-D7: mouse wheel axis

   Note on mouse wheel:
   https://velesoft.speccy.cz/kmouse/km-doc/kempston_mouse_turbo_interface/km-t_2011/k-mouse2011-doc.pdf:
   If wheel is off then D4-D7 on button port return 1111.
   https://www.benophetinternet.nl/hobby/kmt/index.htm
   Bit 4-7 (D4-D7) return the position of the mouse wheel = %1111 is default

 Port #FBDF: X axis (increases from left to right)
 Port #FFDF: Y axis (increases from down to up)
*/
module kempston_mouse
(
	input wire MX, MY, MKEY,
	input wire[7:0] DI,

	input wire [15:0] A,
	input wire M1, // 1
	input wire RD, // active low
	input wire IORQ, // active low

	input wire rst_in,

	output wire IORQGE,
	output wire enable,

	output logic[7:0] D
);

	reg[7:0] register_x, register_y, register_key;

	// Gluk Reset Service mouse detect code expects correct controller registers values early on power on.

	always @(negedge rst_in or posedge MX)
	begin
		if(rst_in == 0)
			register_x = 8'h80;
		else if(MX == 1)
			register_x = DI;
	end

	always @(negedge rst_in or posedge MY)
	begin
		if(rst_in == 0)
			register_y = 8'h60;
		else if(MY == 1)
			register_y = DI;
	end

	always @(negedge rst_in or posedge MKEY)
	begin
		if(rst_in == 0)
			register_key = 8'hFF;
		else if(MKEY == 1)
			register_key = DI;
	end
/*
	// Data written to registers on rising_edge.
	always @(posedge MX) register_x = DI;
	always @(posedge MY) register_y = DI;
	always @(posedge MKEY) register_key = DI;
*/
	// Address lower bits: #DF 11011111 1x0xxx11
	//wire address_partial_match = ~((A[7:0] == 8'hDF) & M1); // more macrocells
	wire address_partial_match = ~(A[0] & A[1] & ~A[5] & A[7] & A[15] & M1); // A15 or DOS/ ?
	assign IORQGE = address_partial_match; // CPLD IORQGE pin connected to 3-state buffer OE/ pin.
	assign enable = ~(address_partial_match | RD | IORQ);

	always_comb
	begin
		case ({A[10], A[8]})
			2'b00: D = register_key; // #FA 11111010 xxxxx0x0
			2'b01: D = register_x;   // #FB 11111011 xxxxx0x1
			2'b11: D = register_y;   // #FF 11111111 xxxxx1x1
			default: D = 8'hFF;
		endcase
	end
/*
	wire MKEY_SEL = enable & ~A[8] & ~A[10]; // #FA 11111010 xxxxx0x0
	wire MX_SEL = enable & A[8] & ~A[10];    // #FB 11111011 xxxxx0x1
	wire MY_SEL = enable & A[8] & A[10];     // #FF 11111111 xxxxx1x1

	assign D = 
		MX_SEL ? register_x :
		MY_SEL ? register_y :
		MKEY_SEL ? {register_key[7:4], 1'b1, register_key[2:0]} : 8'b0; // 3rd bit == 1 and can be omitted.
*/
endmodule

module kempston_joy
(
	input wire JOY,
	input wire[7:0] DI,

	input wire [15:0] A,
	input wire M1, // 1
	input wire RD, // active low
	input wire IORQ, // active low

	input wire rst_in,

	input wire JOY_ENABLE,
	
	output wire IORQGE,
	output wire enable,

	output wire[7:0] D
);

	reg[7:0] register_joy;

	always @(negedge rst_in or posedge JOY)
	begin
		if(rst_in == 0)
			register_joy = 8'h0;
		else 
			register_joy = DI;
	end

	// Port #1F/31 xxxxxxxx00011111 
	//wire address_match = ~((A[5:0] == 6'h1F) & M1 & ~JOY_ENABLE);
	wire address_match = (A[7] | A[6] | A[5] | ~M1 | JOY_ENABLE);
	assign IORQGE = address_match;
	assign enable = ~(address_match | RD | IORQ);

	assign D = register_joy;
endmodule

/*
ZX Spectrum 40-key keyboard matrix:
         ┌─────────────────────────────────────────────────────┐
         │     ┌─────────────────────────────────────────┐     │
         │     │     ┌─────────────────────────────┐     │     │
         │     │     │     ┌─────────────────┐     │     │     │
         │     │     │     │     ┌─────┐     │     │     │     │
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
 A11/ ─┤ 1 ├─┤ 2 ├─┤ 3 ├─┤ 4 ├─┤ 5 │ │ 6 ├─┤ 7 ├─┤ 8 ├─┤ 9 ├─┤ 0 ├─ A12/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
 A10/ ─┤ Q ├─┤ W ├─┤ E ├─┤ R ├─┤ T │ │ Y ├─┤ U ├─┤ I ├─┤ O ├─┤ P ├─ A13/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
  A9/ ─┤ A ├─┤ S ├─┤ D ├─┤ F ├─┤ G │ │ H ├─┤ J ├─┤ K ├─┤ L ├─┤Ent├─ A14/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
       ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐ ┌─┴─┐
  A8/ ─┤ CS├─┤ Z ├─┤ X ├─┤ C ├─┤ V │ │ B ├─┤ N ├─┤ M ├─┤ SS├─┤ SP├─ A15/
       └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘ └─┬─┘
         D0    D1    D2    D3    D4    D4    D3    D2    D1    D0
*/
/*
 Emulates 8x16 Analog Switch Array Chip CH446Q in serial mode.
 Address range limited to AY: 0..4, AX: 0..7.
 Special keys:
 ZX_KEY_MAGIC: Y:5 X:8 ?????
 ZX_KEY_RESET: Y:6 X:8
 ZX_KEY_PAUSE: Y:7 X:8

 Note on keyboard keys state initial reset and hotkey initiated Z80 reset:
 
 - Software like Gluk Reset Service expects keyboard state to be correct (keys released) almost immediately after power on / reset.
 MCU with serial interface timings takes too much time to set up keys matrix after power on.
 So, separate rst_in signal (falling edge) is generated externally some time after power on.
 ZX BUS / Z80 RST signal not used because of:

 - Keyboard can initiate Z80 reset with hotkey. Keyboard keys registers, including PAUSE, MAGIC, RESET are not reset.
 RESET signal stays active while hotkey pressed.
*/
/*
 8x16 Analog Switch Array Chip CH446Q
 Reference: CH446 Datasheet http://wch.cn

 CH446Q is an 8x16 matrix analog switch chip. CH446Q contains 128 analog switches, which are distributed
 at each cross point of 8x16 signal channel matrices.

 - CH446Q supports 7-bit parallel address input and is compatible with existing similar products.
 - Support serial address shift input to save pins.

 Pins:
 - DAT Input Serial data input and switch data input in serial address mode; On at high level, off at low level
 - STB Input Strobe pulse input, active at high level
 - CK Input Serial clock input in serial address mode, active on rising edge;

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
 В·В·В·В·В·В·
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
*/
module keyboard
(
	input wire DAT, // Serial data (7 bit), write on rising edge
	input wire SK, // Serial clock
	input wire STB, // Serial transfer done / set switch state strobe

	input wire [15:0] A,
	input wire M1,
	input wire RD,
	input wire IORQ,

	input wire rst_in,

	output wire [4:0] D,
	output wire IORQGE,
	output wire enable,

	output wire PAUSE, MAGIC, RESET // Active Low (init?)
);
	reg[6:0] serial_data;
	reg[4:0] keys [8]; // 1 - key released, 0 - key pressed
	reg REG_PAUSE, REG_MAGIC, REG_RESET;

	always @(posedge SK)
	begin
		serial_data = serial_data << 1;
		serial_data[0] = DAT;
	end

	wire[3:0] AX = serial_data[3:0]; // X range 0..7
	wire[2:0] AY = serial_data[6:4]; // Y range 0..5
	wire key_state = DAT;

	always @(negedge rst_in or posedge STB)
	begin
		if(rst_in == 0)
		begin
			//{keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6], keys[7]} = 5'b11111;
			keys[0] = 5'b11111;
			keys[1] = 5'b11111;
			keys[2] = 5'b11111;
			keys[3] = 5'b11111;
			keys[4] = 5'b11111;
			keys[5] = 5'b11111;
			keys[6] = 5'b11111;
			keys[7] = 5'b11111;
			// BSRQ, NMI, RST set to 0 on bus RESET
			REG_PAUSE = 0;
			REG_MAGIC = 0;
			REG_RESET = 0;
		end
		else begin // posedge STB
			if(AX[3] == 1'b1) // X = 8 for special keys.
			begin
				if(AY == 3'b101 /*5*/) REG_MAGIC = key_state;
				// Note: rst_in will set REG_RESET to 1... https://en.wikipedia.org/wiki/Ouroboros
				if(AY == 3'b110 /*6*/) REG_RESET = key_state;
				if(AY == 3'b111 /*7*/) REG_PAUSE = key_state;
			end
			else
				keys[AX][AY] = !key_state;
		end
	end
	
	// Special keys outputs drive BSS123 N-channel field effect transistor's gates
	assign PAUSE = REG_PAUSE;
	assign MAGIC = REG_MAGIC;
	assign RESET = REG_RESET;
/*
	// Trade COMB(OR) for MUX in RTL map.
	wire kd0, kd1, kd2, kd3, kd4;

	assign kd0 = (keys[0][0] | A[8]) & (keys[1][0] | A[9]) & (keys[2][0] | A[10]) & (keys[3][0] | A[11]) & (keys[4][0] | A[12]) & (keys[5][0] | A[13]) & (keys[6][0] | A[14]) & (keys[7][0] | A[15]);
	assign kd1 = (keys[0][1] | A[8]) & (keys[1][1] | A[9]) & (keys[2][1] | A[10]) & (keys[3][1] | A[11]) & (keys[4][1] | A[12]) & (keys[5][1] | A[13]) & (keys[6][1] | A[14]) & (keys[7][1] | A[15]);
	assign kd2 = (keys[0][2] | A[8]) & (keys[1][2] | A[9]) & (keys[2][2] | A[10]) & (keys[3][2] | A[11]) & (keys[4][2] | A[12]) & (keys[5][2] | A[13]) & (keys[6][2] | A[14]) & (keys[7][2] | A[15]);
	assign kd3 = (keys[0][3] | A[8]) & (keys[1][3] | A[9]) & (keys[2][3] | A[10]) & (keys[3][3] | A[11]) & (keys[4][3] | A[12]) & (keys[5][3] | A[13]) & (keys[6][3] | A[14]) & (keys[7][3] | A[15]);
	assign kd4 = (keys[0][4] | A[8]) & (keys[1][4] | A[9]) & (keys[2][4] | A[10]) & (keys[3][4] | A[11]) & (keys[4][4] | A[12]) & (keys[5][4] | A[13]) & (keys[6][4] | A[14]) & (keys[7][4] | A[15]);

	wire [4:0] half_rows_state = {kd4, kd3, kd2, kd1, kd0};
*/
	wire [4:0] half_rows_state =
		  (keys[0] | {5{A[8]}})
		& (keys[1] | {5{A[9]}})
		& (keys[2] | {5{A[10]}})
		& (keys[3] | {5{A[11]}})
		& (keys[4] | {5{A[12]}})
		& (keys[5] | {5{A[13]}})
		& (keys[6] | {5{A[14]}})
		& (keys[7] | {5{A[15]}});

/*
	wire [4:0] half_rows_state =
	(
		((~A[8])?  keys[0] : 5'b11111) &
		((~A[9])?  keys[1] : 5'b11111) &
		((~A[10])? keys[2] : 5'b11111) &
		((~A[11])? keys[3] : 5'b11111) &
		((~A[12])? keys[4] : 5'b11111) &
		((~A[13])? keys[5] : 5'b11111) &
		((~A[14])? keys[6] : 5'b11111) &
		((~A[15])? keys[7] : 5'b11111)
	);
*/
	// Port #FE/254 xxxxxxxx11111110
	wire address_partial_match = (A[0] | RD | ~M1);
	//wire address_partial_match = ~((A[7:0] == 8'hFE) & M1);
	assign IORQGE = address_partial_match;
	assign enable = ~(address_partial_match | IORQ);

	assign D = half_rows_state;

endmodule

module hidman_zx_bus
(
	// ToDo: Add ZX BUS pin CLK

	input wire MX, MY, MKEY,
	input wire JOY,
	input wire[7:0] DI,

	input wire JOY_ENABLE, // active low
	input wire TAPE_IN,

	input wire [15:0] A,
	input wire M1, // 1
	input wire RD, // active low
	input wire IORQ, // active low
	input wire RST_IN,

	output wire IORQGE,

	output wire[7:0] D,

	input wire DAT,
	input wire SK,
	input wire STB,

	output wire BSRQ, NMI, RST_OUT // Active Low.
);
	wire iorqge_mouse, iorqge_keyboard, iorqge_joy;
	wire en_m, en_k, en_j;
	wire[7:0] d_m, d_j;
	wire[4:0] d_k;

	kempston_mouse mouse(MX, MY, MKEY, DI, A, M1, RD, IORQ, RST_IN, iorqge_mouse, en_m, d_m);
	keyboard key(DAT, SK, STB, A, M1, RD, IORQ, RST_IN, d_k, iorqge_keyboard, en_k, BSRQ, NMI, RST_OUT);
	//kempston_joy pad(JOY, DI, A, M1, RD, IORQ, RST_IN, JOY_ENABLE, iorqge_joy, en_j, d_j);

	// IORQGE = 0 when address lower bits and M1 == 1 (address partial match) else = 1.
	// Connected to 74LVC1G125 3-state buffer OE/ pin, TTL 5V output.
	assign IORQGE = 
		iorqge_mouse &
		iorqge_keyboard /*&
		iorqge_joy*/;

	// Is it correct to assign "1" value bits to open collector output?
	assign D =
		en_m ? d_m :
		en_k ? {1'b1, 1'b1 /*TAPE_IN*/, 1'b1, d_k} : // TAPE IN on port #FE bit 6
		//en_j ? d_j :
		8'bZZZZZZZZ;

endmodule