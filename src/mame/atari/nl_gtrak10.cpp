// license:BSD-3-Clause
// copyright-holders:Felipe Sanches, Golden Child

/***************************************************************************

  Netlist (gtrak10) included from atarittl.cpp

***************************************************************************/

#include "netlist/devices/net_lib.h"

// Atari Lemans is basically a slightly more advanced version of gran trak 10.
//  The schematics are very similar, and identical in many parts.
//  Lemans has 3 2K ROM chips to allow more tracks and an extended play mode.
//  To fill in some questions about the gtrak10 schematic, referencing the lemans schematic is helpful.


// use this hack to stop horizontal jumping due to netlist timing problems
#define HORIZ_COUNTER_ADJ_HACK  // use 1 less than normal 450, substitute 1V instead of 2V, changes netlist timing

#define GTRAK10_CLOCK 14318181


NETLIST_START(gtrak10)

	// cribbed parameters from Tank
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e-2)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1.0 / GTRAK10_CLOCK)  // needs that timestep or wont render properly
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5)

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	NET_C(VCC, high.VCC, low.VCC)
	NET_C(GND, high.GND, low.GND)

	MAINCLOCK(CLOCK, GTRAK10_CLOCK)

	ALIAS(P, high)
	ALIAS(GROUND, low)

	// COIN SWITCH
	//===============
	SWITCH(COIN1)

	NET_C (COIN1.1, GND)
	ALIAS(CSNC, COIN1.2)

	// do I need a pullup?
	RES(COIN1_PULLUP, RES_R(1000))
	NET_C(COIN1_PULLUP.2, COIN1.2)
	NET_C(COIN1_PULLUP.1, VCC)

	// START SWITCH
	//=================
	SWITCH(STARTSW1)

	NET_C(STARTSW1.1, GND)
	ALIAS(STARTSW1_OUT, STARTSW1.2)

	// do I need a pullup?
	RES(STARTSW1_PULLUP, RES_R(1000))
	NET_C(STARTSW1_PULLUP.2, STARTSW1.2)
	NET_C(STARTSW1_PULLUP.1, VCC)

	TTL_7408_AND(A6_D, STARTSW1_OUT, ATRC)  // A6_D 7408 Pins 11,12,13 is D


	// A7_A and A7_B are a coin accepting filter
	//
	// must have coin active for 128V clocks to pass
	//
	//       name, CLK, D,       CLRQ, PREQ
	TTL_7474(A7_A, 64V, A7_A.QQ, CSNC, P)

	//       name, CLK,     D,       CLRQ, PREQ
	TTL_7474(A7_B, A7_A.QQ, A7_B.QQ, CSNC, P)

	TTL_7408_AND(A6_A, A7_A.Q, A7_B.Q)
	TTL_7404_INVERT(B6_E, A6_A.Q)
	ALIAS(COIN_Q, B6_E.Q)



	// Horizontal Counters
	//     name,   CLK,   ENP, ENT,     CLRQ, LOADQ,      A,      B,      C,      D
	TTL_9316(L2, CLOCK,     P,   P, HRESET_Q,     P, GROUND, GROUND, GROUND, GROUND)
	TTL_9316(K2, CLOCK, L2.RC,   P, HRESET_Q,     P, GROUND, GROUND, GROUND, GROUND)
	//        name,   CLK, J, K,     CLRQ
	TTL_74107(K1_A, K2.RC, P, P, HRESET_Q)
	TTL_74107(K1_B,  256H, P, P,        P)
	TTL_7404_INVERT(E1_F, 16H)
//  TTL_7404_INVERT(unlabeled_F1_1, 16H)   should be labeled E1_F i believe according to lemans
	ALIAS(  1H,   L2.QA)
	ALIAS(  2H,   L2.QB)
	ALIAS(  4H,   L2.QC)
	ALIAS(  8H,   L2.QD)
	ALIAS( 16H,   K2.QA)
	ALIAS( 16H_Q, E1_F.Q)
//  ALIAS( 16H_Q, unlabeled_F1_1.Q)
	ALIAS( 32H,   K2.QB)
	ALIAS( 64H,   K2.QC)
	ALIAS(128H,   K2.QD)
	ALIAS(256H,   K1_A.Q)
	ALIAS(512H,   K1_B.Q)
	ALIAS(512H_Q, K1_B.QQ)

	// Horizontal Reset

#ifdef HORIZ_COUNTER_ADJ_HACK
//#pragma message "USING 1H CLOCK INSTEAD OF 2H CLOCK" HORIZ_COUNTER_ADJ_HACK
//  TTL_7408_AND(E2_A, 1H, 64H)
// seems to need the "delay" of an extra AND gate
	TTL_7408_AND(HACK, 1H, P)        // see if changing it to 1H fixes the car
	TTL_7408_AND(E2_A, HACK.Q, 64H)  // see if changing it to 1H fixes the car
#else
	TTL_7408_AND(E2_A, 2H, 64H)
