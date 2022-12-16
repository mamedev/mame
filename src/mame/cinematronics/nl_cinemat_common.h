// license:CC0
// copyright-holders:Aaron Giles,Couriersud
#ifndef MAME_AUDIO_NL_CINEMAT_COMMON_H
#define MAME_AUDIO_NL_CINEMAT_COMMON_H


//
// Misc common devices used by Cinematronics games.
//
// Eventually these should be included in the core.
//
// Known problems/issues:
//
//    * There are 5 different models of the CA3080 presented here.
//       More analysis on the pros/cons of each should be done.
//
//    * The TL182 analog switch is cobbled together from a pair of
//       CD4066 gates and a pair of inverters on the inputs. This
//       seems to work but perhaps should be properly added as a
//       system model.
//
//    * The 2N6426 Darlington transistors are not properly emulated.
//       Currently they are modeled as a pair of 2N3904s. A pspice
//       model ported into the netlist schema creates a very poor
//       balance with the background noise in Space Wars/Barrier.
//

#include "netlist/devices/net_lib.h"


//
// Direct mappings
//

// AMI S2688 is compatible with MM5837
#define AMI_S2688 MM5837_DIP

// LM555/566 is compatible with NE555/566
#define LM555_DIP NE555_DIP
#define LM566_DIP NE566_DIP

// alias LS devices to real devices
#define TTL_74LS00_DIP TTL_7400_DIP
#define TTL_74LS02_DIP TTL_7402_DIP     // Quad 2-input Nor Gate
#define TTL_74S04_DIP TTL_7404_DIP      // Hex Inverting Gates
#define TTL_74LS04_DIP TTL_7404_DIP     // Hex Inverting Gates
#define TTL_74LS08_DIP TTL_7408_DIP     // Quad 2-Input AND Gates
#define TTL_74LS21_DIP TTL_7421_DIP     // Dual 4-Input AND Gates
#define TTL_74LS74_DIP TTL_7474_DIP     // Dual D Flip Flop
#define TTL_74LS75_DIP TTL_7475_DIP     // 4-Bit Bistable Latches with Complementary Outputs
#define TTL_74LS86_DIP TTL_7486_DIP     // Quad 2-Input Exclusive-OR Gates
#define TTL_74LS107_DIP TTL_74107_DIP
#define TTL_74S113_DIP TTL_74113_DIP
#define TTL_74S113A_DIP TTL_74113A_DIP
#define TTL_74LS123_DIP TTL_74123_DIP
#define TTL_74LS125_DIP TTL_74125_DIP
#define TTL_74LS157_DIP TTL_74157_DIP
#define TTL_74LS163_DIP TTL_74163_DIP
#define TTL_74LS164_DIP TTL_74164_DIP
#define TTL_74LS191_DIP TTL_74191_DIP
#define TTL_74LS259_DIP TTL_9334_DIP    // Seems to be pin-compatible
#define TTL_74LS377_DIP TTL_74377_DIP
#define TTL_74LS393_DIP TTL_74393_DIP



//
// Diode models
//

#define D_1N914(name) DIODE(name, "1N914")
#define D_1N914B(name) DIODE(name, "1N914")
#define D_1N5236B(name) ZDIODE(name, "1N5236B")
#define D_1N5240(name) ZDIODE(name, "1N5240")
#define D_1N5240B(name) ZDIODE(name, "1N5240B")



//
// Transistor models
//

#define Q_2N3904(name) QBJT_EB(name, "2N3904")

#define Q_2N3906(name) QBJT_EB(name, "2N3906")

#define Q_2N6107(name) QBJT_EB(name, "2N6107")

#define Q_2N6292(name) QBJT_EB(name, "2N6292")

#define Q_2N5210(name) QBJT_EB(name, "2N5210")



//
// 556 is just two 555s in one package
//

#define NE556_DIP(name) SUBMODEL(_NE556_DIP, name)
#define LM556_DIP NE556_DIP

