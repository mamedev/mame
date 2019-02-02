// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_ttl74xx.h"

#include "../devices/nld_schmitt.h"
#include "../devices/nld_system.h"


/*
 *  DM7400: Quad 2-Input NAND Gates
 *
 *                  _
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

static NETLIST_START(TTL_7400_DIP)
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

static NETLIST_START(TTL_7402_DIP)
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
 *
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

static NETLIST_START(TTL_7404_DIP)
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

static NETLIST_START(TTL_7408_DIP)
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
 *                  __
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

static NETLIST_START(TTL_7410_DIP)
	TTL_7410_GATE(s1)
	TTL_7410_GATE(s2)
	TTL_7410_GATE(s3)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| C1   */ s1.C,
		s2.A,  /*    A2 |3           12| Y1   */ s1.Q,
		s2.B,  /*    B2 |4    7410   11| C3   */ s3.C,
		s2.C,  /*    C2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7411: Triple 3-Input AND Gates
 *
 *              Y = ABC
 *          +---+---+---++---+
 *          | A | B | C || Y |
 *          +===+===+===++===+
 *          | X | X | 0 || 0 |
 *          | X | 0 | X || 0 |
 *          | 0 | X | X || 0 |
 *          | 1 | 1 | 1 || 1 |
 *          +---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7411_DIP)
	TTL_7411_GATE(s1)
	TTL_7411_GATE(s2)
	TTL_7411_GATE(s3)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| C1   */ s1.C,
		s2.A,  /*    A2 |3           12| Y1   */ s1.Q,
		s2.B,  /*    B2 |4    7411   11| C3   */ s3.C,
		s2.C,  /*    C2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7414/DM74LS14: Hex Inverter with
 *                    Schmitt Trigger Inputs
 *
 */

static NETLIST_START(TTL_7414_GATE)
	SCHMITT_TRIGGER(X, "DM7414")
	ALIAS(A, X.A)
	ALIAS(Q, X.Q)
	ALIAS(GND, X.GND)
NETLIST_END()

static NETLIST_START(TTL_74LS14_GATE)
	SCHMITT_TRIGGER(X, "DM74LS14")
	ALIAS(A, X.A)
	ALIAS(Q, X.Q)
	ALIAS(GND, X.GND)
NETLIST_END()

