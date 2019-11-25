// license:GPL-2.0+
// copyright-holders:Couriersud
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

    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-5)

    // astable NAND Multivibrator ==> f ~ 1.0 / (3 * R * C)
    RES(R1, 4700)
    CAP(C1, 0.22e-6)
    TTL_7400_NAND(n1,R1.1,R1.1)
    TTL_7400_NAND(n2,R1.2,R1.2)
    NET_C(n1.Q, R1.2)
    NET_C(n2.Q, C1.1)
    NET_C(C1.2, R1.1)

    LOG(logC12, C1.2)
    LOG(logn1Q, n1.Q)
    LOG(logn2Q, n2.Q)

NETLIST_END()
