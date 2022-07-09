// license:CC0
// copyright-holders:Couriersud
/*
 * cccs.cpp
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(cccs)
{

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-12)
	PARAM(Solver.METHOD, "MAT_CR")

	ANALOG_INPUT(V5,5)
	CCCS(VV, 0.9)
	PARAM(VV.RI, 1e-3)

	RES(R2, 1000)
	RES(R1, 1000)

	NET_C(V5, clk.VCC)
	NET_C(clk, VV.IP)
	NET_C(VV.IN, R1.1)
	NET_C(GND, R1.2, R2.2, VV.OP, clk.GND)

	NET_C(R2.1, VV.ON)

	/* Simple current source */

	CS(CS1, 1)
	RES(R3, 1)
	NET_C(R3.1, CS1.P)
	NET_C(GND, CS1.N, R3.2)

}
