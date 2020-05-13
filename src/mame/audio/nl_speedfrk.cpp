// license:CC0
// copyright-holders:Aaron Giles,Couriersud

//
// Netlist for Speed Freak
//
// Derived from the schematics and parts list in the Speed Freak
// manual.
//
// Known problems/issues:
//
//    * Compiles but not yet really tested.
//
//    * Schematics have some seriously confusing issues, including
//       many unlabelled parts and pins, and some nonsensical
//       connections. I have attempted to make sense of them the
//       best I could, but needs some careful looking to be sure
//       it's realistic.
//
//    * It's super slow, with the 2MHz clock really bringing things
//       down. Will probably need some clever approaches to get it
//       in the realm of playable.
//

#include "netlist/devices/net_lib.h"


//
// The final amplifier is documented but not emulated.
//
#define EMULATE_FINAL_AMP	0


//
// Substitutes/models
//

// LM555/566 is compatible with NE555/566
#define LM555_DIP NE555_DIP

// alias LS devices to real devices
#define TTL_74LS04_DIP TTL_7404_DIP     // Hex Inverting Gates
#define TTL_74LS08_DIP TTL_7408_DIP     // Quad 2-Input AND Gates
#define TTL_74LS75_DIP TTL_7475_DIP     // 4-Bit Bistable Latches with Complementary Outputs
#define TTL_74LS86_DIP TTL_7486_DIP     // Quad 2-Input Exclusive-OR Gates
#define TTL_74LS107_DIP TTL_74107_DIP   // DUAL J-K FLIP-FLOPS WITH CLEAR
#define TTL_74LS163_DIP TTL_74163_DIP   // Synchronous 4-Bit Counters
#define TTL_74LS164_DIP TTL_74164_DIP   // 8-bit parallel-out serial shift registers
#define TTL_74LS393_DIP TTL_74393_DIP   // Dual 4-Stage Binary Counter


static NETLIST_START(SpeedFreak_schematics)

	// SPICE model taken directly from Fairchild Semiconductor datasheet
	NET_MODEL("2N3904 NPN(Is=6.734f Xti=3 Eg=1.11 Vaf=74.03 Bf=416.4 Ne=1.259 Ise=6.734 Ikf=66.78m Xtb=1.5 Br=.7371 Nc=2 Isc=0 Ikr=0 Rc=1 Cjc=3.638p Mjc=.3085 Vjc=.75 Fc=.5 Cje=4.493p Mje=.2593 Vje=.75 Tr=239.5n f=301.2p Itf=.4 Vtf=4 Xtf=2 Rb=10)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6107
	NET_MODEL("2N6107 PNP(IS=7.62308e-14 BF=6692.56 NF=0.85 VAF=10 IKF=0.032192 ISE=2.07832e-13 NE=2.41828 BR=15.6629 NR=1.5 VAR=1.44572 IKR=0.32192 ISC=4.75e-16 NC=3.9375 RB=7.19824 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.355458 XTB=0.1 XTI=2.97595 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347177 TF=6.63757e-09 XTF=1.50003 VTF=1.0001 ITF=1 CJC=1.06717e-10 VJC=0.942679 MJC=0.245405 XCJC=0.8 FC=0.533334 CJS=0 VJS=0.75 MJS=0.5 TR=1.32755e-07 PTF=0 KF=0 AF=1)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6292
	NET_MODEL("2N6292 NPN(IS=9.3092e-13 BF=2021.8 NF=0.85 VAF=63.2399 IKF=1 ISE=1.92869e-13 NE=1.97024 BR=40.0703 NR=1.5 VAR=0.89955 IKR=10 ISC=4.92338e-16 NC=3.9992 RB=6.98677 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.326141 XTB=0.1 XTI=2.86739 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347174 TF=6.73756e-09 XTF=1.49917 VTF=0.997395 ITF=0.998426 CJC=1.06717e-10 VJC=0.942694 MJC=0.245406 XCJC=0.8 FC=0.533405 CJS=0 VJS=0.75 MJS=0.5 TR=6.0671e-08 PTF=0 KF=0 AF=1)")

	// copied from IN914
	NET_MODEL("1N914B D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)")

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_VM5, -5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)
	ANALOG_INPUT(I_V25, 25)
	ANALOG_INPUT(I_VM25, -25)

//	RES(R1, 2.7)
//	RES(R2, 2.7)
//	RES(R3, 2.7)
//	RES(R4, 2.7)
//	RES(R5, 150)
//	RES(R6, 150)
//	RES(R7, RES_K(10))
//	RES(R8, RES_K(68))
//	RES(R9, RES_K(2.2))
//	RES(R10, 820)
//	RES(R11, RES_K(47))
	RES(R12, RES_K(1))
	RES(R13, RES_K(2.2))
	RES(R14, RES_K(3.9))
	RES(R15, RES_K(8.7))
	RES(R16, RES_K(22))
	RES(R17, RES_K(6.8))
	RES(R18, RES_K(10))
	RES(R19, 330)
	RES(R20, 330)
	RES(R21, RES_K(3.3))
	RES(R22, RES_K(30))
	RES(R23, RES_K(4.7))
	RES(R24, RES_K(10))
	RES(R25, RES_K(4.7))
	RES(R26, RES_K(4.7))
	RES(R27, RES_K(10))
	RES(R28, RES_K(4.7))
	RES(R29, RES_K(10))
	POT(R30, RES_K(10))
	PARAM(R30.DIAL, 0.500000)
	RES(R31, 150)
	RES(R32, RES_K(2.2))
	RES(R33, RES_K(10))
	RES(R34, RES_K(3.3))	// unknown

//	CAP(C1, CAP_U(50))
//	CAP(C2, CAP_U(50))
//	CAP(C3, CAP_U(4.7))
//	CAP(C4, CAP_U(4.7))
//	CAP(C5, CAP_U(4.7))
//	CAP(C6, CAP_U(0.002))
//	CAP(C7, CAP_U(0.002))
//	CAP(C8, CAP_U(0.01))
	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.02))
	CAP(C12, CAP_U(0.001))
	CAP(C13, CAP_U(0.001))
	CAP(C20, CAP_U(0.1))

