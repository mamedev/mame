// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(bjt)
    /* Standard stuff */

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)
    ANALOG_INPUT(V5, 5)
    ANALOG_INPUT(V3, 3.5)

    /* NPN - example */

    QBJT_SW(Q, "BC237B")
    RES(RB, 1000)
    RES(RC, 1000)

    NET_C(RC.1, V5)
    NET_C(RC.2, Q.C)
    NET_C(RB.1, clk)
    NET_C(RB.2, Q.B)
    NET_C(Q.E, GND)

    /* PNP - example */

    QBJT_SW(Q1, "BC556B")
    RES(RB1, 1000)
    RES(RC1, 1000)

    NET_C(RC1.1, GND)
    NET_C(RC1.2, Q1.C)
    NET_C(RB1.1, clk)
    NET_C(RB1.2, Q1.B)
    NET_C(Q1.E, V3)

    LOG(logB, Q.B)
    LOG(logC, Q.C)

NETLIST_END()
