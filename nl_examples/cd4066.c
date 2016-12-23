// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(cd4066)
    /* Standard stuff */

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)

    ANALOG_INPUT(V5, 5)

    CD_4066_DIP(SW)
    RES(R1, 1000)

    NET_C(SW.7,  GND)
    NET_C(SW.14, V5)

    NET_C(SW.13, clk)
    NET_C(SW.1, V5)

    NET_C(SW.2, R1.1)
    NET_C(R1.2, GND)

    // ground anything else

    NET_C(SW.3, GND)
    NET_C(SW.4, GND)
    NET_C(SW.5, GND)
    NET_C(SW.6, GND)
    NET_C(SW.8, GND)
    NET_C(SW.9, GND)
    NET_C(SW.10, GND)
    NET_C(SW.11, GND)
    NET_C(SW.12, GND)

    LOG(logB, clk)
    LOG(logC, R1.1)

NETLIST_END()
