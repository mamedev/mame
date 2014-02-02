/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(bjt)
    /* Standard stuff */

    NETDEV_CLOCK(clk)
    NETDEV_PARAM(clk.FREQ, 1000) // 1000 Hz
    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)

    NETDEV_ANALOG_INPUT(V3, 3)
    NETDEV_ANALOG_INPUT(STOPG, 0)
    NET_ALIAS(SRSTQ, RYf.2)
    NET_ALIAS(SRST, RYc.2)
    NET_C(antenna, GND)
    NET_ALIAS(runQ, Q1.C)

    TTL_7404_INVERT(e4d, STOPG)

    NETDEV_R(RYf, 50)   // output impedance
    NETDEV_R(RYc, 50)   // output impedance

    TTL_7404_INVERT(c9f, RYc.2)
    TTL_7404_INVERT(c9c, RYf.2)
    NET_C(c9f.Q, RYf.1)
    NET_C(c9c.Q, RYc.1)

    NETDEV_SWITCH2(coinsw,  RYc.2, RYf.2)

    NET_C(coinsw.Q, GND)

    /* Antenna circuit */
    /* Also has a diode to clamp negative voltages - omitted here */
    NETDEV_QNPN(Q3, BC237B)
    NET_ALIAS(antenna, Q3.B)
    NET_C(GND, Q3.E)
    NETDEV_R(RX5, 100)
    NETDEV_C(CX1, 100)
    NET_C(RX5.1, CX1.1)
    NET_C(RX5.1, Q3.C)
    NET_C(RX5.2, GND)
    NET_C(CX1.2, GND)
    NETDEV_QNPN(Q1, BC237B)
    NET_C(Q1.B, RX5.1)
    NET_C(Q1.E, GND)

    NETDEV_D(D3, 1N914)
    NET_C(D3.A, Q1.C)
    NET_C(D3.K, SRSTQ)

    NETDEV_D(D2, 1N914)
    NETDEV_R(RX4, 220)
    NET_C(D2.K, e4d.Q)
    NET_C(D2.A, RX4.1)
    NET_C(RX4.2, Q3.C)

    NETDEV_R(RX1, 100)
    NETDEV_R(RX2, 100)
    NETDEV_R(RX3, 330)
    NETDEV_C(CX2, CAP_U(0.1))

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


    //NETDEV_LOG(logB, Q1.B)
    //NETDEV_LOG(logC, Q1.C)

NETLIST_END()
