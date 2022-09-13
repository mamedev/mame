// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Space Fury
//
// Derived from the schematics in the Space Fury manual.
//
// Known problems/issues:
//
//    * Works pretty well. All sounds trigger.
//
//    * Very challenging to get running at speed. We have to use
//       the "slow" CA3080 model for the star spin sound, which
//       creates a giant net that dominates time and is sensitive
//       to cheats to reduce timesteps, etc
//

#include "netlist/devices/net_lib.h"
#include "nl_spacfury.h"


//
// Optimizations
//

#define HLE_SHOOT_VCO (1)
#define SPLIT_SHARED_OSCILLATORS (1)
#define ENABLE_SPIN_VCO_FRONTIER (1)
#define ENABLE_NOISE_FRONTIERS (1)
#define ENABLE_FRONTIERS (1)
#define UNDERCLOCK_NOISE_GEN (1 && ENABLE_NOISE_FRONTIERS)




//
// Hacks
//

#define ADD_CLIPPING_DIODES (1)



//
// Local models
//

#define D_1N914(name) DIODE(name, "1N914")
#define D_1N4002(name) DIODE(name, "1N4002")

#define Q_2N4401(name) QBJT_EB(name, "2N4401")

#define Q_2N4403(name) QBJT_EB(name, "2N4403")

// JFET transistors not supported, but this should do the trick
//#define Q_2N4093(name) MOSFET(name, "NMOS(VTO=-1.0)")
#define Q_2N4093(name) MOSFET(name, "NMOS(VTO=-1 CAPMOD=0)")

#define LM555_DIP NE555_DIP
#define LM566_DIP NE566_DIP

#define TTL_74LS00_DIP TTL_7400_DIP



//
// DIP mappings use the submodels below for CA3080
//
#define CA3080_FAST_DIP(name) SUBMODEL(_CA3080_FAST_DIP, name)
#define CA3080_SLOW_DIP(name) SUBMODEL(_CA3080_SLOW_DIP, name)
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
// Main netlist
//

NETLIST_START(spacfury)
{

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 4e-5)
	// subtle but noticeable effect where the spin "whoosh" is missing
	// some upper harmonics at 4e-5; however, this is the most demanding
	// solver in the system and it keeps jumping around as I tweak things
//  PARAM(Solver.Solver_21.DYNAMIC_MIN_TIMESTEP, 2e-5)

	// Overwrite model - the default model uses minimum datasheet
	// specifications for 5V. These are for 10V and thus closer to the
	// 12V used in this circuit.
	NET_MODEL("CD4XXX FAMILY(TYPE=CMOS IVL=0.3 IVH=0.7 OVL=0.05 OVH=0.05 ORL=384 ORH=384)")

	LOCAL_SOURCE(_CA3080_FAST_DIP)
	LOCAL_SOURCE(_CA3080_SLOW_DIP)

	TTL_INPUT(I_LO_D0, 0)
	ALIAS(I_CRAFTS_SCALE, I_LO_D0)
	TTL_INPUT(I_LO_D1, 0)
	ALIAS(I_MOVING, I_LO_D1)
	TTL_INPUT(I_LO_D2, 0)
	ALIAS(I_THRUST, I_LO_D2)
	TTL_INPUT(I_LO_D6, 0)
	ALIAS(I_STAR_SPIN, I_LO_D6)
	TTL_INPUT(I_LO_D7, 0)
	ALIAS(I_PARTIAL_WARSHIP, I_LO_D7)

	NET_C(GND, I_LO_D0.GND, I_LO_D1.GND, I_LO_D2.GND, I_LO_D6.GND, I_LO_D7.GND)
	NET_C(I_V5, I_LO_D0.VCC, I_LO_D1.VCC, I_LO_D2.VCC, I_LO_D6.VCC, I_LO_D7.VCC)

	TTL_INPUT(I_HI_D0, 0)
	ALIAS(I_CRAFTS_JOINING, I_HI_D0)
	TTL_INPUT(I_HI_D1, 0)
	ALIAS(I_SHOOT, I_HI_D1)
	TTL_INPUT(I_HI_D2, 0)
	ALIAS(I_FIREBALL, I_HI_D2)
	TTL_INPUT(I_HI_D3, 0)
	ALIAS(I_SMALL_EXPL, I_HI_D3)
	TTL_INPUT(I_HI_D4, 0)
	ALIAS(I_LARGE_EXPL, I_HI_D4)
	TTL_INPUT(I_HI_D5, 0)
	ALIAS(I_DOCKING_BANG, I_HI_D5)

	NET_C(GND, I_HI_D0.GND, I_HI_D1.GND, I_HI_D2.GND, I_HI_D3.GND, I_HI_D4.GND, I_HI_D5.GND)
	NET_C(I_V5, I_HI_D0.VCC, I_HI_D1.VCC, I_HI_D2.VCC, I_HI_D3.VCC, I_HI_D4.VCC, I_HI_D5.VCC)

	RES(R_PSG_1, 1000)
	RES(R_PSG_2, 1000)
	RES(R_PSG_3, 1000)
	NET_C(I_V5, R_PSG_1.1, R_PSG_2.1, R_PSG_3.1)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_VM12, -12)

