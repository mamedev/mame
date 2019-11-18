// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_ttl74xx.h"

#include "netlist/devices/nld_schmitt.h"
#include "netlist/devices/nld_system.h"


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
	TTL_7400_GATE(A)
	TTL_7400_GATE(B)
	TTL_7400_GATE(C)
	TTL_7400_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| B4   */ D.B,
		A.Q,  /*    Y1 |3           12| A4   */ D.A,
		B.A,  /*    A2 |4    7400   11| Y4   */ D.Q,
		B.B,  /*    B2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7402_GATE(A)
	TTL_7402_GATE(B)
	TTL_7402_GATE(C)
	TTL_7402_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.Q,  /*    Y1 |1     ++    14| VCC  */ A.VCC,
		A.A,  /*    A1 |2           13| Y4   */ D.Q,
		A.B,  /*    B1 |3           12| B4   */ D.B,
		B.Q,  /*    Y2 |4    7402   11| A4   */ D.A,
		B.A,  /*    A2 |5           10| Y3   */ C.Q,
		B.B,  /*    B2 |6            9| B3   */ C.B,
		A.GND,/*   GND |7            8| A3   */ C.A
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
	TTL_7404_GATE(A)
	TTL_7404_GATE(B)
	TTL_7404_GATE(C)
	TTL_7404_GATE(D)
	TTL_7404_GATE(E)
	TTL_7404_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.Q,  /*    Y1 |2           13| A6   */ F.A,
		B.A,  /*    A2 |3           12| Y6   */ F.Q,
		B.Q,  /*    Y2 |4    7404   11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Q,
		C.Q,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Q
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
	TTL_7408_GATE(A)
	TTL_7408_GATE(B)
	TTL_7408_GATE(C)
	TTL_7408_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| B4   */ D.B,
		A.Q,  /*    Y1 |3           12| A4   */ D.A,
		B.A,  /*    A2 |4    7400   11| Y4   */ D.Q,
		B.B,  /*    B2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7410_GATE(A)
	TTL_7410_GATE(B)
	TTL_7410_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| C1   */ A.C,
		B.A,  /*    A2 |3           12| Y1   */ A.Q,
		B.B,  /*    B2 |4    7410   11| C3   */ C.C,
		B.C,  /*    C2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7411_GATE(A)
	TTL_7411_GATE(B)
	TTL_7411_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| C1   */ A.C,
		B.A,  /*    A2 |3           12| Y1   */ A.Q,
		B.B,  /*    B2 |4    7411   11| C3   */ C.C,
		B.C,  /*    C2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	ALIAS(VCC, X.VCC)
NETLIST_END()

static NETLIST_START(TTL_74LS14_GATE)
	SCHMITT_TRIGGER(X, "DM74LS14")
	ALIAS(A, X.A)
	ALIAS(Q, X.Q)
	ALIAS(GND, X.GND)
	ALIAS(VCC, X.VCC)
NETLIST_END()

static NETLIST_START(TTL_7414_DIP)
	SCHMITT_TRIGGER(A, "DM7414")
	SCHMITT_TRIGGER(B, "DM7414")
	SCHMITT_TRIGGER(C, "DM7414")
	SCHMITT_TRIGGER(D, "DM7414")
	SCHMITT_TRIGGER(E, "DM7414")
	SCHMITT_TRIGGER(F, "DM7414")

	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)

	DIPPINS(   /*       +--------------+      */
		A.A,   /*    A1 |1     ++    14| VCC  */ VCC.I,
		A.Q,   /*    Y1 |2           13| A6   */ F.A,
		B.A,   /*    A2 |3           12| Y6   */ F.Q,
		B.Q,   /*    Y2 |4    7414   11| A5   */ E.A,
		C.A,   /*    A3 |5           10| Y5   */ E.Q,
		C.Q,   /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Q
			   /*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74LS14_DIP)
	SCHMITT_TRIGGER(A, "DM74LS14")
	SCHMITT_TRIGGER(B, "DM74LS14")
	SCHMITT_TRIGGER(C, "DM74LS14")
	SCHMITT_TRIGGER(D, "DM74LS14")
	SCHMITT_TRIGGER(E, "DM74LS14")
	SCHMITT_TRIGGER(F, "DM74LS14")

	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)

	DIPPINS(   /*       +--------------+      */
		A.A,   /*    A1 |1     ++    14| VCC  */ VCC.I,
		A.Q,   /*    Y1 |2           13| A6   */ F.A,
		B.A,   /*    A2 |3           12| Y6   */ F.Q,
		B.Q,   /*    Y2 |4   74LS14  11| A5   */ E.A,
		C.A,   /*    A3 |5           10| Y5   */ E.Q,
		C.Q,   /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Q
			   /*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7416: Hex Inverting Buffers with
 *           High Voltage Open-Collector Outputs
 *
 */

