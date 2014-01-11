/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(bjt)
    /* Standard stuff */

    NETDEV_CLOCK(clk)
    NETDEV_PARAM(clk.FREQ, 1000) // 1000 Hz
    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)
    NETDEV_ANALOG_INPUT(V5, 5)
    NETDEV_ANALOG_INPUT(V3, 3.5)

    /* NPN - example */

    NETDEV_QJT_SW(Q, "BC237B")
    NETDEV_R(RB, 1000)
    NETDEV_R(RC, 1000)

    NET_C(RC.1, V5)
    NET_C(RC.2, Q.C)
    NET_C(RB.1, clk)
    NET_C(RB.2, Q.B)
    NET_C(Q.E, GND)

    /* PNP - example */

    NETDEV_QPNP(Q1, "BC556B")
    NETDEV_R(RB1, 1000)
    NETDEV_R(RC1, 1000)

    NET_C(RC1.1, GND)
    NET_C(RC1.2, Q1.C)
    NET_C(RB1.1, clk)
    NET_C(RB1.2, Q1.B)
    NET_C(Q1.E, V3)

    //NETDEV_LOG(logB, Q1.B)
    //NETDEV_LOG(logC, Q1.C)

NETLIST_END()