static NETLIST_START(_NE556_DIP)
{
	NE555(A)
	NE555(B)

	NET_C(A.GND, B.GND)
	NET_C(A.VCC, B.VCC)

	DIPPINS(      /*        +--------------+        */
		 A.DISCH, /* 1DISCH |1     ++    14| VCC    */ A.VCC,
		A.THRESH, /* 1THRES |2           13| 2DISCH */ B.DISCH,
		  A.CONT, /*  1CONT |3           12| 2THRES */ B.THRESH,
		 A.RESET, /* 1RESET |4   NE556   11| 2CONT  */ B.CONT,
		   A.OUT, /*   1OUT |5           10| 2RESET */ B.RESET,
		  A.TRIG, /*  1TRIG |6            9| 2OUT   */ B.OUT,
		   A.GND, /*    GND |7            8| 2TRIG  */ B.TRIG
				  /*        +--------------+        */
	)
}



//
// TL182 analog switch
//

#define TL182_DIP(name) SUBMODEL(_TL182_DIP, name)

static NETLIST_START(_TL182_DIP)
{
	CD4066_GATE(A)
	CD4066_GATE(B)

	NET_C(A.VDD, B.VDD)
	NET_C(A.VSS, B.VSS)

	PARAM(A.BASER, 270.0)
	PARAM(B.BASER, 270.0)

	RES(VR, 100)
	NC_PIN(NC)

	TTL_7406_GATE(AINV)
	TTL_7406_GATE(BINV)
	NET_C(AINV.VCC, BINV.VCC, A.VDD)
	NET_C(AINV.GND, BINV.GND, A.VSS)
	NET_C(AINV.Y, A.CTL)
	NET_C(BINV.Y, B.CTL)

	DIPPINS(   /*      +--------------+      */
		A.R.1, /*   1S |1     ++    14| 2S   */ B.R.1,
		A.R.2, /*   1D |2           13| 2D   */ B.R.2,
		 NC.I, /*   NC |3           12| NC   */ NC.I,
		 NC.I, /*   NC |4   TL182   11| NC   */ NC.I,
	   AINV.A, /*   1A |5           10| 2A   */ BINV.A,
		 VR.1, /*  VCC |6            9| VEE  */ VR.2,
		A.VDD, /*  VLL |7            8| VREF */ A.VSS
			   /*      +--------------+      */
	)
}



//
// 2N6426, Darlington transistor
//

#define Q_2N6426(name) SUBMODEL(_Q_2N6426, name)

#if 0

//
// This model causes the background noise level to domainate all other sounds in spacewar?
//

// Model dervied from https://www.onsemi.com/support/design-resources/models?rpn=2N6284
static NETLIST_START(_Q_2N6426)
{

	QBJT_EB(Q1, "NPN(IS=1.73583e-11 BF=831.056 NF=1.05532 VAF=957.147 IKF=0.101183 ISE=1.65383e-10 NE=1.59909 BR=2.763 NR=1.03428 VAR=4.18534 IKR=0.0674174 ISC=1.00007e-13 NC=2.00765 RB=22.2759 IRB=0.208089 RBM=22.2759 RE=0.0002 RC=0.001 XTB=2.12676 XTI=1.82449 EG=1.05 CJE=2.62709e-10 VJE=0.95 MJE=0.23 TF=1e-09 XTF=1 VTF=10 ITF=0.01 CJC=3.59851e-10 VJC=0.845279 MJC=0.23 XCJC=0.9 FC=0.5 TR=1e-07 PTF=0 KF=0 AF=1)")

	QBJT_EB(Q2, "NPN(IS=1.73583e-11 BF=831.056 NF=1.05532 VAF=957.147 IKF=0.101183 ISE=1.65383e-10 NE=1.59909 BR=2.763 NR=1.03428 VAR=4.18534 IKR=0.0674174 ISC=1.00007e-13 NC=2.00765 RB=22.2759 IRB=0.208089 RBM=22.2759 RE=0.0002 RC=0.001 XTB=2.12676 XTI=1.82449 EG=1.05 CJE=2.62709e-10 VJE=0.95 MJE=0.23 TF=1e-09 XTF=1 VTF=10 ITF=0.01 CJC=0 VJC=0.845279 MJC=0.23 XCJC=0.9 FC=0.5 TR=1e-07 PTF=0 KF=0 AF=1)")  // NPN

	DIODE(D1, "D(IS=1e-12 RS=10.8089 N=1.00809 XTI=3.00809 CJO=0 VJ=0.75 M=0.33 FC=0.5)")
	RES(R1, RES_K(8))
	RES(R2, 50)

	ALIAS(B, Q1.B)
	ALIAS(C, Q1.C)
	ALIAS(E, Q2.E)
	NET_C(Q1.C, Q2.C, D1.K)
	NET_C(Q1.B, R1.1)
	NET_C(Q2.E, D1.A, R2.2)
	NET_C(Q1.E, Q2.B, R1.2, R2.1)
}

