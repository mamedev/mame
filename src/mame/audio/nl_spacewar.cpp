// license:CC0
// copyright-holders:Aaron Giles,Couriersud

//
// Netlist for Space Wars/Barrier
//
// Derived from the schematics and parts list in the Space Wars
// and Barrier manuals. The Space Wars schematic is a hand-drawn
// mess, and hard to read in places, while the Barrier schematic
// is computer-drawn and much easier to read.
//
// Comparing the two schematics reveals that Barrier is almost
// identical, but simplified by disabling some of the analog
// switches that we not needed. It looks like a few resistor
// values were changed or inserted in a few paths.
//
// Known problems/issues:
//
//    * The core noise source is supposed to be created via a
//       pair of transistors, one with an open base. Because this
//       does not model correctly, this part of the circuit is
//       replaced with a generic noise device. The characteristics
//       of this noise are pretty different compared to recordings
//       of the original, and affects all the sounds.
//
//    * The Barrier schematics show a connection betwee U8.6 and
//       R37.2; however, implementing this leads to a direct input
//       from the noise source at all times to the summing amp.
//       Suspecting this is a typo in the schematics.
//
//    * The 2N6426 Darlington transistors are not properly emulated.
//       Currently they are modeled as a pair of 2N3904s. A pspice
//       model ported into the netlist schema creates a very poor
//       balance with the background noise.
//
//    * The TL182 analog switch is cobbled together from a pair of
//       CD4066 gates and a pair of inverters on the inputs. This
//       seems to work but perhaps should be properly added as a
//       system model.
//
//    * Unsure if Barrier should have the noisy background like
//       Space Wars. Space Wars has a hard overall mute to suppress
//       it when the game isn't running, but Barrier does not.
//

#include "netlist/devices/net_lib.h"


//
// Initial compilation includes this section.
//

#ifndef SOUND_VARIANT


//
// The final amplifier is documented but not emulated.
//
#define EMULATE_FINAL_AMP	0


//
// Substitutes/models
//

NETLIST_START(_TL182_DIP)
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
NETLIST_END()

#define TL182_DIP(name) SUBMODEL(_TL182_DIP, name)


#if 0

//
// This model causes the background noise level to domainate all other sounds?
//

// Model dervied from https://www.onsemi.com/support/design-resources/models?rpn=2N6284
NETLIST_START(_QBJT_2N6426)
	QBJT_EB(Q1, "NPN(IS=1.73583e-11 BF=831.056 NF=1.05532 VAF=957.147 IKF=0.101183 ISE=1.65383e-10 NE=1.59909 BR=2.763 NR=1.03428 VAR=4.18534 IKR=0.0674174 ISC=1.00007e-13 NC=2.00765 RB=22.2759 IRB=0.208089 RBM=22.2759 RE=0.0002 RC=0.001 XTB=2.12676 XTI=1.82449 EG=1.05 CJE=2.62709e-10 VJE=0.95 MJE=0.23 TF=1e-09 XTF=1 VTF=10 ITF=0.01 CJC=3.59851e-10 VJC=0.845279 MJC=0.23 XCJC=0.9 FC=0.5 TR=1e-07 PTF=0 KF=0 AF=1)")
	QBJT_EB(Q2, "NPN(IS=1.73583e-11 BF=831.056 NF=1.05532 VAF=957.147 IKF=0.101183 ISE=1.65383e-10 NE=1.59909 BR=2.763 NR=1.03428 VAR=4.18534 IKR=0.0674174 ISC=1.00007e-13 NC=2.00765 RB=22.2759 IRB=0.208089 RBM=22.2759 RE=0.0002 RC=0.001 XTB=2.12676 XTI=1.82449 EG=1.05 CJE=2.62709e-10 VJE=0.95 MJE=0.23 TF=1e-09 XTF=1 VTF=10 ITF=0.01 CJC=0 VJC=0.845279 MJC=0.23 XCJC=0.9 FC=0.5 TR=1e-07 PTF=0 KF=0 AF=1)")	// NPN
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
NETLIST_END()

#else

