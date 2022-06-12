// license:CC0
// copyright-holders:Couriersud

#include "devices/net_lib.h"

//- Identifier: CD4001_DIP
//- Title: CD4001BM/CD4001BC Quad 2-Input NOR Buffered B Series Gate
//- Pinalias: A1,B1,Y1,Y2,A2,B2,VSS,A3,B3,Y3,Y4,A4,B4,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/166/108518_DS.pdf
//-
//-     +---+---++---+
//-     | A | B || Y |
//-     +===+===++===+
//-     | 0 | 0 || 1 |
//-     | X | 1 || 0 |
//-     | 1 | X || 0 |
//-     +---+---++---+
//-
static NETLIST_START(CD4001_DIP)
	CD4001_GATE(A)
	CD4001_GATE(B)
	CD4001_GATE(C)
	CD4001_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*  A1 |1     ++    14| VDD */ A.VDD,
		  A.B, /*  B1 |2           13| B4  */ D.B,
		  A.Q, /*  Y1 |3           12| A4  */ D.A,
		  B.Q, /*  Y2 |4    4001   11| Y4  */ D.Q,
		  B.A, /*  A2 |5           10| Y3  */ C.Q,
		  B.B, /*  B2 |6            9| B3  */ C.B,
		A.VSS, /* VSS |7            8| A3  */ C.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4006_DIP
//- Title: CD4006BM/CD4006BC 18-Stage Static Shift Register
//- Pinalias: D1,NC,CLOCK,D2,D3,D4,VSS,D4P4,D4P5,D3P4,D2P4,D2P5,D1P4,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS005942.PDF
//-
static NETLIST_START(CD4006_DIP)
	CD4006(A)
	NC_PIN(NC)

	DIPPINS(     /*       +--------------+      */
		   A.D1, /*    D1 |1     ++    14| VDD  */ A.VDD,
		   NC.I, /*    NC |2           13| D1+4 */ A.D1P4,
		A.CLOCK, /* CLOCK |3           12| D2+5 */ A.D2P5,
		   A.D2, /*    D2 |4    4006   11| D2+4 */ A.D2P4,
		   A.D3, /*    D3 |5           10| D3+4 */ A.D3P4,
		   A.D4, /*    D4 |6            9| D4+5 */ A.D4P5,
		  A.VSS, /*   VSS |7            8| D4+4 */ A.D4P4
				 /*       +--------------+      */
	)
NETLIST_END()

//- Identifier: CD4011_DIP
//- Title: CD4011BM/CD4011BC Quad 2-Input NAND Buffered B Series Gate
//- Pinalias: A1,B1,Y1,Y2,A2,B2,VSS,A3,B3,Y3,Y4,A4,B4,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/166/108518_DS.pdf
//-
//-     +---+---++---+
//-     | A | B || Y |
//-     +===+===++===+
//-     | X | 0 || 1 |
//-     | 0 | X || 1 |
//-     | 1 | 1 || 0 |
//-     +---+---++---+
//-
static NETLIST_START(CD4011_DIP)
	   CD4011_GATE(A)
	   CD4011_GATE(B)
	   CD4011_GATE(C)
	   CD4011_GATE(D)

	   NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	   NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	   DIPPINS(    /*     +--------------+     */
			  A.A, /*   A |1     ++    14| VDD */ A.VDD,
			  A.B, /*   B |2           13| H   */ D.B,
			  A.Q, /*   J |3           12| G   */ D.A,
			  B.Q, /*   K |4    4011   11| M   */ D.Q,
			  B.A, /*   C |5           10| L   */ C.Q,
			  B.B, /*   D |6            9| F   */ C.B,
			A.VSS, /* VSS |7            8| E   */ C.A
				   /*     +--------------+     */
	   )
NETLIST_END()

//- Identifier: CD4013_DIP
//- Title: CD4013BM/CD4013BC Dual D Flip-Flop
//- Pinalias: Q1,QQ1,CLOCK1,RESET1,DATA1,SET1,VSS,SET2,DATA2,RESET2,CLOCK2,QQ2,Q2,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/150/108670_DS.pdf
//-
//-     +-----+---+---+---++---+----+
//-     | CLK | D | R | S || Q | QQ |
//-     +=====+===+===+===++===+====+
//-     | 0-1 | 0 | 0 | 0 || 0 |  1 |
//-     | 0-1 | 1 | 0 | 0 || 1 |  0 |
//-     | 1-0 | X | 0 | 0 || Q | QQ |
//-     |  X  | X | 1 | 0 || 0 |  1 |
//-     |  X  | X | 0 | 1 || 1 |  0 |
//-     |  X  | X | 1 | 1 || 1 |  1 |
//-     +-----+---+---+---++---+----+
//-
static NETLIST_START(CD4013_DIP)
	CD4013(A)
	CD4013(B)

	NET_C(A.VDD, B.VDD)
	NET_C(A.VSS, B.VSS)

	DIPPINS(     /*         +--------------+        */
			A.Q, /*      Q1 |1     ++    14| VDD    */ A.VDD,
		   A.QQ, /*     Q1Q |2           13| Q2     */ B.Q,
		A.CLOCK, /*  CLOCK1 |3           12| Q2Q    */ B.QQ,
		A.RESET, /*  RESET1 |4    4013   11| CLOCK2 */ B.CLOCK,
		 A.DATA, /*   DATA1 |5           10| RESET2 */ B.RESET,
		  A.SET, /*    SET1 |6            9| DATA2  */ B.DATA,
		  A.VSS, /*     VSS |7            8| SET2   */ B.SET
				 /*         +--------------+        */
	)
