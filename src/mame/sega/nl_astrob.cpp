// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Astro Blaster
//
// Derived from the schematics in the Astro Blaster manual.
//
// Known problems/issues:
//
//    * All sounds work under the right conditions.
//
//    * SONAR and BONUS sounds fail to trigger after the first
//       few seconds. For some reason the triggering via CD4011
//       never actually fires the one-shot that controls the
//       duration of the sound.
//
//    * Performance is poor, primarily due to the SONAR sound
//       and its connection to the mixing net. All attempts at
//       separating the two have ended in failure so far.
//

#include "netlist/devices/net_lib.h"
#include "nl_astrob.h"


//
// Optimizations
//

#define HLE_LASER_1_VCO (1)
#define HLE_LASER_2_VCO (1)
#define SIMPLIFY_SONAR (0)                  // only use one oscillator
#define ENABLE_SONAR_ALT (1)                // use frontiers to separate oscillators, modify UGF
#define ENABLE_FRONTIERS (1)
#define UNDERCLOCK_NOISE_GEN (1)



//
// Hacks
//

#define ADD_CLIPPING_DIODES (1)



#define D_1N5231(name) ZDIODE(name, "1N5231")
#define D_1N914(name) DIODE(name, "1N914")

#define Q_2N4403(name) QBJT_EB(name, "2N4403")

// JFET transistors not supported, but this should do the trick
#define Q_2N4093(name) MOSFET(name, "NMOS(VTO=-1 CAPMOD=0)")

#define LM555_DIP NE555_DIP
#define LM566_DIP NE566_DIP

#define TTL_74LS00_DIP TTL_7400_DIP
#define TTL_74LS04_DIP TTL_7404_DIP

// Define some random factors (5%)

#define FRND1   1.023
#define FRND2   1.017
#define FRND3   1.005
#define FRND4   1.014
#define FRND5   1.049
#define FRND6   1.044
#define FRND7   1.016
#define FRND8   1.037
#define FRND9   1.030
#define FRND10  1.001
#define FRND11  1.034
#define FRND12  1.043
#define FRND13  1.011
#define FRND14  1.016
#define FRND15  1.011
#define FRND16  1.006
#define FRND17  1.009
#define FRND18  1.007
#define FRND19  1.035
#define FRND20  1.004

//
// Main netlist
//

NETLIST_START(astrob)
{

#if 1
	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 4e-5)
#if (SIMPLIFY_SONAR)
	PARAM(Solver.Solver_54.DYNAMIC_MIN_TIMESTEP, 4e-6)  // gets rid of NR loops failure
#else
#if !(ENABLE_SONAR_ALT)
	PARAM(Solver.Solver_41.DYNAMIC_MIN_TIMESTEP, 6e-6)  // gets rid of NR loops failure
#endif
#endif
#else
	SOLVER(Solver, 48000)
#endif

	// Overwrite model - the default model uses minimum datasheet
	// specifications for 5V. These are for 10V and thus closer to the
	// 12V used in this circuit.
	NET_MODEL("CD4XXX FAMILY(TYPE=CMOS IVL=0.3 IVH=0.7 OVL=0.05 OVH=0.05 ORL=384 ORH=384)")

	TTL_INPUT(I_LO_D0, 0)
	NET_C(I_LO_D0, U31.1)
	ALIAS(I_INVADER_1, U31.2)
	TTL_INPUT(I_LO_D1, 0)
	NET_C(I_LO_D1, U31.3)
	ALIAS(I_INVADER_2, U31.4)
	TTL_INPUT(I_LO_D2, 0)
	NET_C(I_LO_D2, U31.11)
	ALIAS(I_INVADER_3, U31.10)
	TTL_INPUT(I_LO_D3, 0)
	NET_C(I_LO_D3, U31.9)
	ALIAS(I_INVADER_4, U31.8)
	TTL_INPUT(I_LO_D4, 0)
	NET_C(I_LO_D4, U30.13)
	ALIAS(I_ASTROIDS, U30.12)
	TTL_INPUT(I_LO_D5, 0)
	ALIAS(I_MUTE, I_LO_D5)
	TTL_INPUT(I_LO_D6, 0)
	NET_C(I_LO_D6, U30.3, U29.11)
	ALIAS(I_REFILL, U29.10)
	ALIAS(I_REFILL_Q, U30.4)
	TTL_INPUT(I_LO_D7, 0)
	ALIAS(W, I_LO_D7)
	NET_C(I_LO_D7, U31.5)
	ALIAS(I_WARP, U31.6)

	NET_C(GND, I_LO_D0.GND, I_LO_D1.GND, I_LO_D2.GND, I_LO_D3.GND, I_LO_D4.GND, I_LO_D5.GND, I_LO_D6.GND, I_LO_D7.GND)
	NET_C(I_V5, I_LO_D0.VCC, I_LO_D1.VCC, I_LO_D2.VCC, I_LO_D3.VCC, I_LO_D4.VCC, I_LO_D5.VCC, I_LO_D6.VCC, I_LO_D7.VCC)

	TTL_INPUT(I_HI_D0, 0)
	NET_C(I_HI_D0, U26.5)
	ALIAS(I_LASER_1, U26.6)
	TTL_INPUT(I_HI_D1, 0)
	NET_C(I_HI_D1, U26.3)
	ALIAS(I_LASER_2, U26.4)
	TTL_INPUT(I_HI_D2, 0)
	NET_C(I_HI_D2, U26.11)
	ALIAS(I_SHORT_EXPL, U26.10)
	TTL_INPUT(I_HI_D3, 0)
	NET_C(I_HI_D3, U26.9)
	ALIAS(I_LONG_EXPL, U26.8)
	TTL_INPUT(I_HI_D4, 0)
	NET_C(I_HI_D4, U26.1)
	ALIAS(I_ATTACK_RATE, U26.2)
	TTL_INPUT(I_HI_D5, 0)
	NET_C(I_HI_D5, U31.13)
	ALIAS(I_RATE_RESET, U31.12)
	TTL_INPUT(I_HI_D6, 0)
	NET_C(I_HI_D6, U29.9)
	ALIAS(I_BONUS, U29.8)
	TTL_INPUT(I_HI_D7, 0)
	NET_C(I_HI_D7, U29.5)
	ALIAS(I_SONAR, U29.6)

	NET_C(GND, I_HI_D0.GND, I_HI_D1.GND, I_HI_D2.GND, I_HI_D3.GND, I_HI_D4.GND, I_HI_D5.GND, I_HI_D6.GND, I_HI_D7.GND)
	NET_C(I_V5, I_HI_D0.VCC, I_HI_D1.VCC, I_HI_D2.VCC, I_HI_D3.VCC, I_HI_D4.VCC, I_HI_D5.VCC, I_HI_D6.VCC, I_HI_D7.VCC)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_VM12, -12)

	RES(R1, RES_K(100) * FRND1)  // part of SONAR circuit that relies on subtle part differences
	RES(R2, RES_K(1.5) * FRND2)  // part of SONAR circuit that relies on subtle part differences
	RES(R3, RES_K(330) * FRND3)  // part of SONAR circuit that relies on subtle part differences
	RES(R4, RES_K(10)  * FRND4)  // part of SONAR circuit that relies on subtle part differences
