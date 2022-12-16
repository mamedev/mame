// license:CC0
// copyright-holders:Couriersud

/*
 * 74125.cpp
 *
 */

//! [74125_example]
#include "netlist/devices/net_lib.h"

// ./nltool -c run -l RL.1 src/lib/netlist/examples/74125.cpp
//  RL.1 : Output

NETLIST_START(main)
{

	SOLVER(Solver, 48000) // could be 1 in this example

	ANALOG_INPUT(VCC, 5.0)

	TTL_74125_GATE(X1)
	TTL_74126_GATE(X2)
	PARAM(X2.FORCE_TRISTATE_LOGIC, 0)

	CLOCK(C1, 100)
	CLOCK(C2, 300)
	CLOCK(C3,  5)

	RES(RL, 1000)

	NET_C(C1.Q, X1.A)
	NET_C(C2.Q, X2.A)
	NET_C(C3.Q, X1.GQ, X2.G)

	NET_C(X1.Y, X2.Y, RL.1)
	//NET_C(X1.Y, RL.1)

	NET_C(GND, RL.2, C1.GND, C2.GND, C3.GND, X1.GND, X2.GND)
	NET_C(VCC,       C1.VCC, C2.VCC, C3.VCC, X1.VCC, X2.VCC)

}
//! [74125_example]
