// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_other.h"

#include "devices/nld_truthtable.h"
#include "devices/nld_system.h"

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

NETLIST_START(MC14584B_DIP)
	MC14584B_GATE(s1)
	MC14584B_GATE(s2)
	MC14584B_GATE(s3)
	MC14584B_GATE(s4)
	MC14584B_GATE(s5)
	MC14584B_GATE(s6)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.Q,  /*    Y1 |2           13| A6   */ s6.A,
		s2.A,  /*    A2 |3           12| Y6   */ s6.Q,
		s2.Q,  /*    Y2 |4    7416   11| A5   */ s5.A,
		s3.A,  /*    A3 |5           10| Y5   */ s5.Q,
		s3.Q,  /*    Y3 |6            9| A4   */ s4.A,
		GND.I, /*   GND |7            8| Y4   */ s4.Q
				/*       +--------------+      */
	)
NETLIST_END()

NETLIST_START(otheric_lib)
	TRUTHTABLE_START(MC14584B_GATE, 1, 1, 0, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |100")
		TT_LINE(" 1 | 0 |100")
		TT_FAMILY("FAMILY(IVL=2.1 IVH=2.7 OVL=0.05 OVH=4.95 ORL=10.0 ORH=10.0)")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(MC14584B_DIP)
NETLIST_END()
