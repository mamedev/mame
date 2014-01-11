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

    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)

    NETDEV_ANALOG_INPUT(V5, 5)  // 5V

    /* Wiring up the ne555 */

    // astable NE555, 1.13 ms period

    NETDEV_R(RA, 5000)
    NETDEV_R(RB, 3000)
    NETDEV_C(C, 0.15e-6)
    NETDEV_NE555(555)

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

    NETDEV_LOG(log2, C.1)
    NETDEV_LOG(log3, 555.OUT)

NETLIST_END()