static NETLIST_START(TTL_7416_DIP)
	TTL_7416_GATE(A)
	TTL_7416_GATE(B)
	TTL_7416_GATE(C)
	TTL_7416_GATE(D)
	TTL_7416_GATE(E)
	TTL_7416_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.Q,  /*    Y1 |2           13| A6   */ F.A,
		B.A,  /*    A2 |3           12| Y6   */ F.Q,
		B.Q,  /*    Y2 |4    7416   11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Q,
		C.Q,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Q
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
	TTL_7420_GATE(A)
	TTL_7420_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| D2   */ B.D,
		NC.I, /*    NC |3           12| C2   */ B.C,
		A.C,  /*    C1 |4    7420   11| NC   */ NC.I,
		A.D,  /*    D1 |5           10| B2   */ B.B,
		A.Q,  /*    Y1 |6            9| A2   */ B.A,
		A.GND,/*   GND |7            8| Y2   */ B.Q
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
	TTL_7425_GATE(A)
	TTL_7425_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(XA) // FIXME: Functionality needs to be implemented
	NC_PIN(XB) // FIXME: Functionality needs to be implemented

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| D2   */ B.D,
		XA.I, /*    X1 |3           12| C2   */ B.C,
		A.C,  /*    C1 |4    7425   11| X2   */ XB.I,
		A.D,  /*    D1 |5           10| B2   */ B.B,
		A.Q,  /*    Y1 |6            9| A2   */ B.A,
		A.GND,/*   GND |7            8| Y2   */ B.Q
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
	TTL_7427_GATE(A)
	TTL_7427_GATE(B)
	TTL_7427_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| C1   */ A.C,
		B.A,  /*    A2 |3           12| Y1   */ A.Q,
		B.B,  /*    B2 |4    7427   11| C3   */ C.C,
		B.C,  /*    C2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7430_GATE(A)

	NC_PIN(NC9)
	NC_PIN(NC10)
	NC_PIN(NC13)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*     A |1     ++    14| VCC  */ A.VCC,
		A.B,  /*     B |2           13| NC   */ NC13.I,
		A.C,  /*     C |3           12| H    */ A.H,
		A.D,  /*     D |4    7430   11| G    */ A.G,
		A.E,  /*     E |5           10| NC   */ NC10.I,
		A.F,  /*     F |6            9| NC   */ NC9.I,
		A.GND,/*   GND |7            8| Y    */ A.Q
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
	TTL_7432_GATE(A)
	TTL_7432_GATE(B)
	TTL_7432_GATE(C)
	TTL_7432_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| B4   */ D.B,
		A.Q,  /*    Y1 |3           12| A4   */ D.A,
		B.A,  /*    A2 |4    7400   11| Y4   */ D.Q,
		B.B,  /*    B2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7437_GATE(A)
	TTL_7437_GATE(B)
	TTL_7437_GATE(C)
	TTL_7437_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| B4   */ D.B,
		A.Q,  /*    Y1 |3           12| A4   */ D.A,
		B.A,  /*    A2 |4    7400   11| Y4   */ D.Q,
		B.B,  /*    B2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
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
	TTL_7486_GATE(A)
	TTL_7486_GATE(B)
	TTL_7486_GATE(C)
	TTL_7486_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.B,  /*    B1 |2           13| B4   */ D.B,
		A.Q,  /*    Y1 |3           12| A4   */ D.A,
		B.A,  /*    A2 |4    7486   11| Y4   */ D.Q,
		B.B,  /*    B2 |5           10| B3   */ C.B,
		B.Q,  /*    Y2 |6            9| A3   */ C.A,
		A.GND,/*   GND |7            8| Y3   */ C.Q
			  /*       +--------------+      */
	)
