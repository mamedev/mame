// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * cdelay.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(7400_astable)

    /*
     * delay circuit
     *
     */

    /* Standard stuff */

    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-20)
    CLOCK(clk, 5000)

    TTL_7400_NAND(n1,clk,clk)
    CAP(C, 1e-9)
    NET_C(n1.Q, C.2)
    NET_C(GND, C.1)
    TTL_7400_NAND(n2,n1.Q, n1.Q)

    LOG(logclk, clk)
    LOG(logn1Q, C.2)
    LOG(logn2Q, n2.Q)

NETLIST_END()
