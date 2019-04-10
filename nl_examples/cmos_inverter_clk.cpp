// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * cmos_inverter_clk
 *
 */


#include "netlist/devices/net_lib.h"
#include "netlist/analog/nld_twoterm.h"

#define USE_CLOCK (1)

NETLIST_START(cmos_inverter_clk)
    /* Standard stuff */

	//SOLVER(Solver, 480000)
	SOLVER(Solver, 1e5)
	//SOLVER(Solver, 100000000000)
	PARAM(Solver.ACCURACY, 1e-6	)
	PARAM(Solver.NR_LOOPS, 500000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e-5)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-9)
    ANALOG_INPUT(V5, 5)

//	CLOCK(clk, 0.5e6)

#if (USE_CLOCK)
	CLOCK(V, 5000)
	PARAM(NETLIST.DEFAULT_MOS_CAPMODEL, 2) // Disable capacitance modeling
	//CLOCK(V, 500000)
#else
	VS(V, 5)
	PARAM(V.FUNC, "T * 5e6")
#endif

    MOSFET(P, "PMOS(VTO=-0.5 GAMMA=0.5 TOX=20n)")
    MOSFET(M, "NMOS(VTO=0.5 GAMMA=0.5 TOX=20n)")
	RES(RG, 1)

	NET_C(P.S, V5)
	NET_C(P.D, M.D)
#if (USE_CLOCK)
	NET_C(GND, M.S)
	NET_C(V.Q, RG.1)
#else
	NET_C(GND, M.S, V.N)
	NET_C(V.P, RG.1)
#endif
	NET_C(RG.2, M.G, P.G)

	// capacitance over D - S
#if 0
	CAP(C, CAP_N(1))
	NET_C(M.D, C.1)
	NET_C(M.S, C.2)
#endif
#if 1
    LOG(log_G, M.G)
    LOG(log_D, M.D)
	LOGD(log_X, RG.1, RG.2)
#endif
NETLIST_END()