// super brain-dead model I threw together from a pair of 2N3904
NETLIST_START(_QBJT_2N6426)
	QBJT_EB(Q1, "NPN(Is=6.734f Xti=3 Eg=1.11 Vaf=74.03 Bf=416.4 Ne=1.259 Ise=6.734 Ikf=66.78m Xtb=1.5 Br=.7371 Nc=2 Isc=0 Ikr=0 Rc=1 Cjc=3.638p Mjc=.3085 Vjc=.75 Fc=.5 Cje=4.493p Mje=.2593 Vje=.75 Tr=239.5n f=301.2p Itf=.4 Vtf=4 Xtf=2 Rb=10)")	// 2N3904 NPN
	QBJT_EB(Q2, "NPN(Is=6.734f Xti=3 Eg=1.11 Vaf=74.03 Bf=416.4 Ne=1.259 Ise=6.734 Ikf=66.78m Xtb=1.5 Br=.7371 Nc=2 Isc=0 Ikr=0 Rc=1 Cjc=3.638p Mjc=.3085 Vjc=.75 Fc=.5 Cje=4.493p Mje=.2593 Vje=.75 Tr=239.5n f=301.2p Itf=.4 Vtf=4 Xtf=2 Rb=10)")	// 2N3904 NPN

	ALIAS(B, Q1.B)
	ALIAS(C, Q1.C)
	ALIAS(E, Q2.E)
	NET_C(Q1.C, Q2.C)
	NET_C(Q1.E, Q2.B)
NETLIST_END()

#endif

#define QBJT_2N6426(name) SUBMODEL(_QBJT_2N6426, name)


//
// Now include ourselves twice, once for Space Wars and
// once for Barrier
//

#define VARIANT_SPACEWARS 	0
#define VARIANT_BARRIER		1

#define SOUND_VARIANT		(VARIANT_SPACEWARS)
#include "nl_spacewar.cpp"

#undef SOUND_VARIANT
#define SOUND_VARIANT 		(VARIANT_BARRIER)
#include "nl_spacewar.cpp"


#else


//
// Here is the start of the doubly-included code
//

