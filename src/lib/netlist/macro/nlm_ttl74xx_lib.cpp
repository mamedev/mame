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

//- Identifier: TTL_7400_DIP
//- Title: 5400/DM5400/DM7400 Quad 2-Input NAND Gates
//- Description: This device contains four independent gates each of which performs the logic NAND function.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006613.PDF
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 1 |
//-         | 0 | 1 || 1 |
//-         | 1 | 0 || 1 |
//-         | 1 | 1 || 0 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7400_DIP)
	TTL_7400_GATE(A)
	TTL_7400_GATE(B)
	TTL_7400_GATE(C)
	TTL_7400_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| B4   */ D.B,
		  A.Q, /*    Y1 |3           12| A4   */ D.A,
		  B.A, /*    A2 |4    7400   11| Y4   */ D.Q,
		  B.B, /*    B2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7402_DIP
//- Title: 5402/DM5402/DM7402 Quad 2-Input NOR Gates
//- Description: This device contains four independent gates each of which performs the logic NOR function.
//- Pinalias: Y1,A1,B1,Y2,A2,B2,GND,A3,B3,Y3,A4,B4,Y4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006492.PDF
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 1 |
//-         | 0 | 1 || 0 |
//-         | 1 | 0 || 0 |
//-         | 1 | 1 || 0 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7402_DIP)
	TTL_7402_GATE(A)
	TTL_7402_GATE(B)
	TTL_7402_GATE(C)
	TTL_7402_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.Q, /*    Y1 |1     ++    14| VCC  */ A.VCC,
		  A.A, /*    A1 |2           13| Y4   */ D.Q,
		  A.B, /*    B1 |3           12| B4   */ D.B,
		  B.Q, /*    Y2 |4    7402   11| A4   */ D.A,
		  B.A, /*    A2 |5           10| Y3   */ C.Q,
		  B.B, /*    B2 |6            9| B3   */ C.B,
		A.GND, /*   GND |7            8| A3   */ C.A
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7404_DIP
//- Title: 5404/DM5404/DM7404 Hex Inverting Gates
//- Description: This device contains six independent gates each of which performs the logic INVERT function.
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006494.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 1 |
//-         | 1 || 0 |
//-         +---++---+
//-
static NETLIST_START(TTL_7404_DIP)
	TTL_7404_GATE(A)
	TTL_7404_GATE(B)
	TTL_7404_GATE(C)
	TTL_7404_GATE(D)
	TTL_7404_GATE(E)
	TTL_7404_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.Q, /*    Y1 |2           13| A6   */ F.A,
		  B.A, /*    A2 |3           12| Y6   */ F.Q,
		  B.Q, /*    Y2 |4    7404   11| A5   */ E.A,
		  C.A, /*    A3 |5           10| Y5   */ E.Q,
		  C.Q, /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7406_DIP
//- Title: DM5406/DM7406 Hex Inverting Buffers with High Voltage Open-Collector Outputs
//- Description: This device contains six independent buffers each of which performs the logic INVERT function.
//-   The open-collector outputs require external pull-up resistors for proper logical operation.
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: Open collector behavior currently not simulated.
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006496.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 1 |
//-         | 1 || 0 |
//-         +---++---+
//-
static NETLIST_START(TTL_7406_DIP)
	TTL_7406_GATE(A)
	TTL_7406_GATE(B)
	TTL_7406_GATE(C)
	TTL_7406_GATE(D)
	TTL_7406_GATE(E)
	TTL_7406_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.Y, /*    Y1 |2           13| A6   */ F.A,
		  B.A, /*    A2 |3           12| Y6   */ F.Y,
		  B.Y, /*    Y2 |4    7406   11| A5   */ E.A,
		  C.A, /*    A3 |5           10| Y5   */ E.Y,
		  C.Y, /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Y
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7407_DIP
//- Title: DM5407/DM7407 Hex Buffers with High Voltage Open-Collector Outputs
//- Description: This device contains six independent gates each of which performs a buffer function.
//-   The open-collector outputs re-quire external pull-up resistors for proper logical operation.
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: Open collector behavior currently not simulated.
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006497.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 0 |
//-         | 1 || 1 |
//-         +---++---+
//-
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
		B.Y,  /*    Y2 |4    7407   11| A5   */ E.A,
		C.A,  /*    A3 |5           10| Y5   */ E.Y,
		C.Y,  /*    Y3 |6            9| A4   */ D.A,
		A.GND,/*   GND |7            8| Y4   */ D.Y
			  /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7408_DIP
//- Title: 5408/DM5408/DM7408 Quad 2-Input AND Gates
//- Description: This device contains four independent gates each of which performs the logic AND function.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006498.PDF
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 0 |
//-         | 0 | 1 || 0 |
//-         | 1 | 0 || 0 |
//-         | 1 | 1 || 1 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7408_DIP)
	TTL_7408_GATE(A)
	TTL_7408_GATE(B)
	TTL_7408_GATE(C)
	TTL_7408_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| B4   */ D.B,
		  A.Q, /*    Y1 |3           12| A4   */ D.A,
		  B.A, /*    A2 |4    7408   11| Y4   */ D.Q,
		  B.B, /*    B2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7410_DIP
//- Title: 5410/DM5410/DM7410 Triple 3-Input NAND Gates
//- Description: This device contains three independent gates each of which performs the logic NAND function.
//- Pinalias: A1,B1,A2,B2,C2,Y2,GND,Y3,A3,B3,C3,Y1,C1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006500.PDF
//-
//-         +---+---+---++---+
//-         | A | B | C || Y |
//-         +===+===+===++===+
//-         | X | X | 0 || 1 |
//-         | X | 0 | X || 1 |
//-         | 0 | X | X || 1 |
//-         | 1 | 1 | 1 || 0 |
//-         +---+---+---++---+
//-
static NETLIST_START(TTL_7410_DIP)
	TTL_7410_GATE(A)
	TTL_7410_GATE(B)
	TTL_7410_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| C1   */ A.C,
		  B.A, /*    A2 |3           12| Y1   */ A.Q,
		  B.B, /*    B2 |4    7410   11| C3   */ C.C,
		  B.C, /*    C2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7411_DIP
//- Title: DM7411 Triple 3-Input AND Gate
//- Description: This device contains three independent gates with three data inputs each which perform the logic AND function.
//- Pinalias: A1,B1,A2,B2,C2,Y2,GND,Y3,A3,B3,C3,Y1,C1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009774.PDF
//-
//-         +---+---+---++---+
//-         | A | B | C || Y |
//-         +===+===+===++===+
//-         | X | X | 0 || 0 |
//-         | X | 0 | X || 0 |
//-         | 0 | X | X || 0 |
//-         | 1 | 1 | 1 || 1 |
//-         +---+---+---++---+
//-
static NETLIST_START(TTL_7411_DIP)
	TTL_7411_GATE(A)
	TTL_7411_GATE(B)
	TTL_7411_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| C1   */ A.C,
		  B.A, /*    A2 |3           12| Y1   */ A.Q,
		  B.B, /*    B2 |4    7411   11| C3   */ C.C,
		  B.C, /*    C2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7414_DIP
//- Title: DM5414/DM7414 Hex Inverter withSchmitt Trigger Inputs
//- Description: This device contains six independent gates each of whichperforms the logic INVERT function.
//-   Each input has hysteresis which increases the noise immunity and transforms a slowly changing input
//-   signal to a fast changing, jitter free output.
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006503.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 1 |
//-         | 1 || 0 |
//-         +---++---+
//-
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
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.Q, /*    Y1 |2           13| A6   */ F.A,
		  B.A, /*    A2 |3           12| Y6   */ F.Q,
		  B.Q, /*    Y2 |4    7414   11| A5   */ E.A,
		  C.A, /*    A3 |5           10| Y5   */ E.Q,
		  C.Q, /*    Y3 |6            9| A4   */ D.A,
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
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.Q, /*    Y1 |2           13| A6   */ F.A,
		  B.A, /*    A2 |3           12| Y6   */ F.Q,
		  B.Q, /*    Y2 |4   74LS14  11| A5   */ E.A,
		  C.A, /*    A3 |5           10| Y5   */ E.Q,
		  C.Q, /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7416_DIP
//- Title: DM5416/DM7416 Hex Inverting Buffers with High Voltage Open-Collector Outputs
//- Description: This device contains six independent gates each of which performs the logic INVERT function.
//-   The open-collector outputs require external pull-up resistors for proper logical operation.
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006504.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 1 |
//-         | 1 || 0 |
//-         +---++---+
//-
static NETLIST_START(TTL_7416_DIP)
	TTL_7416_GATE(A)
	TTL_7416_GATE(B)
	TTL_7416_GATE(C)
	TTL_7416_GATE(D)
	TTL_7416_GATE(E)
	TTL_7416_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.Q, /*    Y1 |2           13| A6   */ F.A,
		  B.A, /*    A2 |3           12| Y6   */ F.Q,
		  B.Q, /*    Y2 |4    7416   11| A5   */ E.A,
		  C.A, /*    A3 |5           10| Y5   */ E.Q,
		  C.Q, /*    Y3 |6            9| A4   */ D.A,
		A.GND, /*   GND |7            8| Y4   */ D.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7420_DIP
//- Title: 5420/DM5420/DM7420 Dual 4-Input NAND Gates
//- Description: This device contains two independent gates each of which performs the logic NAND function.
//- Pinalias: A1,B1,NC,C1,D1,Y1,GND,Y2,A2,B2,NC,C2,D2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006506.PDF
//-
//-         +---+---+---+---++---+
//-         | A | B | C | D || Y |
//-         +===+===+===+===++===+
//-         | X | X | X | 0 || 1 |
//-         | X | X | 0 | X || 1 |
//-         | X | 0 | X | X || 1 |
//-         | 0 | X | X | X || 1 |
//-         | 1 | 1 | 1 | 1 || 0 |
//-         +---+---+---+---++---+
//-
static NETLIST_START(TTL_7420_DIP)
	TTL_7420_GATE(A)
	TTL_7420_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| D2   */ B.D,
		 NC.I, /*    NC |3           12| C2   */ B.C,
		  A.C, /*    C1 |4    7420   11| NC   */ NC.I,
		  A.D, /*    D1 |5           10| B2   */ B.B,
		  A.Q, /*    Y1 |6            9| A2   */ B.A,
		A.GND, /*   GND |7            8| Y2   */ B.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7421_DIP
//- Title: 54LS21/DM54LS21/DM74LS21 Dual 4-Input AND Gates
//- Description: This device contains two independent 4-input gates each of which performs the logic AND function.
//- Pinalias: A1,B1,NC,C1,D1,Y1,GND,Y2,A2,B2,NC,C2,D2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006356.PDF
//-
//-         +---+---+---+---++---+
//-         | A | B | C | D || Y |
//-         +===+===+===+===++===+
//-         | X | X | X | 0 || 1 |
//-         | X | X | 0 | X || 1 |
//-         | X | 0 | X | X || 1 |
//-         | 0 | X | X | X || 1 |
//-         | 1 | 1 | 1 | 1 || 0 |
//-         +---+---+---+---++---+
//-
static NETLIST_START(TTL_7421_DIP)
	TTL_7421_GATE(A)
	TTL_7421_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| D2   */ B.D,
		 NC.I, /*    NC |3           12| C2   */ B.C,
		  A.C, /*    C1 |4    7421   11| NC   */ NC.I,
		  A.D, /*    D1 |5           10| B2   */ B.B,
		  A.Q, /*    Y1 |6            9| A2   */ B.A,
		A.GND, /*   GND |7            8| Y2   */ B.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7425_DIP
//- Title: 5425/DM7425 Dual 4-Input NOR Gate (with Strobe)
//- Description: This device contains 2, 4-input gates that perform the logical NOR function.
//-   The output of each NOR gate is gated (strobed) by pin 3 and 11 by positive true logic, i.e., logic "1" equals output on.
//- Pinalias: A1,B1,X1,C1,D1,Y1,GND,Y2,A2,B2,X2,C2,D2
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: The "X" input and high impedance output are currently not simulated.
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet_pdf/national-semiconductor/5425DMQB_to_DM7425N.pdf
//-
//-         +---+---+---+---+---++---+
//-         | A | B | C | D | X || Y |
//-         +===+===+===+===+===++===+
//-         | X | X | X | X | 0 || Z |
//-         | 0 | 0 | 0 | 0 | 1 || 1 |
//-         | X | X | X | 1 | 1 || 0 |
//-         | X | X | 1 | X | 1 || 0 |
//-         | X | 1 | X | X | 1 || 0 |
//-         | 1 | X | X | X | 1 || 0 |
//-         +---+---+---+---+---++---+
//-
static NETLIST_START(TTL_7425_DIP)
	TTL_7425_GATE(A)
	TTL_7425_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(XA) // FIXME: Functionality needs to be implemented
	NC_PIN(XB) // FIXME: Functionality needs to be implemented

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| D2   */ B.D,
		 XA.I, /*    X1 |3           12| C2   */ B.C,
		  A.C, /*    C1 |4    7425   11| X2   */ XB.I,
		  A.D, /*    D1 |5           10| B2   */ B.B,
		  A.Q, /*    Y1 |6            9| A2   */ B.A,
		A.GND, /*   GND |7            8| Y2   */ B.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7427_DIP
