// license:CC0
// copyright-holders:Aaron Giles,Couriersud

//
// Netlist for Star Castle/War of the Worlds
//
// Derived from the schematics and parts list in the Star Castle
// manual.
//
// War of the Worlds uses effectively the same sound board, but
// with a swizzled set of inputs.
//
// Known problems/issues:
//
//    * It's very slow, due to needing to run the solver at 384kHz
//       in order to get close to accurate shot and background sounds.
//       These sounds are both derived from 566 timers, so perhaps
//       there is something deficient about the current 566
//       implementation. Schematics state that the background VCO
//       should range from 7.5kHz -> 23.3kHz, while the laser VCO
//       should be either 22kHz or 5.8kHz.
//
//    * Even at 384kHz, the highest frequency background sound is not
//       quite as high pitched as some videos I've seen. The best spot
//       to listen for background pitch is immediately after the
//       player dies.
//
//    * There are 5 different models of the CA3080 presented here.
//       Ultimately this should be moved into the core.
//

#include "netlist/devices/net_lib.h"


//
// The final amplifier is documented but not emulated.
//
#define EMULATE_FINAL_AMP	0


//
// Substitutes/models
//

// AMI S2688 is compatible with MM5837
#define AMI_S2688 MM5837_DIP

// LM555/566 is compatible with NE555/566
#define LM555_DIP NE555_DIP
#define LM566_DIP NE566_DIP

// alias LS devices to real devices
#define TTL_74LS107_DIP TTL_74107_DIP
#define TTL_74LS163_DIP TTL_74163_DIP
#define TTL_74LS164_DIP TTL_74164_DIP
#define TTL_74LS377_DIP TTL_74377_DIP
#define TTL_74LS393_DIP TTL_74393_DIP


//
// CA3080 models:
//
// 0: fast model - not 100% correct - but really fast
// 1: mailing list model - this had C/E reversed, now corrected
// 2: datasheet version -- most likely same as 1
// 3: model derived from https://github.com/xxv/gedasymbols/blob/master/www/user/john_doty/models/opamp/ca3080.mod
// 4: model derived from http://athena.ecs.csus.edu/~matthews/EEE_109/10_21/App_12.pdf
//
#define USE_CA3080_MODEL 0

// use the submodel above for CA3080
#define CA3080_DIP(name) SUBMODEL(CA3080, name)

static NETLIST_START(CA3080)
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

#if (USE_CA3080_MODEL == 0)

	// Fast model
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
	//AFUNC(F, 5, "max(A2, min(A4, 19.2 * (A3 - A2) * (A0-A1)))")
	AFUNC(F, 5, "max(0-0.5e-3, min(0.5e-3, 19.2 * (A3 - A2) * A0))")
	RES(RIABC, 1)
	NET_C(RIABC.2, F.A2)
	NET_C(RIABC.1, F.A3) // IB
	VCCS(VO, 1)
	ANALOG_INPUT(XGND, 0)
	NET_C(XGND, VO.IN, VO.ON) // FIXME: assume symmetric supply
	NET_C(F.Q, VO.IP)

#elif (USE_CA3080_MODEL == 1)

	// mailinglist version

	DIODE(D1, "CA3080_D_OTA")
	DIODE(D2, "CA3080_D_OTA")
	DIODE(D3, "CA3080_D_OTA")
	DIODE(D4, "CA3080_D_OTA")
	DIODE(D5, "CA3080_D_OTA")
	DIODE(D6, "CA3080_D_OTA")
	QBJT_EB(Q1, "CA3080_QN_OTA")
	QBJT_EB(Q2, "CA3080_QN_OTA")
	QBJT_EB(Q3, "CA3080_QN_OTA")
	QBJT_EB(Q4, "CA3080_QP_OTA")
	QBJT_EB(Q5, "CA3080_QP_OTA")
	QBJT_EB(Q6, "CA3080_QP_OTA")
	QBJT_EB(Q7, "CA3080_QP_OTA")
	QBJT_EB(Q8, "CA3080_QP_OTA")
	QBJT_EB(Q9, "CA3080_QP_OTA")
	QBJT_EB(Q10, "CA3080_QN_OTA")
	QBJT_EB(Q11, "CA3080_QN_OTA")

	ALIAS(2, Q1.B)
	ALIAS(3, Q2.B)
	ALIAS(4, Q3.E)
	ALIAS(5, Q3.B)
	ALIAS(6, Q8.C)
	ALIAS(7, Q4.E)
	NET_C(Q10.E, Q11.B, D6.A)
	NET_C(Q8.C, Q9.C, Q10.C)
	NET_C(Q7.B, Q9.E, D5.K)
	NET_C(Q5.E, Q6.B, D2.K)
	NET_C(Q1.C, Q4.C, Q5.B, D2.A)
	NET_C(Q2.C, Q7.C, Q8.B, D4.A)
	NET_C(Q1.E, Q2.E, Q3.C)
	NET_C(Q3.B, D1.A)
	NET_C(Q5.C, Q6.C, Q10.B, Q11.C)
	NET_C(Q3.E, Q11.E, D1.K, D6.K)
	NET_C(Q4.E, Q7.E, D3.A, D5.A)
	NET_C(Q8.E, Q9.B, D4.K)
	NET_C(Q4.B, Q6.E, D3.K)