//  RES(R1, RES_K(2.2))     -- part of final amp (not emulated)
//  RES(R2, RES_K(10))      -- part of final amp (not emulated)
//  RES(R4, RES_K(330))     -- part of final amp (not emulated)
//  RES(R5, RES_M(1))       -- part of final amp (not emulated)
	RES(R6, RES_K(680))
	RES(R7, RES_K(680))
	RES(R8, RES_K(10))
	RES(R9, RES_K(100))
	RES(R10, 680)
	RES(R11, RES_K(47))
	RES(R12, RES_K(47))
	RES(R13, RES_K(47))
	RES(R14, RES_M(1))
//  RES(R15, RES_K(100))    -- part of final amp (not emulated)
	RES(R18, RES_K(10))
	RES(R19, RES_K(470))
	RES(R20, RES_K(22))
	RES(R21, RES_K(47))
	RES(R22, RES_K(3.9))
	RES(R23, RES_K(82))
	RES(R24, RES_K(4.7))
	RES(R25, RES_K(47))
	RES(R26, 100)
	RES(R27, RES_K(100))
	RES(R28, RES_K(100))
	RES(R29, RES_K(2.2))
	RES(R30, RES_K(4.7))
	RES(R31, RES_K(100))
	RES(R32, RES_K(100))
	RES(R33, RES_M(1))
	RES(R34, RES_K(10))
	RES(R35, RES_K(680))
	RES(R36, 680)
	RES(R37, RES_K(47))
	RES(R38, RES_K(47))
	RES(R39, RES_K(2.2))
	RES(R40, RES_K(47))
	RES(R41, RES_K(47))
	RES(R42, RES_K(100))
	RES(R43, RES_K(2.2))
	RES(R44, RES_K(100))
	RES(R45, RES_M(2.2))
	RES(R46, RES_K(100))
	RES(R47, RES_K(9.1))
	RES(R48, 100)
	RES(R49, RES_K(4.7))
	RES(R50, RES_K(47))
	RES(R51, RES_K(10))
	RES(R52, RES_K(47))
	RES(R53, RES_K(47))
	RES(R54, RES_K(100))
	RES(R55, RES_K(47))
	RES(R56, RES_K(100))
	RES(R57, RES_K(47))
	RES(R58, RES_K(100))
	RES(R59, RES_K(10))
	RES(R60, 100)
	RES(R61, RES_K(9.1))
	RES(R62, RES_K(100))
	RES(R63, RES_K(47))
	RES(R64, RES_K(10))
	RES(R65, RES_K(470))
	RES(R66, RES_K(10))
	RES(R67, 100)
	RES(R68, RES_K(10))
	RES(R69, RES_K(22))
	RES(R70, RES_K(220))
	RES(R71, RES_K(2.2))
	RES(R72, RES_K(22))
	RES(R73, RES_K(330))
	RES(R74, RES_K(330))
	RES(R75, RES_K(100))
	RES(R76, RES_K(10))
	RES(R77, RES_K(10))
	RES(R78, RES_K(22))
	RES(R79, RES_K(10))
	RES(R80, RES_K(100))
	RES(R81, RES_K(82))
	RES(R82, RES_K(39))
	RES(R83, RES_K(10))
	RES(R84, RES_K(22))
	RES(R85, RES_K(47))
	RES(R86, RES_K(47))
	RES(R87, RES_K(47))
	RES(R88, RES_K(100))
	RES(R89, RES_K(100))
	RES(R90, RES_K(10))
	RES(R91, RES_M(1))
	RES(R92, RES_K(470))
	RES(R93, RES_K(470))
	RES(R94, RES_K(220))
	RES(R95, RES_K(22))
	RES(R96, RES_K(100))
	RES(R97, RES_K(100))
	RES(R98, RES_K(10))
	RES(R99, RES_K(3.9))
	RES(R100, RES_K(47))
	RES(R101, RES_K(2.2))
	RES(R102, RES_M(1))
	RES(R103, RES_K(10))
	RES(R104, RES_K(10))
	RES(R105, RES_K(100))
	RES(R106, RES_K(10))
	RES(R107, RES_K(220))
	RES(R108, RES_K(220))
	RES(R109, RES_K(10))
	RES(R110, 100)
	RES(R111, RES_K(10))
	RES(R112, RES_K(470))
	RES(R113, RES_K(100))
	RES(R114, RES_K(100))
	RES(R115, RES_K(100))
	RES(R116, RES_K(100))
	RES(R117, RES_K(100))
	RES(R118, RES_K(100))
	RES(R119, RES_M(2.2))
	RES(R120, RES_K(100))
	RES(R121, RES_K(100))
	RES(R122, RES_M(2.2))
	RES(R123, RES_K(22))
	RES(R124, RES_M(3.9))
	RES(R125, RES_K(100))
	RES(R126, RES_K(100))
	RES(R127, RES_M(2.2))
	RES(R128, RES_K(22))
	RES(R129, RES_K(22))
	RES(R130, RES_K(22))
	RES(R131, RES_K(10))
	RES(R132, RES_K(2.2))
	RES(R133, RES_K(2.2))
	RES(R135, RES_K(10))
	RES(R136, RES_K(10))
	RES(R134, RES_K(10))
	RES(R137, RES_K(2.2))
	RES(R138, RES_K(150))
	RES(R139, RES_K(150))
	RES(R140, RES_K(2.2))
	RES(R141, RES_K(220))
	RES(R142, RES_K(100))
	RES(R143, RES_K(10))
	RES(R144, RES_K(220))
	RES(R146, RES_K(220))
	RES(R147, 680)
	RES(R148, RES_K(10))
	RES(R149, RES_K(220))

