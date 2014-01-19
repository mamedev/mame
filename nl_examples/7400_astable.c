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

    SOLVER(Solver)
    PARAM(Solver.FREQ, 48000)
    PARAM(Solver.ACCURACY, 1e-7)

    // astable NAND Multivibrator
    RES(R1, 1000)
    CAP(C1, 1e-6)
    TTL_7400_NAND(n1,R1.1,R1.1)
    TTL_7400_NAND(n2,R1.2,R1.2)
    NET_C(n1.Q, R1.2)
    NET_C(n2.Q, C1.1)
    NET_C(C1.2, R1.1)

    LOG(log2, C1.2)
    //LOG(log2, n1.Q)
    LOG(log3, n2.Q)

NETLIST_END()