#elif (USE_CA3080_MODEL == 2)

	// datasheet version
	DIODE(D1, "CA3080_D_OTA")
	DIODE(D2, "CA3080_D_OTA")
	DIODE(D3, "CA3080_D_OTA")
	DIODE(D4, "CA3080_D_OTA")
	DIODE(D5, "CA3080_D_OTA")
	DIODE(D6, "CA3080_D_OTA")
	QBJT_EB(Q1, "CA3080_QN_OTA")
	QBJT_EB(Q2, "CA3080_QN_OTA")
	QBJT_EB(Q3, "CA3080_QN_OTA")
	QBJT_EB(Q4, "CA3080_QP_OTA")
	QBJT_EB(Q5, "CA3080_QP_OTA")
	QBJT_EB(Q6, "CA3080_QP_OTA")
	QBJT_EB(Q7, "CA3080_QP_OTA")
	QBJT_EB(Q8, "CA3080_QP_OTA")
	QBJT_EB(Q9, "CA3080_QP_OTA")
	QBJT_EB(Q10, "CA3080_QN_OTA")
	QBJT_EB(Q11, "CA3080_QN_OTA")

	// datasheet version
	ALIAS(2, Q1.B)
	ALIAS(3, Q2.B)
	ALIAS(4, Q3.E)
	ALIAS(5, Q3.B)
	ALIAS(6, Q8.C)
	ALIAS(7, Q4.E)
	NET_C(Q1.C, Q4.C, D2.A, Q5.B)
	NET_C(Q1.E, Q2.E, Q3.C)
	NET_C(Q2.C, Q7.C, D4.A, Q8.B)
	NET_C(Q3.B, D1.A)
	NET_C(Q3.E, D1.K, Q11.E, D6.K)
	NET_C(Q4.B, D3.K, Q6.E)
	NET_C(Q4.E, D3.A, Q7.E, D5.A)
	NET_C(Q5.C, Q6.C, Q11.C, Q10.B)
	NET_C(Q5.E, D2.K, Q6.B)
	NET_C(Q7.B, D5.K, Q9.E)
	NET_C(Q8.C, Q9.C, Q10.C)
	NET_C(Q8.E, D4.K, Q9.B)
	NET_C(Q10.E, Q11.B, D6.A)

#elif (USE_CA3080_MODEL == 3)

	// model derived from https://github.com/xxv/gedasymbols/blob/master/www/user/john_doty/models/opamp/ca3080.mod
	DIODE(D1, "CA3080_D_OTA")
	DIODE(D2, "CA3080_D_OTA")
	QBJT_EB(Q1, "CA3080_QN_OTA")
	QBJT_EB(Q2, "CA3080_QP_OTA")
	QBJT_EB(Q3, "CA3080_QP_OTA")
	QBJT_EB(Q4, "CA3080_QN_OTA")
	QBJT_EB(Q5, "CA3080_QN_OTA")
	QBJT_EB(Q6, "CA3080_QP_OTA")
	QBJT_EB(Q7, "CA3080_QP_OTA")
	QBJT_EB(Q8, "CA3080_QP_OTA")
	QBJT_EB(Q9, "CA3080_QP_OTA")
	QBJT_EB(Q10, "CA3080_QN_OTA")
	QBJT_EB(Q11, "CA3080_QN_OTA")
	QBJT_EB(Q12, "CA3080_QN_OTA")
	QBJT_EB(Q13, "CA3080_QN_OTA")
	QBJT_EB(Q14, "CA3080_QP_OTA")
	QBJT_EB(Q15, "CA3080_QP_OTA")

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

#elif (USE_CA3080_MODEL == 4)

	// model derived from http://athena.ecs.csus.edu/~matthews/EEE_109/10_21/App_12.pdf
	DIODE(D1, "CA3080_D_OTA")
	DIODE(D3, "CA3080_D_OTA")
	DIODE(D5, "CA3080_D_OTA")
	DIODE(D6, "CA3080_D_OTA")
	QBJT_EB(Q1, "CA3080_QN_OTA")
	QBJT_EB(Q2, "CA3080_QN_OTA")
	QBJT_EB(Q3, "CA3080_QN_OTA")
	QBJT_EB(Q4, "CA3080_QP_OTA")
	QBJT_EB(Q6, "CA3080_QP_OTA")
	QBJT_EB(Q7, "CA3080_QP_OTA")
	QBJT_EB(Q9, "CA3080_QP_OTA")
	QBJT_EB(Q10, "CA3080_QN_OTA")
	QBJT_EB(Q11, "CA3080_QN_OTA")

	ALIAS(2, Q1.B)
	ALIAS(3, Q2.B)
	ALIAS(4, Q3.E)
	ALIAS(5, Q3.B)
	ALIAS(6, Q9.C)
	ALIAS(7, Q4.E)
	NET_C(Q1.C, Q4.C, Q6.B)
	NET_C(Q1.E, Q2.E, Q3.C)
	NET_C(Q2.C, Q7.C, Q9.B)
	NET_C(Q3.B, D1.A)
	NET_C(Q3.E, Q11.E, D1.K, D6.K)
	NET_C(Q4.B, Q6.E, D3.K)
	NET_C(Q4.E, D3.A, Q7.E, D5.A)
	NET_C(Q6.C, Q10.B, Q11.C)
	NET_C(Q7.B, D5.K, Q9.E)
	NET_C(Q9.C, Q10.C)
	NET_C(Q10.E, Q11.B, D6.A)

