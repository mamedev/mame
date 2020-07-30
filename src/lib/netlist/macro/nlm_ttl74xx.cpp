// license:GPL-2.0+
// copyright-holders:Couriersud

#include "netlist/devices/net_lib.h"

#ifndef NL_USE_TRUTHTABLE_74107
#define NL_USE_TRUTHTABLE_74107 0
#endif

#ifndef NL_USE_TRUTHTABLE_7448
#define NL_USE_TRUTHTABLE_7448 0
#endif

#if 1
//
#elif %&/()
//
#endif

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
 *   DM7406: Hex Inverting Buffers with
 *           High Voltage Open-Collector Outputs
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7406_DIP)
	TTL_7406_GATE(A)
	TTL_7406_GATE(B)
	TTL_7406_GATE(C)
	TTL_7406_GATE(D)
	TTL_7406_GATE(E)
	TTL_7406_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.Y,  /*    Y1 |2           13| A6   */ F.A,
		B.A,  /*    A2 |3           12| Y6   */ F.Y,
		B.Y,  /*    Y2 |4    7406   11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Y,
		C.Y,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Y
			  /*       +--------------+      */
	)
NETLIST_END()

/*
 *   DM7407: Hex Buffers with
 *           High Voltage Open-Collector Outputs
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7407_DIP)
	TTL_7407_GATE(A)
	TTL_7407_GATE(B)
	TTL_7407_GATE(C)
	TTL_7407_GATE(D)
	TTL_7407_GATE(E)
	TTL_7407_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    A1 |1     ++    14| VCC  */ A.VCC,
		A.Y,  /*    Y1 |2           13| A6   */ F.A,
		B.A,  /*    A2 |3           12| Y6   */ F.Y,
		B.Y,  /*    Y2 |4    7406   11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Y,
		C.Y,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Y
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
		A.A,   /*    A1 |1     ++    14| VCC  */ A.VCC,
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
		A.A,   /*    A1 |1     ++    14| VCC  */ A.VCC,
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
 *  DM7421: Dual 4-Input AND Gates
 *
 *                  ___
 *              Y = ABCD
 *          +---+---+---+---++---+
 *          | A | B | C | D || Y |
 *          +===+===+===+===++===+
 *          | X | X | X | 0 || 0 |
 *          | X | X | 0 | X || 0 |
 *          | X | 0 | X | X || 0 |
 *          | 0 | X | X | X || 0 |
 *          | 1 | 1 | 1 | 1 || 1 |
 *          +---+---+---+---++---+
 *
 *  Naming conventions follow National Semiconductor datasheet *
 */

static NETLIST_START(TTL_7421_DIP)
	TTL_7421_GATE(A)
	TTL_7421_GATE(B)

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
 *  DM7450: DUAL 2-WIDE 2-INPUT AND-OR-INVERT GATES (ONE GATE EXPANDABLE)
 *
 *          +--------------+
 *       1A |1     ++    14| VCC
 *       2A |2           13| 1B
 *       2B |3           12| 1XQ
 *       2C |4    7450   11| 1X
 *       2D |5           10| 1D
 *       2Y |6            9| 1C
 *      GND |7            8| 1Y
 *          +--------------+
*/

