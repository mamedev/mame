// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * 9602_mstable.c
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
    TTL_9602_DIP(mf)

    RES(R, 10000)
    CAP(C, 1e-6)

    NET_C(GND, mf.8) //
    NET_C(V5, mf.16) //

    NET_C(C.1, mf.1)
    NET_C(mf.1, GND) // Test - can be removed
    NET_C(C.2, mf.2, R.2)
    NET_C(R.1, V5)

    NET_C(mf.3, V5) // CLR
    NET_C(mf.4, GND)    // B
    NET_C(mf.5, clk.Q) // A

    LOG(logC, C.2)
    LOG(logQ, mf.6) //Q
    LOG(logX, clk.Q)

    // avoid non connected inputs

    NET_C(mf.11, V5)
    NET_C(mf.12, V5)
    NET_C(mf.13, V5)

NETLIST_END()
