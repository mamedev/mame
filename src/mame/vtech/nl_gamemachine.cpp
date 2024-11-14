// license:CC0-1.0
// copyright-holders:Couriersud
// thanks-to:Sean Riddle

#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  Gamemachine schematics
 * ---------------------------------------------------------------------------*/

NETLIST_START(gamemachine)
{
	/* Standard stuff */

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	ANALOG_INPUT(V5, 5)

	/* Schematics: http://seanriddle.com/gamemachineaudio.JPG
	 *
	 * 3870 datasheet: http://nice.kaze.com/MK3870.pdf
	 *
	 * The 3870 has mask-programmable outputs (page VIII-7 in datasheet).
	 *
	 * Given the schematics, in this case the OPENDRAIN configuration is the
	 * most probable.
	 *
	 */

	NET_MODEL("OPENDRAIN FAMILY(TYPE=MOS OVL=0.0 OVH=0.0 ORL=1.0 ORH=1e12)")
	NET_MODEL("TYPE6K FAMILY(TYPE=MOS OVL=0.05 OVH=0.05 ORL=1.0 ORH=6000)")
	NET_MODEL("DIRECTDRIVE FAMILY(TYPE=MOS OVL=0.05 OVH=0.05 ORL=1.0 ORH=1000)")

	LOGIC_INPUT(P08, 1, "OPENDRAIN")
	LOGIC_INPUT(P09, 1, "OPENDRAIN")
	LOGIC_INPUT(P10, 1, "OPENDRAIN")
	LOGIC_INPUT(P11, 1, "OPENDRAIN")
	LOGIC_INPUT(P12, 1, "OPENDRAIN")
	LOGIC_INPUT(P13, 1, "OPENDRAIN")
	LOGIC_INPUT(P14, 1, "OPENDRAIN")
	LOGIC_INPUT(P15, 1, "OPENDRAIN")

	NET_C(V5,  P08.VDD, P09.VDD, P10.VDD, P11.VDD, P12.VDD, P13.VDD, P14.VDD, P15.VDD)
	NET_C(GND, P08.VSS, P09.VSS, P10.VSS, P11.VSS, P12.VSS, P13.VSS, P14.VSS, P15.VSS)

	RES(R1, RES_K(2.4))
	RES(R2, RES_K(10))
	RES(R3, RES_K(4.3))
	RES(R4, RES_K(150))
	RES(R5, RES_K(240))
	RES(R6, RES_K(2.4))
	RES(SPK1, 8)

	CAP(C1, CAP_P(50))
	CAP(C2, CAP_U(0.001))
	CAP(C3, CAP_U(0.002))
	CAP(C4, CAP_U(0.005))
	CAP(C5, CAP_U(0.010))

	CAP(C6, CAP_P(50))
	CAP(C7, CAP_U(0.01))
	CAP(C8, CAP_U(470))

	QBJT_EB(Q1, "9013")

	MC1455P_DIP(IC1)

	NET_C(P08.Q, R2.2, IC1.4)
	NET_C(P09.Q, C8.2)
	NET_C(P15.Q, R1.2)

	NET_C(C1.1, P10.Q)
	NET_C(C2.1, P11.Q)
	NET_C(C3.1, P12.Q)
	NET_C(C4.1, P13.Q)
	NET_C(C5.1, P14.Q)

	NET_C(C1.2, C2.2, C3.2, C4.2, C5.2, C6.2, IC1.2, IC1.6, R5.2)
	NET_C(GND, C6.1, IC1.1, Q1.E)
	NET_C(R5.1, R4.2, IC1.7)
	NET_C(V5, R4.1, R2.1, IC1.8, SPK1.1, R3.1)

	NET_C(C7.1, R6.1, IC1.3)

	NET_C(C7.2, R6.2, Q1.B)
	NET_C(Q1.C, SPK1.2)

	NET_C(C8.1, R1.1, R3.2, IC1.5)
}
