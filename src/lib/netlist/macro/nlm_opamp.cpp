// license:GPL-2.0+
// copyright-holders:Couriersud
#include "nlm_opamp.h"
#include "netlist/devices/net_lib.h"

/*
 * 0 = Basic hack (Norton with just amplification, no voltage cutting)
 * 1 = Model from LTSPICE mailing list - slow!
 * 2 = Simplified model using diode inputs and netlist TYPE=3
 * 3 = Model according to datasheet
 *
 * For Money Money 1 and 3 delivery comparable results.
 * 3 is simpler (less BJTs) and converges a lot faster.
 */
#define USE_LM3900_MODEL (3)

/*
 *   Generic layout with 4 opamps, VCC on pin 4 and GND on pin 11
 */

static NETLIST_START(opamp_layout_4_4_11)
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
NETLIST_END()

/*
 *   Generic layout with 2 opamps, VCC on pin 8 and GND on pin 4
 */

static NETLIST_START(opamp_layout_2_8_4)
	DIPPINS(        /*   +--------------+   */
		A.OUT,      /*   |1     ++     8|   */ A.VCC,
		A.MINUS,    /*   |2            7|   */ B.OUT,
		A.PLUS,     /*   |3            6|   */ B.MINUS,
		A.GND,      /*   |4            5|   */ B.PLUS
					/*   +--------------+   */
	)
	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)
NETLIST_END()

/*
 *   Generic layout with 2 opamps, VCC+ on pins 9/13,  VCC- on pin 4 and compensation
 */

static NETLIST_START(opamp_layout_2_13_9_4)
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
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 7, VCC- on pin 4 and compensation
 */

static NETLIST_START(opamp_layout_1_7_4)
	DIPPINS(        /*   +--------------+   */
		OFFSET.N1,  /*   |1     ++     8|   */ NC,
		MINUS,      /*   |2            7|   */ VCC.PLUS,
		PLUS,       /*   |3            6|   */ OUT,
		VCC.MINUS,  /*   |4            5|   */ OFFSET.N2
					/*   +--------------+   */
	)
	NET_C(A.GND, VCC.MINUS)
	NET_C(A.VCC, VCC.PLUS)
	NET_C(A.MINUS, MINUS)
	NET_C(A.PLUS, PLUS)
	NET_C(A.OUT, OUT)
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 8, VCC- on pin 5 and compensation
 */

static NETLIST_START(opamp_layout_1_8_5)
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
NETLIST_END()

/*
 *   Generic layout with 1 opamp, VCC+ on pin 11, VCC- on pin 6 and compensation
 */

static NETLIST_START(opamp_layout_1_11_6)
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
NETLIST_END()

static NETLIST_START(MB3614_DIP)
	OPAMP(A, "MB3614")
	OPAMP(B, "MB3614")
	OPAMP(C, "MB3614")
	OPAMP(D, "MB3614")

	INCLUDE(opamp_layout_4_4_11)

NETLIST_END()

static NETLIST_START(LM324_DIP)
	OPAMP(A, "LM324")
	OPAMP(B, "LM324")
	OPAMP(C, "LM324")
	OPAMP(D, "LM324")

	INCLUDE(opamp_layout_4_4_11)

NETLIST_END()

static NETLIST_START(LM2902_DIP)
	// Same datasheet and mostly same characteristics as LM324
	OPAMP(A, "LM324")
	OPAMP(B, "LM324")
	OPAMP(C, "LM324")
	OPAMP(D, "LM324")

	INCLUDE(opamp_layout_4_4_11)

NETLIST_END()

static NETLIST_START(LM358_DIP)
	OPAMP(A, "LM358")
	OPAMP(B, "LM358")

	INCLUDE(opamp_layout_2_8_4)

NETLIST_END()

static NETLIST_START(UA741_DIP8)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_7_4)

NETLIST_END()

static NETLIST_START(UA741_DIP10)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_8_5)

NETLIST_END()

static NETLIST_START(UA741_DIP14)
	OPAMP(A, "UA741")

	INCLUDE(opamp_layout_1_11_6)

NETLIST_END()

static NETLIST_START(LM747_DIP)
	OPAMP(A, "LM747")
	OPAMP(B, "LM747")

	INCLUDE(opamp_layout_2_13_9_4)
	NET_C(A.VCC, B.VCC)

NETLIST_END()

static NETLIST_START(LM747A_DIP)
	OPAMP(A, "LM747A")
	OPAMP(B, "LM747A")

	INCLUDE(opamp_layout_2_13_9_4)
	NET_C(A.VCC, B.VCC)

NETLIST_END()