#endif
	ALIAS(2H_AND_64H, E2_A.Q)
	TTL_7410_NAND(J1_C, 2H_AND_64H, 128H, 256H) // 450th horizontal pixel (active low)

	// 256 + 128 = 384 + 64 = 448 + 2 = 450
	ALIAS(450H_Q, J1_C.Q)
	//       name,   CLK,      D, CLRQ, PREQ
	TTL_7474(H1_A, CLOCK, 450H_Q,    P,    P)
	ALIAS(HRESET_Q, H1_A.Q)
	ALIAS(HRESET,   H1_A.QQ)

	// Vertical Counters
	//     name,   CLKA, CLKB,     R1,     R2
	TTL_7493(F2, HBLANK,   1V, VRESET, VRESET)  // MORE THAN ONE F3, lemans schematic calls this F2
	TTL_7493(H2,     8V,  16V, VRESET, VRESET)
	TTL_7493(J2,   128V, 256V, VRESET, VRESET)
	ALIAS(  1V, F2.QA)
	ALIAS(  2V, F2.QB)
	ALIAS(  4V, F2.QC)
	ALIAS(  8V, F2.QD)
	ALIAS( 16V, H2.QA)
	ALIAS( 32V, H2.QB)
	ALIAS( 64V, H2.QC)
	ALIAS(128V, H2.QD)
	ALIAS(256V, J2.QA)
	ALIAS(512V, J2.QB)

	// Vertical Reset
	TTL_7408_AND(E2_B, 512V, 8V)
	ALIAS(520V, E2_B.Q) // 520th "half-line" (?)
	//       name,      CLK,    D, CLRQ, PREQ
	TTL_7474(H1_B, HRESET_Q, 520V,    P,    P)
	ALIAS(VRESET,   H1_B.Q)
	ALIAS(VRESET_Q, H1_B.QQ)

	// Horizontal Blank Flip-Flop:
	TTL_7402_NOR(F1_C, 32H, HBLANK_Q)
	TTL_7402_NOR(F1_B, HRESET, HBLANK)
	ALIAS(HBLANK,   F1_C.Q)
	ALIAS(HBLANK_Q, F1_B.Q)

	// Vertical Sync Flip-Flop:
	TTL_7402_NOR(F1_D, 8V, VSYNC_Q)
	TTL_7402_NOR(F1_A, VRESET, VSYNC)
	ALIAS(VSYNC,   F1_D.Q)
	ALIAS(VSYNC_Q, F1_A.Q)

	// Horizontal Sync = HBLANK & /HRESET & /512H
	TTL_7410_NAND(J1_B, HBLANK, HRESET_Q, 512H_Q)
	TTL_7404_INVERT(E1_E, HSYNC_Q)
	ALIAS(HSYNC,   E1_E.Q)
	ALIAS(HSYNC_Q, J1_B.Q)

	// Composite Sync = /HSYNC XOR /VSYNC
	TTL_7486_XOR(D2_B, HSYNC_Q, VSYNC)
	TTL_7404_INVERT(C2_C, COMP_SYNC)
	ALIAS(COMP_SYNC,   D2_B.Q)
	ALIAS(COMP_SYNC_Q, C2_C.Q)


	// VLd1 signal (vertical load one)
	// Used to address the ROM for the vertical positioning of the car image
	//       name,      CLK,      D, CLRQ,   PREQ
	TTL_7474(B1_B, VRESET_Q, GROUND,    P, VLd1_Q)
	TTL_7474(B1_A,    HSYNC, B1_B.Q,    P,      P)
	ALIAS(VLd1_Q, B1_A.Q)
	ALIAS(VLd1,   B1_A.QQ)


	// Ld1B signal (load one B):
	//       name,   CLK, D,    CLRQ,    PREQ
	TTL_7474(D1_A, 16H_Q, P, HSYNC_Q, VSYNC_Q)
	TTL_7404_INVERT(E1_D, Ld1B_Q)
	ALIAS(Ld1B,   E1_D.Q)
	ALIAS(Ld1B_Q, D1_A.Q)


	// HCOUNT signal:
	//        name, CLK,      J,        K,  CLRQ
	TTL_74107(L1_A,  1H,      P,  L1_B.QQ,  C9.2)
	TTL_74107(L1_B,  1H, L1_A.Q,   GROUND,  C9.2)
	CAP(C9, CAP_P(330))
	RES(R1, RES_R(330))
	RES(R2, RES_R(330))
	NET_C(C9.1, HSYNC_Q)
	NET_C(R1.1, V5)
	NET_C(C9.2, R1.2)
	NET_C(C9.2, R2.1)
	NET_C(R2.2, GROUND)
	ALIAS(HCOUNT, L1_A.QQ)


	// TODO: SPEED_PULSE
	// This signal is generated from one of the proprietary Atari chips.
	// Until we model that, I'll keep a place-holder here
	// corresponding to the typical frequency you'd see when the car is
	// running on the 3rd gear during gameplay.

	// The speed pulse is currently a hack to test the motion of the car.

	ALIAS(SPEED_PULSE, SPEED_PULSES.Q)
	TTL_7408_AND(SPEED_PULSES, P1_LEFT_UP.2, 512V)  // clock with 512V to generate pulses

	NET_C(P1_LEFT_UP.1, SWITCH_R.2)
	NET_C(SWITCH_R.1, VCC)

	RES(SWITCH_R, RES_R(100))
	SWITCH(P1_LEFT_UP)

	RES(SWITCH_R_GND, RES_R(1000))
	NET_C(SWITCH_R_GND.1, GND)
	NET_C(P1_LEFT_UP.2, SWITCH_R_GND.2)



	// 1STOP_Q Signal:
	//       name,         CLK,      D,   CLRQ, PREQ
	TTL_7474(B2_A, SPEED_PULSE, ATRC_Q, A2_D.Q,    P)
	TTL_7474(B2_B,        VLd1, B2_A.Q,      P,    P)
	TTL_7400_NAND(A2_D, 1STOP_Q, VLd1)
	ALIAS(1STOP_Q, B2_B.Q)


	TTL_7474(A4_A, VLd1_Q, P, A6_B.Q, P)
	ALIAS(RESET1_Q, A4_A.Q)
	ALIAS(RESET1,   A4_A.QQ)

	TTL_7400_NAND(A2_B, CAR1VIDEO, COMP_SYNC_Q)
	TTL_7408_AND(A6_B, START_Q, A2_B.Q)

	ALIAS(1D0, GROUND) // FIXME!

	// ------ Car Direction and Rotation --------
	// ==========================================
	// 74193 4 bit up/down counter with load
	// TTL_74193(name,  A, B, C, D, CLEAR, LOADQ, CU,             CD)
	TTL_74193(TEST_ROT, P, P, P, P, GROUND, P,    RIGHT_PULSES.Q, LEFT_PULSES.Q)

	TTL_7400_NAND(LEFT_PULSES, P1_LEFT_LEFT_HIGH.Q, 512V)
	TTL_7400_NAND(RIGHT_PULSES, P1_LEFT_RIGHT_HIGH.Q, 512V)

	TTL_7404_INVERT(P1_LEFT_LEFT_HIGH, P1_LEFT_LEFT.2)
	TTL_7404_INVERT(P1_LEFT_RIGHT_HIGH, P1_LEFT_RIGHT.2)

	SWITCH(P1_LEFT_LEFT)
	SWITCH(P1_LEFT_RIGHT)

	// do I really need all these pullups?
	// don't know exactly how netlist switches work, so just adding extra resistors

	NET_C(P1_LEFT_LEFT_GND.1, GND)
	NET_C(P1_LEFT_RIGHT_GND.1, GND)

	NET_C(P1_LEFT_LEFT.1, P1_LEFT_LEFT_GND.2)
	NET_C(P1_LEFT_RIGHT.1, P1_LEFT_RIGHT_GND.2)

	NET_C(P1_LEFT_LEFT.2, P1_LEFT_LEFT_PULLUP.2)
	NET_C(P1_LEFT_RIGHT.2, P1_LEFT_RIGHT_PULLUP.2)

	RES(P1_LEFT_LEFT_PULLUP, RES_R(1000))
	RES(P1_LEFT_RIGHT_PULLUP, RES_R(1000))

	RES(P1_LEFT_LEFT_GND, RES_R(100))
	RES(P1_LEFT_RIGHT_GND, RES_R(100))

	NET_C(P1_LEFT_LEFT_PULLUP.1, VCC)
	NET_C(P1_LEFT_RIGHT_PULLUP.1, VCC)

	ALIAS(1R0, TEST_ROT.QA)
	ALIAS(1R1, TEST_ROT.QB)
	ALIAS(1R2, TEST_ROT.QC)
	ALIAS(1R3, TEST_ROT.QD)

	ALIAS(1D1, TEST_ROT.QA)
	ALIAS(1D2, TEST_ROT.QB)
	ALIAS(1D3, TEST_ROT.QC)
	ALIAS(1D4, TEST_ROT.QD)

	// ROM multiplexers:
	// --- Addresses 10 and 9 ---
	TTL_7400_NAND(L5_B, Ld1B_Q, SCORE_DISP_Q)
	TTL_7400_NAND(L5_C, Ld1B_Q, L5_D.Q) //labeled "[74]08" on schematics!
	TTL_7400_NAND(L5_D, GROUND, SCORE_DISP_Q)
	ALIAS(ABCD, L5_B.Q)

	// --- Addresses 8 and 7 ---
	//        name, C0, C1,  C2,   C3,      A,            B,      G)
	TTL_74153(H4_A, 1G, 2G, 1D3, 256V, H3_D.Q, SCORE_DISP_Q, GROUND)
	TTL_74153(H4_B, 1F, 2F, 1D2, 128V, H3_D.Q, SCORE_DISP_Q, GROUND)
	TTL_7402_NOR(H3_D, Ld1B, H3_C.Q)
	TTL_7402_NOR(H3_C, 512H, SCORE_DISP_Q)

	// --- Addresses 6 and 5 ---
	//        name,      C0,  C1,  C2, C3,    A,   B,      G)
	TTL_74153(J4_A,     64V, 1D1, 64V,  E, ABCD, ABG, GROUND)
	TTL_74153(J4_B, 1STOP_Q, 1D0, 32V,  D, ABCD, ABG, GROUND)
	TTL_7486_XOR(D2_A, VSYNC, J4_B.AY)

	// --- Address 4 ---
	TTL_7404_INVERT(E1_C, 32H)
	TTL_7400_NAND(F5_B, E1_C.Q, Ld1B)
	TTL_7402_NOR(H3_B, J4_B.AY, VSYNC_Q)
	TTL_7404_INVERT(L6_E, H3_B.Q)
	TTL_7410_NAND(H5_B, VSYNC, L6_E.Q, 1R3)
	TTL_7400_NAND(F5_A, 16V, ABG)
	TTL_7410_NAND(H5_C, F5_B.Q, H5_B.Q, F5_A.Q)

	// --- Addresses 3 and 2 ---
	//        name,  C0,    C1,   C2, C3,    A,   B,      G)
	TTL_74153(K4_A, 1R2,   1D4, 512H, 8V, ABCD, ABG, H3_B.Q)
	TTL_74153(K4_B, 1R1, 1PHI4, 256H, 4V, ABCD, ABG, H3_B.Q)

	// --- Addresses 0 and 1 ---
	//        name,  C0,    C1,   C2, C3,    A,   B,      G)
	TTL_74153(L4_A, 1R0, 1PHI3, 128H,  C, ABCD, ABG, H3_B.Q)
	TTL_74153(L4_B, 1D0, 1PHI2,  64H,  B, ABCD, ABG, H3_B.Q)

	TTL_7402_NOR(H3_A, VLd1, Ld1B)
	ALIAS(ABG, H3_A.Q)


	// ROM AR (address read) Signal:
	TTL_7400_NAND(F5_D, HSYNC_Q, VLd1)
	TTL_7400_NAND(F5_C, F5_D.Q, 16H_Q)

	TTL_7404_INVERT(ARINVERT, AR)  // do we need to invert AR?

	ALIAS(AR, F5_C.Q)
	ALIAS(AR_Q, ARINVERT.Q)


	// Actual ROM chip is labeled 74186:
	// (MK28000,   "+OE1,+OE2,+ARQ,+A1,+A2,+A3,+A4,+A5,+A6,+A7,+A8,+A9,+A10,+A11,@VCC,@GND")
	PROM_MK28000(J5, P,   P, AR, L4_B.AY, L4_A.AY, K4_B.AY, K4_A.AY, H5_C.Q, D2_A.Q, J4_A.AY, H4_B.AY, H4_A.AY, L5_C.Q, ABCD)

	PARAM(J5.ROM, "gamedata")

	ALIAS(A0, L4_B.AY)
	ALIAS(A1, L4_A.AY)
	ALIAS(A2, K4_B.AY)
	ALIAS(A3, K4_A.AY)
	ALIAS(A4, H5_C.Q)
	ALIAS(A5, D2_A.Q)
	ALIAS(A6, J4_A.AY)
	ALIAS(A7, H4_B.AY)
	ALIAS(A8, H4_A.AY)
	ALIAS(A9, L5_C.Q)
	ALIAS(A10, ABCD)

	TTL_7404_INVERT(L6_D, J5.O8) ALIAS(DATA7, L6_D.Q)
	TTL_7404_INVERT(J6_C, J5.O7) ALIAS(DATA6, J6_C.Q)
	TTL_7404_INVERT(J6_B, J5.O6) ALIAS(DATA5, J6_B.Q)
	TTL_7404_INVERT(J6_A, J5.O5) ALIAS(DATA4, J6_A.Q)
	TTL_7404_INVERT(L6_F, J5.O4) ALIAS(DATA3, L6_F.Q)
	TTL_7404_INVERT(J6_F, J5.O3) ALIAS(DATA2, J6_F.Q)
	TTL_7404_INVERT(J6_E, J5.O2) ALIAS(DATA1, J6_E.Q) // Note: J6_E pin numbers on schematics seem wrong (11 and 12)
	TTL_7404_INVERT(J6_D, J5.O1) ALIAS(DATA0, J6_D.Q)

	#if 1 //I'm uncertain if we actually need these:
	RES( R7,  RES_K(6.8)) NET_C( R7.1,  GROUND) NET_C( R7.2,  J5.O8)  // ACTUALLY CONNECTED TO -12V not ground
	RES( R9A, RES_K(6.8)) NET_C( R9A.1, GROUND) NET_C( R9A.2, J5.O7)  // RENAMED TO R9A to avoid naming conflict with lemans
	RES(R11, RES_K(6.8)) NET_C(R11.1, GROUND) NET_C(R11.2, J5.O6)
	RES(R13, RES_K(6.8)) NET_C(R13.1, GROUND) NET_C(R13.2, J5.O5)
	RES(R14, RES_K(6.8)) NET_C(R14.1, GROUND) NET_C(R14.2, J5.O4)
	RES(R12, RES_K(6.8)) NET_C(R12.1, GROUND) NET_C(R12.2, J5.O3)
	RES(R10, RES_K(6.8)) NET_C(R10.1, GROUND) NET_C(R10.2, J5.O2)
	RES( R8, RES_K(6.8)) NET_C( R8.1, GROUND) NET_C( R8.2, J5.O1)
	#endif

	// ----------- Vertical positioning of the car -----------------
	//     name,     EQ, MRQ,    S0Q,    S1Q,    S2Q,    S3Q,    D0,    D1,    D2,    D3
	TTL_9314(H7, VLd1_Q,   P, GROUND, GROUND, GROUND, GROUND, DATA4, DATA5, DATA6, DATA7)  // 4 bit latch for ROM data
	//     name,    CLK,   ENP,   ENT,     CLRQ,  LOADQ,      A,      B,      C,      D
	TTL_9316(J7, HCOUNT,     P,     P, RESET1_Q, L5_A.Q,  H7.Q0,  H7.Q1,  H7.Q2,  H7.Q3)  // bits 0-3 of vpos
	TTL_9316(K7, HCOUNT, J7.RC,     P, RESET1_Q, L5_A.Q,      P,      P,      P,      P)  // bits 4-7 of vpos
	TTL_9316(L7, HCOUNT, J7.RC, K7.RC, RESET1_Q, L5_A.Q,      P, GROUND,      P,      P)  // bits 8-11 of vpos
	TTL_7400_NAND(L5_A, J7.RC, L7.RC)
	ALIAS(1PHI2, J7.QB)
	ALIAS(1PHI3, J7.QC)
	ALIAS(1PHI4, J7.QD)

	// ------------ Horizontal positioning of the car ----------------------
	//     name,   CLK,   ENP,   ENT,      CLRQ, LOADQ,        A,      B,      C,      D
	TTL_9316(D7, CLOCK, E7.RC,     P,  RESET1_Q,  D5_B.Q,      P,      P,      P, GROUND)  // bits 4-7 of hpos
	TTL_9316(E7, CLOCK,     P,     P,  RESET1_Q,  D5_B.Q, F7_A.Q, F7_D.Q, F7_C.Q, D3_C.Q)  // bits 0-3 of hpos

	TTL_7400_NAND(D5_B, E7.RC, B6_D.Q)
	TTL_7404_INVERT(B6_D, H5_A.Q)
	TTL_7410_NAND(H5_A, D7.RC, D6_A.Q, D6_B.Q) //weird: should be D6_B.QQ according to schematics

	//        name, CLK,    J, K,     CLRQ
	TTL_74107(D6_A, D7.RC,  P, P, RESET1_Q)  // bit 8 of hpos
	TTL_74107(D6_B, D6_A.Q, P, P, RESET1_Q)  // bit 9 of hpos

	TTL_7400_NAND(D3_C, VLd1, DATA3)
	TTL_7408_AND(F7_C,  VLd1, DATA2)
	TTL_7408_AND(F7_D,  VLd1, DATA1)
	TTL_7408_AND(F7_A,  VLd1, DATA0)

	// aliases for Car Horizontal Position Bits (10 bits of hpos)
	ALIAS(CARH0, E7.QA)
	ALIAS(CARH1, E7.QB)
	ALIAS(CARH2, E7.QC)
	ALIAS(CARH3, E7.QD)
	ALIAS(CARH4, D7.QA)
	ALIAS(CARH5, D7.QB)
	ALIAS(CARH6, D7.QC)
	ALIAS(CARH7, D7.QD)
	ALIAS(CARH8, D6_A.Q)
	ALIAS(CARH9, D6_B.Q)

	// Car window:
	// this flip-flop is incorrectly labeled "79" in the schematics  (labeled correctly in Lemans schematic)
	//       name,   CLK,      D, CLRQ,  PREQ
	TTL_7474(A5_B, CLOCK, H5_A.Q,    P, L7.RC)

	ALIAS(CAR_WINDOW,   A5_B.Q)
	ALIAS(CAR_WINDOW_Q, A5_B.QQ)

	// ------------- Car1Video shift registers: -----------
	// Two 8 bit shift registers to make up 16 bits of car graphics
	//      name,   CLK, CLK_INHIBIT_Q,   SH_LDQ,    SER,      A,     B,     C,     D,     E,     F,     G,     H
	TTL_74165(H6, CLOCK,    CAR_WINDOW,  HSYNC_Q,      P,  DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7)
	TTL_74165(F6, CLOCK,    CAR_WINDOW,   Ld1B_Q,  H6.QH,  DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7)
	TTL_7408_AND(F7_B, F6.QHQ, CAR_WINDOW_Q)
	ALIAS(CAR1VIDEO, F7_B.Q)

	TTL_7404_INVERT(L6_A, CAR1VIDEO)
	ALIAS(CAR1VIDEO_Q, L6_A.Q)

	ALIAS(RT, C6_C.Q)  // Racetrack data out

	// ------------- Video Mixer -----------------
	RES(R63, RES_K(1))