//  CAP(C1, CAP_U(4.7))     -- part of final amp (not emulated)
//  CAP(C2, CAP_U(0.05))
//  CAP(C3, CAP_U(0.05))
//  CAP(C4, CAP_U(10))      -- part of final amp (not emulated)
	CAP(C5, CAP_U(0.01))
	CAP(C6, CAP_U(0.01))
	CAP(C7, CAP_U(4.7))
	CAP(C8, CAP_U(0.047))
//  CAP(C9, CAP_U(0.05))
//  CAP(C10, CAP_U(0.05))
	CAP(C11, CAP_U(0.01))
	CAP(C12, CAP_U(0.01))
	CAP(C13, CAP_U(0.01))
	CAP(C14, CAP_U(0.01))
	CAP(C15, CAP_U(0.01))
//  CAP(C16, CAP_U(0.05))
//  CAP(C17, CAP_U(0.05))
	CAP(C18, CAP_U(0.01))
	CAP(C19, CAP_U(0.047))
	CAP(C20, CAP_U(0.01))
	CAP(C21, CAP_U(4.7))
	CAP(C22, CAP_U(0.05))
	CAP(C23, CAP_U(0.05))
	CAP(C24, CAP_U(10))
//  CAP(C25, CAP_U(0.05))
	CAP(C26, CAP_U(0.0033))
//  CAP(C27, CAP_U(0.05))
	CAP(C28, CAP_U(0.1))
	CAP(C29, CAP_U(0.001))
	CAP(C30, CAP_U(0.0022))
	CAP(C31, CAP_U(0.022))
	CAP(C32, CAP_U(10))
	CAP(C33, CAP_U(0.1))
	CAP(C34, CAP_U(0.1))
	CAP(C35, CAP_U(0.0033))
	CAP(C36, CAP_U(4.7))
	CAP(C37, CAP_U(0.0022))
//  CAP(C38, CAP_U(0.05))
//  CAP(C39, CAP_U(0.05))
	CAP(C40, CAP_U(0.047))
	CAP(C41, CAP_U(0.0047))
	CAP(C42, CAP_U(0.01))
	CAP(C43, CAP_U(0.01))
	CAP(C44, CAP_U(4.7))
//  CAP(C45, CAP_U(0.05))
//  CAP(C46, CAP_U(0.05))
	CAP(C47, CAP_U(0.047))
	CAP(C48, CAP_U(4.7))
	CAP(C49, CAP_U(0.1))
	CAP(C50, CAP_U(4.7))
	CAP(C51, CAP_U(0.1))
	CAP(C52, CAP_U(0.022))
	CAP(C53, CAP_U(10))
	CAP(C54, CAP_U(0.1))
	CAP(C55, CAP_U(100))
//  CAP(C56, CAP_U(0.05))
	CAP(C57, CAP_U(4.7))
//  CAP(C58, CAP_U(0.05))
	CAP(C59, CAP_U(0.01))
	CAP(C60, CAP_U(0.01))
	CAP(C61, CAP_U(0.047))
	CAP(C62, CAP_U(0.047))
	CAP(C63, CAP_U(0.05))
	CAP(C64, CAP_U(0.05))
//  CAP(C65, CAP_U(0.05))
	CAP(C66, CAP_U(0.033))
//  CAP(C67, CAP_U(0.05))
	CAP(C68, CAP_U(0.1))
	CAP(C69, CAP_U(0.1))
	CAP(C70, CAP_U(0.1))
	CAP(C71, CAP_U(0.047))
	CAP(C72, CAP_P(100))
	CAP(C73, CAP_U(0.1))
//  CAP(C75, CAP_U(10))
//  CAP(C76, CAP_U(10))
//  CAP(C77, CAP_U(0.1))
//  CAP(C78, CAP_U(0.1))
//  CAP(C79, CAP_U(0.1))
//  CAP(C80, CAP_U(0.1))

	D_1N914(D1)
	D_1N914(D2)
	D_1N914(D3)
	D_1N914(D4)
	D_1N914(D5)
	D_1N914(D6)
	D_1N914(D7)
	D_1N914(D8)
	D_1N914(D9)
	D_1N914(D10)
	D_1N914(D11)
	D_1N914(D12)
	D_1N914(D13)
	D_1N914(D14)
	D_1N914(D15)
	D_1N914(D16)

//  Q_2N4093(Q1)        -- part of final amp (not emulated)
	Q_2N4093(Q2)
	Q_2N4093(Q3)
	Q_2N4403(Q4)
	Q_2N4403(Q5)
	Q_2N4403(Q6)
	Q_2N4093(Q7)
	Q_2N4093(Q8)
	Q_2N4403(Q9)
	Q_2N4403(Q10)
	Q_2N4401(Q11)
	Q_2N4401(Q12)
	Q_2N4403(Q13)

//  TL082_DIP(U1)           // Op. Amp. -- part of final amp (not emulated)
//  NET_C(U1.8, I_V12)
//  NET_C(U1.4, I_VM12)

	TL082_DIP(U2)           // Op. Amp.
	NET_C(U2.8, I_V12)
	NET_C(U2.4, I_VM12)

	CD4069_DIP(U3)          // Hex Inverter
	NET_C(U3.7, GND)
	NET_C(U3.14, I_V12)

	TL082_DIP(U4)           // Op. Amp.
	NET_C(U4.8, I_V12)
	NET_C(U4.4, I_VM12)

	TL082_DIP(U5)           // Op. Amp.
	NET_C(U5.8, I_V12)
	NET_C(U5.4, I_VM12)

	CD4011_DIP(U6)          // Quad 2-Input NAND Gates
	NET_C(U6.7, GND)
	NET_C(U6.14, I_V12)

	CA3080_SLOW_DIP(U7)         // Op. Amp.
	NET_C(U7.4, I_VM12)
	NET_C(U7.7, I_V12)

	TL082_DIP(U8)           // Op. Amp.
	NET_C(U8.8, I_V12)
	NET_C(U8.4, I_VM12)

	MM5837_DIP(U9)          // Noise Generator