#if USE_LM3900_MODEL == 0
static NETLIST_START(LM3900)

	/*
	 *  Fast norton opamp model without bandwidth
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, R1.1) // Positive input
	ALIAS(MINUS, R2.1) // Negative input
	ALIAS(OUT, G1.OP) // Opamp output ...
	ALIAS(VCC, G1.ON)  // V- terminal
	ALIAS(GND, DUMMY.I)  // V+ terminal

	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	RES(R1, 1)
	RES(R2, 1)
	NET_C(R1.1, G1.IP)
	NET_C(R2.1, G1.IN)
	NET_C(R1.2, R2.2, G1.ON)
	VCVS(G1)
	PARAM(G1.G, 10000000)
	//PARAM(G1.RI, 1)
	PARAM(G1.RO, RES_K(8))

NETLIST_END()
#endif

#if USE_LM3900_MODEL == 1
//  LTSPICE MODEL OF LM3900 FROM NATIONAL SEMICONDUCTOR
//  MADE BY HELMUT SENNEWALD, 8/6/2004
//  THE LM3900 IS A SO CALLED NORTON AMPLIFIER.
//
//  PIN ORDER:    IN+ IN- VCC VSS OUT
static NETLIST_START(LM3900)
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
	VCVS(E1)
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
NETLIST_END()
#endif

#if USE_LM3900_MODEL == 2
static NETLIST_START(LM3900)
	OPAMP(A, "LM3900")

	DIODE(D1, "D(IS=1e-15 N=1)")
	CCCS(CS1) // Current Mirror

	ALIAS(VCC, A.VCC)
	ALIAS(GND, A.GND)
	ALIAS(PLUS, A.PLUS)
	ALIAS(MINUS, A.MINUS)
	ALIAS(OUT, A.OUT)

	NET_C(A.PLUS, CS1.IP)
	NET_C(D1.A, CS1.IN)
	NET_C(CS1.OP, A.MINUS)
	NET_C(CS1.ON, A.GND, D1.K)

NETLIST_END()
#endif

#if USE_LM3900_MODEL == 3
static NETLIST_START(LM3900)

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
NETLIST_END()
#endif

NETLIST_START(OPAMP_lib)
	LOCAL_LIB_ENTRY(opamp_layout_4_4_11)
	LOCAL_LIB_ENTRY(opamp_layout_2_8_4)
	LOCAL_LIB_ENTRY(opamp_layout_2_13_9_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_7_4)
	LOCAL_LIB_ENTRY(opamp_layout_1_8_5)
	LOCAL_LIB_ENTRY(opamp_layout_1_11_6)

	NET_MODEL("LM324       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.00075)")
	NET_MODEL("LM358       OPAMP(TYPE=3 VLH=2.0 VLL=0.2 FPF=5 UGF=500k SLEW=0.3M RI=1000k RO=50 DAB=0.001)")
	NET_MODEL("MB3614      OPAMP(TYPE=3 VLH=1.4 VLL=0.02 FPF=10 UGF=1000k SLEW=0.6M RI=1000k RO=50 DAB=0.002)")
	NET_MODEL("UA741       OPAMP(TYPE=3 VLH=1.0 VLL=1.0 FPF=5 UGF=1000k SLEW=0.5M RI=2000k RO=75 DAB=0.0017)")
	NET_MODEL("LM747       OPAMP(TYPE=3 VLH=1.0 VLL=1.0 FPF=5 UGF=1000k SLEW=0.5M RI=2000k RO=50 DAB=0.0017)")
	NET_MODEL("LM747A      OPAMP(TYPE=3 VLH=2.0 VLL=2.0 FPF=5 UGF=1000k SLEW=0.7M RI=6000k RO=50 DAB=0.0015)")
	// TI and Motorola Datasheets differ - below are Motorola values values SLEW is average of LH and HL
	NET_MODEL("LM3900      OPAMP(TYPE=3 VLH=1.0 VLL=0.03 FPF=2k UGF=4M SLEW=10M RI=10M RO=2k DAB=0.0015)")

#if USE_LM3900_MODEL == 1
	NET_MODEL("LM3900_NPN1 NPN(IS=1E-14 BF=150 TF=1E-9 CJC=1E-12 CJE=1E-12 VAF=150 RB=100 RE=5 IKF=0.002)")
	NET_MODEL("LM3900_PNP1 PNP(IS=1E-14 BF=40 TF=1E-7 CJC=1E-12 CJE=1E-12 VAF=150 RB=100 RE=5)")
#endif
	LOCAL_LIB_ENTRY(MB3614_DIP)
	LOCAL_LIB_ENTRY(LM324_DIP)
	LOCAL_LIB_ENTRY(LM358_DIP)
	LOCAL_LIB_ENTRY(LM2902_DIP)
	LOCAL_LIB_ENTRY(UA741_DIP8)
	LOCAL_LIB_ENTRY(UA741_DIP10)
	LOCAL_LIB_ENTRY(UA741_DIP14)
	LOCAL_LIB_ENTRY(LM747_DIP)
	LOCAL_LIB_ENTRY(LM747A_DIP)
	LOCAL_LIB_ENTRY(LM3900)

NETLIST_END()