//- Title: DM7427 Triple 3-Input NOR Gates
//- Description: This device contains three independent gates each of which performs the logic NOR function.
//- Pinalias: A1,B1,A2,B2,C2,Y2,GND,Y3,A3,B3,C3,Y1,C1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006509.PDF
//-
//-         +---+---+---++---+
//-         | A | B | C || Y |
//-         +===+===+===++===+
//-         | X | X | 1 || 0 |
//-         | X | 1 | X || 0 |
//-         | 1 | X | X || 0 |
//-         | 0 | 0 | 0 || 1 |
//-         +---+---+---++---+
//-
static NETLIST_START(TTL_7427_DIP)
	TTL_7427_GATE(A)
	TTL_7427_GATE(B)
	TTL_7427_GATE(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| C1   */ A.C,
		  B.A, /*    A2 |3           12| Y1   */ A.Q,
		  B.B, /*    B2 |4    7427   11| C3   */ C.C,
		  B.C, /*    C2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7430_DIP
//- Title: 5430/DM5430/DM7430 8-Input NAND Gate
//- Description: This device contains a single gate which performs the logic NAND function.
//- Pinalias: A,B,C,D,E,F,GND,Y,NC,NC,G,H,NC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006510.PDF
//-
//-         +---+---+---+---+---+---+---+---++---+
//-         | A | B | C | D | E | F | G | H || Y |
//-         +===+===+===+===+===+===+===+===++===+
//-         | X | X | X | X | X | X | X | 0 || 1 |
//-         | X | X | X | X | X | X | 0 | X || 1 |
//-         | X | X | X | X | X | 0 | X | X || 1 |
//-         | X | X | X | X | 0 | X | X | X || 1 |
//-         | X | X | X | 0 | X | X | X | X || 1 |
//-         | X | X | 0 | X | X | X | X | X || 1 |
//-         | X | 0 | X | X | X | X | X | X || 1 |
//-         | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 || 0 |
//-         +---+---+---+---+---+---+---+---++---+
//-
static NETLIST_START(TTL_7430_DIP)
	TTL_7430_GATE(A)
	NC_PIN(NC)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*     A |1     ++    14| VCC  */ A.VCC,
		  A.B, /*     B |2           13| NC   */ NC.I,
		  A.C, /*     C |3           12| H    */ A.H,
		  A.D, /*     D |4    7430   11| G    */ A.G,
		  A.E, /*     E |5           10| NC   */ NC.I,
		  A.F, /*     F |6            9| NC   */ NC.I,
		A.GND, /*   GND |7            8| Y    */ A.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7432_DIP
//- Title: 5432/DM5432/DM7432 Quad 2-Input OR Gates
//- Description: This device contains four independent gates each of whichperforms the logic OR function.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006511.PDF
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 0 |
//-         | 0 | 1 || 1 |
//-         | 1 | 0 || 1 |
//-         | 1 | 1 || 1 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7432_DIP)
	TTL_7432_GATE(A)
	TTL_7432_GATE(B)
	TTL_7432_GATE(C)
	TTL_7432_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| B4   */ D.B,
		  A.Q, /*    Y1 |3           12| A4   */ D.A,
		  B.A, /*    A2 |4    7432   11| Y4   */ D.Q,
		  B.B, /*    B2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7437_DIP
//- Title: 5437/DM5437/DM7437 Quad 2-Input NAND Buffers
//- Description: This device contains four independent gates each of whichperforms the logic OR function.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: Same as 7400, but drains higher output currents. Netlist currently does not model over currents (should it ever?)
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheets/105/236976_DS.pdf
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 1 |
//-         | 0 | 1 || 1 |
//-         | 1 | 0 || 1 |
//-         | 1 | 1 || 0 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7437_DIP)
	TTL_7437_GATE(A)
	TTL_7437_GATE(B)
	TTL_7437_GATE(C)
	TTL_7437_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| B4   */ D.B,
		  A.Q, /*    Y1 |3           12| A4   */ D.A,
		  B.A, /*    A2 |4    7437   11| Y4   */ D.Q,
		  B.B, /*    B2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7442_DIP
//- Title: 5442A/DM5442A/DM7442A BCD to Decimal Decoders
//- Description: These BCD-to-decimal decoders consist of eight inverters
//-   and ten, four-input NAND gates. The inverters are
//-   connected in pairs to make BCD input data available for
//-   decoding by the NAND gates. Full decoding of input
//-   logic ensures that all outputs remain off for all invalid
//-   (10-15) input conditions.
//- Pinalias: 0,1,2,3,4,5,6,GND,7,8,9,D,C,B,A,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006516.PDF
//-
//-         +---+---+---+---++---+---+---+---+---+---+---+---+---+---+
//-         | D | C | B | A || 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
//-         +===+===+===+===++===+===+===+===+===+===+===+===+===+===+
//-         | 0 | 0 | 0 | 0 || 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 0 | 0 | 0 | 1 || 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 0 | 0 | 1 | 0 || 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 0 | 0 | 1 | 1 || 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 0 | 1 | 0 | 0 || 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 | 1 |
//-         | 0 | 1 | 0 | 1 || 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 | 1 |
//-         | 0 | 1 | 1 | 0 || 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 | 1 |
//-         | 0 | 1 | 1 | 1 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 | 1 |
//-         | 1 | 0 | 0 | 0 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 1 |
//-         | 1 | 0 | 0 | 1 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
//-         | 1 | 0 | 1 | 0 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 1 | 0 | 1 | 1 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 1 | 1 | 0 | 0 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 1 | 1 | 0 | 1 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 1 | 1 | 1 | 0 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         | 1 | 1 | 1 | 1 || 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         +---+---+---+---++---+---+---+---+---+---+---+---+---+---+
//-
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

//- Identifier: TTL_7448_DIP
//- Title: DM5448/DM48LS48/DM7448/DM74LS48 BCD to 7-Segment Decoder
//- Description: The 48 and LS48 feature active-high outputs for
//-   driving lamp buffers or common-cathode LED's.
//-   These circuits have full ripple-blanking input/output controls
//-   and a lamp test input. Display patterns
//-   for BCD input counts above nine are unique symbols
//-   to authenticate input conditions.
//-   These circuits incorporate automatic
//-   leading and/or trailing-edge, zero-blanking control
//-   (RBI and RBO). Lamp test (LT) of these devices may
//-   be performed at any time when the BI/RBO node is at
//-   a high logic level. They contain
//-   an overriding blanking input (BI) which can be used
//-   to control the lamp intensity (by pulsing), or to inhibit
//-   the outputs.
//- Pinalias: A1,A2,LTQ,BIQ,RBIQ,A3,A0,GND,e,d,c,b,a,g,f,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS010172.PDF
//-
//-         +-----+------+----+----+----+----++-----+---+---+---+---+---+---+---+
//-         | LTQ | RBIQ | A3 | A2 | A1 | A0 || BIQ | a | b | c | d | e | f | g |
//-         +=====+======+====+====+====+====++=====+===+===+===+===+===+===+===+
//-         |  1  |   1  |  0 |  0 |  0 |  0 ||  1  | 1 | 1 | 1 | 1 | 1 | 1 | 0 |
//-         |  1  |   X  |  0 |  0 |  0 |  1 ||  1  | 0 | 1 | 1 | 0 | 0 | 0 | 0 |
//-         |  1  |   X  |  0 |  0 |  1 |  0 ||  1  | 1 | 1 | 0 | 1 | 1 | 0 | 1 |
//-         |  1  |   X  |  0 |  0 |  1 |  1 ||  1  | 1 | 1 | 1 | 1 | 0 | 0 | 1 |
//-         |  1  |   X  |  0 |  1 |  0 |  0 ||  1  | 0 | 1 | 1 | 0 | 0 | 1 | 1 |
//-         |  1  |   X  |  0 |  1 |  0 |  1 ||  1  | 1 | 0 | 1 | 1 | 0 | 1 | 1 |
//-         |  1  |   X  |  0 |  1 |  1 |  0 ||  1  | 0 | 0 | 1 | 1 | 1 | 1 | 1 |
//-         |  1  |   X  |  0 |  1 |  1 |  1 ||  1  | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
//-         |  1  |   X  |  1 |  0 |  0 |  0 ||  1  | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         |  1  |   X  |  1 |  0 |  0 |  1 ||  1  | 1 | 1 | 1 | 0 | 0 | 1 | 1 |
//-         |  1  |   X  |  1 |  0 |  1 |  0 ||  1  | 0 | 0 | 0 | 1 | 1 | 0 | 1 |
//-         |  1  |   X  |  1 |  0 |  1 |  1 ||  1  | 0 | 0 | 1 | 1 | 0 | 0 | 1 |
//-         |  1  |   X  |  1 |  1 |  0 |  0 ||  1  | 0 | 1 | 0 | 0 | 0 | 1 | 1 |
//-         |  1  |   X  |  1 |  1 |  0 |  1 ||  1  | 1 | 0 | 0 | 1 | 0 | 1 | 1 |
//-         |  1  |   X  |  1 |  1 |  1 |  0 ||  1  | 0 | 0 | 0 | 1 | 1 | 1 | 1 |
//-         |  1  |   X  |  1 |  1 |  1 |  1 ||  1  | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
//-         |  X  |   X  |  X |  X |  X |  X ||  0  | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
//-         |  1  |   0  |  0 |  0 |  0 |  0 ||  0  | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
//-         |  0  |   X  |  X |  X |  X |  X ||  1  | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
//-         +-----+------+----+----+----+----++-----+---+---+---+---+---+---+---+
//-
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

//- Identifier: TTL_7450_DIP
//- Title: DM7450 Expandable Dual 2-Wide 2-Input AND-OR-INVERT Gate
//- Description: This device contains two independent combinations of gates, each of which perform the logic AND-OR-INVERT function.
//-   One set of gates has an expander node.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: Expander signal is not implemented
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheets/105/236976_DS.pdf
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 1 |
//-         | 0 | 1 || 1 |
//-         | 1 | 0 || 1 |
//-         | 1 | 1 || 0 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7450_DIP)
	TTL_7450_ANDORINVERT(A)
	TTL_7450_ANDORINVERT(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    1A |1     ++    14| VCC  */ A.VCC,
		  B.A, /*    2A |2           13| 1B   */ A.B,
		  B.B, /*    2B |3           12| 1XQ  */ NC.I,
		  B.C, /*    2C |4    7450   11| 1X   */ NC.I,
		  B.D, /*    2D |5           10| 1D   */ A.D,
		  B.Q, /*    2Y |6            9| 1C   */ A.C,
		A.GND, /*   GND |7            8| 1Y   */ A.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7473_DIP
//- Title: 5473/DM5473/DM7473 Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
//- Description: This device contains two independent positive pulse triggered J-K flip-flops with complementary outputs.
//-   The J and K data is processed by the flip-flops after a complete clock pulse.
//-   While the clock is low the slave is isolated from the master.
//-   On the positive transition of the clock, the data from the J and K inputs is transferred to teh master.
//-   While the clock is high the J and K inputs are disabled.
//-   On the negative transition of the clock, the data from the master is transferred to the slave.
//-   The logic states of the J and K inputs must not be allowed to change while the clock is high.
//-   Data transfers to the outputs on the falling edge of the clock pulse.
//-   A low logic level on the clear input will reset the outputs regardless of the logic states of the other inputs.
//- Pinalias: CLK1,CLR1,K1,VCC,CLK2,CLR2,J2,QQ2,Q2,K2,GND,Q1,QQ1,J1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet_pdf/national-semiconductor/5473DMQB_to_DM7473N.pdf
//-
//-         +-----+-------+---+---++---+----+
//-         | CLR |  CLK  | J | K || Q | QQ |
//-         +=====+=======+===+===++===+====+
//-         |  0  |   X   | X | X || 0 |  1 |
//-         |  1  | 0-1-0 | 0 | 0 || Q | QQ |
//-         |  1  | 0-1-0 | 1 | 0 || 1 |  0 |
//-         |  1  | 0-1-0 | 0 | 1 || 0 |  1 |
//-         |  1  | 0-1-0 | 1 | 1 || Toggle |
//-         +-----+-------+---+---++---+----+
//-
static NETLIST_START(TTL_7473_DIP)
	TTL_7473(A)
	TTL_7473(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+      */
		 A.CLK, /*  CLK1 |1     ++    14| J1   */ A.J,
		A.CLRQ, /*  CLR1 |2           13| QQ1  */ A.QQ,
		   A.K, /*    K1 |3           12| Q1   */ A.Q,
		 A.VCC, /*   VCC |4    7473   11| GND  */ A.GND,
		 B.CLK, /*  CLK2 |5           10| K2   */ B.K,
		B.CLRQ, /*  CLR2 |6            9| Q2   */ B.Q,
		   B.J, /*    J2 |7            8| QQ2  */ B.QQ
			    /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7473A_DIP
//- Title: DM54LS73A/DM74LS73A Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
//- Description: This device contains two independent negative-edge-triggered J-K flip-flops with complementary outputs.
//-   The J and K data is processed by the flip-flops on the falling edge of the clock pulse.
//-   The clock triggering occurs at a voltage level and is not directly related to the transition time of the negative going edge of the clock pulse.
//-   The data on the J and K inputs is allowed to change while the clock is high or low without affecting the outputs as long as setup and hold times are not violated.
//-   A low logic level on the clear input will reset the outputs regardless of the levels of the other inputs.
//- Pinalias: CLK1,CLR1,K1,VCC,CLK2,CLR2,J2,QQ2,Q2,K2,GND,Q1,QQ1,J1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semicouductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006372.PDF
//-
//-         +-----+-----+---+---++---+----+
//-         | CLR | CLK | J | K || Q | QQ |
//-         +=====+=====+===+===++===+====+
//-         |  0  |  X  | X | X || 0 |  1 |
//-         |  1  | 1-0 | 0 | 0 || Q | QQ |
//-         |  1  | 1-0 | 1 | 0 || 1 |  0 |
//-         |  1  | 1-0 | 0 | 1 || 0 |  1 |
//-         |  1  | 1-0 | 1 | 1 || Toggle |
//-         |  1  |  1  | X | X || Q | QQ |
//-         +-----+-----+---+---++---+----+
//-
static NETLIST_START(TTL_7473A_DIP)
	TTL_7473A(A)
	TTL_7473A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+      */
		 A.CLK, /*  CLK1 |1     ++    14| J1   */ A.J,
		A.CLRQ, /*  CLR1 |2           13| QQ1  */ A.QQ,
		   A.K, /*    K1 |3           12| Q1   */ A.Q,
		 A.VCC, /*   VCC |4   7473A   11| GND  */ A.GND,
		 B.CLK, /*  CLK2 |5           10| K2   */ B.K,
		B.CLRQ, /*  CLR2 |6            9| Q2   */ B.Q,
		   B.J, /*    J2 |7            8| QQ2  */ B.QQ
			    /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7474_DIP
//- Title: 5474/DM5474/DM7474 Dual Positive-Edge-Triggered D Flip-Flops with Preset, Clear and Complementary Outputs
//- Description: This device contains two independent positive-edge-triggered D flip-flops with complementary outputs.
//-   The information on the D input is accepted by the flip-flops on the positive going edge of the clock pulse.
//-   The triggering occurs at a voltage level and is not directly related to the transition time of the rising edge of the clock.
//-   The data on the D input may be changed while the clock is low or high without affecting the outputs as long as the data setup and hold times are not violated.
//-   A low logic level on the preset or clear inputs will set or reset the outputs regardless of the logic levels of the other inputs.
//- Pinalias: CLR1,D1,CLK1,PR1,Q1,QQ1,GND,QQ2,Q2,PR2,CLK2,D2,CLR2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006526.PDF
//-
//-         +----+-----+-----+---++---+----+
//-         | PR | CLR | CLK | D || Q | QQ |
//-         +====+=====+=====+===++===+====+
//-         |  0 |  1  |  X  | X || 1 |  0 |
//-         |  1 |  0  |  X  | X || 0 |  1 |
//-         |  0 |  0  |  X  | X || 1 |  1 | (unstable)
//-         |  1 |  1  | 0-1 | 1 || 1 |  0 |
//-         |  1 |  1  | 0-1 | 0 || 0 |  1 |
//-         |  1 |  1  |  0  | X || Q | QQ |
//-         +----+-----+-----+---++---+----+
//-
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
		  A.QQ, /*   QQ1 |6            9| Q2    */ B.Q,
		 A.GND, /*   GND |7            8| QQ2   */ B.QQ
			    /*       +-------------+        */
	)
NETLIST_END()

//- Identifier: TTL_7475_DIP
//- Title: DM5475/DM7475/DM7475A/DM74LS75 4-bit D Latch
//- Description: These latches are ideally suited for use as temporary
//-    storage for binary information between processing units
//-    and input/output or indicator units. Information present
//-    at a data (D) input is transferred to the Q output when
//-    the enable (G) is high, and the Q output will follow
//-    the data input as long as the enable remains high. When
//-    the enable goes low, the information (that was present
//-    at the data input at the time the transition occurred) is
//-    retained at the Q output until the enable is permitted
//-    to go high.
//-
//-    The DM5475/DM7475, DM54L75A/DM74L75A, and
//-    DM54LS75/DM74LS75 feature complementary Q and
//-    QQ outputs from a 4-bit latch, and are available in 16-pin
//-    packages.
//- Pinalias: QQ1,D1,D2,E34,VCC,D3,D4,QQ4,Q4,Q3,QQ3,GND,E12,QQ2,Q2,Q1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   https://ia800608.us.archive.org/5/items/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
//-
//-         +---+---++---+----+
//-         | D | G || Q | QQ |
//-         +===+===++===+====+
//-         | 0 | 1 || 0 |  1 |
//-         | 1 | 1 || 1 |  0 |
//-         | X | 0 || Q | QQ |
//-         +---+---++---+----+
//-
static NETLIST_START(TTL_7475_DIP)
	TTL_7475_GATE(A)
	TTL_7475_GATE(B)
	TTL_7475_GATE(C)
	TTL_7475_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	NET_C(A.CLK, B.CLK)
	NET_C(C.CLK, D.CLK)

	DIPPINS(   /*       +--------------+       */
		 A.QQ, /*   QQ1 |1     ++    16| Q1    */ A.Q,
		  A.D, /*    D1 |2           15| Q2    */ B.Q,
		  B.D, /*    D2 |3           14| QQ2   */ B.QQ,
		C.CLK, /*   E34 |4    7475   13| E12   */ A.CLK,
		A.VCC, /*   VCC |5           12| GND   */ A.GND,
		  C.D, /*    D3 |6           11| QQ3   */ C.QQ,
		  D.D, /*    D4 |7           10| Q3    */ C.Q,
		 D.QQ, /*   QQ4 |8            9| Q4    */ D.Q
			   /*       +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_7477_DIP
//- Title: DM74LS77 4-bit D Latch
//- Description: These latches are ideally suited for use as temporary
//-    storage for binary information between processing units
//-    and input/output or indicator units. Information present
//-    at a data (D) input is transferred to the Q output when
//-    the enable (G) is high, and the Q output will follow
//-    the data input as long as the enable remains high. When
//-    the enable goes low, the information (that was present
//-    at the data input at the time the transition occurred) is
//-    retained at the Q output until the enable is permitted
//-    to go high.
//- Pinalias: D1,D2,E34,VCC,D3,D4,NC,Q4,Q3,NC,GND,E12,Q2,Q1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   https://ia800608.us.archive.org/5/items/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
//-
//-         +---+---++---+----+
//-         | D | G || Q | QQ |
//-         +===+===++===+====+
//-         | 0 | 1 || 0 |  1 |
//-         | 1 | 1 || 1 |  0 |
//-         | X | 0 || Q | QQ |
//-         +---+---++---+----+
//-
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

	DIPPINS(   /*       +--------------+       */
		  A.D, /*    D1 |1     ++    14| Q1    */ A.Q,
		  B.D, /*    D2 |2           13| Q2    */ B.Q,
		C.CLK, /*   E34 |3           12| E12   */ A.CLK,
		A.VCC, /*   VCC |4    7477   11| GND   */ A.GND,
		  C.D, /*    D3 |5           10| NC    */ NC.I,
		  D.D, /*    D4 |6            9| Q3    */ C.Q,
		 NC.I, /*    NC |7            8| Q4    */ D.Q
			   /*       +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_7483_DIP
//- Title: DM5483/DM7483/DM74LS83A 4-bit Binary Adders With Fast Carry
//- Description: These full adders perform the addition of two 4-bit
//-   binary numbers. The sum (S) outputs are provided for
//-   each bit and the resultant carry (C4) is obtained from
//-   the fourth bit. These adders feature full internal look
//-   ahead across all four bits. This provides the system
//-   designer with partial look-ahead performance at the
//-   economy and reduced package count of a ripple-carry
//-   implementation.
//-
//-   The adder logic, including the carry, is implemented in
//-   its true form meaning that the end-around carry can be
//-   accomplished without the need for logic or level inversion.
//- Pinalias: A4,S3,A3,B3,VCC,S2,B2,A2,S1,A1,B1,GND,C0,C4,S4,B4
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006378.PDF
//-
static NETLIST_START(TTL_7483_DIP)
	TTL_7483(A)

	DIPPINS(   /*      +--------------+     */
		 A.A4, /*   A4 |1     ++    16| B4  */ A.B4,
		 A.S3, /*   S3 |2           15| S4  */ A.S4,
		 A.A3, /*   A3 |3           14| C4  */ A.C4,
		 A.B3, /*   B3 |4    7483   13| C0  */ A.C0,
		A.VCC, /*  VCC |5           12| GND */ A.GND,
		 A.S2, /*   S2 |6           11| B1  */ A.B1,
		 A.B2, /*   B2 |7           10| A1  */ A.A1,
		 A.A2, /*   A2 |8            9| S1  */ A.S1
			   /*      +--------------+     */
	)
NETLIST_END()

//- Identifier: TTL_7485_DIP
//- Title: DM5485/DM7485/DM74L85/DM74LS85 4-Bit Magnitude Comparators
//- Description: These four-bit magnitude comparators perform comparison
//-   of straight binary or BCD codes. Three
//-   fully-decoded decisions about two, 4-bit words (A, B)
//-   are made and are externally available at three outputs.
//-   These devices are fully expandable to any number of
//-   bits without external gates. Words of greater length may
//-   be compared by connecting comparators in cascade. The
//-   A > B, A < B, and A = B outputs of a stage handling
//-   less-significant bits are connected to the corresponding
//-   inputs of the next stage handling more-significant bits.
//-   The stage handling the least-significant bits must have a
//-   high-level voltage applied to the A = B input and in
//-   addition for the L85, low-level voltages applied to the
//-   A> B and A < B inputs. The cascading paths of the 85,
//-   and LS85 are implemented with only a two-gate-Ievel
//-   delay to reduce overall comparison times for long words.
//- Pinalias: B3,LTIN,EQIN,GTIN,GTOUT,EQOUT,LTOUT,GND,B0,A0,B1,A1,A2,B2,A3,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006379.PDF
//-
static NETLIST_START(TTL_7485_DIP)
	TTL_7485(A)

	DIPPINS(     /*       +--------------+      */
		   A.B3, /*    B3 |1     ++    16| VCC  */ A.VCC,
		 A.LTIN, /*  LTIN |2           15| A3   */ A.A3,
		 A.EQIN, /*  EQIN |3           14| B2   */ A.B2,
		 A.GTIN, /*  GTIN |4    7485   13| A2   */ A.A2,
		A.GTOUT, /* GTOUT |5           12| A1   */ A.A1,
		A.EQOUT, /* EQOUT |6           11| B1   */ A.B1,
		A.LTOUT, /* LTOUT |7           10| A0   */ A.A0,
		  A.GND, /*   GND |8            9| B0   */ A.B0
			     /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7486_DIP
//- Title: 5486/DM5486/DM7486 Quad 2-Input Exclusive-OR Gates
//- Description: This device contains four independent gates each of which performs the logic exclusive-OR function.
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006531.PDF
//-
//-         +---+---++---+
//-         | A | B || Y |
//-         +===+===++===+
//-         | 0 | 0 || 0 |
//-         | 0 | 1 || 1 |
//-         | 1 | 0 || 1 |
//-         | 1 | 1 || 0 |
//-         +---+---++---+
//-
static NETLIST_START(TTL_7486_DIP)
	TTL_7486_GATE(A)
	TTL_7486_GATE(B)
	TTL_7486_GATE(C)
	TTL_7486_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.A, /*    A1 |1     ++    14| VCC  */ A.VCC,
		  A.B, /*    B1 |2           13| B4   */ D.B,
		  A.Q, /*    Y1 |3           12| A4   */ D.A,
		  B.A, /*    A2 |4    7486   11| Y4   */ D.Q,
		  B.B, /*    B2 |5           10| B3   */ C.B,
		  B.Q, /*    Y2 |6            9| A3   */ C.A,
		A.GND, /*   GND |7            8| Y3   */ C.Q
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7490_DIP
//- Title: DM5490/DM7490A Decade Counter
//- Description: These monolithic counters contain four
//-   master-slave flip-flops and additional gating to provide
//-   a divide-by-two counter and a three-stage binary counter
//-   for which the count cycle length is divide-by-five.
//-
//-   These counters have a gated zero reset and
//-   also have gated set-to-nine inputs
//-   for use in BCD nine's complement applications.
//-
//-   To use their maximum count length (decade), the B input is connected
//-   to the QA output. The input count pulses are applied to
//-   input A and the outputs are as described in the appropriate
//-   truth table. A symmetrical divide-by-ten count can be
//-   obtained from the by connecting the QD output to
//-   the A input and applying the input count to the B input
//-   which gives a divide-by-ten square wave at output QA.
//- Pinalias: B,R01,R02,NC,VCC,R91,R92,QC,QB,GND,QD,QA,NC,A
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006533.PDF
//-
//-               BCD Count Sequence                    BCD Bi-Quinary
//-         +-------++----+----+----+----+      +-------++----+----+----+----+
//-         | Count || QD | QC | QB | QA |      | Count || QD | QC | QB | QA |
//-         +=======++====+====+====+====+      +=======++====+====+====+====+
//-         |   0   ||  0 |  0 |  0 |  0 |      |   0   ||  0 |  0 |  0 |  0 |
//-         |   1   ||  0 |  0 |  0 |  1 |      |   1   ||  0 |  0 |  0 |  1 |
//-         |   2   ||  0 |  0 |  1 |  0 |      |   2   ||  0 |  0 |  1 |  0 |
//-         |   3   ||  0 |  0 |  1 |  1 |      |   3   ||  0 |  0 |  1 |  1 |
//-         |   4   ||  0 |  1 |  0 |  0 |      |   4   ||  0 |  1 |  0 |  0 |
//-         |   5   ||  0 |  1 |  0 |  1 |      |   5   ||  1 |  0 |  0 |  0 |
//-         |   6   ||  0 |  1 |  1 |  0 |      |   6   ||  1 |  0 |  0 |  1 |
//-         |   7   ||  0 |  1 |  1 |  1 |      |   7   ||  1 |  0 |  1 |  0 |
//-         |   8   ||  1 |  0 |  0 |  0 |      |   8   ||  1 |  0 |  1 |  1 |
//-         |   9   ||  1 |  0 |  0 |  1 |      |   9   ||  1 |  1 |  0 |  0 |
//-         +-------++----+----+----+----+      +-------++----+----+----+----+
//-
//-                   Reset/Count Function Table
//-         +-----+-----+-----+-----++----+----+----+----+
//-         | R01 | R02 | R91 | R92 || QD | QC | QB | QA |
//-         +=====+=====+=====+=====++====+====+====+====+
//-         |  1  |  1  |  0  |  X  ||  0 |  0 |  0 |  0 |
//-         |  1  |  1  |  X  |  0  ||  0 |  0 |  0 |  0 |
//-         |  X  |  X  |  1  |  1  ||  1 |  0 |  0 |  1 |
//-         |  X  |  0  |  X  |  0  ||       COUNT       |
//-         |  0  |  X  |  0  |  X  ||       COUNT       |
//-         |  0  |  X  |  X  |  0  ||       COUNT       |
//-         |  X  |  0  |  0  |  X  ||       COUNT       |
//-         +-----+-----+-----+-----++----+----+----+----+
//-
static NETLIST_START(TTL_7490_DIP)
	TTL_7490(A)
	NC_PIN(NC)

	DIPPINS(   /*     +--------------+     */
		  A.B, /*   B |1     ++    14| A   */ A.A,
		 A.R1, /* R01 |2           13| NC  */ NC.I,
		 A.R2, /* R02 |3           12| QA  */ A.QA,
		 NC.I, /*  NC |4    7490   11| QD  */ A.QD,
		A.VCC, /* VCC |5           10| GND */ A.GND,
		A.R91, /* R91 |6            9| QB  */ A.QB,
		A.R92, /* R92 |7            8| QC  */ A.QC
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: TTL_7492_DIP
//- Title: SN5492A, SN54LS92, SN7492A, SN74LS92 Divide-By-Twelve Counter
//- Description: These monolithic counters contains four
//-   master-slave flip-flops and additional gating to provide
//-   a divide-by-two counter and a three-stage binary counter
//-   for which the count cycle length is divide-by-six.
//-
//-   These counters have a gated zero reset.
//-
//-   To use their maximum count length (divide-by-
//-   twelve), the B input is connected
//-   to the QA output. The input count pulses are applied to
//-   input A and the outputs are as described in the appropriate
//-   truth table.
//- Pinalias: CKB,NC,NC,NC,VCC,R01,R02,QD,QC,GND,QB,QA,NC,CLKA
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://pdf1.alldatasheet.com/datasheet-pdf/view/27430/TI/SN7492A.html
//-
//-                 Count Sequence
//-         +-------++----+----+----+----+
//-         | Count || QD | QC | QB | QA |
//-         +=======++====+====+====+====+
//-         |   0   ||  0 |  0 |  0 |  0 |
//-         |   1   ||  0 |  0 |  0 |  1 |
//-         |   2   ||  0 |  0 |  1 |  0 |
//-         |   3   ||  0 |  0 |  1 |  1 |
//-         |   4   ||  0 |  1 |  0 |  0 |
//-         |   5   ||  0 |  1 |  0 |  1 |
//-         |   6   ||  1 |  0 |  0 |  0 |
//-         |   7   ||  1 |  0 |  0 |  1 |
//-         |   8   ||  1 |  0 |  1 |  0 |
//-         |   9   ||  1 |  0 |  1 |  1 |
//-         |  10   ||  1 |  1 |  0 |  0 |
//-         |  11   ||  1 |  1 |  0 |  1 |
//-         +-------++----+----+----+----+
//-
//-             Reset/Count Function Table
//-         +-----+-----++----+----+----+----+
//-         | R01 | R02 || QD | QC | QB | QA |
//-         +=====+=====++====+====+====+====+
//-         |  1  |  1  ||  0 |  0 |  0 |  0 |
//-         |  0  |  X  ||       COUNT       |
//-         |  X  |  0  ||       COUNT       |
//-         +-----+-----++----+----+----+----+
//-
 static NETLIST_START(TTL_7492_DIP)
	TTL_7492(A)
	NC_PIN(NC)

	DIPPINS(   /*      +--------------+      */
		  A.B, /* CLKB |1     ++    14| CLKA */ A.A,
		 NC.I, /*   NC |2           13| NC   */ NC.I,
		 NC.I, /*   NC |3           12| QA   */ A.QA,
		 NC.I, /*   NC |4    7492   11| QD   */ A.QD,
		A.VCC, /*  VCC |5           10| GND  */ A.GND,
		 A.R1, /*  R01 |6            9| QB   */ A.QB,
		 A.R2, /*  R02 |7            8| QC   */ A.QC
			   /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7493_DIP
//- Title: DM7493A Binary Counter
//- Description: These monolithic counters contains four
//-   master-slave flip-flops and additional gating to provide
//-   a divide-by-two counter and a three-stage binary counter
//-   for which the count cycle length is divide-by-eight.
//-
//-   These counters have a gated zero reset.
//-
//-   To use their maximum count length (four-bit binary), the B input is connected
//-   to the QA output. The input count pulses are applied to
//-   input A and the outputs are as described in the appropriate
//-   truth table.
//- Pinalias: B,R01,R02,NC,VCC,NC,NC,QC,QB,GND,QD,QA,NC,A
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006533.PDF
//-
//-                 Count Sequence
//-         +-------++----+----+----+----+
//-         | Count || QD | QC | QB | QA |
//-         +=======++====+====+====+====+
//-         |   0   ||  0 |  0 |  0 |  0 |
//-         |   1   ||  0 |  0 |  0 |  1 |
//-         |   2   ||  0 |  0 |  1 |  0 |
//-         |   3   ||  0 |  0 |  1 |  1 |
//-         |   4   ||  0 |  1 |  0 |  0 |
//-         |   5   ||  0 |  1 |  0 |  1 |
//-         |   6   ||  0 |  1 |  1 |  0 |
//-         |   7   ||  0 |  1 |  1 |  1 |
//-         |   8   ||  1 |  0 |  0 |  0 |
//-         |   9   ||  1 |  0 |  0 |  1 |
//-         |  10   ||  1 |  0 |  1 |  0 |
//-         |  11   ||  1 |  0 |  1 |  1 |
//-         |  12   ||  1 |  1 |  0 |  0 |
//-         |  13   ||  1 |  1 |  0 |  1 |
//-         |  14   ||  1 |  1 |  1 |  0 |
//-         |  15   ||  1 |  1 |  1 |  1 |
//-         +-------++----+----+----+----+
//-
//-             Reset/Count Function Table
//-         +-----+-----++----+----+----+----+
//-         | R01 | R02 || QD | QC | QB | QA |
//-         +=====+=====++====+====+====+====+
//-         |  1  |  1  ||  0 |  0 |  0 |  0 |
//-         |  0  |  X  ||       COUNT       |
//-         |  X  |  0  ||       COUNT       |
//-         +-----+-----++----+----+----+----+
//-
 static NETLIST_START(TTL_7493_DIP)
	TTL_7493(A)
	NC_PIN(NC)

	DIPPINS(    /*      +--------------+      */
		A.CLKB, /* CLKB |1     ++    14| CLKA */ A.CLKA,
		  A.R1, /*  R01 |2           13| NC   */ NC.I,
		  A.R2, /*  R02 |3           12| QA   */ A.QA,
		  NC.I, /*   NC |4    7493   11| QD   */ A.QD,
		 A.VCC, /*  VCC |5           10| GND  */ A.GND,
		  NC.I, /*   NC |6            9| QB   */ A.QB,
		  NC.I, /*   NC |7            8| QC   */ A.QC
		 	    /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_7497_DIP
//- Title: 5497/DM7497 Synchronous Modulo-64 Bit Rate Multiplier
//- Description: The ’97 contains a synchronous 6-stage binary counter and six decoding gates that serve to gate the clock through to the output at a sub-multiple of the input frequency.
//-   The output pulse rate, relative to the clock frequency, is determined by signals applied to the Select (S0–S5) inputs.
//-   Both true and complement outputs are available, along with an enable input for each.
//-   A Count Enable input and a Terminal Count output are provided for cascading two or more packages.
//-   An asynchronous Master Reset input prevents counting and resets the counter.
//- Pinalias: S1,S4,S5,S0,ZQ,Y,TCQ,GND,CP,EZQ,CEQ,EY,MR,S2,S3,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009780.PDF
//-
static NETLIST_START(TTL_7497_DIP)
	TTL_7497(A)

	DIPPINS(      /*       +--------------+ */
		    A.B1, /*    S1 |1     ++    16| VCC   */ A.VCC,
		    A.B4, /*    S4 |2           15| S3    */ A.B3,
		    A.B5, /*    S5 |3           14| S2    */ A.B2,
		    A.B0, /*    S0 |4    7497   13| MR    */ A.CLR,
		    A.ZQ, /*    ZQ |5           12| EY    */ A.UNITYQ,
		     A.Y, /*     Y |6           11| CEQ   */ A.ENQ,
		A.ENOUTQ, /*   TCQ |7           10| EZQ   */ A.STRBQ,
		   A.GND, /*   GND |8            9| CP    */ A.CLK
			      /*       +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_74107_DIP
//- Title: SN54107, SN74107 Dual J-K Flip-Flops With Clear
//- Description: The '107 contains two independent J-K flip-flops with individual J-K, clock, and direct clear inputs.
//-   The '107 is a positive pulse-triggered flip-flop.
//-   The J-K input data is loaded into the master while the clock is high and transferred to teh slave and the outputs on the high-to-low clock transition.
//-   For these devices the J and K inputs must be stable while the clock is high.
//- Pinalias: 1J,1QQ,1Q,1K,2Q,2QQ,GND,2J,2CLK,2CLRQ,2K,1CLK,1CLRQ,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://pdf1.alldatasheet.com/datasheet-pdf/view/840452/TI1/SN74107.html
//-
//-         +------+-------+---+---++---+----+
//-         | CLRQ |  CLK  | J | K || Q | QQ |
//-         +======+=======+===+===++===+====+
//-         |   0  |   X   | X | X || 0 |  1 |
//-         |   1  | 0-1-0 | 0 | 0 || Q | QQ |
//-         |   1  | 0-1-0 | 1 | 0 || 1 |  0 |
//-         |   1  | 0-1-0 | 0 | 1 || 0 |  1 |
//-         |   1  | 0-1-0 | 1 | 1 || TOGGLE |
//-         +------+-------+---+---++---+----+
//-
static NETLIST_START(TTL_74107_DIP)
#if (NL_USE_TRUTHTABLE_74107)
	TTL_74107_TT(A)
	TTL_74107_TT(B)
#else
	TTL_74107(A)
	TTL_74107(B)
#endif

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*     +--------------+       */
		  A.J, /*  1J |1     ++    14| VCC   */ A.VCC,
		 A.QQ, /* 1QQ |2           13| 1CLRQ */ A.CLRQ,
		  A.Q, /*  1Q |3           12| 1CLK  */ A.CLK,
		  A.K, /*  1K |4   74107   11| 2K    */ B.K,
		  B.Q, /*  2Q |5           10| 2CLRQ */ B.CLRQ,
		 B.QQ, /* 2QQ |6            9| 2CLK  */ B.CLK,
		B.GND, /* GND |7            8| 2J    */ B.J
			   /*     +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_74107A_DIP
//- Title: DM54LS107A/DM74LS107A Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops withClear and Complementary Outputs
//- Description: This device contains two independent negative-edge-triggered J-K flip-flops with complementary outputs.
//-   The J and K data is processed by the flip-flops on the falling edge of the clock pulse.
//-   The clock triggering occurs at a voltage level and is not directly related to the transition time of the negative going edge of the clock pulse.
//-   The data on the J and K inputs may change while the clock is high or low without affecting the outputs as long as setup and hold times are not violated.
//-   A low logic level on the clear input will reset the outputs regardless of the logic levels of the other inputs.
//- Pinalias: J1,QQ1,Q1,K1,Q2,QQ2,GND,J2,CLK2,CLRQ2,K2,CLK1,CLRQ1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006367.PDF
//-
//-         +------+-----+---+---++---+----+
//-         | CLRQ | CLK | J | K || Q | QQ |
//-         +======+=====+===+===++===+====+
//-         |   0  |  X  | X | X || 0 |  1 |
//-         |   1  | 1-0 | 0 | 0 || Q | QQ |
//-         |   1  | 1-0 | 1 | 0 || 1 |  0 |
//-         |   1  | 1-0 | 0 | 1 || 0 |  1 |
//-         |   1  | 1-0 | 1 | 1 || TOGGLE |
//-         |   1  |  1  | X | X || Q | QQ |
//-         +------+-----+---+---++---+----+
//-
static NETLIST_START(TTL_74107A_DIP)
	TTL_74107A(A)
	TTL_74107A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*     +--------------+       */
		  A.J, /*  J1 |1     ++    14| VCC   */ A.VCC,
		 A.QQ, /* QQ1 |2           13| CLRQ1 */ A.CLRQ,
		  A.Q, /*  Q1 |3           12| CLK1  */ A.CLK,
		  A.K, /*  K1 |4   74107A  11| K2    */ B.K,
		  B.Q, /*  Q2 |5           10| CLRQ2 */ B.CLRQ,
		 B.QQ, /* QQ2 |6            9| CLK2  */ B.CLK,
		B.GND, /* GND |7            8| J2    */ B.J
			   /*     +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_74113_DIP
//- Title: DM54S113/DM74S113 Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Preset and Complementary Outputs
//- Description: This device contains two independent negative-edge-triggered J-K flip-flops with complementary outputs.
//-   The J and K data is processed by the flip-flops on the falling edge of the clock pulse.
//-   The clock triggering occurs at a voltage level and is not directly related to the transition time of the negative going edge of the clock pulse.
//-   Data on the J and K inputs may be changed while the clock is high or low without affecting the outputs as long as setup and hold times are not violated.
//-   A low logic level on the preset input will set the outputs regardless of the logic levels of the other inputs.
//- Pinalias: CLK1,K1,J1,PRQ1,Q1,QQ1,GND,QQ2,Q2,PRQ2,J2,K2,CLK2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006460.PDF
//-
//-         +-----+-----+---+---++---+----+
//-         | PRQ | CLK | J | K || Q | QQ |
//-         +=====+=====+===+===++===+====+
//-         |  0  |  X  | X | X || 1 |  0 |
//-         |  1  | 1-0 | 0 | 0 || Q | QQ |
//-         |  1  | 1-0 | 1 | 0 || 1 |  0 |
//-         |  1  | 1-0 | 0 | 1 || 0 |  1 |
//-         |  1  | 1-0 | 1 | 1 || TOGGLE |
//-         |  1  |  1  | X | X || Q | QQ |
//-         +-----+-----+---+---++---+----+
//-
static NETLIST_START(TTL_74113_DIP)
	TTL_74113(A)
	TTL_74113(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+       */
		 A.CLK, /*  CLK1 |1     ++    14| VCC   */ A.VCC,
		   A.K, /*    K1 |2           13| CLK2  */ B.CLK,
		   A.J, /*    J1 |3           12| K2    */ B.K,
		A.SETQ, /*  PRQ1 |4   74113   11| J2    */ B.J,
		   A.Q, /*    Q1 |5           10| PRQ2  */ B.SETQ,
		  A.QQ, /*   QQ1 |6            9| Q2    */ B.Q,
		 A.GND, /*   GND |7            8| QQ2   */ B.QQ
			    /*       +--------------+       */
	)
NETLIST_END()

//- Identifier: TTL_74113A_DIP
//- Title: DM54S113/DM74S113 Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Preset and Complementary Outputs
//- Description: This device contains two independent negative-edge-triggered J-K flip-flops with complementary outputs.
//-   The J and K data is processed by the flip-flops on the falling edge of the clock pulse.
//-   The clock triggering occurs at a voltage level and is not directly related to the transition time of the negative going edge of the clock pulse.
//-   Data on the J and K inputs may be changed while the clock is high or low without affecting the outputs as long as setup and hold times are not violated.
//-   A low logic level on the preset input will set the outputs regardless of the logic levels of the other inputs.
//- Pinalias: CLK1,K1,J1,PRQ1,Q1,QQ1,GND,QQ2,Q2,PRQ2,J2,K2,CLK2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://pdf1.alldatasheet.com/datasheet-pdf/view/131122/TI/SN74LS113A.html
//-
//-         +------+-----+---+---++---+----+
//-         | PREQ | CLK | J | K || Q | QQ |
//-         +======+=====+===+===++===+====+
//-         |   0  |  X  | X | X || 1 |  0 |
//-         |   1  | 1-0 | 0 | 0 || Q | QQ |
//-         |   1  | 1-0 | 1 | 0 || 1 |  0 |
//-         |   1  | 1-0 | 0 | 1 || 0 |  1 |
//-         |   1  | 1-0 | 1 | 1 || TOGGLE |
//-         |   1  |  1  | X | X || Q | QQ |
//-         +------+-----+---+---++---+----+
//-
static NETLIST_START(TTL_74113A_DIP)
	TTL_74113A(A)
	TTL_74113A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+       */
		 A.CLK, /*  CLK1 |1     ++    14| VCC   */ A.VCC,
		   A.K, /*    K1 |2           13| CLK2  */ B.CLK,
		   A.J, /*    J1 |3           12| K2    */ B.K,
		A.SETQ, /*  PRQ1 |4   74113A  11| J2    */ B.J,
		   A.Q, /*    Q1 |5           10| PRQ2  */ B.SETQ,
		  A.QQ, /*   QQ1 |6            9| Q2    */ B.Q,
		 A.GND, /*   GND |7            8| QQ2   */ B.QQ
			    /*       +--------------+       */
	)
NETLIST_END()

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
	NC_PIN(NC)
	RES(RINT, RES_K(2))
	RES(RD, RES_M(1000))

	NET_C(RINT.2, A.RC)
	// Avoid error messages if RINT is not used.
	NET_C(RINT.1, RD.2)
	NET_C(RD.1, A.GND)

	DIPPINS(    /*       +--------------+           */
		  A.QQ, /*    QQ |1     ++    14| VCC       */ A.VCC,
		  NC.I, /*    NC |2           13| NC        */ NC.I,
		  A.A1, /*    A1 |3           12| NC        */ NC.I,
		  A.A2, /*    A2 |4   74121   11| REXT/CEXT */ A.RC,
		   A.B, /*     B |5           10| CEXT      */ A.C,
		   A.Q, /*     Q |6            9| RINT      */ RINT.1,
		 A.GND, /*   GND |7            8| NC        */ NC.I
			    /*       +--------------+           */
	)
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

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(      /*       +--------------+ */
		     A.A, /*    A1 |1     ++    16| VCC   */ A.VCC,
		     A.B, /*    B1 |2           15| RC1   */ A.RC,
		  A.CLRQ, /* CLRQ1 |3           14| C1    */ A.C,
		    A.QQ, /*   QQ1 |4   74123   13| Q1    */ A.Q,
		     B.Q, /*    Q2 |5           12| QQ2   */ B.QQ,
		     B.C, /*    C2 |6           11| CLRQ  */ B.CLRQ,
		    B.RC, /*   RC2 |7           10| B2    */ B.B,
		   A.GND, /*   GND |8            9| A2    */ B.A
			      /*       +--------------+       */
	)
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
	TTL_74125_GATE(A)
	TTL_74125_GATE(B)
	TTL_74125_GATE(C)
	TTL_74125_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DEFPARAM(FORCE_TRISTATE_LOGIC, "$(@.A.FORCE_TRISTATE_LOGIC")
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(B.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(C.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(D.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")

	DIPPINS(   /*      +--------------+      */
		 A.GQ, /*  1GQ |1     ++    14| VCC  */ A.VCC,
		  A.A, /*   1A |2           13| 4GQ  */ D.GQ,
		  A.Y, /*   1Y |3           12| 4A   */ D.A,
		 B.GQ, /*  2GQ |4   74125   11| 4Y   */ D.Y,
		  B.A, /*   2A |5           10| 3GQ  */ C.GQ,
		  B.Y, /*   2Y |6            9| 3A   */ C.A,
		A.GND, /*  GND |7            8| 3Y   */ C.Y
			   /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74126_DIP
//- Title: DM74LS126A Quad 3-STATE Buffer
//- Description: This device contains four independent gates each of which performs a non-inverting buffer function.
//-   The outputs have the 3-STATE feature.
//-   When enabled, the outputs exhibit the low impedance characteristics of a standard LS output with additional drive capability to permit the driving of buslines without external resistors.
//-   When disabled, both the output transistors are turned OFF presenting a high-impedance state to the bus line.
//-   Thus the output will act neither as a significant load nor as a driver.
//-   To minimize the possibility that two outputs will attempt to take a common bus to opposite logic levels, the disable time is shorter than the enable time of the outputs.
//- Pinalias: C1,A1,Y1,C2,A2,Y2,GND,Y3,A3,C3,Y4,A4,C4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheets/50/232293_DS.pdf
//-
//-         +---+---++------+
//-         | A | C ||   Y  |
//-         +===+===++======+
//-         | 0 | 1 ||   0  |
//-         | 1 | 1 ||   1  |
//-         | X | 0 || Hi-Z |
//-         +---+---++------+
//-
static NETLIST_START(TTL_74126_DIP)
	TTL_74126_GATE(A)
	TTL_74126_GATE(B)
	TTL_74126_GATE(C)
	TTL_74126_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DEFPARAM(FORCE_TRISTATE_LOGIC, 0)
	PARAM(A.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(B.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(C.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")
	PARAM(D.FORCE_TRISTATE_LOGIC, "$(@.FORCE_TRISTATE_LOGIC)")

	DIPPINS(   /*      +--------------+      */
		  A.G, /*   C1 |1     ++    14| VCC  */ A.VCC,
		  A.A, /*   A1 |2           13| C4   */ D.G,
		  A.Y, /*   Y1 |3           12| A4   */ D.A,
		  B.G, /*   C2 |4   74126   11| Y4   */ D.Y,
		  B.A, /*   A2 |5           10| C3   */ C.G,
		  B.Y, /*   Y2 |6            9| A3   */ C.A,
		A.GND, /*  GND |7            8| Y3   */ C.Y
			   /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74153_DIP
//- Title: 54153/DM54153/DM74153 Dual 4-Line to 1-LineData Selectors/Multiplexers
//- Description: Each of these data selectors/multiplexers contains inverters and drivers to supply fully complementary, on-chip, binary decoding data selection to the AND-OR-invert gates.
//-   Separate strobe inputs are provided for each of the two four-line sections.
//- Pinalias: G1,B,1C3,1C2,1C1,1C0,Y1,GND,Y2,2C0,2C1,2C2,2C3,A,G2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006547.PDF
//-
//-         +---+---+----+----+----+----+---++---+
//-         | B | A | C0 | C1 | C2 | C3 | G || Y |
//-         +===+===+====+====+====+====+===++===+
//-         | X | X |  X |  X |  X |  X | 1 || 0 |
//-         | 0 | 0 |  0 |  X |  X |  X | 0 || 0 |
//-         | 0 | 0 |  1 |  X |  X |  X | 0 || 1 |
//-         | 0 | 1 |  X |  0 |  X |  X | 0 || 0 |
//-         | 0 | 1 |  X |  1 |  X |  X | 0 || 1 |
//-         | 1 | 0 |  X |  X |  0 |  X | 0 || 0 |
//-         | 1 | 0 |  X |  X |  1 |  X | 0 || 1 |
//-         | 1 | 1 |  X |  X |  X |  0 | 0 || 0 |
//-         | 1 | 1 |  X |  X |  X |  1 | 0 || 1 |
//-         +---+---+----+----+----+----+---++---+
//-
static NETLIST_START(TTL_74153_DIP)
	NET_REGISTER_DEV(TTL_74153, A)
	NET_REGISTER_DEV(TTL_74153, B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	DIPPINS(   /*     +--------------+     */
		  A.G, /*  G1 |1     ++    16| VCC */ A.VCC,
		  A.B, /*   B |2           15| G2  */ B.G,
		 A.C3, /* 1C3 |3           14| A   */ A.A,
		 A.C2, /* 1C2 |4   74153   13| 2C3 */ B.C3,
		 A.C1, /* 1C1 |5           12| 2C2 */ B.C2,
		 A.C0, /* 1C0 |6           11| 2C1 */ B.C1,
		 A.AY, /*  Y1 |7           10| 2C0 */ B.C0,
		A.GND, /* GND |8            9| Y2  */ B.AY
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: TTL_74155_DIP
//- Title: 54LS155/DM54LS155/DM74LS155 Dual 2-Line to 4-Line Decoders/Demultiplexers
//- Description: These TTL circuits feature dual 1-line-to-4-line demultiplexers with individual strobes and common binary-address inputs in a single 16-pin package.
//-   When both sections are enabled by the strobes, the common address inputs sequentially select and route associated input data to the appropriate output of each section.
//-   The individual strobes permit activating or inhibiting each of the 4-bit sections as desired.
//-   Data applied to input C1 is inverted at its outputs anddata applied at C2 is true through its outputs.
//-   The inverter following the C1 data input permits use as a 3-to-8-line decoder, or 1-to-8-line demultiplexer, without external gating.
//-   Input clamping diodes are provided on these circuits to minimize transmission-line effects and simplify system design.
//- Pinalias: C1,G1,B,1Y3,1Y2,1Y1,1Y0,GND,2Y0,2Y1,2Y2,2Y3,A,G2,C2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006395.PDF
//-
//-              2-line-to-4-line decoder                        2-line-to-4-line decoder
//-     +---+---+----+----++-----+-----+-----+-----+    +---+---+----+----++-----+-----+-----+-----+
//-     | B | A | G1 | C1 || 1Y0 | 1Y1 | 1Y2 | 1Y3 |    | B | A | G2 | C2 || 2Y0 | 2Y1 | 2Y2 | 2Y3 |
//-     +===+===+====+====++=====+=====+=====+=====+    +===+===+====+====++=====+=====+=====+=====+
//-     | X | X |  1 |  X ||  1  |  1  |  1  |  1  |    | X | X |  1 |  X ||  1  |  1  |  1  |  1  |
//-     | 0 | 0 |  0 |  1 ||  0  |  1  |  1  |  1  |    | 0 | 0 |  0 |  0 ||  0  |  1  |  1  |  1  |
//-     | 0 | 1 |  0 |  1 ||  1  |  0  |  1  |  1  |    | 0 | 1 |  0 |  0 ||  1  |  0  |  1  |  1  |
//-     | 1 | 0 |  0 |  1 ||  1  |  1  |  0  |  1  |    | 1 | 0 |  0 |  0 ||  1  |  1  |  0  |  1  |
//-     | 1 | 1 |  0 |  1 ||  1  |  1  |  1  |  0  |    | 1 | 1 |  0 |  0 ||  1  |  1  |  1  |  0  |
//-     | X | X |  X |  0 ||  1  |  1  |  1  |  1  |    | X | X |  X |  1 ||  1  |  1  |  1  |  1  |
//-     +---+---+----+----++-----+-----+-----+-----+    +---+---+----+----++-----+-----+-----+-----+
//-
//-                          3-line-to-8-line decoder
//-     +---+---+---+---++-----+-----+-----+-----+-----+-----+-----+-----+
//-     | C | B | A | G || 2Y0 | 2Y1 | 2Y2 | 2Y3 | 1Y0 | 1Y1 | 1Y2 | 1Y3 |
//-     +===+===+===+===++=====+=====+=====+=====+=====+=====+=====+=====+
//-     | X | X | X | 1 ||  1  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 0 | 0 | 0 ||  0  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 0 | 1 | 0 ||  1  |  0  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 1 | 0 | 0 ||  1  |  1  |  0  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 1 | 1 | 0 ||  1  |  1  |  1  |  0  |  1  |  1  |  1  |  1  |
//-     | 1 | 0 | 0 | 0 ||  1  |  1  |  1  |  1  |  0  |  1  |  1  |  1  |
//-     | 1 | 0 | 1 | 0 ||  1  |  1  |  1  |  1  |  1  |  0  |  1  |  1  |
//-     | 1 | 1 | 0 | 0 ||  1  |  1  |  1  |  1  |  1  |  1  |  0  |  1  |
//-     | 1 | 1 | 1 | 0 ||  1  |  1  |  1  |  1  |  1  |  1  |  1  |  0  |
//-     +---+---+---+---++-----+-----+-----+-----+-----+-----+-----+-----+
//-
static NETLIST_START(TTL_74155_DIP)
	NET_REGISTER_DEV(TTL_74155A_GATE, A)
	NET_REGISTER_DEV(TTL_74155B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*       +--------------+      */
		  A.C, /*    C1 |1     ++    16| VCC  */ A.VCC,
		  A.G, /*    G1 |2           15| B4   */ B.C,
		  A.B, /*     B |3           14| B4   */ B.G,
		  A.3, /*   1Y3 |4   74155   13| A4   */ B.A,
		  B.2, /*   1Y2 |5           12| Y4   */ B.3,
		  B.1, /*   1Y1 |6           11| B3   */ B.2,
		  B.0, /*   1Y0 |7           10| A3   */ B.1,
		A.GND, /*   GND |8            9| Y3   */ B.0
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74156_DIP
//- Title: 54LS156/DM54LS156/DM74LS156 Dual 2-Line to 4-Line Decoders/Demultiplexers with Open-Collector Outputs
//- Description: These TTL circuits feature dual 1-line-to-4-line demultiplexers with individual strobes and common binary-address inputs in a single 16-pin package.
//-   When both sections are enabled by the strobes, the common address inputs sequentially select and route associated input data to the appropriate output of each section.
//-   The individual strobes permit activating or inhibiting each of the 4-bit sections as desired.
//-   Data applied to input C1 is inverted at its outputs anddata applied at C2 is true through its outputs.
//-   The inverter following the C1 data input permits use as a 3-to-8-line decoder, or 1-to-8-line demultiplexer, without external gating.
//-   Input clamping diodes are provided on these circuits to minimize transmission-line effects and simplify system design.
//- Pinalias: C1,G1,B,1Y3,1Y2,1Y1,1Y0,GND,2Y0,2Y1,2Y2,2Y3,A,G2,C2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006395.PDF
//-
//-              2-line-to-4-line decoder                        2-line-to-4-line decoder
//-     +---+---+----+----++-----+-----+-----+-----+    +---+---+----+----++-----+-----+-----+-----+
//-     | B | A | G1 | C1 || 1Y0 | 1Y1 | 1Y2 | 1Y3 |    | B | A | G2 | C2 || 2Y0 | 2Y1 | 2Y2 | 2Y3 |
//-     +===+===+====+====++=====+=====+=====+=====+    +===+===+====+====++=====+=====+=====+=====+
//-     | X | X |  1 |  X ||  1  |  1  |  1  |  1  |    | X | X |  1 |  X ||  1  |  1  |  1  |  1  |
//-     | 0 | 0 |  0 |  1 ||  0  |  1  |  1  |  1  |    | 0 | 0 |  0 |  0 ||  0  |  1  |  1  |  1  |
//-     | 0 | 1 |  0 |  1 ||  1  |  0  |  1  |  1  |    | 0 | 1 |  0 |  0 ||  1  |  0  |  1  |  1  |
//-     | 1 | 0 |  0 |  1 ||  1  |  1  |  0  |  1  |    | 1 | 0 |  0 |  0 ||  1  |  1  |  0  |  1  |
//-     | 1 | 1 |  0 |  1 ||  1  |  1  |  1  |  0  |    | 1 | 1 |  0 |  0 ||  1  |  1  |  1  |  0  |
//-     | X | X |  X |  0 ||  1  |  1  |  1  |  1  |    | X | X |  X |  1 ||  1  |  1  |  1  |  1  |
//-     +---+---+----+----++-----+-----+-----+-----+    +---+---+----+----++-----+-----+-----+-----+
//-
//-                          3-line-to-8-line decoder
//-     +---+---+---+---++-----+-----+-----+-----+-----+-----+-----+-----+
//-     | C | B | A | G || 2Y0 | 2Y1 | 2Y2 | 2Y3 | 1Y0 | 1Y1 | 1Y2 | 1Y3 |
//-     +===+===+===+===++=====+=====+=====+=====+=====+=====+=====+=====+
//-     | X | X | X | 1 ||  1  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 0 | 0 | 0 ||  0  |  1  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 0 | 1 | 0 ||  1  |  0  |  1  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 1 | 0 | 0 ||  1  |  1  |  0  |  1  |  1  |  1  |  1  |  1  |
//-     | 0 | 1 | 1 | 0 ||  1  |  1  |  1  |  0  |  1  |  1  |  1  |  1  |
//-     | 1 | 0 | 0 | 0 ||  1  |  1  |  1  |  1  |  0  |  1  |  1  |  1  |
//-     | 1 | 0 | 1 | 0 ||  1  |  1  |  1  |  1  |  1  |  0  |  1  |  1  |
//-     | 1 | 1 | 0 | 0 ||  1  |  1  |  1  |  1  |  1  |  1  |  0  |  1  |
//-     | 1 | 1 | 1 | 0 ||  1  |  1  |  1  |  1  |  1  |  1  |  1  |  0  |
//-     +---+---+---+---++-----+-----+-----+-----+-----+-----+-----+-----+
//-
static NETLIST_START(TTL_74156_DIP)
	NET_REGISTER_DEV(TTL_74156A_GATE, A)
	NET_REGISTER_DEV(TTL_74156B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*       +--------------+      */
		  A.C, /*    C1 |1     ++    16| VCC  */ A.VCC,
		  A.G, /*    G1 |2           15| B4   */ B.C,
		  A.B, /*     B |3           14| B4   */ B.G,
		  A.3, /*   1Y3 |4   74156   13| A4   */ B.A,
		  B.2, /*   1Y2 |5           12| Y4   */ B.3,
		  B.1, /*   1Y1 |6           11| B3   */ B.2,
		  B.0, /*   1Y0 |7           10| A3   */ B.1,
		A.GND, /*   GND |8            9| Y3   */ B.0
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74157_DIP
//- Title: 54157/DM54157/DM74157 Quad 2-Line to 1-Line Data Selectors/Multiplexers
//- Description: These data selectors/multiplexers contain inverters and drivers to supply full on-chip data selection to the four output gates.
//-   A separate strobe input is provided.
//-   A 4-bit word is selected from one of two sources and is routed to the four outputs.
//- Pinalias: S,A1,B1,Y1,A2,B2,Y2,GND,Y3,B3,A3,Y4,B4,A4,G,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheets/120/236599_DS.pdf
//-
//-     +---+---+---+---++---+
//-     | G | S | A | B || Y |
//-     +===+===+===+===++===+
//-     | 1 | X | X | X || 0 |
//-     | 0 | 0 | 0 | X || 0 |
//-     | 0 | 0 | 1 | X || 1 |
//-     | 0 | 1 | X | 0 || 0 |
//-     | 0 | 1 | X | 1 || 1 |
//-     +---+---+---+---++---+
//-
static NETLIST_START(TTL_74157_DIP)
	NET_REGISTER_DEV(TTL_74157_GATE, A)
	NET_REGISTER_DEV(TTL_74157_GATE, B)
	NET_REGISTER_DEV(TTL_74157_GATE, C)
	NET_REGISTER_DEV(TTL_74157_GATE, D)

	NET_C(A.E, B.E, C.E, D.E)
	NET_C(A.S, B.S, C.S, D.S)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*       +--------------+      */
		  A.S, /*     S |1     ++    16| VCC  */ A.VCC,
		  A.I, /*    A1 |2           15| G    */ A.E,
		  A.J, /*    B1 |3           14| A4   */ D.I,
		  A.O, /*    Y1 |4   74157   13| B4   */ D.J,
		  B.I, /*    A2 |5           12| Y4   */ D.O,
		  B.J, /*    B2 |6           11| A3   */ C.I,
		  B.O, /*    Y2 |7           10| B3   */ C.J,
		A.GND, /*   GND |8            9| Y3   */ C.O
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74161_DIP
//- Title: DM54161/DM74161 Synchronous 4-Bit Counter
//- Description: These synchronous, presettable counters feature an internal carry look-ahead for application in high-speed counting designs.
//-   The 161 is a 4-bit binary counter.
//-   The carry output is decoded by means of a NOR gate, thus preventing spikes during the normal counting mode of operation.
//-   Synchronous operation is provided by having all flip-flops clocked simultaneously so that the outputs change co-incident with each other when so instructed by the count-enable inputs and internal gating.
//-   This mode of operation eliminates the output counting spikes which are normally associated with asynchronous (ripple clock) counters.
//-   A buffered clock input triggers the four flip-flops on the rising (positive-going) edge of the clock input waveform.
//-   These counters are fully programmable; that is, the outputs may be preset to either level.
//-   As presetting is synchronous, setting up a low level at the load input disables the counter and causes the outputs to agree with the setup data after the next clock pulse, regardless of the levels of the enable input.
//-   The clear function for the 161 is asynchronous; and a low level at the clear input sets all four of the flip-flop outputs low, regardless of the levels of clock, load, or enable inputs.
//-   The carry look-ahead circuitry provides for cascading counters for n-bit synchronous applications without additional gating.
//-   Instrumental in accomplishing this function are two count-enable inputs and a ripple carry output.
//-   Both count-enable inputs (P and T) must be high to count, and input T is fed forward to enable the ripple carry output.
//-   The ripple carry output thus enabled will produce a high-level output pulse with a duration approximately equal to the high-level portion of the QA output.
//-   This high-level overflow ripple carry pulse can be used to enable successive cascaded stages.
//-   High-to-low-level transitions at the enable P or T inputs of the 161 may occur, regardless of the logic level on the clock.
//- Pinalias: CLRQ,CLK,A,B,C,D,ENP,GND,LOADQ,ENT,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006551.PDF
//-
static NETLIST_START(TTL_74161_DIP)
	TTL_74161(A)

	DIPPINS(    /*          +--------------+          */
		A.CLRQ, /*   /CLEAR |1     ++    16| VCC      */ A.VCC,
		 A.CLK, /*    CLOCK |2           15| RC       */ A.RC,
		   A.A, /*        A |3           14| QA       */ A.QA,
		   A.B, /*        B |4   74161   13| QB       */ A.QB,
		   A.C, /*        C |5           12| QC       */ A.QC,
		   A.D, /*        D |6           11| QD       */ A.QD,
		 A.ENP, /* Enable P |7           10| Enable T */ A.ENT,
		 A.GND, /*      GND |8            9| /LOAD    */ A.LOADQ
			    /*          +--------------+          */
	)
NETLIST_END()

//- Identifier: TTL_74163_DIP
//- Title: DM74163 Synchronous 4-Bit Counter
//- Description: These synchronous, presettable counters feature an internal carry look-ahead for application in high-speed counting designs.
//-   The 163 is a 4-bit binary counter.
//-   The carry output is decoded by means of a NOR gate, thus preventing spikes during the normal counting mode of operation.
//-   Synchronous operation is provided by having all flip-flops clocked simultaneously so that the outputs change co-incident with each other when so instructed by the count-enable inputs and internal gating.
//-   This mode of operation eliminates the output counting spikes which are normally associated with asynchronous (ripple clock) counters.
//-   A buffered clock input triggers the four flip-flops on the rising (positive-going) edge of the clock input waveform.
//-   These counters are fully programmable; that is, the outputs may be preset to either level.
//-   As presetting is synchronous, setting up a low level at the load input disables the counter and causes the outputs to agree with the setup data after the next clock pulse, regardless of the levels of the enable input.
//-   The clear function for the 163 is synchronous; and a low level at the clear input sets all four of the flip-flop outputs low after the next clock pulse, regardless of the levels of the enable inputs.
//-   This synchronous clear allows the count length to be modified easily, as decoding the maximum count desired can be  accomplished with one external NAND gate.
//-   The gate output is connected to the clear input to synchronously clear the counter to all low outputs.
//-   Low-to-high transitions at the clear input of the 163 are also permissible, regardless of the logic levels on the clock,enable, or load inputs.
//-   The carry look-ahead circuitry provides for cascading counters for n-bit synchronous applications without additional gating.
//-   Instrumental in accomplishing this function are two count-enable inputs and a ripple carry output.
//-   Both count-enable inputs (P and T) must be high to count, and input T is fed forward to enable the ripple carry output.
//-   The ripple carry output thus enabled will produce a high-level output pulse with a duration approximately equal to the high-level portion of the QA output.
//-   This high-level overflow ripple carry pulse can be used to enable successive cascaded stages.
//-   High-to-low-level transitions at the enable P or T inputs of the 163 may occur, regardless of the logic level on the clock.
//- Pinalias: CLRQ,CLK,A,B,C,D,ENP,GND,LOADQ,ENT,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006551.PDF
//-
static NETLIST_START(TTL_74163_DIP)
	TTL_74163(A)

	DIPPINS(    /*          +--------------+          */
		A.CLRQ, /*   /CLEAR |1     ++    16| VCC      */ A.VCC,
		 A.CLK, /*    CLOCK |2           15| RC       */ A.RC,
		   A.A, /*        A |3           14| QA       */ A.QA,
		   A.B, /*        B |4   74163   13| QB       */ A.QB,
		   A.C, /*        C |5           12| QC       */ A.QC,
		   A.D, /*        D |6           11| QD       */ A.QD,
		 A.ENP, /* Enable P |7           10| Enable T */ A.ENT,
		 A.GND, /*      GND |8            9| /LOAD    */ A.LOADQ
			    /*          +--------------+          */
	)
NETLIST_END()

//- Identifier: TTL_74164_DIP
//- Title: DM74164 8-Bit Serial In/Parallel Out Shift Registers
//- Description: These 8-bit shift registers feature gated serial inputs and an asynchronous clear.
//-   A LOW logic level at either serial input inhibits entry of the new data, and resets the first flip-flop to the LOW level at the next clock pulse, thus providing complete control over incoming data.
//-   A HIGH logic level on either input enables the other input, which will then determine the state of the first flip-flop.
//-   Data at the serial inputs may be changed while the clock is HIGH or LOW, but only information meeting the setup and hold time requirements will be entered.
//-   Clocking occurs on the LOW-to-HIGH level transition of the clock input.
//-   All inputs are diode-clamped to minimize transmission-line effects
//- Pinalias: A,B,QA,QB,QC,QD,GND,CLOCK,CLEAR,QE,QF,QG,QH,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/fairchild/DM74164.pdf
//-
//-     +-------+-------+---+---++----+----+-----+----+
//-     | CLEAR | CLOCK | A | B || QA | QB | ... | QH |
//-     +=======+=======+===+===++====+====+=====+====+
//-     |   0   |   X   | X | X ||  0 |  0 | ... |  0 |
//-     |   1   |   0   | X | X || QA | QB | ... | QH |
//-     |   1   |  0-1  | 1 | 1 ||  1 | QA | ... | QG |
//-     |   1   |  0-1  | 0 | X ||  0 | QA | ... | QG |
//-     |   1   |  0-1  | X | 0 ||  0 | QA | ... | QG |
//-     +-------+-------+---+---++----+----+-----+----+
//-
static NETLIST_START(TTL_74164_DIP)
	TTL_74164(A)

	DIPPINS(    /*     +--------------+      */
		   A.A, /*   A |1     ++    14| VCC  */ A.VCC,
		   A.B, /*   B |2           13| QH   */ A.QH,
		  A.QA, /*  QA |3           12| QG   */ A.QG,
		  A.QB, /*  QB |4   74164   11| QF   */ A.QF,
		  A.QC, /*  QC |5           10| QE   */ A.QE,
		  A.QD, /*  QD |6            9| CLRQ */ A.CLRQ,
		 A.GND, /* GND |7            8| CLK  */ A.CLK
			    /*     +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74165_DIP
//- Title: 54165/DM74165 8-Bit Parallel-to-Serial Converter
//- Description: The ’165 is an 8-bit parallel load or serial-in register with complementary outputs available from the last stage.
//-   Parallel inputting occurs asynchronously when the Parallel Load (PL) input is LOW.
//-   With PL HIGH, serial shifting occurs on the rising edge of the clock; new data enters via the Serial Data (DS) input.
//-   The 2-input OR clock can be used to combine two independent clock sources, or one input can act as an active LOW clock enable.
//- Pinalias: PLQ,CP1,P4,P5,P6,P7,QQ7,GND,Q7,DS,P0,P1,P2,P3,CP2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009782.PDF
//-
//-     +-----+-----+-----++----+----+----+----+----+----+----+----+
//-     | PLQ | CP1 | CP2 || Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 | Q7 |
//-     +=====+=====+=====++====+====+====+====+====+====+====+====+
//-     |  0  |  X  |  X  || P0 | P1 | P2 | P3 | P4 | P5 | P6 | P7 |
//-     |  1  |  0  | 0-1 || DS | Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 |
//-     |  1  |  1  | 0-1 || Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 | Q7 |
//-     |  1  | 0-1 |  0  || DS | Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 |
//-     |  1  | 0-1 |  1  || Q0 | Q1 | Q2 | Q3 | Q4 | Q5 | Q6 | Q7 |
//-     +-----+-----+-----++----+----+----+----+----+----+----+----+
//-
static NETLIST_START(TTL_74165_DIP)
	TTL_74165(A)

	DIPPINS(      /*      +--------------+      */
		A.SH_LDQ, /*  PLQ |1     ++    16| VCC  */ A.VCC,
		   A.CLK, /*  CP1 |2           15| CP2  */ A.CLKINH,
		     A.E, /*   P4 |3           14| P3   */ A.D,
		     A.F, /*   P5 |4    74165  13| P2   */ A.C,
		     A.G, /*   P6 |5           12| P1   */ A.B,
		     A.H, /*   P7 |6           11| P0   */ A.A,
		   A.QHQ, /*  QQ7 |7           10| DS   */ A.SER,
		   A.GND, /*  GND |8            9| Q7   */ A.QH
			      /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74166_DIP
//- Title: DM74LS166 8-Bit Parallel-In/Serial-Out Shift Registers
//- Description: These parallel-in or serial-in, serial-out shift registers feature gated clock inputs and an overriding clear input.
//-   All inputs are buffered to lower the drive requirements to one normalized load, and input clamping diodes minimize switching transients to simplify system design.
//-   The load mode is established by the shift/load input.
//-   When high, this input enables the serial data input and couples the eight flip-flops for serial shifting with each clock pulse.
//-   When low, the parallel (broadside) data inputs are enabled and synchronous loading occurs on the next clock pulse.
//-   During parallel loading, serial data flow is inhibited.
//-   Clocking is accomplished on the low-to-high-level edge of the clock pulse through a two-input NOR gate, permitting one input to be used as a clock-enable or clock-inhibit function.
//-   Holding either of the clock inputs high inhibits clocking; holding either low enables the other clock input.
//-   This allows the system clock to be free running, and the register can be stopped on command with the other clock input.
//-   The clock-inhibit input should be changed to the high level only while the clock input is high.
//-   A buffered, direct clear input overrides all other inputs, including the clock, and sets all flip-flops to zero.
//- Pinalias: SER,A,B,C,D,CLKINH,CLK,GND,CLRQ,E,F,G,QH,H,SH/LDQ,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006400.PDF
//-
//-     +------+--------+--------+-----+-----+------++----+----+-----+----+
//-     | CLRQ | SH/LDQ | CLKINH | CLK | SER | A..H || QA | QB | ... | QH |
//-     +======+========+========+=====+=====+======++====+====+=====+====+
//-     |   0  |    X   |    X   |  X  |  X  |   X  ||  0 |  0 | ... |  0 |
//-     |   1  |    X   |    0   |  0  |  X  |   X  || QA | QB | ... | QH |
//-     |   1  |    0   |    0   | 0-1 |  X  | a..h ||  a |  b | ... |  h |
//-     |   1  |    1   |    0   | 0-1 |  1  |   X  ||  1 | QA | ... | QG |
//-     |   1  |    1   |    0   | 0-1 |  0  |   X  ||  0 | QA | ... | QG |
//-     |   1  |    X   |    1   | 0-1 |  X  |   X  || QA | QB | ... | QH |
//-     +------+--------+--------+-----+-----+------++----+----+-----+----+
//-
static NETLIST_START(TTL_74166_DIP)
	TTL_74166(A)

	DIPPINS(      /*        +--------------+        */
		   A.SER, /*    SER |1     ++    16| VCC    */ A.VCC,
		     A.A, /*      A |2           15| SH/LDQ */ A.SH_LDQ,
		     A.B, /*      B |3           14| H      */ A.H,
		     A.C, /*      C |4    74166  13| QH     */ A.QH,
		     A.D, /*      D |5           12| G      */ A.G,
		A.CLKINH, /* CLKINH |6           11| F      */ A.F,
		   A.CLK, /*    CLK |7           10| E      */ A.E,
		   A.GND, /*    GND |8            9| CLRQ   */ A.CLRQ
			      /*        +--------------+        */
	)
NETLIST_END()

//- Identifier: TTL_74174_DIP
//- Title: DM74174 Hex/Quad D-Type Flip-Flop with Clear
//- Description: These positive-edge triggered flip-flops utilize TTL circuitry to implement D-type flip-flop logic.
//-   All have a direct clear input.
//-   Information at the D inputs meeting the setup and hold time requirements is transferred to the Q outputs on the positive-going edge of the clock pulse.
//-   Clock triggering occurs at a particular voltage level and is not directly related to the transition time of the positive-going pulse.
//-   When the clock input is at either the HIGH or LOW level, the D input signal has no effect at the output.
//- Pinalias: CLRQ,Q1,D1,D2,Q2,D3,Q3,GND,CLK,Q4,D4,Q5,D5,Q6,D6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/fairchild/DM74174.pdf
//-
//-     +------+-----+---++---+
//-     | CLRQ | CLK | D || Q |
//-     +======+=====+===++===+
//-     |   0  |  X  | X || 0 |
//-     |   1  | 0-1 | 1 || 1 |
//-     |   1  | 0-1 | 0 || 0 |
//-     |   1  |  0  | X || Q |
//-     +------+-----+---++---+
//-
static NETLIST_START(TTL_74174_DIP)
	TTL_74174_GATE(A)
	TTL_74174_GATE(B)
	TTL_74174_GATE(C)
	TTL_74174_GATE(D)
	TTL_74174_GATE(E)
	TTL_74174_GATE(F)

	DIPPINS(    /*      +--------------+      */
		A.CLRQ, /* CLRQ |1     ++    16| VCC  */ A.VCC,
		   A.Q, /*   Q1 |2           15| Q6   */ F.Q,
		   A.D, /*   D1 |3           14| D6   */ F.D,
		   B.D, /*   D2 |4   74174   13| D5   */ E.D,
		   B.Q, /*   Q2 |5           12| Q5   */ E.Q,
		   C.D, /*   D3 |6           11| D4   */ D.D,
		   C.Q, /*   Q3 |7           10| Q4   */ D.Q,
		 A.GND, /*  GND |8            9| CLK  */ A.CLK
			    /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74260_DIP
//- Title: DM54LS260/DM74LS260 Dual 5-Input NOR Gate
//- Description: This device contains two individual five input gates, each of which perform the logic NOR function.
//- Pinalias: A1,B1,C1,A2,Q1,Q2,GND,B2,C2,D2,E2,D1,E1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009824.PDF
//-
//-     +---+---+---+---+---++---+
//-     | A | B | C | D | E || Y |
//-     +===+===+===+===+===++===+
//-     | 0 | 0 | 0 | 0 | 0 || 1 |
//-     | X | X | X | X | 1 || 0 |
//-     | X | X | X | 1 | X || 0 |
//-     | X | X | 1 | X | X || 0 |
//-     | X | 1 | X | X | X || 0 |
//-     | 1 | X | X | X | X || 0 |
//-     +---+---+---+---+---++---+
//-
static NETLIST_START(TTL_74260_DIP)
	TTL_74260_GATE(A)
	TTL_74260_GATE(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*       +--------------+      */
		  A.C, /*    C1 |1     ++    14| VCC  */ A.VCC,
		  A.D, /*    D1 |2           13| B1   */ A.B,
		  A.E, /*    E1 |3           12| A1   */ A.A,
		  B.E, /*    E2 |4   74260   11| D2   */ B.D,
		  A.Q, /*    Y1 |5           10| C2   */ B.C,
		  B.Q, /*    Y2 |6            9| B2   */ B.B,
		A.GND, /*   GND |7            8| A2   */ B.A
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74279_DIP
//- Title: 54279/DM74279 Quad Set-Reset Latch
//- Description: These latches are ideaily suited ·for use as temporary
//-   storage of bfnary information between processing units
//-   and I/O units. When either one of the data inputs is at
//-   a low logic level, the output will follow the level of the
//-   R input. When both data inputs are high, the output will
//-   remain latched in its previous state. When both inputs
//-   are low, the output will .go high. However, this high
//-   level may not persist when either one of the data inputs
//-   returns to the high state.
//- Pinalias: 1RQ,1S1Q,1S2Q,1Q,2RQ,2SQ,2Q,GND,3Q,3RQ,3S1Q,3S2Q,4Q,4RQ,4SQ,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009785.PDF
//-
//-     +-----+-----+----++---+
//-     | S1Q | S2Q | RQ || Q |
//-     +=====+=====+====++===+
//-     |  0  |  0  |  0 || 1 | (as long as SQ1 or SQ2 are low)
//-     |  0  |  X  |  1 || 1 |
//-     |  X  |  0  |  1 || 1 |
//-     |  1  |  1  |  0 || 0 |
//-     |  1  |  1  |  1 || Q |
//-     +-----+-----+----++---+
//-
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

	DIPPINS(   /*      +--------------+      */
		  A.R, /*  1RQ |1     ++    16| VCC  */ A.VCC,
		 A.S1, /* 1S1Q |2           15| 4SQ  */ D.S,
		 A.S2, /* 1S2Q |3           14| 4RQ  */ D.R,
		  A.Q, /*   1Q |4    74279  13| 4Q   */ D.Q,
		  B.R, /*  2RQ |5           12| 3S2Q */ C.S2,
		  B.S, /*  2SQ |6           11| 3S1Q */ C.S1,
		  B.Q, /*   2Q |7           10| 3RQ  */ C.R,
		A.GND, /*  GND |8            9| 3Q   */ C.Q
			   /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74377_DIP
//- Title: DM54LS377/DM74LS377 Octal D Flip-Flop with Common Enable and Clock
//- Description: The ’LS377 is an 8-bit register built using advanced low power Schottky technology.
//-   This register consists of eight D-type flip-flops with a buffered common clock and a buffered common input enable.
//-   The device is packaged in the space-saving (0.3 inch row spacing) 20-pin package.
//- Pinalias: EQ,Q0,D0,D1,Q1,Q2,D2,D3,Q3,GND,CP,Q4,D4,D5,Q5,Q6,D6,D7,Q7,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009785.PDF
//-
//-     +----+-----+----++----+
//-     | EQ |  CP | Dn || Qn |
//-     +====+=====+====++====+
//-     |  1 |  X  |  X || Qn |
//-     |  0 | 0-1 |  1 ||  1 |
//-     |  0 | 0-1 |  0 ||  0 |
//-     +----+-----+----++----+
//-
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

	DIPPINS(   /*       +--------------+      */
		  A.E, /*    EQ |1     ++    20| VCC  */ A.VCC,
		  A.Q, /*    Q0 |2           19| Q7   */ H.Q,
		  A.D, /*    D0 |3           18| D7   */ H.D,
		  B.D, /*    D1 |4   74377   17| D6   */ G.D,
		  B.Q, /*    Q1 |5           16| Q6   */ G.Q,
		  C.Q, /*    Q2 |6           15| Q5   */ F.Q,
		  C.D, /*    D2 |7           14| D5   */ F.D,
		  D.D, /*    D3 |8           13| D4   */ E.D,
		  D.Q, /*    Q3 |9           12| Q4   */ E.Q,
		A.GND, /*   GND |10          11| CP   */ A.CP
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74378_DIP
//- Title: DM54LS378/DM74LS378 Parallel D Register with Enable
//- Description: The ’LS378 is a 6-bit register with a buffered common enable.
//-   This device is similar to the ’LS174, but with common Enable rather than common Master Reset.
//- Pinalias: EQ,Q0,D0,D1,Q1,D2,Q2,GND,CP,Q3,D3,Q4,D4,D5,Q6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009832.PDF
//-
//-     +----+-----+----++----+
//-     | EQ |  CP | Dn || Qn |
//-     +====+=====+====++====+
//-     |  1 |  X  |  X || Qn |
//-     |  0 | 0-1 |  1 ||  1 |
//-     |  0 | 0-1 |  0 ||  0 |
//-     +----+-----+----++----+
//-
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

	DIPPINS(   /*       +--------------+      */
		  A.E, /*    EQ |1     ++    16| VCC  */ A.VCC,
		  A.Q, /*    Q0 |2           15| Q5   */ F.Q,
		  A.D, /*    D0 |3           14| D5   */ F.D,
		  B.D, /*    D1 |4   74378   13| D4   */ E.D,
		  B.Q, /*    Q1 |5           12| Q4   */ E.Q,
		  C.D, /*    D2 |6           11| D3   */ D.D,
		  C.Q, /*    Q2 |7           10| Q3   */ D.Q,
		A.GND, /*   GND |8            9| CP   */ A.CP
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74379_DIP
//- Title: 54LS379/DM74LS379 Quad Parallel Register with Enable
//- Description: The LS379 is a 4-bit register with buffered common Enable.
//-   This device is similar to the LS175 but features the common Enable rather than common Master Reset.
//- Pinalias: EEQ,Q0,QQ0,D0,QQ1,Q1,GND,CP,Q2,QQ2,D2,D3,QQ3,Q3,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009832.PDF
//-
//-     +----+-----+----++----+-----+
//-     | EQ |  CP | Dn || Qn | QQn |
//-     +====+=====+====++====+=====+
//-     |  1 |  X  |  X || Qn | QQn |
//-     |  0 | 0-1 |  1 ||  1 |  0  |
//-     |  0 | 0-1 |  0 ||  0 |  1  |
//-     +----+-----+----++----+-----+
//-
static NETLIST_START(TTL_74379_DIP)
	TTL_74377_GATE(A)
	TTL_74377_GATE(B)
	TTL_74377_GATE(C)
	TTL_74377_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)
	NET_C(A.CP, B.CP, C.CP, D.CP)
	NET_C(A.E, B.E, C.E, D.E)

	DIPPINS(   /*       +--------------+      */
		  A.E, /*    EQ |1     ++    16| VCC  */ A.VCC,
		  A.Q, /*    Q0 |2           15| Q3   */ D.Q,
		 A.QQ, /*   QQ0 |3           14| QQ3  */ D.QQ,
		  A.D, /*    D0 |4   74379   13| D3   */ D.D,
		  B.D, /*    D1 |5           12| D2   */ C.D,
		 B.QQ, /*   QQ1 |6           11| QQ2  */ C.QQ,
		  B.Q, /*    Q1 |7           10| Q2   */ C.Q,
		A.GND, /*   GND |8            9| CP   */ A.CP
			   /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: TTL_74393_DIP
//- Title: Dual 4-Bit Binary Counter
//- Description: DM74LS393 Each of these monolithic circuits contains eight master-slave flip-flops and additional gating to implement two individual four-bit counters in a single package.
//-   The ’LS393 comprises two independent four-bit binary counters each having a clear and a clock input.
//-   N-bit binary counters can be implemented with each package providing the capability of divide-by-256.
//-   The LS393 has parallel outputs from each counter stage so that any submultiple of the input count freqency is available for system-timing signals.
//- Pinalias: 1A,1CLR,1QA,1QB,1QC,1QD,GND,2QD,2QC,2QB,2QA,2CLR,2A,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006434.PDF
//-
static NETLIST_START(TTL_74393_DIP)
	TTL_74393(A)
	TTL_74393(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(   /*      +--------------+      */
		 A.CP, /*   1A |1    ++     14| VCC  */ A.VCC,
		 A.MR, /* 1CLR |2           13| 2A   */ B.CP,
		 A.Q0, /*  1QA |3           12| 2CLR */ B.MR,
		 A.Q1, /*  1QB |4   74393   11| 2QA  */ B.Q0,
		 A.Q2, /*  1QC |5           10| 2QB  */ B.Q1,
		 A.Q3, /*  1QD |6            9| 2QC  */ B.Q2,
		A.GND, /*  GND |7            8| 2QD  */ B.Q3
			   /*      +--------------+      */
	)
NETLIST_END()

//- Identifier: SN74LS629_DIP
//- Title: SN74LS629 VOLTAGE-CONTROLLED OSCILLATORS
//- Description: Please add a detailed description
//-    FIXME: Missing description
//-
//- Pinalias: 2FC,1FC,1RNG,1CX1,1CX2,1ENQ,1Y,OSC_GND,GND,2Y,2ENQ,2CX2,2CX1,2RNG,OSC_VCC,VCC
//- Package: DIP
//- Param: A.CAP
//-    Capacitor value of capacitor connected to 1CX1 and 1CX2 pins
//- Param: B.CAP
//-    Capacitor value of capacitor connected to 2CX1 and 2CX2 pins
//- Limitations:
//-    The capacitor inputs are NC. Capacitor values need to be specified as
//-    ```
//-    SN74LS629_DIP(X)
//-    PARAM(X.A.CAP, CAP_U(1))
//-    PARAM(X.B.CAP, CAP_U(2))
//-    ```
//-
//- Example: 74ls629.cpp,74ls629_example
//-
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/400/335051_DS.pdf
//-
static NETLIST_START(SN74LS629_DIP)
	SN74LS629(A, CAP_U(1))
	SN74LS629(B, CAP_U(1))

	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
	NET_C(A.OSCGND, B.OSCGND)
	NET_C(A.OSCVCC, B.OSCVCC)
	NC_PIN(NC)

	DIPPINS(   /*          +--------------+         */
		 B.FC, /*      2FC |1     ++    16| VCC     */ A.VCC,
		 A.FC, /*      1FC |2           15| OSC VCC */ A.OSCVCC,
		A.RNG, /*     1RNG |3           14| 2RNG    */ B.RNG,
		 NC.I, /*     1CX1 |4  74LS629  13| 2CX1    */ NC.I,
		 NC.I, /*     1CX2 |5           12| 2CX2    */ NC.I,
		A.ENQ, /*     1ENQ |6           11| 2ENQ    */ B.ENQ,
		  B.Y, /*       1Y |7           10| 2Y      */ B.Y,
	 A.OSCGND, /*  OSC GND |8            9| GND     */ A.GND
			   /*          +--------------+         */
	)
NETLIST_END()

//- Identifier: TTL_9310_DIP
//- Title: DM9310/DM8310 Synchronous 4-Bit Decade Counter
//- Description: These synchronous, presettable counters feature an
//-   internal carry look-ahead for application in high-speed
//-   counting designs. The DM9310/DM8310 are decade
//-   counters. The carry output is decoded by means of a
//-   NOR gate, thus preventing spikes during the normal
//-   counting mode of operation. Synchronous operation
//-   is provided by having all flip-flops clocked simultaneously
//-   so that the outputs change coincident with each
//-   other when so instructed by the count-enable inputs
//-   and internal gating. This mode of operation eliminates
//-   the output counting spikes which are normally
//-   associated with asynchronous (ripple clock) counters.
//-   A buffered clock input triggers the four flip-flops on
//-   the rising (positive-going) edge of the clock input
//-   waveform.
//-
//-   These counters are fully programmable; that is, the
//-   outputs may be preset to either level. As presetting is
//-   synchronous, setting up a low level at the load input
//-   disables the counter and causes the outputs to agree
//-   with the setup data after the next clock pulse regardless
//-   of the levels of the enable input. Low-to-high transitions
//-   at the· load input are perfectly acceptable regardless of
//-   the logic levels on the clock or enable inputs. The clear
//-   function is asynchronous and a low level at the clear
//-   input sets all four of the flip-flop outputs low regardless
//-   of the levels of clock, load, or enable inputs.
//-
//-   The carry look-ahead circuitry provides for cascading
//-   counters for n-bit synchronous applications without
//-   additional gating. Instrumental in accomplishing this
//-   function are two count-enable inputs and a ripple carry
//-   output. Both count-enable inputs (P and T) must be
//-   high to count, and input T is fed-forward to enable the
//-   ripple carry output. The ripple carry output thus
//-   enabled will produce a high-level output pulse with a
//-   duration approximately equal to the high-level portion
//-   of the QA output. This high-level overflow ripply carry
//-   pulse can be used to enable successive cascaded stages.
//-   High-to-Low level transitions at the enable P or T inputs
//-   may occur regardless of the logic level in the clock.
//- Pinalias: CLEARQ,CLOCK,A,B,C,D,ENABLEP,GND,LOADQ,ENABLET,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    https://ia800608.us.archive.org/5/items/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
//-
static NETLIST_START(TTL_9310_DIP)
	TTL_9310(A)

	DIPPINS(    /*          +--------------+          */
		A.CLRQ, /*   /CLEAR |1     ++    16| VCC      */ A.VCC,
		 A.CLK, /*    CLOCK |2           15| RC       */ A.RC,
		   A.A, /*        A |3           14| QA       */ A.QA,
		   A.B, /*        B |4    9310   13| QB       */ A.QB,
		   A.C, /*        C |5           12| QC       */ A.QC,
		   A.D, /*        D |6           11| QD       */ A.QD,
		 A.ENP, /* Enable P |7           10| Enable T */ A.ENT,
		 A.GND, /*      GND |8            9| /LOAD    */ A.LOADQ
			    /*          +--------------+          */
	)
NETLIST_END()

//- Identifier: TTL_9312_DIP
//- Title: DM9312/DM8312 One of Eight Line Data Selectors/Multiplexers
//- Description: These data selectors/multiplexers contain inverter/
//-   drivers to supply full complementary, on-chip, binary
//-   decoded data selection to the AND-OR-INVERT gates.
//-
//-   The DM9312/8312 is a single 8-bit multiplexer with
//-   complementary outputs and a strobe control. When the
//-   strobe is low, the function is enabled. When a high logic
//-   level is applied to the strobe, the outputs are latched.
//- Pinalias: D0,D1,D2,D3,D4,D5,D6,GND,D7,G,A,B,C,YQ,Y,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DM9312.pdf
//-
//-     +---+---+---+---++----+-----+
//-     | C | B | A | G ||  Y |  YQ |
//-     +===+===+===+===++====+=====+
//-     | X | X | X | 1 ||  0 |  1  |
//-     | 0 | 0 | 0 | 0 || D0 | D0Q |
//-     | 0 | 0 | 1 | 0 || D1 | D1Q |
//-     | 0 | 1 | 0 | 0 || D2 | D2Q |
//-     | 0 | 1 | 1 | 0 || D3 | D3Q |
//-     | 1 | 0 | 0 | 0 || D4 | D4Q |
//-     | 1 | 0 | 1 | 0 || D5 | D5Q |
//-     | 1 | 1 | 0 | 0 || D6 | D6Q |
//-     | 1 | 1 | 1 | 0 || D7 | D7Q |
//-     +---+---+---+---++----+-----+
//-
static NETLIST_START(TTL_9312_DIP)
	TTL_9312(A)

	DIPPINS(   /*     +--------------+     */
		 A.D0, /*  D0 |1     ++    16| VCC */ A.VCC,
		 A.D1, /*  D1 |2           15| Y   */ A.Y,
		 A.D2, /*  D2 |3           14| YQ  */ A.YQ,
		 A.D3, /*  D3 |4    9312   13| C   */ A.C,
		 A.D4, /*  D4 |5           12| B   */ A.B,
		 A.D5, /*  D5 |6           11| A   */ A.A,
		 A.D6, /*  D6 |7           10| G   */ A.G, //Strobe
		A.GND, /* GND |8            9| D7  */ A.D7
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: TTL_9316_DIP
//- Title: DM9316/DM8316 Synchronous 4-Bit Counters
//- Description: These synchronous, presettable counters feature an
//-   internal carry look-ahead for application in high-speed
//-   counting designs. The DM9316/DM8316 are 4-bit binary
//-   counters. The carry output is decoded by means of a
//-   NOR gate, thus preventing spikes during the normal
//-   counting mode of operation. Synchronous operation
//-   is provided by having all flip-flops clocked simultaneously
//-   so that the outputs change coincident with each
//-   other when so instructed by the count-enable inputs
//-   and internal gating. This mode of operation eliminates
//-   the output counting spikes which are normally
//-   associated with asynchronous (ripple clock) counters.
//-   A buffered clock input triggers the four flip-flops on
//-   the rising (positive-going) edge of the clock input
//-   waveform.
//-
//-   These counters are fully programmable; that is, the
//-   outputs may be preset to either level. As presetting is
//-   synchronous, setting up a low level at the load input
//-   disables the counter and causes the outputs to agree
//-   with the setup data after the next clock pulse regardless
//-   of the levels of the enable input. Low-to-high transitions
//-   at the· load input are perfectly acceptable regardless of
//-   the logic levels on the clock or enable inputs. The clear
//-   function is asynchronous and a low level at the clear
//-   input sets all four of the flip-flop outputs low regardless
//-   of the levels of clock, load, or enable inputs.
//-
//-   The carry look-ahead circuitry provides for cascading
//-   counters for n-bit synchronous applications without
//-   additional gating. Instrumental in accomplishing this
//-   function are two count-enable inputs and a ripple carry
//-   output. Both count-enable inputs (P and T) must be
//-   high to count, and input T is fed-forward to enable the
//-   ripple carry output. The ripple carry output thus
//-   enabled will produce a high-level output pulse with a
//-   duration approximately equal to the high-level portion
//-   of the QA output. This high-level overflow ripply carry
//-   pulse can be used to enable successive cascaded stages.
//-   High-to-Low level transitions at the enable P or T inputs
//-   may occur regardless of the logic level in the clock.
//- Pinalias: CLEARQ,CLOCK,A,B,C,D,ENABLEP,GND,LOADQ,ENABLET,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006606.PDF
//-
static NETLIST_START(TTL_9316_DIP)
	TTL_9316(A)

	DIPPINS(    /*          +--------------+          */
		A.CLRQ, /*   CLEARQ |1     ++    16| VCC      */ A.VCC,
		 A.CLK, /*    CLOCK |2           15| RC       */ A.RC,
		   A.A, /*        A |3           14| QA       */ A.QA,
		   A.B, /*        B |4    9316   13| QB       */ A.QB,
		   A.C, /*        C |5           12| QC       */ A.QC,
		   A.D, /*        D |6           11| QD       */ A.QD,
		 A.ENP, /* Enable P |7           10| Enable T */ A.ENT,
		 A.GND, /*      GND |8            9| LOADQ    */ A.LOADQ
			    /*          +--------------+          */
	)
NETLIST_END()

//- Identifier: TTL_9322_DIP
//- Title: DM9322/DM8322 Quad 2-Line to 1-Line Data Selectors/Multiplexers
//- Description: These data selectors/multiplexers contain inverters and
//-   drivers to supply full on-chip data selection to the four
//-   output gates. A separate strobe input is provided. A
//-   4-bit word is selected from one of two sources and is
//-   routed to the four outputs. True data is presented
//-   at the outputs.
//- Pinalias: SELECT,A1,B1,Y1,A2,B2,Y2,GND,Y3,B3,A3,Y4,B4,A4,STROBE,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006606.PDF
//-
static NETLIST_START(TTL_9322_DIP)
	TTL_9322_GATE(A)
	TTL_9322_GATE(B)
	TTL_9322_GATE(C)
	TTL_9322_GATE(D)

	NET_C(A.SELECT, B.SELECT, C.SELECT, D.SELECT)
	NET_C(A.STROBE, B.STROBE, C.STROBE, D.STROBE)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(      /*        +--------------+        */
		A.SELECT, /* SELECT |1     ++    16| VCC    */ A.VCC,
		     A.A, /*     A1 |2           15| STROBE */ A.STROBE,
		     A.B, /*     B1 |3           14| A4     */ D.A,
		     A.Y, /*     Y1 |4    9322   13| B4     */ D.B,
		     B.A, /*     A2 |5           12| Y4     */ D.Y,
		     B.B, /*     B2 |6           11| A3     */ C.A,
		     B.Y, /*     Y2 |7           10| B3     */ C.B,
		   A.GND, /*    GND |8            9| Y3     */ C.Y
			      /*        +--------------+        */
	)
NETLIST_END()

//- Identifier: TTL_9602_DIP
//- Title: DM9602/DM6802 Dual Retriggerable, Resettable One Shots
//- Description: These dual resettable, retriggerable one shots have two
//-   inputs per function; one which is active high, and one
//-   which is active low. This allows the designer to employ
//-   either leading-edge or trailing-edge triggering, which is
//-   independent of input transition times. When input con·
//-   ditions for triggering are met, a new cycle starts and the
//-   external capacitor is allowed to rapidly discharge and
//-   then charge again. The retriggerable feature permits
//-   output pulse widths to be extended. In fact a continuous
//-   true output can be maintained by having an input cycle
//-   time which is shorter than the output cycle time. The
//-   output pulse may then be terminated at any time by
//-   applying a low logic level to the RESET pin. Retriggering
//-   may be inhibited by either connecting the Q output to
//-   an active high input, or the Q output to an active
//-   low input.
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

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(    /*       +--------------+ */
		   A.C, /*    C1 |1     ++    16| VCC   */ A.VCC,
		  A.RC, /*   RC1 |2           15| C2    */ B.C,
		A.CLRQ, /* CLRQ1 |3           14| RC2   */ B.RC,
		   A.B, /*    B1 |4    9602   13| CLRQ2 */ B.CLRQ,
		   A.A, /*    A1 |5           12| B2    */ B.B,
		   A.Q, /*    Q1 |6           11| A2    */ B.A,
		  A.QQ, /*   QQ1 |7           10| Q2    */ B.Q,
		 A.GND, /*   GND |8            9| QQ2   */ B.QQ
			    /*       +--------------+       */
	)
NETLIST_END()


NETLIST_START(ttl74xx_lib)
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
	TRUTHTABLE_START(TTL_74107_TT, 6, 4, "+CLK,+J,+K,+CLRQ,@VCC,@GND")
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

	TRUTHTABLE_START(TTL_9312, 12, 2, "+A,+B,+C,+G,+D0,+D1,+D2,+D3,+D4,+D5,+D6,+D7,@VCC,@GND")
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
	LOCAL_LIB_ENTRY(TTL_7483_DIP)
	LOCAL_LIB_ENTRY(TTL_7485_DIP)
	LOCAL_LIB_ENTRY(TTL_7486_DIP)
	LOCAL_LIB_ENTRY(TTL_7490_DIP)
	LOCAL_LIB_ENTRY(TTL_7492_DIP)
	LOCAL_LIB_ENTRY(TTL_7493_DIP)
	LOCAL_LIB_ENTRY(TTL_7497_DIP)
	LOCAL_LIB_ENTRY(TTL_74107_DIP)
	LOCAL_LIB_ENTRY(TTL_74107A_DIP)
	LOCAL_LIB_ENTRY(TTL_74113_DIP)
	LOCAL_LIB_ENTRY(TTL_74113A_DIP)
	LOCAL_LIB_ENTRY(TTL_74121_DIP)
	LOCAL_LIB_ENTRY(TTL_74123_DIP)
	LOCAL_LIB_ENTRY(TTL_9602_DIP)
	LOCAL_LIB_ENTRY(TTL_74125_DIP)
	LOCAL_LIB_ENTRY(TTL_74126_DIP)
	LOCAL_LIB_ENTRY(TTL_74153_DIP)
	LOCAL_LIB_ENTRY(TTL_74155_DIP)
	LOCAL_LIB_ENTRY(TTL_74156_DIP)
	LOCAL_LIB_ENTRY(TTL_74157_DIP)
	LOCAL_LIB_ENTRY(TTL_74161_DIP)
	LOCAL_LIB_ENTRY(TTL_74163_DIP)
	LOCAL_LIB_ENTRY(TTL_74164_DIP)
	LOCAL_LIB_ENTRY(TTL_74165_DIP)
	LOCAL_LIB_ENTRY(TTL_74166_DIP)
	LOCAL_LIB_ENTRY(TTL_74174_DIP)
	LOCAL_LIB_ENTRY(TTL_74260_DIP)
	LOCAL_LIB_ENTRY(TTL_74279_DIP)
	LOCAL_LIB_ENTRY(TTL_74377_DIP)
	LOCAL_LIB_ENTRY(TTL_74378_DIP)
	LOCAL_LIB_ENTRY(TTL_74379_DIP)
	LOCAL_LIB_ENTRY(TTL_74393_DIP)
	LOCAL_LIB_ENTRY(SN74LS629_DIP)
	LOCAL_LIB_ENTRY(TTL_9312_DIP)
	LOCAL_LIB_ENTRY(TTL_9310_DIP)
	LOCAL_LIB_ENTRY(TTL_9316_DIP)
	LOCAL_LIB_ENTRY(TTL_9322_DIP)
NETLIST_END()
