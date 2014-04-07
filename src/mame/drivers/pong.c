/***************************************************************************

Pong (c) 1972 Atari

driver by Couriersud

Notes:

TODO:

***************************************************************************/


#include "emu.h"

#include "machine/rescap.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "sound/dac.h"
#include "video/fixfreq.h"
#include "astring.h"

//#define TEST_SOUND
#define PONGD   (0)

/*
 * H count width to 512
 * Reset at 1C6 = 454
 * V count width to 512, counts on HReset
 * Reset at 105 = 261

 * Clock = 7.159 MHz

 * ==> 15.768 Khz Horz Freq
 * ==> 60.41 Refresh

 * HBlank 0 to 79
 * HSync 32 to 63
 * VBlank 0 to 15
 * VSync 4 to 7

 * Video = (HVID & VVID ) & (NET & PAD1 & PAD2)

 * Net at 256H alternating at 4V
 *
 *
 * http://www.youtube.com/watch?v=pDrRnJOCKZc
 */

#define MASTER_CLOCK    7159000
#define V_TOTAL         (0x105+1)       // 262
#define H_TOTAL         (0x1C6+1)       // 454

#define HBSTART                 (H_TOTAL)
#define HBEND                   (80)
#define VBSTART                 (V_TOTAL)
#define VBEND                   (16)

#define HRES_MULT                   (1)

fixedfreq_interface fixedfreq_mode_pongd = {
	MASTER_CLOCK,
	H_TOTAL-67,H_TOTAL-52,H_TOTAL-8,H_TOTAL,
	V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL,
	1,  /* non-interlaced */
	0.31
};

fixedfreq_interface fixedfreq_mode_pong = {
	MASTER_CLOCK,
	H_TOTAL-67,H_TOTAL-40,H_TOTAL-8,H_TOTAL,
	V_TOTAL-22,V_TOTAL-19,V_TOTAL-12,V_TOTAL,
	1,  /* non-interlaced */
	0.31
};

fixedfreq_interface fixedfreq_mode_pongX2 = {
	MASTER_CLOCK * 2,
	(H_TOTAL-67) * 2, (H_TOTAL-40) * 2, (H_TOTAL-8) * 2, (H_TOTAL) * 2,
	V_TOTAL-22,V_TOTAL-19,V_TOTAL-16,V_TOTAL,
	1,  /* non-interlaced */
	0.31
};

enum input_changed_enum
{
	IC_PADDLE1,
	IC_PADDLE2,
	IC_COIN,
	IC_SWITCH,
	IC_VR1,
	IC_VR2
};

#if PONGD
#include "../../nl_examples/tdice.c"

#undef SRST
#undef VCC
#undef GND
#endif


static NETLIST_START(pong_schematics)
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient

	ANALOG_INPUT(V5, 5)

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

#if 1
#if 0
	/* this is the clock circuit in schematics. */
	MAINCLOCK(xclk, 7159000.0*2)
	TTL_74107(ic_f6a, xclk, high, high, high)
	ALIAS(clk, ic_f6a.Q)