#if (UNDERCLOCK_NOISE_GEN)
	// officially runs at 48-112kHz, but little noticeable difference
	// in exchange for a big performance boost
	PARAM(U9.FREQ, 24000)

	// the NOISE_B signal can run even lower, so use a second
	// MM5837 instance at a lower frequency to drive it; this
	// second instance doesn't really exist, it just allows us
	// a bit more performance
	MM5837_DIP(U9B)         // Noise Generator
	PARAM(U9B.FREQ, 12000)
#endif

	TL082_DIP(U10)          // Op. Amp.
	NET_C(U10.8, I_V12)
	NET_C(U10.4, I_VM12)

	CD4024_DIP(U11)         // 7-Stage Ripple Binary Counter
	NET_C(U11.7, GND)
	NET_C(U11.14, I_V12)

	CA3080_SLOW_DIP(U12)            // Op. Amp.
	NET_C(U12.4, I_VM12)
	NET_C(U12.7, I_V12)

	TL082_DIP(U13)          // Op. Amp.
	NET_C(U13.8, I_V12)
	NET_C(U13.4, I_VM12)

	TL082_DIP(U14)          // Op. Amp.
	NET_C(U14.8, I_V12)
	NET_C(U14.4, I_VM12)

	TL082_DIP(U15)          // Op. Amp.
	NET_C(U15.8, I_V12)
	NET_C(U15.4, I_VM12)

	TL082_DIP(U16)          // Op. Amp.
	NET_C(U16.8, I_V12)
	NET_C(U16.4, I_VM12)

	LM555_DIP(U17)

	TL082_DIP(U18)          // Op. Amp.
	NET_C(U18.8, I_V12)
	NET_C(U18.4, I_VM12)

	CD4069_DIP(U19)         // Hex Inverter
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V12)

	CD4011_DIP(U20)         // Quad 2-Input NAND Gates
	NET_C(U20.7, GND)
	NET_C(U20.14, I_V12)

	TL082_DIP(U21)          // Op. Amp.
	NET_C(U21.8, I_V12)
	NET_C(U21.4, I_VM12)

	CD4011_DIP(U22)         // Quad 2-Input NAND Gates
	NET_C(U22.7, GND)
	NET_C(U22.14, I_V12)

	TL082_DIP(U23)          // Op. Amp.
	NET_C(U23.8, I_V12)
	NET_C(U23.4, I_VM12)

//  TTL_74LS08_DIP(U25)     // Quad 2-Input AND Gates
//  NET_C(U25.7, GND)
//  NET_C(U25.14, I_V5)

	TTL_7407_DIP(U26)       // Hex Buffers with High Votage Open-Collector Outputs
	NET_C(U26.7, GND)
	NET_C(U26.14, I_V5)

//  TTL_74LS10_DIP(U28)     // Triple 3-Input NAND Gate
//  NET_C(U28.7, GND)
//  NET_C(U28.14, I_V5)

//  TTL_74LS14_DIP(U30)
//  NET_C(U30.7, GND)
//  NET_C(U30.14, I_V5)

	TTL_7406_DIP(U31)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U31.7, GND)
	NET_C(U31.14, I_V5)

//  TTL_74LS374_DIP(U32)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U32.10, GND)
//  NET_C(U32.20, I_V5)
//
//  TTL_74LS374_DIP(U33)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U33.10, GND)
//  NET_C(U33.20, I_V5)

//  TTL_74LS30_DIP(U34)     // 8-Input NAND Gate
//  NET_C(U34.7, GND)
//  NET_C(U34.14, I_V5)

//  TTL_74LS14_DIP(U35)
//  NET_C(U35.7, GND)
//  NET_C(U35.14, I_V5)

	//
	// Sheet 7, top (Moving)
	//

	// Pairs of CD4069 are used together with resistors and capacitors
	// to create oscillators. The formula for the frequency is
	// 1/1.39*R*C. The following are simulated here:
	//
	// #1: R35=680K, C12=0.01uF,  Freq=105.80Hz
	// #2:  R6=680K, C11=0.01uF,  Freq=105.80Hz
	// #3:  R7=680K, C13=0.01uF,  Freq=105.80Hz
	//
	// Since these are all the same and components have tolerances,
	// we use tweaked frequencies for #2 and #3

	CLOCK(O1CLK, 105.8)
	NET_C(O1CLK.GND, GND)
	NET_C(O1CLK.VCC, I_V12)
	NET_C(O1CLK.Q, R56.1)
	NET_C(GND, R35.1, R35.2, C12.1, C12.2, U3.5, U3.9)

	CLOCK(O2CLK, 105.7)
	NET_C(O2CLK.GND, GND)
	NET_C(O2CLK.VCC, I_V12)
	NET_C(O2CLK.Q, R54.1)
	NET_C(GND, R6.1, R6.2, C11.1, C11.2, U3.1, U3.3)

	CLOCK(O3CLK, 105.9)
	NET_C(O3CLK.GND, GND)
	NET_C(O3CLK.VCC, I_V12)
	NET_C(O3CLK.Q, R58.1)
	NET_C(GND, R7.1, R7.2, C13.1, C13.2, U3.13, U3.11)

	NET_C(I_MOVING, U31.13)
	NET_C(U31.12, R56.2, R54.2, R58.2, R59.1)
	NET_C(R59.2, C15.2, R36.2, C14.1)
	NET_C(C15.1, U4.2, R28.2)
	NET_C(R28.1, U4.1, C14.2, R8.1)
	NET_C(U4.3, GND, R36.1)

	NET_C(R8.2, C5.2, R10.2, C6.1)
	NET_C(C5.1, U4.6, R9.2)
	NET_C(R9.1, U4.7, C6.2)
	ALIAS(MOVING_SUM, C6.2)
	NET_C(U4.5, GND, R10.1)

	//
	// Sheet 7, 2nd from top (Star Spin/Partial Warship)
	//

