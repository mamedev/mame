// license:CC0
// copyright-holders:Couriersud

/*
 * zdiode.cpp
 *
 */

//! [zdiode_example]
#include "netlist/devices/net_lib.h"

// ./nltool -t 1 -l clk.Q -l ZD.K -v --extended src/lib/netlist/examples/zdiode.cpp
// ./plot_nl.sh clk.Q ZD.K
// clk.Q: clock output
// ZD.K:  voltage at Zener

NETLIST_START(zdiode)
{

	SOLVER(Solver, 48000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.VNTOL, 1e-7)
	PARAM(Solver.RELTOL, 1e-4)

	ANALOG_INPUT(V12, 12.0)
	ANALOG_INPUT(V4, 4.0)

	CLOCK(clk, 100)
	ZDIODE(ZD, "D(BV=7.5 IBV=0.01 NBV=3)")
	RES(R, 20)
	RES(Rclk, 200)
	CAP(C, 1e-6)

	NET_C(V4, clk.GND) // make clock between 4 and 12 V
	NET_C(V12, clk.VCC, R.1)

	NET_C(GND, ZD.A, C.2)
	NET_C(clk.Q, Rclk.1)
	NET_C(Rclk.2, R.2, ZD.K, C.1)

	RES(RL, 1000)
	NET_C(RL.1, ZD.K)
	NET_C(RL.2, GND)

}
//! [zdiode_example]