#if (SIMPLIFY_SONAR)
	// use less resistance to account for only emulating 1/4 identical circuits
	RES(R5, RES_K(17))  // part of SONAR circuit that relies on subtle part differences
#else
	RES(R5, RES_K(68)  * FRND5)  // part of SONAR circuit that relies on subtle part differences
#endif
	RES(R6, RES_K(68))
	RES(R7, RES_K(22))
	RES(R8, RES_K(18))
	RES(R9, RES_K(33))
	RES(R10, RES_K(100))
	RES(R11, RES_K(150))
	RES(R12, RES_K(39))
	RES(R13, RES_K(330))
	RES(R14, RES_K(47))
	RES(R15, RES_K(220))
	RES(R16, RES_K(10))
	RES(R17, RES_K(10))
	RES(R18, RES_M(2.7))
//  RES(R19, RES_K(1))
	RES(R20, RES_M(1))
	RES(R21, RES_K(100))
	RES(R22, RES_K(470))
	RES(R23, RES_K(100))
	RES(R24, RES_M(2.2))
	RES(R25, RES_K(10))
	RES(R26, RES_K(820))
	RES(R27, RES_K(47))
	RES(R28, RES_K(22))
	RES(R29, RES_K(10))
	RES(R30, RES_K(82))
	RES(R31, RES_K(10))
	RES(R32, RES_K(22))
	RES(R33, RES_K(39))
	RES(R34, RES_K(4.7))
	RES(R35, RES_K(4.7))
	RES(R36, RES_K(100) * FRND6)   // part of SONAR circuit that relies on subtle part differences
	RES(R37, RES_K(1.5) * FRND7)   // part of SONAR circuit that relies on subtle part differences
	RES(R38, RES_K(330) * FRND8)   // part of SONAR circuit that relies on subtle part differences
	RES(R39, RES_K(10)  * FRND9)   // part of SONAR circuit that relies on subtle part differences
	RES(R40, RES_K(68)  * FRND10)  // part of SONAR circuit that relies on subtle part differences
	RES(R41, RES_K(10))
	RES(R42, RES_K(100))
	RES(R43, RES_K(470))
	RES(R44, RES_M(1))
	RES(R45, RES_K(100))
	RES(R46, RES_K(470))
	RES(R47, RES_K(10))
	RES(R48, RES_K(82))
	RES(R49, RES_K(39))
	RES(R50, RES_K(22))
	RES(R51, RES_K(10))
	RES(R52, RES_K(68))
	RES(R53, RES_K(10))
	RES(R54, RES_K(22))
	RES(R55, RES_M(2.2))
	RES(R56, RES_K(33))
	RES(R57, RES_K(33))
	RES(R58, RES_K(820))
	RES(R59, RES_K(100))
	RES(R60, RES_K(10))
	RES(R61, RES_K(100))
	RES(R62, RES_K(100) * FRND11)   // part of SONAR circuit that relies on subtle part differences
	RES(R63, RES_K(1.5) * FRND12)   // part of SONAR circuit that relies on subtle part differences
	RES(R64, RES_K(330) * FRND13)   // part of SONAR circuit that relies on subtle part differences
	RES(R65, RES_K(10)  * FRND14)   // part of SONAR circuit that relies on subtle part differences
	RES(R66, RES_K(68)  * FRND15)   // part of SONAR circuit that relies on subtle part differences
	RES(R67, RES_K(10))
	RES(R68, RES_K(82))
	RES(R69, RES_K(470))
	RES(R70, RES_K(100))
	RES(R71, RES_K(10))
	RES(R72, RES_K(10))
	RES(R73, RES_K(100))
	RES(R74, RES_K(4.7))
	RES(R75, RES_K(100))
	RES(R76, RES_K(22))
	RES(R77, RES_K(330))
	RES(R78, RES_K(1))
	RES(R79, RES_K(150))
	RES(R80, RES_M(1))
	RES(R81, RES_K(10))
	RES(R82, RES_K(100))
	RES(R83, RES_K(1))
	RES(R84, RES_K(5.6))
	RES(R85, RES_K(150))
	RES(R86, RES_K(10))
	RES(R87, RES_K(10))
	RES(R88, RES_K(100))
	RES(R89, RES_K(10))
	RES(R90, RES_K(100))
	RES(R91, RES_K(100) * FRND16)  // part of SONAR circuit that relies on subtle part differences
	RES(R92, RES_K(1.5) * FRND17)   // part of SONAR circuit that relies on subtle part differences
	RES(R93, RES_K(330) * FRND18)  // part of SONAR circuit that relies on subtle part differences
	RES(R94, RES_K(10)  * FRND19)   // part of SONAR circuit that relies on subtle part differences
	RES(R95, RES_K(68)  * FRND20)   // part of SONAR circuit that relies on subtle part differences
	RES(R96, RES_K(10))
	RES(R97, RES_K(4.7))
	RES(R98, RES_M(1))
	RES(R99, RES_M(1))
	RES(R100, RES_M(1))
	RES(R101, RES_M(1))
	RES(R102, RES_K(470))
	RES(R103, RES_M(1))
	RES(R104, RES_K(10))
	RES(R105, RES_K(10))
	RES(R106, RES_M(2.7))
	RES(R107, RES_K(100))
	RES(R108, RES_K(100))
	RES(R109, RES_K(10))
	RES(R110, RES_K(10))
	RES(R111, RES_K(10))
	RES(R112, RES_K(10))
	RES(R113, RES_K(10))
	RES(R114, 100)
//  RES(R115, RES_K(100))   -- part of final amp (not emulated)
//  RES(R116, RES_K(470))   -- part of final amp (not emulated)
	RES(R117, RES_K(1))
	RES(R118, RES_K(470))
	RES(R119, RES_K(470))
	RES(R120, RES_K(220))
	RES(R121, RES_K(10))
	RES(R122, RES_K(4.7))
//  RES(R123, RES_K(1))     -- part of final amp (not emulated)
//  RES(R124, RES_K(100))   -- part of final amp (not emulated)
	RES(R125, RES_K(82))
	RES(R126, RES_K(39))
	RES(R127, RES_K(10))
	RES(R128, RES_K(22))
	RES(R129, RES_K(10))
	RES(R130, RES_K(22))
	RES(R131, RES_K(39))
	RES(R132, RES_K(82))
	RES(R133, RES_K(100))
	RES(R134, RES_M(2.7))
