// license:CC0
// copyright-holders:Couriersud
/*
 * 7414.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(7414_astable)
{

	/*
	 * Simple oscillator with 74LS14
	 *
	 * ./nltool -f nl_examples/7414.cpp -l C37.1 -l U4A1.Q
	 *
	 */

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-8)

	TTL_74LS14_GATE(U4A1)
	//TTL_7404_GATE(U4A1)
	RES(R39, 2200)
	CAP(C37, CAP_U(1))
	NET_C(U4A1.A, R39.2, C37.1)
	NET_C(GND, C37.2)
	NET_C(GND, U4A1.GND)
	NET_C(R39.1, U4A1.Q)

}
