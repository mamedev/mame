// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/***************************************************************************

  Netlist (tp1985) included from testpat.cpp

***************************************************************************/

#ifndef __PLIB_PREPROCESSOR__
	#define NL_PROHIBIT_BASEH_INCLUDE   1
	#include "netlist/devices/net_lib.h"
#endif

// gray scale
#define SB3 0
// vertical stripes
#define SB4 1
// vertical lines
#define SB5 0
// horizontal stripes
#define SB6 0
// horizontal lines
#define SB7 0
// "M"
#define SB8 1
// inverse
//efine SB9 0

//
#define _R19 0
//
#define _VD4 0
//
#define _DD4 1

NETLIST_START(tp1985)

	SOLVER(Solver, 48000)
//  PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-5) // ???
//  PARAM(Solver.LTE,     1e-4) // Default is not enough for paddle control if using LTE
	PARAM(NETLIST.USE_DEACTIVATE, 0)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5) // no-ttl-dip devices need VCC!

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	MAINCLOCK(clk, 4000000.0)

// raster generator: DD1-DD4, DD5.3, DD5.4, DD6-DD7

	RES(R1, 1000)
	NET_C(R1.1, V5)

	// A, B, C, D,  CLEAR, LOADQ, CU, CD  [15, 1, 10, 9,  14, 11, 5, 4]
	TTL_74193(DD1, low, low, low, low,   GND, R1.2, DD2.CARRYQ, R1.2)
	TTL_74193(DD2, low, low, low, low,   GND, R1.2, clk, R1.2)

	ALIAS(F250K, DD2.CARRYQ)
	ALIAS(F15625, DD1.CARRYQ)

	RES(R2, 2000)
	RES(R3, 1000)
	RES(R4, 510)
	RES(R5, 240)
	RES(R6, 510)
	RES(R7, 510)
	RES(R8, 240)
	DIODE(VD1, "1N34A") // XXX actually D9B
	DIODE(VD2, "1N34A") // XXX actually D9B
#if SB3
	DIODE(VD6, "1N34A") // XXX actually D9B
#endif

	NET_C(DD1.QA, R2.1, R6.1)
	NET_C(DD1.QB, R3.1, VD1.A)
	NET_C(DD1.QC, R4.1)
	NET_C(DD1.QD, R5.1)

	NET_C(DD2.QC, R7.1)
	NET_C(DD2.QD, R8.1)
	NET_C(R6.2, DD3_2.D)

	// and to SB1.1, SB4.2, SB3...
#if SB3
#if SB4
	NET_C(R2.2, R3.2, R4.2, R5.2)
	NET_C(GND, VD6.A)
#else
	NET_C(R2.2, R3.2, R4.2, R5.2, VD6.A)
#endif
#else
	NET_C(R2.2, R3.2, R4.2, R5.2)
#endif

	// and to SB6.2
	NET_C(R7.2, R8.2, VD2.A)

	// CLK, D, CLRQ, PREQ  [3, 2, 1, 4]
	TTL_7474(DD3_1, DD1.QA, DD1.QB, DD1.CARRYQ, DD3_1.QQ)
//  TTL_7474(DD3_2, VD1.K, DD2.QD, high, DD3_1.QQ) // per book, produces one pulse, shifted too far.
#if _DD4
	TTL_7474(DD3_2, DD2.QD, VD1.K, DD4.Y, DD3_1.QQ) // per journal, produces two pulse, shifted correctly.
#else
	TTL_7474(DD3_2, DD2.QD, VD1.K, high, DD3_1.QQ) // per journal, produces two pulse, shifted correctly.
#endif
	/*
	 * verified:
	 *
	 * DD3.1 = hblank generator, 12 us @ 75%
	 * DD3.2 = hsync generator, 4 us @ 100%, shifted 2 us after hblank
	 * hsync period = 64us (15625 Hz)
	 */

#if _DD4
	RES(R12, 2000)
	RES(R13, 2000)
	CAP(C2, CAP_P(1000))

	NET_C(V5, R13.1)
	NET_C(GND, R12.1)
	NET_C(C2.2, R13.2)
	NET_C(R13.2, R12.2)
	NET_C(DD6.QD, C2.1)

	// CLK, STROBE, ENABLE, UNITY, CLR,  Bx
	//
	// STRB, ENin are tied to GND
	TTL_7497(DD4, DD3_1.QQ, low, low, DD5_3.Q, DD5_4.Q,  low, DD4.Y, low, DD5_3.Q, low, low)