static NETLIST_START(TTL_7414_DIP)
	SCHMITT_TRIGGER(s1, "DM7414")
	SCHMITT_TRIGGER(s2, "DM7414")
	SCHMITT_TRIGGER(s3, "DM7414")
	SCHMITT_TRIGGER(s4, "DM7414")
	SCHMITT_TRIGGER(s5, "DM7414")
	SCHMITT_TRIGGER(s6, "DM7414")

	NET_C(s1.GND, s2.GND, s3.GND, s4.GND, s5.GND, s6.GND)
	DUMMY_INPUT(VCC)

	DIPPINS(    /*       +--------------+      */
		s1.A,   /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.Q,   /*    Y1 |2           13| A6   */ s6.A,
		s2.A,   /*    A2 |3           12| Y6   */ s6.Q,
		s2.Q,   /*    Y2 |4    7414   11| A5   */ s5.A,
		s3.A,   /*    A3 |5           10| Y5   */ s5.Q,
		s3.Q,   /*    Y3 |6            9| A4   */ s4.A,
		s1.GND, /*   GND |7            8| Y4   */ s4.Q
				/*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74LS14_DIP)
	SCHMITT_TRIGGER(s1, "DM74LS14")
	SCHMITT_TRIGGER(s2, "DM74LS14")
	SCHMITT_TRIGGER(s3, "DM74LS14")
	SCHMITT_TRIGGER(s4, "DM74LS14")
	SCHMITT_TRIGGER(s5, "DM74LS14")
	SCHMITT_TRIGGER(s6, "DM74LS14")

	NET_C(s1.GND, s2.GND, s3.GND, s4.GND, s5.GND, s6.GND)
	DUMMY_INPUT(VCC)

	DIPPINS(    /*       +--------------+      */
		s1.A,   /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.Q,   /*    Y1 |2           13| A6   */ s6.A,
		s2.A,   /*    A2 |3           12| Y6   */ s6.Q,
		s2.Q,   /*    Y2 |4   74LS14  11| A5   */ s5.A,
		s3.A,   /*    A3 |5           10| Y5   */ s5.Q,
		s3.Q,   /*    Y3 |6            9| A4   */ s4.A,
		s1.GND, /*   GND |7            8| Y4   */ s4.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7416: Hex Inverting Buffers with
 *           High Voltage Open-Collector Outputs
 *
 */

static NETLIST_START(TTL_7416_DIP)
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

/*
 *  DM7420: Dual 4-Input NAND Gates
 *
 *                  ___
 *              Y = ABCD
 *          +---+---+---+---++---+
 *          | A | B | C | D || Y |
 *          +===+===+===+===++===+
 *          | X | X | X | 0 || 1 |
 *          | X | X | 0 | X || 1 |
 *          | X | 0 | X | X || 1 |
 *          | 0 | X | X | X || 1 |
 *          | 1 | 1 | 1 | 1 || 0 |
 *          +---+---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet *
 */

static NETLIST_START(TTL_7420_DIP)
	TTL_7420_GATE(s1)
	TTL_7420_GATE(s2)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)
	DUMMY_INPUT(NC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| D2   */ s2.D,
		NC.I,  /*    NC |3           12| C2   */ s2.C,
		s1.C,  /*    C1 |4    7420   11| NC   */ NC.I,
		s1.D,  /*    D1 |5           10| B2   */ s2.B,
		s1.Q,  /*    Y1 |6            9| A2   */ s2.A,
		GND.I, /*   GND |7            8| Y2   */ s2.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7425: Dual 4-Input NOR Gates
 *
 *                  ______
 *              Y = A+B+C+D
 *          +---+---+---+---+---++---+
 *          | A | B | C | D | X || Y |
 *          +===+===+===+===+===++===+
 *          | X | X | X | X | 0 || Z |
 *          | 0 | 0 | 0 | 0 | 1 || 1 |
 *          | X | X | X | 1 | 1 || 0 |
 *          | X | X | 1 | X | 1 || 0 |
 *          | X | 1 | X | X | 1 || 0 |
 *          | 1 | X | X | X | 1 || 0 |
 *          +---+---+---+---+---++---+
 *
 *  FIXME: The "X" input and high impedance output are currently not simulated.
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7425_DIP)
	TTL_7425_GATE(s1)
	TTL_7425_GATE(s2)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)
	DUMMY_INPUT(X)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| D2   */ s2.D,
			X.I,  /*    X1 |3           12| C2   */ s2.C,
		s1.C,  /*    C1 |4    7425   11| X2   */  X.I,
		s1.D,  /*    D1 |5           10| B2   */ s2.B,
		s1.Q,  /*    Y1 |6            9| A2   */ s2.A,
		GND.I, /*   GND |7            8| Y2   */ s2.Q
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7427: Triple 3-Input NOR Gates
 *
 *                  ____
 *              Y = A+B+C
 *          +---+---+---++---+
 *          | A | B | C || Y |
 *          +===+===+===++===+
 *          | X | X | 1 || 0 |
 *          | X | 1 | X || 0 |
 *          | 1 | X | X || 0 |
 *          | 0 | 0 | 0 || 1 |
 *          +---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7427_DIP)
	TTL_7427_GATE(s1)
	TTL_7427_GATE(s2)
	TTL_7427_GATE(s3)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| C1   */ s1.C,
		s2.A,  /*    A2 |3           12| Y1   */ s1.Q,
		s2.B,  /*    B2 |4    7427   11| C3   */ s3.C,
		s2.C,  /*    C2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7430: 8-Input NAND Gate
 *
 *                  _______
 *              Y = ABCDEFGH
 *          +---+---+---+---+---+---+---+---++---+
 *          | A | B | C | D | E | F | G | H || Y |
 *          +===+===+===+===+===+===+===+===++===+
 *          | X | X | X | X | X | X | X | 0 || 1 |
 *          | X | X | X | X | X | X | 0 | X || 1 |
 *          | X | X | X | X | X | 0 | X | X || 1 |
 *          | X | X | X | X | 0 | X | X | X || 1 |
 *          | X | X | X | 0 | X | X | X | X || 1 |
 *          | X | X | 0 | X | X | X | X | X || 1 |
 *          | X | 0 | X | X | X | X | X | X || 1 |
 *          | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 || 0 |
 *          +---+---+---+---+---+---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 */

