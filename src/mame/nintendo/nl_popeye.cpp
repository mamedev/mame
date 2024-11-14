// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  popeye schematics
 * ---------------------------------------------------------------------------*/

/* This is the output stage of the audio circuit.
 * It is solely an impedance changer and could be left away
 */

static NETLIST_START(popeye_imp_changer)
{
	RES(R62, 510000)
	RES(R63, 100)
	RES(R64, 510000)
	RES(R65, 2100)
	RES(R66, 330)
	RES(R67, 51)

	QBJT_EB(Q8, "2SC1815")
	QBJT_EB(Q9, "2SA1015")

	NET_C(V5, R62.1, Q8.C, R66.1)
	NET_C(R62.2, R64.1, R63.1, C7.2)
	NET_C(R63.2, Q8.B)
	NET_C(Q8.E, R65.1, Q9.B)
	NET_C(R66.2, Q9.E, R67.1)

	NET_C(GND, Q9.C, R65.2, R64.2)
}

NETLIST_START(popeye)
{

	/* register hard coded netlists */

	LOCAL_SOURCE(popeye_imp_changer)

	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-5)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000)
	RES(R_AY1_2, 1000)
	RES(R_AY1_3, 1000)

	RES(R52, 2000)
	RES(R55, 2000)
	RES(R58, 2000)
	RES(R53, 2000)
	RES(R56, 2000)
	RES(R59, 2000)
	RES(R51, 20000)
	RES(R57, 30000)
	RES(R60, 30000)

	RES(R61, 120000)

	RES(ROUT, 5000)

	CAP(C4, 0.047e-6)
	CAP(C5, 330e-12)
	CAP(C6, 330e-12)
	CAP(C7, 3.3e-6)
	CAP(C40, 680e-12)

	NET_C(V5, R_AY1_1.1, R_AY1_2.1, R_AY1_3.1)

	NET_C(R_AY1_1.2, R52.1, R53.1)
	NET_C(R_AY1_2.2, R55.1, R56.1)
	NET_C(R_AY1_3.2, R58.1, R59.1)

	NET_C(R53.2, R51.1, C4.1)
	NET_C(R56.2, R57.1, C5.1)
	NET_C(R59.2, R60.1, C6.1)

	NET_C(R51.2, R57.2, R60.2, R61.1, C40.1, C7.1)

	NET_C(GND, R52.2, R55.2, R58.2, C4.2, C5.2, C6.2, R61.2, C40.2)

	INCLUDE(popeye_imp_changer)

	/* output resistor (actually located in TV */

	NET_C(R67.2, ROUT.1)

	NET_C(GND, ROUT.2)

}
