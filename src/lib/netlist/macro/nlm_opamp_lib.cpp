// license:CC0
// copyright-holders:Couriersud

#include "devices/net_lib.h"

/*
 * 0 = Basic hack (Norton with just amplification, no voltage cutting)
 * 1 = Model from LTSPICE mailing list - slow!
 * 2 = Simplified model using diode inputs and netlist
 * 3 = Model according to datasheet
 * 4 = Faster model by Colin Howell
 *
 * For Money Money 1 and 3 delivery comparable results.
 * 3 is simpler (less BJTs) and converges a lot faster.
 *
 * Model 4 uses a lot less resources and pn-junctions. The preferred new normal.
 */
#define USE_LM3900_MODEL (4)

/*
 *   Generic layout with 4 opamps, VCC on pin 4 and GND on pin 11
 */

static NETLIST_START(opamp_layout_4_4_11)
{
	DIPPINS(        /*   +--------------+   */
		A.OUT,      /*   |1     ++    14|   */ D.OUT,
		A.MINUS,    /*   |2           13|   */ D.MINUS,
		A.PLUS,     /*   |3           12|   */ D.PLUS,
		A.VCC,      /*   |4           11|   */ A.GND,
		B.PLUS,     /*   |5           10|   */ C.PLUS,
		B.MINUS,    /*   |6            9|   */ C.MINUS,
		B.OUT,      /*   |7            8|   */ C.OUT
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND, C.GND, D.GND)
	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
}

/*
 *   Generic layout with 2 opamps, VCC on pin 8 and GND on pin 4
 */