//  DIODE(CR1, "1N914B")	// OK
//  DIODE(CR2, "1N914B")	// OK
    DIODE(CR3, "1N914B")	// OK

//	QBJT_EB(Q1, "2N6292")	// NPN
//	QBJT_EB(Q2, "2N6107")	// PNP
//	QBJT_EB(Q3, "2N3904")	// NPN -- unknown type
	QBJT_EB(Q4, "2N3904")	// NPN

	TTL_74LS04_DIP(U2)		// Hex Inverting Gates
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

	TL081_DIP(U3)			// Op. Amp.
	NET_C(U3.4, I_VM5)		// not documented on schematics
	NET_C(U3.7, I_V5)

	TTL_74LS163_DIP(U4)		// Synchronous 4-Bit Counters
	NET_C(U4.8, GND)
	NET_C(U4.16, I_V5)

	TTL_74LS107_DIP(U5)		// DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U5.7, GND)
	NET_C(U5.14, I_V5)

	TTL_74LS08_DIP(U6)		// Quad 2-Input AND Gates
	NET_C(U6.7, GND)
	NET_C(U6.14, I_V5)

	TTL_74LS163_DIP(U7)		// Synchronous 4-Bit Counters
	NET_C(U7.8, GND)
	NET_C(U7.16, I_V5)

	TTL_74LS163_DIP(U8)		// Synchronous 4-Bit Counters
	NET_C(U8.8, GND)
	NET_C(U8.16, I_V5)

	TTL_74LS163_DIP(U9)		// Synchronous 4-Bit Counters
	NET_C(U9.8, GND)
	NET_C(U9.16, I_V5)