#if (SOUND_VARIANT == VARIANT_SPACEWARS)
NETLIST_START(SpaceWars_schematics)
#else // (SOUND_VARIANT == VARIANT_BARRIER)
NETLIST_START(Barrier_schematics)
#endif

	// SPICE model taken directly from Fairchild Semiconductor datasheet
	NET_MODEL("2N3904 NPN(Is=6.734f Xti=3 Eg=1.11 Vaf=74.03 Bf=416.4 Ne=1.259 Ise=6.734 Ikf=66.78m Xtb=1.5 Br=.7371 Nc=2 Isc=0 Ikr=0 Rc=1 Cjc=3.638p Mjc=.3085 Vjc=.75 Fc=.5 Cje=4.493p Mje=.2593 Vje=.75 Tr=239.5n f=301.2p Itf=.4 Vtf=4 Xtf=2 Rb=10)")

	// SPICE model taken directly from Fairchild Semiconductor datasheet
	NET_MODEL("2N3906 PNP(Is=1.41f Xti=3 Eg=1.11 Vaf=18.7 Bf=180.7 Ne=1.5 Ise=0 Ikf=80m Xtb=1.5 Br=4.977 Nc=2 Isc=0 Ikr=0 Rc=2.5 Cjc=9.728p Mjc=.5776 Vjc=.75 Fc=.5 Cje=8.063p Mje=.3677 Vje=.75 Tr=33.42n Tf=179.3p Itf=.4 Vtf=4 Xtf=6 Rb=10)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6107
	NET_MODEL("2N6107 PNP(IS=7.62308e-14 BF=6692.56 NF=0.85 VAF=10 IKF=0.032192 ISE=2.07832e-13 NE=2.41828 BR=15.6629 NR=1.5 VAR=1.44572 IKR=0.32192 ISC=4.75e-16 NC=3.9375 RB=7.19824 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.355458 XTB=0.1 XTI=2.97595 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347177 TF=6.63757e-09 XTF=1.50003 VTF=1.0001 ITF=1 CJC=1.06717e-10 VJC=0.942679 MJC=0.245405 XCJC=0.8 FC=0.533334 CJS=0 VJS=0.75 MJS=0.5 TR=1.32755e-07 PTF=0 KF=0 AF=1)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6292
	NET_MODEL("2N6292 NPN(IS=9.3092e-13 BF=2021.8 NF=0.85 VAF=63.2399 IKF=1 ISE=1.92869e-13 NE=1.97024 BR=40.0703 NR=1.5 VAR=0.89955 IKR=10 ISC=4.92338e-16 NC=3.9992 RB=6.98677 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.326141 XTB=0.1 XTI=2.86739 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347174 TF=6.73756e-09 XTF=1.49917 VTF=0.997395 ITF=0.998426 CJC=1.06717e-10 VJC=0.942694 MJC=0.245406 XCJC=0.8 FC=0.533405 CJS=0 VJS=0.75 MJS=0.5 TR=6.0671e-08 PTF=0 KF=0 AF=1)")

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)
	ANALOG_INPUT(I_V25, 25)
	ANALOG_INPUT(I_VM25, -25)

	RES(R1, RES_K(18))
	RES(R2, 470)
	RES(R3, RES_K(47))
	RES(R4, RES_K(220))
	RES(R5, 470)
	RES(R6, 150)
	RES(R7, RES_K(180))
	RES(R8, RES_M(10))
	RES(R9, RES_K(39))
	RES(R10, RES_K(2.2))
	RES(R11, 470)
	RES(R12, RES_K(30))
	RES(R13, RES_K(8.2))
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	RES(R14, RES_K(120))
	RES(R15, RES_K(20))
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	RES(R14, RES_K(33))
	RES(R15, RES_K(15))
#endif
	RES(R16, RES_M(10))
	RES(R17, RES_K(10))
	RES(R18, RES_K(47))
	RES(R19, 820)
//	POT(R20, RES_K(10))		// don't understand this
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	RES(R21, 0)				// doesn't exist on Space Wars
	RES(R22, 0)				// doesn't exist on Space Wars
	RES(R23, 0)				// doesn't exist on Space Wars
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	RES(R21, 150)
	RES(R22, 1.35)
	RES(R23, 1.35)
#endif
	RES(R24, RES_K(47))
	RES(R25, 150)
	RES(R26, RES_K(160))
	RES(R27, 750)
	RES(R28, RES_K(150))	// completely illegible on Space Wars
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	POT(R29, RES_K(50))
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	POT(R29, RES_K(10))
#endif
	RES(R30, 470)
	RES(R31, 470)
	RES(R32, RES_K(1))
	RES(R33, RES_K(39))
	RES(R34, RES_K(6.6))
	RES(R35, RES_K(560))
	RES(R36, RES_M(1))
	RES(R37, RES_K(10))
#if (SOUND_VARIANT == VARIANT_BARRIER)
	RES(R38, RES_K(10))
#else // (SOUND_VARIANT == VARIANT_SPACEWARS)
	RES(R40, RES_K(120))
	RES(R41, RES_K(20))
	RES(R42, RES_K(1))
	RES(R43, RES_K(10))
#endif

	CAP(C1, CAP_U(1))
	CAP(C2, CAP_U(1))
	CAP(C3, CAP_U(0.01))
	CAP(C4, CAP_U(0.01))
	CAP(C5, CAP_U(0.1))
//	CAP(C6, CAP_U(4.7))		// not needed
#if (SOUND_VARIANT == VARIANT_BARRIER)
	CAP(C7, CAP_U(0.01))
#endif
	CAP(C8, CAP_U(1))
	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_P(220))
	CAP(C11, CAP_U(0.1))
	CAP(C12, CAP_U(0.01))
	CAP(C13, CAP_P(470))
	CAP(C14, CAP_P(470))
//	CAP(C15, CAP_U(50))		// not needed
//	CAP(C16, CAP_U(2.2))	// not needed
	CAP(C17, CAP_U(0.01))
	CAP(C18, CAP_U(15))		// Space Wars might be 33?
//	CAP(C19, CAP_U(50))		// not needed
//	CAP(C20, CAP_U(2.2))	// not needed
	CAP(C21, CAP_U(0.02))
	CAP(C22, CAP_U(0.1))
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	CAP(C23, CAP_U(0.1))
#endif

	DIODE(CR1, "1N914")
	DIODE(CR2, "1N914")
	DIODE(CR3, "1N914")
	DIODE(CR4, "1N914")
	DIODE(CR5, "1N914")
	DIODE(CR6, "1N914")

	QBJT_EB(Q1, "2N3906")	// PNP
	QBJT_EB(Q2, "2N3904")	// NPN
	QBJT_2N6426(Q3)			// NPN Darlington
	QBJT_EB(Q4, "2N6292")	// NPN
	QBJT_EB(Q5, "2N6107")	// PNP
	QBJT_2N6426(Q6)			// NPN Darlington
	QBJT_EB(Q7, "2N3904")	// NPN

	TL081_DIP(U1)			// Op. Amp.
	NET_C(U1.4, I_VM15)
	NET_C(U1.7, I_V15)

	TTL_7406_DIP(U2)		// Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

	TL081_DIP(U3)			// Op. Amp.
	NET_C(U3.4, I_VM15)
	NET_C(U3.7, I_V15)