//  TTL_7497(DD4, DD3_1.QQ, low, low, low, DD5_4.Q,  low, DD4.Y, low, DD5_3.Q, low, low)
//  TTL_7497(DD4, DD3_1.QQ, low, low, low, DD5_4.Q,  low, DD4.Y, low, low, low, low)
	TTL_7400_NAND(DD5_3, R13.2, DD4.Y)
//  TTL_7400_NAND(DD5_3, high, DD4.Y)
	TTL_7400_NAND(DD5_4, DD4.ENOUTQ, DD4.ENOUTQ)
	TTL_7493(DD6, DD4.ZQ, DD4.ENOUTQ, DD6.QD, DD6.QB) // CLK1, CLK2, R1, R2  [14, 1, 2, 3]
#else
	TTL_7497(DD4, DD3_1.QQ, low, low, low, low,  low, high, low, low, low, low)
//  TTL_7493(DDx, DD3_1.QQ, DD3_1.QQ, low, low)
	TTL_7493(DDx, DD3_1.QQ, DD3_1.QQ, DDx.QD, DDx.QB)
#endif

// pattern selector

	RES(R10, 1000)
	RES(R11, 240)
	RES(R14, 510)
	RES(R15, 1000)
	RES(R16, 510)
	DIODE(VD3, "1N34A") // XXX actually D9B
#if _VD4
	DIODE(VD4, "1N34A") // XXX actually D9B
#endif
	DIODE(VD5, "1N34A") // XXX actually D9B

#if SB4
	ALIAS(hpatsource, DD1.QA)
#endif
#if SB5
	ALIAS(hpatsource, DD2.CARRYQ)
#endif
#if !(SB4+SB5)
	ALIAS(hpatsource, R11.2)
#endif

#if SB6
	ALIAS(vpatsource, DD6.QA)
#endif
#if SB7
	ALIAS(vpatsource, DD4.Z)
#endif
#if !(SB6+SB7)
	ALIAS(vpatsource, R11.2)
#endif

	NET_C(V5, R10.1)
	NET_C(V5, R11.1)
	NET_C(V5, R15.1)
	NET_C(R11.2, VD3.A)
	NET_C(hpatsource, R14.1)
	NET_C(R14.2, DD7_2.A)

	TTL_7400_NAND(DD7_1, hpatsource, vpatsource)
#if !SB8
	TTL_7400_NAND(DD7_2, VD3.K, R15.2)
	NET_C(VD2.K, DD7_2.B)
#else
	TTL_7400_NAND(DD7_2, VD3.K, DD7_1.Q)
	NET_C(R15.2, GND)
	NET_C(VD2.K, GND)
#endif

#if _VD4
	NET_C(DD7_2.Q, VD4.K)
	NET_C(VD4.A, R16.2)
#else
	NET_C(DD7_2.Q, R16.2)
#endif
	NET_C(DD3_1.Q, R16.1)

	TTL_7400_NAND(DD7_3, DD7_2.Q, R10.2)
	TTL_7400_NAND(DD7_4, R16.1, DD7_3.Q)

	NET_C(DD7_4.Q, VD5.A)
	NET_C(VD5.K, R17.1)
#if SB3
	NET_C(VD6.K, R17.1)
#endif

// video mixer

#ifdef VD7
	DIODE(VD7, "1N34A") // XXX actually D9B
#endif
	RES(R17, 10) // XXX actually 470 ohm POT
	RES(R18, 430)
#if _R19
	RES(R19, 430)
	CAP(C3, CAP_U(50))
#endif

#ifdef VD7
	NET_C(DD3_2.QQ, VD7.A)
	NET_C(VD7.K, R18.1)
#else
	NET_C(DD3_2.QQ, R18.1)
#endif
	NET_C(R17.2, R18.2) // XXX

#if _R19
	NET_C(GND, R19.1)
	NET_C(R18.2, C3.2)
	NET_C(R19.2, C3.2)
	ALIAS(videomix, C3.1)
#else
	ALIAS(videomix, R18.2)
	//LOG(logV, R18.2)
#endif

#if 0
	HINT(clk, NO_DEACTIVATE)
#endif

NETLIST_END()

