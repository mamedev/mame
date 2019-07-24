// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

/***************************************************************************

  Netlist (gtrak10) included from atarittl.cpp

***************************************************************************/

#include "netlist/devices/net_lib.h"

//render a white square with the shape of the car window
//instead of drawing the actual car sprite graphics
//#define CAR_WINDOW_HACK

//generate a regular pattern of squares
// instead of the race track
//#define SIGNAL_RT_HACK
//#define SIGNAL_RT_HACK_XOR

NETLIST_START(gtrak10)

	SOLVER(Solver, 48000)
	PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5)

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	MAINCLOCK(CLOCK, 14318181)

	ALIAS(P, high)
	ALIAS(GROUND, low)

	// Horizontal Counters
	//     name,   CLK,   ENP, ENT,     CLRQ, LOADQ,      A,      B,      C,      D
	TTL_9316(L2, CLOCK,     P,   P, HRESET_Q,     P, GROUND, GROUND, GROUND, GROUND)
	TTL_9316(K2, CLOCK, L2.RC,   P, HRESET_Q,     P, GROUND, GROUND, GROUND, GROUND)
	//        name,   CLK, J, K,     CLRQ
	TTL_74107(K1_A, K2.RC, P, P, HRESET_Q)
	TTL_74107(K1_B,  256H, P, P,        P)
	TTL_7404_INVERT(unlabeled_F1_1, 16H)
	ALIAS(  1H,   L2.QA)
	ALIAS(  2H,   L2.QB)
	ALIAS(  4H,   L2.QC)
	ALIAS(  8H,   L2.QD)
	ALIAS( 16H,   K2.QA)
	ALIAS( 16H_Q, unlabeled_F1_1.Q)
	ALIAS( 32H,   K2.QB)
	ALIAS( 64H,   K2.QC)
	ALIAS(128H,   K2.QD)
	ALIAS(256H,   K1_A.Q)
	ALIAS(512H,   K1_B.Q)
	ALIAS(512H_Q, K1_B.QQ)

	// Horizontal Reset
	TTL_7408_AND(E2_A, 2H, 64H)
	ALIAS(2H_AND_64H, E2_A.Q)
	TTL_7410_NAND(J1_C, 2H_AND_64H, 128H, 256H) // 450th horizontal pixel (active low)
	ALIAS(450H_Q, J1_C.Q)
	//       name,   CLK,      D, CLRQ, PREQ
	TTL_7474(H1_A, CLOCK, 450H_Q,    P,    P)
	ALIAS(HRESET_Q, H1_A.Q)
	ALIAS(HRESET,   H1_A.QQ)

	// Vertical Counters
	//     name,   CLKA, CLKB,     R1,     R2
	TTL_7493(F3, HBLANK,   1V, VRESET, VRESET)
	TTL_7493(H2,     8V,  16V, VRESET, VRESET)
	TTL_7493(I2,   128V, 256V, VRESET, VRESET)
	ALIAS(  1V, F3.QA)
	ALIAS(  2V, F3.QB)
	ALIAS(  4V, F3.QC)
	ALIAS(  8V, F3.QD)
	ALIAS( 16V, H2.QA)
	ALIAS( 32V, H2.QB)
	ALIAS( 64V, H2.QC)
	ALIAS(128V, H2.QD)
	ALIAS(256V, I2.QA)
	ALIAS(512V, I2.QB)

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
	ALIAS(SPEED_PULSE, VRESET) //FIXME!


	// 1STOP_Q Signal:
	//       name,         CLK,      D,   CLRQ, PREQ
	TTL_7474(B2_A, SPEED_PULSE, ATRC_Q, A2_D.Q,    P)
	TTL_7474(B2_B,        VLd1, B2_A.Q,      P,    P)
	TTL_7400_NAND(A2_D, 1STOP_Q, VLd1)
	ALIAS(1STOP_Q, B2_B.Q)


	// TODO: ATRC signal:
	// ...
	ALIAS(ATRC,   GROUND) // FIXME!
	ALIAS(ATRC_Q,      P) // FIXME!


	// TODO: RESET1_Q signal:
	// ...
	ALIAS(RESET1_Q, P) // FIXME!


	// TODO: SCORE_DISP_Q signal:
	// ...
	ALIAS(SCORE_DISP_Q, GROUND) // FIXME!

	// Car direction (?)
	ALIAS(1D0, GROUND) // FIXME!
	ALIAS(1D1, GROUND) // FIXME!
	ALIAS(1D2, GROUND) // FIXME!
	ALIAS(1D3, GROUND) // FIXME!
	ALIAS(1D4, GROUND) // FIXME!

	// Car rotation (?)
	ALIAS(1R0, GROUND) // FIXME!
	ALIAS(1R1, GROUND) // FIXME!
	ALIAS(1R2, GROUND) // FIXME!
	ALIAS(1R3, GROUND) // FIXME!


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

	// --- Addresses 3 and 2 ---
	//        name,  C0,    C1,   C2, C3,    A,   B,      G)
	TTL_74153(K4_A, 1R2,   1D4, 512H, 8V, ABCD, ABG, H3_B.Q)
	TTL_74153(K4_B, 1R1, 1PHI4, 256H, 4V, ABCD, ABG, H3_B.Q)

	// --- Address 4 ---
	TTL_7404_INVERT(E1_C, 32H)
	TTL_7400_NAND(F5_B, E1_C.Q, Ld1B)
	TTL_7402_NOR(H3_B, J4_B.AY, VSYNC_Q)
	TTL_7404_INVERT(L6_E, H3_B.Q)
	TTL_7410_NAND(H5_B, VSYNC, L6_E.Q, 1R3)
	TTL_7400_NAND(F5_A, 16V, ABG)
	TTL_7410_NAND(H5_C, F5_B.Q, H5_B.Q, F5_A.Q)

	// --- Addresses 0 and 1 ---
	//        name,  C0,    C1,   C2, C3,    A,   B,      G)
	TTL_74153(L4_A, 1R0, 1PHI3, 128V,  C, ABCD, ABG, H3_B.Q) // "128A" ?
	TTL_74153(L4_B, 1D0, 1PHI2,  64H,  B, ABCD, ABG, H3_B.Q)
	TTL_7402_NOR(H3_A, VLd1, Ld1B)
	ALIAS(ABG, H3_A.Q)

	// ROM AR (address read) Signal:
	TTL_7400_NAND(F5_D, HSYNC_Q, VLd1)
	TTL_7400_NAND(F5_C, F5_D.Q, 16H_Q)


	ALIAS(1A, GROUND) //FIXME!
	ALIAS(1B, GROUND) //FIXME!
	ALIAS(1C, GROUND) //FIXME!
	ALIAS(1D, GROUND) //FIXME!
	ALIAS(1E, GROUND) //FIXME!
	ALIAS(1F, GROUND) //FIXME!
	ALIAS(1G, GROUND) //FIXME!

	ALIAS(2B, GROUND) //FIXME!
	ALIAS(2C, GROUND) //FIXME!
	ALIAS(2D, GROUND) //FIXME!
	ALIAS(2E, GROUND) //FIXME!
	ALIAS(2F, GROUND) //FIXME!
	ALIAS(2G, GROUND) //FIXME!

	ALIAS(B, GROUND) //FIXME!
	ALIAS(C, GROUND) //FIXME!
	ALIAS(D, GROUND) //FIXME!
	ALIAS(E, GROUND) //FIXME!

	// Actual ROM chip is labeled 74186:
	//        name,     AR, OE1, OE2,      A0,      A1,      A2,      A3,     A4,     A5,      A6,      A7,      A8,     A9,  A10
	ROM_TMS4800(J5, F5_C.Q,   P,   P, L4_B.AY, L4_A.AY, K4_B.AY, K4_A.AY, H5_B.Q, D2_A.Q, J4_A.AY, H4_B.AY, H4_A.AY, L5_C.Q, ABCD)
	PARAM(J5.ROM, "gamedata")
	TTL_7404_INVERT(L6_D, J5.D7) ALIAS(DATA7, L6_D.Q)
	TTL_7404_INVERT(J6_C, J5.D6) ALIAS(DATA6, J6_C.Q)
	TTL_7404_INVERT(J6_B, J5.D5) ALIAS(DATA5, J6_B.Q)
	TTL_7404_INVERT(J6_A, J5.D4) ALIAS(DATA4, J6_A.Q)
	TTL_7404_INVERT(L6_F, J5.D3) ALIAS(DATA3, L6_F.Q)
	TTL_7404_INVERT(J6_F, J5.D2) ALIAS(DATA2, J6_F.Q)
	TTL_7404_INVERT(J6_E, J5.D1) ALIAS(DATA1, J6_E.Q) // Note: J6_E pin numbers on schematics seem wrong (11 and 12)
	TTL_7404_INVERT(J6_D, J5.D0) ALIAS(DATA0, J6_D.Q)

	#if 1 //I'm uncertain if we actually need these:
	RES( R7, RES_K(6.8)) NET_C( R7.1, GROUND) NET_C( R7.2, J5.D7)
	RES( R9, RES_K(6.8)) NET_C( R9.1, GROUND) NET_C( R9.2, J5.D6)
	RES(R11, RES_K(6.8)) NET_C(R11.1, GROUND) NET_C(R11.2, J5.D5)
	RES(R13, RES_K(6.8)) NET_C(R13.1, GROUND) NET_C(R13.2, J5.D4)
	RES(R14, RES_K(6.8)) NET_C(R14.1, GROUND) NET_C(R14.2, J5.D3)
	RES(R12, RES_K(6.8)) NET_C(R12.1, GROUND) NET_C(R12.2, J5.D2)
	RES(R10, RES_K(6.8)) NET_C(R10.1, GROUND) NET_C(R10.2, J5.D1)
	RES( R8, RES_K(6.8)) NET_C( R8.1, GROUND) NET_C( R8.2, J5.D0)
	#endif

	// ----------- Vertical positioning of the car -----------------
	//     name,     EQ, MRQ,    S0Q,    S1Q,    S2Q,    S3Q,    D0,    D1,    D2,    D3
	TTL_9314(H7, VLd1_Q,   P, GROUND, GROUND, GROUND, GROUND, DATA4, DATA5, DATA6, DATA7)
	//     name,    CLK,   ENP,   ENT,     CLRQ,  LOADQ,      A,      B,      C,      D
	TTL_9316(J7, HCOUNT,     P,     P, RESET1_Q, L5_A.Q,  H7.Q0,  H7.Q1,  H7.Q2,  H7.Q3)
	TTL_9316(K7, HCOUNT, J7.RC,     P, RESET1_Q, L5_A.Q,      P,      P,      P,      P)
	TTL_9316(L7, HCOUNT, J7.RC, K7.RC, RESET1_Q, L5_A.Q,      P, GROUND,      P,      P)
	TTL_7400_NAND(L5_A, J7.RC, L7.RC)
	ALIAS(1PHI2, J7.QB)
	ALIAS(1PHI3, J7.QC)
	ALIAS(1PHI4, J7.QD)

	// ------------ Horizontal positioning of the car ----------------------
	//     name,   CLK,   ENP,   ENT,      CLRQ, LOADQ,        A,      B,      C,      D
	TTL_9316(E7, CLOCK,     P,     P,  RESET1_Q,  D5_B.Q, F7_A.Q, F7_D.Q, F7_C.Q, D3_C.Q)
	TTL_9316(D7, CLOCK, E7.RC,     P,  RESET1_Q,  D5_B.Q,      P,      P,      P, GROUND)
	TTL_7400_NAND(D5_B, E7.RC, B6_D.Q)
	TTL_7404_INVERT(B6_D, H5_A.Q)
	TTL_7410_NAND(H5_A, D7.RC, D6_A.Q, D6_B.Q) //weird: should be D6_B.QQ according to schematics
	//        name,    CLK, J, K,     CLRQ
	TTL_74107(D6_A,  D7.RC, P, P, RESET1_Q)
	TTL_74107(D6_B, D6_A.Q, P, P, RESET1_Q)

	TTL_7400_NAND(D3_C, VLd1, DATA3)
	TTL_7408_AND(F7_C,  VLd1, DATA2)
	TTL_7408_AND(F7_D,  VLd1, DATA1)
	TTL_7408_AND(F7_A,  VLd1, DATA0)


	// Car window:
	// this flip-flop is incorrectly labeled "79" in the schematics
	//       name,   CLK,      D, CLRQ,  PREQ
	TTL_7474(A5_B, CLOCK, H5_A.Q,    P, L7.RC)
	ALIAS(CAR_WINDOW,   A5_B.Q)
	ALIAS(CAR_WINDOW_Q, A5_B.QQ)

	// ------------- Car1Video shift registers: -----------
	//      name,   CLK, CLK_INHIBIT_Q,   SH_LDQ,    SER,      A,     B,     C,     D,     E,     F,     G,     H
	TTL_74165(H6, CLOCK,    CAR_WINDOW,  HSYNC_Q,      P,  DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7)
	TTL_74165(F6, CLOCK,    CAR_WINDOW,     Ld1B,  H6.QH,  DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7)
	TTL_7408_AND(F7_B, F6.QHQ, CAR_WINDOW_Q)