NETLIST_END()

//- Identifier: CD4016_DIP
//- Title: CD4016BM/CD4016BC Quad Bilateral Switch
//- Pinalias: INOUTA,OUTINA,OUTINB,INOUTB,CONTROLB,CONTROLC,VSS,INOUTC,OUTINC,OUTIND,INOUTD,CONTROLD,CONTROLA,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/185/108711_DS.pdf
//-
static NETLIST_START(CD4016_DIP)
	CD4066_GATE(A)
	CD4066_GATE(B)
	CD4066_GATE(C)
	CD4066_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	PARAM(A.BASER, 1000.0)
	PARAM(B.BASER, 1000.0)
	PARAM(C.BASER, 1000.0)
	PARAM(D.BASER, 1000.0)

	DIPPINS(        /*          +--------------+          */
		A.R.1,      /*   INOUTA |1     ++    14| VDD      */ A.VDD,
		A.R.2,      /*   OUTINA |2           13| CONTROLA */ A.CTL,
		B.R.1,      /*   OUTINB |3           12| CONTROLD */ D.CTL,
		B.R.2,      /*   INOUTB |4    4016   11| INOUTD   */ D.R.1,
		B.CTL,      /* CONTROLB |5           10| OUTIND   */ D.R.2,
		C.CTL,      /* CONTROLC |6            9| OUTINC   */ C.R.1,
		A.VSS,      /*      VSS |7            8| INOUTC   */ C.R.2
					/*          +--------------+          */
	)
NETLIST_END()

//- Identifier: CD4017_DIP
//- Title: CD4017BM/CD4017BC Decade Counter/Divider with 10 Decoded Outputs
//- Pinalias: Q5,Q1,Q0,Q2,Q6,Q7,Q3,VSS,Q8,Q4,Q9,CARRY_OUT,CLOCK_ENABLE,CLOCK,RESET,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/166/108736_DS.pdf
//-
static NETLIST_START(CD4017_DIP)
	CD4017(A)

	DIPPINS(   /*     +--------------+              */
		 A.Q5, /*  Q5 |1     ++    16| VDD          */ A.VDD,
		 A.Q1, /*  Q1 |2           15| RESET        */ A.RESET,
		 A.Q0, /*  Q0 |3           14| CLOCK        */ A.CLK,
		 A.Q2, /*  Q2 |4    4017   13| CLOCK ENABLE */ A.CLKEN,
		 A.Q6, /*  Q6 |5           12| CARRY OUT    */ A.CO,
		 A.Q7, /*  Q7 |6           11| Q9           */ A.Q9,
		 A.Q3, /*  Q3 |7           10| Q4           */ A.Q4,
		A.VSS, /* VSS |8            9| Q8           */ A.Q8
			   /*     +--------------+              */
	)
NETLIST_END()

//- Identifier: CD4020_DIP
//- Title: CD4020BC 14-Stage Ripple Carry Binary Counters
//- Pinalias: Q12,Q13,Q14,Q6,Q5,Q7,Q4,VSS,Q1,PHI1,RESET,Q9,Q8,Q10,Q11,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/90/109006_DS.pdf
//-
static NETLIST_START(CD4020_DIP)
	CD4020(A)

	DIPPINS(   /*     +--------------+       */
		A.Q12, /* Q12 |1     ++    16| VDD   */ A.VDD,
		A.Q13, /* Q13 |2           15| Q11   */ A.Q11,
		A.Q14, /* Q14 |3           14| Q10   */ A.Q10,
		 A.Q6, /*  Q6 |4    4020   13| Q8    */ A.Q8,
		 A.Q5, /*  Q5 |5           12| Q9    */ A.Q9,
		 A.Q7, /*  Q7 |6           11| RESET */ A.RESET,
		 A.Q4, /*  Q4 |7           10| PHI1  */ A.IP,
		A.VSS, /* VSS |8            9| Q1    */ A.Q1
			   /*     +--------------+       */)
NETLIST_END()

//- Identifier: CD4022_DIP
//- Title: CD4022BM/CD4022BC Divide-by-8 Counter/Divider with 8 Decoded Outputs
//- Pinalias: Q1,Q0,Q2,Q5,Q6,NC,Q3,VSS,NC,Q7,Q4,CARRY_OUT,CLOCK_ENABLE,CLOCK,RESET,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/166/108736_DS.pdf
//-
static NETLIST_START(CD4022_DIP)
	CD4022(A)
	NC_PIN(NC)

	DIPPINS(   /*     +--------------+              */
		 A.Q1, /*  Q1 |1     ++    16| VDD          */ A.VDD,
		 A.Q0, /*  Q0 |2           15| RESET        */ A.RESET,
		 A.Q2, /*  Q2 |3           14| CLOCK        */ A.CLK,
		 A.Q5, /*  Q5 |4    4022   13| CLOCK ENABLE */ A.CLKEN,
		 A.Q6, /*  Q6 |5           12| CARRY OUT    */ A.CO,
		 NC.I, /*  NC |6           11| Q4           */ A.Q4,
		 A.Q3, /*  Q3 |7           10| Q7           */ A.Q7,
		A.VSS, /* VSS |8            9| NC           */ NC.I
			   /*     +--------------+              */
	)
