// license:GPL-2.0+
// copyright-holders:Couriersud

/*
 * ne566.cpp
 *
 */

//! [ne566_example]
#include "netlist/devices/net_lib.h"


NETLIST_START(oscillator)

	SOLVER(Solver, 48000)

	ANALOG_INPUT(I_V12, 12.0)

	ANALOG_INPUT(I_VC, 11.9 - 0.2)

	NE566_DIP(X)

	RES(R1, 10000)  // ~20 Hz.
	CAP(C1, 1e-6)
	NET_C(I_V12, R1.1, X.8)
	NET_C(R1.2, X.6)
	NET_C(I_VC, X.5)
	NET_C(GND, X.1, C1.2)
	NET_C(C1.1, X.7)
NETLIST_END()
//! [ne566_example]