//  RES(R73, RES_R(150))
	RES(R73, RES_R(330))
	RES(R64, RES_R(330))
	RES(R74, RES_R(330))
	RES(R66, RES_R(330))
	RES(R65, RES_K(4.7))

	NET_C(VCC, R63.1)
//  NET_C(GROUND, R63.1)  // CONNECT TO GROUND?
//  NET_C(GROUND, R63.2)  // CONNECT TO GROUND?
	NET_C(COMP_SYNC, R73.1)
	NET_C(CAR1VIDEO, R64.1)
	NET_C(FINISH_LINE, R74.1)
	NET_C(RT, R66.1)
	NET_C(SLICK_Q, R65.1)

#if 0
	CAP(C44, CAP_U(10))
	NET_C(C44.1, R63.2)
	NET_C(C44.1, R73.2)
	NET_C(C44.1, R64.2)
	NET_C(C44.1, R74.2)
	NET_C(C44.1, R66.2)
	NET_C(C44.1, R65.2)
	ALIAS(VIDEO_OUT, C44.2)
#else
	// A capacitor just kills performance completely
	RES(RVID, RES_K(10))
	NET_C(GND, RVID.2)

	NET_C(RVID.1, R63.2, R73.2, R64.2, R74.2, R66.2, R65.2)
