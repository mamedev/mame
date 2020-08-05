// license:GPL-2.0+
// copyright-holders:Couriersud

#include "netlist/devices/net_lib.h"

//- Identifier: CD4001BM/CD4001BC
//- Title: Quad 2-Input NOR Buffered B Series Gate
//- Description: These quad gates are monolithic complementary MOS (CMOS) integrated circuits constructed with N- and P-channel enhancement mode transistors.
//-   They have equal source and sink current capabilities and conform to standard B series output drive.
//-   The devices also have buffered outputs which improve transfer characteristics by providing very high gain.
//-   All inputs are protected against static discharge with diodes to VDD and VSS.
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

//- Identifier: CD4006BM/CD4006BC
//- Title: 18-Stage Static Shift Register
//- Description: The CD4006BM/CD4006BC 18-stage static shift register is comprised of four separate shift register sections, two sections of four stages and two sections of five stages.
//-   Each section has an independent data input.
//-   Outputs are available at the fourth stage and the fifth stage of each section.
//-   A common clock signal is used for all stages.
//-   Data is shifted to the next stage on the negative-going transition of the clock.
//-   Through appropriate connections of inputs and outputs, multiple register sections of 4, 5, 8, and 9 stages, or single register sections of 10, 12, 13, 14, 16, 17, and 18 stages can be implemented using one package.
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

//- Identifier: CD4011BM/CD4011BC
//- Title: Quad 2-Input NAND Buffered B Series Gate
//- Description: These quad gates are monolithic complementary MOS (CMOS) integrated circuits constructed with N- and P-channel enhancement mode transistors.
//-   They have equal source and sink current capabilities and conform to standard B series output drive.
//-   The devices also have buffered outputs which improve transfer characteristics by providing very high gain.
//-   All inputs are protected against static discharge with diodes to VDD and VSS.
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

//- Identifier: CD4013BM/CD4013BC
//- Title: Dual D Flip-Flop
//- Description: The CD4013B dual D flip-flop is a monolithic complementary MOS (CMOS) integrated circuit constructed with N- and P-channel enhancement mode transistors.
//-   Each flip-flop has independent data, set, reset, and clock inputs and Q and QQ outputs.
//-   These devices can be used for shift register applications, and by connecting Q output to the data input, for counter and toggle applications.
//-   The logic levelpresent at the D input is transferred to the Q output during the positive-going transition of the clock pulse.
//-   Setting or resetting is independent of the clock and is accomplished by a high level on the set or reset line respectively.
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

//- Identifier: CD4016BM/CD4016BC
//- Title: Quad Bilateral Switch
//- Description: The CD4016BM/CD4016BC is a quad bilateral switch intended for the transmission or multiplexing of analog or digital signals.
//-   It is pin-for-pin compatible with CD4066BM/CD4066BC.
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

//- Identifier: CD4017BM/CD4017BC
//- Title: Decade Counter/Divider with 10 Decoded Outputs
//- Description: The CD4017BM/CD4017BC is a 5-stage divide-by-10 Johnson counter with 10 decoded outputs and a carry out bit.
//-   These counters are cleared to their zero count by a logical 1 on their reset line.
//-   These counters are advanced on the positive edge of the clock signal when the clock enable signal is in the logical 0 state.
//-   The configuration of the CD4017BM/CD4017BC permits medium speed operation and assures a hazard free counting sequence.
//-   The 10 decoded outputs are normally in the logical 0 state and go to the logical 1 state only at their respective time slot.
//-   Each decoded output remains high for 1 full clock cycle.
//-   The carry-out signal completes a full cycle for every 10 clock input cycles and is used as a ripple carry signal to any succeeding stages.
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

//- Identifier: CD4020BC
//- Title: 14-Stage Ripple Carry Binary Counters
//- Description: The CD4020BC is a 14-stage ripple carry binary counter.
//-   The counters are advanced one count on the negative transition of each clock pulse.
//-   The counters are reset to the zero state by a logical 1 at the reset input independent of clock.
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
			   /*     +--------------+       */
	)
NETLIST_END()