#if (SPLIT_SHARED_OSCILLATORS)

	// In the schematics, the below oscillators are shared; however,
	// connecting them also brings the two nets together, so we just
	// duplicate the clocks here to help keep the very expensive net
	// below more isolated.

	CLOCK(O1CLKA, 105.8)
	NET_C(O1CLKA.GND, GND)
	NET_C(O1CLKA.VCC, I_V12)
	NET_C(O1CLKA.Q, R55.2)

	CLOCK(O2CLKB, 105.7)
	NET_C(O2CLKB.GND, GND)
	NET_C(O2CLKB.VCC, I_V12)
	NET_C(O2CLKB.Q, R53.2)

	CLOCK(O3CLKC, 105.9)
	NET_C(O3CLKC.GND, GND)
	NET_C(O3CLKC.VCC, I_V12)
	NET_C(O3CLKC.Q, R57.2)

#else

	NET_C(O1CLK.Q, R55.2)
	NET_C(O2CLK.Q, R53.2)
	NET_C(O3CLK.Q, R57.2)

#endif

	NET_C(I_STAR_SPIN, U31.3, U31.1)
	NET_C(U31.2, R148.1)
	NET_C(C32.1, I_V12, R62.1, R72.1)

	NET_C(R148.2, C32.2, R62.2, U13.5)
	NET_C(R72.2, R90.2, R91.1, D6.A)
	NET_C(R90.1, GND)
	NET_C(U13.6, U13.7, D5.A)
	NET_C(D5.K, D6.K, R94.1, R92.1)
	NET_C(R94.2, R108.2, U16.5)
	NET_C(R108.1, GND)
	NET_C(R92.2, U16.6, R107.1, C47.1)

#if (ENABLE_SPIN_VCO_FRONTIER)
	// split the envelope VCO from the rest of the circuit
	// to break up a giant net into 2 smaller ones
	NET_C(C47.2, U16.7, U16.2, TESTFUNC.A0)
	AFUNC(TESTFUNC, 1, "A0")
	NET_C(TESTFUNC.Q, R89.1, R88.1)
#else
	NET_C(C47.2, U16.7, U16.2, R89.1, R88.1)
#endif

	NET_C(U16.3, R93.1, R91.2)
	NET_C(R93.2, U16.1, D10.A)
	NET_C(D10.K, R106.2)
	NET_C(R106.1, Q11.B, R133.2)
	NET_C(R133.1, GND)
	NET_C(Q11.E, GND)
	NET_C(Q11.C, R107.2)

	NET_C(I_PARTIAL_WARSHIP, U31.5)
	NET_C(U31.6, R57.1, R53.1, R55.1, C33.1)
	NET_C(C33.2, R63.1)
	NET_C(R63.2, R34.1, R51.2, R64.2, U8.6)
	NET_C(R34.2, U8.1, C26.2)
	ALIAS(STAR_SPIN_PARTIAL_WARSHIP_SUM, U8.1)
	NET_C(C26.1, U8.2, U7.6)
	NET_C(U8.3, GND)
	NET_C(U31.4, C34.1)
	NET_C(NOISE_A, C34.1)
	NET_C(C34.2, R64.1)
	NET_C(U8.5, R49.2, R50.1)
	NET_C(R49.1, GND)
	NET_C(U8.7, R61.1, R51.1)
	NET_C(R61.2, R60.2, U12.3)
	NET_C(R60.1, GND)
	NET_C(U12.2, GND)
	NET_C(U12.5, R88.2)
	NET_C(R89.2, U7.5)
	NET_C(U7.3, R48.2, R47.2)
	NET_C(U7.2, GND)
	NET_C(R48.1, GND)
	NET_C(R47.1, C35.2, U13.1, R50.2)
	NET_C(U13.3, GND)
	NET_C(U13.2, C35.1, U12.6)

	//
	// Sheet 7, middle (Fireball)
	//

	NET_C(I_FIREBALL, U26.11)
	NET_C(U26.10, U22.12, R125.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_FIREBALL)
	NET_C(D_FIREBALL.A, U22.12)
	NET_C(D_FIREBALL.K, I_V12)