static NETLIST_START(TTL_7430_DIP)
	TTL_7430_GATE(s1)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)
	DUMMY_INPUT(NC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*     A |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*     B |2           13| NC   */ NC.I,
		s1.C,  /*     C |3           12| H    */ s1.H,
		s1.D,  /*     D |4    7430   11| G    */ s1.G,
		s1.E,  /*     E |5           10| NC   */ NC.I,
		s1.F,  /*     F |6            9| NC   */ NC.I,
		GND.I, /*   GND |7            8| Y    */ s1.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM7432: Quad 2-Input OR Gates
 *
 *                  __
 *              Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 1 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7432_DIP)
	TTL_7432_GATE(s1)
	TTL_7432_GATE(s2)
	TTL_7432_GATE(s3)
	TTL_7432_GATE(s4)

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
 *  DM7437: Quad 2-Input NAND Gates
 *
 *                  _
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
 *  NOTE: Same as 7400, but drains higher output currents.
 *         Netlist currently does not model over currents (should it ever?)
 */

static NETLIST_START(TTL_7437_DIP)
	TTL_7437_GATE(s1)
	TTL_7437_GATE(s2)
	TTL_7437_GATE(s3)
	TTL_7437_GATE(s4)

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
 *  DM7486: Quad 2-Input Exclusive-OR Gates
 *
 *             Y = A+B
 *          +---+---++---+
 *          | A | B || Y |
 *          +===+===++===+
 *          | 0 | 0 || 0 |
 *          | 0 | 1 || 1 |
 *          | 1 | 0 || 1 |
 *          | 1 | 1 || 0 |
 *          +---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7486_DIP)
	TTL_7486_GATE(s1)
	TTL_7486_GATE(s2)
	TTL_7486_GATE(s3)
	TTL_7486_GATE(s4)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.A,  /*    A1 |1     ++    14| VCC  */ VCC.I,
		s1.B,  /*    B1 |2           13| B4   */ s4.B,
		s1.Q,  /*    Y1 |3           12| A4   */ s4.A,
		s2.A,  /*    A2 |4    7486   11| Y4   */ s4.Q,
		s2.B,  /*    B2 |5           10| B3   */ s3.B,
		s2.Q,  /*    Y2 |6            9| A3   */ s3.A,
		GND.I, /*   GND |7            8| Y3   */ s3.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 * DM74155/DM74156: Dual 2-Line to 4-Line Decoders/Demultiplexers
 *
 *      +-----+-------++-----------------+
 *      | B A | G1 C1 || 1Y0 1Y1 1Y2 1Y3 |
 *      +=====+=======++=================+
 *      | X X | 1  X  ||  1   1   1   1  |
 *      | 0 0 | 0  1  ||  0   1   1   1  |
 *      | 0 1 | 0  1  ||  1   0   1   1  |
 *      | 1 0 | 0  1  ||  1   1   0   1  |
 *      | 1 1 | 0  1  ||  1   1   1   0  |
 *      | X X | X  0  ||  1   1   1   1  |
 *      +-----+-------++-----------------+
 *
 *      +-----+-------++-----------------+
 *      | B A | G2 C2 || 2Y0 2Y1 2Y2 2Y3 |
 *      +=====+=======++=================+
 *      | X X | 1  X  ||  1   1   1   1  |
 *      | 0 0 | 0  0  ||  0   1   1   1  |
 *      | 0 1 | 0  0  ||  1   0   1   1  |
 *      | 1 0 | 0  0  ||  1   1   0   1  |
 *      | 1 1 | 0  0  ||  1   1   1   0  |
 *      | X X | X  1  ||  1   1   1   1  |
 *      +-----+-------++-----------------+
 *
 * Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_74155_DIP)
	NET_REGISTER_DEV(TTL_74155A_GATE, s1)
	NET_REGISTER_DEV(TTL_74155B_GATE, s2)

	NET_C(s1.A, s2.A)
	NET_C(s1.B, s2.B)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.C,  /*    C1 |1     ++    16| VCC  */ VCC.I,
		s1.G,  /*    G1 |2           15| B4   */ s2.C,
		s1.B,  /*     B |3           14| B4   */ s2.G,
		s1.3,  /*   1Y3 |4   74155   13| A4   */ s2.A,
		s2.2,  /*   1Y2 |5           12| Y4   */ s2.3,
		s2.1,  /*   1Y1 |6           11| B3   */ s2.2,
		s2.0,  /*   1Y0 |7           10| A3   */ s2.1,
		GND.I, /*   GND |8            9| Y3   */ s2.0
			   /*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74156_DIP)
	NET_REGISTER_DEV(TTL_74156A_GATE, s1)
	NET_REGISTER_DEV(TTL_74156B_GATE, s2)

	NET_C(s1.A, s2.A)
	NET_C(s1.B, s2.B)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.C,  /*    C1 |1     ++    16| VCC  */ VCC.I,
		s1.G,  /*    G1 |2           15| B4   */ s2.C,
		s1.B,  /*     B |3           14| B4   */ s2.G,
		s1.3,  /*   1Y3 |4   74156   13| A4   */ s2.A,
		s2.2,  /*   1Y2 |5           12| Y4   */ s2.3,
		s2.1,  /*   1Y1 |6           11| B3   */ s2.2,
		s2.0,  /*   1Y0 |7           10| A3   */ s2.1,
		GND.I, /*   GND |8            9| Y3   */ s2.0
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM74260: Dual 5-Input NOR Gates
 *                 _________
 *             Y = A+B+C+D+E
 *          +---+---+---+---+---++---+
 *          | A | B | B | B | B || Y |
 *          +===+===+===+===+===++===+
 *          | 0 | 0 | 0 | 0 | 0 || 1 |
 *          | 0 | 0 | 0 | 0 | 1 || 0 |
 *          | 0 | 0 | 0 | 1 | 0 || 0 |
 *          | 0 | 0 | 1 | 0 | 0 || 0 |
 *          | 0 | 1 | 0 | 0 | 0 || 0 |
 *          | 1 | 0 | 0 | 0 | 0 || 0 |
 *          +---+---+---+---+---++---+
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