//  NET_C(RVID.1, R73.2, R64.2, R74.2, R66.2, R65.2)

	ALIAS(VIDEO_OUT, RVID.1)
	ALIAS(COMP_SYNC_2, D2_B.Q)
#endif


	// ---------------- RACETRACK DATA ------------------------
	// ========================================================
	//  74165  8 bit shift register that holds 8 bits of Racetrack data
	//
	// TTL_74165(name, CLK, CLKINH, SH_LDQ, SER, A, B, C, D, E, F, G, H)
	TTL_74165(E6, 4H_Q, GROUND, RT_CLOCK_Q, P, DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7)

	TTL_7404_INVERT(E1_A, 4H)
	ALIAS (4H_Q, E1_A.Q)

	TTL_7400_NAND(D3_B, Ld1B_Q, CAP4.2)
	CAP(CAP4, CAP_P(100))
	RES(R9, RES_R(470))
	NET_C(CAP4.1, 32H)
	NET_C(CAP4.2, R9.2)
	NET_C(R9.1, GROUND)

	ALIAS(RT_CLOCK_Q, D3_B.Q)

	// ----- VBLANK -----
	TTL_7404_INVERT(N2_D, 512V)
	ALIAS(512V_Q, N2_D.Q)
	// 7474: name,   CLK,  D, CLRQ, PREQ
	TTL_7474(D1_B, 32V, P, 512V_Q, P)
	ALIAS(VBLANK, D1_B.QQ)
	ALIAS(VBLANK_Q, D1_B.Q)

	// ----------ROM Data Decode Special Bit Patterns-------------
	// 9314 Quad Latch used to latch a single racetrack data byte
	// Special bit patterns signify special handling.
	// =========================
	// 0 3 not 1 is special data  (that is one of the finish,slick,checkpoint or score)
	// bits 4 and 2 determine the special type:
	// 2 not 4 = slick
	// 2 4 = finish line
	// not 2 4 = checkpoint
	// not 2 not 4 = score
	// =========================
	// 65 32 0 is a slick       pattern 01 (24)  not 2 4 reversed = not 4 2
	// 65432 0 is finish line   pattern 00 (24)  not 2 not 4 reversed = 4 2
	// checkpoint uses bit 6 and 5 to indicate checkpoint number
	// 6543  0 is checkpoint   65 is checkpoint 1
	// 6 43  0 is checkpoint   6  is checkpoint 2
	//  543  0 is checkpoint    5 is checkpoint 3
	//   43  0 is checkpoint      is checkpoint 4
	// checkpoint is pattern 10 (24) 2 not 4 reversed = 4 not 2 and 65 determine checkpoint number
	//   3  0 is score   pattern 11 (24) 2 4  reversed is not 4 not 2
	// =========================
	// TTL_9314(name, EQ, MRQ, S0Q, S1Q, S2Q, S3Q, D0, D1, D2, D3)
	TTL_9314(E5, RT_CLOCK_Q, VBLANK_Q, GROUND, GROUND, GROUND, GROUND, DATA2, DATA6, DATA5, DATA4)

	// DATA2 = Q0
	// DATA6 = Q1
	// DATA5 = Q2
	// DATA4 = Q3

	// ---- Decode Score Display, Checkpoint, Slick, Finish Line ----
	// usage       : TTL_9321(name, E, A0, A1)
	// 9321  Q3 (9314/10) 14 = A , 13 = B  (9314/15) Q0
	// Decodes B = Q0 (DATA2) and A = Q3 (DATA4) for special type

	TTL_9321(E4_B, SPECIAL_DATA_Q, E5.Q3, E5.Q0)  // OUTPUT D0-D3
	// Labeled in both schematics as E4 9321

	// ---- Decode Checkpoint 1 to 4 ----
	// 9321  Q2 (9314/12) 2 = A , 3 = B   (9314/13) Q1
	// Decodes B = Q1 (DATA6) and A = Q2 (DATA5)

	TTL_9321(E4_A, CHECKPOINT_Q, E5.Q2, E5.Q1)
	// Labeled in both schematics as E3 9321, circuit board labels it E4 9321, must be E4

	// ---------- Checkpoint Scoring Counter -------------
	// TTL_9314(name, EQ, MRQ, S0Q, S1Q, S2Q, S3Q, D0, D1, D2, D3)
	TTL_9314(F3, CAR1VIDEO_Q, START_Q, CHECK1, CHECK2, CHECK3, CHECK4, C2_F.Q, F3.Q0, F3.Q1, F3.Q2 )
	// lemans says CAR1VIDEO_Q, gran trak 10 says CAR1VIDEO

	NET_C(E4_A.VCC, P)
	NET_C(E4_A.GND, GROUND)

	ALIAS(CHECK4, E4_A.D3)
	ALIAS(CHECK3, E4_A.D2)
	ALIAS(CHECK2, E4_A.D1)
	ALIAS(CHECK1, E4_A.D0)

	ALIAS(1D, F3.Q3)
	ALIAS(1C, F3.Q2)
	ALIAS(1B, F3.Q1)
	ALIAS(1A, F3.Q0)

	ALIAS(SCORE_DISP_Q,  E4_B.D3)
	ALIAS(CHECKPOINT_Q,  E4_B.D2)
	ALIAS(SLICK_Q,       E4_B.D1)
	ALIAS(FINISH_LINE_Q, E4_B.D0)

	TTL_7404_INVERT(C2_F, B3_C.Q)
	TTL_7404_INVERT(C2_E, 1D)
	ALIAS(1D_Q, C2_E.Q)

	TTL_7402_NOR(B3_C, 1D_Q, FINISH_LINE_Q)
	TTL_7402_NOR(B3_D, RT_CLOCK_Q, FINISH_LINE_Q)

	TTL_7408_AND(A6_C, 4V, B3_D.Q)
	ALIAS(FINISH_LINE, A6_C.Q)

	// TTL_7493(name, CLKA, CLKB, R1, R2)
	TTL_7493(K5, 1SEC, 2SEC, GROUND, GROUND)

	ALIAS(2SEC, K5.QA)
	ALIAS(4SEC, K5.QB)

	TTL_7493(F4, 1D,  F4.QA, START, START)

	ALIAS(1E, F4.QA)
	ALIAS(1F, F4.QB)
	ALIAS(1G, F4.QC)

	// TTL_74193(name,   A,  B,  C,  D,     CLEAR, LOADQ, CU,   CD)
	TTL_74193(J5_LEMANS, 1E, 1F, 1G, F4.QD, P,     ATRC,  4SEC, P)
	// SHOULD BE CALLED J5 in lemans, rename it to J5_LEMANS to avoid name conflict
	ALIAS(TSA, J5_LEMANS.QA)
	ALIAS(TSB, J5_LEMANS.QB)
	ALIAS(TSC, J5_LEMANS.QC)

	// B3_A SCORE DELAY   123
	//  TTL_7402_NOR(B3_A,
	// B3_B  4V output from J1 7410     456

	// B3_C    8 9 10
	// B3_D    11 12 13

	TTL_7410_NAND(J1_A, 4H_Q, 8V, SPECIAL_DATA_Q) // 450th horizontal pixel (active low)
	TTL_7402_NOR(B3_B, J1_A.Q, 4V)
	TTL_7402_NOR(B3_A, B3_B.Q, SCORE_DELAY)

	ALIAS(SCORE_DELAY, C3_B.QQ)

	//       name,   CLK,      D, CLRQ, PREQ
	TTL_7474(C3_B, 32H, SCORE_DISP_Q,    P,    P)   // C3_B is SCORE DELAY latch
	TTL_7474(C3_A, 32H, D3_D.Q,    P,    Ld1B_Q)    // C3_A is SPECIAL_DATA_Q Latch

	// computing SPECIAL DATA = (0 3 NOT 1) in ROM DATA.
	// inverted it becomes NOT 0 NOT 3 1
	// same as 1 AND (NOT 0 AND NOT 3)
	// same as 1 AND NOT ( NOT ( NOT 0 AND NOT 3 ))  -> use rule (NOT (A AND B)) =  (NOT A OR NOT B)
	// same as 1 AND NOT( 0 OR 3))
	// same as 1 AND (0 NOR 3)
	// we invert it for SPECIAL_DATA_Q, so it's (DATA1 NAND (DATA0 NOR DATA3))

	TTL_7400_NAND(D3_D, C6_B.Q, DATA1)  // output gets clocked into 7474 D-Latch C3_A
	TTL_7402_NOR(C6_B, DATA0, DATA3)


	ALIAS(SPECIAL_DATA_Q, C3_A.Q)

	TTL_7402_NOR(C6_C, E6.QH, B3_A.Q)

	TTL_7400_NAND(D5_A, RT, A5_B.QQ)


	//  7474 name, CLK,   D,       CLRQ, PREQ
	TTL_7474(A5_A, 512V, A5_A.QQ,    P,    P)
	ALIAS(1024V, A5_A.Q)

	TTL_7474(B5_A, 1024V, P,   D5_A.Q,  P)

	TTL_7474(B5_B, 1024V, B5_A.Q,   D5_A.Q,  P)

	TTL_7474(C5_B, D5_D.Q, CAR1VIDEO, B5_B.QQ,  ATRC_Q)
	TTL_7474(C5_A, D5_D.Q, CAR1VIDEO, 1SEC,     P)


	ALIAS(CRASH_A,   C5_B.Q)
	ALIAS(CRASH_A_Q, C5_B.QQ)

	TTL_7400_NAND(D5_C, CRASH_A_Q, ATRC_Q)
	ALIAS(SPEED_KILL, D5_C.Q)
	TTL_7400_NAND(D5_D, CRASH_SEQUENCE_Q, RT)


	// 4 bit multiplexer to handle the score and the timer
	// Note that we have to "fix" 1B and 1C
	// count pattern:
	// DCBA -> D__x
	// 000x -> 000x   0
	// 001x -> 001x   2
	// 010x -> 010x   4
	// 011x -> 011x   6
	// 100x -> 100x   8
	//
	// Since we are counting by twos, we ignore the lowest bit, 1A.
	//
	//  TTL_9322(name, SELECT, A1, B1, A2, B2, A3, B3, A4, B4, STROBE)
	TTL_9322(D4, 512H, 1B_FIX, 2B, 1C_FIX, 2C, 1D, 2D, 1E, 2E, GROUND)

	ALIAS(B, D4.Y1)
	ALIAS(C, D4.Y2)
	ALIAS(D, D4.Y3)
	ALIAS(E, D4.Y4)

	TTL_7400_NAND(A2_C, 1B, 1C_Q)  // 7400 PINS 8,9,10 IS C
	TTL_7404_INVERT(C2_A, 1C)
	ALIAS(1C_Q, C2_A.Q)

	TTL_7402_NOR(A3_C, 1A, 1B)  // 7402 PINS 8910 IS C

	TTL_7486_XOR(D2_D, A2_C.Q, A3_C.Q)  // 7486 PINS 11,12,13 IS D

	TTL_7408_AND(E2_C, 1B, 1D_Q)      // 7408 PINS 8,9,10   IS C
	TTL_7408_AND(E2_D, D2_D.Q, 1D_Q)  // 7408 PINS 11,12,13 IS D
	ALIAS(1C_FIX, E2_C.Q)
	ALIAS(1B_FIX, E2_D.Q)


	//  74192(name, A, B, C, D,                   CLEAR, LOADQ, CU, CD)