#endif
NETLIST_END()


static NETLIST_START(StarCastle_schematics)

	// Used by some CA3080 models above
	NET_MODEL("CA3080_QN_OTA NPN(IS=21.48f XTI=3 EG=1.11 VAF=80 BF=550 ISE=50f NE=1.5 IKF=10m XTB=1.5 BR=.1 ISC=10f NC=2 IKR=3m RC=10 CJC=800f MJC=.3333 VJC=.75 FC=.5 CJE=1.3p MJE=.3333 VJE=.75 TR=30n TF=400P ITF=30m XTF=1 VTF=10 CJS=5.8P MJS=.3333 VJS=.75)")
	NET_MODEL("CA3080_QP_OTA PNP(IS=50f XTI=3 EG=1.11 VAF=80 BF=100 ISE=130f NE=1.5 IKF=1m XTB=1.5 BR=1 ISC=0 NC=2 IKR=0 RC=0 CJC=4p MJC=.3333 VJC=.75 FC=.5 CJE=1.4p MJE=.3333 VJE=.75 TR=500n TF=23n ITF=.1 XTF=1 VTF=10 CJS=5.5P MJS=.3333 VJS=.75)")
	NET_MODEL("CA3080_D_OTA D(IS=2p RS=5 BV=40 CJO=3p TT=6n)")

	// SPICE model taken directly from Fairchild Semiconductor datasheet
	NET_MODEL("2N3904 NPN(Is=6.734f Xti=3 Eg=1.11 Vaf=74.03 Bf=416.4 Ne=1.259 Ise=6.734 Ikf=66.78m Xtb=1.5 Br=.7371 Nc=2 Isc=0 Ikr=0 Rc=1 Cjc=3.638p Mjc=.3085 Vjc=.75 Fc=.5 Cje=4.493p Mje=.2593 Vje=.75 Tr=239.5n f=301.2p Itf=.4 Vtf=4 Xtf=2 Rb=10)")

	// SPICE model taken directly from Fairchild Semiconductor datasheet
	NET_MODEL("2N3906 PNP(Is=1.41f Xti=3 Eg=1.11 Vaf=18.7 Bf=180.7 Ne=1.5 Ise=0 Ikf=80m Xtb=1.5 Br=4.977 Nc=2 Isc=0 Ikr=0 Rc=2.5 Cjc=9.728p Mjc=.5776 Vjc=.75 Fc=.5 Cje=8.063p Mje=.3677 Vje=.75 Tr=33.42n Tf=179.3p Itf=.4 Vtf=4 Xtf=6 Rb=10)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6107
	NET_MODEL("2N6107 PNP(IS=7.62308e-14 BF=6692.56 NF=0.85 VAF=10 IKF=0.032192 ISE=2.07832e-13 NE=2.41828 BR=15.6629 NR=1.5 VAR=1.44572 IKR=0.32192 ISC=4.75e-16 NC=3.9375 RB=7.19824 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.355458 XTB=0.1 XTI=2.97595 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347177 TF=6.63757e-09 XTF=1.50003 VTF=1.0001 ITF=1 CJC=1.06717e-10 VJC=0.942679 MJC=0.245405 XCJC=0.8 FC=0.533334 CJS=0 VJS=0.75 MJS=0.5 TR=1.32755e-07 PTF=0 KF=0 AF=1)")

	// SPICE model taken from https://www.onsemi.com/support/design-resources/models?rpn=2N6292
	NET_MODEL("2N6292 NPN(IS=9.3092e-13 BF=2021.8 NF=0.85 VAF=63.2399 IKF=1 ISE=1.92869e-13 NE=1.97024 BR=40.0703 NR=1.5 VAR=0.89955 IKR=10 ISC=4.92338e-16 NC=3.9992 RB=6.98677 IRB=0.1 RBM=0.1 RE=0.0001 RC=0.326141 XTB=0.1 XTI=2.86739 EG=1.206 CJE=1.84157e-10 VJE=0.99 MJE=0.347174 TF=6.73756e-09 XTF=1.49917 VTF=0.997395 ITF=0.998426 CJC=1.06717e-10 VJC=0.942694 MJC=0.245406 XCJC=0.8 FC=0.533405 CJS=0 VJS=0.75 MJS=0.5 TR=6.0671e-08 PTF=0 KF=0 AF=1)")

	// models copied from https://www.diodes.com/assets/Spice-Models/Discrete-Prodcut-Groups/Zener-Diodes.txt
	NET_MODEL("1N5236B D(BV=7.5 IS=27.5p RS=33.8 N=1.10 CJO=58.2p VJ=0.750 M=0.330 TT=50.1n)")
	NET_MODEL("1N5240B D(BV=10 IS=14.4p RS=32.0 N=1.10 CJO=24.1p VJ=0.750 M=0.330 TT=50.1n)")

	// copied from IN914
	NET_MODEL("1N914B D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)")

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)
	ANALOG_INPUT(I_V25, 25)
	ANALOG_INPUT(I_VM25, -25)

	RES(R1, RES_K(1))
    RES(R2, 160)
    RES(R3, RES_K(1))
    RES(R4, RES_K(1))
    RES(R5, RES_K(2))
    RES(R6, RES_K(2))
    RES(R7, RES_K(4.7))
    RES(R8, RES_K(3.3))
    RES(R9, 820)
    RES(R10, RES_M(3.3))
    RES(R11, RES_M(3.3))
    RES(R12, RES_M(5.1))
    RES(R13, RES_M(1.6))
    RES(R14, RES_K(2))
    RES(R15, RES_K(18))
    RES(R16, RES_K(10))
    RES(R17, RES_K(10))
    RES(R18, RES_K(91))
    RES(R19, RES_K(10))
    RES(R20, RES_K(1))
    RES(R21, RES_K(2))
    RES(R22, RES_K(1))
    RES(R24, RES_K(200))
    RES(R25, RES_K(30))
    RES(R26, RES_K(200))
    RES(R27, RES_K(51))
	RES(R28, RES_M(1))
	RES(R29, 430)
	RES(R30, 560)
	RES(R31, RES_K(3.3))
	RES(R32, RES_K(2))
	RES(R33, RES_K(130))
	RES(R34, RES_K(4.7))
	RES(R35, RES_K(2.7))
	RES(R36, RES_K(1))
	RES(R37, RES_K(39))
	RES(R38, RES_K(12))
	RES(R39, RES_K(51))
	RES(R40, RES_K(2.4))
	RES(R41, RES_K(270))
	RES(R42, RES_M(1))
	RES(R43, RES_K(4.3))
	RES(R44, RES_K(10))
	RES(R45, RES_K(1))
	RES(R46, RES_K(2))
	RES(R47, RES_K(82))
	RES(R48, RES_K(39))
	RES(R49, RES_K(20))
	RES(R50, RES_K(1))
	RES(R51, RES_K(12))
    RES(R52, RES_K(4.7))
	RES(R53, RES_K(1))
	RES(R54, RES_K(39))
	RES(R55, RES_K(12))
	RES(R56, RES_K(1))
	RES(R57, RES_K(100))
    RES(R58, RES_K(18))
	RES(R59, RES_K(15))
	RES(R60, RES_K(7.5))
	RES(R61, 430)
	RES(R62, 430)
    RES(R63, RES_K(4.7))
	RES(R64, RES_K(1))
	RES(R65, RES_K(39))
	RES(R66, RES_K(12))
	RES(R67, RES_K(1))
	RES(R68, RES_K(100))
	RES(R69, RES_K(6.8))
    RES(R70, RES_K(18))
	RES(R71, RES_K(47))
	RES(R72, 390)
	RES(R73, 390)
    RES(R74, RES_K(4.7))
	RES(R75, RES_K(2.7))
    RES(R76, RES_K(4.7))
	RES(R77, RES_K(39))
	RES(R78, RES_K(12))
	RES(R79, RES_K(1))
    RES(R80, RES_K(200))
    RES(R81, RES_K(300))
    RES(R82, RES_K(240))
	RES(R83, 200)
	RES(R84, 200)
    RES(R85, RES_K(4.7))
	RES(R86, RES_K(2.7))
    RES(R87, RES_K(4.7))
	RES(R88, RES_K(1))
	RES(R89, RES_K(1.8))
    RES(R90, RES_K(3.9))
	RES(R91, RES_K(39))
	RES(R92, RES_K(12))
	RES(R93, 620)
    RES(R94, RES_K(360))
	RES(R95, RES_K(27))
    RES(R96, RES_K(33))
	RES(R97, 47)
	RES(R98, 47)
    RES(R99, RES_K(4.7))
	RES(R100, RES_K(2.7))
    RES(R101, RES_K(4.7))
	RES(R102, RES_K(39))
	RES(R103, RES_K(12))
	RES(R104, RES_K(1))
    RES(R105, RES_K(36))
    RES(R106, RES_K(36))
	RES(R107, RES_K(8.2))
	RES(R108, RES_K(47))
	RES(R109, RES_K(22))
	RES(R110, RES_K(1))
	RES(R111, RES_K(1))
	RES(R112, RES_K(10))
	RES(R113, RES_K(160))
	RES(R114, RES_K(39))
	RES(R115, RES_K(47))
	RES(R116, RES_K(3.9))
	RES(R117, RES_K(5.1))
	RES(R118, RES_K(820))
	RES(R119, RES_K(100))
    RES(R120, RES_K(390))
	RES(R121, RES_K(15))
	RES(R122, 150)
	RES(R123, RES_K(22))
	RES(R124, 150)
	RES(R125, RES_K(8.2))
	RES(R126, RES_K(20))
    RES(R127, RES_K(30))
	POT(R128, RES_K(10))
	PARAM(R128.DIAL, 0.500000)

