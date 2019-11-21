// license:CC0
// copyright-holders:Couriersud

#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  cocoloco schematics
 * ---------------------------------------------------------------------------*/

// 7W LM383T Amplifier not included below

NETLIST_START(cocoloco)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000)
	RES(R_AY1_2, 1000)
	RES(R_AY1_3, 1000)

	RES(R1, 4700)
	RES(R2, 4700)
	RES(R3, 4700)
	RES(RAMP, 150000)
	//RES(RAMP, 150)
	POT(P1, 5000)
	PARAM(P1.DIAL, 0.5) // 50%

	CAP(C1, 10e-6)

	NET_C(V5, R_AY1_1.1, R_AY1_2.1, R_AY1_3.1)

	NET_C(R_AY1_1.2, R1.1)
	NET_C(R_AY1_2.2, R2.1)
	NET_C(R_AY1_3.2, R3.1)

	NET_C(R1.2, R2.2, R3.2, P1.1)

	NET_C(P1.3, RAMP.2, GND)
	NET_C(P1.2, C1.1)
	NET_C(C1.2, RAMP.1)
#if 0
	CAP(C2, 0.1e-6)
	NET_C(C2.2, GND)
	NET_C(C2.1, RAMP.1)
#endif
NETLIST_END()

