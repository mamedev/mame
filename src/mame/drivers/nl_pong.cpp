// license:GPL-2.0+
// copyright-holders: Couriersud

/***************************************************************************

  Netlist (pong) included from pong.c

***************************************************************************/

#include "netlist/devices/net_lib.h"

#define FAST_CLOCK  (1)

#ifndef __PLIB_PREPROCESSOR__
#if 0
#define TTL_7400A_NAND(_name, _A, _B) TTL_7400_NAND(_name, _A, _B)
#else
#define TTL_7400A_NAND(_name, _A, _B)                                          \
		NET_REGISTER_DEV(TTL_7400A_NAND, _name)                              \
		NET_CONNECT(_name, A, _A)                                              \
		NET_CONNECT(_name, B, _B)
#endif
#endif

NETLIST_START(lib)
	TRUTHTABLE_START(TTL_7400A_NAND, 2, 1, 0, "A,B")
		TT_HEAD(" A , B | Q ")
		TT_LINE(" 0 , X | 1 |22")
		TT_LINE(" X , 0 | 1 |22")
		TT_LINE(" 1 , 1 | 0 |15")
	TRUTHTABLE_END()
NETLIST_END()

NETLIST_START(pong_fast)

	LOCAL_SOURCE(lib)

	INCLUDE(lib)
	SOLVER(Solver, 48000)
	PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient
	PARAM(Solver.LTE,     1e-4) // Default is not enough for paddle control if using LTE
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

#if 1
#if (FAST_CLOCK)
	/* abstracting this, performance increases by 60%
	 * No surprise, the clock is extremely expensive */
	MAINCLOCK(clk, 7159000.0)
#else
	/* this is the clock circuit in schematics. */
	MAINCLOCK(xclk, 14318000.0) //7159000.0*2
	TTL_74107(ic_f6a, xclk, high, high, high)
	ALIAS(clk, ic_f6a.Q)
#endif
#else
	// benchmarking ...
	NETDEV_TTL_CONST(clk, 0)
	MAINCLOCK(xclk)
	PARAM(xclk.FREQ, 7159000.0*2)