static NETLIST_START(TTL_74260_DIP)
	TTL_74260_GATE(s1)
	TTL_74260_GATE(s2)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(   /*       +--------------+      */
		s1.C,  /*    C1 |1     ++    14| VCC  */ VCC.I,
		s1.D,  /*    D1 |2           13| B1   */ s1.B,
		s1.E,  /*    E1 |3           12| A1   */ s1.A,
		s2.E,  /*    E2 |4   74260   11| D2   */ s2.D,
		s1.Q,  /*    Y1 |5           10| C2   */ s2.C,
		s2.Q,  /*    Y2 |6            9| B2   */ s2.B,
		GND.I, /*   GND |7            8| A2   */ s2.A
				/*       +--------------+      */
	)
NETLIST_END()

/*
 *  DM74279: Quad S-R Latch
 *
 *          +---+---+---++---+
 *          |S1 |S2 | R || Q |
 *          +===+===+===++===+
 *          | 0 | 0 | 0 || 1 |
 *          | 0 | 1 | 1 || 1 |
 *          | 1 | 0 | 1 || 1 |
 *          | 1 | 1 | 0 || 0 |
 *          | 1 | 1 | 1 ||QP |
 *          +---+---+---++---+
 *
 *  QP: Previous Q
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef __PLIB_PREPROCESSOR__
#define TTL_74279A(name)                                                         \
		NET_REGISTER_DEV(TTL_74279A, name)
#define TTL_74279B(name)                                                         \
		NET_REGISTER_DEV(TTL_74279B, name)
#endif

static NETLIST_START(TTL_74279_DIP)
	TTL_74279B(s1)
	TTL_74279A(s2)
	TTL_74279B(s3)
	TTL_74279A(s4)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

	DIPPINS(    /*     +--------------+     */
		s1.R,   /*  1R |1     ++    16| VCC */ VCC.I,
		s1.S1,  /* 1S1 |2           15| 4S  */ s4.S,
		s1.S2,  /* 1S2 |3           14| 4R  */ s4.R,
		s1.Q,   /*  1Q |4    74279  13| 4Q  */ s4.Q,
		s2.R,   /*  2R |5           12| 3S2 */ s3.S2,
		s2.S,   /*  2S |6           11| 3S1 */ s3.S1,
		s2.Q,   /*  2Q |7           10| 3R  */ s3.R,
		GND.I,  /* GND |8            9| 3Q  */ s3.Q
				/*     +--------------+     */
	)
