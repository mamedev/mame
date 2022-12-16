/*
 * cdelay.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(rc)
{

	/*
	 * delay circuit
	 *
	 */

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-6)
	CLOCK(clk, 20000)

	CAP(C1, 0.022e-6)
	RES(R1, 10000)
	RES(R2, 20000)

	NET_C(clk, R2.1)
	NET_C(R2.2, R1.1, C1.1)
	NET_C(C1.2, R1.2, GND)

	LOG(tt, C1.1)

}
