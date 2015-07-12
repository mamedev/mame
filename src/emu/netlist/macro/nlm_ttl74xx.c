
#include "nlm_ttl74xx.h"

#include "devices/nld_truthtable.h"
#include "devices/nld_system.h"

/*
 *   DM7416: Hex Inverting Buffers with
 *   	     High Voltage Open-Collector Outputs
 *
 *          +--------------+
 *       A1 |1     ++    14| VCC
 *       Y1 |2           13| A6
 *       A2 |3           12| Y6
 *       Y2 |4    7416   11| A5
 *       A3 |5           10| Y5
 *       Y3 |6            9| A4
 *      GND |7            8| Y4
 *          +--------------+
 *
 */

NETLIST_START(TTL_7416_DIP)
	TTL_7416_GATE(s1)
	TTL_7416_GATE(s2)
	TTL_7416_GATE(s3)
	TTL_7416_GATE(s4)
	TTL_7416_GATE(s5)
	TTL_7416_GATE(s6)

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

NETLIST_START(TTL74XX_lib)
	TRUTHTABLE_START(TTL_7416_GATE, 1, 1, 0, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |15")
		TT_LINE(" 1 | 0 |23")
		/* Open Collector */
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(TTL_7416_DIP)
NETLIST_END()
