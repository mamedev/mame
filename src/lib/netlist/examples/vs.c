// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * vs_cs.c
 *
 * Voltage and current source test
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(vs)

    /* Standard stuff */

    SOLVER(Solver, 480)
    PARAM(Solver.ACCURACY, 1e-6)
	PARAM(Solver.METHOD, "MAT_CR")
	PARAM(Solver.DYNAMIC_TS, 1)

	RES(R1, 1000)
	VS(VS1, 1)
	PARAM(VS1.R, 1)
	PARAM(VS1.FUNC, "T 10000.0 * sin 1.0 +")
	NET_C(R1.1, VS1.P)
	NET_C(R1.2, VS1.N)
	NET_C(GND, VS1.N)

    //LOG(tt, VS1.P)
    //LOG(tt1, R1.1)

NETLIST_END()