static NETLIST_START(opamp_layout_2_8_4)
{
	DIPPINS(        /*   +--------------+   */
		A.OUT,      /*   |1     ++     8|   */ A.VCC,
		A.MINUS,    /*   |2            7|   */ B.OUT,
		A.PLUS,     /*   |3            6|   */ B.MINUS,
		A.GND,      /*   |4            5|   */ B.PLUS
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
}

/*
 *   Generic layout with 2 opamps, VCC+ on pins 9/13,  VCC- on pin 4 and compensation
 */

static NETLIST_START(opamp_layout_2_13_9_4)
{
	DIPPINS(        /*   +--------------+   */
		A.MINUS,    /*   |1     ++    14|   */ A.N2,
		A.PLUS,     /*   |2           13|   */ A.VCC,
		A.N1,       /*   |3           12|   */ A.OUT,
		A.GND,      /*   |4           11|   */ NC,
		B.N1,       /*   |5           10|   */ B.OUT,
		B.PLUS,     /*   |6            9|   */ B.VCC,
		B.MINUS,    /*   |7            8|   */ B.N2
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND)
}

/*
 *   Generic layout with 1 opamp, VCC+ on pin 7, VCC- on pin 4 and compensation
 *   // FIXME: Offset inputs are not supported!
 */

static NETLIST_START(opamp_layout_1_7_4)
{
	DIPPINS(             /*   +--------------+   */
		NC /* OFFSET */, /*   |1     ++     8|   */ NC,
		A.MINUS,         /*   |2            7|   */ A.VCC,
		A.PLUS,          /*   |3            6|   */ A.OUT,
		A.GND,           /*   |4            5|   */ NC /* OFFSET */
						 /*   +--------------+   */
	)
}

/*
 *   Generic layout with 1 opamp, VCC+ on pin 8, VCC- on pin 5 and compensation
 */

static NETLIST_START(opamp_layout_1_8_5)
{
	DIPPINS(        /*   +--------------+   */
		NC.1,       /*   |1           10|   */ NC.3,
		OFFSET.N1,  /*   |2            9|   */ NC.2,
		MINUS,      /*   |3            8|   */ VCC.PLUS,
		PLUS,       /*   |4            7|   */ OUT,
		VCC.MINUS,  /*   |5            6|   */ OFFSET.N2
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
}

/*
 *   Generic layout with 1 opamp, VCC+ on pin 11, VCC- on pin 6 and compensation
 */

static NETLIST_START(opamp_layout_1_11_6)
{
	DIPPINS(        /*   +--------------+   */
		NC.1,       /*   |1     ++    14|   */ NC.7,
		NC.2,       /*   |2           13|   */ NC.6,
		OFFSET.N1,  /*   |3           12|   */ NC.5,
		MINUS,      /*   |4           11|   */ VCC.PLUS,
		PLUS,       /*   |5           10|   */ OUT,
		VCC.MINUS,  /*   |6            9|   */ OFFSET.N2,
		NC.3,       /*   |7            8|   */ NC.4
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
}

static NETLIST_START(MB3614_DIP)
{
	OPAMP(A, "MB3614")
	OPAMP(B, "MB3614")
	OPAMP(C, "MB3614")
	OPAMP(D, "MB3614")

	INCLUDE(opamp_layout_4_4_11)

}

static NETLIST_START(MC3340_DIP)
{
	// A netlist description of the Motorola MC3340 Electronic Attenuator
	// IC, a voltage-controlled amplifier/attenuator. It amplifies or
	// attenuates an input signal according to the voltage of a second,
	// control signal, with a maximum gain of about 12-13 dB (about a
	// factor of 4 in voltage), and higher control voltages giving greater
	// attenuation, which scales logarithmically.

	// The netlist here is based on the circuit schematic given in
	// Motorola's own data books, especially the most recent ones
	// published in the 1990s (e.g. _Motorola Analog/Interface ICs Device
	// Data, Vol. II_ (1996), p. 9-67), which are the only schematics that
	// include resistor values. However, the 1990s schematics are missing
	// one crossover connection which is present in older schematics
	// published in the 1970s (e.g. _Motorola Linear Integrated Circuits_
	// (1979), p. 5-130). This missing connection is clearly an error
	// which has been fixed in this netlist; without it, the circuit won't
	// amplify properly, generating only a very weak output signal.

	// The 1990s schematics also omit a couple of diodes which are present
	// in the 1970s schematics. Both of these diodes have been included
	// here. One raises the minimum control voltage at which signal
	// attenuation starts, so it makes the netlist's profile of
	// attenuation vs. control voltage better match Motorola's charts for
	// the device. The other affects the level of the input "midpoint",
	// and including it makes the engine sound closer to that on real
	// 280-ZZZAP machines.

	// The Motorola schematics do not label components, so I've created my
	// own labeling scheme based on numbering components on the schematics
	// from top to bottom, left to right, with resistors also getting
	// their value (expressed European-style to avoid decimal points) as
	// part of the name. The netlist is also listed following the
	// schematics in roughly top-to-bottom, left-to-right order.

	// A very simple model is used for the transistors here, based on the
	// generic NPN default but with a larger scale current. Again, this
	// was chosen to better match the netlist's attenuation vs. control
	// voltage profile to that given in Motorola's charts for the device.

	// The MC3340 has the same circuit internally as an older Motorola
	// device, the MFC6040, which was replaced by the MC3340 in the
	// mid-1970s. The two chips differ only in packaging. Older arcade
	// games which use the MFC6040 may also benefit from this netlist
	// implementation.

	RES(R1_5K1, RES_K(5.1))

	DIODE(D1, "D(IS=1e-15 N=1)")

	RES(R2_4K7, RES_K(4.7))

	QBJT_EB(Q1, "NPN(IS=1E-13 BF=100)")

	RES(R3_750, RES_R(750))
	RES(R4_10K, RES_K(10))

	QBJT_EB(Q2, "NPN(IS=1E-13 BF=100)")

	RES(R5_750, RES_R(750))
	RES(R6_3K9, RES_K(3.9))

	RES(R7_5K1, RES_K(5.1))
	RES(R8_20K, RES_K(20))

	DIODE(D2, "D(IS=1e-15 N=1)")

	RES(R9_510, RES_R(510))

	QBJT_EB(Q3, "NPN(IS=1E-13 BF=100)")

	QBJT_EB(Q4, "NPN(IS=1E-13 BF=100)")

	QBJT_EB(Q5, "NPN(IS=1E-13 BF=100)")

	RES(R10_1K3, RES_K(1.3))

	QBJT_EB(Q6, "NPN(IS=1E-13 BF=100)")

	RES(R11_5K1, RES_K(5.1))

	QBJT_EB(Q7, "NPN(IS=1E-13 BF=100)")

	QBJT_EB(Q8, "NPN(IS=1E-13 BF=100)")

	RES(R12_1K5, RES_K(1.5))

	RES(R13_6K2, RES_K(6.2))

	QBJT_EB(Q9, "NPN(IS=1E-13 BF=100)")

	RES(R14_5K1, RES_K(5.1))

	QBJT_EB(Q10, "NPN(IS=1E-13 BF=100)")

	RES(R15_5K1, RES_K(5.1))

	RES(R16_200, RES_R(200))

	RES(R17_5K1, RES_K(5.1))

	DIODE(D3, "D(IS=1e-15 N=1)")

	RES(R18_510, RES_R(510))

	ALIAS(VCC, R1_5K1.1)
	NET_C(R1_5K1.1, Q1.C, Q2.C, R7_5K1.1, Q3.C, Q4.C, Q7.C,
		R13_6K2.1, Q10.C, R17_5K1.1)
	// Location of first diode present on 1970s schematics but omitted on
	// 1990s ones. Including it raises the control voltage threshold for
	// attenuation significantly.
	NET_C(R1_5K1.2, D1.A, Q1.B)
	NET_C(D1.K, R2_4K7.1)
	NET_C(R2_4K7.2, GND)

	NET_C(Q1.E, R3_750.1, R5_750.1)
	NET_C(R3_750.2, R4_10K.1, Q2.B)
	NET_C(R4_10K.2, GND)

	NET_C(R5_750.2, R6_3K9.1, Q3.B)
	ALIAS(CONTROL, R6_3K9.2)

	ALIAS(INPUT, Q5.B)

	NET_C(INPUT, R8_20K.1)
	// Location of second diode present on 1970s schematics but omitted on
	// 1990s ones. Including it is critical to making the tone of the
	// output engine sound match that of real 280-ZZZAP machines.
	NET_C(R7_5K1.2, R8_20K.2, D2.A)
	NET_C(D2.K, R9_510.1)
	NET_C(R9_510.2, GND)

	NET_C(Q4.E, Q6.E, Q5.C)
	NET_C(Q5.E, R10_1K3.1)
	NET_C(R10_1K3.2, GND)

	NET_C(Q6.B, Q7.B, Q2.E, R11_5K1.1)
	NET_C(R11_5K1.2, GND)

	NET_C(Q7.E, Q9.E, Q8.C)
	NET_C(Q8.E, R12_1K5.1)
	NET_C(R12_1K5.2, GND)

	NET_C(Q4.B, Q9.B, Q3.E, R14_5K1.1)
	NET_C(R14_5K1.2, GND)

	// This is where the cross-connection is erroneously omitted from
	// 1990s schematics.
	NET_C(Q6.C, R13_6K2.2, Q9.C, Q10.B)

	// Connection for external frequency compensation capacitor; unused
	// here.
	ALIAS(ROLLOFF, Q10.B)

	NET_C(Q10.E, R16_200.1, R15_5K1.1)
	NET_C(R15_5K1.2, GND)
	ALIAS(OUTPUT, R16_200.2)

	NET_C(R17_5K1.2, D3.A, Q8.B)
	NET_C(D3.K, R18_510.1)
	ALIAS(GND, R18_510.2)

	ALIAS(1, INPUT)
	ALIAS(2, CONTROL)
	ALIAS(3, GND)
	ALIAS(6, ROLLOFF)
	ALIAS(7, OUTPUT)
	ALIAS(8, VCC)
}

static NETLIST_START(TL081_DIP)
{
	OPAMP(A, "TL084")

	INCLUDE(opamp_layout_1_7_4)

}

static NETLIST_START(TL082_DIP)
{
	OPAMP(A, "TL084")
	OPAMP(B, "TL084")

	INCLUDE(opamp_layout_2_8_4)

}

static NETLIST_START(TL084_DIP)
{
	OPAMP(A, "TL084")
	OPAMP(B, "TL084")
	OPAMP(C, "TL084")
	OPAMP(D, "TL084")

	INCLUDE(opamp_layout_4_4_11)

}

static NETLIST_START(LM324_DIP)
{
	OPAMP(A, "LM324")
	OPAMP(B, "LM324")
	OPAMP(C, "LM324")
	OPAMP(D, "LM324")

	INCLUDE(opamp_layout_4_4_11)

}

static NETLIST_START(LM2902_DIP)
{
	// Same datasheet and mostly same characteristics as LM324
	OPAMP(A, "LM324")
	OPAMP(B, "LM324")
	OPAMP(C, "LM324")
	OPAMP(D, "LM324")

	INCLUDE(opamp_layout_4_4_11)

}

static NETLIST_START(LM348_DIP)
{
	OPAMP(A, "UA741")
	OPAMP(B, "UA741")
	OPAMP(C, "UA741")
	OPAMP(D, "UA741")

	INCLUDE(opamp_layout_4_4_11)

}

static NETLIST_START(LM358_DIP)
{
	OPAMP(A, "LM358")
	OPAMP(B, "LM358")

	INCLUDE(opamp_layout_2_8_4)

}

static NETLIST_START(UA741_DIP8)
{
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_7_4)

}

static NETLIST_START(UA741_DIP10)
{
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_8_5)

}

static NETLIST_START(UA741_DIP14)
{
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_11_6)

}