#endif
	NET_C(R125.1, I_V12, R124.1)
	NET_C(R124.2, U22.13, C70.1)
	NET_C(C70.2, U22.10)
	NET_C(U22.8, U22.9, U22.11)
	NET_C(U22.11, R123.1)
	NET_C(R123.2, D12.A)
	NET_C(D12.K, R86.1, R87.1, C50.1)
	NET_C(C50.2, GND)
	NET_C(R86.2, U14.6, R85.1)
	NET_C(R87.2, U14.5, D8.K, Q8.D)
	NET_C(NOISE_B, Q8.G)
	NET_C(Q8.S, GND)
	NET_C(D8.A, GND)
	NET_C(R85.2, R71.1, U14.7)
	NET_C(R71.2, R70.1, C31.2)
	NET_C(C31.1, GND)
	NET_C(R70.2, C30.1, R149.1)
	NET_C(C30.2, U14.2, U14.1, C44.1)
	NET_C(R149.2, C29.2, U14.3)
	NET_C(C29.1, GND)
	ALIAS(FIREBALL_SUM, C44.2)

	//
	// Sheet 7, 2nd from bottom (Explosions)
	//

	NET_C(U9.1, GND)
	NET_C(U9.4, I_V12)
	NET_C(U9.2, I_VM12)
	NET_C(C28.1, R14.2)

#if (UNDERCLOCK_NOISE_GEN)

	NET_C(U9B.1, GND)
	NET_C(U9B.4, I_V12)
	NET_C(U9B.2, I_VM12)
	NET_C(U9B.3, C28.2)
	// Yes - AFuncs are frontiers as well
	AFUNC(NOISE_B_FUNC, 1, "A0")
	NET_C(R14.2, NOISE_B_FUNC.A0)
	ALIAS(NOISE_B, NOISE_B_FUNC.Q)
	NET_C(R14.1, GND)
	// Yes - AFuncs are frontiers as well
	AFUNC(NOISE_A_FUNC, 1, "A0")
	NET_C(U9.3, NOISE_A_FUNC.A0)
	NET_C(R52.1, NOISE_A_FUNC.Q)
	ALIAS(NOISE_A, R52.2)

#else

#if (ENABLE_NOISE_FRONTIERS)
	// Yes - AFuncs are frontiers as well
	AFUNC(NOISE_B_FUNC, 1, "A0")
	NET_C(R14.2, NOISE_B_FUNC.A0)
	ALIAS(NOISE_B, NOISE_B_FUNC.Q)
#else
	ALIAS(NOISE_B, R14.2)
#endif
	NET_C(R14.1, GND)
#if (ENABLE_NOISE_FRONTIERS)
	// Yes - AFuncs are frontiers as well
	AFUNC(NOISE_A_FUNC, 1, "A0")
	NET_C(U9.3, NOISE_A_FUNC.A0, C28.2)
	NET_C(R52.1, NOISE_A_FUNC.Q)
#else
	NET_C(U9.3, R52.1, C28.2)
#endif
	ALIAS(NOISE_A, R52.2)

#endif

	NET_C(I_SMALL_EXPL, U26.13)
	NET_C(U26.12, R32.2, U6.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_SMALL_EXPL)
	NET_C(D_SMALL_EXPL.A, U6.2)
	NET_C(D_SMALL_EXPL.K, I_V12)
#endif
	NET_C(U6.1, C22.1, R33.2)
	NET_C(R33.1, I_V12, R45.2, R46.2, R32.1)
	NET_C(R45.1, C23.2, U6.13)
	NET_C(R46.1, U6.12, U26.6)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(I_LARGE_EXPL)
	NET_C(I_LARGE_EXPL.A, U6.12)
	NET_C(I_LARGE_EXPL.K, I_V12)
#endif
	NET_C(I_LARGE_EXPL, U26.5)
	NET_C(C23.1, U6.10)
	NET_C(U6.8, U6.9, U6.11)
	NET_C(U6.11, D3.A)
	NET_C(D3.K, R43.1)
	NET_C(R43.2, R42.1, R44.1, C24.1)
	NET_C(C24.2, C21.2, Q3.S, Q2.S, D4.A, D1.A, C18.2, C20.1, GND)
	NET_C(R42.2, U5.6, R31.1)
	NET_C(R31.2, R38.1, U5.7)
	NET_C(U5.5, R44.2, Q3.D, D4.K)
	NET_C(Q3.G, NOISE_B)
	NET_C(R38.2, C19.1, R37.1)
	NET_C(C19.2, U5.2, U5.1, R29.1)
	NET_C(R37.2, U5.3, C18.1)
	NET_C(R29.2, C7.1, R30.1)
	ALIAS(EXPL_SUM, C7.2)

	NET_C(C22.2, U6.4)
	NET_C(U6.5, U6.6, D2.A, U6.3)
	NET_C(D2.K, R39.1)
	NET_C(R39.2, R41.1, R40.1, C21.1)
	NET_C(R41.2, U2.6, R13.1)
	NET_C(R13.2, U2.7, R11.1)
	NET_C(U2.5, R40.2, Q2.D, D1.K)
	NET_C(Q2.G, NOISE_B)
	NET_C(R11.2, C8.1, R12.1)
	NET_C(R12.2, C20.2, U2.3)
	NET_C(C8.2, U2.2, U2.1, R30.2)

	//
	// Sheet 7, bottom-left (PSG)
	//

	NET_C(R_PSG_1.2, R_PSG_2.2, R_PSG_3.2, R147.2, C73.1)
	NET_C(R147.1, GND)
	NET_C(C73.2, R146.1)
	NET_C(R146.2, U23.2, R141.1, C72.1)
	NET_C(U23.3, GND)
	NET_C(C72.2, R141.2, U23.1)
	ALIAS(PSG_SUM, U23.1)

	//
	// Sheet 7, bottom (Thrust)
	//

	NET_C(I_THRUST, U31.11)
	NET_C(U31.10, R96.1, R97.1, R98.2)
	NET_C(R98.1, I_V12)
	NET_C(R96.2, U15.2, R75.1)
	NET_C(U15.3, R97.2, Q7.D, D7.K)
	NET_C(Q7.G, NOISE_B)
	NET_C(Q7.S, GND)
	NET_C(D7.A, GND)
	NET_C(U15.1, R75.2, R95.1)
	NET_C(R95.2, C40.2, R73.1)
	NET_C(C40.1, GND)
	NET_C(R73.2, R74.1, C41.1)
	NET_C(R74.2, C37.2, U15.5)
	NET_C(C37.1, GND)
	NET_C(C41.2, U15.6, U15.7, C36.1)
	ALIAS(THRUST_SUM, C36.2)

	//
	// Sheet 8, top (Crafts Joining)
	//

	NET_C(I_CRAFTS_JOINING, U26.1)
	NET_C(U26.2, R120.1, U20.12)
	NET_C(R120.2, I_V12, R119.2, C48.1, R80.2, R79.2)
	NET_C(R119.1, C64.1, U20.13)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_CRAFTS_JOINING)
	NET_C(D_CRAFTS_JOINING.A, U20.13)
	NET_C(D_CRAFTS_JOINING.K, I_V12)
