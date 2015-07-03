/*
 * vs_cs.c
 *
 * Voltage and current source test
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(rc)

    /*
     * delay circuit
     *
     */

    /* Standard stuff */

    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-6)
    CLOCK(clk, 20000)

	/* Voltage source. Inner resistance will make clock visible */

    RES(R1, 1000)
	VS(VS1, 1)
	NET_C(R1.1, clk)
	NET_C(R1.2, VS1.P)
	NET_C(GND, VS1.N)

	/* Current source. Current flows from "+" to "-", thus for a source we
	 * need negative current
	 */

    RES(R2, 1000)
    RES(R3, 1000)
	CS(CS1, -0.001)
	NET_C(CS1.P, R2.1)
	NET_C(R2.2, R3.1)
	NET_C(GND, CS1.N, R3.2)

	CAP(C1, CAP_U(1))
	NET_C(C1.1, R3.1)
	NET_C(C1.2, R3.2)


    LOG(tt, VS1.P)
    LOG(tx, R2.2)

NETLIST_END()
