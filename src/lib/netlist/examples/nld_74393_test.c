// license:CC0
// copyright-holders:Couriersud
/*
 * cdelay.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(perf)
{

	SOLVER(Solver, 48000)

	ANALOG_INPUT(V5, 5)

	CLOCK(CLK, 100)             // clock for driving the 74393
	CLOCK(CLK2, 3)              // off-beat clock for master reset
	TTL_74393_DIP(TESTCHIP)

	NET_C(V5, CLK.VCC, CLK2.VCC, TESTCHIP.14)
	NET_C(GND, CLK.GND, CLK2.GND, TESTCHIP.7)

	NET_C(CLK, TESTCHIP.1, TESTCHIP.13)
	NET_C(CLK2, TESTCHIP.2, TESTCHIP.12)

}
