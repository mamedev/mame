// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  1942 schematics
 * ---------------------------------------------------------------------------*/

NETLIST_START(1942)
{

	/* Standard stuff */

	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)

	/* AY 8910 internal resistors */

	RES(R_AY1_1, 1000)
	RES(R_AY1_2, 1000)
	RES(R_AY1_3, 1000)
	RES(R_AY2_1, 1000)
	RES(R_AY2_2, 1000)
	RES(R_AY2_3, 1000)

	RES(R2, 220000)
	RES(R3, 220000)
	RES(R4, 220000)
	RES(R5, 220000)
	RES(R6, 220000)
	RES(R7, 220000)

	RES(R11, 10000)
	RES(R12, 10000)
	RES(R13, 10000)
	RES(R14, 10000)
	RES(R15, 10000)
	RES(R16, 10000)

	CAP(CC7, 10e-6)
	CAP(CC8, 10e-6)
	CAP(CC9, 10e-6)
	CAP(CC10, 10e-6)
	CAP(CC11, 10e-6)
	CAP(CC12, 10e-6)

	NET_C(V5, R_AY2_3.1, R_AY2_2.1, R_AY2_1.1, R_AY1_3.1, R_AY1_2.1, R_AY1_1.1)
	NET_C(GND, R13.2, R15.2, R11.2, R12.2, R14.2, R16.2)
	//NLFILT(R_AY2_3, R13, CC7, R2)
	NET_C(R_AY2_3.2, R13.1)
	NET_C(R13.1, CC7.1)
	NET_C(CC7.2, R2.1)
	//NLFILT(R_AY2_2, R15, CC8, R3)
	NET_C(R_AY2_2.2, R15.1)
	NET_C(R15.1, CC8.1)
	NET_C(CC8.2, R3.1)
	//NLFILT(R_AY2_1, R11, CC9, R4)
	NET_C(R_AY2_1.2, R11.1)
	NET_C(R11.1, CC9.1)
	NET_C(CC9.2, R4.1)

	//NLFILT(R_AY1_3, R12, CC10, R5)
	NET_C(R_AY1_3.2, R12.1)
	NET_C(R12.1, CC10.1)
	NET_C(CC10.2, R5.1)
	//NLFILT(R_AY1_2, R14, CC11, R6)
	NET_C(R_AY1_2.2, R14.1)
	NET_C(R14.1, CC11.1)
	NET_C(CC11.2, R6.1)
	//NLFILT(R_AY1_1, R16, CC12, R7)
	NET_C(R_AY1_1.2, R16.1)
	NET_C(R16.1, CC12.1)
	NET_C(CC12.2, R7.1)

	POT(VR, 2000)
	NET_C(VR.3, GND)

	NET_C(R2.2, VR.1)
	NET_C(R3.2, VR.1)
	NET_C(R4.2, VR.1)
	NET_C(R5.2, VR.1)
	NET_C(R6.2, VR.1)
	NET_C(R7.2, VR.1)

	CAP(CC6, 10e-6)
	RES(R1, 100000)

	NET_C(CC6.1, VR.2)
	NET_C(CC6.2, R1.1)
	CAP(CC3, 220e-6)
	NET_C(R1.2, CC3.1)
	NET_C(CC3.2, GND)

}