#else
	/* abstracting this, performance increases by 60%
	 * No surprise, the clock is extremely expensive */
	MAINCLOCK(clk, 7159000.0)
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
	TTL_7400_NAND(hit, hit1Q, hit2Q)

	TTL_7402_NOR(attractQ, StopG, runQ)
	TTL_7404_INVERT(attract, attractQ)

	TTL_7420_NAND(ic_h6a, hvidQ, hvidQ, hvidQ, hvidQ)
	ALIAS(hvid, ic_h6a.Q)

	TTL_7400_NAND(ic_e6c, hvid, hblank)
	ALIAS(MissQ, ic_e6c.Q)

	TTL_7404_INVERT(ic_d1e, MissQ)
	TTL_7400_NAND(ic_e1a, ic_d1e.Q, attractQ)
	ALIAS(Missed, ic_e1a.Q)

	TTL_7400_NAND(rstspeed, SRSTQ, MissQ)
	TTL_7400_NAND(StopG, StopG1Q, StopG2Q)
	ALIAS(L, ic_h3b.Q)
	ALIAS(R, ic_h3b.QQ)

	TTL_7400_NAND(hit1Q, pad1, ic_g1b.Q)
	TTL_7400_NAND(hit2Q, pad2, ic_g1b.Q)

	TTL_7400_NAND(ic_g3c, 128H, ic_h3a.QQ)
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

	TTL_7400_NAND(ic_g5b, 16H, 64H)

	// the time critical one
	TTL_7400_NAND(ic_h5c, ic_h5b.Q, hresetQ)
	TTL_7400_NAND(ic_h5b, ic_h5c.Q, ic_g5b.Q)

	ALIAS(hblank,  ic_h5c.Q)
	ALIAS(hblankQ,  ic_h5b.Q)
	TTL_7400_NAND(hsyncQ, hblank, 32H)

	// ----------------------------------------------------------------------------------------
	// vblank flip flop
	// ----------------------------------------------------------------------------------------
	TTL_7402_NOR(ic_f5c, ic_f5d.Q, vreset)
	TTL_7402_NOR(ic_f5d, ic_f5c.Q, 16V)

	ALIAS(vblank,  ic_f5d.Q)
	ALIAS(vblankQ, ic_f5c.Q)

	TTL_7400_NAND(ic_h5a, 8V, 8V)
	TTL_7410_NAND(ic_g5a, vblank, 4V, ic_h5a.Q)
	ALIAS(vsyncQ, ic_g5a.Q)

	// ----------------------------------------------------------------------------------------
	// move logic
	// ----------------------------------------------------------------------------------------

	TTL_7400_NAND(ic_e1d, hit_sound, ic_e1c.Q)
	TTL_7400_NAND(ic_e1c, ic_f1.QC, ic_f1.QD)
	TTL_7493(ic_f1, ic_e1d.Q, ic_f1.QA, rstspeed, rstspeed)

	TTL_7402_NOR(ic_g1d, ic_f1.QC, ic_f1.QD)
	TTL_7400_NAND(ic_h1a, ic_g1d.Q, ic_g1d.Q)
	TTL_7400_NAND(ic_h1d, ic_e1c.Q, ic_h1a.Q)

	TTL_7400_NAND(ic_h1c, ic_h1d.Q, vreset)
	TTL_7400_NAND(ic_h1b, ic_h1a.Q, vreset)
	TTL_7402_NOR(ic_g1c, 256HQ, vreset)

	TTL_74107(ic_h2a, ic_g1c.Q, ic_h2b.Q, low, ic_h1b.Q)
	TTL_74107(ic_h2b, ic_g1c.Q, high, move, ic_h1c.Q)

	TTL_7400_NAND(ic_h4a, ic_h2b.Q, ic_h2a.Q)
	ALIAS(move, ic_h4a.Q)

	TTL_7400_NAND(ic_c1d, SC, attract)
	TTL_7404_INVERT(ic_d1a, ic_c1d.Q)
	TTL_7474(ic_h3b, ic_d1a.Q, ic_h3b.QQ, hit1Q, hit2Q)

	TTL_7400_NAND(ic_h4d, ic_h3b.Q, move)
	TTL_7400_NAND(ic_h4b, ic_h3b.QQ, move)
	TTL_7400_NAND(ic_h4c, ic_h4d.Q, ic_h4b.Q)
	ALIAS(Aa, ic_h4c.Q)
	ALIAS(Ba, ic_h4b.Q)

	// ----------------------------------------------------------------------------------------
	// hvid circuit
	// ----------------------------------------------------------------------------------------

	TTL_7400_NAND(hball_resetQ, Serve, attractQ)

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
	TTL_7400_NAND(ic_b2b, ic_a3.RC, ic_b3.RC)
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
	TTL_7400_NAND(hit_sound, hit_sound_en, vpos16)
	TTL_7400_NAND(score_sound, SC, vpos32)
	TTL_7400_NAND(topbothitsound, ic_f3_topbot.Q, vpos32)

	TTL_7410_NAND(ic_c4b, topbothitsound, hit_sound, score_sound)
	TTL_7400_NAND(ic_c1b, ic_c4b.Q, attractQ)
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
	CAP(ic_b9_C, CAP_U(.1))
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
	TTL_7400_NAND(ic_b7b, ic_a7b.Q, hsyncQ)
	TTL_7493(ic_b8, ic_b7b.Q, ic_b8.QA, ic_b9.OUT, ic_b9.OUT)
	TTL_7400_NAND(ic_b7a, ic_c9b.Q, ic_a7b.Q)
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
	CAP(ic_a9_C, CAP_U(.1))
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
	TTL_7400_NAND(ic_b7c, ic_a7a.Q, hsyncQ)
	TTL_7493(ic_a8, ic_b7c.Q, ic_a8.QA, ic_a9.OUT, ic_a9.OUT)
	TTL_7400_NAND(ic_b7d, ic_c9a.Q, ic_a7a.Q)
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

	NET_C(sw1a.1, high)
	//NET_C(sw1a.1, V5)
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

	NET_C(sw1b.1, high)
	//NET_C(sw1b.1, V5)
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

	TTL_7400_NAND(ic_c3d, 8H, 4H)
	//TTL_7400_NAND(ic_c3d, 4H, 8H)
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
	TTL_7400_NAND(ic_g3b, ic_f3b.QQ, 256H)
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

