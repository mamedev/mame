// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "devices/net_lib.h"

//
// ICL8038 is broadly similar to a 566 VCO, and can be simulated partially as such.
//

NETLIST_START(ICL8038_DIP)
	VCVS(VI, 1)
	CCCS(CI1, -1)
	CCCS(CI2, 2)
	SYS_COMPD(COMP)
	SYS_DSW2(SW)
	VCVS(VO, 1)
	RES(R_SHUNT, RES_R(50))

	PARAM(VO.RO, 50)
	PARAM(COMP.MODEL, "FAMILY(TYPE=CUSTOM IVL=0.16 IVH=0.4 OVL=0.01 OVH=0.01 ORL=50 ORH=50)")
	PARAM(SW.GOFF, 0) // This has to be zero to block current sources

	NET_C(VI.OP, CI1.IN, CI2.IN)
	NET_C(CI1.OP, VO.IP)
	NET_C(COMP.Q, SW.I)
	NET_C(CI2.OP, SW.2)
	NET_C(COMP.VCC, R_SHUNT.1)
	NET_C(SW.1, R_SHUNT.2)
	NET_C(SW.3, VO.IP)
	NET_C(VO.OP, COMP.IN)

	// Avoid singular Matrix due to G=0 switch
	RES(RX1, 1e10)
	RES(RX2, 1e10)
	NET_C(RX1.1, SW.1)
	NET_C(RX2.1, SW.3)

	NET_C(COMP.GND, RX1.2, RX2.2)

	RES(R1, 5000)
	RES(R2, 5000)
	RES(R3, 5000)

	// Square output wave
	VCVS(V_SQR, 1)
	NET_C(COMP.Q, V_SQR.IP)

	NET_C(COMP.GND, SW.GND, VI.ON, VI.IN, CI1.ON, CI2.ON, VO.IN, VO.ON, R2.2, V_SQR.IN, V_SQR.ON)
	NET_C(COMP.VCC, SW.VCC, R1.2)
	NET_C(COMP.IP, R1.1, R2.1, R3.1)
	NET_C(COMP.Q, R3.2)

	ALIAS(11, VI.ON) // GND
	ALIAS(9, V_SQR.OP) // Square out
	ALIAS(3, VO.OP) // Triag out
	ALIAS(8, VI.IP) // VC
	ALIAS(4, CI1.IP) // R1
	ALIAS(5, CI2.IP) // R2
	ALIAS(10, VO.IP) // C1
	ALIAS(6, COMP.VCC) // V+
NETLIST_END()