NETLIST_END()

/*
 *  DM9312: One of Eight Line Data Selectors/Multiplexers
 *
 *          +--------------+
 *       D0 |1     ++    16| VCC
 *       D1 |2           15| Y
 *       D2 |3           14| YQ
 *       D3 |4    9312   13| C
 *       D4 |5           12| B
 *       D5 |6           11| A
 *       D6 |7           10| G   Strobe
 *      GND |8            9| D7
 *          +--------------+
 *                  __
 *          +---+---+---+---++---+---+
 *          | C | B | A | G || Y | YQ|
 *          +===+===+===+===++===+===+
 *          | X | X | X | 1 ||  0| 1 |
 *          | 0 | 0 | 0 | 0 || D0|D0Q|
 *          | 0 | 0 | 1 | 0 || D1|D1Q|
 *          | 0 | 1 | 0 | 0 || D2|D2Q|
 *          | 0 | 1 | 1 | 0 || D3|D3Q|
 *          | 1 | 0 | 0 | 0 || D4|D4Q|
 *          | 1 | 0 | 1 | 0 || D5|D5Q|
 *          | 1 | 1 | 0 | 0 || D6|D6Q|
 *          | 1 | 1 | 1 | 0 || D7|D7Q|
 *          +---+---+---+---++---+---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef __PLIB_PREPROCESSOR__
#define DM9312_TT(name)     \
		NET_REGISTER_DEV(DM9312_TT, name)
#endif

static NETLIST_START(DM9312_DIP)
	DM9312_TT(s)

	DUMMY_INPUT(GND)
	DUMMY_INPUT(VCC)

DIPPINS(        /*     +--------------+     */
		s.D0,   /*  D0 |1     ++    16| VCC */ VCC.I,
		s.D1,   /*  D1 |2           15| Y   */ s.Y,
		s.D2,   /*  D2 |3           14| YQ  */ s.YQ,
		s.D3,   /*  D3 |4    9312   13| C   */ s.C,
		s.D4,   /*  D4 |5           12| B   */ s.B,
		s.D5,   /*  D5 |6           11| A   */ s.A,
		s.D6,   /*  D6 |7           10| G   */ s.G, //Strobe
		GND.I,  /* GND |8            9| D7  */ s.D7
				/*     +--------------+     */
	)
NETLIST_END()