//	TTL_7815_DIP(U4)		// +15V Regulator -- not needed

	TL182_DIP(U5)			// Analog switch
	NET_C(U5.6, I_V15)
	NET_C(U5.7, I_V5)
	NET_C(U5.8, GND)
	NET_C(U5.9, I_VM15)

	TL081_DIP(U6)			// Op. Amp.
	NET_C(U6.4, I_VM15)
	NET_C(U6.7, I_V15)

//	TTL_7915_DIP(U7)		// -15V Regulator -- not needed

	TL081_DIP(U8)			// Op. Amp.
	NET_C(U8.4, I_VM15)
	NET_C(U8.7, I_V15)

	TL081_DIP(U9)			// Op. Amp.
	NET_C(U9.4, I_VM15)
	NET_C(U9.7, I_V15)

#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	TL182_DIP(U10)			// Analog switch
	NET_C(U10.6, I_V15)
	NET_C(U10.7, I_V5)
	NET_C(U10.8, GND)
	NET_C(U10.9, I_VM15)
#endif

	//
	// Top-left until output from U1
	//

#if 0
	NET_C(I_V15, CR1.A)
	NET_C(CR1.K, CR2.A)
	NET_C(CR2.K, R1.2, Q1.B)
	NET_C(R1.1, GND)

	NET_C(I_V15, R2.2)
	NET_C(R2.1, Q1.E)
	NET_C(Q1.C, Q2.E, C1.1)
	NET_C(Q2.C, GND)
#else
	NET_C(GND, R1.1, R1.2, R2.1, R2.2, CR1.A, CR1.K, CR2.A, CR2.K)

	CLOCK(NOISE_CLOCK, 2000)
	NET_C(NOISE_CLOCK.GND, GND)
	NET_C(NOISE_CLOCK.VCC, I_V5)

	SYS_NOISE_MT_N(NOISE, 0.5)
	NET_C(NOISE.I, NOISE_CLOCK.Q)
	NET_C(NOISE.1, I_V15)
	NET_C(NOISE.2, C1.1)
#endif

	NET_C(C1.2, R3.2, U1.3)
	NET_C(R3.1, GND)
	NET_C(U1.2, R5.2, R4.1)
	NET_C(R5.1, GND)
	NET_C(R4.2, U1.6)

	//
	// Top-middle, from O1 until output from CR3
	//

	NET_C(I_OUT_1, U2.13)
	NET_C(U2.12, R6.1)
	NET_C(R6.2, R7.1, C2.2, Q3.B)
	NET_C(R7.2, I_V5)
	NET_C(C2.1, GND)
	NET_C(Q3.C, I_V5)
	NET_C(Q3.E, R11.2)
	NET_C(R11.1, CR3.A)

	//
	// Middle chunk, from C3 until output from R13
	//

	NET_C(U1.6, C3.1)
	NET_C(C3.2, R8.1, U3.2)
	NET_C(U3.3, GND)
	NET_C(R8.2, U3.6, R9.1)
	NET_C(R9.2, CR3.K, C4.1, CR4.A, R10.2)
	NET_C(R10.1, CR4.K, GND)
	NET_C(C4.2, R12.1)
	NET_C(C5.1, GND)
	NET_C(R12.2, C5.2, R13.1)

	//
	// Big middle section, from C8 until output from R15/R41/R37
	//

	NET_C(U1.6, C8.2)
	NET_C(C8.1, R24.1)
	NET_C(R24.2, U8.2, C10.1, R16.1)
	NET_C(U8.3, GND)
	NET_C(U8.6, R16.2, C10.2, R31.1)
	NET_C(I_OUT_2, U5.10)
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	NET_C(U8.6, U5.14)		// 1.
	NET_C(U5.13, R14.1)		// 2.
	NET_C(U8.6, U5.1)		// 3.
	NET_C(I_OUT_3, U5.5)
	NET_C(U5.2, R40.1)
	NET_C(R40.2, C23.2, R41.1)
	NET_C(R15.2, R41.2)
	NET_C(C23.1, GND)
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	NET_C(U8.6, R38.1)		// 1. Barrier has an extra resistor inline here
	NET_C(R38.2, U5.14)
	NET_C(U5.13, C7.1)		// 2. Barrier has an extra capacitor inline here
	NET_C(C7.2, R14.1)
