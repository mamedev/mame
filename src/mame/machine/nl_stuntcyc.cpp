// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

/***************************************************************************

  Netlist (stuntcyc) included from atarittl.cpp

***************************************************************************/

#include "netlist/devices/net_lib.h"

NETLIST_START(stuntcyc)

	SOLVER(Solver, 48000)
	PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(Solver.ACCURACY, 1e-4) // works and is sufficient
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	ANALOG_INPUT(V5, 5)

	TTL_INPUT(high, 1)
	TTL_INPUT(low, 0)

	MAINCLOCK(main_clk, 14318181.8)
	ALIAS(Y1, main_clk)

NETLIST_END()
