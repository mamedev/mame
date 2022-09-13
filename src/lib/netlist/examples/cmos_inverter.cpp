// license:CC0
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(cmos_inverter)
{
	/* Standard stuff */

	//EXTERNAL_SOURCE(modules_lib)

	//INCLUDE(modules_lib)

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.NR_LOOPS, 50)
	PARAM(Solver.METHOD, "MAT_CR")
	ANALOG_INPUT(V5, 5)

	RTEST(X)
	NET_C(X.1, V5)
	NET_C(X.2, GND)

	VS(IN, 5)
	PARAM(IN.FUNC, "T * 5")

	MOSFET(P, "PMOS(VTO=-1.0 KP=2e-3 LAMBDA=2E-2)")
	MOSFET(M, "NMOS(VTO=1.0 KP=2e-3 LAMBDA=2E-2)")

	NET_C(P.S, V5)
	NET_C(P.D, M.D)
	NET_C(GND, M.S, IN.N)

	NET_C(IN.P, M.G, P.G)

	// capacitance over D - S
#if 0
	CAP(C, CAP_N(1))
	NET_C(M.D, C.1)
	NET_C(M.S, C.2)
#endif
	LOG(log_G, M.G)
	LOG(log_D, M.D)

}
