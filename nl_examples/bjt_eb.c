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
    NETDEV_PARAM(Solver.ACCURACY, 1e-4)
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

    //NETDEV_LOG(logB, Q.B)
    //NETDEV_LOG(logC, Q.C)

NETLIST_END()