NETLIST_END()

//- Identifier: CD4024_DIP
//- Title: CD4024BM/CD4024BC 7-Stage Ripple Carry Binary Counter
//- Pinalias: IP,RESET,Q7,Q6,Q5,Q4,VSS,NC,Q3,NC,Q2,Q1,NC,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/120/108894_DS.pdf
//-
static NETLIST_START(CD4024_DIP)
	CD4024(A)
	NC_PIN(NC)

	DIPPINS(     /*       +--------------+     */
		   A.IP, /*    IP |1     ++    14| VDD */ A.VDD,
		A.RESET, /* RESET |2           13| NC  */ NC.I,
		   A.Q7, /*    Q7 |3           12| Q1  */ A.Q1,
		   A.Q6, /*    Q6 |4    4024   11| Q2  */ A.Q2,
		   A.Q5, /*    Q5 |5           10| NC  */ NC.I,
		   A.Q4, /*    Q4 |6            9| Q3  */ A.Q3,
		  A.VSS, /*   VSS |7            8| NC  */ NC.I
				 /*       +--------------+     */)
NETLIST_END()

//- Identifier: CD4029_DIP
//- Title: CD4029BM/CD4029BC Presettable Binary/Decade Up/Down Counter
//- Pinalias: PE,Q4,J4,J1,CI,Q1,CO,VSS,BD,UD,Q2,J2,J3,Q3,CLK,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    Voltage-dependent timing is not implemented.
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/166/108968_DS.pdf
//-    See nld_4029.cpp for tables explaining the counting order for all states
//-
//-         Counter Sequences:
//-
//-         B/D high (Binary), U/D high (Up)
//-         +-------++----+----+----+----+----++-------+
//-         | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
//-         +=======++====+====+====+====+====++=======+
//-         |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
//-         |    1  ||  0 |  0 |  0 |  1 |  1 ||     2 |
//-         |    2  ||  0 |  0 |  1 |  0 |  1 ||     3 |
//-         |    3  ||  0 |  0 |  1 |  1 |  1 ||     4 |
//-         |    4  ||  0 |  1 |  0 |  0 |  1 ||     5 |
//-         |    5  ||  0 |  1 |  0 |  1 |  1 ||     6 |
//-         |    6  ||  0 |  1 |  1 |  0 |  1 ||     7 |
//-         |    7  ||  0 |  1 |  1 |  1 |  1 ||     8 |
//-         |    8  ||  1 |  0 |  0 |  0 |  1 ||     9 |
//-         |    9  ||  1 |  0 |  0 |  1 |  1 ||     A |
//-         |    A  ||  1 |  0 |  1 |  0 |  1 ||     B |
//-         |    B  ||  1 |  0 |  1 |  1 |  1 ||     C |
//-         |    C  ||  1 |  1 |  0 |  0 |  1 ||     D |
//-         |    D  ||  1 |  1 |  0 |  1 |  1 ||     E |
//-         |    E  ||  1 |  1 |  1 |  0 |  1 ||     F |
//-         |    F  ||  1 |  1 |  1 |  1 |0|CI||     0 |
//-         |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
//-         +-------++----+----+----+----+----++-------+
//-
//-         B/D high (Binary), U/D low (Down)
//-         +-------++----+----+----+----+----++-------+
//-         | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
//-         +=======++====+====+====+====+====++=======+
//-         |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
//-         |    E  ||  1 |  1 |  1 |  0 |  1 ||     D |
//-         |    D  ||  1 |  1 |  0 |  1 |  1 ||     C |
//-         |    C  ||  1 |  1 |  0 |  0 |  1 ||     B |
//-         |    B  ||  1 |  0 |  1 |  1 |  1 ||     A |
//-         |    A  ||  1 |  0 |  1 |  0 |  1 ||     9 |
//-         |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
//-         |    8  ||  1 |  0 |  0 |  0 |  1 ||     7 |
//-         |    7  ||  0 |  1 |  1 |  1 |  1 ||     6 |
//-         |    6  ||  0 |  1 |  1 |  0 |  1 ||     5 |
//-         |    5  ||  0 |  1 |  0 |  1 |  1 ||     4 |
//-         |    4  ||  0 |  1 |  0 |  0 |  1 ||     3 |
//-         |    3  ||  0 |  0 |  1 |  1 |  1 ||     2 |
//-         |    2  ||  0 |  0 |  1 |  0 |  1 ||     1 |
//-         |    1  ||  0 |  0 |  0 |  1 |  1 ||     0 |
//-         |    0  ||  0 |  0 |  0 |  0 |0|CI||     F |
//-         |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
//-         +-------++----+----+----+----+----++-------+
//-
//-         B/D low (Decimal), U/D high (Up)
//-         +-------++----+----+----+----+----++-------+
//-         | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
//-         +=======++====+====+====+====+====++=======+
//-         |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
//-         |    1  ||  0 |  0 |  0 |  1 |  1 ||     2 |
//-         |    2  ||  0 |  0 |  1 |  0 |  1 ||     3 |
//-         |    3  ||  0 |  0 |  1 |  1 |  1 ||     4 |
//-         |    4  ||  0 |  1 |  0 |  0 |  1 ||     5 |
//-         |    5  ||  0 |  1 |  0 |  1 |  1 ||     6 |
//-         |    6  ||  0 |  1 |  1 |  0 |  1 ||     7 |
//-         |    7  ||  0 |  1 |  1 |  1 |  1 ||     8 |
//-         |    8  ||  1 |  0 |  0 |  0 |  1 ||     9 |
//-         |    9  ||  1 |  0 |  0 |  1 |0|CI||     0 |
//-         |    A  ||  1 |  0 |  1 |  0 |  1 ||     B |
//-         |    B  ||  1 |  0 |  1 |  1 |0|CI||     6 |
//-         |    C  ||  1 |  1 |  0 |  0 |  1 ||     D |
//-         |    D  ||  1 |  1 |  0 |  1 |0|CI||     4 |
//-         |    E  ||  1 |  1 |  1 |  0 |  1 ||     F |
//-         |    F  ||  1 |  1 |  1 |  1 |0|CI||     2 |
//-         |    0  ||  0 |  0 |  0 |  0 |  1 ||     1 |
//-         +-------++----+----+----+----+----++-------+
//-
//-         B/D low (Decimal), U/D low (Down)
//-         +-------++----+----+----+----+----++-------+
//-         | COUNT || Q4 | Q3 | Q2 | Q1 | CO ||  NEXT |
//-         +=======++====+====+====+====+====++=======+
//-         |    F  ||  1 |  1 |  1 |  1 |  1 ||     E |
//-         |    E  ||  1 |  1 |  1 |  0 |  1 ||     D |
//-         |    D  ||  1 |  1 |  0 |  1 |  1 ||     C |
//-         |    C  ||  1 |  1 |  0 |  0 |  1 ||     B |
//-         |    B  ||  1 |  0 |  1 |  1 |  1 ||     A |
//-         |    A  ||  1 |  0 |  1 |  0 |  1 ||     9 |
//-         |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
//-         |    8  ||  1 |  0 |  0 |  0 |  1 ||     7 |
//-         |    7  ||  0 |  1 |  1 |  1 |  1 ||     6 |
//-         |    6  ||  0 |  1 |  1 |  0 |  1 ||     5 |
//-         |    5  ||  0 |  1 |  0 |  1 |  1 ||     4 |
//-         |    4  ||  0 |  1 |  0 |  0 |  1 ||     3 |
//-         |    3  ||  0 |  0 |  1 |  1 |  1 ||     2 |
//-         |    2  ||  0 |  0 |  1 |  0 |  1 ||     1 |
//-         |    1  ||  0 |  0 |  0 |  1 |  1 ||     0 |
//-         |    0  ||  0 |  0 |  0 |  0 |0|CI||     9 |
//-         |    9  ||  1 |  0 |  0 |  1 |  1 ||     8 |
//-         +-------++----+----+----+----+----++-------+
//-
//-         Note that in all cases where Carry-out is generating a non-1 signal (0 | Carry-in),
//-         this signal is asynchronous, so if carry-in changes the carry-out value will
//-         change immediately.
//-
//-         Preset/Carry in function table
//-         +-----+-----+-----++----+----+----+----+
//-         | PE  | CLK | CI  || Q1 | Q2 | Q3 | Q4 |
//-         +=====+=====+=====++====+====+====+====+
//-         |  1  |  X  |  X  || J1 | J2 | J3 | J4 |
//-         |  0  |  X  |  1  || Q1 | Q2 | Q3 | Q4 |
//-         |  0  | ./` |  0  ||       COUNT       |
//-         +-----+-----+-----++----+----+----+----+
//-
static NETLIST_START(CD4029_DIP)
	CD4029(A)

	DIPPINS(     /*     +--------------+     */
		   A.PE, /*  PE |1     ++    16| VDD */ A.VDD,
		   A.Q4, /*  Q4 |2           15| CLK */ A.CLK,
		   A.J4, /*  J4 |3           14| Q3  */ A.Q3,
		   A.J1, /*  J1 |4    4029   13| J3  */ A.J3,
		   A.CI, /*  CI |5           12| J2  */ A.J2,
		   A.Q1, /*  Q1 |6           11| Q2  */ A.Q2,
		   A.CO, /*  CO |7           10| U/D */ A.UD,
		  A.VSS, /* VSS |8            9| B/D */ A.BD
				 /*     +--------------+  */)
