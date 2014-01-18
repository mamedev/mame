/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(bjt)
    /* Standard stuff */

    NETDEV_CLOCK(clk)
    NETDEV_PARAM(clk.FREQ, 10000) // 1000 Hz
    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)
    NETDEV_PARAM(Solver.ACCURACY, 1e-6)
    NETDEV_PARAM(Solver.RESCHED_LOOPS, 50)
    NETDEV_ANALOG_INPUT(V5, 5)
    NETDEV_ANALOG_INPUT(V3, 3.5)

    /* NPN - example */

    NETDEV_QBJT_EB(Q, "BC237B")
    NETDEV_R(RB, 1000)
    NETDEV_R(RC, 1000)

    NET_C(RC.1, V5)
    NET_C(RC.2, Q.C)
    NET_C(RB.1, clk)
    //NET_C(RB.1, V3)
    NET_C(RB.2, Q.B)
    NET_C(Q.E, GND)

    // put some load on Q.C

    NETDEV_R(RCE, 150000)
    NET_C(RCE.1, Q.C)
    NET_C(RCE.2, GND)

    NETDEV_LOG(logB, Q.B)
    NETDEV_LOG(logC, Q.C)

NETLIST_END()
