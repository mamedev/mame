// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.c
 *
 */

#include "nld_opamps.h"
#include "../devices/net_lib.h"

NETLIST_START(opamp_lm3900)

	/*
	 *  Fast norton opamp model without bandwidth
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, R1.1) // Positive input
	ALIAS(MINUS, R2.1) // Negative input
	ALIAS(OUT, G1.OP) // Opamp output ...
	ALIAS(VM, G1.ON)  // V- terminal
	ALIAS(VP, DUMMY.I)  // V+ terminal

	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	RES(R1, 1)
	RES(R2, 1)
	NET_C(R1.1, G1.IP)
	NET_C(R2.1, G1.IN)
	NET_C(R1.2, R2.2, G1.ON)
	VCVS(G1)
	PARAM(G1.G, 10000000)
	//PARAM(G1.RI, 1)
	PARAM(G1.RO, RES_K(8))

NETLIST_END()
