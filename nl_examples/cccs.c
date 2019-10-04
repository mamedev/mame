// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * cccs.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(cccs)

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-12)
    PARAM(Solver.METHOD, "MAT_CR")

    CCCS(VV)
    PARAM(VV.G, 1)
    PARAM(VV.RI, 1e-3)

	RES(R2, 1000)
	RES(R1, 1000)
    NET_C(clk, VV.IP)
    NET_C(VV.IN, R1.1)
	NET_C(R1.2, GND)

    NET_C(R2.1, VV.OP)
    NET_C(R2.2, GND)
    NET_C(VV.ON, GND)

	/* Simple current source */

	CS(CS1, 1)
	RES(R3, 1)
	NET_C(R3.1, CS1.P)
	NET_C(GND, CS1.N, R3.2)

NETLIST_END()