//  TTL_74192(C4, GROUND, GROUND, GROUND, P,      ATRC, START_Q , P,  1SEC)        // LOAD WITH 8  // lemans connected to 1SEC
	TTL_74192(C4, GROUND, GROUND, GROUND, P,      ATRC, START_Q , P,  D7_D.Q)      // LOAD WITH 8  // gran trak has a 7402 NOR gate
	TTL_74192(B4, P,      P,      P,      GROUND, ATRC, START_Q , P,  C4.BORROWQ)  // LOAD WITH 7

	ALIAS(ENDGAME, B4.BORROWQ)

	ALIAS(2B, C4.QB)
	ALIAS(2C, C4.QC)
	ALIAS(2D, C4.QD)
	ALIAS(2E, B4.QA)
	ALIAS(2F, B4.QB)
	ALIAS(2G, B4.QC)


	//  7474 name,   CLK,      D, CLRQ, PREQ
	TTL_7474(C7_B, 512V, A6_D.Q, C6_D.Q, P)

	ALIAS(START,   C7_B.Q)
	ALIAS(START_Q, C7_B.QQ)

	//  7474 name, CLK,      D, CLRQ, PREQ
	TTL_7474(C7_A, ENDGAME, P, START_Q, Q) // should be Q, TODO FIX Q

	ALIAS(Q, P) // TODO FIX Q

	ALIAS(ATRC,   C7_A.Q)
	ALIAS(ATRC_Q, C7_A.QQ)

	TTL_7402_NOR(C6_D, CR_Q, CSNC)


	//       name,   CLK,      D, CLRQ, PREQ
	TTL_7474(B7_A, ATRC, B7_B.Q, COIN_Q, Q)

	TTL_7474(B7_B, ATRC, P, EXTPLAY_Q, Q)

	ALIAS(CR_Q, B7_A.Q)
	ALIAS(CR,   B7_A.QQ)


	ALIAS(CRASH_SEQUENCE_Q, P) // FIX
	ALIAS(EXTPLAY_Q, P)


	// 1SEC COUNTER 555
	// ================

	NE555_DIP(B8)
	RES(R72, RES_K(10))  // should be a potentiometer 1M // lemans calls it R40  // Less resistance, faster count
	RES(R71, RES_K(20))  // lemans calls it R39  // should be 220K , we'll set to less because we want to see it count
	//RES(R71, RES_K(220))  // lemans calls it R39  // should be 220K
	RES(R70, RES_K(6.8))  // lemans calls it R35
	RES(1SEC_PULLDOWN, RES_K(1))  // lemans calls it R29, gran trak has it unlabeled
	CAP(C42, CAP_U(0.1)) // assume uF  // lemans calls it C22
	CAP(C43, CAP_U(4.7))  // lemans calls it C23

	ALIAS(1SEC, B8.3)

	NET_C(B8.5, C42.1)
	NET_C(C42.2, GND)

	NET_C(B8.2, B8.6, R70.2, C43.1)
	NET_C(C43.2, GND)  // polarized, is this correct direction?

	NET_C(B8.7, R70.1, R71.2)
	NET_C(R72.2, R71.1)
	NET_C(R72.1, VCC)
	NET_C(B8.1, GND)

	NET_C(B8.8, VCC)
	NET_C(B8.4, START_Q)  // lemans has it connected to START_Q, gran trak 10 connected to S1A

	NET_C(B8.3, 1SEC_PULLDOWN.1)
	NET_C(1SEC_PULLDOWN.2, GND)

	CAP(C5, CAP_U(.01))  // lemans calls it as C52 .001 uF and directly connected to 1SEC

	NET_C(D7_D.Q, C5.1)
	NET_C(C5.2, GND)

	TTL_7402_NOR(D7_D, 1SEC, 1SEC)  // only on gran trak 10, lemans directly connected to 1SEC


// MINI PIN REFERENCE to assign consistent sub letters to DIP
//===========================================================
// 7400 NAND A 123 B 456 C 8910 D 11 12 13
// 7402 NOR  A 123 B 456 C 8910 D 11 12 13
// 7404 NOT  A 12 B 34 C 56 D 89 E 10 11 F 12 13
// 7408 AND  A 123 B 456 C 8910 D 11 12 13
// 7410 A 1,2,12,13  B 3456 C 8 9 10 11
// 7432 OR   A 123 B 456 C 8910 D 11 12 13
// 7474 A 123456 B 8 9 10 11 12 13
// 7486 XOR A 123 B 456 C 8910 D 11 12 13


NETLIST_END()