static NETLIST_START(TTL_7450_DIP)
	TTL_7450_ANDORINVERT(A)
	TTL_7450_ANDORINVERT(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(  /*       +--------------+      */
		A.A,  /*    1A |1     ++    14| VCC  */ A.VCC,
		B.A,  /*    2A |2           13| 1B   */ A.B,
		B.B,  /*    2B |3           12| 1XQ  */ NC.I,
		B.C,  /*    2C |4    7450   11| 1X   */ NC.I,
		B.D,  /*    2D |5           10| 1D   */ A.D,
		B.Q,  /*    2Y |6            9| 1C   */ A.C,
		A.GND,/*   GND |7            8| 1Y   */ A.Q
			  /*       +--------------+      */
	)
NETLIST_END()

/*
 *  7473: Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *  7473A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *
 *          +----------+
 *     1CLK |1   ++  14| 1J
 *    1CLRQ |2       13| 1QQ
 *       1K |3       12| 1Q
 *      VCC |4  7473 11| GND
 *     2CLK |5       10| 2K
 *    2CLRQ |6        9| 2Q
 *       2J |7        8| 2QQ
 *          +----------+
 */

static NETLIST_START(TTL_7473_DIP)
	TTL_7473(A)
	TTL_7473(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +----------+     */
		 A.CLK, /*  1CLK |1   ++  14| 1J  */ A.J,
		A.CLRQ, /* 1CLRQ |2       13| 1QQ */ A.QQ,
		   A.K, /*    1K |3       12| 1Q  */ A.Q,
		 A.VCC, /*   VCC |4  7473 11| GND */ A.GND,
		 B.CLK, /*  2CLK |5       10| 2K  */ B.K,
		B.CLRQ, /* 2CLRQ |6        9| 2Q  */ B.Q,
		   B.J, /*    2J |7        8| 2QQ */ B.QQ
			    /*       +----------+     */
	)
NETLIST_END()

static NETLIST_START(TTL_7473A_DIP)
	TTL_7473A(A)
	TTL_7473A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +----------+     */
		 A.CLK, /*  1CLK |1   ++  14| 1J  */ A.J,
		A.CLRQ, /* 1CLRQ |2       13| 1QQ */ A.QQ,
		   A.K, /*    1K |3       12| 1Q  */ A.Q,
		 A.VCC, /*   VCC |4 7473A 11| GND */ A.GND,
		 B.CLK, /*  2CLK |5       10| 2K  */ B.K,
		B.CLRQ, /* 2CLRQ |6        9| 2Q  */ B.Q,
		   B.J, /*    2J |7        8| 2QQ */ B.QQ
			    /*       +----------+     */
	)
NETLIST_END()

/*
 *  DM7474: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Preset, Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLR1 |1     ++    14| VCC
 *       D1 |2           13| CLR2
 *     CLK1 |3           12| D2
 *      PR1 |4    7474   11| CLK2
 *       Q1 |5           10| PR2
 *      Q1Q |6            9| Q2
 *      GND |7            8| Q2Q
 *          +--------------+
 */

static NETLIST_START(TTL_7474_DIP)
	TTL_7474(A)
	TTL_7474(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+       */
		A.CLRQ, /*  CLR1 |1     ++    14| VCC   */ A.VCC,
		   A.D, /*    D1 |2           13| CLR2  */ B.CLRQ,
		 A.CLK, /*  CLK1 |3           12| D2    */ B.D,
		A.PREQ, /*   PR1 |4    7474   11| CLK2  */ B.CLK,
		   A.Q, /*    Q1 |5           10| PR2   */ B.PREQ,
		  A.QQ, /*   Q1Q |6            9| Q2    */ B.Q,
		 A.GND, /*   GND |7            8| Q2Q   */ B.QQ
			    /*       +-------------+        */
	)
NETLIST_END()

/*
 *  7475: 4-Bit Bistable Latches with Complementary Outputs
 *  7477: 4-Bit Bistable Latches
 *
 *          +----------+               +----------+
 *      1QQ |1   ++  16| 1Q         1D |1   ++  14| 1Q
 *       1D |2       15| 2Q         2D |2       13| 2Q
 *       2D |3       14| 2QQ      3C4C |3       12| 1C2C
 *     3C4C |4  7475 13| 1C2C      VCC |4  7477 11| GND
 *      VCC |5       12| GND        3D |5       10| NC
 *       3D |6       11| 3QQ        4D |6        9| 3Q
 *       4D |7       10| 3Q         NC |7        8| 4Q
 *      4QQ |8        9| 4Q            +----------+
 *          +----------+
 */

static NETLIST_START(TTL_7475_DIP)
	TTL_7475_GATE(A)
	TTL_7475_GATE(B)
	TTL_7475_GATE(C)
	TTL_7475_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	NET_C(A.CLK, B.CLK)
	NET_C(C.CLK, D.CLK)

	DIPPINS(   /*       +--------------+   */
		 A.QQ, /*   1QQ |1   ++  16| 1Q    */ A.Q,
		  A.D, /*    1D |2       15| 2Q    */ B.Q,
		  B.D, /*    2D |3       14| 2QQ   */ B.QQ,
		C.CLK, /*  3C4C |4  7475 13| 1C2C  */ A.CLK,
		A.VCC, /*   VCC |5       12| GND   */ A.GND,
		  C.D, /*    3D |6       11| 3QQ   */ C.QQ,
		  D.D, /*    4D |7       10| 3Q    */ C.Q,
		 D.QQ, /*   4QQ |8        9| 4Q    */ D.Q
			   /*       +-------------+    */
	)
NETLIST_END()

