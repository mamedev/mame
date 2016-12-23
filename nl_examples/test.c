// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */

//  CURRENTLY BROKEN

#include "netlist/devices/net_lib.h"

NETLIST_START(bjt)
    /* Standard stuff */

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)

    ANALOG_INPUT(V3, 3)
    ANALOG_INPUT(STOPG, 0)
    ALIAS(SRSTQ, RYf.2)
    ALIAS(SRST, RYc.2)
    NET_C(antenna, GND)
    ALIAS(runQ, Q1.C)

    TTL_7404_INVERT(e4d, STOPG)

    RES(RYf, 50)   // output impedance
    RES(RYc, 50)   // output impedance

    TTL_7404_INVERT(c9f, RYc.2)
    TTL_7404_INVERT(c9c, RYf.2)
    NET_C(c9f.Q, RYf.1)
    NET_C(c9c.Q, RYc.1)

    SWITCH2(coinsw,  RYc.2, RYf.2)

    NET_C(coinsw.Q, GND)

    /* Antenna circuit */
    /* Also has a diode to clamp negative voltages - omitted here */
    NETDEV_QNPN(Q3, BC237B)
    ALIAS(antenna, Q3.B)
    NET_C(GND, Q3.E)
    RES(RX5, 100)
    CAP(CX1, 100)
    NET_C(RX5.1, CX1.1)
    NET_C(RX5.1, Q3.C)
    NET_C(RX5.2, GND)
    NET_C(CX1.2, GND)
    NETDEV_QNPN(Q1, BC237B)
    NET_C(Q1.B, RX5.1)
    NET_C(Q1.E, GND)

    DIODE(D3, 1N914)
    NET_C(D3.A, Q1.C)
    NET_C(D3.K, SRSTQ)

    DIODE(D2, 1N914)
    RES(RX4, 220)
    NET_C(D2.K, e4d.Q)
    NET_C(D2.A, RX4.1)
    NET_C(RX4.2, Q3.C)

    RES(RX1, 100)
    RES(RX2, 100)
    RES(RX3, 330)
    CAP(CX2, CAP_U(0.1))

    NET_C(RX3.2, D3.A)
    NET_C(RX3.1, RX1.2)
    NET_C(RX1.1, V3)

    NET_C(RX1.1, CX2.1)
    NET_C(RX1.2, CX2.2)

    NETDEV_QPNP(Q2, BC556B)
    NET_C(Q2.E, V3)
    NET_C(Q2.B, RX1.2)
    NET_C(Q2.C, RX2.2)

    NET_C(RX2.1, D2.A)


    //LOG(logB, Q1.B)
    //LOG(logC, Q1.C)

NETLIST_END()