#else

// super brain-dead model I threw together from a pair of 2N3904
static NETLIST_START(_Q_2N6426)
{
	QBJT_EB(Q1, "NPN")
	QBJT_EB(Q2, "NPN")

	ALIAS(B, Q1.B)
	ALIAS(C, Q1.C)
	ALIAS(E, Q2.E)
	NET_C(Q1.C, Q2.C)
	NET_C(Q1.E, Q2.B)
}

#endif



//
// LM3900, quad op-amp
//

#define LM3900_DIP(name) SUBMODEL(_LM3900_DIP, name)

static NETLIST_START(_LM3900_DIP)
{
	LM3900(A)
	LM3900(B)
	LM3900(C)
	LM3900(D)

	NET_C(A.VCC, B.VCC, C.VCC, D.VCC)
	NET_C(A.GND, B.GND, C.GND, D.GND)

	DIPPINS(   /*      +--------------+      */
	   A.PLUS, /* 1IN+ |1     ++    14| VCC  */ A.VCC,
	   B.PLUS, /* 2IN+ |2           13| 3IN+ */ C.PLUS,
	  B.MINUS, /* 2IN- |3           12| 4IN+ */ D.PLUS,
		B.OUT, /* 2OUT |4  LM3900   11| 4IN- */ D.MINUS,
		A.OUT, /* 1OUT |5           10| 4OUT */ D.OUT,
	  A.MINUS, /* 1IN- |6            9| 3OUT */ C.OUT,
		A.GND, /*  GND |7            8| 3IN- */ C.MINUS
			   /*      +--------------+      */
	)
}



// *************************************************
//  EQUIVALENT SUBSTITUTE FOR CA3080 IS NTE996
//  (SAME 8-PIN DIP PINOUT)
// *************************************************
//
//  CA3080 OPERATIONAL TRANSCONDUCTANCE AMPLIFIER
//
//  SPICE (SIMULATION PROGRAM WITH INTEGRATED CIRCUIT EMPHASIS)
//  SUBCIRCUIT
//
//  CONNECTIONS:
//               INVERTING INPUT
//                | NON-INVERTING INPUT
//                |  |  NEGATIVE POWER SUPPLY
//                |  |  |  I BIAS
//                |  |  |  | OUTPUT
//                |  |  |  |  |  POSITIVE POWER SUPPLY
//                |  |  |  |  |  |
//.SUBCKT CA3080  2  3  4  5  6  7

//
// DIP mappings use the submodels below for CA3080
//
#define CA3080_FAST_DIP(name) SUBMODEL(_CA3080_FAST_DIP, name)
#define CA3080_SLOW_DIP(name) SUBMODEL(_CA3080_SLOW_DIP, name)


//
// Default to the fast model unless otherwise directed
//
#ifndef CA3080_DIP
#define CA3080_DIP CA3080_FAST_DIP
#endif


//
// Fast model: works well for most cases in Cinematronics games,
// except when used in a network with multiple CA3080's, due to
// the presence of an AFUNC.
//
static NETLIST_START(_CA3080_FAST_DIP)
{
	ALIAS(2, F.A0) // -
	ALIAS(3, F.A1) // +
	ALIAS(4, F.A2) // V-
	ALIAS(5, RIABC.1) // IB
	ALIAS(6, VO.OP)  // FIXME
	ALIAS(7, F.A4) // V+

	RES(RI, 26000)
	NET_C(RI.1, F.A0)
	NET_C(RI.2, F.A1)
	// Delivers I0
	AFUNC(F, 5, "max(-0.5e-3, min(0.5e-3, 19.2 * (A3 - A2) * A0))")
	RES(RIABC, 1)
	NET_C(RIABC.2, F.A2)
	NET_C(RIABC.1, F.A3) // IB
	VCCS(VO, 1)
	ANALOG_INPUT(XGND, 0)
	NET_C(XGND, VO.IN, VO.ON) // FIXME: assume symmetric supply
	NET_C(F.Q, VO.IP)
}


