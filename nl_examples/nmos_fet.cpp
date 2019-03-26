// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"
#include "netlist/analog/nld_twoterm.h"

NETLIST_START(bjt)
    /* Standard stuff */

    CLOCK(clk, 100) // 100 Hz
    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.NR_LOOPS, 5000)
    ANALOG_INPUT(V5, 5)
    ANALOG_INPUT(V3, 3.5)

    /* NMOS - example */

	NET_MODEL("MM NMOS(VTO=1.0 KP=2e-3 LAMBDA=2E-2)")
    MOSFET(M, "MM")

    RES(RB, 1000)
    RES(RC, 10000)

    NET_C(RC.1, V5)
    NET_C(RC.2, M.D)
    NET_C(RB.1, clk)
    //NET_C(RB.1, V3)
    NET_C(RB.2, M.G)
    NET_C(M.S, GND)

    // put some load on M.D

    RES(RCE, 150000)
    NET_C(RCE.1, M.D)
    NET_C(RCE.2, GND)

	// capacitance over D - S

	CAP(C, CAP_N(1))
	NET_C(M.D, C.1)
	NET_C(M.S, C.2)

    LOG(log_G, M.G)
    LOG(log_D, M.D)

NETLIST_END()