NETLIST_END()

//- Identifier: CD4030_DIP
//- Title: CD4030M/CD4030C Quad EXCLUSIVE-OR Gate
//- Pinalias: A1,B1,Y1,Y2,A2,B2,VSS,A3,B3,Y3,Y4,A4,B4,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    Voltage-dependent timing is not implemented.
//- FunctionTable:
//-    https://www.uni-kl.de/elektronik-lager/418055
//-
static NETLIST_START(CD4030_DIP)
	   CD4030_GATE(A)
	   CD4030_GATE(B)
	   CD4030_GATE(C)
	   CD4030_GATE(D)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*  A1 |1     ++    14| VDD */ A.VDD,
		  A.B, /*  B1 |2           13| B4  */ D.B,
		  A.Q, /*  Y1 |3           12| A4  */ D.A,
		  B.Q, /*  Y2 |4    4030   11| Y4  */ D.Q,
		  B.A, /*  A2 |5           10| Y3  */ C.Q,
		  B.B, /*  B2 |6            9| B3  */ C.B,
		A.VSS, /* VSS |7            8| A3  */ C.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4042_DIP
//- Title: CD4042BM/CD4042BC Quad Clocked D Latch
//- Pinalias: Q4,Q1,QQ1,D1,CLK,POL,D2,VSS,QQ2,Q2,Q3,QQ3,D3,D4,QQ4,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    Voltage-dependent timing is partially implemented.
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS005966.PDF
//-
//-         +-----+-----+----++----+-----+
//-         | POL | CLK | Dx || Qx | QQx |
//-         +=====+=====+====++====+=====+
//-         |  0  |  0  |  X || Qx | /Qx |
//-         |  0  |  1  |  D ||  D | /D  |
//-         |  1  |  1  |  X || Qx | /Qx |
//-         |  1  |  0  |  D ||  D | /D  |
//-         +-----+-----+----++----+-----+
//-         Note that since this is a level triggered transparent latch,
//-         as long as POL ^ CLK == true, the latch is transparent, and
//-         if D changes Q and QQ(/Q) will instantly change.
//-
static NETLIST_START(CD4042_DIP)
	CD4042(A)

	DIPPINS(     /*     +--------------+     */
		   A.Q4, /*  Q4 |1     ++    16| VDD */ A.VDD,
		   A.Q1, /*  Q1 |2           15| Q4Q */ A.Q4Q,
		  A.Q1Q, /* Q1Q |3           14| D4  */ A.D4,
		   A.D1, /*  D1 |4    4042   13| D3  */ A.D3,
		  A.CLK, /* CLK |5           12| Q3Q */ A.Q3Q,
		  A.POL, /* POL |6           11| Q3  */ A.Q3,
		   A.D2, /*  D2 |7           10| Q2  */ A.Q2,
		  A.VSS, /* VSS |8            9| Q2Q */ A.Q2Q
				 /*     +--------------+        */
	)