//
// Slow model: derived from
//    https://github.com/xxv/gedasymbols/blob/master/www/user/john_doty/models/opamp/ca3080.mod
//
// This seems to produce reasonable results as a slow drop-in
// replacement for the fast model above in solarq, but is quite
// slow. One advantage is that it works in a network with multiple
// CA3080's.
//
static NETLIST_START(_CA3080_SLOW_DIP)
{
//
// These items are common to several models
//
#define CA3080_D(name) DIODE(name, "D(IS=2p RS=5 BV=40 CJO=3p TT=6n)")
#define CA3080_NPN(name) QBJT_EB(name, "NPN(IS=21.48f XTI=3 EG=1.11 VAF=80 BF=550 ISE=50f NE=1.5 IKF=10m XTB=1.5 BR=.1 ISC=10f NC=2 IKR=3m RC=10 CJC=800f MJC=.3333 VJC=.75 FC=.5 CJE=1.3p MJE=.3333 VJE=.75 TR=30n TF=400P ITF=30m XTF=1 VTF=10 CJS=5.8P MJS=.3333 VJS=.75)")
#define CA3080_PNP(name) QBJT_EB(name, "PNP(IS=50f XTI=3 EG=1.11 VAF=80 BF=100 ISE=130f NE=1.5 IKF=1m XTB=1.5 BR=1 ISC=0 NC=2 IKR=0 RC=0 CJC=4p MJC=.3333 VJC=.75 FC=.5 CJE=1.4p MJE=.3333 VJE=.75 TR=500n TF=23n ITF=.1 XTF=1 VTF=10 CJS=5.5P MJS=.3333 VJS=.75)")

	CA3080_D(D1)
	CA3080_D(D2)
	CA3080_NPN(Q1)
	CA3080_PNP(Q2)
	CA3080_PNP(Q3)
	CA3080_NPN(Q4)
	CA3080_NPN(Q5)
	CA3080_PNP(Q6)
	CA3080_PNP(Q7)
	CA3080_PNP(Q8)
	CA3080_PNP(Q9)
	CA3080_NPN(Q10)
	CA3080_NPN(Q11)
	CA3080_NPN(Q12)
	CA3080_NPN(Q13)
	CA3080_PNP(Q14)
	CA3080_PNP(Q15)

	ALIAS(2, Q10.B)     // N1
	ALIAS(3, Q5.B)      // N28
	ALIAS(4, Q1.E)      // N13
	ALIAS(5, Q1.B)      // N11
	ALIAS(6, Q6.C)      // N30
	ALIAS(7, Q8.E)      // N8
	NET_C(Q8.E, Q9.E, Q14.E, Q15.E)     // N8
	NET_C(Q1.B, Q1.C, Q4.B)             // N11
	NET_C(Q1.E, Q4.E, Q11.E, Q12.E)     // N13
	NET_C(Q6.C, Q7.C, Q13.C)            // N30
	NET_C(Q3.B, Q10.C, Q14.C, D1.A)     // N1N13
	NET_C(Q2.E, Q14.B, Q15.C, Q15.B)    // N1N15
	NET_C(Q2.B, Q3.E, D1.K)             // N1N17
	NET_C(Q2.C, Q3.C, Q11.C, Q13.B)     // N1N22
	NET_C(Q5.C, Q6.B, Q9.C, D2.A)       // N1N32
	NET_C(Q6.E, Q7.B, D2.K)             // N1N34
	NET_C(Q7.E, Q8.C, Q8.B, Q9.B)       // N1N36
	NET_C(Q4.C, Q5.E, Q10.E)            // N1N52
	NET_C(Q11.B, Q12.C, Q12.B, Q13.E)   // N1N44
}



//
// List of local models to include to make MAME happy
//

#define CINEMAT_LOCAL_MODELS \
	LOCAL_SOURCE(_NE556_DIP) \
	LOCAL_SOURCE(_TL182_DIP) \
	LOCAL_SOURCE(_Q_2N6426) \
	LOCAL_SOURCE(_LM3900_DIP) \
	LOCAL_SOURCE(_CA3080_FAST_DIP) \
	LOCAL_SOURCE(_CA3080_SLOW_DIP) \


#endif // MAME_AUDIO_NL_CINEMAT_COMMON_H
