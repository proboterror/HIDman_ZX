`timescale 1ns/1ps
module testbench();

reg MX, MY, MKEY, JOY; 
reg M1, RD, IORQ, RST_IN;
reg[7:0] DI;
reg[15:0] A;

reg DAT, SK, STB;

wire[7:0] D;
wire IORQGE;

wire BSRQ, NMI, RST_OUT; 

hidman_zx_bus hidman_zx_bus0(
	.MX(MX),
	.MY(MY),
	.MKEY(MKEY),
	.JOY(JOY),
	.DI(DI),

	.JOY_ENABLE(1'b0), // active low
	.TAPE_IN(1'b0),

	.A(A),
	.M1(M1), // 1
	.RD(RD), // active low
	.IORQ(IORQ), // active low
	.RST_IN(RST_IN),

	.IORQGE(IORQGE),
	.D(D),

	.BSRQ(BSRQ),
	.NMI(NMI),
	.RST_OUT(RST_OUT),

	.DAT(DAT),
	.SK(SK),
	.STB(STB)
);

// https://verificationacademy.com/forums/t/what-is-meant-by-reference-argument-must-be-automatic/29753
task automatic ControllerWrite(ref logic strobe, input reg [7:0] data);
	begin
		DI = data;
		#2
		strobe = 1;
		#2
		strobe = 0;
	end
endtask

task PortRead(input reg[15:0] addr);
	begin
		M1 = 1;
		A = addr;
		#2
		RD = 0;
		IORQ = 0;
		#2;
	end
endtask

task keyboard_write_bit(input logic data);
	begin
		SK = 0;
		#2
		DAT = data;
		#2
		SK = 1;
		#2;
	end
endtask

task keyboard_commit_state(input logic state);
	begin
		DAT = state;
		#2
		STB = 1;
		#2
		STB = 0;
	end
endtask

/*
enum CH446Q_AY
{
	D0 = 0 << 4,
	D1 = 1 << 4,
	D2 = 2 << 4,
	D3 = 3 << 4,
	D4 = 4 << 4
};

enum CH446Q_AX
{
	A8  = 0,
	A9  = 1,
	A10 = 2,
	A11 = 3,
	A12 = 4,
	A13 = 5,
	A14 = 6,
	A15 = 7,
};
*/

task keyboard_set_switch(input logic[2:0] Y, input logic[3:0] X, input logic state);
	begin
		keyboard_write_bit(Y[2]);
		keyboard_write_bit(Y[1]);
		keyboard_write_bit(Y[0]);

		keyboard_write_bit(X[3]);
		keyboard_write_bit(X[2]);
		keyboard_write_bit(X[1]);
		keyboard_write_bit(X[0]);

		keyboard_commit_state(state);
	end
endtask

task reset();
	begin
		RST_IN = 0;
		#2
		RST_IN = 1;
		#2;
	end;
endtask;

initial begin
	MX = 0; 
	MY = 0;
	MKEY = 0;
	JOY = 0;
	DI = 0;

	DAT = 0;
	SK = 0;
	STB = 0;

	RD = 1;
	IORQ = 1;
	M1 = 1;

	A = 16'hFFFF;

	reset();
	// ToDo: IORQGE tests: address + M1.
/*
	Check mouse registers state after reset
	#FADF 1111 1010 1101 1111 BUTTONS
	#FBDF 1111 1011 1101 1111 MX
	#FFDF 1111 1111 1101 1111 MY
*/
	PortRead(16'hFBDF);
	assert (D === 8'h80) else $error("MX reset failed.");
	#10
	PortRead(16'hFFDF);
	assert (D === 8'h60) else $error("MY reset failed.");
	#10
	PortRead(16'hFADF);
	assert (D === 8'hFF) else $error("MKEY reset failed.");
	#10

	// Set mouse controller registers
	ControllerWrite(MX, 8'hCC);
	#10
	ControllerWrite(MY, 8'h55);
	#10
	ControllerWrite(MKEY, 8'hAA);
	#10
	// Check Kempston Mouse controller ports.
	PortRead(16'hFBDF);
	assert (D === 8'hCC) else $error("MX set failed.");
	#10
	PortRead(16'hFFDF);
	assert (D === 8'h55) else $error("MY set failed.");
	#10;
	PortRead(16'hFADF);
	assert (D === 8'hAA) else $error("MKEY set failed."); // 3rd bit always == 1 (reserved/unused)
	#10
/*
	// Check Kempston Joystick state after reset.
	PortRead(16'h001F);
	assert (D === 8'h0) else $error("JOY reset failed.");
	#10;
*/
	// Set and check Kempston Joystick 8-bit port #1F
	A = 16'hFFFF;
	ControllerWrite(JOY, 8'hAA);
	#10
	PortRead(16'h001F);
	assert (D === 8'hAA) else $error("JOY set failed.");
	#10;
/*
	Keyboard port xxFE
	Space...B	7FFE 01111111 11111110
	Enter...H	BFFE 10111111 11111110
	P...V		DFFE 11011111 11111110
	0...6		EFFE 11101111 11111110
	1...5		F7FE 11110111 11111110
	Q...T		FBFE 11111011 11111110
	A...G		FDFE 11111101 11111110
	CS...V		FEFE 11111110 11111110
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
	// Check keyboard rows state after reset.
	A = {8'b11111110, 8'hFE}; // [CS..V]
	#20;
	assert (D[4:0] === 5'b11111) else $error("Keyboard row [CS..V] reset failed.");

	A = {8'b01111111, 8'hFE}; // [B..SP]
	#20;
	assert (D[4:0] === 5'b11111) else $error("Keyboard row [B..SP] reset failed.");

	// AY, AX, state
	keyboard_set_switch(3'h0, 3'h0, 1); // A8 [CS..V]
	keyboard_set_switch(3'h1, 3'h1, 1); // A9 [A..G]
	keyboard_set_switch(3'h2, 3'h2, 1); // A10 [Q..T]

	keyboard_set_switch(3'h0, 3'h3, 0); // A11 [1..5]
	keyboard_set_switch(3'h1, 3'h3, 1);
	keyboard_set_switch(3'h2, 3'h3, 0);
	keyboard_set_switch(3'h3, 3'h3, 1);
	keyboard_set_switch(3'h4, 3'h3, 0);

	keyboard_set_switch(3'h4, 3'h4, 1); // A12 [6..0]
	keyboard_set_switch(3'h3, 3'h5, 1); // A13 [Y..P]
	keyboard_set_switch(3'h2, 3'h6, 1); // A14 [H..Ent]
	keyboard_set_switch(3'h1, 3'h7, 1); // A15 [B..SP]

	// Read keyboard single rows
	A = {8'b11111110, 8'hFE}; // [CS..V]
	#20;
	assert (D[4:0] === 5'b11110) else $error("Keyboard row [CS..V] failed.");

	A = {8'b11111101, 8'hFE}; // [A..G]
	#20;
	assert (D[4:0] === 5'b11101) else $error("Keyboard row [A..G] failed.");

	A = {8'b11111011, 8'hFE}; // [Q..T]
	#20;
	assert (D[4:0] === 5'b11011) else $error("Keyboard row [Q..T] failed.");

	A = {8'b11110111, 8'hFE}; // [1..5]
	#20;
	assert (D[4:0] === 5'b10101) else $error("Keyboard row [1..5] failed.");

	A = {8'b11101111, 8'hFE}; // [6..0]
	#20;
	assert (D[4:0] === 5'b01111) else $error("Keyboard row [6..0] failed.");

	A = {8'b11011111, 8'hFE}; // [Y..P]
	#20;
	assert (D[4:0] === 5'b10111) else $error("Keyboard row [Y..P] failed.");

	A = {8'b10111111, 8'hFE}; // [H..Ent]
	#20;
	assert (D[4:0] === 5'b11011) else $error("Keyboard row [H..Ent] failed.");

	A = {8'b01111111, 8'hFE}; // [B..SP]
	#20;
	assert (D[4:0] === 5'b11101) else $error("Keyboard row [B..SP] failed.");

	// Read keyboard multiple rows
	A = {8'b00111111, 8'hFE}; // [H..Ent], [B..SP]
	#20;
	assert (D[4:0] === 5'b11001) else $error("Keyboard multiple rows failed.");
/*
	Special keys:
	ZX_KEY_MAGIC: Y:5 X:8
	ZX_KEY_RESET: Y:6 X:8
	ZX_KEY_PAUSE: Y:7 X:8
*/
	assert (NMI === 1'h0) else $error("NMI active.");
	assert (RST_OUT === 1'h0) else $error("RST_OUT active.");
	assert (BSRQ === 1'h0) else $error("BSRQ active.");

	// AY, AX, state
	keyboard_set_switch(3'h5, 4'h8, 1);
	#10;
	assert (NMI === 1) else $error("NMI set active failed.");
	keyboard_set_switch(3'h6, 4'h8, 1);
	#10;
	assert (RST_OUT === 1) else $error("RST_OUT set active failed.");
	keyboard_set_switch(3'h7, 4'h8, 1);
	#10;
	assert (BSRQ === 1) else $error("BSRQ set active failed.");
end

endmodule