//  CAP(C2, CAP_U(25))		// electrolytic
//  CAP(C4, CAP_U(25))		// electrolytic
//  CAP(C5, CAP_U(25))		// electrolytic
//  CAP(C7, CAP_U(25))		// electrolytic
//  CAP(C9, CAP_U(25))		// electrolytic
    CAP(C11, CAP_U(0.68))	// film
    CAP(C12, CAP_U(0.001))	// disk
    CAP(C13, CAP_U(0.0022))	// film
    CAP(C14, CAP_U(0.1))	// film
    CAP(C15, CAP_U(0.1))	// film
	CAP(C16, CAP_U(0.1))	// disk*
	CAP(C17, CAP_U(100))	// electrolytic
	CAP(C18, CAP_U(0.1))	// film
	CAP(C19, CAP_U(0.1))	// disk*
	CAP(C20, CAP_U(0.1))	// film
	CAP(C21, CAP_U(0.01))	// disk
	CAP(C22, CAP_U(0.68))	// film
	CAP(C23, CAP_U(0.001))	// disk
	CAP(C24, CAP_U(0.0047))	// film
	CAP(C25, CAP_U(0.1))	// film
	CAP(C26, CAP_U(0.1))	// film
	CAP(C27, CAP_U(2.2))	// electrolytic
    CAP(C28, CAP_U(0.22))	// film
    CAP(C29, CAP_U(0.1))	// film
	CAP(C30, CAP_U(4.7))	// electrolytic
    CAP(C31, CAP_U(0.1))	// film
	CAP(C32, CAP_U(0.01))	// film
	CAP(C33, CAP_U(0.68))	// film
	CAP(C34, CAP_U(3.3))	// electrolytic
    CAP(C35, CAP_U(0.22))	// film
    CAP(C36, CAP_U(0.33))	// film
	CAP(C37, CAP_U(0.47))	// film
	CAP(C38, CAP_U(0.01))	// disk
	CAP(C39, CAP_U(0.68))	// film
	CAP(C40, CAP_U(0.1))	// film
	CAP(C41, CAP_U(0.01))	// disk
	CAP(C42, CAP_U(0.1))	// film
	CAP(C43, CAP_U(0.68))	// film
	CAP(C44, CAP_P(470))	// disk
	CAP(C45, CAP_P(470))	// disk
	CAP(C46, CAP_P(470))	// disk
	CAP(C47, CAP_U(0.005))	// disk
    CAP(C48, CAP_U(0.33))	// film