static NETLIST_START(MC1558_DIP)
{
	OPAMP(A, "UA741")
	OPAMP(B, "UA741")

	INCLUDE(opamp_layout_2_8_4)

}

static NETLIST_START(LM747_DIP)
{
	OPAMP(A, "LM747")
	OPAMP(B, "LM747")

	INCLUDE(opamp_layout_2_13_9_4)
	NET_C(A.VCC, B.VCC)

}

static NETLIST_START(LM747A_DIP)
{
	OPAMP(A, "LM747A")
	OPAMP(B, "LM747A")

	INCLUDE(opamp_layout_2_13_9_4)
	NET_C(A.VCC, B.VCC)

}

//- Identifier: AN6551_SIL
//- Title: AN6551 Dual Operational Amplifier
//- Pinalias: VCC,A.OUT,A-,A+,GND,B+,B-,B.OUT,VCC
//- Package: SIL
//- NamingConvention: Naming conventions follow Panasonic datasheet
//- FunctionTable:
//-   https://datasheetspdf.com/pdf-file/182163/PanasonicSemiconductor/AN6551/1
//-
static NETLIST_START(AN6551_SIL)
{
	OPAMP(A, "AN6551")
	OPAMP(B, "AN6551")

	NET_C(A.GND, B.GND)

	ALIAS(1, A.VCC)
	ALIAS(2, A.OUT)
	ALIAS(3, A.MINUS)
	ALIAS(4, A.PLUS)
	ALIAS(5, A.GND)
	ALIAS(6, B.PLUS)
	ALIAS(7, B.MINUS)
	ALIAS(8, B.OUT)
	ALIAS(9, B.VCC)
}