//  RES(R135, RES_K(22))    -- part of final amp (not emulated)
	RES(R136, RES_K(1))
	RES(R137, RES_K(4.7))
	RES(R138, RES_M(2.2))
	RES(R139, RES_K(4.7))
	RES(R140, RES_K(4.7))
//  RES(R141, RES_K(22))
//  RES(R142, RES_M(1))     -- part of final amp (not emulated)
//  RES(R143, RES_K(22))    -- part of final amp (not emulated)
//  RES(R144, RES_K(22))    -- part of final amp (not emulated)
	RES(R145, RES_K(4.7))
	RES(R146, RES_K(100))
	RES(R147, RES_M(2.2))
	RES(R148, RES_K(100))
	RES(R149, RES_K(100))
	RES(R150, RES_K(33))
	RES(R151, RES_M(1))
	RES(R152, RES_K(22))
	RES(R153, RES_K(47))
	RES(R154, RES_K(24))
	RES(R155, RES_K(56))
	RES(R156, RES_K(39))
	RES(R157, RES_K(27))
	RES(R158, RES_K(33))
	RES(R159, RES_K(62))
	RES(R160, RES_K(82))
	RES(R161, RES_K(120))
	RES(R162, RES_K(4.7))
	RES(R163, RES_K(10))
	RES(R164, RES_K(10))
	RES(R165, RES_M(1))
	RES(R166, RES_M(1))
	RES(R167, RES_K(1))
	RES(R168, RES_M(1))
	RES(R169, RES_K(100))
	RES(R170, RES_M(2.2))
	RES(R171, RES_K(100))
	RES(R172, RES_K(33))
	RES(R173, RES_K(47))
	RES(R174, RES_K(47))
	RES(R175, RES_K(47))
	RES(R176, RES_K(47))
	RES(R177, RES_K(47))
	RES(R178, RES_K(22))
	RES(R179, RES_K(47))
	RES(R180, RES_K(47))
	RES(R181, RES_K(47))
	RES(R182, RES_K(10))
	RES(R183, RES_K(10))
	RES(R184, RES_K(4.7))
	RES(R185, RES_M(1))
	RES(R186, RES_K(1))

	CAP(C1, CAP_U(0.01))
	CAP(C2, CAP_U(0.01))
	CAP(C3, CAP_U(0.1))
	CAP(C4, CAP_U(0.05))
	CAP(C5, CAP_U(0.1))
	CAP(C6, CAP_U(10))
	CAP(C7, CAP_U(0.1))
	CAP(C8, CAP_U(0.1))
	CAP(C9, CAP_U(0.1))
//  CAP(C10, CAP_U(0.05))
//  CAP(C11, CAP_U(0.05))
	CAP(C12, CAP_U(10))
//  CAP(C13, CAP_U(0.05))
//  CAP(C14, CAP_U(0.05))
	CAP(C15, CAP_U(10))
	CAP(C16, CAP_U(0.01))
	CAP(C17, CAP_U(0.01))
	CAP(C18, CAP_U(0.022))
	CAP(C19, CAP_U(0.05))
	CAP(C20, CAP_U(0.1))
	CAP(C21, CAP_U(0.33))
	CAP(C22, CAP_U(0.33))
	CAP(C23, CAP_U(0.01))
	CAP(C24, CAP_U(0.01))
	CAP(C25, CAP_U(0.01))
	CAP(C26, CAP_U(0.022))
	CAP(C27, CAP_U(10))
	CAP(C28, CAP_U(0.05))
//  CAP(C29, CAP_U(0.05))
//  CAP(C30, CAP_U(0.05))
	CAP(C31, CAP_U(0.0047))
	CAP(C32, CAP_U(0.05))
	CAP(C33, CAP_U(10))
	CAP(C34, CAP_U(10))
	CAP(C35, CAP_U(0.1))
	CAP(C36, CAP_U(0.0022))
	CAP(C37, CAP_U(0.1))
	CAP(C38, CAP_U(0.1))
	CAP(C39, CAP_U(0.1))
	CAP(C40, CAP_U(0.01))
	CAP(C41, CAP_U(0.01))
//  CAP(C42, CAP_U(0.001))
//  CAP(C43, CAP_U(10))
//  CAP(C44, CAP_U(0.001))
//  CAP(C45, CAP_U(0.001))
//  CAP(C46, CAP_U(10))
	CAP(C47, CAP_U(4.7))
//  CAP(C48, CAP_U(22))     -- part of final amp (not emulated)
	CAP(C49, CAP_U(0.05))
	CAP(C50, CAP_U(0.05))
	CAP(C51, CAP_U(0.05))
	CAP(C52, CAP_U(0.05))
	CAP(C53, CAP_U(0.1))
//  CAP(C54, CAP_U(0.05))
	CAP(C55, CAP_U(0.05))
	CAP(C56, CAP_U(10))
	CAP(C57, CAP_U(0.1))
//  CAP(C58, CAP_U(4.7))    -- part of final amp (not emulated)
	CAP(C59, CAP_U(0.33))
	CAP(C60, CAP_U(4.7))
	CAP(C61, CAP_U(4.7))
	CAP(C62, CAP_U(10))
	CAP(C63, CAP_U(0.1))
	CAP(C64, CAP_U(0.1))
	CAP(C65, CAP_U(0.01))
//  CAP(C66, CAP_U(0.05))
	CAP(C67, CAP_U(0.05))
	CAP(C68, CAP_U(0.0047))
	CAP(C69, CAP_U(4.7))
	CAP(C70, CAP_U(0.1))
	CAP(C71, CAP_U(0.022))
	CAP(C72, CAP_U(0.05))
//  CAP(C73, CAP_U(0.05))
	CAP(C74, CAP_U(0.05))
	CAP(C75, CAP_U(0.05))
	CAP(C76, CAP_U(0.022))
	CAP(C77, CAP_U(0.0047))
	CAP(C78, CAP_U(0.1))
//  CAP(C79, CAP_U(0.05))
	CAP(C80, CAP_U(4.7))
	CAP(C81, CAP_U(0.05))
	CAP(C82, CAP_U(0.1))

	D_1N5231(D1)
	D_1N914(D2)
	D_1N5231(D3)
	D_1N5231(D4)
	D_1N5231(D5)
	D_1N5231(D6)
	D_1N914(D7)
	D_1N914(D8)
	D_1N914(D9)
	D_1N5231(D10)
	D_1N914(D11)
	D_1N914(D12)