NETLIST_END()

//- Identifier: CD4049_DIP
//- Title: CD4049UBM/CD4049UBC Hex Inverting Buffer
//- Pinalias: VDD,G,A,H,B,I,C,VSS,D,J,E,K,NC,F,L,NC
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    Voltage-dependent timing is not implemented.
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/134/109125_DS.pdf
//-
static NETLIST_START(CD4049_DIP)
	CD4049_GATE(A)
	CD4049_GATE(B)
	CD4049_GATE(C)
	CD4049_GATE(D)
	CD4049_GATE(E)
	CD4049_GATE(F)
	NC_PIN(NC)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD, E.VDD, F.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS, E.VSS, F.VSS)

	//DIPPINS( /*     +--------------+     */
	//  A.VDD, /* VCC |1     ++    16| NC  */ NC.I,
	//    A.G, /*G=/A |2           15| L=/F*/ F.L,
	//    A.A, /*   A |3           14| F   */ F.F,
	//    B.H, /*H=/B |4           13| NC  */ NC.I,
	//    B.B, /*   B |5    4049   12| K=/E*/ E.K,
	//    C.I, /*I=/C |6           11| E   */ E.E,
	//    C.C, /*   C |7           10| J=/D*/ D.J,
	//  A.VSS, /* VSS |8            9| D   */ D.D
	//         /*     +--------------+     */
	//)
	DIPPINS(   /*     +--------------+     */
		A.VDD, /* VCC |1     ++    16| NC  */ NC.I,
		  A.Q, /*G=/A |2           15| L=/F*/ F.Q,
		  A.A, /*   A |3           14| F   */ F.A,
		  B.Q, /*H=/B |4           13| NC  */ NC.I,
		  B.A, /*   B |5    4049   12| K=/E*/ E.Q,
		  C.Q, /*I=/C |6           11| E   */ E.A,
		  C.A, /*   C |7           10| J=/D*/ D.Q,
		A.VSS, /* VSS |8            9| D   */ D.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4053_DIP
//- Title: CD4053BM/CD4053BC Triple 2-Channel AnalogMultiplexer/Demultiplexer
//- Pinalias: INOUTBY,INOUTBX,INOUTCY,OUTINC,INOUTCX,INH,VEE,VSS,C,B,A,INOUTAX,INOUTAY,OUTINA,OUTINB,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS005662.PDF
//-
static NETLIST_START(CD4053_DIP)
	CD4053_GATE(A)
	CD4053_GATE(B)
	CD4053_GATE(C)

	NET_C(A.VEE, B.VEE, C.VEE)
	NET_C(A.VDD, B.VDD, C.VDD)
	NET_C(A.VSS, B.VSS, C.VSS)
	NET_C(A.INH, B.INH, C.INH)

	PARAM(A.BASER, 270.0)
	PARAM(B.BASER, 270.0)
	PARAM(C.BASER, 270.0)

	DIPPINS(   /*         +--------------+         */
		  B.Y, /* INOUTBY |1     ++    16| VDD     */ A.VDD,
		  B.X, /* INOUTBX |2           15| OUTINB  */ B.XY,
		  C.Y, /* INOUTCY |3           14| OUTINA  */ A.XY,
		 C.XY, /*  OUTINC |4    4053   13| INOUTAY */ A.Y,
		  C.X, /* INOUTCX |5           12| INOUTAX */ A.X,
		A.INH, /*     INH |6           11| A       */ A.S,
		A.VEE, /*     VEE |7           10| B       */ B.S,
		A.VSS, /*     VSS |8            9| C       */ C.S
			   /*         +--------------+         */
	)
NETLIST_END()

//- Identifier: CD4066_DIP
//- Title: CD4066BM/CD4066BC Quad Bilateral Switch
//- Pinalias: INOUTA,OUTINA,OUTINB,INOUTB,CONTROLB,CONTROLC,VSS,INOUTC,OUTINC,OUTIND,INOUTD,CONTROLD,CONTROLA,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS005665.PDF
//-
static NETLIST_START(CD4066_DIP)
	CD4066_GATE(A)
	CD4066_GATE(B)
	CD4066_GATE(C)
	CD4066_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	PARAM(A.BASER, 270.0)
	PARAM(B.BASER, 270.0)
	PARAM(C.BASER, 270.0)
	PARAM(D.BASER, 270.0)

	DIPPINS(   /*          +--------------+          */
		A.R.1, /*   INOUTA |1     ++    14| VDD      */ A.VDD,
		A.R.2, /*   OUTINA |2           13| CONTROLA */ A.CTL,
		B.R.1, /*   OUTINB |3           12| CONTROLD */ D.CTL,
		B.R.2, /*   INOUTB |4    4066   11| INOUTD   */ D.R.1,
		B.CTL, /* CONTROLB |5           10| OUTIND   */ D.R.2,
		C.CTL, /* CONTROLC |6            9| OUTINC   */ C.R.1,
		A.VSS, /*      VSS |7            8| INOUTC   */ C.R.2
			   /*          +--------------+          */
	)
NETLIST_END()

//- Identifier: CD4069_DIP
//- Title: CD4069UBM/CD4069UBC Inverter Circuits
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,VSS,Y4,A4,Y5,A5,Y6,A6,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/150/206783_DS.pdf
//-
static NETLIST_START(CD4069_DIP)
	CD4069_GATE(A)
	CD4069_GATE(B)
	CD4069_GATE(C)
	CD4069_GATE(D)
	CD4069_GATE(E)
	CD4069_GATE(F)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD, E.VDD, F.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS, E.VSS, F.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*  A1 |1     ++    14| VDD */ A.VDD,
		  A.Q, /*  Y1 |2           13| A6  */ F.A,
		  B.A, /*  A2 |3           12| Y6  */ F.Q,
		  B.Q, /*  Y2 |4    4069   11| A5  */ E.A,
		  C.A, /*  A3 |5           10| Y5  */ E.Q,
		  C.Q, /*  Y3 |6            9| A4  */ D.A,
		A.VSS, /* VSS |7            8| Y4  */ D.Q
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4070_DIP
//- Title: CD4070BM/CD4070BC Quad 2-Input EXCLUSIVE-OR Gate
//- Pinalias: A,B,J,K,C,D,VSS,E,F,L,M,G,H,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/150/206783_DS.pdf
//-
static NETLIST_START(CD4070_DIP)
	CD4070_GATE(A)
	CD4070_GATE(B)
	CD4070_GATE(C)
	CD4070_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*   A |1     ++    14| VDD */ A.VDD,
		  A.B, /*   B |2           13| H   */ D.B,
		  A.Q, /*   J |3           12| G   */ D.A,
		  B.Q, /*   K |4    4070   11| M   */ D.Q,
		  B.A, /*   C |5           10| L   */ C.Q,
		  B.B, /*   D |6            9| F   */ C.B,
		A.VSS, /* VSS |7            8| E   */ C.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4071_DIP
//- Title: CD4071BC Quad 2-Input OR Buffered B Series Gate
//- Pinalias: A,B,J,K,C,D,VSS,E,F,L,M,G,H,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/185/109289_DS.pdf
//-
static NETLIST_START(CD4071_DIP)
	CD4071_GATE(A)
	CD4071_GATE(B)
	CD4071_GATE(C)
	CD4071_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*   A |1     ++    14| VDD */ A.VDD,
		  A.B, /*   B |2           13| H   */ D.B,
		  A.Q, /*   J |3           12| G   */ D.A,
		  B.Q, /*   K |4    4071   11| M   */ D.Q,
		  B.A, /*   C |5           10| L   */ C.Q,
		  B.B, /*   D |6            9| F   */ C.B,
		A.VSS, /* VSS |7            8| E   */ C.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4076_DIP
//- Title: CD4076BM/CD4076BC TRI-STATE(R) Quad D Flip-Flop
//- Pinalias: OD1,OD2,OA,OB,OC,OD,CLK,VSS,ID1,ID2,ID,IC,IB,IA,CLR,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- Limitations:
//-    Voltage-dependent timing is not implemented.
//- FunctionTable:
//-    http://www.bitsavers.org/components/national/_dataBooks/1981_Natonal_CMOS_Databook.pdf 5-186 (pdf page 567)
//-
//-         Function table for ID1/2 pins
//-         +-----+-----+-----+----+-----++-----+
//-         | ID1 | ID2 | CLR | Ix | CLK || iOx |
//-         +=====+=====+=====+====+=====++=====+
//-         |  X  |  X  |  0  |  X |  0  || iOx |
//-         |  X  |  X  |  0  |  X |  1  || iOx |
//-         |  1  |  X  |  0  |  X | 0>1 || iOx |
//-         |  X  |  1  |  0  |  X | 0>1 || iOx |
//-         |  0  |  0  |  0  |  0 | 0>1 ||  0  |
//-         |  0  |  0  |  0  |  1 | 0>1 ||  1  |
//-         |  X  |  X  |  1  |  X |  X  ||  0  |
//-         +-----+-----+-----+----+-----++-----+
//-         Note: iOX is an internal signal, the output of each D-latch
//-
//-         Function table for OD1/2 pins vs output of the internal D-latches
//-         +-----+-----+-----++-----+
//-         | OD1 | OD2 | iOx ||  Ox |
//-         +=====+=====+=====++=====+
//-         |  1  |  X  |  X  ||  Z  |
//-         |  X  |  1  |  X  ||  Z  |
//-         |  0  |  0  |  0  ||  0  |
//-         |  0  |  0  |  1  ||  1  |
//-         +-----+-----+-----++-----+
//-
static NETLIST_START(CD4076_DIP)
	CD4076(A)

	DIPPINS(     /*     +--------------+     */
		  A.OD1, /* OD1 |1     ++    16| VDD */ A.VDD,
		  A.OD2, /* OD2 |2           15| CLR */ A.CLR,
		   A.OA, /*  OA |3           14| IA  */ A.IA,
		   A.OB, /*  OB |4    4076   13| IB  */ A.IB,
		   A.OC, /*  OC |5           12| IC  */ A.IC,
		   A.OD, /*  OD |6           11| ID  */ A.ID,
		  A.CLK, /* CLK |7           10| ID2 */ A.ID2,
		  A.VSS, /* VSS |8            9| ID1 */ A.ID1
				 /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4081_DIP
//- Title: CD4081BC Quad 2-Input AND Buffered B Series Gate
//- Pinalias: A,B,J,K,C,D,VSS,E,F,L,M,G,H,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/185/109289_DS.pdf
//-
static NETLIST_START(CD4081_DIP)
	CD4081_GATE(A)
	CD4081_GATE(B)
	CD4081_GATE(C)
	CD4081_GATE(D)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*   A |1     ++    14| VDD */ A.VDD,
		  A.B, /*   B |2           13| H   */ D.B,
		  A.Q, /*   J |3           12| G   */ D.A,
		  B.Q, /*   K |4    4081   11| M   */ D.Q,
		  B.A, /*   C |5           10| L   */ C.Q,
		  B.B, /*   D |6            9| F   */ C.B,
		A.VSS, /* VSS |7            8| E   */ C.A
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4316_DIP
//- Title: 74HC/HCT4316 Quad bilateral switches
//- Pinalias: 1Z,1Y,2Y,2Z,2S,3S,EQ,GND,VEE,3Z,3Y,4Y,4Z,4S,1S,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Philips datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheet/philips/74HCT4316.pdf
//-
static NETLIST_START(CD4316_DIP)
	CD4316_GATE(A)
	CD4316_GATE(B)
	CD4316_GATE(C)
	CD4316_GATE(D)

	NET_C(A.E, B.E, C.E, D.E)
	NET_C(A.VDD, B.VDD, C.VDD, D.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS)

	PARAM(A.BASER, 45.0)
	PARAM(B.BASER, 45.0)
	PARAM(C.BASER, 45.0)
	PARAM(D.BASER, 45.0)

	DIPPINS(   /*     +--------------+     */
		A.R.2, /*  1Z |1     ++    16| VCC */ A.VDD,
		A.R.1, /*  1Y |2           15| 1S  */ A.S,
		B.R.1, /*  2Y |3           14| 4S  */ D.S,
		B.R.2, /*  2Z |4    4316   13| 4Z  */ D.R.2,
		  B.S, /*  2S |5           12| 4Y  */ D.R.1,
		  C.S, /*  3S |6           11| 3Y  */ C.R.1,
		  A.E, /*  EQ |7           10| 3Z  */ C.R.2,
		A.VSS, /* GND |8            9| VEE */ VEE
			   /*     +--------------+     */
	)
NETLIST_END()

//- Identifier:  CD4538_DIP
//- Title: CD4538BC Dual Precision Monostable
//- Pinalias: C1,RC1,CLRQ1,B1,A1,Q1,QQ1,GND,QQ2,Q2,A2,B2,CLRQ2,RC2,C2,VCC
//- Package: DIP
//- NamingConvention: Naming conventions follow Fairchild Semiconductor datasheet
//- Limitations:
//-    Timing inaccuracies may occur for capacitances < 1nF. Please consult datasheet
//-
//- Example: 74123.cpp,74123_example
//-
//- FunctionTable:
//-    https://pdf1.alldatasheet.com/datasheet-pdf/view/50871/FAIRCHILD/CD4538.html
//-
static NETLIST_START(CD4538_DIP)
	CD4538(A)
	CD4538(B)

	NET_C(A.VDD, B.VDD)
	NET_C(A.VSS, B.VSS)

	DIPPINS(    /*     +--------------+     */
		   A.C, /*  1Z |1     ++    16| VCC */ A.VDD,
		  A.RC, /*  1Y |2           15| 1S  */ B.C,
		A.CLRQ, /*  2Y |3           14| 4S  */ B.RC,
		   A.A, /*  2Z |4    4316   13| 4Z  */ B.CLRQ,
		   A.B, /*  2S |5           12| 4Y  */ B.A,
		   A.Q, /*  3S |6           11| 3Y  */ B.B,
		  A.QQ, /*  EQ |7           10| 3Z  */ B.Q,
		 A.VSS, /* GND |8            9| VEE */ B.QQ
				/*     +--------------+     */
	)
NETLIST_END()

//FIXME: Documentation
static NETLIST_START(MM5837_DIP)
	MM5837(A)
	NC_PIN(NC)

	// Create a parameter freq for the dip model
	// The default will be A's FREQ parameter.
	DEFPARAM(FREQ, "$(@.A.FREQ")
	PARAM(A.FREQ, "$(@.FREQ)")

	DIPPINS(    /*       +--------+    */
		A.VDD,  /*   VDD |1  ++  8| NC */ NC.I,
		A.VGG,  /*   VGG |2      7| NC */ NC.I,
		A.OUT,  /*   OUT |3      6| NC */ NC.I,
		A.VSS,  /*   VSS |4      5| NC */ NC.I
				/*       +--------+    */
	)
NETLIST_END()

static TRUTHTABLE_START(CD4001_GATE, 2, 1, "")
	TT_HEAD("A , B | Q ")
	TT_LINE("0,0|1|110")
	TT_LINE("X,1|0|120")
	TT_LINE("1,X|0|120")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4011_GATE, 2, 1, "")
	TT_HEAD("A,B|Q ")
	TT_LINE("0,X|1|100")
	TT_LINE("X,0|1|100")
	TT_LINE("1,1|0|100")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4030_GATE, 2, 1, "")
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|0|100")
	TT_LINE("0,1|1|100")
	TT_LINE("1,0|1|100")
	TT_LINE("1,1|0|100")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4049_GATE, 1, 1, "")
	TT_HEAD("A|Q ")
	TT_LINE("0|1|45")
	TT_LINE("1|0|45")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4069_GATE, 1, 1, "")
	TT_HEAD("A|Q ")
	TT_LINE("0|1|55")
	TT_LINE("1|0|55")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4070_GATE, 2, 1, "")
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|0|15")
	TT_LINE("0,1|1|22")
	TT_LINE("1,0|1|22")
	TT_LINE("1,1|0|15")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4071_GATE, 2, 1, "")
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|0|200")
	TT_LINE("0,1|1|200")
	TT_LINE("1,0|1|200")
	TT_LINE("1,1|1|200")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