#if USE_LM3900_MODEL == 0
static NETLIST_START(LM3900)
{

	/*
	 *  Fast norton opamp model without bandwidth
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, R1.1) // Positive input
	ALIAS(MINUS, R2.1) // Negative input
	ALIAS(OUT, G1.OP) // Opamp output ...
	ALIAS(GND, G1.ON)  // V- terminal
	ALIAS(VCC, DUMMY.1)  // V+ terminal

	RES(DUMMY, RES_K(1))
	NET_C(DUMMY.2, GND)

	/* The opamp model */

	RES(R1, 1)
	RES(R2, 1)
	NET_C(R1.1, G1.IP)
	NET_C(R2.1, G1.IN)
	NET_C(R1.2, R2.2, G1.ON)
	VCVS(G1, 10000000)
	//PARAM(G1.RI, 1)
	PARAM(G1.RO, RES_K(8))

}
#endif

#if USE_LM3900_MODEL == 1
//  LTSPICE MODEL OF LM3900 FROM NATIONAL SEMICONDUCTOR
//  MADE BY HELMUT SENNEWALD, 8/6/2004
//  THE LM3900 IS A SO CALLED NORTON AMPLIFIER.
//
//  PIN ORDER:    IN+ IN- VCC VSS OUT
static NETLIST_START(LM3900)
{
	PARAM(E1.G, 0.5)
	//ALIAS(IN+, Q2.B)
	//ALIAS(IN-, Q2.C)
	//ALIAS(VCC, Q10.C)
	//ALIAS(VSS, Q2.E)

	ALIAS(PLUS, Q2.B)
	ALIAS(MINUS, Q2.C)
	ALIAS(VCC, Q10.C)
	ALIAS(GND, Q2.E)
	ALIAS(OUT, Q6.C)

	//CS(B1/*I=LIMIT(0, V(VCC,VSS)/10K, 0.2m)*/)
	CS(B1, 2e-4)
	CAP(C1, CAP_P(6.000000))
	VCVS(E1, 1)
	QBJT_EB(Q1, "LM3900_NPN1")
	QBJT_EB(Q10, "LM3900_NPN1")
	QBJT_EB(Q11, "LM3900_NPN1")
	QBJT_EB(Q12, "LM3900_NPN1")
	QBJT_EB(Q2, "LM3900_NPN1")
	QBJT_EB(Q3, "LM3900_NPN1")
	QBJT_EB(Q4, "LM3900_PNP1")
	QBJT_EB(Q5, "LM3900_PNP1")
	QBJT_EB(Q6, "LM3900_PNP1")
	QBJT_EB(Q7, "LM3900_PNP1")
	QBJT_EB(Q8, "LM3900_NPN1")
	QBJT_EB(Q9, "LM3900_NPN1")
	RES(R1, RES_K(2.000000))
	RES(R6, RES_K(1.600000))
	NET_C(Q11.B, Q12.B, R6.2)
	NET_C(Q5.C, Q5.B, B1.P, Q4.B)
	NET_C(Q8.C, Q8.B, B1.N, R1.1, E1.IP)
	NET_C(Q9.B, R1.2)
	NET_C(R6.1, E1.OP)
	NET_C(Q10.C, Q5.E, Q4.E, Q11.C, Q12.C)
	NET_C(Q2.C, Q3.B, Q12.E)
	NET_C(Q2.E, Q3.E, Q9.E, C1.2, Q1.E, Q8.E, E1.ON, E1.IN, Q7.C)
	NET_C(Q3.C, Q6.B, C1.1, Q7.B)
	NET_C(Q6.E, Q10.B, Q4.C)
	NET_C(Q6.C, Q10.E, Q9.C, Q7.E)
	NET_C(Q2.B, Q1.C, Q1.B, Q11.E)
}
#endif