#endif

	/* 3V Logic - Just a resistor - the value is not given in schematics */

	RES(R3V, 50)   // Works ...
	NET_C(R3V.1, V5)
	ALIAS(V3, R3V.2)

	/* Coin, antenna and startup circuit */

	ANALOG_INPUT(STOPG, 0)

	ALIAS(SRSTQ, c9f.Q)
	ALIAS(SRST, c9c.Q)

	/* SRSTQ has a diode to +3V to protect against overvoltage - omitted */

	TTL_INPUT(antenna, 0)

	ALIAS(runQ, Q1.C)

	TTL_7404_INVERT(e4d, STOPG)

	TTL_7404_INVERT(c9f, c9c.Q)
	TTL_7404_INVERT(c9c, c9f.Q)

	SWITCH2(coinsw)
	NET_C(c9c.Q, coinsw.1)
	NET_C(c9f.Q, coinsw.2)

	NET_C(coinsw.Q, GND)

	/* Antenna circuit */
	/* Has a diode to clamp negative voltages - omitted here */

	QBJT_SW(Q3, "BC237B")
	NET_C(antenna, Q3.B)
	NET_C(GND, Q3.E)
	RES(RX5, 100)
	CAP(CX1, CAP_U(0.1))

	NET_C(RX5.1, CX1.1)
	NET_C(RX5.1, Q3.C)
	NET_C(RX5.2, GND)
	NET_C(CX1.2, GND)
	QBJT_SW(Q1, "BC237B")
	NET_C(Q1.B, RX5.1)
	NET_C(Q1.E, GND)

	DIODE(D3, "1N914")
	NET_C(D3.A, Q1.C)
	NET_C(D3.K, SRSTQ)

	DIODE(D2, "1N914")
	RES(RX4, 220)
	NET_C(D2.K, e4d.Q)
	NET_C(D2.A, RX4.1)
	NET_C(RX4.2, Q3.C)

	RES(RX1, 100)
	RES(RX2, 100)
	RES(RX3, 330)
	CAP(CX2, CAP_U(0.1))

	NET_C(RX3.2, D3.A)
	NET_C(RX3.1, RX1.2)
	NET_C(RX1.1, V3)

	NET_C(RX1.1, CX2.1)
	NET_C(RX1.2, CX2.2)

	QBJT_SW(Q2, "BC556B")
	NET_C(Q2.E, V3)
	NET_C(Q2.B, RX1.2)
	NET_C(Q2.C, RX2.2)

	NET_C(RX2.1, D2.A)

	/* hit logic */

	TTL_7404_INVERT(hitQ, hit)
	TTL_7400A_NAND(hit, hit1Q, hit2Q)

	TTL_7402_NOR(attractQ, StopG, runQ)
	TTL_7404_INVERT(attract, attractQ)

	TTL_7420_NAND(ic_h6a, hvidQ, hvidQ, hvidQ, hvidQ)
	ALIAS(hvid, ic_h6a.Q)

	TTL_7400A_NAND(ic_e6c, hvid, hblank)
	ALIAS(MissQ, ic_e6c.Q)

	TTL_7404_INVERT(ic_d1e, MissQ)
	TTL_7400A_NAND(ic_e1a, ic_d1e.Q, attractQ)
	ALIAS(Missed, ic_e1a.Q)

	TTL_7400A_NAND(rstspeed, SRSTQ, MissQ)
	TTL_7400A_NAND(StopG, StopG1Q, StopG2Q)
	ALIAS(L, ic_h3b.Q)
	ALIAS(R, ic_h3b.QQ)

	TTL_7400A_NAND(hit1Q, pad1, ic_g1b.Q)
	TTL_7400A_NAND(hit2Q, pad2, ic_g1b.Q)

	TTL_7400A_NAND(ic_g3c, 128H, ic_h3a.QQ)
	TTL_7427_NOR(ic_g2c, ic_g3c.Q, 256H, vpad1Q)
	ALIAS(pad1, ic_g2c.Q)
	TTL_7427_NOR(ic_g2a, ic_g3c.Q, 256HQ, vpad2Q)
	ALIAS(pad2, ic_g2a.Q)

	// ----------------------------------------------------------------------------------------
	// horizontal counter
	// ----------------------------------------------------------------------------------------
	TTL_7493(ic_f8, clk, ic_f8.QA, ic_e7b.QQ, ic_e7b.QQ)    // f8, f9, f6b
	TTL_7493(ic_f9, ic_f8.QD, ic_f9.QA, ic_e7b.QQ, ic_e7b.QQ)   // f8, f9, f6b
	TTL_74107(ic_f6b, ic_f9.QD, high, high, ic_e7b.Q)
	TTL_7430_NAND(ic_f7, ic_f8.QB, ic_f8.QC, ic_f9.QC, ic_f9.QD, ic_f6b.Q, high, high, high)
	TTL_7474(ic_e7b, clk, ic_f7, high, high)

	ALIAS(hreset, ic_e7b.QQ)
	ALIAS(hresetQ, ic_e7b.Q)
	ALIAS(  4H, ic_f8.QC)
	ALIAS(  8H, ic_f8.QD)
	ALIAS( 16H, ic_f9.QA)
	ALIAS( 32H, ic_f9.QB)
	ALIAS( 64H, ic_f9.QC)
	ALIAS(128H, ic_f9.QD)
	ALIAS(256H, ic_f6b.Q)
	ALIAS(256HQ, ic_f6b.QQ)

	// ----------------------------------------------------------------------------------------
	// vertical counter
	// ----------------------------------------------------------------------------------------
	TTL_7493(ic_e8, hreset, ic_e8.QA, ic_e7a.QQ, ic_e7a.QQ) // e8, e9, d9b
	TTL_7493(ic_e9, ic_e8.QD,ic_e9.QA,  ic_e7a.QQ, ic_e7a.QQ)   // e8, e9, d9b
	TTL_74107(ic_d9b, ic_e9.QD, high, high, ic_e7a.Q)
	TTL_7474(ic_e7a, hreset, e7a_data, high, high)
	TTL_7410_NAND(e7a_data, ic_e8.QA, ic_e8.QC, ic_d9b.Q)

	ALIAS(vreset, ic_e7a.QQ)
	ALIAS(  4V, ic_e8.QC)
	ALIAS(  8V, ic_e8.QD)
	ALIAS( 16V, ic_e9.QA)
	ALIAS( 32V, ic_e9.QB)
	ALIAS( 64V, ic_e9.QC)
	ALIAS(128V, ic_e9.QD)
	ALIAS(256V,  ic_d9b.Q)
	ALIAS(256VQ, ic_d9b.QQ)


	// ----------------------------------------------------------------------------------------
	// hblank flip flop
	// ----------------------------------------------------------------------------------------

	TTL_7400A_NAND(ic_g5b, 16H, 64H)

	// the time critical one
	TTL_7400A_NAND(ic_h5c, ic_h5b.Q, hresetQ)
	TTL_7400A_NAND(ic_h5b, ic_h5c.Q, ic_g5b.Q)

	ALIAS(hblank,  ic_h5c.Q)
	ALIAS(hblankQ,  ic_h5b.Q)
	TTL_7400A_NAND(hsyncQ, hblank, 32H)

	// ----------------------------------------------------------------------------------------
	// vblank flip flop
	// ----------------------------------------------------------------------------------------
	TTL_7402_NOR(ic_f5c, ic_f5d.Q, vreset)
	TTL_7402_NOR(ic_f5d, ic_f5c.Q, 16V)

	ALIAS(vblank,  ic_f5d.Q)
	ALIAS(vblankQ, ic_f5c.Q)

	TTL_7400A_NAND(ic_h5a, 8V, 8V)
	TTL_7410_NAND(ic_g5a, vblank, 4V, ic_h5a.Q)
	ALIAS(vsyncQ, ic_g5a.Q)

	// ----------------------------------------------------------------------------------------
	// move logic
	// ----------------------------------------------------------------------------------------

	TTL_7400A_NAND(ic_e1d, hit_sound, ic_e1c.Q)
	TTL_7400A_NAND(ic_e1c, ic_f1.QC, ic_f1.QD)
	TTL_7493(ic_f1, ic_e1d.Q, ic_f1.QA, rstspeed, rstspeed)

	TTL_7402_NOR(ic_g1d, ic_f1.QC, ic_f1.QD)
	TTL_7400A_NAND(ic_h1a, ic_g1d.Q, ic_g1d.Q)
	TTL_7400A_NAND(ic_h1d, ic_e1c.Q, ic_h1a.Q)

	TTL_7400A_NAND(ic_h1c, ic_h1d.Q, vreset)
	TTL_7400A_NAND(ic_h1b, ic_h1a.Q, vreset)
	TTL_7402_NOR(ic_g1c, 256HQ, vreset)

	TTL_74107(ic_h2a, ic_g1c.Q, ic_h2b.Q, low, ic_h1b.Q)
	TTL_74107(ic_h2b, ic_g1c.Q, high, move, ic_h1c.Q)

	TTL_7400A_NAND(ic_h4a, ic_h2b.Q, ic_h2a.Q)
	ALIAS(move, ic_h4a.Q)

	TTL_7400A_NAND(ic_c1d, SC, attract)
	TTL_7404_INVERT(ic_d1a, ic_c1d.Q)
	TTL_7474(ic_h3b, ic_d1a.Q, ic_h3b.QQ, hit1Q, hit2Q)

	TTL_7400A_NAND(ic_h4d, ic_h3b.Q, move)
	TTL_7400A_NAND(ic_h4b, ic_h3b.QQ, move)
	TTL_7400A_NAND(ic_h4c, ic_h4d.Q, ic_h4b.Q)
	ALIAS(Aa, ic_h4c.Q)
	ALIAS(Ba, ic_h4b.Q)

	// ----------------------------------------------------------------------------------------
	// hvid circuit
	// ----------------------------------------------------------------------------------------

	TTL_7400A_NAND(hball_resetQ, Serve, attractQ)

	TTL_9316(ic_g7, clk, high, hblankQ, hball_resetQ, ic_g5c.Q, Aa, Ba, low, high)
	TTL_9316(ic_h7, clk, ic_g7.RC, high, hball_resetQ, ic_g5c.Q, low, low, low, high)
	TTL_74107(ic_g6b, ic_h7.RC, high, high, hball_resetQ)
	TTL_7410_NAND(ic_g5c, ic_g6b.Q, ic_h7.RC, ic_g7.RC)
	TTL_7420_NAND(ic_h6b, ic_g6b.Q, ic_h7.RC, ic_g7.QC, ic_g7.QD)
	ALIAS(hvidQ, ic_h6b.Q)

	// ----------------------------------------------------------------------------------------
	// vvid circuit
	// ----------------------------------------------------------------------------------------

	TTL_9316(ic_b3, hsyncQ, high, vblankQ, high, ic_b2b.Q, a6, b6, c6, d6)
	TTL_9316(ic_a3, hsyncQ, ic_b3.RC, high, high, ic_b2b.Q, low, low, low, low)
	TTL_7400A_NAND(ic_b2b, ic_a3.RC, ic_b3.RC)
	TTL_7410_NAND(ic_e2b, ic_a3.RC, ic_b3.QC, ic_b3.QD)
	ALIAS(vvidQ, ic_e2b.Q)
	TTL_7404_INVERT(vvid, vvidQ)    // D2D
	ALIAS(vpos256, ic_a3.RC)
	ALIAS(vpos32, ic_a3.QB)
	ALIAS(vpos16, ic_a3.QA)

	// ----------------------------------------------------------------------------------------
	// vball ctrl circuit
	// ----------------------------------------------------------------------------------------

	TTL_7450_ANDORINVERT(ic_a6a, b1, 256HQ, b2, 256H)
	TTL_7450_ANDORINVERT(ic_a6b, c1, 256HQ, c2, 256H)
	TTL_7450_ANDORINVERT(ic_b6b, d1, 256HQ, d2, 256H)

	TTL_7474(ic_a5b, hit, ic_a6a, attractQ, high)
	TTL_7474(ic_a5a, hit, ic_a6b, attractQ, high)
	TTL_7474(ic_b5a, hit, ic_b6b, attractQ, high)
	TTL_74107(ic_h2x, vblank, vvid, vvid, hitQ) // two marked at position h2a ==> this h2x

	TTL_7486_XOR(ic_a4c, ic_a5b.Q, ic_h2x.Q)
	TTL_7486_XOR(ic_a4b, ic_a5a.Q, ic_h2x.Q)

	TTL_7450_ANDORINVERT(ic_b6a, ic_b5a.Q, ic_h2x.Q, ic_b5a.QQ, ic_h2x.QQ)

	TTL_7404_INVERT(ic_c4a, ic_b6a)

	TTL_7483(ic_b4, ic_a4c, ic_a4b, ic_b6a, low, ic_c4a, high, high, low, low)
	ALIAS(a6, ic_b4.S1)
	ALIAS(b6, ic_b4.S2)
	ALIAS(c6, ic_b4.S3)
	ALIAS(d6, ic_b4.S4)

	// ----------------------------------------------------------------------------------------
	// serve monoflop
	// ----------------------------------------------------------------------------------------

	TTL_7404_INVERT(f4_trig, rstspeed)

	RES(ic_f4_serve_R, RES_K(330))
	CAP(ic_f4_serve_C, CAP_U(4.7))
	NE555(ic_f4_serve)

	NET_C(ic_f4_serve.VCC, V5)
	NET_C(ic_f4_serve.GND, GND)
	NET_C(ic_f4_serve.RESET, V5)
	NET_C(ic_f4_serve_R.1, V5)
	NET_C(ic_f4_serve_R.2, ic_f4_serve.THRESH)
	NET_C(ic_f4_serve_R.2, ic_f4_serve.DISCH)
	NET_C(f4_trig, ic_f4_serve.TRIG)
	NET_C(ic_f4_serve_R.2, ic_f4_serve_C.1)
	NET_C(GND, ic_f4_serve_C.2)

	TTL_7427_NOR(ic_e5a, ic_f4_serve.OUT, StopG, runQ)
	TTL_7474(ic_b5b_serve, pad1, ic_e5a, ic_e5a, high)

	ALIAS(Serve, ic_b5b_serve.QQ)
	ALIAS(ServeQ, ic_b5b_serve.Q)

	// ----------------------------------------------------------------------------------------
	// score logic
	// ----------------------------------------------------------------------------------------

	TTL_7474(ic_h3a, 4H, 128H, high, attractQ)

	// ----------------------------------------------------------------------------------------
	// sound logic
	// ----------------------------------------------------------------------------------------
	TTL_7474(ic_c2a, vpos256, high, hitQ, high)
	TTL_74107(ic_f3_topbot, vblank, vvid, vvidQ, ServeQ)

	// ----------------------------------------------------------------------------------------
	// monoflop with NE555 determines duration of score sound
	// ----------------------------------------------------------------------------------------

	RES(ic_g4_R, RES_K(220))
	CAP(ic_g4_C, CAP_U(1))
	NE555(ic_g4_sc)
	ALIAS(SC, ic_g4_sc.OUT)

	NET_C(ic_g4_sc.VCC, V5)
	NET_C(ic_g4_sc.GND, GND)
	NET_C(ic_g4_sc.RESET, V5)
	NET_C(ic_g4_R.1, V5)
	NET_C(ic_g4_R.2, ic_g4_sc.THRESH)
	NET_C(ic_g4_R.2, ic_g4_sc.DISCH)
	NET_C(MissQ, ic_g4_sc.TRIG)
	NET_C(ic_g4_R.2, ic_g4_C.1)
	NET_C(GND, ic_g4_C.2)

	ALIAS(hit_sound_en, ic_c2a.QQ)
	TTL_7400A_NAND(hit_sound, hit_sound_en, vpos16)
	TTL_7400A_NAND(score_sound, SC, vpos32)
	TTL_7400A_NAND(topbothitsound, ic_f3_topbot.Q, vpos32)

	TTL_7410_NAND(ic_c4b, topbothitsound, hit_sound, score_sound)
	TTL_7400A_NAND(ic_c1b, ic_c4b.Q, attractQ)
	ALIAS(sound, ic_c1b.Q)


	// ----------------------------------------------------------------------------------------
	// paddle1 logic 1
	// ----------------------------------------------------------------------------------------

	POT(ic_b9_POT, RES_K(1))     // This is a guess!!
	PARAM(ic_b9_POT.DIALLOG, 1)  // Log Dial ...
	RES(ic_b9_RPRE, 470)

	NET_C(ic_b9_POT.1, V5)
	NET_C(ic_b9_POT.3, GND)
	NET_C(ic_b9_POT.2, ic_b9_RPRE.1)
	NET_C(ic_b9_RPRE.2, ic_b9.CONT)

	RES(ic_b9_R, RES_K(81))        // Adjustment pot
	CAP(ic_b9_C, CAP_U(0.1))
	DIODE(ic_b9_D, "1N914")
	NE555(ic_b9)

	NET_C(ic_b9.VCC, V5)
	NET_C(ic_b9.GND, GND)
	NET_C(ic_b9.RESET, V5)
	NET_C(ic_b9_R.1, V5)
	NET_C(ic_b9_R.2, ic_b9.THRESH)
	NET_C(ic_b9_R.2, ic_b9_D.A)
	NET_C(ic_b9_D.K, ic_b9.DISCH)
	NET_C(256VQ, ic_b9.TRIG)
	NET_C(ic_b9_R.2, ic_b9_C.1)
	NET_C(GND, ic_b9_C.2)

	TTL_7404_INVERT(ic_c9b, ic_b9.OUT)
	TTL_7400A_NAND(ic_b7b, ic_a7b.Q, hsyncQ)
	TTL_7493(ic_b8, ic_b7b.Q, ic_b8.QA, ic_b9.OUT, ic_b9.OUT)
	TTL_7400A_NAND(ic_b7a, ic_c9b.Q, ic_a7b.Q)
	TTL_7420_NAND(ic_a7b, ic_b8.QA, ic_b8.QB, ic_b8.QC, ic_b8.QD)
	ALIAS(vpad1Q, ic_b7a.Q)

	ALIAS(b1, ic_b8.QB)
	ALIAS(c1, ic_b8.QC)
	ALIAS(d1, ic_b8.QD)

	// ----------------------------------------------------------------------------------------
	// paddle1 logic 2
	// ----------------------------------------------------------------------------------------

	POT(ic_a9_POT, RES_K(1))     // This is a guess!!
	PARAM(ic_a9_POT.DIALLOG, 1)  // Log Dial ...
	RES(ic_a9_RPRE, 470)

	NET_C(ic_a9_POT.1, V5)
	NET_C(ic_a9_POT.3, GND)
	NET_C(ic_a9_POT.2, ic_a9_RPRE.1)
	NET_C(ic_a9_RPRE.2, ic_a9.CONT)

	RES(ic_a9_R, RES_K(81))        // Adjustment pot
	CAP(ic_a9_C, CAP_U(0.1))
	DIODE(ic_a9_D, "1N914")
	NE555(ic_a9)

	NET_C(ic_a9.VCC, V5)
	NET_C(ic_a9.GND, GND)
	NET_C(ic_a9.RESET, V5)
	NET_C(ic_a9_R.1, V5)
	NET_C(ic_a9_R.2, ic_a9.THRESH)
	NET_C(ic_a9_R.2, ic_a9_D.A)
	NET_C(ic_a9_D.K, ic_a9.DISCH)
	NET_C(256VQ, ic_a9.TRIG)
	NET_C(ic_a9_R.2, ic_a9_C.1)
	NET_C(GND, ic_a9_C.2)

	TTL_7404_INVERT(ic_c9a, ic_a9.OUT)
	TTL_7400A_NAND(ic_b7c, ic_a7a.Q, hsyncQ)
	TTL_7493(ic_a8, ic_b7c.Q, ic_a8.QA, ic_a9.OUT, ic_a9.OUT)
	TTL_7400A_NAND(ic_b7d, ic_c9a.Q, ic_a7a.Q)
	TTL_7420_NAND(ic_a7a, ic_a8.QA, ic_a8.QB, ic_a8.QC, ic_a8.QD)
	ALIAS(vpad2Q, ic_b7d.Q)

	ALIAS(b2, ic_a8.QB)
	ALIAS(c2, ic_a8.QC)
	ALIAS(d2, ic_a8.QD)

	// ----------------------------------------------------------------------------------------
	// C5-EN Logic
	// ----------------------------------------------------------------------------------------

	TTL_7404_INVERT(ic_e3a, 128H)
	TTL_7427_NOR( ic_e3b, 256H, 64H, ic_e3a.Q)
	TTL_7410_NAND(ic_e2c, 256H, 64H, ic_e3a.Q)
	TTL_7404_INVERT(ic_e3c, ic_e2c.Q)
	TTL_7402_NOR(ic_d2c, ic_e3c.Q, ic_e3b.Q)
	TTL_7404_INVERT(ic_g1a, 32V)
	TTL_7425_NOR(ic_f2a, ic_g1a.Q, 64V, 128V, ic_d2c.Q)
	ALIAS(c5_en, ic_f2a.Q)

	// ----------------------------------------------------------------------------------------
	// Score logic ...
	// ----------------------------------------------------------------------------------------

	TTL_7402_NOR(ic_f5b, L, Missed)
	TTL_7490(ic_c7, ic_f5b, ic_c7.QA, SRST, SRST, low, low)
	TTL_74107(ic_c8a, ic_c7.QD, high, high, SRSTQ)
	SWITCH2(sw1a)
	PARAM(sw1a.POS, 0)

	NET_C(sw1a.1, V5)
	NET_C(sw1a.2, ic_c7.QC)

	TTL_7410_NAND(ic_d8a, ic_c7.QA, sw1a.Q, ic_c8a.Q)       // would be nand2 for 11 instead of 15 points, need a switch dev!

	ALIAS(StopG1Q, ic_d8a.Q)
	ALIAS(score1_1, ic_c7.QA)
	ALIAS(score1_2, ic_c7.QB)
	ALIAS(score1_4, ic_c7.QC)
	ALIAS(score1_8, ic_c7.QD)
	ALIAS(score1_10, ic_c8a.Q)
	ALIAS(score1_10Q, ic_c8a.QQ)

	TTL_7402_NOR(ic_f5a, R, Missed)
	TTL_7490(ic_d7, ic_f5a, ic_d7.QA, SRST, SRST, low, low)
	TTL_74107(ic_c8b, ic_d7.QD, high, high, SRSTQ)
	SWITCH2(sw1b)
	PARAM(sw1b.POS, 0)

	NET_C(sw1b.1, V5)
	NET_C(sw1b.2, ic_d7.QC)


	TTL_7410_NAND(ic_d8b, ic_d7.QA, sw1b.Q, ic_c8b.Q)       // would be nand2 for 11 instead of 15 points, need a switch dev!

	ALIAS(StopG2Q, ic_d8b.Q)
	ALIAS(score2_1, ic_d7.QA)
	ALIAS(score2_2, ic_d7.QB)
	ALIAS(score2_4, ic_d7.QC)
	ALIAS(score2_8, ic_d7.QD)
	ALIAS(score2_10, ic_c8b.Q)
	ALIAS(score2_10Q, ic_c8b.QQ)

	// ----------------------------------------------------------------------------------------
	// Score display
	// ----------------------------------------------------------------------------------------

	TTL_74153(ic_d6a, score1_10Q, score1_4, score2_10Q, score2_4, 32H, 64H, low)
	TTL_74153(ic_d6b, score1_10Q, score1_8, score2_10Q, score2_8, 32H, 64H, low)

	TTL_74153(ic_c6a, high, score1_1, high, score2_1, 32H, 64H, low)
	TTL_74153(ic_c6b, score1_10Q, score1_2, score2_10Q, score2_2, 32H, 64H, low)

	TTL_7448(ic_c5, ic_c6a.AY, ic_c6b.AY, ic_d6a.AY, ic_d6b.AY, high, c5_en, high)

	TTL_7404_INVERT(ic_e4b, 16H)
	TTL_7427_NOR(ic_e5c, ic_e4b.Q, 8H, 4H)
	ALIAS(scoreFE, ic_e5c.Q)

	TTL_7400A_NAND(ic_c3d, 8H, 4H)
	//TTL_7400A_NAND(ic_c3d, 4H, 8H)
	TTL_7402_NOR(ic_d2b, ic_e4b.Q, ic_c3d.Q)
	ALIAS(scoreBC, ic_d2b.Q)

	TTL_7427_NOR(ic_e5b, ic_e4b.Q, 8V, 4V)
	ALIAS(scoreA, ic_e5b.Q)

	TTL_7410_NAND(ic_e2a, 16H, 8V, 4V)
	TTL_7404_INVERT(ic_e4a, ic_e2a.Q)
	ALIAS(scoreGD, ic_e4a.Q)

	TTL_7404_INVERT(ic_e4c, 16V)

	TTL_7410_NAND(ic_d4a, ic_e4c.Q, ic_c5.f, scoreFE)
	TTL_7410_NAND(ic_d5c,      16V, ic_c5.e, scoreFE)
	TTL_7410_NAND(ic_c4c, ic_e4c.Q, ic_c5.b, scoreBC)
	TTL_7410_NAND(ic_d5a,      16V, ic_c5.c, scoreBC)
	TTL_7410_NAND(ic_d4c, ic_e4c.Q, ic_c5.a, scoreA)
	TTL_7410_NAND(ic_d4b, ic_e4c.Q, ic_c5.g, scoreGD)
	TTL_7410_NAND(ic_d5b,      16V, ic_c5.d, scoreGD)

	TTL_7430_NAND(ic_d3, ic_d4a, ic_d5c, ic_c4c, ic_d5a, ic_d4c, ic_d4b, ic_d5b, high)
	ALIAS(score, ic_d3.Q)       //FIXME

	// net
	TTL_74107(ic_f3b, clk, 256H, 256HQ, high)
	TTL_7400A_NAND(ic_g3b, ic_f3b.QQ, 256H)
	TTL_7427_NOR(ic_g2b, ic_g3b.Q, vblank, 4V)
	ALIAS(net, ic_g2b.Q)

	// ----------------------------------------------------------------------------------------
	// video
	// ----------------------------------------------------------------------------------------

	TTL_7402_NOR(ic_g1b, hvidQ, vvidQ)
	TTL_7425_NOR(ic_f2b, ic_g1b.Q, pad1, pad2, net)
	TTL_7404_INVERT(ic_e4e, ic_f2b.Q)
	ALIAS(video, ic_e4e.Q)

	TTL_7486_XOR(ic_a4d, hsyncQ, vsyncQ)
	TTL_7404_INVERT(ic_e4f, ic_a4d.Q)

	RES(RV1, RES_K(1))
	RES(RV2, RES_K(1.2))
	RES(RV3, RES_K(22))
	NET_C(video, RV1.1)
	NET_C(score, RV2.1)
	NET_C(ic_e4f.Q, RV3.1)
	NET_C(RV1.2, RV2.2)
	NET_C(RV2.2, RV3.2)

	ALIAS(videomix, RV3.2)

NETLIST_END()