NETLIST_END()

#if (NL_USE_TRUTHTABLE_74107)
#ifndef __PLIB_PREPROCESSOR__
#define TTL_74107_TT(name)                                                         \
		NET_REGISTER_DEV(TTL_74107, name)
#endif

static NETLIST_START(TTL_74107_DIP)
	TTL_74107_TT(A)
	TTL_74107_TT(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*          +--------------+        */
		A.J,    /*       1J |1     ++    14| VCC    */ A.VCC,
		A.QQ,   /*      1QQ |2           13| 1CLRQ  */ A.CLRQ,
		A.Q,    /*       1Q |3           12| 1CLK   */ A.CLK,
		A.K,    /*       1K |4    74107  11| 2K     */ B.K,
		B.Q,    /*       2Q |5           10| 2CLRQ  */ B.CLRQ,
		B.QQ,   /*      2QQ |6            9| 2CLK   */ B.CLK,
		B.GND,  /*      GND |7            8| 2J     */ B.J
				/*          +--------------+        */
	)

NETLIST_END()
#endif


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
	NET_REGISTER_DEV(TTL_74155A_GATE, A)
	NET_REGISTER_DEV(TTL_74155B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(  /*       +--------------+      */
		A.C,  /*    C1 |1     ++    16| VCC  */ A.VCC,
		A.G,  /*    G1 |2           15| B4   */ B.C,
		A.B,  /*     B |3           14| B4   */ B.G,
		A.3,  /*   1Y3 |4   74155   13| A4   */ B.A,
		B.2,  /*   1Y2 |5           12| Y4   */ B.3,
		B.1,  /*   1Y1 |6           11| B3   */ B.2,
		B.0,  /*   1Y0 |7           10| A3   */ B.1,
		A.GND,/*   GND |8            9| Y3   */ B.0
			  /*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74156_DIP)
	NET_REGISTER_DEV(TTL_74156A_GATE, A)
	NET_REGISTER_DEV(TTL_74156B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(  /*       +--------------+      */
		A.C,  /*    C1 |1     ++    16| VCC  */ A.VCC,
		A.G,  /*    G1 |2           15| B4   */ B.C,
		A.B,  /*     B |3           14| B4   */ B.G,
		A.3,  /*   1Y3 |4   74156   13| A4   */ B.A,
		B.2,  /*   1Y2 |5           12| Y4   */ B.3,
		B.1,  /*   1Y1 |6           11| B3   */ B.2,
		B.0,  /*   1Y0 |7           10| A3   */ B.1,
		A.GND,/*   GND |8            9| Y3   */ B.0
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
	TTL_74260_GATE(A)
	TTL_74260_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(  /*       +--------------+      */
		A.C,  /*    C1 |1     ++    14| VCC  */ A.VCC,
		A.D,  /*    D1 |2           13| B1   */ A.B,
		A.E,  /*    E1 |3           12| A1   */ A.A,
		B.E,  /*    E2 |4   74260   11| D2   */ B.D,
		A.Q,  /*    Y1 |5           10| C2   */ B.C,
		B.Q,  /*    Y2 |6            9| B2   */ B.B,
		A.GND,/*   GND |7            8| A2   */ B.A
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
	TTL_74279B(A)
	TTL_74279A(B)
	TTL_74279B(C)
	TTL_74279A(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*     +--------------+     */
		A.R,   /*  1R |1     ++    16| VCC */ A.VCC,
		A.S1,  /* 1S1 |2           15| 4S  */ D.S,
		A.S2,  /* 1S2 |3           14| 4R  */ D.R,
		A.Q,   /*  1Q |4    74279  13| 4Q  */ D.Q,
		B.R,   /*  2R |5           12| 3S2 */ C.S2,
		B.S,   /*  2S |6           11| 3S1 */ C.S1,
		B.Q,   /*  2Q |7           10| 3R  */ C.R,
		A.GND, /* GND |8            9| 3Q  */ C.Q
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
		NET_REGISTER_DEV(DM9312, name)
#endif

static NETLIST_START(DM9312_DIP)
	DM9312_TT(s)

	DIPPINS(    /*     +--------------+     */
		s.D0,   /*  D0 |1     ++    16| VCC */ s.VCC,
		s.D1,   /*  D1 |2           15| Y   */ s.Y,
		s.D2,   /*  D2 |3           14| YQ  */ s.YQ,
		s.D3,   /*  D3 |4    9312   13| C   */ s.C,
		s.D4,   /*  D4 |5           12| B   */ s.B,
		s.D5,   /*  D5 |6           11| A   */ s.A,
		s.D6,   /*  D6 |7           10| G   */ s.G, //Strobe
		s.GND,  /* GND |8            9| D7  */ s.D7
				/*     +--------------+     */
	)
NETLIST_END()

#if (NL_USE_TRUTHTABLE_7448)

/*
 *  DM7448: BCD to 7-Segment decoders/drivers
 *
 *           +--------------+
 *         B |1     ++    16| VCC
 *         C |2           15| f
 * LAMP TEST |3           14| g
 *    BI/RBQ |4    7448   13| a
 *       RBI |5           12| b
 *         D |6           11| c
 *         A |7           10| d
 *       GND |8            9| e
 *           +--------------+
 *
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */
#ifndef __PLIB_PREPROCESSOR__
#define TTL_7448_TT(name)     \
		NET_REGISTER_DEV(TTL_7448_TT, name)
#endif

static NETLIST_START(TTL_7448_DIP)
	TTL_7448_TT(s)

	DIPPINS(    /*      +--------------+     */
		s.B,    /* B    |1     ++    16| VCC */ s.VCC,
		s.C,    /* C    |2           15| f   */ s.f,
		s.LTQ,  /* LTQ  |3           14| g   */ s.g,
		s.BIQ,  /* BIQ  |4    7448   13| a   */ s.a,
		s.RBIQ, /* RBIQ |5           12| b   */ s.b,
		s.D,    /* D    |6           11| c   */ s.c,
		s.A,    /* A    |7           10| d   */ s.d,
		s.GND,  /* GND  |8            9| e   */ s.e
				/*      +--------------+     */
	)
NETLIST_END()
#endif

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

	TRUTHTABLE_START(TTL_7400_NAND, 2, 1, "+A,+B,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7402_NOR, 2, 1, "+A,+B,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7404_INVERT, 1, 1, "+A,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7408_AND, 2, 1, "+A,+B,@VCC,@GND")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|0|15")
		TT_LINE("X,0|0|15")
		TT_LINE("1,1|1|22")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7410_NAND, 3, 1, "+A,+B,+C,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7411_AND, 3, 1, "+A,+B,+C,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7420_NAND, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7425_NOR, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7427_NOR, 3, 1, "+A,+B,+C,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7430_NAND, 8, 1, "+A,+B,+C,+D,+E,+F,+G,+H,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_7432_OR, 2, 1, "+A,+B,@VCC,@GND")
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

#if (NL_USE_TRUTHTABLE_7448)
	TRUTHTABLE_START(TTL_7448, 7, 7, "+A,+B,+C,+D,+LTQ,+BIQ,+RBIQ,@VCC,@GND")
		TT_HEAD(" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g")

		TT_LINE("  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100")

		// BI/RBO is input output. In the next case it is used as an input will go low.
		TT_LINE("  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // RBI

		TT_LINE("  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100") // LT

		// This condition has precedence
		TT_LINE("  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // BI
		TT_FAMILY("74XX")

	TRUTHTABLE_END()

	// FIXME: We need a more elegant solution than defining twice
	TRUTHTABLE_START(TTL_7448_TT, 7, 7, "")
		TT_HEAD(" LTQ,BIQ,RBIQ, A , B , C , D | a, b, c, d, e, f, g")

		TT_LINE("  1,  1,  1,   0,  0,  0,  0 | 1, 1, 1, 1, 1, 1, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  0 | 0, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  0 | 1, 1, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  0 | 1, 1, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  0 | 0, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  0 | 1, 0, 1, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  0 | 0, 0, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  0 | 1, 1, 1, 0, 0, 0, 0|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  0,  1 | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  0,  1 | 1, 1, 1, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  0,  1 | 0, 0, 0, 1, 1, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  0,  1 | 0, 0, 1, 1, 0, 0, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  0,  1,  1 | 0, 1, 0, 0, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  0,  1,  1 | 1, 0, 0, 1, 0, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   0,  1,  1,  1 | 0, 0, 0, 1, 1, 1, 1|100,100,100,100,100,100,100")
		TT_LINE("  1,  1,  X,   1,  1,  1,  1 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100")

		// BI/RBO is input output. In the next case it is used as an input will go low.
		TT_LINE("  1,  1,  0,   0,  0,  0,  0 | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // RBI

		TT_LINE("  0,  1,  X,   X,  X,  X,  X | 1, 1, 1, 1, 1, 1, 1|100,100,100,100,100,100,100") // LT

		// This condition has precedence
		TT_LINE("  X,  0,  X,   X,  X,  X,  X | 0, 0, 0, 0, 0, 0, 0|100,100,100,100,100,100,100") // BI
		TT_FAMILY("74XX")

	TRUTHTABLE_END()

#endif

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

	TRUTHTABLE_START(TTL_7486_XOR, 2, 1, "+A,+B,@VCC,@GND")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|0|15")
		TT_LINE("0,1|1|22")
		TT_LINE("1,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

#if (NL_USE_TRUTHTABLE_74107)
	/*
	 *          +-----+-----+-----+---++---+-----+
	 *          | CLRQ| CLK |  J  | K || Q | QQ  |
	 *          +=====+=====+=====+===++===+=====+
	 *          |  0  |  X  |  X  | X || 0 |  1  |
	 *          |  1  |  *  |  0  | 0 || Q0| Q0Q |
	 *          |  1  |  *  |  1  | 0 || 1 |  0  |
	 *          |  1  |  *  |  0  | 1 || 0 |  1  |
	 *          |  1  |  *  |  1  | 1 || TOGGLE  |
	 *          +-----+-----+-----+---++---+-----+
	 */
	TRUTHTABLE_START(TTL_74107, 6, 4, "+CLK,+J,+K,+CLRQ,@VCC,@GND")
		TT_HEAD("CLRQ, CLK, _CO,  J, K,_QX | Q, QQ, CO, QX")
		TT_LINE("  0,   0,    X,  X, X,  X | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  0,   1,    X,  X, X,  X | 0,  1,  1,  0 | 16, 25, 1, 1")

		TT_LINE("  1,   0,    X,  0, 0,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  0, 0,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    X,  0, 0,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  0, 0,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  1, 0,  X | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   0,    0,  1, 0,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  1, 0,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  1, 0,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  1, 0,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  0, 1,  X | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  0, 1,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  0, 1,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    X,  0, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    X,  0, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		// Toggle
		TT_LINE("  1,   0,    0,  1, 1,  0 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    0,  1, 1,  1 | 1,  0,  0,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    0,  1, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    0,  1, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")
		TT_LINE("  1,   1,    1,  1, 1,  0 | 0,  1,  1,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   1,    1,  1, 1,  1 | 1,  0,  1,  1 | 25, 16, 1, 1")

		TT_LINE("  1,   0,    1,  1, 1,  1 | 0,  1,  0,  0 | 16, 25, 1, 1")
		TT_LINE("  1,   0,    1,  1, 1,  0 | 1,  0,  0,  1 | 25, 16, 1, 1")
	TRUTHTABLE_END()
#endif

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

	TRUTHTABLE_START(TTL_74260_NOR, 5, 1, "+A,+B,+C,+D,+E,@VCC,@GND")
		TT_HEAD("A,B,C,D,E|Q")
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

	TRUTHTABLE_START(DM9312, 12, 2, "+A,+B,+C,+G,+D0,+D1,+D2,+D3,+D4,+D5,+D6,+D7,@VCC,@GND")
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
#if (NL_USE_TRUTHTABLE_7448)
	LOCAL_LIB_ENTRY(TTL_7448_DIP)
#endif
	LOCAL_LIB_ENTRY(TTL_7486_DIP)
#if (NL_USE_TRUTHTABLE_74107)
	LOCAL_LIB_ENTRY(TTL_74107_DIP)
#endif
	LOCAL_LIB_ENTRY(TTL_74155_DIP)
	LOCAL_LIB_ENTRY(TTL_74156_DIP)
	LOCAL_LIB_ENTRY(TTL_74260_DIP)
	LOCAL_LIB_ENTRY(TTL_74279_DIP)
	LOCAL_LIB_ENTRY(DM9312_DIP)
NETLIST_END()