#if USE_LM3900_MODEL == 2
static NETLIST_START(LM3900)
{
	OPAMP(A, "LM3900")

	DIODE(D1, "D(IS=1e-15 N=1)")
	CCCS(CS1, 1) // Current Mirror

	ALIAS(VCC, A.VCC)
	ALIAS(GND, A.GND)
	ALIAS(PLUS, A.PLUS)
	ALIAS(MINUS, A.MINUS)
	ALIAS(OUT, A.OUT)

	NET_C(A.PLUS, CS1.IP)
	NET_C(D1.A, CS1.IN)
	NET_C(CS1.ON, A.MINUS)
	NET_C(CS1.OP, A.GND, D1.K)

}
#endif

#if USE_LM3900_MODEL == 3
static NETLIST_START(LM3900)
{

	ALIAS(VCC, Q5.C)
	ALIAS(GND, Q1.E)
	ALIAS(PLUS, Q1.B)
	ALIAS(MINUS, Q1.C)
	ALIAS(OUT, Q5.E)

	CAP(C1, CAP_P(6.000000))
	CS(I1, 1.300000e-3)
	CS(I2, 200e-6)
	QBJT_EB(Q1, "NPN")
	QBJT_EB(Q2, "NPN")
	QBJT_EB(Q3, "PNP")
	QBJT_EB(Q4, "PNP")
	QBJT_EB(Q5, "NPN")
	QBJT_EB(Q6, "NPN")
	NET_C(Q3.E, Q5.B, I2.2)
	NET_C(Q3.C, Q4.E, Q5.E, I1.1)
	NET_C(Q5.C, I2.1)
	NET_C(Q1.B, Q6.C, Q6.B)
	NET_C(Q1.E, Q2.E, Q4.C, C1.2, I1.2, Q6.E)
	NET_C(Q1.C, Q2.B)
	NET_C(Q2.C, Q3.B, Q4.B, C1.1)
}
#endif

#if USE_LM3900_MODEL == 4
static NETLIST_START(LM3900)
{
	OPAMP(A, "OPAMP(TYPE=3 VLH=0.5 VLL=0.03 FPF=2k UGF=2.5M SLEW=1M RI=10M RO=100 DAB=0.0015)")

	DIODE(D1, "D(IS=6e-15 N=1)")
	DIODE(D2, "D(IS=6e-15 N=1)")
	CCCS(CS1, 1) // Current Mirror

	ALIAS(VCC, A.VCC)
	ALIAS(GND, A.GND)
	ALIAS(OUT, A.OUT)

	ALIAS(PLUS, CS1.IP)
	NET_C(D1.A, CS1.IN)
	NET_C(A.GND, D1.K)

	CS(CS_BIAS, 10e-6)
	NET_C(A.VCC, CS_BIAS.P)

	ALIAS(MINUS, CS1.OP)
	NET_C(CS1.ON, A.GND)

	CCVS(VS1, 200000) // current-to-voltage gain
	NET_C(CS1.OP, VS1.IP)
	NET_C(VS1.IN, CS_BIAS.N, D2.A)
	NET_C(D2.K, A.GND)
	NET_C(VS1.OP, A.MINUS)
	NET_C(VS1.ON, A.PLUS, A.GND)
}
#endif