//  D_1N914(D13)            -- part of final amp (not emulated)
	D_1N914(D14)
	D_1N914(D15)
	D_1N914(D16)
	D_1N914(D17)
	D_1N914(D18)
	D_1N914(D19)
	D_1N914(D20)
	D_1N914(D21)
	D_1N914(D22)
	D_1N914(D23)
	D_1N914(D24)
	D_1N914(D25)
	D_1N914(D26)
	D_1N914(D27)
	D_1N914(D28)
	D_1N914(D29)
	D_1N914(D30)
	D_1N914(D31)

	Q_2N4403(Q1)
	Q_2N4403(Q2)
	Q_2N4403(Q3)
	//Q_2N4093(Q4)  // avoid singular matrix being created
	Q_2N4093(Q5)
	Q_2N4093(Q6)
	Q_2N4403(Q7)
	Q_2N4403(Q8)
	Q_2N4403(Q9)
	Q_2N4403(Q10)

	TL084_DIP(U1)           // Op. Amp.
	NET_C(U1.4, I_V12)
	NET_C(U1.11, I_VM12)

	TL084_DIP(U2)           // Op. Amp.
	NET_C(U2.4, I_V12)
	NET_C(U2.11, I_VM12)

#if (ENABLE_SONAR_ALT)
	// Oscillators are of order 1kHz. No need to to have UGF in MHz range
	PARAM(U1.A.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U1.B.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U1.C.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U1.D.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U2.A.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U2.B.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U2.C.MODEL, "TL084(TYPE=3 UGF=10k)")
	PARAM(U2.D.MODEL, "TL084(TYPE=3 UGF=10k)")
#endif

	CD4017_DIP(U3)          // Decade Counter/Divider
	NET_C(U3.8, GND)
	NET_C(U3.16, I_V12)

	NE555_DIP(U4)           // Timer

	NE555_DIP(U5)           // Timer

	NE555_DIP(U6)           // Timer

	TL084_DIP(U7)           // Op. Amp.
	NET_C(U7.4, I_V12)
	NET_C(U7.11, I_VM12)

	MM5837_DIP(U8)          // Noise Generator
#if (UNDERCLOCK_NOISE_GEN)
	// officially runs at 48-112kHz, but little noticeable difference
	// in exchange for a big performance boost
	PARAM(U8.FREQ, 12000)
#endif

	CD4011_DIP(U9)          // Quad 2-Input NAND Gates
	NET_C(U9.7, GND)
	NET_C(U9.14, I_V12)

	CD4011_DIP(U10)         // Quad 2-Input NAND Gates
	NET_C(U10.7, GND)
	NET_C(U10.14, I_V12)

	CD4011_DIP(U11)         // Quad 2-Input NAND Gates
	NET_C(U11.7, GND)
	NET_C(U11.14, I_V12)

	CD4024_DIP(U12)         // 7-Stage Ripple Binary Counter
	NET_C(U12.7, GND)
	NET_C(U12.14, I_V12)

	NE555_DIP(U13)          // Timer

	CD4024_DIP(U14)         // 7-Stage Ripple Binary Counter
	NET_C(U14.7, GND)
	NET_C(U14.14, I_V12)

	CD4017_DIP(U15)         // Decade Counter/Divider
	NET_C(U15.8, GND)
	NET_C(U15.16, I_V12)

	TL084_DIP(U16)          // Op. Amp.
	NET_C(U16.4, I_V12)
	NET_C(U16.11, I_VM12)

	NE555_DIP(U17)          // Timer

	NE555_DIP(U18)          // Timer

	CD4024_DIP(U19)         // 7-Stage Ripple Binary Counter
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V12)

	NE555_DIP(U20)          // Timer

	CD4011_DIP(U21)         // Quad 2-Input NAND Gates
	NET_C(U21.7, GND)
	NET_C(U21.14, I_V12)

	TL084_DIP(U22)          // Op. Amp.
	NET_C(U22.4, I_V12)
	NET_C(U22.11, I_VM12)

	NE555_DIP(U23)          // Timer

	NE555_DIP(U24)          // Timer

	CD4011_DIP(U25)         // Quad 2-Input NAND Gates
	NET_C(U25.7, GND)
	NET_C(U25.14, I_V12)

	TTL_7407_DIP(U26)       // Hex Buffers with High Votage Open-Collector Outputs
	NET_C(U26.7, GND)
	NET_C(U26.14, I_V5)

	CD4011_DIP(U27)         // Quad 2-Input NAND Gates
	NET_C(U27.7, GND)
	NET_C(U27.14, I_V12)

	CD4024_DIP(U28)         // 7-Stage Ripple Binary Counter
	NET_C(U28.7, GND)
	NET_C(U28.14, I_V12)

	TTL_7407_DIP(U29)       // Hex Buffers with High Votage Open-Collector Outputs
	NET_C(U29.7, GND)
	NET_C(U29.14, I_V5)

	TTL_7406_DIP(U30)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U30.7, GND)
	NET_C(U30.14, I_V5)

	TTL_7406_DIP(U31)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U31.7, GND)
	NET_C(U31.14, I_V5)

//  TTL_74LS374_DIP(U32)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U32.10, GND)
//  NET_C(U32.20, I_V5)

//  TTL_74LS374_DIP(U33)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U33.10, GND)
//  NET_C(U33.20, I_V5)

//  TTL_74LS00_DIP(U34)     // Quad 4-Input NAND Gate
//  NET_C(U34.7, GND)
//  NET_C(U34.14, I_V5)

//  TTL_74LS30_DIP(U35)     // 8-Input NAND Gate
//  NET_C(U35.7, GND)
//  NET_C(U35.14, I_V5)

//  TTL_74LS04_DIP(U36)     // Hex Inverting Gates
//  NET_C(U36.7, GND)
//  NET_C(U36.14, I_V5)

	NE555_DIP(U37)          // Timer

	NE555_DIP(U38)          // Timer

	//
	// Sheet 7, top-left (SONAR)
	//

	NET_C(U1.3, GND)
	NET_C(U1.2, D1.A, R4.1)
	NET_C(U1.1, D1.K, R1.1)
	NET_C(R1.2, R2.2, C1.1, C2.1)
	NET_C(R2.1, GND)
	NET_C(R4.2, R3.2, U1.7, C2.2, R5.2)
	NET_C(C1.2, R3.1, U1.6)
	NET_C(U1.5, GND)