//- Identifier: CD4022BM/CD4022BC
//- Title: Divide-by-8 Counter/Divider with 8 Decoded Outputs
//- Description: The CD4022BM/CD4022BC is a 4-stage divide-by-8 Johnson counter with 8 decoded outputs and a carry-out bit.
//-   These counters are cleared to their zero count by a logical 1 on their reset line.
//-   These counters are advanced on the positive edge of the clock signal when the clock enable signal is in the logical 0 state.
//-   The configuration of the CD4022BM/CD4022BC permits medium speed operation and assures a hazard free counting sequence.
//-   The 8 decoded outputs are normally in the logical 0 state and go to the logical 1 state only at their respective time slot.
//-   Each decoded output remains high for 1 full clock cycle.
//-   The carry-out signal completes a full cycle for every 8 clock input cycles and is used as a ripple carry signal to any succeeding stages
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

//- Identifier: CD4024BM/CD4024BC
//- Title: 7-Stage Ripple Carry Binary Counter
//- Description: The CD4024BM/CD4024BC is a 7-stage ripple-carry binary counter.
//-   Buffered outputs are externally available from stages 1 through 7.
//-   The counter is reset to its logical 0 stage by a logical 1 on the reset input.
//-   The counter is advanced one count on the negative transition of each clock pulse.
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
				 /*       +--------------+     */
	)
NETLIST_END()

//- Identifier: CD4053BM/CD4053BC
//- Title: Triple 2-Channel AnalogMultiplexer/Demultiplexer
//- Description: These analog multiplexers/demultiplexers are digitally controlled analog switches having low ON impedance andvery low OFF leakage currents.
//-   Control of analog signalsup to 15V(p-p) can be achieved by digital signal amplitudes of 3–15V.
//-   For example, if VDD=5V, VSS=0V and VEE=-5V, analog signals from -5V to +5V can be controlled by digital inputs of 0–5V.
//-   The multiplexer circuits dissipate extremely low quiescent power over the full VDD-VSS and VDD-VEE supply voltage ranges, independent of the logic state of the control signals.
//-   When a logical 1 is present atthe inhibit input terminal all channels are OFF.
//-   CD4053BM/CD4053BC is a triple 2-channel multiplexer having three separate digital control inputs, A, B, and C, and an inhibit input.
//-   Each control input selects one of a pair of channels which are connected in a single-pole double-throw configuration.
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

//- Identifier: CD4066BM/CD4066BC
//- Title: Quad Bilateral Switch
//- Description: The CD4066BM/CD4066BC is a quad bilateral switch intended for the transmission or multiplexing of analog or digital signals.
//-   It is pin-for-pin compatible with CD4016BM/CD4016BC, but has a much lower ON resistance, and ON resistance is relatively constant over the input-signal range.
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

//- Identifier: CD4069UBM/CD4069UBC
//- Title: Inverter Circuits
//- Description: The CD4069UB consists of six inverter circuits and is manufactured using complementary MOS (CMOS) to achieve wide power supply operating range, low power consumption, high noise immunity, and symmetric controlled rise and fall times.
//-   This device is intended for all general purpose inverter applications where the special characteristics of the MM74C901, MM74C903, MM74C907, and CD4049A Hex Inverter/Buffers are not required.
//-   In those applications requiring larger noise immunity the MM74C14 or MM74C914 Hex Schmitt Trigger is suggested.
//-   All inputs are protected from damage due to static discharge by diode clamps to VDD and VSS.
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

//- Identifier: CD4070BM/CD4070BC
//- Title: Quad 2-Input EXCLUSIVE-OR Gate
//- Description: Employing complementary MOS (CMOS) transistors to achieve wide power supply operating range, low power consumption, and high noise margin, the CD4070BM/BC provides basic functions used in the implementation of digital integrated circuit systems.
//-   The N- and P-channel enhancement mode transistors provide a symmetrical circuit with output swing essentially equal to the supply voltage.
//-   No DC power other than that caused by leakage current is consumed during static condition.
//-   All inputs are protected from damage due to static discharge by diode clamps to VDD and VSS.
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