NETLIST_START(opamp_lib)
{
	LOCAL_LIB_ENTRY(opamp_layout_4_4_11)
	LOCAL_LIB_ENTRY(opamp_layout_2_8_4)
	LOCAL_LIB_ENTRY(opamp_layout_2_13_9_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_7_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_8_5)
	LOCAL_LIB_ENTRY(opamp_layout_1_11_6)

	// FIXME: JFET Opamp may need better model
	// VLL and VHH for +-6V  RI=10^12 (for numerical stability 10^9 is used below
	// RO from data sheet
	NET_MODEL("TL084       OPAMP(TYPE=3 VLH=0.75 VLL=0.75 FPF=10 UGF=3000k SLEW=13M RI=1000M RO=192 DAB=0.0014)")

	NET_MODEL("LM324       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.00075)")
	NET_MODEL("LM358       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.001)")
	NET_MODEL("MB3614      OPAMP(TYPE=3 VLH=1.4 VLL=0.02 FPF=3 UGF=1000k SLEW=0.6M RI=1000k RO=100 DAB=0.002)")
	NET_MODEL("UA741       OPAMP(TYPE=3 VLH=1.0 VLL=1.0 FPF=5 UGF=1000k SLEW=0.5M RI=2000k RO=75 DAB=0.0017)")
	NET_MODEL("LM747       OPAMP(TYPE=3 VLH=1.0 VLL=1.0 FPF=5 UGF=1000k SLEW=0.5M RI=2000k RO=50 DAB=0.0017)")
	NET_MODEL("LM747A      OPAMP(TYPE=3 VLH=2.0 VLL=2.0 FPF=5 UGF=1000k SLEW=0.7M RI=6000k RO=50 DAB=0.0015)")
	NET_MODEL("LM748       OPAMP(TYPE=3 VLH=2.0 VLL=2.0 FPF=5 UGF=800k SLEW=0.7M RI=800k RO=60 DAB=0.001)")
	// TI and Motorola datasheets differ - below are Motorola values, SLEW is average of LH and HL
	NET_MODEL("LM3900      OPAMP(TYPE=3 VLH=1.0 VLL=0.03 FPF=2k UGF=4M SLEW=10M RI=10M RO=2k DAB=0.0015)")

	NET_MODEL("AN6551      OPAMP(TYPE=3 VLH=1.0 VLL=0.03 FPF=20 UGF=2M SLEW=1M RI=10M RO=200 DAB=0.0015)")

	#if USE_LM3900_MODEL == 1
	NET_MODEL("LM3900_NPN1 NPN(IS=1E-14 BF=150 TF=1E-9 CJC=1E-12 CJE=1E-12 VAF=150 RB=100 RE=5 IKF=0.002)")
	NET_MODEL("LM3900_PNP1 PNP(IS=1E-14 BF=40 TF=1E-7 CJC=1E-12 CJE=1E-12 VAF=150 RB=100 RE=5)")
#endif
	LOCAL_LIB_ENTRY(MB3614_DIP)
	LOCAL_LIB_ENTRY(MC3340_DIP)
	LOCAL_LIB_ENTRY(TL081_DIP)
	LOCAL_LIB_ENTRY(TL082_DIP)
	LOCAL_LIB_ENTRY(TL084_DIP)
	LOCAL_LIB_ENTRY(LM324_DIP)
	LOCAL_LIB_ENTRY(LM348_DIP)
	LOCAL_LIB_ENTRY(LM358_DIP)
	LOCAL_LIB_ENTRY(LM2902_DIP)
	LOCAL_LIB_ENTRY(UA741_DIP8)
	LOCAL_LIB_ENTRY(UA741_DIP10)
	LOCAL_LIB_ENTRY(UA741_DIP14)
	LOCAL_LIB_ENTRY(MC1558_DIP)
	LOCAL_LIB_ENTRY(LM747_DIP)
	LOCAL_LIB_ENTRY(LM747A_DIP)
	LOCAL_LIB_ENTRY(LM3900)
	LOCAL_LIB_ENTRY(AN6551_SIL)

}