#if (SIMPLIFY_SONAR)
	// sonar has 4 identical circuits; reduce the net size
	// by only emulating one and multiplying it by 4
	NET_C(U1.9, U1.10, U1.12, U1.13, GND)
	NET_C(R36.1, R36.2, R37.1, R37.2, R38.1, R38.2, R39.1, R39.2, R40.1, R40.2, GND)
	NET_C(C16.1, C16.2, C17.1, C17.2, GND)
	NET_C(D4.A, D4.K, GND)

	NET_C(U2.2, U2.3, U2.5, U2.6, GND)
	NET_C(R62.1, R62.2, R63.1, R63.2, R64.1, R64.2, R65.1, R65.2, R66.1, R66.2, GND)
	NET_C(C24.1, C24.2, C25.1, C25.2, GND)
	NET_C(D6.A, D6.K, GND)

	NET_C(U2.9, U2.10, U2.12, U2.13, GND)
	NET_C(R91.1, R91.2, R92.1, R92.2, R93.1, R93.2, R94.1, R94.2, R95.1, R95.2, GND)
	NET_C(C40.1, C40.2, C41.1, C41.2, GND)
	NET_C(D10.A, D10.K, GND)

	NET_C(R5.1, R114.1, Q3.B)
#else
	NET_C(U1.12, GND)
	NET_C(U1.13, D4.A, R39.1)
	NET_C(U1.14, D4.K, R36.1)
	NET_C(R36.2, R37.2, C16.1, C17.1)
	NET_C(R37.1, GND)
	NET_C(R39.2, R38.2, U1.8, C17.2, R40.2)
	NET_C(C16.2, R38.1, U1.9)
	NET_C(U1.10, GND)

	NET_C(U2.3, GND)
	NET_C(U2.2, D6.A, R65.1)
	NET_C(U2.1, D6.K, R62.1)
	NET_C(R62.2, R63.2, C24.1, C25.1)
	NET_C(R63.1, GND)
	NET_C(R65.2, R64.2, U2.7, C25.2, R66.2)
	NET_C(C24.2, R64.1, U2.6)
	NET_C(U2.5, GND)

	NET_C(U2.12, GND)
	NET_C(U2.13, D10.A, R94.1)
	NET_C(U2.14, D10.K, R91.1)
	NET_C(R91.2, R92.2, C40.1, C41.1)
	NET_C(R92.1, GND)
	NET_C(R94.2, R93.2, U2.8, C41.2, R95.2)
	NET_C(C40.2, R93.1, U2.9)
	NET_C(U2.10, GND)

	NET_C(R5.1, R40.1, R114.1, Q3.B, R95.1, R66.1)

#if (ENABLE_SONAR_ALT)
	// The oscillators need a small offset voltage to start oscillating.
	// Without frontiers I assume the offset voltage is created due to the
	// slight differences in resistor values in the four (now connected)
	// oscillators.

	ANALOG_INPUT(I_VOFF, 0.1)
	RES(RDUM1, RES_M(1))
	RES(RDUM2, RES_M(1))
	RES(RDUM3, RES_M(1))
	RES(RDUM4, RES_M(1))
	NET_C(R5.2, RDUM1.1)
	NET_C(R40.2, RDUM2.1)
	NET_C(R95.2, RDUM3.1)
	NET_C(R66.2, RDUM4.1)
	NET_C(I_VOFF, RDUM1.2, RDUM2.2, RDUM3.2, RDUM4.2)

	OPTIMIZE_FRONTIER(R5.2, RES_K(68), 192)
	OPTIMIZE_FRONTIER(R40.2, RES_K(68), 192)
	OPTIMIZE_FRONTIER(R95.2, RES_K(68), 192)
	OPTIMIZE_FRONTIER(R66.2, RES_K(68), 192)
#endif
#endif

	NET_C(R114.2, GND)
	NET_C(Q3.E, Q2.E, Q1.C)
	NET_C(Q1.B, R108.2, C47.2, R117.2)
	NET_C(C47.1, R108.1, I_V12)
	NET_C(I_V12, R109.1)
	NET_C(R109.2, Q1.E)
	NET_C(R117.1, D2.A)
	NET_C(D2.K, U11.4, C4.2)
	NET_C(C4.1, U11.2, R20.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_SONAR)
	NET_C(D_SONAR.A, U11.2)
	NET_C(D_SONAR.K, I_V12)
#endif
	NET_C(U11.5, U11.6, U11.3)
	NET_C(U11.1, R21.1)
	NET_C(I_SONAR, R21.1)
	NET_C(R21.2, R20.2, I_V12)

	NET_C(Q2.B, GND)
	NET_C(Q2.C, R110.2, R111.2, U7.3)
	NET_C(R110.1, GND)
	NET_C(R111.1, R112.1, I_VM12)
	NET_C(Q3.C, U7.2, R112.2, R113.1)
	NET_C(R113.2, U7.1)
	ALIAS(SONAR, U7.1)

	//
	// Sheet 7, middle-left (LASER-1)
	//

	NET_C(I_LASER_1, R146.1, U25.9)
	NET_C(U25.8, C64.2, R138.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_LASER_1)
	NET_C(D_LASER_1.A, U25.8)
	NET_C(D_LASER_1.K, I_V12)
#endif
	NET_C(R146.2, R138.2, R134.2, R139.2, U24.8, C56.1, I_V12)
	NET_C(U25.10, U25.5, U25.6, D11.K, U24.4)
	NET_C(C64.1, U25.4, U19.2)
	NET_C(D11.A, R107.1, C57.2)
	NET_C(C57.1, Q10.C, C65.1, U24.1, GND)
	NET_C(R107.2, Q10.B, R134.1)
	NET_C(Q10.E, U24.5)
#if (HLE_LASER_1_VCO)
	//
	//    R2 = 0.98461: HP = (0.00000524285*A0) + 0.00000563193
	//    R2 = 0.99441: HP = (0.000000368659*A0*A0) + (0.00000116694*A0) + 0.0000155514
	//    R2 = 0.99797: HP = (0.000000154808*A0*A0*A0) - (0.00000213809*A0*A0) + (0.0000138122*A0) - 0.00000398935
	//    R2 = 0.99877: HP = (-0.0000000527853*A0*A0*A0*A0) + (0.00000128033*A0*A0*A0) - (0.0000107258*A0*A0) + (0.0000413916*A0) - 0.0000352437
	//    R2 = 0.99943: HP = (0.0000000343262*A0*A0*A0*A0*A0) - (0.00000096054*A0*A0*A0*A0) + (0.0000105481*A0*A0*A0) - (0.0000561978*A0*A0) + (0.000148191*A0) - 0.000131018
	//
	VARCLOCK(LASER1CLK, 1, "max(0.000001,min(0.1,(0.0000000343262*A0*A0*A0*A0*A0) - (0.00000096054*A0*A0*A0*A0) + (0.0000105481*A0*A0*A0) - (0.0000561978*A0*A0) + (0.000148191*A0) - 0.000131018))")
	NET_C(LASER1CLK.GND, GND)
	NET_C(LASER1CLK.VCC, I_V12)
	NET_C(LASER1CLK.A0, Q10.E)
	NET_C(LASER1CLK.Q, LASER1ENV.A1)
	AFUNC(LASER1ENV, 2, "if(A0>6,A1,0)")
	NET_C(LASER1ENV.A0, U25.10)
	NET_C(LASER1ENV.Q, U19.1)
	NET_C(U24.3, GND)
	NET_C(U24.2, U24.6, R140.1, C65.2, GND)