//	DIODE(D1, "1N4003")		// not needed
//	DIODE(D2, "1N4003")		// not needed
//	DIODE(D3, "1N4003")		// not needed
//	DIODE(D4, "1N4003")		// not needed
    ZDIODE(D5, "1N5240B")	// OK
    ZDIODE(D6, "1N5236B")	// OK
    DIODE(D7, "1N914B")		// OK
    DIODE(D8, "1N914B")		// OK
    DIODE(D9, "1N914B")		// OK
    DIODE(D10, "1N914B")	// OK

	QBJT_EB(Q1, "2N3904")	// OK: NPN
	QBJT_EB(Q2, "2N3904")	// OK: NPN
	QBJT_EB(Q3, "2N3906")	// OK: PNP
	QBJT_EB(Q4, "2N3904")	// OK: NPN
	QBJT_EB(Q5, "2N3904")	// OK: NPN
	QBJT_EB(Q6, "2N3906")	// OK: PNP
	QBJT_EB(Q7, "2N3906")	// OK: PNP
	QBJT_EB(Q8, "2N3906")	// OK: PNP
	QBJT_EB(Q9, "2N3906")	// OK: PNP
	QBJT_EB(Q10, "2N3906")	// OK: PNP
	QBJT_EB(Q11, "2N3906")	// OK: PNP
	QBJT_EB(Q12, "2N3906")	// OK: PNP
	QBJT_EB(Q13, "2N3906")	// OK: PNP
	QBJT_EB(Q14, "2N3906")	// OK: PNP
	QBJT_EB(Q15, "2N3906")	// OK: PNP
	QBJT_EB(Q16, "2N3906")	// OK: PNP
	QBJT_EB(Q17, "2N6107")	// OK: PNP
	QBJT_EB(Q18, "2N6292")	// OK: NPN