//	TTL_7915_DIP(U8)		// -15V Regulator -- not needed
//	TTL_7815_DIP(U9)		// +15V Regulator -- not needed

	TTL_74LS04_DIP(U10)		// Hex Inverting Gates
	NET_C(U10.7, GND)
	NET_C(U10.14, I_V5)

	TTL_74LS08_DIP(U11)		// Quad 2-Input AND Gates
	NET_C(U11.7, GND)
	NET_C(U11.14, I_V5)

	TTL_74LS75_DIP(U12)		// 4-Bit Bistable Latches with Complementary Outputs
	NET_C(U12.12, GND)
	NET_C(U12.5, I_V5)

	TTL_74LS164_DIP(U13)	// 8-bit parallel-out serial shift registers
	NET_C(U13.7, GND)
	NET_C(U13.14, I_V5)

	TTL_74LS164_DIP(U14)	// 8-bit parallel-out serial shift registers
	NET_C(U14.7, GND)
	NET_C(U14.14, I_V5)

	TTL_74LS163_DIP(U15)	// Synchronous 4-Bit Counters
	NET_C(U15.8, GND)
	NET_C(U15.16, I_V5)

	TTL_74LS107_DIP(U17)	// DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U17.7, GND)
	NET_C(U17.14, I_V5)

	TTL_74LS393_DIP(U18)	// Dual 4-Stage Binary Counter
	NET_C(U18.7, GND)
	NET_C(U18.14, I_V5)

	TTL_74LS86_DIP(U19)		// Quad 2-Input XOR Gates
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V5)

	TTL_74LS164_DIP(U20)	// 8-bit parallel-out serial shift registers
	NET_C(U20.7, GND)
	NET_C(U20.14, I_V5)

	LM555_DIP(U22)			// 5-5-5 Timer

	TTL_74LS163_DIP(U23)	// Dual 4-Stage Binary Counter
	NET_C(U23.8, GND)
	NET_C(U23.16, I_V5)

	TTL_74LS164_DIP(U24)	// 8-bit parallel-out serial shift registers
	NET_C(U24.7, GND)
	NET_C(U24.14, I_V5)

	//
	// 78kHz coming from the logic PCB
	//
	CLOCK(J4_2, 78000)
	NET_C(J4_2.GND, GND)
	NET_C(J4_2.VCC, I_V5)

	//
	// Skip the clock generator and just do it directly
	//
#if 1
	CLOCK(C2MHZ, 2000000)
	NET_C(C2MHZ.GND, GND)
	NET_C(C2MHZ.VCC, I_V5)
	NET_C(GND, R19.1, R19.2, R20.1, R20.2, C12.1, C12.2, C13.1, C13.2, U10.1, U10.3, U10.5)
#else
	NET_C(R19.1, U10.1, C13.1)
	NET_C(R19.2, U10.2, C12.1)
	NET_C(C12.2, U10.3, R20.1)
	NET_C(R20.2, U10.4, C13.2, U10.5)
	ALIAS(C2MHZ, U10.6)