#else
	NET_C(U24.3, U19.1)
	NET_C(U24.2, U24.6, R140.1, C65.2)
#endif
	NET_C(R140.2, U24.7, R139.1)
	NET_C(C56.2, GND)
	NET_C(U19.12, R132.1)
	NET_C(U19.11, R131.1)
	NET_C(U19.9, R130.1)
	NET_C(U19.6, R105.1)
	NET_C(U19.4, R104.1)
	NET_C(R104.2, R105.2, R130.2, R131.2, R132.2, C52.1)
	ALIAS(LASER_1, C52.2)

	//
	// Sheet 7, bottom-left (LASER-2)
	//

	NET_C(I_LASER_2, R149.1, U25.12)
	NET_C(U25.13, C63.2, R147.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_LASER_2)
	NET_C(D_LASER_2.A, U25.13)
	NET_C(D_LASER_2.K, I_V12)
#endif
	NET_C(R149.2, R147.2, R106.2, R145.2, U20.8, C62.1, I_V12)
	NET_C(U25.11, U25.1, U25.2, D12.K, U20.4)
	NET_C(C63.1, U25.3, U14.2)
	NET_C(D12.A, R133.1, C53.2)
	NET_C(C53.1, Q9.C, C68.1, U20.1, GND)
	NET_C(R133.2, Q9.B, R106.1)
	NET_C(Q9.E, U20.5)
#if (HLE_LASER_2_VCO)
	//
	//    R2 = 0.98942: HP = (0.00000251528*A0) + 0.00000244265
	//    R2 = 0.99596: HP = (0.000000160852*A0*A0) + (0.000000694298*A0) + 0.00000690896
	//    R2 = 0.99821: HP = (0.000000068284*A0*A0*A0) - (0.000000949931*A0*A0) + (0.00000630369*A0) - 0.00000175548
	//    R2 = 0.99896: HP = (-0.0000000291178*A0*A0*A0*A0) + (0.000000689145*A0*A0*A0) - (0.00000568073*A0*A0) + (0.0000214603*A0) - 0.0000188875
	//    R2 = 0.99937: HP = (0.0000000153499*A0*A0*A0*A0*A0) - (0.000000433800*A0*A0*A0*A0) + (0.00000480504*A0*A0*A0) - (0.0000257871*A0*A0) + (0.000068467*A0) - 0.0000608574
	//
	VARCLOCK(LASER2CLK, 1, "max(0.000001,min(0.1,(0.0000000153499*A0*A0*A0*A0*A0) - (0.000000433800*A0*A0*A0*A0) + (0.00000480504*A0*A0*A0) - (0.0000257871*A0*A0) + (0.000068467*A0) - 0.0000608574))")
	NET_C(LASER2CLK.GND, GND)
	NET_C(LASER2CLK.VCC, I_V12)
	NET_C(LASER2CLK.A0, Q9.E)
	NET_C(LASER2CLK.Q, LASER2ENV.A1)
	AFUNC(LASER2ENV, 2, "if(A0>6,A1,0)")
	NET_C(LASER2ENV.A0, U25.11)
	NET_C(LASER2ENV.Q, U14.1)
	NET_C(U20.3, GND)
	NET_C(U20.2, U20.6, R137.1, C68.2, GND)
#else
	NET_C(U20.3, U14.1)
	NET_C(U20.2, U20.6, R137.1, C68.2)
#endif
	NET_C(R137.2, U20.7, R145.1)
	NET_C(C62.2, GND)
	NET_C(U14.12, R125.1)
	NET_C(U14.11, R126.1)
	NET_C(U14.9, R128.1)
	NET_C(U14.6, R129.1)
	NET_C(U14.4, R127.1)
	NET_C(R127.2, R129.2, R128.2, R126.2, R125.2, C51.1)
	ALIAS(LASER_2, C51.2)

	//
	// Sheet 7, middle-top (REFILL)
	//

	NET_C(I_REFILL, U3.15, R59.1)
	NET_C(R59.2, I_V12, R17.2)

	// CD4011 oscillator: R18=2.7M, C3=0.1u, freq=2.664
	CLOCK(OSC1, 2.664)
	NET_C(OSC1.GND, GND)
	NET_C(OSC1.VCC, I_V12)
	NET_C(OSC1.Q, U3.14)
	NET_C(R18.1, R18.2, C3.1, C3.2, U9.1, U9.2, U9.5, U9.6, GND)

	NET_C(U3.3, R8.1)
	NET_C(U3.2, R7.1)
	NET_C(U3.4, R9.1)
	NET_C(U3.7, R12.1)
	NET_C(U3.10, R14.1)
	NET_C(U3.1, R6.1)
	NET_C(U3.5, R10.1)
	NET_C(U3.6, R11.1)
	NET_C(U3.9, R15.1)
	NET_C(U3.11, R13.1)
	NET_C(U3.13, GND)
	NET_C(R17.1, R8.2, R7.2, R9.2, R12.2, R14.2, R6.2, R10.2, R11.2, R15.2, R13.2, R16.2)
	NET_C(R16.1, Q7.B)

	// CD4011 oscillator: R46=470k, C20=0.1u, freq=15.307
	CLOCK(OSC2, 15.307)
	NET_C(OSC2.GND, GND)
	NET_C(OSC2.VCC, I_V12)
	NET_C(OSC2.Q, U4.4, U9.12, U9.13)
	NET_C(U9.11, U5.4)
	NET_C(R46.1, R46.2, C20.1, C20.2, U9.8, U9.9, GND)

	NET_C(U5.7, R67.1, R68.2)
	NET_C(R68.1, U5.2, U5.6, C26.2)
	NET_C(C26.1, GND, U5.1, C18.1, U4.1, Q7.C)
	NET_C(U5.5, U4.5, Q7.E)
	NET_C(R67.2, U5.8, R41.2, U4.8, I_V12)
	NET_C(R41.1, R42.2, U4.7)
	NET_C(R42.1, U4.2, U4.6, C18.2)
	NET_C(U5.3, R69.2)
	NET_C(U4.3, R43.2)
	NET_C(R43.1, C49.1, R69.1, I_REFILL_Q)
	ALIAS(REFILL, C49.2)

	//
	// Sheet 7, middle-bottom (ATTACK, RATE RESET)
	//

	NET_C(I_RATE_RESET, U15.15, R96.1)
	NET_C(R96.2, I_V12)
	NET_C(I_ATTACK_RATE, R148.1, U21.5)
	NET_C(R148.2, R166.2, I_V12)
	NET_C(U21.6, R166.1, C67.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_ATTACK_RATE)
	NET_C(D_ATTACK_RATE.A, U21.6)
	NET_C(D_ATTACK_RATE.K, I_V12)