//	TTL_7414_DIP(IC1)		// Schmidt Trigger -- not needed
//	TTL_74LS164_DIP(IC2)	// 8-bit Shift Reg. -- not needed
//	TTL_74LS377_DIP(IC3)	// Octal D Flip Flop -- not needed
//	TTL_7815_DIP(IC4)		// +15V Regulator -- not needed
//	TTL_7915_DIP(IC5)		// -15V Regulator -- not needed
	TTL_7406_DIP(IC6)		// OK? Hex Inverter -- currently using a clone of 7416, no open collector behavior
	TL081_DIP(IC7)			// OK: Op. Amp.
	TL081_DIP(IC8)			// OK: Op. Amp.
	LM566_DIP(IC9)			// 566 VCO
	TTL_74LS163_DIP(IC10)	// OK: Binary Counter (schems say can sub a 74161)
	TTL_74LS163_DIP(IC11)	// OK: Binary Counter (schems say can sub a 74161)
	TTL_74LS393_DIP(IC12)	// Dual 4 Bit B.C.
	TTL_74LS393_DIP(IC13)	// Dual 4 Bit B.C.
	AMI_S2688(IC14)			// OK: Noise generator
	TL081_DIP(IC15)			// OK: Op. Amp.
	LM555_DIP(IC16)			// OK: Timer
	LM566_DIP(IC17)			// 566 VCO
	CA3080_DIP(IC18)		// Trnscndt. Op. Amp. -- needed
	CA3080_DIP(IC19)		// Trnscndt. Op. Amp. -- needed
	CA3080_DIP(IC20)		// Trnscndt. Op. Amp. -- needed
	CA3080_DIP(IC21)		// Trnscndt. Op. Amp. -- needed
	CA3080_DIP(IC22)		// Trnscndt. Op. Amp. -- needed
	LM555_DIP(IC23)			// OK: Timer
	LM555_DIP(IC24)			// OK: Timer
	TL081_DIP(IC25)			// OK: Op. Amp.
	TL081_DIP(IC26)			// OK: Op. Amp.
	TL081_DIP(IC27)			// OK: Op. Amp.
	TTL_74LS107_DIP(IC28)	// OK: Dual J-K Flip Flop

	//
	// Sheet 1, BACKGROUND (top portion)
	//

	NET_C(GND, D5.A, D6.K, D7.A, D8.A, R13.1, R16.1, R17.1, IC6.7, IC7.3, IC9.8)
	NET_C(I_V5, R20.1, IC6.14)
	NET_C(I_V15, R2.1, IC7.7, IC8.7)
	NET_C(I_VM15, C13.2, R12.1, R18.1, R21.1, IC7.4, IC8.4, IC9.1)
	NET_C(R2.2, D5.K, R3.1, R5.1, R7.1)
	NET_C(I_SHIFTREG_2, IC6.9)
	NET_C(IC6.8, R3.2, R4.1)
	NET_C(I_SHIFTREG_1, IC6.11)
	NET_C(IC6.10, R5.2, R6.1)
	NET_C(I_SHIFTREG_0, IC6.13)
	NET_C(IC6.12, R7.2, R8.1)
	NET_C(R4.2, R6.2, R8.2, IC7.2, R9.1)
	NET_C(IC7.6, R9.2, R10.1)
	NET_C(R10.2, IC8.2, R11.1, C11.1)
	NET_C(R12.2, R13.2, IC8.3)
	NET_C(IC8.6, C11.2, R11.2, R14.1)
	NET_C(R14.2, D6.A, R15.1)
	NET_C(R17.2, R15.2, R18.2, IC9.5, C12.2)
	NET_C(C13.1, IC9.7)
	NET_C(C12.1, IC9.6, R16.2)
	NET_C(IC9.3, C14.1)
	NET_C(C14.2, D7.K, R19.1)
	NET_C(R19.2, Q1.B)
	NET_C(Q1.E, R21.2, D8.K)
	NET_C(Q1.C, R20.2)
	ALIAS(CLK, R20.2)

	//
	// Sheet 1, BACKGROUND (bottom portion)
	//

	NET_C(I_V5, R1.1)
	ALIAS(HI, R1.2)
	NET_C(GND, R27.1, IC10.4, IC10.5, IC10.6, IC10.8, IC11.3, IC11.4, IC11.8, IC12.7, IC12.12, IC13.7, IC13.12, IC28.4, IC28.7)
	NET_C(I_V5, R22.1, IC10.16, IC11.16, IC12.14, IC13.14, IC28.14)
	NET_C(CLK, IC10.2, IC11.2, IC13.1, IC28.12)
	NET_C(I_SHIFTREG_3, IC6.1, IC13.2)
	NET_C(IC6.2, R22.2, IC10.1, IC11.1)
	NET_C(HI, IC10.3, IC10.7, IC10.10, IC11.5, IC11.6, IC11.7)
	NET_C(IC10.15, IC11.10)
	NET_C(IC11.15, IC28.1, IC28.13)
	NET_C(IC28.2, IC11.9, IC10.9, IC12.13)
	NET_C(IC12.11, R24.1)
	NET_C(R24.2, C15.1, R26.2, R27.2)
	NET_C(IC13.6, IC13.13)
	NET_C(IC13.9, R26.1)
	NET_C(C15.2, R25.1)
	ALIAS(SJ, R25.2)

	//
	// Sheet 2, NOISE GENERATOR
	//

	NET_C(GND, C16.2, C17.2, R28.1, IC14.1, IC14.2)
	NET_C(I_V15, C16.1, C17.1, IC14.4, IC15.7)
	NET_C(I_VM15, IC15.4)
	NET_C(IC14.3, C18.2)
	NET_C(C18.1, R28.2, IC15.3)
	NET_C(IC15.6, IC15.2)
	ALIAS(NOISE, IC15.6)

	//
	// Sheet 2, +2.2V
	//

	NET_C(GND, C19.2, R30.1, R31.1)
	NET_C(I_V5, R29.1, Q2.C)
	NET_C(R30.2, R29.2, Q2.B)
	NET_C(R31.2, Q2.E, C19.1)
	ALIAS(V2_2, Q2.E)

	//
	// Sheet 2, SQUARE WAVE
	//

	NET_C(GND, C20.2, C21.2, IC16.1)
	NET_C(I_V5, R32.1, IC16.4, IC16.8)
	NET_C(R32.2, R33.1, IC16.7)
	NET_C(R33.2, IC16.6, IC16.2, C20.1)
	NET_C(C21.1, IC16.5)
	ALIAS(SQUAREWAVE, IC16.3)

	//
	// Sheet 2, LASER VCO
	//

	NET_C(GND, C22.2, C24.2, D9.A, D10.A, Q4.E, R38.1, R42.1, R50.1, IC17.1)
	NET_C(I_V5, R34.1, R36.1, R45.1, Q3.E)
	NET_C(I_V15, R39.1, R43.1, IC17.8)
	NET_C(I_VM15, R37.1, R46.1)
	NET_C(I_OUT_3, IC6.5, IC12.2)
	NET_C(IC6.6, R36.2, R35.1)
	NET_C(R35.2, Q3.B, R34.2)
	NET_C(Q3.C, R37.2, R38.2, R40.1)
	NET_C(R40.2, Q4.B)
	NET_C(R39.2, R41.1, C22.1, R42.2, C23.2, IC17.5)
	NET_C(Q4.C, R41.2)
	NET_C(C23.1, IC17.6, R43.2)
	NET_C(IC17.3, C25.1)
	NET_C(C24.1, IC17.7)
	NET_C(C25.2, D9.K, R44.1)
	NET_C(R44.2, Q5.B)
	NET_C(Q5.E, D10.K, R46.2)
	NET_C(Q5.C, R45.2, IC12.1)
	NET_C(IC12.3, R47.1)
	NET_C(IC12.4, R48.1)
	NET_C(IC12.6, R49.1)
	NET_C(R47.2, R48.2, R49.2, R50.2, R51.1)
	NET_C(R51.2, C26.1)
	NET_C(C26.2, SJ)

	//
	// Sheet 2, SOFT EXPLOSION
	//

	NET_C(GND, C28.2, C29.2, R55.1, R56.1, R61.2, R62.1)
	NET_C(I_V5, R52.1)
	NET_C(I_V15, IC18.7)
	NET_C(I_VM15, C27.2, R54.1, IC18.4)
	NET_C(I_OUT_2, R52.2, R53.1)
	NET_C(R53.2, Q6.B)
	NET_C(Q6.E, V2_2)
	NET_C(Q6.C, R54.2, R55.2, Q7.E)
	NET_C(R56.2, Q7.B)
	NET_C(Q7.C, C27.1, R57.1)
	NET_C(R57.2, IC18.5)
	NET_C(NOISE, R58.1)
	NET_C(R58.2, C28.1, R59.1)
	NET_C(R59.2, C29.1, R60.1)
	NET_C(R60.2, R61.1, IC18.2)
	NET_C(R62.2, IC18.3)

	//
	// Sheet 2, LOUD EXPLOSION
	//

	NET_C(GND, C31.2, C32.2, R66.1, R67.1, R72.2, R73.1)
	NET_C(I_V5, R63.1)
	NET_C(I_V15, IC19.7)
	NET_C(I_VM15, C30.2, R65.1, IC19.4)
	NET_C(I_OUT_1, R63.2, R64.1)
	NET_C(R64.2, Q8.B)
	NET_C(Q8.E, V2_2)
	NET_C(Q8.C, R65.2, R66.2, Q9.E)
	NET_C(R67.2, Q9.B)
	NET_C(Q9.C, C30.1, R68.1)
	NET_C(R68.2, IC19.5)
	NET_C(NOISE, R69.1)
	NET_C(R69.2, C31.1, R70.1)
	NET_C(R70.2, C32.1, R71.1)
	NET_C(R71.2, R72.1, IC19.2)
	NET_C(R73.2, IC19.3)

	//
	// Sheet 2, FIREBALL
	//

	NET_C(GND, R78.1, R79.1, R83.1, R84.1)
	NET_C(I_V5, Q10.E, R74.1, R76.1)
	NET_C(I_V15, IC20.7)
	NET_C(I_VM15, C33.2, R77.1, IC20.4)
	NET_C(I_SHIFTREG_7, R74.2, R75.1)
	NET_C(R75.2, R76.2, Q10.B)
	NET_C(Q10.C, R77.2, R78.2, Q11.E)
	NET_C(R79.2, Q11.B)
	NET_C(Q11.C, C33.1, R80.1)
	NET_C(R80.2, IC20.5)
	NET_C(NOISE, R81.1)
	NET_C(SQUAREWAVE, R82.1)
	NET_C(R81.2, R82.2, R83.2, IC20.2)
	NET_C(R84.2, IC20.3)

	//
	// Sheet 2, SHIELD
	//

	NET_C(GND, R92.1, R93.1, R97.1, R98.1)
	NET_C(I_V5, R85.1, R87.1, R88.1, Q12.E)
	NET_C(I_V15, IC21.7)
	NET_C(I_VM15, R91.1, IC21.4)
	NET_C(I_SHIFTREG_6, R85.2, R86.1)
	NET_C(R86.2, R87.2, Q12.B)
	NET_C(Q12.C, R90.1, Q13.E)
	NET_C(SQUAREWAVE, R88.2, R89.1)
	NET_C(R89.2, R90.2, Q13.B)
	NET_C(Q13.C, R91.2, R92.2, Q14.E)
	NET_C(R93.2, Q14.B)
	NET_C(Q14.C, R94.1)
	NET_C(R94.2, IC21.5)
	NET_C(SQUAREWAVE, R96.1)
	NET_C(NOISE, R95.1)
	NET_C(R96.2, R95.2, R97.2, IC21.2)
	NET_C(R98.2, IC21.3)

	//
	// Sheet 2, THRUST
	//

	NET_C(GND, R103.1, R104.1, C35.2, C36.2, R110.1, R111.1)
	NET_C(I_V5, R99.1, R101.1, Q15.E)
	NET_C(I_V15, IC22.7)
	NET_C(I_VM15, C34.2, R102.1, IC22.4)
	NET_C(I_SHIFTREG_4, R99.2, R100.1)
	NET_C(R100.2, R101.2, Q15.B)
	NET_C(Q15.C, R102.2, R103.2, Q16.E)
	NET_C(R104.2, Q16.B)
	NET_C(Q16.C, R105.1)
	NET_C(R105.2, C34.1, R106.1)
	NET_C(R106.2, IC22.5)
	NET_C(NOISE, R107.1)
	NET_C(R107.2, C35.1, R108.1)
	NET_C(R108.2, C36.1, R109.1)
	NET_C(R109.2, R110.2, IC22.2)
	NET_C(R111.2, IC22.3)

	//
	// Sheet 2, STAR SOUND
	//

	NET_C(GND, C37.1, C38.1, C39.1, C40.1, C41.1, R119.1, IC23.1, IC24.1)
	NET_C(I_V5, R112.1, R117.1, IC23.8, IC24.8)
	NET_C(I_SHIFTREG_5, IC23.4, IC24.4)
	NET_C(R112.2, IC23.7, R113.1)
	NET_C(R113.2, IC23.2, IC23.6, C37.2)
	NET_C(C38.2, IC23.5)
	NET_C(IC23.3, R114.1)
	NET_C(R114.2, C39.2, R115.1)
	NET_C(R115.2, C40.2, IC24.6, IC24.2, R116.1)
	NET_C(R116.2, IC24.7, R117.2)
	NET_C(C41.2, IC24.5)
	NET_C(IC24.3, R119.2, R118.1)
	NET_C(R118.2, C42.1)
	NET_C(C42.2, SJ)

	//
	// Sheet 2, preamp
	//

	NET_C(GND, R127.1, IC27.3, R128.1)
	NET_C(I_V15, IC26.7, IC27.7)
	NET_C(I_VM15, IC26.4, IC27.4)
	NET_C(IC18.6, IC19.6, IC20.6, IC21.6, IC22.6, R127.2, IC26.3)
	NET_C(IC26.2, IC26.6, C48.1)
	NET_C(C48.2, R125.1)
	NET_C(R125.2, SJ, IC27.2, R126.1)
	NET_C(R126.2, IC27.6, R128.3)

	//
	// Sheet 2, final amp
	//

