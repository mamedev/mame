// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_other.h"

#include "netlist/devices/nld_system.h"

/*
 *   MC14584B: Hex Schmitt Trigger
 *             ON Semiconductor
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       Y1 |2           13| A6
 *       A2 |3           12| Y6
 *       Y2 |4  MC14584B 11| A5
 *       A3 |5           10| Y5
 *       Y3 |6            9| A4
 *      GND |7            8| Y4
 *          +--------------+
 *
 */

static NETLIST_START(MC14584B_DIP)
	MC14584B_GATE(A)
	MC14584B_GATE(B)
	MC14584B_GATE(C)
	MC14584B_GATE(D)
	MC14584B_GATE(E)
	MC14584B_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.Q,  /*    Y1 |2           13| A6   */ F.A,
		B.A,  /*    A2 |3           12| Y6   */ F.Q,
		B.Q,  /*    Y2 |4  MC14584B 11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Q,
		C.Q,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Q
			  /*       +--------------+      */
	)
NETLIST_END()

NETLIST_START(otheric_lib)
	TRUTHTABLE_START(MC14584B_GATE, 1, 1, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |100")
		TT_LINE(" 1 | 0 |100")
		// 2.1V negative going and 2.7V positive going at 5V
		TT_FAMILY("FAMILY(FV=0 IVL=0.42 IVH=0.54 OVL=0.05 OVH=0.05 ORL=10.0 ORH=10.0)")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(MC14584B_DIP)
NETLIST_END()