static NETLIST_START(TTL_7477_DIP)
	TTL_7477_GATE(A)
	TTL_7477_GATE(B)
	TTL_7477_GATE(C)
	TTL_7477_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	NET_C(A.CLK, B.CLK)
	NET_C(C.CLK, D.CLK)

	NC_PIN(NC)

	DIPPINS(   /*       +----------+       */
		  A.D, /*    1D |1   ++  14| 1Q    */ A.Q,
		  B.D, /*    2D |2       13| 2Q    */ B.Q,
		C.CLK, /*  3C4C |3       12| 1C2C  */ A.CLK,
		A.VCC, /*   VCC |4  7477 11| GND   */ A.GND,
		  C.D, /*    3D |5       10| NC    */ NC.I,
		  D.D, /*    4D |6        9| 3Q    */ C.Q,
		 NC.I, /*    NC |7        8| 4Q    */ D.Q
			   /*       +----------+       */
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

//- Identifier:  TTL_74121_DIP
//- Title: DM74121 One-Shot with Clear and Complementary Outputs
//- Description: The DM74121 is a monostable multivibrator featuring both
//-   positive and negative edge triggering with complementary
//-   outputs. An internal 2kΩ timing resistor is provided for
//-   design convenience minimizing component count and layout problems. this device can be used with a single external capacitor. Inputs (A) are active-LOW trigger transition
//-   inputs and input (B) is and active-HIGH transition Schmitttrigger input that allows jitter-free triggering from inputs with
//-   transition rates as slow as 1 volt/second. A high immunity
//-   to VCC noise of typically 1.5V is also provided by internal
//-   circuitry at the input stage.
//-   To obtain optimum and trouble free operation please read
//-   operating rules and one-shot application notes carefully
//-   and observe recommendations.
//-
//- Pinalias: QQ,NC,A1,A2,B,Q,GND,NC,RINT,C,RC,NC,NC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- Limitations:
//-    Timing inaccuracies may occur for capacitances < 1nF. Please consult datasheet
//-
//- Example: 74123.cpp,74123_example
//-
//- FunctionTable:
//-    https://pdf1.alldatasheet.com/datasheet-pdf/view/50894/FAIRCHILD/74121.html
//-
static NETLIST_START(TTL_74121_DIP)

	TTL_74121(A)
	RES(RINT, RES_K(2))
	RES(RD,   RES_M(1000))

	ALIAS(1, A.QQ)
	//ALIAS(2", ); NC
	ALIAS(3, A.A1)
	ALIAS(4, A.A2)
	ALIAS(5, A.B)
	ALIAS(6, A.Q)
	ALIAS(7, A.GND)

	//ALIAS(8", ); NC
	ALIAS(9,  RINT.1) // RINT
	ALIAS(10, A.C) // CEXT
	ALIAS(11, A.RC) // REXT
	//ALIAS(12", ); NC
	//ALIAS(13", ); NC
	ALIAS(14, A.VCC)

	NET_C(RINT.2, A.RC)

	// Avoid error messages if RINT is not used.
	NET_C(RINT.1, RD.2)
	NET_C(RD.1, A.GND)

NETLIST_END()

//- Identifier:  TTL_74123_DIP
//- Title: DM74123 Dual Retriggerable One-Shot with Clear and Complementary Outputs
//- Description: The DM74123 is a dual retriggerable monostable multivibrator
//-   capable of generating output pulses from a few
//-   nano-seconds to extremely long duration up to 100% duty
//-   cycle. Each device has three inputs permitting the choice of
//-   either leading-edge or trailing edge triggering. Pin (A) is an
//-   active-LOW transition trigger input and pin (B) is an activeHIGH transition trigger input. A LOW at the clear (CLR)
//-   input terminates the output pulse: which also inhibits triggering. An internal connection from CLR to the input gate
//-   makes it possible to trigger the circuit by a positive-going
//-   signal on CLR as shown in the Truth Table.
//-
//-   To obtain the best and trouble free operation from this
//-   device please read the Operating Rules as well as the
//-   One–Shot Application Notes carefully and observe recommendations.
//-
//- Pinalias: A1,B1,CLRQ1,QQ1,Q2,C2,RC2,GND,A2,B2,CLRQ2,QQ2,Q1,C1,RC1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- Limitations:
//-    Timing inaccuracies may occur for capacitances < 1nF. Please consult datasheet
//-
//- Example: 74123.cpp,74123_example
//-
//- FunctionTable:
//-    https://pdf1.alldatasheet.com/datasheet-pdf/view/50893/FAIRCHILD/DM74123.html
//-
static NETLIST_START(TTL_74123_DIP)

	TTL_74123(A)
	TTL_74123(B)

	ALIAS(1, A.A)
	ALIAS(2, A.B)
	ALIAS(3, A.CLRQ)
	ALIAS(4, A.QQ)
	ALIAS(5, B.Q)
	ALIAS(6, B.C) // CEXT
	ALIAS(7, B.RC) // REXT
	ALIAS(8, A.GND)

	ALIAS(9, B.A)
	ALIAS(10, B.B)
	ALIAS(11, B.CLRQ)
	ALIAS(12, B.QQ)
	ALIAS(13, A.Q)
	ALIAS(14, A.C) // CEXT
	ALIAS(15, A.RC) // REXT
	ALIAS(16, A.VCC)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
NETLIST_END()

//- Identifier:  TTL_9602_DIP
//- Title: DM9602 Dual Retriggerable, Resettable One Shots
//- Description: These dual resettable, retriggerable one shots have two
//-   inputs per function; one which is active HIGH, and one
//-   which is active LOW. This allows the designer to employ
//-   either leading-edge or trailing-edge triggering, which is
//-   independent of input transition times. When input conditions for triggering are met, a new cycle starts and the
//-   external capacitor is allowed to rapidly discharge and then
//-   charge again. The retriggerable feature permits output
//-   pulse widths to be extended. In fact a continuous true output can be maintained by having an input cycle time which
//-   is shorter than the output cycle time. The output pulse may
//-   then be terminated at any time by applying a LOW logic
//-   level to the RESET pin. Retriggering may be inhibited by
//-   either connecting the Q output to an active HIGH input, or
//-   the Q output to an active LOW input.
//-   The DM74123 is a dual retriggerable monostable multivibrator
//-
//- Pinalias: C1,RC1,CLRQ1,B1,A1,Q1,QQ1,GND,QQ2,Q2,A2,B2,CLRQ2,RC2,C2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- Limitations:
//-    Timing inaccuracies may occur for capacitances < 1nF. Please consult datasheet
//-
//- Example: 74123.cpp,74123_example
//-
//- FunctionTable:
//-    https://pdf1.alldatasheet.com/datasheet-pdf/view/51137/FAIRCHILD/DM9602.html
//-
static NETLIST_START(TTL_9602_DIP)

	TTL_9602(A)
	TTL_9602(B)

	ALIAS(1, A.C) // C1
	ALIAS(2, A.RC) // RC1
	ALIAS(3, A.CLRQ)
	ALIAS(4, A.B)
	ALIAS(5, A.A)
	ALIAS(6, A.Q)
	ALIAS(7, A.QQ)
	ALIAS(8, A.GND)

	ALIAS(9, B.QQ)
	ALIAS(10, B.Q)
	ALIAS(11, B.A)
	ALIAS(12, B.B)
	ALIAS(13, B.CLRQ)
	ALIAS(14, B.RC) // RC2
	ALIAS(15, B.C) // C2
	ALIAS(16, A.VCC)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
NETLIST_END()

//- Identifier:  TTL_74125_DIP
//- Title: SN74125 QUADRUPLE BUS BUFFERS WITH 3-STATE OUTPUTS
//- Description: These bus buffers feature three-state outputs
//-    that, when enabled, have the low impedance characteristics of a
//-    TTL output with additional drive capability at high logic levels
//-    to permit driving heavily loaded bus lines without external
//-    pullup resistors. When disabled, both output transistors are turned
//-    off, presenting a high-impedance state to the bus so the output will
//-    act neither as a significant load nor as a driver. The ’125 and
//-    ’LS125A devices’ outputs are disabled when G is high.
//-    The ’126 and ’LS126A devices’ outputs are disabled when G is low
//-
//- Pinalias: 1GQ,1A,1Y,2GQ,2A,2Y,GND,3Y,3A,3GQ,4Y,4A,4GQ,VCC
//- Package: DIP
//- Param: FORCE_TRISTATE_LOGIC
//-    Set this parameter to 1 force tristate outputs into logic mode.
//-    This should be done only if the device enable inputs are connected
//-    in a way which always enables the device.
//- NamingConvention: Naming conventions follow Texas instruments datasheet
//- Limitations:
//-    No limitations
//-
//- Example: 74125.cpp,74125_example
//-
//- FunctionTable:
//-
//-    | GQ  | A  | Y  |
//-    |:---:|:--:|:--:|
//-    |  L  |  L |  L |
//-    |  L  |  H |  H |
//-    |  H  |  X |  Z |
//-

static NETLIST_START(TTL_74125_DIP)

	TTL_74125_GATE(A1)
	TTL_74125_GATE(A2)
	TTL_74125_GATE(A3)
	TTL_74125_GATE(A4)

	DEFPARAM(FORCE_TRISTATE_LOGIC, "$(@.A1.FORCE_TRISTATE_LOGIC")

	PARAM(A1.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A2.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A3.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A4.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")

	ALIAS(1, A1.GQ)
	ALIAS(2, A1.A)
	ALIAS(3, A1.Y)
	ALIAS(4, A2.GQ)
	ALIAS(5, A2.A)
	ALIAS(6, A2.Y)
	ALIAS(7, A1.GND)

	ALIAS(8, A3.Y)
	ALIAS(9, A3.A)
	ALIAS(10, A3.GQ)
	ALIAS(11, A4.Y)
	ALIAS(12, A4.A)
	ALIAS(13, A4.GQ)
	ALIAS(14, A1.VCC)

	NET_C(A1.VCC, A2.VCC, A3.VCC, A4.VCC)
	NET_C(A1.GND, A2.GND, A3.GND, A4.GND)
NETLIST_END()

static NETLIST_START(TTL_74126_DIP)

	TTL_74126_GATE(A1)
	TTL_74126_GATE(A2)
	TTL_74126_GATE(A3)
	TTL_74126_GATE(A4)

	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	PARAM(A1.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A2.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A3.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(A4.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")

	ALIAS(1, A1.G)
	ALIAS(2, A1.A)
	ALIAS(3, A1.Y)
	ALIAS(4, A2.G)
	ALIAS(5, A2.A)
	ALIAS(6, A2.Y)
	ALIAS(7, A1.GND)

	ALIAS(8, A3.Y)
	ALIAS(9, A3.A)
	ALIAS(10, A3.G)
	ALIAS(11, A4.Y)
	ALIAS(12, A4.A)
	ALIAS(13, A4.G)
	ALIAS(14, A1.VCC)

	NET_C(A1.VCC, A2.VCC, A3.VCC, A4.VCC)
	NET_C(A1.GND, A2.GND, A3.GND, A4.GND)
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
 * DM74157: Quad 2-Input Multiplexor
 *
 *      +---+---+-------+---+
 *      | E | S | I0 I1 | Z |
 *      +===+===+=======+===+
 *      | 1 | X |  X  X | 0 |
 *      | 0 | 1 |  X  0 | 0 |
 *      | 0 | 1 |  X  1 | 1 |
 *      | 0 | 0 |  0  X | 0 |
 *      | 0 | 0 |  1  X | 1 |
 *      +---+---+-------+---+
 *
 * Naming conventions follow TI datasheet
 *
 */

static NETLIST_START(TTL_74157_DIP)
	NET_REGISTER_DEV(TTL_74157_GATE, A)
	NET_REGISTER_DEV(TTL_74157_GATE, B)
	NET_REGISTER_DEV(TTL_74157_GATE, C)
	NET_REGISTER_DEV(TTL_74157_GATE, D)

	NET_C(A.E, B.E, C.E, D.E)
	NET_C(A.S, B.S, C.S, D.S)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(  /*       +--------------+      */
		A.S,  /*     S |1     ++    16| VCC  */ A.VCC,
		A.I,  /*   I0a |2           15| /E   */ A.E,
		A.J,  /*   I1a |3           14| I0c  */ C.I,
		A.O,  /*    Za |4   74157   13| I1c  */ C.J,
		B.I,  /*   I0b |5           12| Zc   */ C.O,
		B.J,  /*   I1b |6           11| I0d  */ D.I,
		B.O,  /*    Zb |7           10| I1d  */ D.J,
		A.GND,/*   GND |8            9| Zd   */ D.O
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
#if !NL_AUTO_DEVICES
#define TTL_74279A(name)                                                         \
		NET_REGISTER_DEV(TTL_74279A, name)
#define TTL_74279B(name)                                                         \
		NET_REGISTER_DEV(TTL_74279B, name)
#endif
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
 *  DM74377: Octal D Flip-Flop With Enable
 *  DM74378: Hex D Flip-Flop With Enable
 *  DM74379: 4-bit D Flip-Flop With Enable
 *
 */

static NETLIST_START(TTL_74377_DIP)
	TTL_74377_GATE(A)
	TTL_74377_GATE(B)
	TTL_74377_GATE(C)
	TTL_74377_GATE(D)
	TTL_74377_GATE(E)
	TTL_74377_GATE(F)
	TTL_74377_GATE(G)
	TTL_74377_GATE(H)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC, G.VCC, H.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND, G.GND, H.GND)
	NET_C(A.CP, B.CP, C.CP, D.CP, E.CP, F.CP, G.CP, H.CP)
	NET_C(A.E, B.E, C.E, D.E, E.E, F.E, G.E, H.E)

	HINT(A.QQ, NC)
	HINT(B.QQ, NC)
	HINT(C.QQ, NC)
	HINT(D.QQ, NC)
	HINT(E.QQ, NC)
	HINT(F.QQ, NC)
	HINT(G.QQ, NC)
	HINT(H.QQ, NC)

	DIPPINS(  /*       +--------------+      */
		A.E,  /*    /E |1     ++    20| VCC  */ A.VCC,
		A.Q,  /*    Q0 |2           19| Q7   */ H.Q,
		A.D,  /*    D0 |3           18| D7   */ H.D,
		B.D,  /*    D1 |4   74377   17| D6   */ G.D,
		B.Q,  /*    Q1 |5           16| Q6   */ G.Q,
		C.Q,  /*    Q2 |6           15| Q5   */ F.Q,
		C.D,  /*    D2 |7           14| D5   */ F.D,
		D.D,  /*    D3 |8           13| D4   */ E.D,
		D.Q,  /*    Q3 |9           12| Q4   */ E.Q,
		A.GND,/*   GND |10          11| CP   */ A.CP
			  /*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74378_DIP)
	TTL_74377_GATE(A)
	TTL_74377_GATE(B)
	TTL_74377_GATE(C)
	TTL_74377_GATE(D)
	TTL_74377_GATE(E)
	TTL_74377_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	NET_C(A.CP, B.CP, C.CP, D.CP, E.CP, F.CP)
	NET_C(A.E, B.E, C.E, D.E, E.E, F.E)

	DIPPINS(  /*       +--------------+      */
		A.E,  /*    /E |1     ++    16| VCC  */ A.VCC,
		A.Q,  /*    Q0 |2           15| Q5   */ F.Q,
		A.D,  /*    D0 |3           14| D5   */ F.D,
		B.D,  /*    D1 |4   74378   13| D4   */ E.D,
		B.Q,  /*    Q1 |5           12| Q4   */ E.Q,
		C.D,  /*    D2 |6           11| D3   */ D.D,
		C.Q,  /*    Q2 |7           10| Q3   */ D.Q,
		A.GND,/*   GND |8            9| CP   */ A.CP
			  /*       +--------------+      */
	)
NETLIST_END()

static NETLIST_START(TTL_74379_DIP)
	TTL_74377_GATE(A)
	TTL_74377_GATE(B)
	TTL_74377_GATE(C)
	TTL_74377_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)
	NET_C(A.CP, B.CP, C.CP, D.CP)
	NET_C(A.E, B.E, C.E, D.E)

	DIPPINS(  /*       +--------------+      */
		A.E,  /*    /E |1     ++    16| VCC  */ A.VCC,
		A.Q,  /*    Q0 |2           15| Q3   */ D.Q,
		A.QQ, /*   /Q0 |3           14| /Q3  */ D.QQ,
		A.D,  /*    D0 |4   74379   13| D3   */ D.D,
		B.D,  /*    D1 |5           12| D2   */ C.D,
		B.QQ, /*   /Q1 |6           11| /Q2  */ C.QQ,
		B.Q,  /*    Q1 |7           10| Q2   */ C.Q,
		A.GND,/*   GND |8            9| CP   */ A.CP
			  /*       +--------------+      */
	)
NETLIST_END()

/*
 *  SN74LS629: VOLTAGE-CONTROLLED OSCILLATORS
 *
 *          +--------------+
 *      2FC |1     ++    16| VCC
 *      1FC |2           15| QSC VCC
 *     1RNG |3           14| 2RNG
 *     1CX1 |4  74LS629  13| 2CX1
 *     1CX2 |5           12| 2CX2
 *     1ENQ |6           11| 2ENQ
 *       1Y |7           10| 2Y
 *  OSC GND |8            9| GND
 *          +--------------+
 */

static NETLIST_START(SN74LS629_DIP)
	SN74LS629(A, CAP_U(1))
	SN74LS629(B, CAP_U(1))

	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
	NC_PIN(NC)

	DIPPINS(   /*          +--------------+         */
		 B.FC, /*      2FC |1     ++    16| VCC     */ NC.I,
		 A.FC, /*      1FC |2           15| OSC VCC */ A.VCC,
		A.RNG, /*     1RNG |3           14| 2RNG    */ B.RNG,
		 NC.I, /*     1CX1 |4  74LS629  13| 2CX1    */ NC.I,
		 NC.I, /*     1CX2 |5           12| 2CX2    */ NC.I,
		A.ENQ, /*     1ENQ |6           11| 2ENQ    */ B.ENQ,
		  B.Y, /*       1Y |7           10| 2Y      */ B.Y,
		A.GND, /*  OSC GND |8            9| GND     */ NC.I
			   /*          +--------------+         */
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

static NETLIST_START(DM9312_DIP)
	DM9312(s)

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

/*  SN7442: 4-Line BCD to 10-Line Decimal Decoder
 *
 *          +--------------+
 *        0 |1     ++    16| VCC
 *        1 |2           15| A
 *        2 |3           14| B
 *        3 |4           13| C
 *        4 |5    7442   12| D
 *        5 |6           11| 9
 *        6 |7           10| 8
 *      GND |8            9| 7
 *          +--------------+
 */

static NETLIST_START(TTL_7442_DIP)
	NET_REGISTER_DEV(TTL_7442, A)

	DIPPINS(    /*      +--------------+     */
		  A.Q0, /*    0 |1     ++    16| VCC */ A.VCC,
		  A.Q1, /*    1 |2           15| A   */ A.A,
		  A.Q2, /*    2 |3           14| B   */ A.B,
		  A.Q3, /*    3 |4           13| C   */ A.C,
		  A.Q4, /*    4 |5    7442   12| D   */ A.D,
		  A.Q5, /*    5 |6           11| 9   */ A.Q9,
		  A.Q6, /*    6 |7           10| 8   */ A.Q8,
		 A.GND, /*  GND |8            9| 7   */ A.Q7
				/*      +--------------+     */
	)
NETLIST_END()


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
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

static NETLIST_START(TTL_7448_DIP)

#if (NL_USE_TRUTHTABLE_7448)
	NET_REGISTER_DEV(TTL_7448_TT, A)
#else
	NET_REGISTER_DEV(TTL_7448, A)
#endif

	DIPPINS(    /*      +--------------+     */
		A.B,    /* B    |1     ++    16| VCC */ A.VCC,
		A.C,    /* C    |2           15| f   */ A.f,
		A.LTQ,  /* LTQ  |3           14| g   */ A.g,
		A.BIQ,  /* BIQ  |4    7448   13| a   */ A.a,
		A.RBIQ, /* RBIQ |5           12| b   */ A.b,
		A.D,    /* D    |6           11| c   */ A.c,
		A.A,    /* A    |7           10| d   */ A.d,
		A.GND,  /* GND  |8            9| e   */ A.e
				/*      +--------------+     */
	)
NETLIST_END()

NETLIST_START(TTL74XX_lib)
	NET_MODEL("DM7414         SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL("TTL_7414_GATE  SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL("DM74LS14       SCHMITT_TRIGGER(VTP=1.6 VTM=0.8 VI=4.4 RI=19.3k VOH=3.45 ROH=130 VOL=0.1 ROL=31.2 TPLH=15 TPHL=15)")
	//NET_MODEL("DM7414 FAMILY(IVL=0.16 IVH=0.4 OVL=0.1 OVH=0.05 ORL=10.0 ORH=1.0e8)")


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

	TRUTHTABLE_START(TTL_7406_GATE, 1, 1, "")
		TT_HEAD("A|Y ")
		TT_LINE("0|1|15")
		TT_LINE("1|0|23")
		/* Open Collector */
		TT_FAMILY("74XXOC")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7407_GATE, 1, 1, "")
		TT_HEAD("A|Y ")
		TT_LINE("0|0|15")
		TT_LINE("1|1|23")
		/* Open Collector */
		TT_FAMILY("74XXOC")
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

	TRUTHTABLE_START(TTL_7421_GATE, 4, 1, "")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("0,X,X,X|0|22")
		TT_LINE("X,0,X,X|0|22")
		TT_LINE("X,X,0,X|0|22")
		TT_LINE("X,X,X,0|0|22")
		TT_LINE("1,1,1,1|1|15")
		TT_FAMILY("74XX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(TTL_7421_AND, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
		TT_HEAD("A,B,C,D|Q ")
		TT_LINE("0,X,X,X|0|22")
		TT_LINE("X,0,X,X|0|22")
		TT_LINE("X,X,0,X|0|22")
		TT_LINE("X,X,X,0|0|22")
		TT_LINE("1,1,1,1|1|15")
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

	TRUTHTABLE_START(TTL_7442, 4, 10, "")
		TT_HEAD("D,C,B,A|0,1,2,3,4,5,6,7,8,9")
		TT_LINE("0,0,0,0|0,1,1,1,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,0,0,1|1,0,1,1,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,0,1,0|1,1,0,1,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,0,1,1|1,1,1,0,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,1,0,0|1,1,1,1,0,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,1,0,1|1,1,1,1,1,0,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,1,1,0|1,1,1,1,1,1,0,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("0,1,1,1|1,1,1,1,1,1,1,0,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("1,0,0,0|1,1,1,1,1,1,1,1,0,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("1,0,0,1|1,1,1,1,1,1,1,1,1,0|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("1,0,1,X|1,1,1,1,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
		TT_LINE("1,1,X,X|1,1,1,1,1,1,1,1,1,1|30,30,30,30,30,30,30,30,30,30")
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

	TRUTHTABLE_START(TTL_74157_GATE, 4, 4, "")
		TT_HEAD("E,S,I,J|O")
		TT_LINE("1,X,X,X|0|14")
		TT_LINE("0,1,X,0|0|14")
		TT_LINE("0,1,X,1|1|14")
		TT_LINE("0,0,0,X|0|14")
		TT_LINE("0,0,1,X|1|14")
		TT_FAMILY("74XX")
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
	LOCAL_LIB_ENTRY(TTL_7406_DIP)
	LOCAL_LIB_ENTRY(TTL_7407_DIP)
	LOCAL_LIB_ENTRY(TTL_7408_DIP)
	LOCAL_LIB_ENTRY(TTL_7410_DIP)
	LOCAL_LIB_ENTRY(TTL_7411_DIP)
	LOCAL_LIB_ENTRY(TTL_7414_GATE)
	LOCAL_LIB_ENTRY(TTL_74LS14_GATE)
	LOCAL_LIB_ENTRY(TTL_7414_DIP)
	LOCAL_LIB_ENTRY(TTL_74LS14_DIP)
	LOCAL_LIB_ENTRY(TTL_7416_DIP)
	LOCAL_LIB_ENTRY(TTL_7420_DIP)
	LOCAL_LIB_ENTRY(TTL_7421_DIP)
	LOCAL_LIB_ENTRY(TTL_7425_DIP)
	LOCAL_LIB_ENTRY(TTL_7427_DIP)
	LOCAL_LIB_ENTRY(TTL_7430_DIP)
	LOCAL_LIB_ENTRY(TTL_7432_DIP)
	LOCAL_LIB_ENTRY(TTL_7437_DIP)
	LOCAL_LIB_ENTRY(TTL_7442_DIP)
	LOCAL_LIB_ENTRY(TTL_7448_DIP)
	LOCAL_LIB_ENTRY(TTL_7450_DIP)
	LOCAL_LIB_ENTRY(TTL_7473_DIP)
	LOCAL_LIB_ENTRY(TTL_7473A_DIP)
	LOCAL_LIB_ENTRY(TTL_7474_DIP)
	LOCAL_LIB_ENTRY(TTL_7475_DIP)
	LOCAL_LIB_ENTRY(TTL_7477_DIP)
	LOCAL_LIB_ENTRY(TTL_7486_DIP)
	LOCAL_LIB_ENTRY(TTL_74121_DIP)
	LOCAL_LIB_ENTRY(TTL_74123_DIP)
	LOCAL_LIB_ENTRY(TTL_9602_DIP)
	LOCAL_LIB_ENTRY(TTL_74125_DIP)
	LOCAL_LIB_ENTRY(TTL_74126_DIP)
#if (NL_USE_TRUTHTABLE_74107)
	LOCAL_LIB_ENTRY(TTL_74107_DIP)
#endif
	LOCAL_LIB_ENTRY(TTL_74155_DIP)
	LOCAL_LIB_ENTRY(TTL_74156_DIP)
	LOCAL_LIB_ENTRY(TTL_74157_DIP)
	LOCAL_LIB_ENTRY(TTL_74260_DIP)
	LOCAL_LIB_ENTRY(TTL_74279_DIP)
	LOCAL_LIB_ENTRY(TTL_74377_DIP)
	LOCAL_LIB_ENTRY(TTL_74378_DIP)
	LOCAL_LIB_ENTRY(TTL_74379_DIP)
	LOCAL_LIB_ENTRY(SN74LS629_DIP)
	LOCAL_LIB_ENTRY(DM9312_DIP)
NETLIST_END()
