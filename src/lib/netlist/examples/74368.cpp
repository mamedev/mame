// license:CC0-1.0
// copyright-holders:stonedDiscord

/*
 * 74368.cpp
 *
 */

//! [74368_example]
#include "netlist/devices/net_lib.h"

// ./nltool -c run -l RL.1 src/lib/netlist/examples/74368.cpp
//  RL.1 : Output

NETLIST_START(main)
{

	SOLVER(Solver, 50000)

	ANALOG_INPUT(VCC, 5.0)

	TTL_74368_DIP(X1)
	PARAM(X1.FORCE_TRISTATE_LOGIC, 0)

	CLOCK(C1, 100)
	CLOCK(C2, 300)
	CLOCK(C3,  5)

	RES(RL, 1000)
	RES(RM, 1000)

	NET_C(C1.Q, X1.2)
	NET_C(C2.Q, X1.4)
	NET_C(C3.Q, X1.1, X1.15)

	NET_C(X1.6, X1.10, X1.12, X1.14, RM.2, GND)
	NET_C(RM.1, X1.5, X1.7, X1.9, X1.11, X1.13)

	NET_C(X1.3, RL.1)

	NET_C(GND, RL.2, C1.GND, C2.GND, C3.GND, X1.8)
	NET_C(VCC,       C1.VCC, C2.VCC, C3.VCC, X1.16)

}
//! [74125_example]