#if EMULATE_FINAL_AMP
	NET_C(GND, C43.2, C47.2)
	NET_C(I_V15, IC25.7)
	NET_C(I_VM15, IC25.4)
	NET_C(I_V25, C45.1, Q17.C)
	NET_C(I_VM25, C46.2, Q18.C)
	NET_C(C43.1, R121.1)
	NET_C(R121.2, IC25.2, C44.1, R120.1)
	NET_C(R120.2, Q17.E, Q18.E, R123.2)				// SPEAKER
	NET_C(C44.2, IC25.6, R122.1, R123.1, R124.1)
	NET_C(C47.1, IC25.3, R128.2)
	NET_C(R122.2, C45.2, Q17.B)
	NET_C(R124.2, C46.1, Q18.B)
	ALIAS(OUTPUT, Q18.E)
#else
	NET_C(GND, C47.2)
	NET_C(C47.1, R128.2)
	NET_C(GND, IC25.4, IC25.7, C46.1, C43.2, C45.2, C45.1, R124.1, R122.1, R121.1, R124.2, R120.1, R120.2, R121.2, R123.1, C43.1, IC25.3, R123.2, C46.2, C44.2, C44.1, R122.2, IC25.2)
	ALIAS(OUTPUT, C47.1)
#endif

	//
	// Unconnected pins
	//

	NET_C(GND, IC6.3, IC28.8, IC28.9, IC28.10, IC28.11)