#endif
	NET_C(U20.11, U20.8, U20.9)
	NET_C(U20.10, C64.2, D9.K)
	NET_C(D9.A, R101.1)
	NET_C(R101.2, C48.2, R80.1, Q6.B)
	NET_C(Q6.E, R79.1)
	NET_C(Q6.C, Q5.E, Q4.E)

	// Another set of CD4069 oscillators:
	//
	// #4: R138=150K, C61=0.047uF,  Freq=102.05Hz
	// #5: R144=220K, C71=0.047uF,  Freq=69.58Hz
	// #6: R139=150K, C62=0.047uF,  Freq=102.05Hz

	CLOCK(O4CLK, 102.05)
	NET_C(O4CLK.GND, GND)
	NET_C(O4CLK.VCC, I_V12)
	NET_C(O4CLK.Q, R115.1, R116.2)
	NET_C(GND, R138.1, R138.2, C61.1, C61.2, U19.1, U19.3)

	CLOCK(O5CLK, 69.58)
	NET_C(O5CLK.GND, GND)
	NET_C(O5CLK.VCC, I_V12)
	NET_C(O5CLK.Q, R114.1, R117.2)
	NET_C(GND, R144.1, R144.2, C71.1, C71.2, U19.5, U19.9)

	CLOCK(O6CLK, 102.15)
	NET_C(O6CLK.GND, GND)
	NET_C(O6CLK.VCC, I_V12)
	NET_C(O6CLK.Q, R113.1, R118.2)
	NET_C(GND, R139.1, R139.2, C62.1, C62.2, U19.13, U19.11)

	NET_C(R115.2, R114.2, R113.2, C43.2, C42.1, R100.2)
	NET_C(C43.1, R65.1, U10.2)
	NET_C(U10.3, R100.1, R99.2, U18.5, R67.1, R110.2, R68.1, Q5.B, R134.2, Q9.B, GND)
	NET_C(R65.2, U10.1, C42.2, R78.1)
	NET_C(R78.2, Q4.B, R67.2)
	NET_C(Q4.C, R68.2, R76.1, U10.5)
	NET_C(Q5.C, R77.1, R66.1, U10.6)
	NET_C(R76.2, R77.2, I_VM12)
	NET_C(R66.2, U10.7)
	ALIAS(JOINING_SUM, U10.7)

	//
	// Sheet 8, middle (Docking Bang)
	//

	NET_C(R116.1, R117.1, R118.1, R99.1, C60.1, C59.1)
	NET_C(C60.2, R112.1, U18.6)
	NET_C(C59.2, R112.2, U18.7, R111.1)
	NET_C(R111.2, R110.1, Q10.B)
	NET_C(Q10.C, R134.1, R135.2, U18.3)
	NET_C(Q10.E, Q9.E, Q13.C)
	NET_C(Q9.C, U18.2, R109.1, R136.2)
	NET_C(R135.1, R136.1, I_VM12)
	NET_C(R109.2, U18.1)
	ALIAS(BANG_SUM, U18.1)

	NET_C(I_DOCKING_BANG, U26.3)
	NET_C(U26.4, R121.2, U20.2)
	NET_C(R121.1, I_V12, R127.2, R126.2, R122.1, C57.1, R142.1, R143.1)
	NET_C(R122.2, C63.1, U20.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_DOCKING_BANG)
	NET_C(D_DOCKING_BANG.A, U20.1)
	NET_C(D_DOCKING_BANG.K, I_V12)
#endif
	NET_C(U20.3, U20.5, U20.6)
	NET_C(U20.4, C63.2, D16.K)
	NET_C(D16.A, R137.1)
	NET_C(R137.2, C57.2, R142.2, Q13.B)
	NET_C(Q13.E, R143.2)

	//
	// Sheet 8, 2nd from bottom (Shoot)
	//

	NET_C(I_SHOOT, U26.9)
	NET_C(U26.8, R126.1, U22.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_SHOOT)
	NET_C(D_SHOOT.A, U22.2)
	NET_C(D_SHOOT.K, I_V12)
