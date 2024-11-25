// license:CC0-1.0
// copyright-holders:Couriersud

//#rewritten with sed -e "s_^\(.*\)/\*\(.*\)\*/\(.*\)\$_\1\3 // \2_g"  ../macro/nlm_ttl74xx_lib.cpp

#include "devices/net_lib.h"

//- Identifier: TTL_7400_DIP
//- Title: 5400/DM5400/DM7400 Quad 2-Input NAND Gates
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
{
	TTL_7400_NAND(A)
	TTL_7400_NAND(B)
	TTL_7400_NAND(C)
	TTL_7400_NAND(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //     +--------------+
		A.A, A.VCC, //  A1 |1     ++    14| VCC
		A.B, D.B,   //  B1 |2           13| B4
		A.Q, D.A,   //  Y1 |3           12| A4
		B.A, D.Q,   //  A2 |4    7400   11| Y4
		B.B, C.B,   //  B2 |5           10| B3
		B.Q, C.A,   //  Y2 |6            9| A3
		A.GND, C.Q) // GND |7            8| Y3
					//     +--------------+
}

//- Identifier: TTL_7402_DIP
//- Title: 5402/DM5402/DM7402 Quad 2-Input NOR Gates
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
{
	TTL_7402_NOR(A)
	TTL_7402_NOR(B)
	TTL_7402_NOR(C)
	TTL_7402_NOR(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.Q, A.VCC, //     Y1 |1     ++    14| VCC
		A.A, D.Q,   //     A1 |2           13| Y4
		A.B, D.B,   //     B1 |3           12| B4
		B.Q, D.A,   //     Y2 |4    7402   11| A4
		B.A, C.Q,   //     A2 |5           10| Y3
		B.B, C.B,   //     B2 |6            9| B3
		A.GND, C.A  //    GND |7            8| A3
					//        +--------------+
	)
}

