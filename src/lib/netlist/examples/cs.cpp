// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * cs.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(cs)

    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-12)
    PARAM(Solver.METHOD, "MAT_CR")

	/* Positive current flows into pin 1 of the current source
	 * Thus we observe a negative voltage on R1.1 !
	 */
    CS(CS1, 1e-3)
	RES(R1, 1000)

	NET_C(CS1.1, R1.1)
	NET_C(GND, CS1.2, R1.2)

	LOG(log_P, R1.1)
NETLIST_END()