#ifdef CAR_WINDOW_HACK
	ALIAS(CAR1VIDEO, CAR_WINDOW_Q)
#else
	ALIAS(CAR1VIDEO, F7_B.Q)
#endif
	TTL_7404_INVERT(L6_A, CAR1VIDEO)
	ALIAS(CAR1VIDEO_Q, L6_A.Q)


	// --------------- Hack ----------------------
	// This is a signal only used for debugging.
	// It generates a pattern of squares on the screen.
	// CAR1VIDEO = not(16H & 16V | VBLANK | HBLANK)

#ifdef SIGNAL_RT_HACK
#ifdef SIGNAL_RT_HACK_XOR
	TTL_7486_XOR(16H_xor_16V, 16H, 16V)
	TTL_7402_NOR(nor, 16H_xor_16V, BLANK)
#else
	TTL_7408_AND(16H_and_16V, 16H, 16V)
	TTL_7402_NOR(nor, 16H_and_16V, BLANK)
#endif
	TTL_7432_OR(BLANK, HBLANK, VRESET)
	ALIAS(RT, nor.Q)
#else
	ALIAS(RT, GROUND) //FIXME!
#endif

	ALIAS(FINISH_LINE, GROUND) //FIXME!
	ALIAS(SLICK_Q, P) //FIXME!

	// ------------- Video Mixer -----------------
	RES(R63, RES_K(1))
	RES(R73, RES_R(330))
	RES(R64, RES_R(330))
	RES(R74, RES_R(330))
	RES(R66, RES_R(330))
	RES(R65, RES_K(4.7))

	NET_C(V5, R63.1)
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

	ALIAS(VIDEO_OUT, RVID.1)
#endif

NETLIST_END()