NETLIST_START(TTL74XX_lib)
	NET_MODEL("DM7414         SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL("TTL_7414_GATE  SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL("DM74LS14       SCHMITT_TRIGGER(VTP=1.6 VTM=0.8 VI=4.4 RI=19.3k VOH=3.45 ROH=130 VOL=0.1 ROL=31.2 TPLH=15 TPHL=15)")
	//NET_MODEL("DM7414 FAMILY(FV=5 IVL=0.16 IVH=0.4 OVL=0.1 OVH=0.05 ORL=10.0 ORH=1.0e8)")


	TRUTHTABLE_START(TTL_7400_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7400_NAND, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7402_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|1|22")
		TT_LINE("X,1|0|15")
		TT_LINE("1,X|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7402_NOR, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|1|22")
		TT_LINE("X,1|0|15")
		TT_LINE("1,X|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7404_GATE, 1, 1, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |22")
		TT_LINE(" 1 | 0 |15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7404_INVERT, 1, 1, "+A")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |22")
		TT_LINE(" 1 | 0 |15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7408_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|0|15")
		TT_LINE("X,0|0|15")
		TT_LINE("1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7408_AND, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|0|15")
		TT_LINE("X,0|0|15")
		TT_LINE("1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7410_NAND, 3, 1, "+A,+B,+C")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|1|22")
		TT_LINE("X,0,X|1|22")
		TT_LINE("X,X,0|1|22")
		TT_LINE("1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7410_GATE, 3, 1, "")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|1|22")
		TT_LINE("X,0,X|1|22")
		TT_LINE("X,X,0|1|22")
		TT_LINE("1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7411_AND, 3, 1, "+A,+B,+C")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|0|15")
		TT_LINE("X,0,X|0|15")
		TT_LINE("X,X,0|0|15")
		TT_LINE("1,1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7411_GATE, 3, 1, "")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("0,X,X|0|15")
		TT_LINE("X,0,X|0|15")
		TT_LINE("X,X,0|0|15")
		TT_LINE("1,1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7416_GATE, 1, 1, "")
		TT_HEAD(" A | Q ")
		TT_LINE(" 0 | 1 |15")
		TT_LINE(" 1 | 0 |23")
		/* Open Collector */
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7420_GATE, 4, 1, "")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("0,X,X,X|1|22")
		TT_LINE("X,0,X,X|1|22")
		TT_LINE("X,X,0,X|1|22")
		TT_LINE("X,X,X,0|1|22")
		TT_LINE("1,1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7420_NAND, 4, 1, "+A,+B,+C,+D")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("0,X,X,X|1|22")
		TT_LINE("X,0,X,X|1|22")
		TT_LINE("X,X,0,X|1|22")
		TT_LINE("X,X,X,0|1|22")
		TT_LINE("1,1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7425_GATE, 4, 1, "")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("1,X,X,X|0|15")
		TT_LINE("X,1,X,X|0|15")
		TT_LINE("X,X,1,X|0|15")
		TT_LINE("X,X,X,1|0|15")
		TT_LINE("0,0,0,0|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7425_NOR, 4, 1, "+A,+B,+C,+D")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("1,X,X,X|0|15")
		TT_LINE("X,1,X,X|0|15")
		TT_LINE("X,X,1,X|0|15")
		TT_LINE("X,X,X,1|0|15")
		TT_LINE("0,0,0,0|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7427_GATE, 3, 1, "")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("1,X,X|0|15")
		TT_LINE("X,1,X|0|15")
		TT_LINE("X,X,1|0|15")
		TT_LINE("0,0,0|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7427_NOR, 3, 1, "+A,+B,+C")
		TT_HEAD("A,B,C|Q ")
		TT_LINE("1,X,X|0|15")
		TT_LINE("X,1,X|0|15")
		TT_LINE("X,X,1|0|15")
		TT_LINE("0,0,0|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7430_GATE, 8, 1, "")
		TT_HEAD("A,B,C,D,E,F,G,H|Q ")
		TT_LINE("0,X,X,X,X,X,X,X|1|22")
		TT_LINE("X,0,X,X,X,X,X,X|1|22")
		TT_LINE("X,X,0,X,X,X,X,X|1|22")
		TT_LINE("X,X,X,0,X,X,X,X|1|22")
		TT_LINE("X,X,X,X,0,X,X,X|1|22")
		TT_LINE("X,X,X,X,X,0,X,X|1|22")
		TT_LINE("X,X,X,X,X,X,0,X|1|22")
		TT_LINE("X,X,X,X,X,X,X,0|1|22")
		TT_LINE("1,1,1,1,1,1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7430_NAND, 8, 1, "+A,+B,+C,+D,+E,+F,+G,+H")
		TT_HEAD("A,B,C,D,E,F,G,H|Q ")
		TT_LINE("0,X,X,X,X,X,X,X|1|22")
		TT_LINE("X,0,X,X,X,X,X,X|1|22")
		TT_LINE("X,X,0,X,X,X,X,X|1|22")
		TT_LINE("X,X,X,0,X,X,X,X|1|22")
		TT_LINE("X,X,X,X,0,X,X,X|1|22")
		TT_LINE("X,X,X,X,X,0,X,X|1|22")
		TT_LINE("X,X,X,X,X,X,0,X|1|22")
		TT_LINE("X,X,X,X,X,X,X,0|1|22")
		TT_LINE("1,1,1,1,1,1,1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7432_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("1,X|1|22")
		TT_LINE("X,1|1|22")
		TT_LINE("0,0|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7432_OR, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("1,X|1|22")
		TT_LINE("X,1|1|22")
		TT_LINE("0,0|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	/*  FIXME: Same as 7400, but drains higher output currents.
	 *         Netlist currently does not model over currents (should it ever?)
	 */

	TRUTHTABLE_START(TTL_7437_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7437_NAND, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|22")
		TT_LINE("X,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7486_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|0|15")
		TT_LINE("0,1|1|22")
		TT_LINE("1,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7486_XOR, 2, 1, "+A,+B")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|0|15")
		TT_LINE("0,1|1|22")
		TT_LINE("1,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74155A_GATE, 4, 4, "")
		TT_HEAD("B,A,G,C|0,1,2,3")
		TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
		TT_LINE("X,X,0,0|1,1,1,1|13,13,13,13")
		TT_LINE("0,0,0,1|0,1,1,1|13,13,13,13")
		TT_LINE("0,1,0,1|1,0,1,1|13,13,13,13")
		TT_LINE("1,0,0,1|1,1,0,1|13,13,13,13")
		TT_LINE("1,1,0,1|1,1,1,0|13,13,13,13")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74155B_GATE, 4, 4, "")
		TT_HEAD("B,A,G,C|0,1,2,3")
		TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
		TT_LINE("X,X,0,1|1,1,1,1|13,13,13,13")
		TT_LINE("0,0,0,0|0,1,1,1|13,13,13,13")
		TT_LINE("0,1,0,0|1,0,1,1|13,13,13,13")
		TT_LINE("1,0,0,0|1,1,0,1|13,13,13,13")
		TT_LINE("1,1,0,0|1,1,1,0|13,13,13,13")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74156A_GATE, 4, 4, "")
		TT_HEAD("B,A,G,C|0,1,2,3")
		TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
		TT_LINE("X,X,0,0|1,1,1,1|13,13,13,13")
		TT_LINE("0,0,0,1|0,1,1,1|13,13,13,13")
		TT_LINE("0,1,0,1|1,0,1,1|13,13,13,13")
		TT_LINE("1,0,0,1|1,1,0,1|13,13,13,13")
		TT_LINE("1,1,0,1|1,1,1,0|13,13,13,13")
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74156B_GATE, 4, 4, "")
		TT_HEAD("B,A,G,C|0,1,2,3")
		TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
		TT_LINE("X,X,0,1|1,1,1,1|13,13,13,13")
		TT_LINE("0,0,0,0|0,1,1,1|13,13,13,13")
		TT_LINE("0,1,0,0|1,0,1,1|13,13,13,13")
		TT_LINE("1,0,0,0|1,1,0,1|13,13,13,13")
		TT_LINE("1,1,0,0|1,1,1,0|13,13,13,13")
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74260_GATE, 5, 1, "")
		TT_HEAD("A,B,C,D,E|Q ")
		TT_LINE("0,0,0,0,0|1|10")
		TT_LINE("X,X,X,X,1|0|12")
		TT_LINE("X,X,X,1,X|0|12")
		TT_LINE("X,X,1,X,X|0|12")
		TT_LINE("X,1,X,X,X|0|12")
		TT_LINE("1,X,X,X,X|0|12")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74260_NOR, 5, 1, "+A,+B,+C,+D,+E")
		TT_HEAD("A,B,C,D,E|Q ")
		TT_LINE("0,0,0,0,0|1|10")
		TT_LINE("X,X,X,X,1|0|12")
		TT_LINE("X,X,X,1,X|0|12")
		TT_LINE("X,X,1,X,X|0|12")
		TT_LINE("X,1,X,X,X|0|12")
		TT_LINE("1,X,X,X,X|0|12")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	// FIXME: We need "private" devices
	TRUTHTABLE_START(TTL_74279A, 3, 1, "")
		TT_HEAD("S,R,_Q|Q")
		TT_LINE("0,X,X|1|22")
		TT_LINE("1,0,X|0|27")
		TT_LINE("1,1,0|0|27")
		TT_LINE("1,1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_74279B, 4, 1, "")
		TT_HEAD("S1,S2,R,_Q|Q")
		TT_LINE("0,X,X,X|1|22")
		TT_LINE("X,0,X,X|1|22")
		TT_LINE("1,1,0,X|0|27")
		TT_LINE("1,1,1,0|0|27")
		TT_LINE("1,1,1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(DM9312_TT, 12, 2, "+A,+B,+C,+G,+D0,+D1,+D2,+D3,+D4,+D5,+D6,+D7")
		TT_HEAD(" C, B, A, G,D0,D1,D2,D3,D4,D5,D6,D7| Y,YQ")
		TT_LINE(" X, X, X, 1, X, X, X, X, X, X, X, X| 0, 1|33,19")
		TT_LINE(" 0, 0, 0, 0, 0, X, X, X, X, X, X, X| 0, 1|33,28")
		TT_LINE(" 0, 0, 0, 0, 1, X, X, X, X, X, X, X| 1, 0|33,28")
		TT_LINE(" 0, 0, 1, 0, X, 0, X, X, X, X, X, X| 0, 1|33,28")
		TT_LINE(" 0, 0, 1, 0, X, 1, X, X, X, X, X, X| 1, 0|33,28")
		TT_LINE(" 0, 1, 0, 0, X, X, 0, X, X, X, X, X| 0, 1|33,28")
		TT_LINE(" 0, 1, 0, 0, X, X, 1, X, X, X, X, X| 1, 0|33,28")
		TT_LINE(" 0, 1, 1, 0, X, X, X, 0, X, X, X, X| 0, 1|33,28")
		TT_LINE(" 0, 1, 1, 0, X, X, X, 1, X, X, X, X| 1, 0|33,28")
		TT_LINE(" 1, 0, 0, 0, X, X, X, X, 0, X, X, X| 0, 1|33,28")
		TT_LINE(" 1, 0, 0, 0, X, X, X, X, 1, X, X, X| 1, 0|33,28")
		TT_LINE(" 1, 0, 1, 0, X, X, X, X, X, 0, X, X| 0, 1|33,28")
		TT_LINE(" 1, 0, 1, 0, X, X, X, X, X, 1, X, X| 1, 0|33,28")
		TT_LINE(" 1, 1, 0, 0, X, X, X, X, X, X, 0, X| 0, 1|33,28")
		TT_LINE(" 1, 1, 0, 0, X, X, X, X, X, X, 1, X| 1, 0|33,28")
		TT_LINE(" 1, 1, 1, 0, X, X, X, X, X, X, X, 0| 0, 1|33,28")
		TT_LINE(" 1, 1, 1, 0, X, X, X, X, X, X, X, 1| 1, 0|33,28")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(TTL_7400_DIP)
	LOCAL_LIB_ENTRY(TTL_7402_DIP)
	LOCAL_LIB_ENTRY(TTL_7404_DIP)
	LOCAL_LIB_ENTRY(TTL_7408_DIP)
	LOCAL_LIB_ENTRY(TTL_7410_DIP)
	LOCAL_LIB_ENTRY(TTL_7411_DIP)
	LOCAL_LIB_ENTRY(TTL_7414_GATE)
	LOCAL_LIB_ENTRY(TTL_74LS14_GATE)
	LOCAL_LIB_ENTRY(TTL_7414_DIP)
	LOCAL_LIB_ENTRY(TTL_74LS14_DIP)
	LOCAL_LIB_ENTRY(TTL_7416_DIP)
	LOCAL_LIB_ENTRY(TTL_7420_DIP)
	LOCAL_LIB_ENTRY(TTL_7425_DIP)
	LOCAL_LIB_ENTRY(TTL_7427_DIP)
	LOCAL_LIB_ENTRY(TTL_7430_DIP)
	LOCAL_LIB_ENTRY(TTL_7432_DIP)
	LOCAL_LIB_ENTRY(TTL_7437_DIP)
	LOCAL_LIB_ENTRY(TTL_7486_DIP)
	LOCAL_LIB_ENTRY(TTL_74155_DIP)
	LOCAL_LIB_ENTRY(TTL_74156_DIP)
	LOCAL_LIB_ENTRY(TTL_74260_DIP)
	LOCAL_LIB_ENTRY(TTL_74279_DIP)
	LOCAL_LIB_ENTRY(DM9312_DIP)
NETLIST_END()
