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
    PARAM(Solver.ACCURACY, 1e-12)
    PARAM(Solver.GS_LOOPS, 10000)

    VCCS(VV)
    PARAM(VV.G, 100)  // typical OP-AMP amplification
    RES(R2, 1)

    NET_C(clk, VV.IN)
    NET_C(R2.1, VV.OP)
    NET_C(R2.2, GND)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    LOG(logX, VV.OP)
    LOG(logY, clk)

NETLIST_END()
