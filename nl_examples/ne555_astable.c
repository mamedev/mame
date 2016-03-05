// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * ne555_astable.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(ne555_astable)

    /*
     * Astable ne555
     *
     */

    /* Standard stuff */

    SOLVER(Solver, 48000)

    ANALOG_INPUT(V5, 5)  // 5V

    /* Wiring up the ne555 */

    // astable NE555, 1.13 ms period

    RES(RA, 5000)
    RES(RB, 3000)
    CAP(C, 0.15e-6)
    NE555(555)

    NET_C(GND, 555.GND)
    NET_C(V5, 555.VCC)
    NET_C(V5, 555.RESET)

    NET_C(RA.1, 555.VCC)
    NET_C(RA.2, 555.DISCH)

    NET_C(RB.1, 555.DISCH)
    NET_C(RB.2, 555.TRIG)

    NET_C(RB.2, 555.THRESH)

    NET_C(555.TRIG, C.1)
    NET_C(C.2, GND)

    LOG(log2, C.1)
    LOG(log3, 555.OUT)

NETLIST_END()
