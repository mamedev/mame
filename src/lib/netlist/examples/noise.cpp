// license:CC0
// copyright-holders:Couriersud

/*
 * noise.cpp
 *
 */

//! [noise_example]
#include "netlist/devices/net_lib.h"

// ./nltool -t 1 -l R1.2 -n noise src/lib/netlist/examples/noise.cpp
//  X.3 : Square out
//  X.4 : Triangle out

NETLIST_START(noise)
{

	SOLVER(Solver, 48000)

	CLOCK(nclk, 2000)

	SYS_NOISE_MT_U(noise, 2.5)

	RES(R1,1000)
	RES(R2,1000)

	ANALOG_INPUT(VP, 12.0)

	NET_C(nclk.Q, noise.I)
	NET_C(VP, R1.1, nclk.VCC)
	NET_C(noise.1, R1.2)
	NET_C(noise.2, R2.1)
	NET_C(GND, R2.2, nclk.GND)

}
//! [noise_example]
