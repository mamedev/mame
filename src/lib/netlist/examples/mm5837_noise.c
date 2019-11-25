// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * mm5837_noise.c
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

    ANALOG_INPUT(V12, 12)  // 5V

    /* Wiring up the ne555 */

    // astable NE555, 1.13 ms period

    MM5837_DIP(NOISE)

    RES(R, 10000)

	NET_C(NOISE.1, NOISE.2, R.2, GND)
	NET_C(NOISE.4, V12)

	NET_C(NOISE.3, R.1)

	LOG(log3, NOISE.3)

NETLIST_END()
