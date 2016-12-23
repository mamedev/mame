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

    CLOCK(clk, 10000) // 10000 Hz
    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-6)
    PARAM(Solver.RESCHED_LOOPS, 30)
    ANALOG_INPUT(V5, 5)
    ANALOG_INPUT(V3, 3.5)

    /* NPN - example */

    QBJT_EB(Q, "BC237B")
    RES(RB, 1000)
    RES(RC, 1000)

    NET_C(RC.1, V5)
    NET_C(RC.2, Q.C)
    NET_C(RB.1, clk)
    //NET_C(RB.1, V3)
    NET_C(RB.2, Q.B)
    NET_C(Q.E, GND)

    // put some load on Q.C

    RES(RCE, 150000)
    NET_C(RCE.1, Q.C)
    NET_C(RCE.2, GND)

    //LOG(logB, Q.B)
    //LOG(logC, Q.C)

NETLIST_END()