//- Identifier: 74HC/HCT4316
//- Title: Quad bilateral switches
//- Description: The 74HC/HCT4316 are high-speed Si-gate CMOS devices.
//-   They are specified in compliance with JEDEC standard no. 7A.
//-   The 74HC/HCT4316 have four independent analog switches.
//-   Each switch has two input/output terminals (nY, nZ) and an active HIGH select input (nS).
//-   When the enable input (E) is HIGH, all four analog switches are turned off.
//-   Current through a switch will not cause additional VCC current provided the voltage at the terminals of the switch is maintained within the supply voltage range; VCC >> (VY, VZ) >> VEE.
//-   Inputs nY and nZ are electrically equivalent terminals.
//-   VCC and GND are the supply voltage pins for the digital control inputs (EQ and nS).
//-   The VCC to GND ranges are 2.0 to 10.0V for HC and 4.5 to 5.5V for HCT.
//-   The analog inputs/outputs (nY and nZ) can swing between VCC as a positive limit and VEE as a negative limit.
//-   VCC−VEE may not exceed 10.0 V
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
//- Description: The CD4538BC is a dual, precision monostable multivibrator with
//-   independent trigger and reset controls. The device
//-   is retriggerable and resettable, and the control inputs are
//-   internally latched. Two trigger inputs are provided to allow
//-   either rising or falling edge triggering. The reset inputs are
//-   active LOW and prevent triggering while active. Precise
//-   control of output pulse-width has been achieved using linear
//-   CMOS techniques. The pulse duration and accuracy
//-   are determined by external components RX and CX. The
//-   device does not allow the timing capacitor to discharge
//-   through the timing pin on power-down condition. For this
//-   reason, no external protection resistor is required in series
//-   with the timing pin. Input protection from static discharge is
//-   provided on all pins.
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


NETLIST_START(CD4XXX_lib)

	TRUTHTABLE_START(CD4001_GATE, 2, 1, "")
		TT_HEAD("A , B | Q ")
		TT_LINE("0,0|1|110")
		TT_LINE("X,1|0|120")
		TT_LINE("1,X|0|120")
		TT_FAMILY("CD4XXX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(CD4011_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,X|1|100")
		TT_LINE("X,0|1|100")
		TT_LINE("1,1|0|100")
		TT_FAMILY("CD4XXX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(CD4069_GATE, 1, 1, "")
		TT_HEAD("A|Q ")
		TT_LINE("0|1|55")
		TT_LINE("1|0|55")
		TT_FAMILY("CD4XXX")
	TRUTHTABLE_END()

	TRUTHTABLE_START(CD4070_GATE, 2, 1, "")
		TT_HEAD("A,B|Q ")
		TT_LINE("0,0|0|15")
		TT_LINE("0,1|1|22")
		TT_LINE("1,0|1|22")
		TT_LINE("1,1|0|15")
		TT_FAMILY("CD4XXX")
	TRUTHTABLE_END()

	LOCAL_LIB_ENTRY(CD4001_DIP)
	LOCAL_LIB_ENTRY(CD4011_DIP)
	LOCAL_LIB_ENTRY(CD4069_DIP)
	LOCAL_LIB_ENTRY(CD4070_DIP)

	/* DIP ONLY */
	LOCAL_LIB_ENTRY(CD4006_DIP)
	LOCAL_LIB_ENTRY(CD4013_DIP)
	LOCAL_LIB_ENTRY(CD4017_DIP)
	LOCAL_LIB_ENTRY(CD4022_DIP)
	LOCAL_LIB_ENTRY(CD4020_DIP)
	LOCAL_LIB_ENTRY(CD4024_DIP)
	LOCAL_LIB_ENTRY(CD4053_DIP)
	LOCAL_LIB_ENTRY(CD4066_DIP)
	LOCAL_LIB_ENTRY(CD4016_DIP)
	LOCAL_LIB_ENTRY(CD4316_DIP)
	LOCAL_LIB_ENTRY(CD4538_DIP)

NETLIST_END()