#endif
	NET_C(C67.1, U21.3)
	NET_C(U21.4, U21.1, U21.2, U15.14)

	NET_C(U15.3, D23.A)
	NET_C(D23.K, R161.1)
	NET_C(U15.2, D22.A)
	NET_C(D22.K, R160.1)
	NET_C(U15.4, D21.A)
	NET_C(D21.K, R159.1)
	NET_C(U15.7, D17.A)
	NET_C(D17.K, R155.1)
	NET_C(U15.10, D15.A)
	NET_C(D15.K, R153.1)
	NET_C(U15.1, D18.A)
	NET_C(D18.K, R156.1)
	NET_C(U15.5, D20.A)
	NET_C(D20.K, R158.1)
	NET_C(U15.6, D19.A)
	NET_C(D19.K, R157.1)
	NET_C(U15.9, D16.A)
	NET_C(D16.K, R154.1)
	NET_C(U15.11, D14.A)
	NET_C(D14.K, R152.1)
	NET_C(U15.13, GND)
	NET_C(R161.2, R160.2, R159.2, R155.2, R153.2, R156.2, R158.2, R157.2, R154.2, R152.2, R178.2, U16.6)

	NET_C(I_WARP, R164.1)
	NET_C(R164.2, R162.1, R163.2, U16.5)
	NET_C(R162.2, I_V12)
	NET_C(R163.1, GND)
	NET_C(U16.7, R178.1)
	ALIAS(V, U16.7)

	//
	// Sheet 7, top-right (EXPLOSIONS)
	//

	NET_C(I_SHORT_EXPL, R169.1, U27.2)
	NET_C(R169.2, R185.2, I_V12)
	NET_C(U27.1, R185.1, C81.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_SHORT_EXPL)
	NET_C(D_SHORT_EXPL.A, U27.1)
	NET_C(D_SHORT_EXPL.K, I_V12)
#endif
	NET_C(C81.1, U27.11)
	NET_C(U27.13, U27.12, U27.3)
	NET_C(U27.3, R184.1)
	NET_C(R184.2, D31.A)
	NET_C(D31.K, D30.A, R182.2, R183.2)
	NET_C(R182.1, GND)
	NET_C(D30.K, R180.1, R181.1, C80.1, D29.K)
	NET_C(C80.2, GND)
	NET_C(R180.2, U16.9, R179.1)
	NET_C(R179.2, U16.8, R177.1)
	NET_C(R181.2, U16.10, Q6.D, D26.K)  // D and S swapped on schematics???
	NET_C(Q6.G, C72.2, R151.2, Q5.G)
	NET_C(R151.1, GND)
	NET_C(C72.1, U8.3)
	NET_C(U8.4, I_V12)
	NET_C(U8.2, I_VM12)
	NET_C(U8.1, GND)
	NET_C(Q6.S, D26.A, GND)
	NET_C(D29.A, D24.K, C69.1)
	NET_C(C69.2, GND)
	NET_C(R183.1, R186.2, U27.4, U27.8, U27.9, D28.K)
	NET_C(R186.1, C75.1)
	NET_C(C75.2, U21.8, U21.9, R168.2)
	NET_C(R168.1, U21.11)
	NET_C(I_LONG_EXPL, R171.1, U27.6)
	NET_C(R171.2, R170.2, I_V12)
	NET_C(R170.1, U27.5, C82.2)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_LONG_EXPL)
	NET_C(D_LONG_EXPL.A, U27.5)
	NET_C(D_LONG_EXPL.K, I_V12)
#endif
	NET_C(C82.1, U27.10)
	NET_C(U21.12, U21.13, C74.1, R165.2)
	NET_C(U21.10, C74.2, R167.1)
	NET_C(R165.1, GND)
	NET_C(R167.2, D24.A)
	NET_C(R177.2, R176.1, C78.1)
	NET_C(C78.2, U16.2, U16.1, C61.1)
	NET_C(R176.2, U16.3, C77.2, C76.2)
	ALIAS(EXPLOSIONS, C61.2)
	NET_C(C77.1, GND)
	NET_C(C76.1, D27.K, D28.A)
	NET_C(D27.A, GND)

	//
	// Sheet 7, middle-right (ASTROIDS)
	//

	NET_C(I_ASTROIDS, R97.1, R174.1, R175.1)
	NET_C(R97.2, I_V12)
	NET_C(R174.2, U16.13, R173.1)
	NET_C(R175.2, U16.12, Q5.D, D25.K)  // D and S swapped on schematics???
	NET_C(D25.A, Q5.S, GND)
	NET_C(R173.2, U16.14, R172.1)
	NET_C(R172.2, R150.1, C70.1)
	NET_C(C70.2, U7.9, U7.8, R136.1)
	NET_C(R150.2, C71.2, U7.10)
	NET_C(C71.1, GND)
	NET_C(R136.2, C59.2, C60.1)
	NET_C(C59.1, GND)
	ALIAS(ASTROIDS, C60.2)

	//
	// Sheet 7, bottom-right (mixer)
	//

	ALIAS(INVADER_1, R98.1)
	ALIAS(INVADER_2, R99.1)
	ALIAS(INVADER_3, R100.1)
	ALIAS(INVADER_4, R101.1)
	NET_C(R98.2, R99.2, R100.2, R101.2, C55.1)
	NET_C(ASTROIDS, R121.1)
	NET_C(EXPLOSIONS, R122.1)
	ALIAS(BONUS, R118.1)
	NET_C(REFILL, R119.1)
	NET_C(LASER_1, R102.1)
	NET_C(LASER_2, R103.1)
	NET_C(SONAR, R120.1)
	NET_C(C55.2, R121.2, R122.2, R118.2, R119.2, R102.2, R103.2, R120.2)
	AFUNC(MUTEFUNC, 2, "if(A0>2.5,0,A1)")
	NET_C(MUTEFUNC.A0, I_MUTE)
	NET_C(MUTEFUNC.A1, R121.2)
	ALIAS(OUTPUT, MUTEFUNC.Q)