//	NET_C(U8.6, R37.2)		// 3. Barrier connects noise source into summing amp -- wrong??
#endif
	NET_C(R14.2, C9.2, R15.1)
	NET_C(C9.1, GND)
	NET_C(R15.2, R13.2, R37.2)
	NET_C(I_OUT_0, U2.9)
	NET_C(U2.8, R25.1)
	NET_C(R25.2, R26.1, C17.1, Q6.B, C18.2)
	NET_C(R26.2, C17.2, I_V5)
	NET_C(C18.1, GND)
	NET_C(Q6.C, I_V5)
	NET_C(Q6.E, R27.2)
	NET_C(R27.1, CR5.A)
	NET_C(CR5.K, R33.2, CR6.A, R34.2, C21.2, C22.1)
	NET_C(R31.2, R32.2, R33.1, Q7.E)
	NET_C(R32.1, Q7.B)
	NET_C(Q7.C, CR6.K, R34.1, C21.1, GND)
	NET_C(C22.2, R35.1)
	NET_C(R35.2, U9.2, R36.1)
	NET_C(U9.3, GND)
	NET_C(U9.6, R36.2, R37.1)

	//
	// Final stage
	//

#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	NET_C(R15.2, U10.1, U10.14)
	NET_C(I_OUT_4, U10.5, U2.1)
	NET_C(U2.2, R42.1, U10.10)
	NET_C(R42.2, I_V5)
	NET_C(U10.2, R17.2, C11.1)
	NET_C(U10.13, R43.2)
	NET_C(R43.1, GND)
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	NET_C(R15.2, R17.2, C11.1)
#endif
	NET_C(R17.1, GND)
	NET_C(C11.2, R18.2, R19.2, U6.3)
	NET_C(R18.1, GND)
	NET_C(R19.1, C12.2)
	NET_C(C12.1, U6.2, R29.2)
	NET_C(U6.6, R21.1)
	NET_C(R29.1, R30.2)
	NET_C(R30.1, GND)
#if EMULATE_FINAL_AMP
	NET_C(R21.2, C13.1, C14.2, Q4.B, Q5.B)
	NET_C(C13.2, Q4.C, I_V25)
	NET_C(C14.1, Q5.C, I_VM25)
	NET_C(R29.3, R28.1)
	NET_C(Q4.E, R22.2)
	NET_C(Q5.E, R23.1)
	NET_C(R22.1, R23.2, R28.2)
	ALIAS(OUTPUT, R28.2)
#else
	ALIAS(OUTPUT, R29.3)
	NET_C(GND, C13.1, C13.2, C14.1, C14.2, R21.2, R22.1, R22.2, R23.1, R23.2, R28.2, R28.1)
#endif

	//
	// Unconnected
	//

	NET_C(GND, U2.3, U2.5, U2.11)
#if (SOUND_VARIANT == VARIANT_BARRIER)
	NET_C(GND, U5.1, U5.2, U5.5, U2.1)
#endif

NETLIST_END()


#if (SOUND_VARIANT == VARIANT_SPACEWARS)
NETLIST_START(spacewar)
#else // (SOUND_VARIANT == VARIANT_BARRIER)
NETLIST_START(barrier)
#endif

	SOLVER(Solver, 48000)

	TTL_INPUT(I_OUT_0, 0)				// active high
	TTL_INPUT(I_OUT_1, 0)				// active high
	TTL_INPUT(I_OUT_2, 0)				// active high
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	TTL_INPUT(I_OUT_3, 0)				// active high
	TTL_INPUT(I_OUT_4, 0)				// active high
#endif

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC)
#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	NET_C(GND, I_OUT_3.GND, I_OUT_4.GND)
	NET_C(I_V5, I_OUT_3.VCC, I_OUT_4.VCC)
#endif

	LOCAL_SOURCE(_TL182_DIP)
	LOCAL_SOURCE(_QBJT_2N6426)

#if (SOUND_VARIANT == VARIANT_SPACEWARS)
	LOCAL_SOURCE(SpaceWars_schematics)
	INCLUDE(SpaceWars_schematics)
#else // (SOUND_VARIANT == VARIANT_BARRIER)
	LOCAL_SOURCE(Barrier_schematics)
	INCLUDE(Barrier_schematics)
#endif

NETLIST_END()

#endif