//- Identifier: TTL_7404_DIP
//- Title: 5404/DM5404/DM7404 Hex Inverting Gates
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
{
	TTL_7404_INVERT(A)
	TTL_7404_INVERT(B)
	TTL_7404_INVERT(C)
	TTL_7404_INVERT(D)
	TTL_7404_INVERT(E)
	TTL_7404_INVERT(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Q, F.A,   //     Y1 |2           13| A6
		B.A, F.Q,   //     A2 |3           12| Y6
		B.Q, E.A,   //     Y2 |4    7404   11| A5
		C.A, E.Q,   //     A3 |5           10| Y5
		C.Q, D.A,   //     Y3 |6            9| A4
		A.GND, D.Q  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7406_DIP
//- Title: DM5406/DM7406 Hex Inverting Buffers with High Voltage Open-Collector Outputs
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
{
	TTL_7406_GATE(A)
	TTL_7406_GATE(B)
	TTL_7406_GATE(C)
	TTL_7406_GATE(D)
	TTL_7406_GATE(E)
	TTL_7406_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Y, F.A,   //     Y1 |2           13| A6
		B.A, F.Y,   //     A2 |3           12| Y6
		B.Y, E.A,   //     Y2 |4    7406   11| A5
		C.A, E.Y,   //     A3 |5           10| Y5
		C.Y, D.A,   //     Y3 |6            9| A4
		A.GND, D.Y  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7407_DIP
//- Title: DM5407/DM7407 Hex Buffers with High Voltage Open-Collector Outputs
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
{
	TTL_7407_GATE(A)
	TTL_7407_GATE(B)
	TTL_7407_GATE(C)
	TTL_7407_GATE(D)
	TTL_7407_GATE(E)
	TTL_7407_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Y, F.A,   //     Y1 |2           13| A6
		B.A, F.Y,   //     A2 |3           12| Y6
		B.Y, E.A,   //     Y2 |4    7407   11| A5
		C.A, E.Y,   //     A3 |5           10| Y5
		C.Y, D.A,   //     Y3 |6            9| A4
		A.GND, D.Y  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7408_DIP
//- Title: 5408/DM5408/DM7408 Quad 2-Input AND Gates
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
{
	TTL_7408_AND(A)
	TTL_7408_AND(B)
	TTL_7408_AND(C)
	TTL_7408_AND(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, D.B,   //     B1 |2           13| B4
		A.Q, D.A,   //     Y1 |3           12| A4
		B.A, D.Q,   //     A2 |4    7408   11| Y4
		B.B, C.B,   //     B2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7410_DIP
//- Title: 5410/DM5410/DM7410 Triple 3-Input NAND Gates
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
{
	TTL_7410_NAND(A)
	TTL_7410_NAND(B)
	TTL_7410_NAND(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, A.C,   //     B1 |2           13| C1
		B.A, A.Q,   //     A2 |3           12| Y1
		B.B, C.C,   //     B2 |4    7410   11| C3
		B.C, C.B,   //     C2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7411_DIP
//- Title: DM7411 Triple 3-Input AND Gate
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
{
	TTL_7411_AND(A)
	TTL_7411_AND(B)
	TTL_7411_AND(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, A.C,   //     B1 |2           13| C1
		B.A, A.Q,   //     A2 |3           12| Y1
		B.B, C.C,   //     B2 |4    7411   11| C3
		B.C, C.B,   //     C2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7414_DIP
//- Title: DM5414/DM7414 Hex Inverter withSchmitt Trigger Inputs
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
{
	SCHMITT_TRIGGER(X, "DM7414")
	ALIAS(A, X.A)
	ALIAS(Q, X.Q)
	ALIAS(GND, X.GND)
	ALIAS(VCC, X.VCC)
}

static NETLIST_START(TTL_74LS14_GATE)
{
	SCHMITT_TRIGGER(X, "DM74LS14")
	ALIAS(A, X.A)
	ALIAS(Q, X.Q)
	ALIAS(GND, X.GND)
	ALIAS(VCC, X.VCC)
}

static NETLIST_START(TTL_7414_DIP)
{
	SCHMITT_TRIGGER(A, "DM7414")
	SCHMITT_TRIGGER(B, "DM7414")
	SCHMITT_TRIGGER(C, "DM7414")
	SCHMITT_TRIGGER(D, "DM7414")
	SCHMITT_TRIGGER(E, "DM7414")
	SCHMITT_TRIGGER(F, "DM7414")

	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Q, F.A,   //     Y1 |2           13| A6
		B.A, F.Q,   //     A2 |3           12| Y6
		B.Q, E.A,   //     Y2 |4    7414   11| A5
		C.A, E.Q,   //     A3 |5           10| Y5
		C.Q, D.A,   //     Y3 |6            9| A4
		A.GND, D.Q  //    GND |7            8| Y4
					//        +--------------+
	)
}

static NETLIST_START(TTL_74LS14_DIP)
{
	SCHMITT_TRIGGER(A, "DM74LS14")
	SCHMITT_TRIGGER(B, "DM74LS14")
	SCHMITT_TRIGGER(C, "DM74LS14")
	SCHMITT_TRIGGER(D, "DM74LS14")
	SCHMITT_TRIGGER(E, "DM74LS14")
	SCHMITT_TRIGGER(F, "DM74LS14")

	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Q, F.A,   //     Y1 |2           13| A6
		B.A, F.Q,   //     A2 |3           12| Y6
		B.Q, E.A,   //     Y2 |4   74LS14  11| A5
		C.A, E.Q,   //     A3 |5           10| Y5
		C.Q, D.A,   //     Y3 |6            9| A4
		A.GND, D.Q  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7416_DIP
//- Title: DM5416/DM7416 Hex Inverting Buffers with High Voltage Open-Collector Outputs
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
{
	TTL_7416_GATE(A)
	TTL_7416_GATE(B)
	TTL_7416_GATE(C)
	TTL_7416_GATE(D)
	TTL_7416_GATE(E)
	TTL_7416_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Q, F.A,   //     Y1 |2           13| A6
		B.A, F.Q,   //     A2 |3           12| Y6
		B.Q, E.A,   //     Y2 |4    7416   11| A5
		C.A, E.Q,   //     A3 |5           10| Y5
		C.Q, D.A,   //     Y3 |6            9| A4
		A.GND, D.Q  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7417_DIP
//- Title: DM5417/DM7417  Hex Buffers withHigh Voltage Open-Collector Output
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,GND,Y4,A4,Y5,A5,Y6,A6,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006505.PDF
//-
//-         +---++---+
//-         | A || Y |
//-         +===++===+
//-         | 0 || 0 |
//-         | 1 || 1 |
//-         +---++---+
//-
static NETLIST_START(TTL_7417_DIP)
{
	TTL_7417_GATE(A)
	TTL_7417_GATE(B)
	TTL_7417_GATE(C)
	TTL_7417_GATE(D)
	TTL_7417_GATE(E)
	TTL_7417_GATE(F)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC, E.VCC, F.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND, E.GND, F.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.Q, F.A,   //     Y1 |2           13| A6
		B.A, F.Q,   //     A2 |3           12| Y6
		B.Q, E.A,   //     Y2 |4    7417   11| A5
		C.A, E.Q,   //     A3 |5           10| Y5
		C.Q, D.A,   //     Y3 |6            9| A4
		A.GND, D.Q  //    GND |7            8| Y4
					//        +--------------+
	)
}

//- Identifier: TTL_7420_DIP
//- Title: 5420/DM5420/DM7420 Dual 4-Input NAND Gates
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
{
	TTL_7420_NAND(A)
	TTL_7420_NAND(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, B.D,   //     B1 |2           13| D2
		NC.I, B.C,  //     NC |3           12| C2
		A.C, NC.I,  //     C1 |4    7420   11| NC
		A.D, B.B,   //     D1 |5           10| B2
		A.Q, B.A,   //     Y1 |6            9| A2
		A.GND, B.Q  //    GND |7            8| Y2
					//        +--------------+
	)
}

//- Identifier: TTL_7421_DIP
//- Title: 54LS21/DM54LS21/DM74LS21 Dual 4-Input AND Gates
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
{
	TTL_7421_AND(A)
	TTL_7421_AND(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, B.D,   //     B1 |2           13| D2
		NC.I, B.C,  //     NC |3           12| C2
		A.C, NC.I,  //     C1 |4    7421   11| NC
		A.D, B.B,   //     D1 |5           10| B2
		A.Q, B.A,   //     Y1 |6            9| A2
		A.GND, B.Q  //    GND |7            8| Y2
					//        +--------------+
	)
}

//- Identifier: TTL_7425_DIP
//- Title: 5425/DM7425 Dual 4-Input NOR Gate (with Strobe)
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
{
	TTL_7425_NOR(A)
	TTL_7425_NOR(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(XA) // FIXME: Functionality needs to be implemented
	NC_PIN(XB) // FIXME: Functionality needs to be implemented

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, B.D,   //     B1 |2           13| D2
		XA.I, B.C,  //     X1 |3           12| C2
		A.C, XB.I,  //     C1 |4    7425   11| X2
		A.D, B.B,   //     D1 |5           10| B2
		A.Q, B.A,   //     Y1 |6            9| A2
		A.GND, B.Q  //    GND |7            8| Y2
					//        +--------------+
	)
}

//- Identifier: TTL_7427_DIP
//- Title: DM7427 Triple 3-Input NOR Gates
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
{
	TTL_7427_NOR(A)
	TTL_7427_NOR(B)
	TTL_7427_NOR(C)

	NET_C(A.VCC, B.VCC, C.VCC)
	NET_C(A.GND, B.GND, C.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, A.C,   //     B1 |2           13| C1
		B.A, A.Q,   //     A2 |3           12| Y1
		B.B, C.C,   //     B2 |4    7427   11| C3
		B.C, C.B,   //     C2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7430_DIP
//- Title: 5430/DM5430/DM7430 8-Input NAND Gate
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
{
	TTL_7430_NAND(A)
	NC_PIN(NC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //      A |1     ++    14| VCC
		A.B, NC.I,  //      B |2           13| NC
		A.C, A.H,   //      C |3           12| H
		A.D, A.G,   //      D |4    7430   11| G
		A.E, NC.I,  //      E |5           10| NC
		A.F, NC.I,  //      F |6            9| NC
		A.GND, A.Q  //    GND |7            8| Y
					//        +--------------+
	)
}

//- Identifier: TTL_7432_DIP
//- Title: 5432/DM5432/DM7432 Quad 2-Input OR Gates
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
{
	TTL_7432_OR(A)
	TTL_7432_OR(B)
	TTL_7432_OR(C)
	TTL_7432_OR(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, D.B,   //     B1 |2           13| B4
		A.Q, D.A,   //     Y1 |3           12| A4
		B.A, D.Q,   //     A2 |4    7432   11| Y4
		B.B, C.B,   //     B2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7437_DIP
//- Title: 5437/DM5437/DM7437 Quad 2-Input NAND Buffers
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
{
	TTL_7437_NAND(A)
	TTL_7437_NAND(B)
	TTL_7437_NAND(C)
	TTL_7437_NAND(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, D.B,   //     B1 |2           13| B4
		A.Q, D.A,   //     Y1 |3           12| A4
		B.A, D.Q,   //     A2 |4    7437   11| Y4
		B.B, C.B,   //     B2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7438_DIP
//- Title: DM74LS38 Quad 2-Input NAND Buffer with Open-Collector Outputs
//- Pinalias: A1,B1,Y1,A2,B2,Y2,GND,Y3,A3,B3,Y4,A4,B4,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/70/375632_DS.pdf
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
static NETLIST_START(TTL_7438_DIP)
{
	TTL_7438_NAND(A)
	TTL_7438_NAND(B)
	TTL_7438_NAND(C)
	TTL_7438_NAND(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, D.B,   //     B1 |2           13| B4
		A.Q, D.A,   //     Y1 |3           12| A4
		B.A, D.Q,   //     A2 |4    7438   11| Y4
		B.B, C.B,   //     B2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7442_DIP
//- Title: 5442A/DM5442A/DM7442A BCD to Decimal Decoders
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
{
	NET_REGISTER_DEV(TTL_7442, A)

	DIPPINS(         //       +--------------+
		A.Q0, A.VCC, //     0 |1     ++    16| VCC
		A.Q1, A.A,   //     1 |2           15| A
		A.Q2, A.B,   //     2 |3           14| B
		A.Q3, A.C,   //     3 |4           13| C
		A.Q4, A.D,   //     4 |5    7442   12| D
		A.Q5, A.Q9,  //     5 |6           11| 9
		A.Q6, A.Q8,  //     6 |7           10| 8
		A.GND, A.Q7  //   GND |8            9| 7
					 //       +--------------+
	)
}

//- Identifier: TTL_7448_DIP
//- Title: DM5448/DM48LS48/DM7448/DM74LS48 BCD to 7-Segment Decoder
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
{

	TTL_7448(A)

	DIPPINS(         //       +--------------+
		A.B, A.VCC,  //  B    |1     ++    16| VCC
		A.C, A.f,    //  C    |2           15| f
		A.LTQ, A.g,  //  LTQ  |3           14| g
		A.BIQ, A.a,  //  BIQ  |4    7448   13| a
		A.RBIQ, A.b, //  RBIQ |5           12| b
		A.D, A.c,    //  D    |6           11| c
		A.A, A.d,    //  A    |7           10| d
		A.GND, A.e   //  GND  |8            9| e
					 //       +--------------+
	)
}

//- Identifier: TTL_7450_DIP
//- Title: DM7450 Expandable Dual 2-Wide 2-Input AND-OR-INVERT Gate
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
{
	TTL_7450_ANDORINVERT(A)
	TTL_7450_ANDORINVERT(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)
	NC_PIN(NC)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     1A |1     ++    14| VCC
		B.A, A.B,   //     2A |2           13| 1B
		B.B, NC.I,  //     2B |3           12| 1XQ
		B.C, NC.I,  //     2C |4    7450   11| 1X
		B.D, A.D,   //     2D |5           10| 1D
		B.Q, A.C,   //     2Y |6            9| 1C
		A.GND, A.Q  //    GND |7            8| 1Y
					//        +--------------+
	)
}

//- Identifier: TTL_7473_DIP
//- Title: 5473/DM5473/DM7473 Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
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
{
	TTL_7473(A)
	TTL_7473(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //        +--------------+
		A.CLK, A.J,   //   CLK1 |1     ++    14| J1
		A.CLRQ, A.QQ, //   CLR1 |2           13| QQ1
		A.K, A.Q,     //     K1 |3           12| Q1
		A.VCC, A.GND, //    VCC |4    7473   11| GND
		B.CLK, B.K,   //   CLK2 |5           10| K2
		B.CLRQ, B.Q,  //   CLR2 |6            9| Q2
		B.J, B.QQ     //     J2 |7            8| QQ2
					  //        +--------------+
	)
}

//- Identifier: TTL_7473A_DIP
//- Title: DM54LS73A/DM74LS73A Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
//- Pinalias: CLK1,CLR1,K1,VCC,CLK2,CLR2,J2,QQ2,Q2,K2,GND,Q1,QQ1,J1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
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
{
	TTL_7473A(A)
	TTL_7473A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //        +--------------+
		A.CLK, A.J,   //   CLK1 |1     ++    14| J1
		A.CLRQ, A.QQ, //   CLR1 |2           13| QQ1
		A.K, A.Q,     //     K1 |3           12| Q1
		A.VCC, A.GND, //    VCC |4   7473A   11| GND
		B.CLK, B.K,   //   CLK2 |5           10| K2
		B.CLRQ, B.Q,  //   CLR2 |6            9| Q2
		B.J, B.QQ     //     J2 |7            8| QQ2
					  //        +--------------+
	)
}

//- Identifier: TTL_7474_DIP
//- Title: 5474/DM5474/DM7474 Dual Positive-Edge-Triggered D Flip-Flops with Preset, Clear and Complementary Outputs
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
{
	TTL_7474(A)
	TTL_7474(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(           //        +--------------+
		A.CLRQ, A.VCC, //   CLR1 |1     ++    14| VCC
		A.D, B.CLRQ,   //     D1 |2           13| CLR2
		A.CLK, B.D,    //   CLK1 |3           12| D2
		A.PREQ, B.CLK, //    PR1 |4    7474   11| CLK2
		A.Q, B.PREQ,   //     Q1 |5           10| PR2
		A.QQ, B.Q,     //    QQ1 |6            9| Q2
		A.GND, B.QQ    //    GND |7            8| QQ2
					   //        +-------------+
	)
}

//- Identifier: TTL_7475_DIP
//- Title: DM5475/DM7475/DM7475A/DM74LS75 4-bit D Latch
//- Pinalias: QQ1,D1,D2,E34,VCC,D3,D4,QQ4,Q4,Q3,QQ3,GND,E12,QQ2,Q2,Q1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   https://archive.org/download/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
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
{
	TTL_7475_GATE(A)
	TTL_7475_GATE(B)
	TTL_7475_GATE(C)
	TTL_7475_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	NET_C(A.CLK, B.CLK)
	NET_C(C.CLK, D.CLK)

	DIPPINS(          //        +--------------+
		A.QQ, A.Q,    //    QQ1 |1     ++    16| Q1
		A.D, B.Q,     //     D1 |2           15| Q2
		B.D, B.QQ,    //     D2 |3           14| QQ2
		C.CLK, A.CLK, //    E34 |4    7475   13| E12
		A.VCC, A.GND, //    VCC |5           12| GND
		C.D, C.QQ,    //     D3 |6           11| QQ3
		D.D, C.Q,     //     D4 |7           10| Q3
		D.QQ, D.Q     //    QQ4 |8            9| Q4
					  //        +--------------+
	)
}

//- Identifier: TTL_7477_DIP
//- Title: DM74LS77 4-bit D Latch
//- Pinalias: D1,D2,E34,VCC,D3,D4,NC,Q4,Q3,NC,GND,E12,Q2,Q1
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   https://archive.org/download/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
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
{
	TTL_7477_GATE(A)
	TTL_7477_GATE(B)
	TTL_7477_GATE(C)
	TTL_7477_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	NET_C(A.CLK, B.CLK)
	NET_C(C.CLK, D.CLK)

	NC_PIN(NC)

	DIPPINS(          //        +--------------+
		A.D, A.Q,     //     D1 |1     ++    14| Q1
		B.D, B.Q,     //     D2 |2           13| Q2
		C.CLK, A.CLK, //    E34 |3           12| E12
		A.VCC, A.GND, //    VCC |4    7477   11| GND
		C.D, NC.I,    //     D3 |5           10| NC
		D.D, C.Q,     //     D4 |6            9| Q3
		NC.I, D.Q     //     NC |7            8| Q4
					  //        +--------------+
	)
}

//- Identifier: TTL_7483_DIP
//- Title: DM5483/DM7483/DM74LS83A 4-bit Binary Adders With Fast Carry
//- Pinalias: A4,S3,A3,B3,VCC,S2,B2,A2,S1,A1,B1,GND,C0,C4,S4,B4
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006378.PDF
//-
static NETLIST_START(TTL_7483_DIP)
{
	TTL_7483(A)

	DIPPINS(          //       +--------------+
		A.A4, A.B4,   //    A4 |1     ++    16| B4
		A.S3, A.S4,   //    S3 |2           15| S4
		A.A3, A.C4,   //    A3 |3           14| C4
		A.B3, A.C0,   //    B3 |4    7483   13| C0
		A.VCC, A.GND, //   VCC |5           12| GND
		A.S2, A.B1,   //    S2 |6           11| B1
		A.B2, A.A1,   //    B2 |7           10| A1
		A.A2, A.S1    //    A2 |8            9| S1
					  //       +--------------+
	)
}

//- Identifier: TTL_7485_DIP
//- Title: DM5485/DM7485/DM74L85/DM74LS85 4-Bit Magnitude Comparators
//- Pinalias: B3,LTIN,EQIN,GTIN,GTOUT,EQOUT,LTOUT,GND,B0,A0,B1,A1,A2,B2,A3,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006379.PDF
//-
static NETLIST_START(TTL_7485_DIP)
{
	TTL_7485(A)

	DIPPINS(           //        +--------------+
		A.B3, A.VCC,   //     B3 |1     ++    16| VCC
		A.LTIN, A.A3,  //   LTIN |2           15| A3
		A.EQIN, A.B2,  //   EQIN |3           14| B2
		A.GTIN, A.A2,  //   GTIN |4    7485   13| A2
		A.GTOUT, A.A1, //  GTOUT |5           12| A1
		A.EQOUT, A.B1, //  EQOUT |6           11| B1
		A.LTOUT, A.A0, //  LTOUT |7           10| A0
		A.GND, A.B0    //    GND |8            9| B0
					   //        +--------------+
	)
}

//- Identifier: TTL_7486_DIP
//- Title: 5486/DM5486/DM7486 Quad 2-Input Exclusive-OR Gates
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
{
	TTL_7486_XOR(A)
	TTL_7486_XOR(B)
	TTL_7486_XOR(C)
	TTL_7486_XOR(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.A, A.VCC, //     A1 |1     ++    14| VCC
		A.B, D.B,   //     B1 |2           13| B4
		A.Q, D.A,   //     Y1 |3           12| A4
		B.A, D.Q,   //     A2 |4    7486   11| Y4
		B.B, C.B,   //     B2 |5           10| B3
		B.Q, C.A,   //     Y2 |6            9| A3
		A.GND, C.Q  //    GND |7            8| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_7490_DIP
//- Title: DM5490/DM7490A Decade Counter
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
{
	TTL_7490(A)
	NC_PIN(NC)

	DIPPINS(          //      +--------------+
		A.B, A.A,     //    B |1     ++    14| A
		A.R1, NC.I,   //  R01 |2           13| NC
		A.R2, A.QA,   //  R02 |3           12| QA
		NC.I, A.QD,   //   NC |4    7490   11| QD
		A.VCC, A.GND, //  VCC |5           10| GND
		A.R91, A.QB,  //  R91 |6            9| QB
		A.R92, A.QC   //  R92 |7            8| QC
					  //      +--------------+
	)
}

//- Identifier: TTL_7492_DIP
//- Title: SN5492A, SN54LS92, SN7492A, SN74LS92 Divide-By-Twelve Counter
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
{
	TTL_7492(A)
	NC_PIN(NC)

	DIPPINS(          //       +--------------+
		A.B, A.A,     //  CLKB |1     ++    14| CLKA
		NC.I, NC.I,   //    NC |2           13| NC
		NC.I, A.QA,   //    NC |3           12| QA
		NC.I, A.QD,   //    NC |4    7492   11| QD
		A.VCC, A.GND, //   VCC |5           10| GND
		A.R1, A.QB,   //   R01 |6            9| QB
		A.R2, A.QC    //   R02 |7            8| QC
					  //       +--------------+
	)
}

//- Identifier:  TTL_7493_DIP
//- Title: 7493 Binary Counters
//- Pinalias: B,R01,R02,NC,VCC,NC,NC,QC,QB,GND,QD,QA,NC,A
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations: Internal resistor network currently fixed to 5k
//-      more limitations
//- Example: ne555_astable.c,ne555_example
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006533.PDF
//-
//-    Counter Sequence
//-
//-    | COUNT || QD | QC | QB | QA |
//-    |------:||:--:|:--:|:--:|:--:|
//-    |    0  ||  0 |  0 |  0 |  0 |
//-    |    1  ||  0 |  0 |  0 |  1 |
//-    |    2  ||  0 |  0 |  1 |  0 |
//-    |    3  ||  0 |  0 |  1 |  1 |
//-    |    4  ||  0 |  1 |  0 |  0 |
//-    |    5  ||  0 |  1 |  0 |  1 |
//-    |    6  ||  0 |  1 |  1 |  0 |
//-    |    7  ||  0 |  1 |  1 |  1 |
//-    |    8  ||  1 |  0 |  0 |  0 |
//-    |    9  ||  1 |  0 |  0 |  1 |
//-    |   10  ||  1 |  0 |  1 |  0 |
//-    |   11  ||  1 |  0 |  1 |  1 |
//-    |   12  ||  1 |  1 |  0 |  0 |
//-    |   13  ||  1 |  1 |  0 |  1 |
//-    |   14  ||  1 |  1 |  1 |  0 |
//-    |   15  ||  1 |  1 |  1 |  1 |
//-
//-    Note C Output QA is connected to input B
//-
//-    Reset Count Function table
//-
//-    | R01 | R02 | QD | QC | QB | QA |
//-    |:---:|:---:|:--:|:--:|:--:|:--:|
//-    |  1  |  1  |  0 |  0 |  0 |  0 |
//-    |  0  |  X  |       COUNT       ||||
//-    |  X  |  0  |       COUNT       ||||
//-
//-

static NETLIST_START(TTL_7493_DIP)
{
	TTL_7493(A)
	NC_PIN(NC)

	DIPPINS(            //       +--------------+
		A.CLKB, A.CLKA, //  CLKB |1     ++    14| CLKA
		A.R1, NC.I,     //   R01 |2           13| NC
		A.R2, A.QA,     //   R02 |3           12| QA
		NC.I, A.QD,     //    NC |4    7493   11| QD
		A.VCC, A.GND,   //   VCC |5           10| GND
		NC.I, A.QB,     //    NC |6            9| QB
		NC.I, A.QC      //    NC |7            8| QC
						//       +--------------+
	)
}

//- Identifier: TTL_7497_DIP
//- Title: 5497/DM7497 Synchronous Modulo-64 Bit Rate Multiplier
//- Pinalias: S1,S4,S5,S0,ZQ,Y,TCQ,GND,CP,EZQ,CEQ,EY,MR,S2,S3,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS009780.PDF
//-
static NETLIST_START(TTL_7497_DIP)
{
	TTL_7497(A)

	DIPPINS(               //        +--------------+
		A.B1, A.VCC,       //     S1 |1     ++    16| VCC
		A.B4, A.B3,        //     S4 |2           15| S3
		A.B5, A.B2,        //     S5 |3           14| S2
		A.B0, A.CLR,       //     S0 |4    7497   13| MR
		A.ZQ, A.UNITYQ,    //     ZQ |5           12| EY
		A.Y, A.ENQ,        //      Y |6           11| CEQ
		A.ENOUTQ, A.STRBQ, //    TCQ |7           10| EZQ
		A.GND, A.CLK       //    GND |8            9| CP
						   //        +--------------+
	)
}

//- Identifier: TTL_74107_DIP
//- Title: SN54107, SN74107 Dual J-K Flip-Flops With Clear
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
{
	TTL_74107(A)
	TTL_74107(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //      +--------------+
		A.J, A.VCC,   //   1J |1     ++    14| VCC
		A.QQ, A.CLRQ, //  1QQ |2           13| 1CLRQ
		A.Q, A.CLK,   //   1Q |3           12| 1CLK
		A.K, B.K,     //   1K |4   74107   11| 2K
		B.Q, B.CLRQ,  //   2Q |5           10| 2CLRQ
		B.QQ, B.CLK,  //  2QQ |6            9| 2CLK
		B.GND, B.J    //  GND |7            8| 2J
					  //      +--------------+
	)
}

//- Identifier: TTL_74107A_DIP
//- Title: DM54LS107A/DM74LS107A Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops withClear and Complementary Outputs
//- Pinalias: J1,QQ1,Q1,K1,Q2,QQ2,GND,J2,CLK2,CLRQ2,K2,CLK1,CLRQ1,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006367.PDF
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
{
	TTL_74107A(A)
	TTL_74107A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //      +--------------+
		A.J, A.VCC,   //   J1 |1     ++    14| VCC
		A.QQ, A.CLRQ, //  QQ1 |2           13| CLRQ1
		A.Q, A.CLK,   //   Q1 |3           12| CLK1
		A.K, B.K,     //   K1 |4   74107A  11| K2
		B.Q, B.CLRQ,  //   Q2 |5           10| CLRQ2
		B.QQ, B.CLK,  //  QQ2 |6            9| CLK2
		B.GND, B.J    //  GND |7            8| J2
					  //      +--------------+
	)
}

//- Identifier: TTL_74113_DIP
//- Title: DM54S113/DM74S113 Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Preset and Complementary Outputs
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
{
	TTL_74113(A)
	TTL_74113(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //        +--------------+
		A.CLK, A.VCC, //   CLK1 |1     ++    14| VCC
		A.K, B.CLK,   //     K1 |2           13| CLK2
		A.J, B.K,     //     J1 |3           12| K2
		A.SETQ, B.J,  //   PRQ1 |4   74113   11| J2
		A.Q, B.SETQ,  //     Q1 |5           10| PRQ2
		A.QQ, B.Q,    //    QQ1 |6            9| Q2
		A.GND, B.QQ   //    GND |7            8| QQ2
					  //        +--------------+
	)
}

//- Identifier: TTL_74113A_DIP
//- Title: DM54S113/DM74S113 Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Preset and Complementary Outputs
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
{
	TTL_74113A(A)
	TTL_74113A(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //        +--------------+
		A.CLK, A.VCC, //   CLK1 |1     ++    14| VCC
		A.K, B.CLK,   //     K1 |2           13| CLK2
		A.J, B.K,     //     J1 |3           12| K2
		A.SETQ, B.J,  //   PRQ1 |4   74113A  11| J2
		A.Q, B.SETQ,  //     Q1 |5           10| PRQ2
		A.QQ, B.Q,    //    QQ1 |6            9| Q2
		A.GND, B.QQ   //    GND |7            8| QQ2
					  //        +--------------+
	)
}

//- Identifier:  TTL_74121_DIP
//- Title: DM74121 One-Shot with Clear and Complementary Outputs
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
{
	TTL_74121(A)
	NC_PIN(NC)
	RES(RINT, RES_K(2))
	RES(RD, RES_M(1000))

	NET_C(RINT.2, A.RC)
	// Avoid error messages if RINT is not used.
	NET_C(RINT.1, RD.2)
	NET_C(RD.1, A.GND)

	DIPPINS(          //        +--------------+
		A.QQ, A.VCC,  //     QQ |1     ++    14| VCC
		NC.I, NC.I,   //     NC |2           13| NC
		A.A1, NC.I,   //     A1 |3           12| NC
		A.A2, A.RC,   //     A2 |4   74121   11| REXT/CEXT
		A.B, A.C,     //      B |5           10| CEXT
		A.Q, RINT.1,  //      Q |6            9| RINT
		A.GND, NC.I   //    GND |7            8| NC
					  //        +--------------+
	)
}

//- Identifier:  TTL_74123_DIP
//- Title: DM74123 Dual Retriggerable One-Shot with Clear and Complementary Outputs
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
{
	TTL_74123(A)
	TTL_74123(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(         //        +--------------+
		A.A, A.VCC,  //     A1 |1     ++    16| VCC
		A.B, A.RC,   //     B1 |2           15| RC1
		A.CLRQ, A.C, //  CLRQ1 |3           14| C1
		A.QQ, A.Q,   //    QQ1 |4   74123   13| Q1
		B.Q, B.QQ,   //     Q2 |5           12| QQ2
		B.C, B.CLRQ, //     C2 |6           11| CLRQ
		B.RC, B.B,   //    RC2 |7           10| B2
		A.GND, B.A   //    GND |8            9| A2
					 //        +--------------+
	)
}

//- Identifier:  TTL_74125_DIP
//- Title: SN74125 QUADRUPLE BUS BUFFERS WITH 3-STATE OUTPUTS
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
{
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

	DIPPINS(         //       +--------------+
		A.GQ, A.VCC, //   1GQ |1     ++    14| VCC
		A.A, D.GQ,   //    1A |2           13| 4GQ
		A.Y, D.A,    //    1Y |3           12| 4A
		B.GQ, D.Y,   //   2GQ |4   74125   11| 4Y
		B.A, C.GQ,   //    2A |5           10| 3GQ
		B.Y, C.A,    //    2Y |6            9| 3A
		A.GND, C.Y   //   GND |7            8| 3Y
					 //       +--------------+
	)
}

//- Identifier: TTL_74126_DIP
//- Title: DM74LS126A Quad 3-STATE Buffer
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
{
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

	DIPPINS(        //       +--------------+
		A.G, A.VCC, //    C1 |1     ++    14| VCC
		A.A, D.G,   //    A1 |2           13| C4
		A.Y, D.A,   //    Y1 |3           12| A4
		B.G, D.Y,   //    C2 |4   74126   11| Y4
		B.A, C.G,   //    A2 |5           10| C3
		B.Y, C.A,   //    Y2 |6            9| A3
		A.GND, C.Y  //   GND |7            8| Y3
					//       +--------------+
	)
}

// FIXME: Pinalias completely wrong.

//- Identifier: TTL_74139_DIP
//- Title: 54LS139/DM54LS139/DM74LS139 Decoders/Demultiplexers
//- Pinalias: G1,A1,B1,1Y0,1Y1,1Y2,1Y3,GND,2Y3,2Y2,2Y1,2Y0,B2,A2,G2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheets/166/375388_DS.pdf
//-
//-         +---+-------+-------------+
//-         | E | A0 A1 | O0 O1 O2 O3 |
//-         +===+=======+=============+
//-         | 1 |  X  X |  1  1  1  1 |
//-         | 0 |  0  0 |  0  1  1  1 |
//-         | 0 |  1  0 |  1  0  1  1 |
//-         | 0 |  0  1 |  1  1  0  1 |
//-         | 0 |  1  1 |  1  1  1  0 |
//-         +---+-------+-------------+
//-
static NETLIST_START(TTL_74139_DIP)
{
	NET_REGISTER_DEV(TTL_74139_GATE, A)
	NET_REGISTER_DEV(TTL_74139_GATE, B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(        //        +--------------+
		A.E, A.VCC, //    /Ea |1     ++    16| VCC
		A.A, B.E,   //    A0a |2           15| /Eb
		A.B, B.A,   //    A1a |3           14| A0b
		A.O0, B.B,  //   /O0a |4   74139   13| A1b
		A.O1, B.O0, //   /O1a |5           12| /O0b
		A.O2, B.O1, //   /O2a |6           11| /O1b
		A.O3, B.O2, //   /O3a |7           10| /O2b
		A.GND, B.O3 //    GND |8            9| /O3b
					//        +--------------+
	)
}

//- Identifier: TTL_74147_DIP
//- Title: SN74147 10-Line to 4-Line priority encoder
//- Pinalias: 4,5,6,7,8,C,B,GND,A,9,1,2,3,D,NC,Vcc
//- Package: DIP-16
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://www.ti.com/lit/ds/symlink/sn74ls148.pdf
//-
//-                          10-line to 4 line encoder
//-     +-----+-----+-----+-----+-----+-----+-----+-----+-----+---+---+---+---+
//-     |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  | D | C | B | A |
//-     +=====+=====+=====+=====+=====+=====+=====+=====+=====+===+===+===+===+
//-     |  H  |  H  |  H  |  H  |  H  |  H  |  H  |  H  |  H  | H | H | H | H |
//-     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  L  | L | H | H | L |
//-     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  L  |  H  | L | H | H | H |
//-     |  X  |  X  |  X  |  X  |  X  |  X  |  L  |  H  |  H  | H | L | L | L |
//-     |  X  |  X  |  X  |  X  |  X  |  L  |  H  |  H  |  H  | H | L | L | H |
//-     |  X  |  X  |  X  |  X  |  L  |  H  |  H  |  H  |  H  | H | L | H | L |
//-     |  X  |  X  |  X  |  L  |  H  |  H  |  H  |  H  |  H  | H | L | H | H |
//-     |  X  |  X  |  L  |  H  |  H  |  H  |  H  |  H  |  H  | H | H | L | L |
//-     |  X  |  L  |  H  |  H  |  H  |  H  |  H  |  H  |  H  | H | H | L | H |
//-     |  L  |  H  |  H  |  H  |  H  |  H  |  H  |  H  |  H  | H | H | H | L |
//-     +-----+-----+-----+-----+-----+-----+-----+-----+-----+---+---+---+---+
static NETLIST_START(TTL_74147_DIP)
{
	NET_REGISTER_DEV(TTL_74147_GATE, A)
	NC_PIN(NC)

	DIPPINS(         //        +--------------+
		A.I4, A.VCC, //      4 |1     ++    16| VCC
		A.I5, NC.I,  //      5 |2           15| NC
		A.I6, A.D,   //      6 |3           14| D
		A.I7, A.I3,  //      7 |4   74147   13| 3
		A.I8, A.I2,  //      8 |5           12| 2
		A.C, A.I1,   //      C |6           11| 1
		A.B, A.I9,   //      B |7           10| 9
		A.GND, A.A   //    GND |8            9| A
					 //        +--------------+
	)
}

//- Identifier: TTL_74148_DIP
//- Title: SN74148 8-line to 3-line priority encoders
//- Pinalias: 4,5,6,7,EI,A2,A1,GND,A0,0,1,2,3,GS,E0,Vcc
//- Package: DIP-16
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://www.ti.com/lit/ds/symlink/sn74ls148.pdf
//-
//-                          10-line to 4 line encoder
//-     +-----+-----+-----+-----+-----+-----+-----+-----+-----+----+----+----+----+----+
//-     | EI  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  | A2 | A1 | A0 | GS | E0 |
//-     +=====+=====+=====+=====+=====+=====+=====+=====+=====+====+====+====+====+====+
//-     |  H  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  H |  H |  H |  H |  H |
//-     |  L  |  H  |  X  |  H  |  H  |  H  |  H  |  H  |  H  |  H |  H |  H |  H |  L |
//-     |  L  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  L  |  L |  L |  L |  L |  H |
//-     |  L  |  X  |  X  |  X  |  X  |  X  |  X  |  L  |  H  |  L |  L |  H |  L |  H |
//-     |  L  |  X  |  X  |  X  |  X  |  X  |  L  |  H  |  H  |  L |  H |  L |  L |  H |
//-     |  L  |  X  |  X  |  X  |  X  |  L  |  H  |  H  |  H  |  L |  H |  H |  L |  H |
//-     |  L  |  X  |  X  |  X  |  L  |  H  |  H  |  H  |  H  |  H |  L |  L |  L |  H |
//-     |  L  |  X  |  X  |  L  |  H  |  H  |  H  |  H  |  H  |  H |  L |  H |  L |  H |
//-     |  L  |  X  |  L  |  H  |  H  |  H  |  H  |  H  |  H  |  H |  H |  L |  L |  H |
//-     |  L  |  L  |  H  |  H  |  H  |  H  |  H  |  H  |  H  |  H |  H |  H |  L |  H |
//-     +-----+-----+-----+-----+-----+-----+-----+-----+-----+----+----+----+----+----+
static NETLIST_START(TTL_74148_DIP)
{
	NET_REGISTER_DEV(TTL_74148_GATE, A)

	DIPPINS(         //        +--------------+
		A.I4, A.VCC, //      4 |1     ++    16| VCC
		A.I5, A.E0,  //      5 |2           15| E0
		A.I6, A.GS,  //      6 |3           14| GS
		A.I7, A.I3,  //      7 |4   74148   13| 3
		A.EI, A.I2,  //     EI |5           12| 2
		A.A2, A.I1,  //     A2 |6           11| 1
		A.A1, A.I0,  //     A1 |7           10| 0
		A.GND, A.A0  //    GND |8            9| A0
					 //        +--------------+
	)
}

//- Identifier: TTL_74151_DIP
//- Title: SN74151 Data selector/multiplexer
//- Pinalias: D3,D2,D1,D0,Y,W,G,Gnd,C,B,A,D7,D6,D5,D4,Vcc
//- Package: DIP-16
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://www.ti.com/lit/ds/symlink/sn74ls151.pdf
//-
//-         +---+---+----+--++----+-----+
//-         | C | B | A | G ||  Y |  W  |
//-         +===+===+===+===++====+=====+
//-         | X | X | X | H ||  L |  H  |
//-         | L | L | L | L || D0 | ~D0 |
//-         | L | L | H | L || D1 | ~D1 |
//-         | L | H | L | L || D2 | ~D2 |
//-         | L | H | H | L || D3 | ~D3 |
//-         | H | L | L | L || D4 | ~D4 |
//-         | H | L | H | L || D5 | ~D5 |
//-         | H | H | L | L || D6 | ~D6 |
//-         | H | H | H | L || D7 | ~D7 |
//-         +---+---+---+---++----+-----+
//-
static NETLIST_START(TTL_74151_DIP)
{
	NET_REGISTER_DEV(TTL_74151_GATE, A)

	DIPPINS(         //        +--------------+
		A.D3, A.VCC, //     D3 |1     ++    16| VCC
		A.D2, A.D4,  //     D2 |2           15| D4
		A.D1, A.D5,  //     D1 |3           14| D5
		A.D0, A.D6,  //     D0 |4   74151   13| D6
		A.Y,  A.D7,  //      Y |5           12| D7
		A.W,   A.A,  //      W |6           11| A
		A.G,   A.B,  //      G |7           10| B
		A.GND, A.C   //    GND |8            9| C
					 //        +--------------+
	)
}

//- Identifier: TTL_74153_DIP
//- Title: 54153/DM54153/DM74153 Dual 4-Line to 1-LineData Selectors/Multiplexers
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
{
	NET_REGISTER_DEV(TTL_74153, A)
	NET_REGISTER_DEV(TTL_74153, B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	DIPPINS(        //      +--------------+
		A.G, A.VCC, //   G1 |1     ++    16| VCC
		A.B, B.G,   //    B |2           15| G2
		A.C3, A.A,  //  1C3 |3           14| A
		A.C2, B.C3, //  1C2 |4   74153   13| 2C3
		A.C1, B.C2, //  1C1 |5           12| 2C2
		A.C0, B.C1, //  1C0 |6           11| 2C1
		A.AY, B.C0, //   Y1 |7           10| 2C0
		A.GND, B.AY //  GND |8            9| Y2
					//      +--------------+
	)
}

//- Identifier: TTL_74155_DIP
//- Title: 54LS155/DM54LS155/DM74LS155 Dual 2-Line to 4-Line Decoders/Demultiplexers
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
{
	NET_REGISTER_DEV(TTL_74155A_GATE, A)
	NET_REGISTER_DEV(TTL_74155B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(        //        +--------------+
		A.C, A.VCC, //     C1 |1     ++    16| VCC
		A.G, B.C,   //     G1 |2           15| C2
		A.B, B.G,   //      B |3           14| G2
		A.Y3, B.A,  //    1Y3 |4   74155   13| A
		B.Y2, B.Y3, //    1Y2 |5           12| 2Y3
		B.Y1, B.Y2, //    1Y1 |6           11| 2Y2
		B.Y0, B.Y1, //    1Y0 |7           10| 2Y1
		A.GND, B.Y0 //    GND |8            9| 2Y0
					//        +--------------+
	)
}

//- Identifier: TTL_74156_DIP
//- Title: 54LS156/DM54LS156/DM74LS156 Dual 2-Line to 4-Line Decoders/Demultiplexers with Open-Collector Outputs
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
{
	NET_REGISTER_DEV(TTL_74156A_GATE, A)
	NET_REGISTER_DEV(TTL_74156B_GATE, B)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(        //        +--------------+
		A.C, A.VCC, //     C1 |1     ++    16| VCC
		A.G, B.C,   //     G1 |2           15| C2
		A.B, B.G,   //      B |3           14| G2
		A.Y3, B.A,  //    1Y3 |4   74156   13| A
		B.Y2, B.Y3, //    1Y2 |5           12| 2Y3
		B.Y1, B.Y2, //    1Y1 |6           11| 2Y2
		B.Y0, B.Y1, //    1Y0 |7           10| 2Y1
		A.GND, B.Y0 //    GND |8            9| 2Y0
					//        +--------------+
	)
}

//- Identifier: TTL_74157_DIP
//- Title: 54157/DM54157/DM74157 Quad 2-Line to 1-Line Data Selectors/Multiplexers
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
{
	NET_REGISTER_DEV(TTL_74157_GATE, A)
	NET_REGISTER_DEV(TTL_74157_GATE, B)
	NET_REGISTER_DEV(TTL_74157_GATE, C)
	NET_REGISTER_DEV(TTL_74157_GATE, D)

	NET_C(A.E, B.E, C.E, D.E)
	NET_C(A.S, B.S, C.S, D.S)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //        +--------------+
		A.S, A.VCC, //      S |1     ++    16| VCC
		A.I, A.E,   //     A1 |2           15| G
		A.J, D.I,   //     B1 |3           14| A4
		A.O, D.J,   //     Y1 |4   74157   13| B4
		B.I, D.O,   //     A2 |5           12| Y4
		B.J, C.I,   //     B2 |6           11| A3
		B.O, C.J,   //     Y2 |7           10| B3
		A.GND, C.O  //    GND |8            9| Y3
					//        +--------------+
	)
}

//- Identifier: TTL_74161_DIP
//- Title: DM54161/DM74161 Synchronous 4-Bit Counter
//- Pinalias: CLRQ,CLK,A,B,C,D,ENP,GND,LOADQ,ENT,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006551.PDF
//-
static NETLIST_START(TTL_74161_DIP)
{
	TTL_74161(A)

	DIPPINS(           //           +--------------+
		A.CLRQ, A.VCC, //    /CLEAR |1     ++    16| VCC
		A.CLK, A.RC,   //     CLOCK |2           15| RC
		A.A, A.QA,     //         A |3           14| QA
		A.B, A.QB,     //         B |4   74161   13| QB
		A.C, A.QC,     //         C |5           12| QC
		A.D, A.QD,     //         D |6           11| QD
		A.ENP, A.ENT,  //  Enable P |7           10| Enable T
		A.GND, A.LOADQ //       GND |8            9| /LOAD
					   //           +--------------+
	)
}

//- Identifier: TTL_74163_DIP
//- Title: DM74163 Synchronous 4-Bit Counter
//- Pinalias: CLRQ,CLK,A,B,C,D,ENP,GND,LOADQ,ENT,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006551.PDF
//-
static NETLIST_START(TTL_74163_DIP)
{
	TTL_74163(A)

	DIPPINS(           //           +--------------+
		A.CLRQ, A.VCC, //    /CLEAR |1     ++    16| VCC
		A.CLK, A.RC,   //     CLOCK |2           15| RC
		A.A, A.QA,     //         A |3           14| QA
		A.B, A.QB,     //         B |4   74163   13| QB
		A.C, A.QC,     //         C |5           12| QC
		A.D, A.QD,     //         D |6           11| QD
		A.ENP, A.ENT,  //  Enable P |7           10| Enable T
		A.GND, A.LOADQ //       GND |8            9| /LOAD
					   //           +--------------+
	)
}

//- Identifier: TTL_74164_DIP
//- Title: DM74164 8-Bit Serial In/Parallel Out Shift Registers
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
{
	TTL_74164(A)

	DIPPINS(          //      +--------------+
		A.A, A.VCC,   //    A |1     ++    14| VCC
		A.B, A.QH,    //    B |2           13| QH
		A.QA, A.QG,   //   QA |3           12| QG
		A.QB, A.QF,   //   QB |4   74164   11| QF
		A.QC, A.QE,   //   QC |5           10| QE
		A.QD, A.CLRQ, //   QD |6            9| CLRQ
		A.GND, A.CLK  //  GND |7            8| CLK
					  //      +--------------+
	)
}

//- Identifier: TTL_74165_DIP
//- Title: 54165/DM74165 8-Bit Parallel-to-Serial Converter
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
{
	TTL_74165(A)

	DIPPINS(             //       +--------------+
		A.SH_LDQ, A.VCC, //   PLQ |1     ++    16| VCC
		A.CLK, A.CLKINH, //   CP1 |2           15| CP2
		A.E, A.D,        //    P4 |3           14| P3
		A.F, A.C,        //    P5 |4    74165  13| P2
		A.G, A.B,        //    P6 |5           12| P1
		A.H, A.A,        //    P7 |6           11| P0
		A.QHQ, A.SER,    //   QQ7 |7           10| DS
		A.GND, A.QH      //   GND |8            9| Q7
						 //       +--------------+
	)
}

//- Identifier: TTL_74166_DIP
//- Title: DM74LS166 8-Bit Parallel-In/Serial-Out Shift Registers
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
{
	TTL_74166(A)

	DIPPINS(           //         +--------------+
		A.SER, A.VCC,  //     SER |1     ++    16| VCC
		A.A, A.SH_LDQ, //       A |2           15| SH/LDQ
		A.B, A.H,      //       B |3           14| H
		A.C, A.QH,     //       C |4    74166  13| QH
		A.D, A.G,      //       D |5           12| G
		A.CLKINH, A.F, //  CLKINH |6           11| F
		A.CLK, A.E,    //     CLK |7           10| E
		A.GND, A.CLRQ  //     GND |8            9| CLRQ
					   //         +--------------+
	)
}

//- Identifier: TTL_74174_DIP
//- Title: DM74174 Hex/Quad D-Type Flip-Flop with Clear
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
{
	TTL_74174(A)

	DIPPINS(           //       +------------+
		A.CLRQ, A.VCC, //  CLRQ |1    ++   16| VCC
		A.Q1, A.Q6,    //    Q1 |2         15| Q6
		A.D1, A.D6,    //    D1 |3         14| D6
		A.D2, A.D5,    //    D2 |4  74174  13| D5
		A.Q2, A.Q5,    //    Q2 |5         12| Q5
		A.D3, A.D4,    //    D3 |6         11| D4
		A.Q3, A.Q4,    //    Q3 |7         10| Q4
		A.GND, A.CLK   //   GND |8          9| CLK
					   //       +------------+
	)
}

// FIXME: add documentation
static NETLIST_START(TTL_74175_DIP)
{
	TTL_74175(A)

	DIPPINS(           //       +--------------+
		A.CLRQ, A.VCC, //  CLRQ |1     ++    16| VCC
		A.Q1, A.Q4,    //    Q1 |2           15| Q4
		A.Q1Q, A.Q4Q,  //   Q1Q |3           14| Q4Q
		A.D1, A.D4,    //    D1 |4   74175   13| D4
		A.D2, A.D3,    //    D2 |5           12| D3
		A.Q2Q, A.Q3Q,  //   Q2Q |6           11| Q3Q
		A.Q2, A.Q3,    //    Q2 |7           10| Q3
		A.GND, A.CLK   //   GND |8            9| CLK
					   //       +--------------+
	)
}

// FIXME: add documentation
static NETLIST_START(TTL_74192_DIP)
{
	TTL_74192(A)

	DIPPINS(             //       +--------------+
		A.B, A.VCC,      //     B |1     ++    16| VCC
		A.QB, A.A,       //    QB |2           15| A
		A.QA, A.CLEAR,   //    QA |3           14| CLEAR
		A.CD, A.BORROWQ, //    CD |4    74192  13| BORROWQ
		A.CU, A.CARRYQ,  //    CU |5           12| CARRYQ
		A.QC, A.LOADQ,   //    QC |6           11| LOADQ
		A.QD, A.C,       //    QD |7           10| C
		A.GND, A.D       //   GND |8            9| D
						 //       +--------------+
	)
}

// FIXME: add documentation
static NETLIST_START(TTL_74193_DIP)
{
	TTL_74193(A)

	DIPPINS(             //       +--------------+
		A.B, A.VCC,      //     B |1     ++    16| VCC
		A.QB, A.A,       //    QB |2           15| A
		A.QA, A.CLEAR,   //    QA |3           14| CLEAR
		A.CD, A.BORROWQ, //    CD |4    74192  13| BORROWQ
		A.CU, A.CARRYQ,  //    CU |5           12| CARRYQ
		A.QC, A.LOADQ,   //    QC |6           11| LOADQ
		A.QD, A.C,       //    QD |7           10| C
		A.GND, A.D       //   GND |8            9| D
						 //       +--------------+
	)
}

// FIXME: add documentation
static NETLIST_START(TTL_74194_DIP)
{
	TTL_74194(A)

	DIPPINS(           //         +--------------+
		A.CLRQ, A.VCC, //    CLRQ |1     ++    16| VCC
		A.SRIN, A.QA,  //    SRIN |2           15| QA
		A.A, A.QB,     //       A |3           14| QB
		A.B, A.QC,     //       B |4    74194  13| QC
		A.C, A.QD,     //       C |5           12| QD
		A.D, A.CLK,    //       D |6           11| CLK
		A.SLIN, A.S1,  //    SLIN |7           10| S1
		A.GND, A.S0    //     GND |8            9| S0
					   //         +--------------+
	)
}

//- Identifier: TTL_74260_DIP
//- Title: DM54LS260/DM74LS260 Dual 5-Input NOR Gate
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
{
	TTL_74260_NOR(A)
	TTL_74260_NOR(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(        //        +--------------+
		A.C, A.VCC, //     C1 |1     ++    14| VCC
		A.D, A.B,   //     D1 |2           13| B1
		A.E, A.A,   //     E1 |3           12| A1
		B.E, B.D,   //     E2 |4   74260   11| D2
		A.Q, B.C,   //     Y1 |5           10| C2
		B.Q, B.B,   //     Y2 |6            9| B2
		A.GND, B.A  //    GND |7            8| A2
					//        +--------------+
	)
}

//- Identifier: TTL_74279_DIP
//- Title: 54279/DM74279 Quad Set-Reset Latch
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
#endif

static NETLIST_START(TTL_74279_DIP)
{
	TTL_74279B(A)
	TTL_74279A(B)
	TTL_74279B(C)
	TTL_74279A(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(        //       +--------------+
		A.R, A.VCC, //   1RQ |1     ++    16| VCC
		A.S1, D.S,  //  1S1Q |2           15| 4SQ
		A.S2, D.R,  //  1S2Q |3           14| 4RQ
		A.Q, D.Q,   //    1Q |4    74279  13| 4Q
		B.R, C.S2,  //   2RQ |5           12| 3S2Q
		B.S, C.S1,  //   2SQ |6           11| 3S1Q
		B.Q, C.R,   //    2Q |7           10| 3RQ
		A.GND, C.Q  //   GND |8            9| 3Q
					//       +--------------+
	)
}

// FIXME: Documentation
static NETLIST_START(TTL_74365_DIP)
{
	TTL_74365(A)

	DIPPINS(          //       +--------------+
		A.G1Q, A.VCC, //   G1Q |1     ++    16| VCC
		A.A1, A.G2Q,  //    A1 |2           15| G2Q
		A.Y1, A.A6,   //    Y1 |3           14| A6
		A.A2, A.Y6,   //    A2 |4    74365  13| Y6
		A.Y2, A.A5,   //    Y2 |5           12| A5
		A.A3, A.Y5,   //    A3 |6           11| Y5
		A.Y3, A.A4,   //    Y3 |7           10| A4
		A.GND, A.Y4   //   GND |8            9| Y4
					  //       +--------------+
	)
}

//- Identifier: TTL_74290_DIP
//- Title: SN54290/SN74290, SN54LS290/SN74LS290 Decade Counter
//- Pinalias: R91,NC,R92,QC,QB,NC,GND,QD,QA,CKA,CKB,R01,R02,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://pdf1.alldatasheet.com/datasheet-pdf/view/27403/TI/SN74293.html
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
static NETLIST_START(TTL_74290_DIP)
{
	TTL_7490(A)
	NC_PIN(NC)

	DIPPINS(          //      +--------------+
		A.R91, A.VCC, //  R91 |1     ++    14| VCC
		NC.I, A.R2,   //   NC |2           13| R02
		A.R92, A.R1,  //  R92 |3           12| R01
		A.QC, A.B,    //   QC |4   74290   11| CKB
		A.QB, A.A,    //   QB |5           10| CKA
		NC.I, A.QA,   //   NC |6            9| QA
		A.GND, A.QD   //  GND |7            8| QD
					  //      +--------------+
	)
}

//- Identifier:  TTL_74293_DIP
//- Title: SN54293/SN74293, SN54LS293/SN74LS293 Binary Counters
//- Pinalias: B,R01,R02,NC,VCC,NC,NC,QC,QB,GND,QD,QA,NC,A
//- Package: DIP
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- Limitations: Internal resistor network currently fixed to 5k
//-      more limitations
//- FunctionTable:
//-   https://pdf1.alldatasheet.com/datasheet-pdf/view/27403/TI/SN74293.html
//-
//-    Counter Sequence
//-
//-    | COUNT || QD | QC | QB | QA |
//-    |------:||:--:|:--:|:--:|:--:|
//-    |    0  ||  0 |  0 |  0 |  0 |
//-    |    1  ||  0 |  0 |  0 |  1 |
//-    |    2  ||  0 |  0 |  1 |  0 |
//-    |    3  ||  0 |  0 |  1 |  1 |
//-    |    4  ||  0 |  1 |  0 |  0 |
//-    |    5  ||  0 |  1 |  0 |  1 |
//-    |    6  ||  0 |  1 |  1 |  0 |
//-    |    7  ||  0 |  1 |  1 |  1 |
//-    |    8  ||  1 |  0 |  0 |  0 |
//-    |    9  ||  1 |  0 |  0 |  1 |
//-    |   10  ||  1 |  0 |  1 |  0 |
//-    |   11  ||  1 |  0 |  1 |  1 |
//-    |   12  ||  1 |  1 |  0 |  0 |
//-    |   13  ||  1 |  1 |  0 |  1 |
//-    |   14  ||  1 |  1 |  1 |  0 |
//-    |   15  ||  1 |  1 |  1 |  1 |
//-
//-    Note C Output QA is connected to input B
//-
//-    Reset Count Function table
//-
//-    | R01 | R02 | QD | QC | QB | QA |
//-    |:---:|:---:|:--:|:--:|:--:|:--:|
//-    |  1  |  1  |  0 |  0 |  0 |  0 |
//-    |  0  |  X  |       COUNT       ||||
//-    |  X  |  0  |       COUNT       ||||
//-
//-

static NETLIST_START(TTL_74293_DIP)
{
	TTL_7493(A)
	NC_PIN(NC)

	DIPPINS(          //      +--------------+
		NC.I, A.VCC,  //   NC |1     ++    14| VCC
		NC.I, A.R2,   //   NC |2           13| R02
		NC.I, A.R1,   //   NC |3           12| R01
		A.QC, A.CLKB, //   QC |4   74293   11| CKB
		A.QB, A.CLKA, //   QB |5           10| CKA
		NC.I, A.QA,   //   NC |6            9| QA
		A.GND, A.QD   //  GND |7            8| QD
					  //      +--------------+
	)
}

//- Identifier: TTL_74368_DIP
//- Title: SN74368 Hex Inverting Buffers and Line Drivers With 3-State Outputs
//- Pinalias: 1G,1A1,1Y1,1A2,1Y2,1A3,1Y3,Gnd,1Y4,1A4,2Y1,2A1,2Y2,2A2,2G,Vcc
//- Package: DIP-16
//- NamingConvention: Naming conventions follow Texas Instruments datasheet
//- FunctionTable:
//-   https://www.ti.com/lit/ds/symlink/sn74hc368.pdf
//-   https://www.ti.com/lit/ds/symlink/sn74ls368a.pdf
//-
//-         +----+---++---+
//-         | OE | A || Y |
//-         +====+===++===+
//-         |  1 | X || Z |
//-         |  0 | 1 || 0 |
//-         |  0 | 0 || 1 |
//-         +---+----++---+
//-
static NETLIST_START(TTL_74368_DIP)
{
	NET_REGISTER_DEV(TTL_74368_GATE, A)
	NET_REGISTER_DEV(TTL_74368_GATE, B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	NET_C(A.A, B.A)
	NET_C(A.B, B.B)

	DIPPINS(         //      +--------------+
		A.OE, A.VCC, //  1OE |1     ++    16| VCC
		A.A1, B.OE,  //  1A1 |2           15| 2OE
		A.Y1, B.A2,  //  1Y1 |3           14| 2A2
		A.A2, B.Y2,  //  1A2 |4   74368   13| 2Y2
		A.Y2, B.A1,  //  1Y2 |5           12| 2A1
		A.A3, B.Y1,  //  1A3 |6           11| 2Y1
		A.Y3, A.A4,  //  1Y4 |7           10| 1A4
		A.GND, A.Y4  //  GND |8            9| 1Y4
					 //      +--------------+
	)
}

//- Identifier: TTL_74377_DIP
//- Title: DM54LS377/DM74LS377 Octal D Flip-Flop with Common Enable and Clock
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
{
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

	DIPPINS(        //        +--------------+
		A.E, A.VCC, //     EQ |1     ++    20| VCC
		A.Q, H.Q,   //     Q0 |2           19| Q7
		A.D, H.D,   //     D0 |3           18| D7
		B.D, G.D,   //     D1 |4   74377   17| D6
		B.Q, G.Q,   //     Q1 |5           16| Q6
		C.Q, F.Q,   //     Q2 |6           15| Q5
		C.D, F.D,   //     D2 |7           14| D5
		D.D, E.D,   //     D3 |8           13| D4
		D.Q, E.Q,   //     Q3 |9           12| Q4
		A.GND, A.CP //    GND |10          11| CP
					//        +--------------+
	)
}

//- Identifier: TTL_74378_DIP
//- Title: DM54LS378/DM74LS378 Parallel D Register with Enable
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
{
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

	DIPPINS(        //        +--------------+
		A.E, A.VCC, //     EQ |1     ++    16| VCC
		A.Q, F.Q,   //     Q0 |2           15| Q5
		A.D, F.D,   //     D0 |3           14| D5
		B.D, E.D,   //     D1 |4   74378   13| D4
		B.Q, E.Q,   //     Q1 |5           12| Q4
		C.D, D.D,   //     D2 |6           11| D3
		C.Q, D.Q,   //     Q2 |7           10| Q3
		A.GND, A.CP //    GND |8            9| CP
					//        +--------------+
	)
}

//- Identifier: TTL_74379_DIP
//- Title: 54LS379/DM74LS379 Quad Parallel Register with Enable
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
{
	TTL_74377_GATE(A)
	TTL_74377_GATE(B)
	TTL_74377_GATE(C)
	TTL_74377_GATE(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)
	NET_C(A.CP, B.CP, C.CP, D.CP)
	NET_C(A.E, B.E, C.E, D.E)

	DIPPINS(        //        +--------------+
		A.E, A.VCC, //     EQ |1     ++    16| VCC
		A.Q, D.Q,   //     Q0 |2           15| Q3
		A.QQ, D.QQ, //    QQ0 |3           14| QQ3
		A.D, D.D,   //     D0 |4   74379   13| D3
		B.D, C.D,   //     D1 |5           12| D2
		B.QQ, C.QQ, //    QQ1 |6           11| QQ2
		B.Q, C.Q,   //     Q1 |7           10| Q2
		A.GND, A.CP //    GND |8            9| CP
					//        +--------------+
	)
}

//- Identifier: TTL_74393_DIP
//- Title: Dual 4-Bit Binary Counter
//- Pinalias: 1A,1CLR,1QA,1QB,1QC,1QD,GND,2QD,2QC,2QB,2QA,2CLR,2A,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-   http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006434.PDF
//-
static NETLIST_START(TTL_74393_DIP)
{
	TTL_74393(A)
	TTL_74393(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(         //       +--------------+
		A.CP, A.VCC, //    1A |1    ++     14| VCC
		A.MR, B.CP,  //  1CLR |2           13| 2A
		A.Q0, B.MR,  //   1QA |3           12| 2CLR
		A.Q1, B.Q0,  //   1QB |4   74393   11| 2QA
		A.Q2, B.Q1,  //   1QC |5           10| 2QB
		A.Q3, B.Q2,  //   1QD |6            9| 2QC
		A.GND, B.Q3  //   GND |7            8| 2QD
					 //       +--------------+
	)
}

//- Identifier: SN74LS629_DIP
//- Title: SN74LS629 VOLTAGE-CONTROLLED OSCILLATORS
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
{
	SN74LS629(A, CAP_U(1))
	SN74LS629(B, CAP_U(1))

	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
	NET_C(A.OSCGND, B.OSCGND)
	NET_C(A.OSCVCC, B.OSCVCC)
	NC_PIN(NC)

	DIPPINS(            //           +--------------+
		B.FC, A.VCC,    //       2FC |1     ++    16| VCC
		A.FC, A.OSCVCC, //       1FC |2           15| OSC VCC
		A.RNG, B.RNG,   //      1RNG |3           14| 2RNG
		NC.I, NC.I,     //      1CX1 |4  74LS629  13| 2CX1
		NC.I, NC.I,     //      1CX2 |5           12| 2CX2
		A.ENQ, B.ENQ,   //      1ENQ |6           11| 2ENQ
		B.Y, B.Y,       //        1Y |7           10| 2Y
		A.OSCGND, A.GND //   OSC GND |8            9| GND
						//           +--------------+
	)
}

//- Identifier: TTL_9310_DIP
//- Title: DM9310/DM8310 Synchronous 4-Bit Decade Counter
//- Pinalias: CLEARQ,CLOCK,A,B,C,D,ENABLEP,GND,LOADQ,ENABLET,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    https://archive.org/download/bitsavers_nationaldaTTLDatabook_40452765/1976_National_TTL_Databook.pdf
//-
static NETLIST_START(TTL_9310_DIP)
{
	TTL_9310(A)

	DIPPINS(           //           +--------------+
		A.CLRQ, A.VCC, //    /CLEAR |1     ++    16| VCC
		A.CLK, A.RC,   //     CLOCK |2           15| RC
		A.A, A.QA,     //         A |3           14| QA
		A.B, A.QB,     //         B |4    9310   13| QB
		A.C, A.QC,     //         C |5           12| QC
		A.D, A.QD,     //         D |6           11| QD
		A.ENP, A.ENT,  //  Enable P |7           10| Enable T
		A.GND, A.LOADQ //       GND |8            9| /LOAD
					   //           +--------------+
	)
}

//- Identifier: TTL_9312_DIP
//- Title: DM9312/DM8312 One of Eight Line Data Selectors/Multiplexers
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
{
	TTL_9312(A)

	DIPPINS(         //      +--------------+
		A.D0, A.VCC, //   D0 |1     ++    16| VCC
		A.D1, A.Y,   //   D1 |2           15| Y
		A.D2, A.YQ,  //   D2 |3           14| YQ
		A.D3, A.C,   //   D3 |4    9312   13| C
		A.D4, A.B,   //   D4 |5           12| B
		A.D5, A.A,   //   D5 |6           11| A
		A.D6, A.G,   // Strobe //   D6 |7           10| G
		A.GND, A.D7  //  GND |8            9| D7
					 //      +--------------+
	)
}

// FIXME: Documentation
static NETLIST_START(TTL_9314_DIP)
{
	TTL_9314(A)

	DIPPINS(         //        +--------------+
		A.EQ, A.VCC, //     /E |1     ++    16| VCC
		A.S0Q, A.Q0, //    /S0 |2           15| Q0
		A.D0, A.S1Q, //     D0 |3           14| /S1
		A.D1, A.Q1,  //     D1 |4   DM9314  13| Q1
		A.S2Q, A.Q2, //    /S2 |5           12| Q2
		A.D2, A.S3Q, //     D2 |6           11| /S3
		A.D3, A.Q3,  //     D3 |7           10| Q3
		A.GND, A.MRQ //    GND |8            9| /MR
					 //        +--------------+
	)
}

//- Identifier: TTL_9316_DIP
//- Title: DM9316/DM8316 Synchronous 4-Bit Counters
//- Pinalias: CLEARQ,CLOCK,A,B,C,D,ENABLEP,GND,LOADQ,ENABLET,QD,QC,QB,QA,RC,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006606.PDF
//-
static NETLIST_START(TTL_9316_DIP)
{
	TTL_9316(A)

	DIPPINS(           //           +--------------+
		A.CLRQ, A.VCC, //    CLEARQ |1     ++    16| VCC
		A.CLK, A.RC,   //     CLOCK |2           15| RC
		A.A, A.QA,     //         A |3           14| QA
		A.B, A.QB,     //         B |4    9316   13| QB
		A.C, A.QC,     //         C |5           12| QC
		A.D, A.QD,     //         D |6           11| QD
		A.ENP, A.ENT,  //  Enable P |7           10| Enable T
		A.GND, A.LOADQ //       GND |8            9| LOADQ
					   //           +--------------+
	)
}

//- Identifier: TTL_9322_DIP
//- Title: DM9322/DM8322 Quad 2-Line to 1-Line Data Selectors/Multiplexers
//- Pinalias: SELECT,A1,B1,Y1,A2,B2,Y2,GND,Y3,B3,A3,Y4,B4,A4,STROBE,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS006606.PDF
//-
static NETLIST_START(TTL_9322_DIP)
{
	TTL_9322(A)

	DIPPINS(             //         +--------------+
		A.SELECT, A.VCC, //  SELECT |1     ++    16| VCC
		A.A1, A.STROBE,  //      A1 |2           15| STROBE
		A.B1, A.A4,      //      B1 |3           14| A4
		A.Y1, A.B4,      //      Y1 |4    9322   13| B4
		A.A2, A.Y4,      //      A2 |5           12| Y4
		A.B2, A.A3,      //      B2 |6           11| A3
		A.Y2, A.B3,      //      Y2 |7           10| B3
		A.GND, A.Y3      //     GND |8            9| Y3
						 //         +--------------+
	)
}

//- Identifier: TTL_9321_DIP
//- Title: DM9321/DM8321 Dual 4-Line to 1-Line Data Selectors/Multiplexers
//- Pinalias: AE,AA0,AA1,AD0,AD1,AD2,AD3,GND,BD3,BD2,BD1,BD0,BA1,BA0,BE,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//-
static NETLIST_START(TTL_9321_DIP)
{
	TTL_9321(A)
	TTL_9321(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(        //         +--------------+
		A.E, A.VCC, //      /E |1     ++    16| VCC
		A.A0, B.E,  //      A0 |2           15| /E
		A.A1, B.A0, //      A1 |3           14| A0
		A.D0, B.A1, //     /D0 |4    9321   13| A1
		A.D1, B.D0, //     /D1 |5           12| /D0
		A.D2, B.D1, //     /D2 |6           11| /D1
		A.D3, B.D2, //     /D3 |7           10| /D2
		A.GND, B.D3 //     GND |8            9| /D3
					//         +--------------+
	)
}

// FIXME: Documentation
static NETLIST_START(TTL_9334_DIP)
{
	TTL_9334(A)

	DIPPINS(         //        +--------------+
		A.A0, A.VCC, //     A0 |1     ++    16| VCC
		A.A1, A.CQ,  //     A1 |2           15| /C
		A.A2, A.EQ,  //     A2 |3           14| /E
		A.Q0, A.D,   //     Q0 |4   DM9334  13| D
		A.Q1, A.Q7,  //     Q1 |5           12| Q7
		A.Q2, A.Q6,  //     Q2 |6           11| Q6
		A.Q3, A.Q5,  //     Q3 |7           10| Q5
		A.GND, A.Q4  //    GND |8            9| Q4
					 //        +--------------+
	)
}

//- Identifier: TTL_9602_DIP
//- Title: DM9602/DM6802 Dual Retriggerable, Resettable One Shots
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
{
	TTL_9602(A)
	TTL_9602(B)

	NET_C(A.VCC, B.VCC)
	NET_C(A.GND, B.GND)

	DIPPINS(          //        +--------------+
		A.C, A.VCC,   //     C1 |1     ++    16| VCC
		A.RC, B.C,    //    RC1 |2           15| C2
		A.CLRQ, B.RC, //  CLRQ1 |3           14| RC2
		A.B, B.CLRQ,  //     B1 |4    9602   13| CLRQ2
		A.A, B.B,     //     A1 |5           12| B2
		A.Q, B.A,     //     Q1 |6           11| A2
		A.QQ, B.Q,    //    QQ1 |7           10| Q2
		A.GND, B.QQ   //    GND |8            9| QQ2
					//        +--------------+
	)
}

// FIXME: Documentation and naming
static NETLIST_START(TTL_8277_DIP)
{
	TTL_8277(A)

	DIPPINS(            //        +--------------+
		A.RESET, A.VCC, //  RESET |1     ++    16| VCC
		A.Q7QA, A.Q7QB, //   /Q7A |2           15| /Q7B
		A.Q7A, A.Q7B,   //    Q7A |3           14| Q7B
		A.DSA, A.DSB,   //    DSA |4    8277   13| DSB
		A.D1A, A.D1B,   //    D1A |5           12| D1B
		A.D0A, A.D0B,   //    D0A |6           11| D0B
		A.CLKA, A.CLKB, //   CLKA |7           10| CLKB
		A.GND, A.CLK    //    GND |8            9| CLK
						//        +--------------+
	)
}

// FIXME: Documentation, add model, seems to be a CMOS device
static NETLIST_START(TTL_AM2847_DIP)
{
	TTL_AM2847(A)

	DIPPINS(            //        +--------------+
		A.OUTA, A.VSS,  //   OUTA |1     ++    16| VSS
		A.RCA, A.IND,   //    RCA |2           15| IND
		A.INA, A.RCD,   //    INA |3           14| RCD
		A.OUTB, A.OUTD, //   OUTB |4   Am2847  13| OUTD
		A.RCB, A.VGG,   //    RCB |5           12| VGG
		A.INB, A.CP,    //    INB |6           11| CP
		A.OUTC, A.INC,  //   OUTC |7           10| INC
		A.VDD, A.RCC    //    VDD |8            9| RCC
						//        +--------------+
	)
}

static TRUTH_TABLE(TTL_7400_NAND, 2, 1, "+A,+B,@VCC,@GND")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|1|22")
	TT_LINE("X,0|1|22")
	TT_LINE("1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7402_NOR, 2, 1, "+A,+B,@VCC,@GND")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|1|22")
	TT_LINE("X,1|0|15")
	TT_LINE("1,X|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7404_INVERT, 1, 1, "+A,@VCC,@GND")
{
	TT_HEAD(" A | Q ")
	TT_LINE(" 0 | 1 |22")
	TT_LINE(" 1 | 0 |15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7406_GATE, 1, 1, "")
{
	TT_HEAD("A|Y ")
	TT_LINE("0|1|15")
	TT_LINE("1|0|23")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_7407_GATE, 1, 1, "")
{
	TT_HEAD("A|Y ")
	TT_LINE("0|0|15")
	TT_LINE("1|1|23")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_7408_GATE, 2, 1, "")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|0|15")
	TT_LINE("X,0|0|15")
	TT_LINE("1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7408_AND, 2, 1, "+A,+B,@VCC,@GND")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|0|15")
	TT_LINE("X,0|0|15")
	TT_LINE("1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7410_NAND, 3, 1, "+A,+B,+C,@VCC,@GND")
{
	TT_HEAD("A,B,C|Q ")
	TT_LINE("0,X,X|1|22")
	TT_LINE("X,0,X|1|22")
	TT_LINE("X,X,0|1|22")
	TT_LINE("1,1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7410_GATE, 3, 1, "")
{
	TT_HEAD("A,B,C|Q ")
	TT_LINE("0,X,X|1|22")
	TT_LINE("X,0,X|1|22")
	TT_LINE("X,X,0|1|22")
	TT_LINE("1,1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7411_AND, 3, 1, "+A,+B,+C,@VCC,@GND")
{
	TT_HEAD("A,B,C|Q ")
	TT_LINE("0,X,X|0|15")
	TT_LINE("X,0,X|0|15")
	TT_LINE("X,X,0|0|15")
	TT_LINE("1,1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7411_GATE, 3, 1, "")
{
	TT_HEAD("A,B,C|Q ")
	TT_LINE("0,X,X|0|15")
	TT_LINE("X,0,X|0|15")
	TT_LINE("X,X,0|0|15")
	TT_LINE("1,1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7416_GATE, 1, 1, "")
{
	TT_HEAD("A|Q")
	TT_LINE("0|1|15")
	TT_LINE("1|0|23")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_7417_GATE, 1, 1, "")
{
	TT_HEAD("A|Q")
	TT_LINE("0|0|15")
	TT_LINE("1|1|23")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_7420_NAND, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
{
	TT_HEAD("A,B,C,D|Q ")
	TT_LINE("0,X,X,X|1|22")
	TT_LINE("X,0,X,X|1|22")
	TT_LINE("X,X,0,X|1|22")
	TT_LINE("X,X,X,0|1|22")
	TT_LINE("1,1,1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7421_AND, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
{
	TT_HEAD("A,B,C,D|Q ")
	TT_LINE("0,X,X,X|0|22")
	TT_LINE("X,0,X,X|0|22")
	TT_LINE("X,X,0,X|0|22")
	TT_LINE("X,X,X,0|0|22")
	TT_LINE("1,1,1,1|1|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7425_NOR, 4, 1, "+A,+B,+C,+D,@VCC,@GND")
{
	TT_HEAD("A,B,C,D|Q ")
	TT_LINE("1,X,X,X|0|15")
	TT_LINE("X,1,X,X|0|15")
	TT_LINE("X,X,1,X|0|15")
	TT_LINE("X,X,X,1|0|15")
	TT_LINE("0,0,0,0|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7427_NOR, 3, 1, "+A,+B,+C,@VCC,@GND")
{
	TT_HEAD("A,B,C|Q ")
	TT_LINE("1,X,X|0|15")
	TT_LINE("X,1,X|0|15")
	TT_LINE("X,X,1|0|15")
	TT_LINE("0,0,0|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7430_NAND, 8, 1, "+A,+B,+C,+D,+E,+F,+G,+H,@VCC,@GND")
{
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
}

static TRUTH_TABLE(TTL_7432_OR, 2, 1, "+A,+B,@VCC,@GND")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("1,X|1|22")
	TT_LINE("X,1|1|22")
	TT_LINE("0,0|0|15")
	TT_FAMILY("74XX")
}

/*  FIXME: Same as 7400, but drains higher output currents.
 *         Netlist currently does not model over currents (should it ever?)
 */

static TRUTH_TABLE(TTL_7437_NAND, 2, 1, "+A,+B")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|1|22")
	TT_LINE("X,0|1|22")
	TT_LINE("1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_7438_NAND, 2, 1, "+A,+B")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|1|22")
	TT_LINE("X,0|1|22")
	TT_LINE("1,1|0|15")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_7442, 4, 10, "")
{
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
}

static TRUTH_TABLE(TTL_7486_XOR, 2, 1, "+A,+B,@VCC,@GND")
{
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|0|15")
	TT_LINE("0,1|1|22")
	TT_LINE("1,0|1|22")
	TT_LINE("1,1|0|15")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74139_GATE, 3, 4, "")
{
	TT_HEAD("E,A,B|O0,O1,O2,O3")
	TT_LINE("1,X,X|1,1,1,1|14")
	TT_LINE("0,0,0|0,1,1,1|14")
	TT_LINE("0,0,1|1,0,1,1|14")
	TT_LINE("0,1,0|1,1,0,1|14")
	TT_LINE("0,1,1|1,1,1,0|14")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74147_GATE, 9, 4, "")
{
	TT_HEAD("1,2,3,4,5,6,7,8,9|D,C,B,A")
	TT_LINE("1,1,1,1,1,1,1,1,1|1,1,1,1|10")
	TT_LINE("X,X,X,X,X,X,X,X,0|0,1,1,0|10")
	TT_LINE("X,X,X,X,X,X,X,0,1|0,1,1,1|10")
	TT_LINE("X,X,X,X,X,X,0,1,1|1,0,0,0|10")
	TT_LINE("X,X,X,X,X,0,1,1,1|1,0,0,1|10")
	TT_LINE("X,X,X,X,0,1,1,1,1|1,0,1,0|10")
	TT_LINE("X,X,X,0,1,1,1,1,1|1,0,1,1|10")
	TT_LINE("X,X,0,1,1,1,1,1,1|1,1,0,0|10")
	TT_LINE("X,0,1,1,1,1,1,1,1|1,1,0,1|10")
	TT_LINE("0,1,1,1,1,1,1,1,1|1,1,1,0|10")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74148_GATE, 9, 5, "")
{
	TT_HEAD("EI,0,1,2,3,4,5,6,7|A2,A1,A0,GS,EO")
	TT_LINE( "1,X,X,X,X,X,X,X,X|1,1,1,1,1|10")
	TT_LINE( "0,1,1,1,1,1,1,1,1|1,1,1,1,0|10")
	TT_LINE( "0,X,X,X,X,X,X,X,0|0,0,0,0,1|10")
	TT_LINE( "0,X,X,X,X,X,X,0,1|0,0,1,0,1|10")
	TT_LINE( "0,X,X,X,X,X,0,1,1|0,1,0,0,1|10")
	TT_LINE( "0,X,X,X,X,0,1,1,1|0,1,1,0,1|10")
	TT_LINE( "0,X,X,X,0,1,1,1,1|1,0,0,0,1|10")
	TT_LINE( "0,X,X,0,1,1,1,1,1|1,0,1,0,1|10")
	TT_LINE( "0,X,0,1,1,1,1,1,1|1,1,0,0,1|10")
	TT_LINE( "0,0,1,1,1,1,1,1,1|1,1,1,0,1|10")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74151_GATE, 12, 2, "")
{
	TT_HEAD("C,B,A,G,D0,D1,D2,D3,D4,D5,D6,D7|Y,W")
	TT_LINE( "X,X,X,1,X,X,X,X,X,X,X,X|0,1|12,10")
	TT_LINE( "0,0,0,0,1,X,X,X,X,X,X,X|1,0|12,10")
	TT_LINE( "0,0,0,0,0,X,X,X,X,X,X,X|0,1|12,10")
	TT_LINE( "0,0,1,0,X,1,X,X,X,X,X,X|1,0|12,10")
	TT_LINE( "0,0,1,0,X,0,X,X,X,X,X,X|0,1|12,10")
	TT_LINE( "0,1,0,0,X,X,1,X,X,X,X,X|1,0|12,10")
	TT_LINE( "0,1,0,0,X,X,0,X,X,X,X,X|0,1|12,10")
	TT_LINE( "0,1,1,0,X,X,X,1,X,X,X,X|1,0|12,10")
	TT_LINE( "0,1,1,0,X,X,X,0,X,X,X,X|0,1|12,10")
	TT_LINE( "1,0,0,0,X,X,X,X,1,X,X,X|1,0|12,10")
	TT_LINE( "1,0,0,0,X,X,X,X,0,X,X,X|0,1|12,10")
	TT_LINE( "1,0,1,0,X,X,X,X,X,1,X,X|1,0|12,10")
	TT_LINE( "1,0,1,0,X,X,X,X,X,0,X,X|0,1|12,10")
	TT_LINE( "1,1,0,0,X,X,X,X,X,X,1,X|1,0|12,10")
	TT_LINE( "1,1,0,0,X,X,X,X,X,X,0,X|0,1|12,10")
	TT_LINE( "1,1,1,0,X,X,X,X,X,X,X,1|1,0|12,10")
	TT_LINE( "1,1,1,0,X,X,X,X,X,X,X,0|0,1|12,10")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74155A_GATE, 4, 4, "")
{
	TT_HEAD("B,A,G,C|Y0,Y1,Y2,Y3")
	TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
	TT_LINE("X,X,0,0|1,1,1,1|13,13,13,13")
	TT_LINE("0,0,0,1|0,1,1,1|13,13,13,13")
	TT_LINE("0,1,0,1|1,0,1,1|13,13,13,13")
	TT_LINE("1,0,0,1|1,1,0,1|13,13,13,13")
	TT_LINE("1,1,0,1|1,1,1,0|13,13,13,13")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74155B_GATE, 4, 4, "")
{
	TT_HEAD("B,A,G,C|Y0,Y1,Y2,Y3")
	TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
	TT_LINE("X,X,0,1|1,1,1,1|13,13,13,13")
	TT_LINE("0,0,0,0|0,1,1,1|13,13,13,13")
	TT_LINE("0,1,0,0|1,0,1,1|13,13,13,13")
	TT_LINE("1,0,0,0|1,1,0,1|13,13,13,13")
	TT_LINE("1,1,0,0|1,1,1,0|13,13,13,13")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74156A_GATE, 4, 4, "")
{
	TT_HEAD("B,A,G,C|Y0,Y1,Y2,Y3")
	TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
	TT_LINE("X,X,0,0|1,1,1,1|13,13,13,13")
	TT_LINE("0,0,0,1|0,1,1,1|13,13,13,13")
	TT_LINE("0,1,0,1|1,0,1,1|13,13,13,13")
	TT_LINE("1,0,0,1|1,1,0,1|13,13,13,13")
	TT_LINE("1,1,0,1|1,1,1,0|13,13,13,13")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_74156B_GATE, 4, 4, "")
{
	TT_HEAD("B,A,G,C|Y0,Y1,Y2,Y3")
	TT_LINE("X,X,1,X|1,1,1,1|13,13,13,13")
	TT_LINE("X,X,0,1|1,1,1,1|13,13,13,13")
	TT_LINE("0,0,0,0|0,1,1,1|13,13,13,13")
	TT_LINE("0,1,0,0|1,0,1,1|13,13,13,13")
	TT_LINE("1,0,0,0|1,1,0,1|13,13,13,13")
	TT_LINE("1,1,0,0|1,1,1,0|13,13,13,13")
	//  Open Collector
	TT_FAMILY("74XXOC")
}

static TRUTH_TABLE(TTL_74157_GATE, 4, 1, "")
{
	TT_HEAD("E,S,I,J|O")
	TT_LINE("1,X,X,X|0|14")
	TT_LINE("0,1,X,0|0|14")
	TT_LINE("0,1,X,1|1|14")
	TT_LINE("0,0,0,X|0|14")
	TT_LINE("0,0,1,X|1|14")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74260_NOR, 5, 1, "+A,+B,+C,+D,+E,@VCC,@GND")
{
	TT_HEAD("A,B,C,D,E|Q")
	TT_LINE("0,0,0,0,0|1|10")
	TT_LINE("X,X,X,X,1|0|12")
	TT_LINE("X,X,X,1,X|0|12")
	TT_LINE("X,X,1,X,X|0|12")
	TT_LINE("X,1,X,X,X|0|12")
	TT_LINE("1,X,X,X,X|0|12")
	TT_FAMILY("74XX")
}

// FIXME: We need "private" devices
static TRUTH_TABLE(TTL_74279A, 3, 1, "")
{
	TT_HEAD("S,R,_Q|Q")
	TT_LINE("0,X,X|1|22")
	TT_LINE("1,0,X|0|27")
	TT_LINE("1,1,0|0|27")
	TT_LINE("1,1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74279B, 4, 1, "")
{
	TT_HEAD("S1,S2,R,_Q|Q")
	TT_LINE("0,X,X,X|1|22")
	TT_LINE("X,0,X,X|1|22")
	TT_LINE("1,1,0,X|0|27")
	TT_LINE("1,1,1,0|0|27")
	TT_LINE("1,1,1,1|1|22")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_74368_GATE, 5, 4, "")
{
	TT_HEAD("OE,A1,A2,A3,A4|Y1,Y2,Y3,Y4")
	TT_LINE("1,X,X,X,X|0,0,0,0|12")
	TT_LINE("0,0,0,0,0|1,1,1,1|12")
	TT_LINE("0,0,0,0,1|1,1,1,0|12")
	TT_LINE("0,0,0,1,0|1,1,0,1|12")
	TT_LINE("0,0,0,1,1|1,1,0,0|12")
	TT_LINE("0,0,1,0,0|1,0,1,1|12")
	TT_LINE("0,0,1,0,1|1,0,1,0|12")
	TT_LINE("0,0,1,1,0|1,0,0,1|12")
	TT_LINE("0,0,1,1,1|1,0,0,0|12")
	TT_LINE("0,1,0,0,0|0,1,1,1|12")
	TT_LINE("0,1,0,0,1|0,1,1,0|12")
	TT_LINE("0,1,0,1,0|0,1,0,1|12")
	TT_LINE("0,1,0,1,1|0,1,0,0|12")
	TT_LINE("0,1,1,0,0|0,0,1,1|12")
	TT_LINE("0,1,1,0,1|0,0,1,0|12")
	TT_LINE("0,1,1,1,0|0,0,0,1|12")
	TT_LINE("0,1,1,1,1|0,0,0,0|12")
	TT_FAMILY("74XX")
}

static TRUTH_TABLE(TTL_9312, 12, 2,
				   "+A,+B,+C,+G,+D0,+D1,+D2,+D3,+D4,+D5,+D6,+D7,@VCC,@GND")
{
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
}

NETLIST_START(ttl74xx_lib)
{
	NET_MODEL(
		"DM7414         SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL(
		"TTL_7414_GATE  SCHMITT_TRIGGER(VTP=1.7 VTM=0.9 VI=4.35 RI=6.15k VOH=3.5 ROH=120 VOL=0.1 ROL=37.5 TPLH=15 TPHL=15)")
	NET_MODEL(
		"DM74LS14       SCHMITT_TRIGGER(VTP=1.6 VTM=0.8 VI=4.4 RI=19.3k VOH=3.45 ROH=130 VOL=0.1 ROL=31.2 TPLH=15 TPHL=15)")
	// NET_MODEL("DM7414 FAMILY(IVL=0.16 IVH=0.4 OVL=0.1 OVH=0.05 ORL=10.0
	// ORH=1.0e8)")

	TRUTHTABLE_ENTRY(TTL_7400_NAND)
	TRUTHTABLE_ENTRY(TTL_7402_NOR)
	TRUTHTABLE_ENTRY(TTL_7404_INVERT)
	TRUTHTABLE_ENTRY(TTL_7406_GATE)
	TRUTHTABLE_ENTRY(TTL_7407_GATE)
	TRUTHTABLE_ENTRY(TTL_7408_GATE)
	TRUTHTABLE_ENTRY(TTL_7408_AND)
	TRUTHTABLE_ENTRY(TTL_7410_NAND)
	TRUTHTABLE_ENTRY(TTL_7410_GATE)
	TRUTHTABLE_ENTRY(TTL_7411_AND)
	TRUTHTABLE_ENTRY(TTL_7411_GATE)
	TRUTHTABLE_ENTRY(TTL_7416_GATE)
	TRUTHTABLE_ENTRY(TTL_7417_GATE)
	TRUTHTABLE_ENTRY(TTL_7420_NAND)
	TRUTHTABLE_ENTRY(TTL_7421_AND)
	TRUTHTABLE_ENTRY(TTL_7425_NOR)
	TRUTHTABLE_ENTRY(TTL_7427_NOR)
	TRUTHTABLE_ENTRY(TTL_7430_NAND)
	TRUTHTABLE_ENTRY(TTL_7432_OR)
	TRUTHTABLE_ENTRY(TTL_7437_NAND)
	TRUTHTABLE_ENTRY(TTL_7438_NAND)
	TRUTHTABLE_ENTRY(TTL_7442)
	TRUTHTABLE_ENTRY(TTL_7486_XOR)
	TRUTHTABLE_ENTRY(TTL_74139_GATE)
	TRUTHTABLE_ENTRY(TTL_74147_GATE)
	TRUTHTABLE_ENTRY(TTL_74148_GATE)
	TRUTHTABLE_ENTRY(TTL_74151_GATE)
	TRUTHTABLE_ENTRY(TTL_74155A_GATE)
	TRUTHTABLE_ENTRY(TTL_74155B_GATE)
	TRUTHTABLE_ENTRY(TTL_74156A_GATE)
	TRUTHTABLE_ENTRY(TTL_74156B_GATE)
	TRUTHTABLE_ENTRY(TTL_74157_GATE)
	TRUTHTABLE_ENTRY(TTL_74260_NOR)
	TRUTHTABLE_ENTRY(TTL_74279A)
	TRUTHTABLE_ENTRY(TTL_74279B)
	TRUTHTABLE_ENTRY(TTL_74368_GATE)
	TRUTHTABLE_ENTRY(TTL_9312)

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
	LOCAL_LIB_ENTRY(TTL_7417_DIP)
	LOCAL_LIB_ENTRY(TTL_7420_DIP)
	LOCAL_LIB_ENTRY(TTL_7421_DIP)
	LOCAL_LIB_ENTRY(TTL_7425_DIP)
	LOCAL_LIB_ENTRY(TTL_7427_DIP)
	LOCAL_LIB_ENTRY(TTL_7430_DIP)
	LOCAL_LIB_ENTRY(TTL_7432_DIP)
	LOCAL_LIB_ENTRY(TTL_7437_DIP)
	LOCAL_LIB_ENTRY(TTL_7438_DIP)
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
	LOCAL_LIB_ENTRY(TTL_74139_DIP)
	LOCAL_LIB_ENTRY(TTL_74147_DIP)
	LOCAL_LIB_ENTRY(TTL_74148_DIP)
	LOCAL_LIB_ENTRY(TTL_74151_DIP)
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
	LOCAL_LIB_ENTRY(TTL_74175_DIP)
	LOCAL_LIB_ENTRY(TTL_74192_DIP)
	LOCAL_LIB_ENTRY(TTL_74193_DIP)
	LOCAL_LIB_ENTRY(TTL_74194_DIP)
	LOCAL_LIB_ENTRY(TTL_74260_DIP)
	LOCAL_LIB_ENTRY(TTL_74279_DIP)
	LOCAL_LIB_ENTRY(TTL_74290_DIP)
	LOCAL_LIB_ENTRY(TTL_74293_DIP)
	LOCAL_LIB_ENTRY(TTL_74365_DIP)
	LOCAL_LIB_ENTRY(TTL_74368_DIP)
	LOCAL_LIB_ENTRY(TTL_74377_DIP)
	LOCAL_LIB_ENTRY(TTL_74378_DIP)
	LOCAL_LIB_ENTRY(TTL_74379_DIP)
	LOCAL_LIB_ENTRY(TTL_74393_DIP)
	LOCAL_LIB_ENTRY(SN74LS629_DIP)
	LOCAL_LIB_ENTRY(TTL_9310_DIP)
	LOCAL_LIB_ENTRY(TTL_9312_DIP)
	LOCAL_LIB_ENTRY(TTL_9314_DIP)
	LOCAL_LIB_ENTRY(TTL_9316_DIP)
	LOCAL_LIB_ENTRY(TTL_9321_DIP)
	LOCAL_LIB_ENTRY(TTL_9322_DIP)
	LOCAL_LIB_ENTRY(TTL_9334_DIP)
	LOCAL_LIB_ENTRY(TTL_8277_DIP)
	LOCAL_LIB_ENTRY(TTL_AM2847_DIP)
}