//  ALIAS(OUTPUT, C55.2)

	//
	// Sheet 8, middle-top (INVADER_1)
	//

	NET_C(U22.10, GND)
	NET_C(U22.9, D5.A, R29.1)
	NET_C(R29.2, R55.2, U22.14, C21.2, R72.1)
	NET_C(D5.K, U22.8, R58.1)
	NET_C(R58.2, C22.1, R57.2, R56.2, C21.1)
	NET_C(W, U30.5)
	NET_C(U30.6, R57.1)
	NET_C(R56.1, GND)
	NET_C(U22.13, C22.2, R55.1)
	NET_C(U22.12, GND)

	NET_C(R85.1, U23.7, R86.2)
	NET_C(R85.2, U23.8, R83.2, R84.2, R73.2, U18.8, I_V12)
	NET_C(R86.1, U23.2, U23.6, C38.2)
	NET_C(C38.1, U23.1, C37.1, C28.1, U18.1, GND)
	NET_C(I_INVADER_1, U23.4, R73.1, U18.4)
	NET_C(C28.2, U18.5)
	NET_C(U23.3, U18.2)
	NET_C(U23.5, R87.2)
	NET_C(R87.1, V)
	NET_C(R83.1, D9.A)
	NET_C(D9.K, Q8.B, R72.2)
	NET_C(Q8.E, R84.1)
	NET_C(Q8.C, C37.2, U18.6, U18.7)
	NET_C(U18.3, INVADER_1)

	//
	// Sheet 8, middle (INVADER_2)
	//

	NET_C(R81.2, R70.2, U13.8, C27.1, I_V12)
	NET_C(C27.2, GND)
	NET_C(R81.1, R82.2, U13.7)
	NET_C(R82.1, U13.2, U13.6, C36.2)
	NET_C(C36.1, U13.1, GND)
	NET_C(U13.5, R71.2)
	NET_C(R71.1, V)
	NET_C(I_INVADER_2, U13.4, R70.1)
	NET_C(U13.3, U12.1)
	// CD4011 oscillator: R22=470k, C5=0.1u, freq=15.3Hz
	CLOCK(OSC3, 15.3)
	NET_C(OSC3.GND, GND)
	NET_C(OSC3.VCC, I_V12)
	NET_C(OSC3.Q, U12.2)
	NET_C(R22.1, R22.2, C5.1, C5.2, U11.8, U11.9, U11.12, U11.13, GND)
	NET_C(U12.12, R48.1)
	NET_C(U12.11, R49.1)
	NET_C(U12.9, R50.1)
	NET_C(U12.6, R47.1)
	NET_C(R47.2, R50.2, R49.2, R48.2)
	NET_C(R48.2, INVADER_2)

	//
	// Sheet 8, middle-bottom (INVADER_4)
	//

	NET_C(R89.2, U37.4, U37.8, C15.1, R60.2, R88.2, U38.8, I_V12)
	NET_C(R89.1, U37.7, R90.2)
	NET_C(R90.1, U37.2, U37.6, C39.2)
	NET_C(C39.1, U37.1, C23.1, U38.1, GND)
	NET_C(U37.5, R35.1)
	NET_C(R35.2, V, R34.1)
	NET_C(C15.2, GND)
	NET_C(U37.3, U28.1)
	NET_C(U28.2, U28.5)
	NET_C(U28.12, R31.1)
	NET_C(U28.11, R32.1)
	NET_C(U28.9, R33.1)
	NET_C(U28.6, R30.1)
	NET_C(R31.2, R32.2, R33.2, R30.2, R34.2, U38.5)
	NET_C(U38.2, U38.6, C23.2, R61.1)
	NET_C(R61.2, U38.7, R60.1)
	NET_C(U38.4, R88.1, I_INVADER_4)
	NET_C(INVADER_4, U38.3)

	//
	// Sheet 8, middle-bottom (BONUS)
	//

	NET_C(I_BONUS, R45.1, U10.6)
	NET_C(C34.1, R77.2, I_V12)
	NET_C(R45.2, R44.2, R74.2, U6.4, U6.8, C33.1, I_V12)

	NET_C(R44.1, U10.5, C19.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_BONUS)
	NET_C(D_BONUS.A, U10.5)
	NET_C(D_BONUS.K, I_V12)
#endif
	NET_C(U10.4, U10.1, U10.2)
	NET_C(C19.2, U10.3, D7.K)
	NET_C(D7.A, R78.1)
	NET_C(R78.2, R77.1, C34.2, R76.1)
	// CD4011 oscillator: R80=1M, C35=0.1, freq=7.19Hz
	CLOCK(OSC4, 7.19)
	NET_C(OSC4.GND, GND)
	NET_C(OSC4.VCC, I_V12)
	NET_C(OSC4.Q, R79.1)
	NET_C(R80.1, R80.2, C35.1, C35.2, U10.8, U10.9, U10.12, U10.13, GND)
	NET_C(R79.2, R74.1, R75.2, U6.7)
	NET_C(R75.1, U6.2, U6.6, C31.2)
	NET_C(C31.1, C32.1, U6.1, GND)
	NET_C(C32.2, U6.5)
	NET_C(R76.2, D8.K, C50.1)
	NET_C(U6.3, D8.A)
	NET_C(C33.2, GND)
	NET_C(BONUS, C50.2)

	//
	// Sheet 8, top-right (INVADER_3)
	//

	NET_C(U22.5, GND)
	NET_C(U22.6, D3.A, R25.1)
	NET_C(R25.2, C12.2, R24.2, U22.1, C8.2)
	NET_C(D3.K, U22.7, R26.1)
	NET_C(W, U30.9)
	NET_C(U30.8, R28.1)
	NET_C(R28.2, R26.2, C9.1, R27.2, C8.1)
	NET_C(R27.1, GND)
	NET_C(C9.2, U22.2, R24.1)
	NET_C(U22.3, GND)
	NET_C(C12.1, R54.1)
	NET_C(R54.2, R53.1, U17.5)
	NET_C(R53.2, V)
	NET_C(U17.1, C7.1, GND)
	NET_C(C7.2, U17.2, U17.6, R52.1)
	NET_C(R52.2, U17.7, R51.1)
	NET_C(R51.2, R23.2, U17.8, C6.1, I_V12)
	NET_C(R23.1, U17.4, I_INVADER_3)
	NET_C(C6.2, GND)
	NET_C(INVADER_3, U17.3)

	//
	// Unconnected inputs
	//

	NET_C(GND, U7.5, U7.6, U7.12, U7.13)
	NET_C(GND, U26.13)
	NET_C(GND, U29.1, U29.3, U29.13)
	NET_C(GND, U30.1, U30.11)

	//
	// Unconnected outputs
	//

#if (ENABLE_FRONTIERS)
#define RXX 192
	OPTIMIZE_FRONTIER(INVADER_1, RES_M(1), RXX)
	OPTIMIZE_FRONTIER(INVADER_2, RES_M(1), RXX)
	OPTIMIZE_FRONTIER(INVADER_3, RES_M(1), RXX)
	OPTIMIZE_FRONTIER(INVADER_4, RES_M(1), RXX)

	OPTIMIZE_FRONTIER(C50.1, RES_M(10), RXX)
	OPTIMIZE_FRONTIER(C60.1, RES_M(10), RXX)    // this is a big one
	OPTIMIZE_FRONTIER(C61.1, RES_M(10), RXX)
#endif

}