class pong_state : public driver_device
{
public:
	pong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_video(*this, "fixfreq"),

			m_dac(*this, "dac"),                /* just to have a sound device */
			m_sw1a(*this, "maincpu:sw1a"),
			m_sw1b(*this, "maincpu:sw1b")
	{
	}

	// devices
	required_device<netlist_mame_device_t> m_maincpu;
	required_device<fixedfreq_device> m_video;
	required_device<dac_device> m_dac; /* just to have a sound device */

	// sub devices
	required_device<netlist_mame_logic_input_t> m_sw1a;
	required_device<netlist_mame_logic_input_t> m_sw1b;

	//UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

	NETDEV_ANALOG_CALLBACK_MEMBER(sound_cb)
	{
		//printf("snd %f\n", newval);
		//dac_w(m_dac, 0, newval*64);
		m_dac->write_unsigned8(64*data);
	}

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

private:

};

static NETLIST_START(pong)

	//NETLIST_INCLUDE(pong_schematics)
	NETLIST_MEMREGION("maincpu")

	//NETDEV_ANALOG_CALLBACK(sound_cb, sound, pong_state, sound_cb, "")
	//NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")
NETLIST_END()

static NETLIST_START(pong_fast)

	INCLUDE(pong_schematics)

	//NETDEV_ANALOG_CALLBACK(sound_cb, sound, pong_state, sound_cb, "")
	//NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")

NETLIST_END()

#if PONGD

static NETLIST_START(pongd)

	INCLUDE(pongdoubles)

	//NETDEV_ANALOG_CALLBACK(sound_cb, AUDIO, pong_state, sound_cb, "")
	//NETDEV_ANALOG_CALLBACK(video_cb, videomix, fixedfreq_device, update_vid, "fixfreq")

NETLIST_END()
#endif

#ifdef TEST_SOUND
static NETLIST_START(test)

	/*
	 * Astable multivibrator using two 7400 gates (or inverters)
	 *
	 */

	/* Standard stuff */

	SOLVER(Solver)
	PARAM(Solver.FREQ, 48000)

	//NETDEV_SOUND_IN(SND_IN)
	//PARAM(SND_IN.CHAN0, "tin.IN")

	ANALOG_INPUT(tin, 0)

	// astable NAND Multivibrator
	RES(R1, 1000)
	CAP(C1, 1e-6)
	TTL_7400_NAND(n1,R1.1,R1.1)
	TTL_7400_NAND(n2,R1.2,R1.2)
	NET_C(n1.Q, R1.2)
	NET_C(n2.Q, C1.1)
	NET_C(C1.2, R1.1)
	LOG(log1, n2.Q)

#if 0
	NETDEV_SOUND_OUT(CH0, 0)
	NET_C(CH0.IN, n2.Q)

	NETDEV_SOUND_OUT(CH1, 1)
	NET_C(CH1.IN, tin.Q)
#endif

NETLIST_END()
#endif

void pong_state::machine_start()
{
}

void pong_state::machine_reset()
{
}


void pong_state::video_start()
{
}


INPUT_CHANGED_MEMBER(pong_state::input_changed)
{
	int numpad = (FPTR) (param);

	switch (numpad)
	{
	case IC_SWITCH:
		m_sw1a->write(newval ? 1 : 0);
		m_sw1b->write(newval ? 1 : 0);
		break;
	}
}

static INPUT_PORTS_START( pong )
	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot0")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")

	PORT_DIPNAME( 0x06, 0x00, "Game Won" )          PORT_DIPLOCATION("SW1A:1,SW1B:1") PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x06, "15" )

	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")

	PORT_START("VR1")
	PORT_ADJUSTER( 50, "VR1 - 50k, Paddle 1 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr0")
	PORT_START("VR2")
	PORT_ADJUSTER( 50, "VR2 - 50k, Paddle 2 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr1")

INPUT_PORTS_END

#if PONGD
static INPUT_PORTS_START( pongd )
#if 0
	PORT_START( "PADDLE0" ) /* fake input port for player 1 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0)   NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot0")

	PORT_START( "PADDLE1" ) /* fake input port for player 2 paddle */
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(2) PORT_KEYDELTA(100) PORT_CENTERDELTA(0) PORT_PLAYER(2) NETLIST_ANALOG_PORT_CHANGED("maincpu", "pot1")
#endif

	PORT_START("IN0") /* fake as well */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "coinsw")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )     NETLIST_LOGIC_PORT_CHANGED("maincpu", "startsw")

