// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//

#include "devices/net_lib.h"

NETLIST_START(NE556_DIP)
	NE555(A)
	NE555(B)

	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)

	DIPPINS(      /*        +--------------+        */
		 A.DISCH, /* 1DISCH |1     ++    14| VCC    */ A.VCC,
		A.THRESH, /* 1THRES |2           13| 2DISCH */ B.DISCH,
		  A.CONT, /*  1CONT |3           12| 2THRES */ B.THRESH,
		 A.RESET, /* 1RESET |4   NE556   11| 2CONT  */ B.CONT,
		   A.OUT, /*   1OUT |5           10| 2RESET */ B.RESET,
		  A.TRIG, /*  1TRIG |6            9| 2OUT   */ B.OUT,
		   A.GND, /*    GND |7            8| 2TRIG  */ B.TRIG
				  /*        +--------------+        */
	)
NETLIST_END()
