/*
 * lr.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(lr)
{

	/*
	 * delay circuit
	 *
	 */

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-6)
	CLOCK(clk, 50)
	PARAM(Solver.METHOD, "MAT_CR")

	IND(L1, 10)
	RES(R1, 10000)

	NET_C(clk, L1.1)
	NET_C(L1.2, R1.1)
	NET_C(R1.2, GND)

	//LOG(log_1, R1.1)
	//LOG(log_2, clk)

}