static TRUTHTABLE_START(CD4081_GATE, 2, 1, "")
	TT_HEAD("A,B|Q ")
	TT_LINE("0,0|0|200")
	TT_LINE("0,1|0|200")
	TT_LINE("1,0|0|200")
	TT_LINE("1,1|1|200")
	TT_FAMILY("CD4XXX")
TRUTHTABLE_END()

NETLIST_START(cd4xxx_lib)

	TRUTHTABLE_ENTRY(CD4001_GATE)
	TRUTHTABLE_ENTRY(CD4011_GATE)
	TRUTHTABLE_ENTRY(CD4030_GATE)
	TRUTHTABLE_ENTRY(CD4049_GATE)
	TRUTHTABLE_ENTRY(CD4069_GATE)
	TRUTHTABLE_ENTRY(CD4070_GATE)
	TRUTHTABLE_ENTRY(CD4071_GATE)
	TRUTHTABLE_ENTRY(CD4081_GATE)

	LOCAL_LIB_ENTRY(CD4001_DIP)
	LOCAL_LIB_ENTRY(CD4011_DIP)
	LOCAL_LIB_ENTRY(CD4030_DIP)
	LOCAL_LIB_ENTRY(CD4049_DIP)
	LOCAL_LIB_ENTRY(CD4069_DIP)
	LOCAL_LIB_ENTRY(CD4070_DIP)
	LOCAL_LIB_ENTRY(CD4071_DIP)
	LOCAL_LIB_ENTRY(CD4081_DIP)

	/* DIP ONLY */
	LOCAL_LIB_ENTRY(CD4006_DIP)
	LOCAL_LIB_ENTRY(CD4013_DIP)
	LOCAL_LIB_ENTRY(CD4017_DIP)
	LOCAL_LIB_ENTRY(CD4022_DIP)
	LOCAL_LIB_ENTRY(CD4020_DIP)
	LOCAL_LIB_ENTRY(CD4024_DIP)
	LOCAL_LIB_ENTRY(CD4029_DIP)
	LOCAL_LIB_ENTRY(CD4042_DIP)
	LOCAL_LIB_ENTRY(CD4053_DIP)
	LOCAL_LIB_ENTRY(CD4066_DIP)
	LOCAL_LIB_ENTRY(CD4016_DIP)
	LOCAL_LIB_ENTRY(CD4076_DIP)
	LOCAL_LIB_ENTRY(CD4316_DIP)
	LOCAL_LIB_ENTRY(CD4538_DIP)

	LOCAL_LIB_ENTRY(MM5837_DIP)

NETLIST_END()