#if 0
	PORT_DIPNAME( 0x06, 0x00, "Game Won" )          PORT_DIPLOCATION("SW1A:1,SW1B:1") PORT_CHANGED_MEMBER(DEVICE_SELF, pong_state, input_changed, IC_SWITCH)
	PORT_DIPSETTING(    0x00, "11" )
	PORT_DIPSETTING(    0x06, "15" )

	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )  PORT_NAME("Antenna") NETLIST_LOGIC_PORT_CHANGED("maincpu", "antenna")

	PORT_START("VR1")
	PORT_ADJUSTER( 50, "VR1 - 50k, Paddle 1 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr0")
	PORT_START("VR2")
	PORT_ADJUSTER( 50, "VR2 - 50k, Paddle 2 adjustment" )   NETLIST_ANALOG_PORT_CHANGED("maincpu", "vr1")
#endif
INPUT_PORTS_END

#endif

static MACHINE_CONFIG_START( pong, pong_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(pong)

	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr0", "ic_b9_R.R")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr1", "ic_a9_R.R")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot0", "ic_b9_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot1", "ic_a9_POT.DIAL")
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1a", "sw1a.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1b", "sw1b.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw", "coinsw.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "antenna", "antenna.IN", 0, 0x01)

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "snd0", "sound", pong_state, sound_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "vid0", "videomix", fixedfreq_device, update_vid, "fixfreq")

	/* video hardware */

	//MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_ntsc720)
	//MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_pongX2)
	MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_pong)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
#ifdef TEST_SOUND
	MCFG_SOUND_ROUTE_EX(0, "snd_test", 1.0, 0)
#else
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
#endif

#ifdef TEST_SOUND
	MCFG_SOUND_ADD("snd_test", NETLIST_SOUND, 48000)
	MCFG_NETLIST_SETUP(test)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NETLIST_STREAM_INPUT("snd_test", 0, "tin.IN")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(0.001, 0.0)

	//MCFG_NETLIST_STREAM_OUTPUT("snd_test", 0, "tin.Q", 100)
	MCFG_NETLIST_STREAM_OUTPUT("snd_test", 0, "n2.Q")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1000.0, 0.0)
	MCFG_NETLIST_STREAM_OUTPUT("snd_test", 1, "tin.Q")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(1000.0, 0.0)

#endif

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pongf, pong )

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_NETLIST_SETUP(pong_fast)

MACHINE_CONFIG_END

#if PONGD
static MACHINE_CONFIG_START( pongd, pong_state )

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", NETLIST_CPU, NETLIST_CLOCK)
	MCFG_NETLIST_SETUP(pongd)

#if 0
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr0", "ic_b9_R.R")
	MCFG_NETLIST_ANALOG_INPUT_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "vr1", "ic_a9_R.R")
	MCFG_NETLIST_ANALOG_INPUT_MULT_OFFSET(1.0 / 100.0 * RES_K(50), RES_K(56) )
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot0", "ic_b9_POT.DIAL")
	MCFG_NETLIST_ANALOG_INPUT("maincpu", "pot1", "ic_a9_POT.DIAL")
#endif
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1a", "DIPSW1.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "sw1b", "DIPSW2.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "coinsw", "COIN_SW.POS", 0, 0x01)
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "startsw", "START_SW.POS", 0, 0x01)
#if 0
	MCFG_NETLIST_LOGIC_INPUT("maincpu", "antenna", "antenna.IN", 0, 0x01)
#endif

	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "snd0", "AUDIO", pong_state, sound_cb, "")
	MCFG_NETLIST_ANALOG_OUTPUT("maincpu", "vid0", "videomix", fixedfreq_device, update_vid, "fixfreq")

	/* video hardware */

	//MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_ntsc720)
	//MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_pongX2)
	MCFG_FIXFREQ_ADD("fixfreq", "screen", fixedfreq_mode_pongd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 48000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

#endif

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pong ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", 0 ) /* enough for netlist */
	ROM_LOAD( "pong.netlist", 0x000000, 0x00457f, CRC(72d5e4fe) SHA1(7bb15828223c34915c5e2869dd7917532a4bb7b4) )
ROM_END

ROM_START( pongf ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END

#if PONGD
ROM_START( pongd ) /* dummy to satisfy game entry*/
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
ROM_END
#endif

GAME( 1972, pong,  0, pong, pong, driver_device,  0, ROT0, "Atari", "Pong (Rev E) external", GAME_SUPPORTS_SAVE)
GAME( 1972, pongf,  0, pongf, pong, driver_device,  0, ROT0, "Atari", "Pong (Rev E)", GAME_SUPPORTS_SAVE )
#if PONGD
GAME( 1974, pongd,  0, pongd, pongd, driver_device,  0, ROT0, "Atari", "Pong Doubles", GAME_SUPPORTS_SAVE )
#endif
