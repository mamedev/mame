// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * 74123_mstable.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(74123_mstable)

    /*
     * Monoflog
     *
     */

    /* Standard stuff */

    SOLVER(Solver, 48000)

    ANALOG_INPUT(V5, 5)  // 5V

    /* Wiring up the 74123 */

    CLOCK(clk, 50)
    TTL_74123(mf)

    RES(R, 10000)
    CAP(C, 1e-6)

    NET_C(GND, mf.GND)
    NET_C(V5, mf.VCC)

    NET_C(C.1, mf.C)
    NET_C(mf.C, GND)
    NET_C(C.2, mf.RC, R.2)
    NET_C(R.1, V5)

    NET_C(mf.CLRQ, V5)
    NET_C(mf.B,    V5)
    NET_C(mf.A, clk.Q)

    LOG(logC, C.2)
    LOG(logQ, mf.Q)
    LOG(logX, clk.Q)

NETLIST_END()
