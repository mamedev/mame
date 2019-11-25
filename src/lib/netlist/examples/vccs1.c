// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * vccs.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(vccs)

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-6)


    VCCS(VV)
    PARAM(VV.G, 100000)  // typical OP-AMP amplification
    RES(R1, 10000)
    RES(R2, 1)
    RES(R3, 1000)

    NET_C(clk, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R2.1, VV.OP)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(R2.2, GND)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    LOG(logX, VV.OP)
    LOG(logY, clk)

NETLIST_END()
