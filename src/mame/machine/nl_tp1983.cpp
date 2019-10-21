// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/***************************************************************************

  Netlist (tp1983) included from testpat.cpp

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

NETLIST_START(tp1983)

	SOLVER(Solver, 480000)
//  PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-5) // ???
//  PARAM(Solver.LTE,     1e-4) // Default is not enough for paddle control if using LTE
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5) // no-ttl-dip devices need VCC!

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	// skipping D7.1 D7.2 D8.2 clock generator circuit
	MAINCLOCK(clk, 250000.0)

// vsync generator

#define _C6 0

	CAP(C6, CAP_P(11))
	RES(R19, RES_K(30))

#if _C6
	NET_C(V5, C6.1)
#else
	NET_C(DD6_2.Q, C6.1)
#endif
	NET_C(C6.2, R19.1)
	//NET_C(R19.2, GND)
	NET_C(R19.2, V5)

	// CLK, STROBE, ENABLE, UNITY, CLR,  Bx  [9, 10, 11, 12, 13, ...]
	TTL_7497(DD4, clk, low, low, low, DD5.Y,  high, vsync, low, vsync, vsync, low)
#if 1
	TTL_7497(DD5, DD4.ZQ, DD6_2.QQ, low, DD5.ENOUTQ, DD5.Y,  high, DD6_1.QQ, low, low, low, low)
#else
	TTL_7497(DD5, DD4.Z, low, low, DD5.ENOUT, DD5.Y,  high, DD6_1.QQ, low, low, low, low)
#endif

	// CLK, D, CLRQ, PREQ  [3, 2, 1, 4]
#if 1
	TTL_7474(DD6_1, DD5.Y, DD6_2.Q, C6.2, high)
	TTL_7474(DD6_2, DD5.Y, DD6_1.QQ, C6.2, high)
#else
	TTL_7474(DD6_1, DD5.Y, DD6_2.Q, low, high)
	TTL_7474(DD6_2, DD5.Y, DD6_1.QQ, low, high)
#endif

	ALIAS(vsync, DD6_1.Q)

// hsync generator

#if 0
	CAP(C1, CAP_P(330))
	CAP(C2, CAP_P(3300)) // XXX tunable 3300
	RES(R2, RES_K(30))
	RES(R3, 1200)

	// A, B, C, D,  CLEAR, LOADQ, CU, CD  [15, 1, 10, 9,  14, 11, 5, 4]
	TTL_74193(DD1, low, low, low, low,   low, high, clk, high)
	NET_C(DD1.CARRYQ, C1.1)
	NET_C(R2.2, GND)

	TTL_7400_NAND(DD2_1, hsync, C1.2)
	NET_C(C1.2, R2.1)
	NET_C(DD2_1.Q, C2.1)
	NET_C(vsync, R3.1)
	NET_C(R3.2, C2.2)

	TTL_7400_NAND(DD2_2, C2.2, C2.2)
	TTL_7400_NAND(DD2_3, hsync, hsync)

	ALIAS(hsync, DD2_2.Q)

// video mixer

	RES(R10, 820)
	NET_C(hsync, R10.1)

	ALIAS(videomix, R10.2)
#else
	ALIAS(videomix, vsync)
#endif

	//LOG(log_V0, vsync)
	//LOG(log_V1, DD6_2.Q)

#if 1
	HINT(clk, NO_DEACTIVATE)
#endif

NETLIST_END()

