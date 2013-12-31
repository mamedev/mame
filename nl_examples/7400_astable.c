/*
 * 7400_astable.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(7400_astable)

    /*
     * Astable multivibrator using two 7400 gates (or inverters)
     *
     */

    /* Standard stuff */

    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)

    // astable NAND Multivibrator
    NETDEV_R(R1, 1000)
    NETDEV_C(C1, 1e-6)
    TTL_7400_NAND(n1,R1.1,R1.1)
    TTL_7400_NAND(n2,R1.2,R1.2)
    NET_C(n1.Q, R1.2)
    NET_C(n2.Q, C1.1)
    NET_C(C1.2, R1.1)

    NETDEV_LOG(log2, C1.2)
    //NETDEV_LOG(log2, n1.Q)
    NETDEV_LOG(log3, n2.Q)

NETLIST_END()