#endif
	NET_C(U22.1, R127.1, C69.2)
	NET_C(C69.1, U22.4, U11.2)
	NET_C(U22.5, U22.6, U22.3)
	NET_C(U22.3, D14.K)
	NET_C(D14.A, R140.1)
	NET_C(R140.2, D15.K, C68.2)
	NET_C(C68.1, C66.1, D13.A, GND)
	NET_C(D15.A, R102.1, D13.K) // and U21.3
	NET_C(C66.2, R103.1)        // and U21.2
	NET_C(R103.2, R102.2, R84.1)// and U21.1

#if (HLE_SHOOT_VCO)
	//
	//    R2 = 0.96721: HP = (0.000138452*A0) - 0.0000076731
	//    R2 = 0.97638: HP = (0.0000289361*A0*A0) + (0.000074399*A0) + 0.0000125173
	//    R2 = 0.97705: HP = (0.0000152345*A0*A0*A0) - (0.0000278973*A0*A0) + (0.000132881*A0) - 0.00000173354
	//    R2 = 0.97706: HP = (-0.00000384290*A0*A0*A0*A0) + (0.0000353504*A0*A0*A0) - (0.000063189*A0*A0) + (0.000156402*A0) - 0.00000642730
	//    R2 = 0.97712: HP = (0.0000167037*A0*A0*A0*A0*A0) - (0.000115736*A0*A0*A0*A0) + (0.000312133*A0*A0*A0) - (0.000372722*A0*A0) + (0.000308685*A0) - 0.0000319600
	//
	VARCLOCK(SHOOTCLK, 1, "max(0.000001,min(0.1,(0.0000152345*A0*A0*A0) - (0.0000278973*A0*A0) + (0.000132881*A0) - 0.00000173354))")
	NET_C(SHOOTCLK.GND, GND)
	NET_C(SHOOTCLK.VCC, I_V5)
	NET_C(SHOOTCLK.A0, D15.K)
	NET_C(SHOOTCLK.Q, SHOOTENV.A0)
	AFUNC(SHOOTENV, 1, "if(A0>2.5,11.1,-11.1)")
	NET_C(SHOOTENV.Q, U11.1)
	NET_C(GND, U21.3, U21.2)
#else
	NET_C(D15.A, U21.3)
	NET_C(U21.2, C66.2)
	NET_C(R103.2, U21.1)
#endif

	NET_C(R84.2, U11.1)
	NET_C(U11.12, R81.1)
	NET_C(U11.11, R82.1)
	NET_C(U11.9, R83.1)
	NET_C(U11.6, R69.1)
	NET_C(R81.2, R82.2, R83.2, R69.2, C49.1)
	ALIAS(SHOOT_SUM, C49.2)

	//
	// Sheet 8, bottom (Crafts Scale)
	//

	NET_C(I_CRAFTS_SCALE, U31.9)
	NET_C(U31.8, R130.1, R129.1, U17.4)
	NET_C(R130.2, R132.2, C55.1, R104.2, U17.8, C53.1, I_V12)
	NET_C(R129.2, R128.2, Q12.B)
	NET_C(Q12.C, D11.A, R131.1, R132.1)
	NET_C(R128.1, Q12.E, C52.1, U17.1, C54.1, C53.2, GND)
	NET_C(D11.K, R131.2, C55.2, C54.2, U17.5)
	NET_C(R104.1, R105.2, U17.7)
	NET_C(C52.2, U17.6, U17.2, R105.1)
	NET_C(U17.3, C51.1)
	ALIAS(SCALE_SUM, C51.2)

	//
	// Sheet 8, right (summing)
	//

	NET_C(JOINING_SUM, R25.1)
	NET_C(BANG_SUM, R21.1)
	NET_C(SHOOT_SUM, R23.1)
	NET_C(SCALE_SUM, R19.1)
	NET_C(EXPL_SUM, R26.1)
	NET_C(THRUST_SUM, R22.1)
	NET_C(MOVING_SUM, R27.1)
	NET_C(FIREBALL_SUM, R24.1)
	NET_C(STAR_SPIN_PARTIAL_WARSHIP_SUM, R20.1)
	NET_C(PSG_SUM, R18.1)

	NET_C(R25.2, R21.2, R23.2, R19.2, R26.2, R22.2, R27.2, R24.2, R20.2, R18.2)
	ALIAS(OUTPUT, R18.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, U21.5, U21.6)
	NET_C(GND, U23.5, U23.6)

	//
	// Unconnected outputs
	//

#if (ENABLE_FRONTIERS)
#define RXX 384
	OPTIMIZE_FRONTIER(C36.1, RES_K(3.9), RXX)
	OPTIMIZE_FRONTIER(C44.1, RES_K(4.7), RXX)
	OPTIMIZE_FRONTIER(R25.1, RES_K(47), RXX)
	OPTIMIZE_FRONTIER(R21.1, RES_K(47), RXX)
	OPTIMIZE_FRONTIER(R23.1, RES_K(82), RXX)
	OPTIMIZE_FRONTIER(R19.1, RES_K(470), RXX)
	OPTIMIZE_FRONTIER(R26.1, 100, RXX)
	OPTIMIZE_FRONTIER(R22.1, RES_K(3.9), RXX)
	OPTIMIZE_FRONTIER(R27.1, RES_K(100), RXX)
	OPTIMIZE_FRONTIER(R24.1, RES_K(4.7), RXX)
	OPTIMIZE_FRONTIER(R20.1, RES_K(22), RXX)
#endif

}