#endif

	NET_C(J4_2.Q, U3.3)
	NET_C(R17.1, GND)
	NET_C(R17.2, U3.2, R18.1)
	NET_C(R18.2, I_V5)
	NET_C(U3.6, R16.1)

	NET_C(R16.2, CR3.K, U5.12)
	NET_C(CR3.A, GND)
	NET_C(U5.1, I_V5)
	NET_C(U5.4, GND)
	NET_C(U5.13, U5.10, U10.9, U4.15)
	NET_C(U5.3, U5.8)
	NET_C(U5.2, U5.11)
	NET_C(U5.6, U4.9)
	NET_C(U5.9, U4.2, C2MHZ.Q)

	NET_C(U10.8, U4.7)
	NET_C(U4.3, U4.1, U4.6, U4.10, I_V5)	// .10 (CET) not drawn on schems
	NET_C(U4.4, U4.5, GND)
	NET_C(U4.14, U18.1, U6.13, U7.2)

	NET_C(U18.6, U18.13)
	NET_C(U18.2, U18.12, GND)
	NET_C(U18.8, U20.8, U24.8)
	NET_C(U18.10, U23.2)

	NET_C(I_OUT_0, U2.13, U6.4)
	NET_C(U2.12, U2.1)
	ALIAS(STEERING, U2.12)
	NET_C(I_OUT_1, U6.5)
	NET_C(U6.6, U19.13, U19.4)
	NET_C(U19.12, I_V5)
	NET_C(U19.11, U20.1)
	NET_C(U19.1, U20.12)
	NET_C(U19.2, U24.12)		// guessing here -- the schematic makes no sense otherwise
	NET_C(U20.13, U24.1, U19.5) // guessing here -- the schematic makes no sense otherwise
	NET_C(U19.6, U24.2)			// guessing here -- the schematic makes no sense otherwise
	NET_C(U19.3, U20.2)
	NET_C(U20.9, U24.9, I_V5)

	NET_C(U24.13, R22.1)
	NET_C(R22.2, R23.1, R24.2, U22.7)
	NET_C(I_V5, R23.2, U22.4, U22.8)
	NET_C(R24.1, C10.2, U22.6, U22.2)
	NET_C(GND, C10.1, U22.1, C11.1)
	NET_C(C11.2, U22.5)

	NET_C(I_OUT_4, U6.2)
	NET_C(U22.3, U6.1)
	NET_C(U6.3, R33.1)

	NET_C(I_V5, U19.9)
	NET_C(U23.15, U19.10)
	NET_C(U19.8, U23.9, U17.9)
	NET_C(U23.3, U23.4, U23.5, U23.6, GND)	// P0-P3 not defined on schematics
	NET_C(U23.1, U23.7, U23.10, I_V5)		// /SR, CEP, CET not defined on schematics
	NET_C(I_V5, U17.8, U17.11)
	NET_C(I_OUT_7, U2.3)
	NET_C(U2.4, U17.10)
	NET_C(U17.5, R25.1)
	NET_C(U17.6, U17.13)					// U17 pin is not documented

	NET_C(I_OUT_1, U6.10, R32.2, U2.6, U12.4, U12.13)
	NET_C(U6.9, I_V5)
	NET_C(U6.8, U2.5, U6.12)
	NET_C(R32.1, Q4.B)
	NET_C(Q4.E, GND)
	NET_C(Q4.C, R31.1)
	ALIAS(O_START_LIGHT, R31.2)
	NET_C(R31.2, GND)
	NET_C(U6.11, U15.2, U9.2, U8.2)
	NET_C(U15.15, U10.11, U7.10)
	NET_C(U15.9, U9.9, U8.9, U10.10)
	NET_C(U15.1, U15.7, U9.1, U9.7, U8.1, U8.7, I_V5)
	NET_C(U15.10, U9.15)
	NET_C(U9.10, U8.15)
	NET_C(U8.10, I_V5)

	NET_C(I_OUT_2, U2.9)
	NET_C(U2.8, U14.1, U14.2)
	NET_C(I_OUT_3, U2.11)
	NET_C(U2.10, U14.8, U13.8)
	NET_C(U14.3, U15.6)
	NET_C(U14.4, U15.5)
	NET_C(U14.5, U15.4)
	NET_C(U14.6, U15.3)
	NET_C(U14.10, U9.6)
	NET_C(U14.11, U9.5)
	NET_C(U14.12, U9.4)
	NET_C(U14.13, U9.3, U13.1, U13.2)
	NET_C(U14.9, U13.9, I_V5)
	NET_C(U13.3, U8.6)
	NET_C(U13.4, U8.5)
	NET_C(U13.5, U8.4)
	NET_C(U13.6, U8.3)
	NET_C(U13.10, U12.7)
	NET_C(U13.11, U12.6)
	NET_C(U13.12, U12.3)
	NET_C(U13.13, U12.2)
	NET_C(U24.13, U11.12, U11.1, U11.4, U11.9)
	NET_C(U12.9, U11.13)
	NET_C(U12.10, U11.2)
	NET_C(U12.15, U11.5)
	NET_C(U12.16, U11.10)

	NET_C(U17.12, U18.11)
	NET_C(U17.1, U17.4, I_V5)
	NET_C(U17.3, R26.1)
	NET_C(U7.1, U7.5, I_V5)
	NET_C(U7.3, U7.4, U7.6, GND)
	NET_C(U7.7, STEERING)
	NET_C(U7.15, U10.13)
	NET_C(U10.12, U7.9)
	NET_C(U7.11, R27.1)
	NET_C(U7.12, R28.1)
	NET_C(U7.13, R29.1)
	NET_C(R33.2, R25.2, R26.2, R27.2, R28.2, R29.2, R21.2)

	NET_C(C20.1, GND)
	NET_C(C20.2, R21.1, R34.2)
	NET_C(U11.11, R12.1)
	NET_C(U11.3, R13.1)
	NET_C(U11.6, R14.1)
	NET_C(U11.8, R15.1)
	NET_C(R15.2, R14.2, R13.2, R12.2, R34.1)

	//
	// Amplifier
	//

	NET_C(R21.2, R30.3)
	NET_C(R30.1, GND)
	NET_C(R30.2, C9.1)
#if EMULATE_FINAL_AMP
#else
	ALIAS(OUTPUT, C9.2)
	NET_C(C9.2, GND)
#endif

	//
	// Unconnected pins
	//

//	NET_C(GND, )

NETLIST_END()


NETLIST_START(speedfrk)

	SOLVER(Solver, 48000)

	TTL_INPUT(I_OUT_0, 1)		// active low
	TTL_INPUT(I_OUT_1, 1)		// active low
	TTL_INPUT(I_OUT_2, 1)		// active low
	TTL_INPUT(I_OUT_3, 1)		// active low
	TTL_INPUT(I_OUT_4, 1)		// active low
	TTL_INPUT(I_OUT_7, 1)		// active low

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND, I_OUT_7.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC, I_OUT_7.VCC)

	LOCAL_SOURCE(SpeedFreak_schematics)
	INCLUDE(SpeedFreak_schematics)

NETLIST_END()