NETLIST_END()


NETLIST_START(starcas)

	// 192k is not high enough to make the laser and background pitches high enough
	SOLVER(Solver, 384000)
//	PARAM(Solver.ACCURACY, 1e-10)
//	PARAM(Solver.NR_LOOPS, 300)
//	PARAM(Solver.METHOD, "MAT_CR")
//	PARAM(Solver.PARALLEL, 0)
//	PARAM(Solver.DYNAMIC_TS, 0)
//	PARAM(Solver.DYNAMIC_LTE, 5e-4)
//	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 20e-6)

	TTL_INPUT(I_SHIFTREG_0, 0)	// active high
	TTL_INPUT(I_SHIFTREG_1, 0)	// active high
	TTL_INPUT(I_SHIFTREG_2, 0)	// active high
	TTL_INPUT(I_SHIFTREG_3, 1)	// active low
	TTL_INPUT(I_SHIFTREG_4, 1)	// active low
	TTL_INPUT(I_SHIFTREG_5, 0)	// active high
	TTL_INPUT(I_SHIFTREG_6, 1)	// active low
	TTL_INPUT(I_SHIFTREG_7, 1)	// active low
	TTL_INPUT(I_OUT_1, 1)		// active low
	TTL_INPUT(I_OUT_2, 1)		// active low
	TTL_INPUT(I_OUT_3, 1)		// active low

	NET_C(GND, I_SHIFTREG_0.GND, I_SHIFTREG_1.GND, I_SHIFTREG_2.GND, I_SHIFTREG_3.GND, I_SHIFTREG_4.GND, I_SHIFTREG_5.GND, I_SHIFTREG_6.GND, I_SHIFTREG_7.GND)
	NET_C(I_V5, I_SHIFTREG_0.VCC, I_SHIFTREG_1.VCC, I_SHIFTREG_2.VCC, I_SHIFTREG_3.VCC, I_SHIFTREG_4.VCC, I_SHIFTREG_5.VCC, I_SHIFTREG_6.VCC, I_SHIFTREG_7.VCC)
	NET_C(GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND)
	NET_C(I_V5, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC)

	LOCAL_SOURCE(CA3080)

	LOCAL_SOURCE(StarCastle_schematics)
	INCLUDE(StarCastle_schematics)

NETLIST_END()
