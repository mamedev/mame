// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_ttl74xx.h"

#include "devices/nld_truthtable.h"
#include "devices/nld_system.h"


/*
 *  DM7400: Quad 2-Input NAND Gates
 *
 *                  __
 *              Y = AB
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 1 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

NETLIST_START(TTL_7400_DIP)
	TTL_7400_GATE(s1)
	TTL_7400_GATE(s2)
	TTL_7400_GATE(s3)
	TTL_7400_GATE(s4)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| B4   */ s4.B,
		s1.Q,  /*    Y1 |3           12| A4   */ s4.A,
		s2.A,  /*    A2 |4    7400   11| Y4   */ s4.Q,
		s2.B,  /*    B2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7402: Quad 2-Input NOR Gates
 *
 *              Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 1 |
 *          | 0 | 1 || 0 |
 *          | 1 | 0 || 0 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

NETLIST_START(TTL_7402_DIP)
	TTL_7402_GATE(s1)
	TTL_7402_GATE(s2)
	TTL_7402_GATE(s3)
	TTL_7402_GATE(s4)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.Q,  /*    Y1 |1     ++    14| VCC  */ VCC.I,
		s1.A,  /*    A1 |2           13| Y4   */ s4.Q,
		s1.B,  /*    B1 |3           12| B4   */ s4.B,
		s2.Q,  /*    Y2 |4    7402   11| A4   */ s4.A,
		s2.A,  /*    A2 |5           10| Y3   */ s3.Q,
		s2.B,  /*    B2 |6            9| B3   */ s3.B,
		GND.I, /*   GND |7            8| A3   */ s3.A
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7404: Hex Inverting Gates
 *                 _
 *             Y = A
 *          +---++---+
 *          | A || Y |
 *          +===++===+
 *          | 0 || 1 |
 *          | 1 || 0 |
 *          +---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

NETLIST_START(TTL_7404_DIP)
	TTL_7404_GATE(s1)
	TTL_7404_GATE(s2)
	TTL_7404_GATE(s3)
	TTL_7404_GATE(s4)
	TTL_7404_GATE(s5)
	TTL_7404_GATE(s6)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.Q,  /*    Y1 |2           13| A6   */ s6.A,
		s2.A,  /*    A2 |3           12| Y6   */ s6.Q,
		s2.Q,  /*    Y2 |4    7404   11| A5   */ s5.A,
		s3.A,  /*    A3 |5           10| Y5   */ s5.Q,
		s3.Q,  /*    Y3 |6            9| A4   */ s4.A,
		GND.I, /*   GND |7            8| Y4   */ s4.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7408: Quad 2-Input AND Gates
 *
 *
 *              Y = AB
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 0 |
 *          | 1 | 0 || 0 |
 *          | 1 | 1 || 1 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

NETLIST_START(TTL_7408_DIP)
	TTL_7408_GATE(s1)
	TTL_7408_GATE(s2)
	TTL_7408_GATE(s3)
	TTL_7408_GATE(s4)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| B4   */ s4.B,
		s1.Q,  /*    Y1 |3           12| A4   */ s4.A,
		s2.A,  /*    A2 |4    7400   11| Y4   */ s4.Q,
		s2.B,  /*    B2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7410: Triple 3-Input NAND Gates
 *                  ___
 *              Y = ABC
 *          +---+---+---++---+
 *          | A | B | C || Y |
 *          +===+===+===++===+
 *          | X | X | 0 || 1 |
 *          | X | 0 | X || 1 |
 *          | 0 | X | X || 1 |
 *          | 1 | 1 | 1 || 0 |
 *          +---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

NETLIST_START(TTL_7410_DIP)
	TTL_7410_GATE(s1)
	TTL_7410_GATE(s2)
	TTL_7410_GATE(s3)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| C1   */ s1.C,
		s2.A,  /*    A2 |3           12| Y1   */ s1.Q,
		s2.B,  /*    B2 |4    7400   11| C3   */ s3.C,
		s2.C,  /*    C2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7416: Hex Inverting Buffers with
 *           High Voltage Open-Collector Outputs
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

	TRUTHTABLE_START(TTL_7400_GATE, 2, 1, 0, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7400_NAND, 2, 1, 0, "A,B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7402_GATE, 2, 1, 0, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|1|22")
		TT_LINE("X,1|0|15")
		TT_LINE("1,X|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7402_NOR, 2, 1, 0, "A,B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|1|22")
		TT_LINE("X,1|0|15")
		TT_LINE("1,X|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7404_GATE, 1, 1, 0, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |22")
		TT_LINE(" 1 | 0 |15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7404_INVERT, 1, 1, 0, "A")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |22")
		TT_LINE(" 1 | 0 |15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7408_GATE, 2, 1, 0, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|0|15")
		TT_LINE("X,0|0|15")
		TT_LINE("1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7408_AND, 2, 1, 0, "A,B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|0|15")
		TT_LINE("X,0|0|15")
		TT_LINE("1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7410_NAND, 3, 1, 0, "A,B")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|1|22")
		TT_LINE("X,0,X|1|22")
		TT_LINE("X,X,0|1|22")
		TT_LINE("1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7410_GATE, 3, 1, 0, "")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|1|22")
		TT_LINE("X,0,X|1|22")
		TT_LINE("X,X,0|1|22")
		TT_LINE("1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7416_GATE, 1, 1, 0, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |15")
		TT_LINE(" 1 | 0 |23")
		/* Open Collector */
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(TTL_7400_DIP)
	LOCAL_LIB_ENTRY(TTL_7402_DIP)
	LOCAL_LIB_ENTRY(TTL_7404_DIP)
	LOCAL_LIB_ENTRY(TTL_7408_DIP)
	LOCAL_LIB_ENTRY(TTL_7410_DIP)
	LOCAL_LIB_ENTRY(TTL_7416_DIP)
NETLIST_END()
