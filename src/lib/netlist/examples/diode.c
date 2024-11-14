/*
 * diode.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(diode)
{
	/* Standard stuff */

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 48)
	ANALOG_INPUT(V5, 5)

	//DIODE(D, "1N914")
	DIODE(D, "D(IS=1e-15)")

	RES(R, RES_K(10))
	RES(R1, RES_K(10))

	NET_C(clk, D.K)
	NET_C(D.A, R.1)
	NET_C(R.2, V5)
	NET_C(R1.2, GND)
	NET_C(R.1, R1.1)

	LOG(logB, clk)
	LOG(logC, D.A)

}
