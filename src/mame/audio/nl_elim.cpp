// license:CC0
// copyright-holders:Aaron Giles

//NL_CONTAINS elim zektor

//
// Netlist for Eliminator/Zektor
//
// Derived from the schematics in the Eliminator and Zektor manuals.
//
// Known problems/issues:
//
//    * The noise signal (WBN) is connected into the TORPEDO_2 sound
//       but seems to make no audible difference, either in the
//       simulation or in recordings. Disabling it gives a nice speedup.
//
//    * Torpedo sounds don't retrigger until they're complete. This means
//       you can fire several times before the next firing sound triggers.
//       In recordings of games this doesn't appear to be the case.
//

#include "netlist/devices/net_lib.h"
#include "nl_elim.h"


//
// Optimizations
//

#define UNDERCLOCK_NOISE_GEN (1)
#define HLE_BACKGROUND_VCO (1)
#define HLE_TORPEDO1_VCO (1)
#define HLE_TORPEDO2_VCO (1)
#define HLE_SKITTER_VCO (1)
#define ENABLE_FRONTIERS (1)



//
// Hacks
//

#define ADD_CLIPPING_DIODES (1)
#define DISABLE_TORPEDO2_NOISE (1)



//
// Initial compilation includes this section.
//

#ifndef SOUND_VARIANT


#define D_1N914(name) DIODE(name, "1N914")
#define D_1N4002(name) DIODE(name, "1N4002")

#define Q_2N4403(name) QBJT_EB(name, "2N4403")

#define LM555_DIP NE555_DIP
#define LM566_DIP NE566_DIP

#define TTL_74LS00_DIP TTL_7400_DIP

//
// DIP mappings use the submodels below for CA3080
//
#define CA3080_DIP(name) SUBMODEL(_CA3080_FAST_DIP, name)
static NETLIST_START(_CA3080_FAST_DIP)
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
NETLIST_END()



//
// Now include ourselves twice, once for Eliminator and
// once for Zektor
//

#define VARIANT_ELIMINATOR  0
#define VARIANT_ZEKTOR      1

#define SOUND_VARIANT       (VARIANT_ELIMINATOR)
#include "nl_elim.cpp"

#undef SOUND_VARIANT
#define SOUND_VARIANT       (VARIANT_ZEKTOR)
#include "nl_elim.cpp"


#else


//
// Main netlist
//

#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
NETLIST_START(elim)
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
NETLIST_START(zektor)
#endif

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	// Overwrite model - the default model uses minimum datasheet
	// specifications for 5V. These are for 10V and thus closer to the
	// 12V used in this circuit.
	NET_MODEL("CD4XXX FAMILY(TYPE=CMOS IVL=0.3 IVH=0.7 OVL=0.05 OVH=0.05 ORL=384 ORH=384)")

	LOCAL_SOURCE(_CA3080_FAST_DIP)

//  TTL_INPUT(I_LO_D0, 0)
	TTL_INPUT(I_LO_D1, 0)
	ALIAS(I_FIREBALL, I_LO_D1)
	TTL_INPUT(I_LO_D2, 0)
	ALIAS(I_EXPLOSION_1, I_LO_D2)
	TTL_INPUT(I_LO_D3, 0)
	ALIAS(I_EXPLOSION_2, I_LO_D3)
	TTL_INPUT(I_LO_D4, 0)
	ALIAS(I_EXPLOSION_3, I_LO_D4)
	TTL_INPUT(I_LO_D5, 0)
	ALIAS(I_BOUNCE, I_LO_D5)
	TTL_INPUT(I_LO_D6, 0)
	ALIAS(I_TORPEDO_1, I_LO_D6)
	TTL_INPUT(I_LO_D7, 0)
	ALIAS(I_TORPEDO_2, I_LO_D7)

	NET_C(GND, I_LO_D1.GND, I_LO_D2.GND, I_LO_D3.GND, I_LO_D4.GND, I_LO_D5.GND, I_LO_D6.GND, I_LO_D7.GND)
	NET_C(I_V5, I_LO_D1.VCC, I_LO_D2.VCC, I_LO_D3.VCC, I_LO_D4.VCC, I_LO_D5.VCC, I_LO_D6.VCC, I_LO_D7.VCC)

	TTL_INPUT(I_HI_D0, 0)
	ALIAS(I_THRUST_LOW, I_HI_D0)
	TTL_INPUT(I_HI_D1, 0)
	ALIAS(I_THRUST_HI, I_HI_D1)
	TTL_INPUT(I_HI_D2, 0)
	ALIAS(I_THRUST_LSB, I_HI_D2)
	TTL_INPUT(I_HI_D3, 0)
	ALIAS(I_THRUST_MSB, I_HI_D3)
	TTL_INPUT(I_HI_D4, 0)
	ALIAS(I_SKITTER, I_HI_D4)
	TTL_INPUT(I_HI_D5, 0)
	ALIAS(I_ENEMY_SHIP, I_HI_D5)
	TTL_INPUT(I_HI_D6, 0)
	ALIAS(I_BACKGROUND_LSB, I_HI_D6)
	TTL_INPUT(I_HI_D7, 0)
	ALIAS(I_BACKGROUND_MSB, I_HI_D7)

	NET_C(GND, I_HI_D0.GND, I_HI_D1.GND, I_HI_D2.GND, I_HI_D3.GND, I_HI_D4.GND, I_HI_D5.GND, I_HI_D6.GND, I_HI_D7.GND)
	NET_C(I_V5, I_HI_D0.VCC, I_HI_D1.VCC, I_HI_D2.VCC, I_HI_D3.VCC, I_HI_D4.VCC, I_HI_D5.VCC, I_HI_D6.VCC, I_HI_D7.VCC)

	RES(R_PSG_1, 1000)
	RES(R_PSG_2, 1000)
	RES(R_PSG_3, 1000)
	NET_C(I_V5, R_PSG_1.1, R_PSG_2.1, R_PSG_3.1)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V12, 12)
	ANALOG_INPUT(I_VM12, -12)

	//
	// Part differences between Eliminator and Zektor
	//
	//  Ref Des         ELIMINATOR      ZEKTOR
	//  R5              10K             4.7K
	//  R9              33K             12K
	//  R71             270K            100K
	//  R79             2 MEG           unused
	//  R122            220K            390K
	//  R132            220K            100K
	//  C9              0.01uF          0.0047uF
	//  C46             0.022uF         0.047uF
	//

//  RES(R1, RES_K(100))     -- part of final amp (not emulated)
//  RES(R2, RES_M(1))       -- part of final amp (not emulated)
//  RES(R3, RES_K(22))      -- part of final amp (not emulated)
//  RES(R4, RES_K(2.2))     -- part of final amp (not emulated)
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
//  RES(R5, RES_K(10))      -- part of final amp (not emulated)
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
//  RES(R5, RES_K(4.7))     -- part of final amp (not emulated)
#endif
	RES(R6, RES_K(220))
	RES(R7, RES_K(220))
	RES(R8, RES_K(10))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	RES(R9, RES_K(33))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	RES(R9, RES_K(12))
#endif
	RES(R10, RES_K(10))
//  RES(R11, RES_K(2.2))    -- part of final amp (not emulated)
//  RES(R12, RES_M(1))      -- part of final amp (not emulated)
//  RES(R13, RES_K(330))    -- part of final amp (not emulated)
	RES(R14, RES_K(470))
	RES(R15, RES_K(100))
	RES(R16, RES_K(100))
	RES(R17, RES_K(10))
	RES(R18, RES_K(10))
	RES(R19, RES_K(1))
	RES(R20, RES_K(10))
	RES(R21, RES_K(100))
	RES(R22, RES_K(10))
	RES(R23, RES_K(4.7))
// R24??
	RES(R25, RES_K(220))
	RES(R26, RES_K(470))
	RES(R27, RES_K(470))
	RES(R28, RES_K(33))
	RES(R29, RES_K(68))
	RES(R30, RES_K(22))
	RES(R31, RES_K(1))
	RES(R32, RES_K(47))
	RES(R33, RES_K(82))
	RES(R34, RES_K(22))
	RES(R35, RES_K(1))
	RES(R36, RES_M(1.5))
	RES(R37, RES_K(6.8))
	RES(R38, RES_K(33))
	RES(R39, RES_K(22))
	RES(R40, RES_K(1))
	RES(R41, RES_M(2.2))
	RES(R42, RES_M(2.2))
	RES(R43, RES_K(1))
	RES(R44, RES_K(10))
	RES(R45, RES_K(47))
	RES(R46, RES_K(22))
	RES(R47, RES_K(2))
	RES(R48, RES_K(6.8))
	RES(R49, RES_K(33))
	RES(R50, RES_K(22))
	RES(R51, RES_K(1))
	RES(R52, RES_K(1))
	RES(R53, RES_K(1))
	RES(R54, RES_K(2))
	RES(R55, RES_K(1))
	RES(R56, RES_K(1))
	RES(R57, RES_K(1))
	RES(R58, RES_K(33))
	RES(R59, RES_K(2))
	RES(R60, RES_K(10))
	RES(R61, RES_K(47))
	RES(R62, RES_K(220))
	RES(R63, RES_K(1))
	RES(R64, RES_K(150))
	RES(R65, RES_K(390))
	RES(R66, RES_K(33))
	RES(R67, RES_K(100))
	RES(R68, RES_K(100))
	RES(R69, RES_K(1))
	RES(R70, RES_K(470))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	RES(R71, RES_K(270))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	RES(R71, RES_K(100))
#endif
	RES(R72, RES_K(1))
	RES(R73, RES_K(680))
	RES(R74, RES_K(390))
	RES(R75, RES_K(150))
	RES(R76, RES_K(10))
	RES(R77, RES_K(10))
	RES(R78, RES_K(10))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	RES(R79, RES_M(2))
#endif
	RES(R80, RES_K(1))
	RES(R81, RES_M(1.5))
	RES(R82, 470)
	RES(R83, 470)
	RES(R84, RES_K(2.2))
	RES(R85, RES_K(2.2))
	RES(R86, RES_K(2.2))
	RES(R87, RES_K(2.2))
	RES(R88, RES_K(2.2))
	RES(R89, RES_K(2.2))
	RES(R90, RES_K(15))
	RES(R91, RES_M(1))
	RES(R92, RES_K(22))
	RES(R93, RES_M(1))
	RES(R94, RES_K(10))
	RES(R95, RES_K(10))
	RES(R96, RES_K(22))
	RES(R97, RES_K(39))
	RES(R98, RES_K(82))
	RES(R99, RES_K(10))
	RES(R100, RES_K(10))
	RES(R101, 910)
	RES(R102, RES_K(6.8))
	RES(R103, RES_K(9.1))  // RES_K(2.2))
	RES(R104, RES_K(15))
	RES(R105, RES_K(10))
	RES(R106, RES_K(680))
	RES(R107, RES_K(4.7))
	RES(R108, RES_K(4.7))
	RES(R109, RES_K(2.2))
	RES(R110, RES_K(2.2))
	RES(R111, RES_K(100))
	RES(R112, RES_M(2.2))
	RES(R113, RES_M(1))
	RES(R114, RES_K(100))
	RES(R115, RES_M(1))
	RES(R116, RES_M(1))
	RES(R117, RES_M(1))
	RES(R118, RES_K(100))
	RES(R119, RES_K(100))
	RES(R120, RES_K(100))
	RES(R121, RES_M(2.2))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	RES(R122, RES_K(220))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	RES(R122, RES_K(390))
#endif
	RES(R123, RES_K(100))
	RES(R124, RES_M(2.2))
	RES(R125, RES_K(100))
	RES(R126, RES_K(10))
	RES(R127, RES_K(22))
	RES(R128, RES_K(39))
	RES(R129, RES_K(82))
	RES(R130, RES_K(22))
	RES(R131, RES_K(1))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	RES(R132, RES_K(220))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	RES(R132, RES_K(100))
#endif
	RES(R133, 680)
	RES(R134, RES_K(10))
	RES(R135, RES_K(10))
	RES(R136, RES_K(680))
	RES(R137, RES_K(10))
	RES(R138, RES_K(10))
	RES(R139, RES_K(330))
	RES(R140, RES_K(270))
	RES(R141, RES_K(220))
	RES(R142, RES_K(330))
	RES(R143, RES_K(680))
	RES(R144, RES_K(100))
	RES(R145, RES_K(68))
	RES(R146, RES_K(22))
	RES(R147, RES_K(2.2))

//  CAP(C1, CAP_U(4.7))     -- part of final amp (not emulated)
//  CAP(C2, CAP_U(0.1))
//  CAP(C3, CAP_U(0.1))
//  CAP(C4, CAP_U(10))      -- part of final amp (not emulated)
	CAP(C5, CAP_U(0.1))
	CAP(C6, CAP_U(0.1))
	CAP(C7, CAP_U(0.001))
	CAP(C8, CAP_U(10))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	CAP(C9, CAP_U(0.01))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	CAP(C9, CAP_U(0.0047))
#endif
	CAP(C10, CAP_U(0.068))
	CAP(C11, CAP_U(0.068))
	CAP(C12, CAP_U(0.1))
	CAP(C13, CAP_U(0.1))
	CAP(C14, CAP_U(0.1))
	CAP(C15, CAP_U(0.068))
	CAP(C16, CAP_U(0.068))
	CAP(C17, CAP_U(0.068))
	CAP(C18, CAP_U(0.068))
	CAP(C19, CAP_U(0.068))
	CAP(C20, CAP_U(0.068))
	CAP(C21, CAP_U(2.2))
//  CAP(C22, CAP_U(0.1))
//  CAP(C23, CAP_U(0.1))
//  CAP(C24, CAP_U(0.1))
//  CAP(C25, CAP_U(0.1))
//  CAP(C26, CAP_U(0.1))
//  CAP(C27, CAP_U(0.1))
	CAP(C28, CAP_U(0.1))
	CAP(C29, CAP_U(0.1))
	CAP(C30, CAP_U(0.022))
//  CAP(C31, CAP_U(0.1))
//  CAP(C32, CAP_U(0.1))
	CAP(C33, CAP_U(0.1))
	CAP(C34, CAP_U(0.1))
	CAP(C35, CAP_U(1))
	CAP(C36, CAP_U(2.2))
	CAP(C37, CAP_U(10))
	CAP(C38, CAP_U(4.7))
	CAP(C39, CAP_U(0.1))
//  CAP(C40, CAP_U(0.1))
//  CAP(C41, CAP_U(0.1))
// C42??
	CAP(C43, CAP_U(0.033))
	CAP(C44, CAP_U(0.1))
	CAP(C45, CAP_U(0.1))
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	CAP(C46, CAP_U(0.022))
#else // (SOUND_VARIANT == VARIANT_ZEKTOR)
	CAP(C46, CAP_U(0.047))
#endif
	CAP(C47, CAP_U(0.047))
	CAP(C48, CAP_U(0.05))
	CAP(C49, CAP_U(0.1))
	CAP(C50, CAP_U(0.05))
	CAP(C51, CAP_U(0.05))
	CAP(C52, CAP_U(0.05))
	CAP(C53, CAP_U(0.1))
	CAP(C54, CAP_U(0.1))
	CAP(C55, CAP_U(0.1))
	CAP(C56, CAP_U(33))
	CAP(C57, CAP_U(0.1))
	CAP(C58, CAP_U(0.1))
	CAP(C59, CAP_U(0.1))
	CAP(C60, CAP_U(0.001))
	CAP(C61, CAP_U(0.068))
//  CAP(C62, CAP_U(0.1))
	CAP(C63, CAP_P(100))
	CAP(C64, CAP_U(0.1))
	CAP(C65, CAP_U(0.01))
	CAP(C66, CAP_U(0.022))
	CAP(C67, CAP_U(0.047))
	CAP(C68, CAP_U(0.01))
	CAP(C69, CAP_U(0.1))
//  CAP(C70, CAP_U(0.1))
//  CAP(C71, CAP_U(0.1))
//  CAP(C72, CAP_U(0.1))
//  CAP(C73, CAP_U(10))
//  CAP(C74, CAP_U(0.1))
	CAP(C75, CAP_U(0.1))
	CAP(C76, CAP_U(0.1))
	CAP(C77, CAP_U(0.1))

	D_1N914(D1)
	D_1N914(D2)
	D_1N914(D3)
	D_1N914(D4)
	D_1N914(D5)
	D_1N914(D6)
	D_1N914(D7)
	D_1N4002(D8)

//  Q_2N4093(Q1)        -- part of final amp (not emulated)
	Q_2N4403(Q2)
	Q_2N4403(Q3)
	Q_2N4403(Q4)
	Q_2N4403(Q5)
	Q_2N4403(Q6)
	Q_2N4403(Q7)
	Q_2N4403(Q8)
	Q_2N4403(Q9)
	Q_2N4403(Q10)
	Q_2N4403(Q11)

	TL081_DIP(U1)           // Op. Amp.
	NET_C(U1.7, I_V12)
	NET_C(U1.4, I_VM12)

//  TL082_DIP(U2)           // Op. Amp. -- part of final amp (not emulated)
//  NET_C(U2.8, I_V12)
//  NET_C(U2.4, I_VM12)

	TL081_DIP(U3)           // Op. Amp.
	NET_C(U3.7, I_V12)
	NET_C(U3.4, I_VM12)

	MM5837_DIP(U4)          // Noise Generator
#if (UNDERCLOCK_NOISE_GEN)
	PARAM(U4.FREQ, 24000)
#endif

	LM555_DIP(U5)           // Timer

	CA3080_DIP(U6)          // Op. Amp.
	NET_C(U6.4, I_VM12)
	NET_C(U6.7, I_V12)

	CA3080_DIP(U7)          // Op. Amp.
	NET_C(U7.4, I_VM12)
	NET_C(U7.7, I_V12)

	CA3080_DIP(U8)          // Op. Amp.
	NET_C(U8.4, I_VM12)
	NET_C(U8.7, I_V12)

	CA3080_DIP(U9)          // Op. Amp.
	NET_C(U9.4, I_VM12)
	NET_C(U9.7, I_V12)

	CA3080_DIP(U10)         // Op. Amp.
	NET_C(U10.4, I_VM12)
	NET_C(U10.7, I_V12)

	CA3080_DIP(U11)         // Op. Amp.
	NET_C(U11.4, I_VM12)
	NET_C(U11.7, I_V12)

	TL081_DIP(U12)          // Op. Amp.
	NET_C(U12.7, I_V12)
	NET_C(U12.4, I_VM12)

	LM555_DIP(U13)          // Timer

	CA3080_DIP(U14)         // Op. Amp.
	NET_C(U14.4, I_VM12)
	NET_C(U14.7, I_V12)

	CA3080_DIP(U15)         // Op. Amp.
	NET_C(U15.4, I_VM12)
	NET_C(U15.7, I_V12)

	TL082_DIP(U16)          // Op. Amp.
	NET_C(U16.8, I_V12)
	NET_C(U16.4, I_VM12)

	CD4011_DIP(U17)         // Quad 2-Input NAND Gates
	NET_C(U17.7, GND)
	NET_C(U17.14, I_V12)

	CD4011_DIP(U18)         // Quad 2-Input NAND Gates
	NET_C(U18.7, GND)
	NET_C(U18.14, I_V12)

	CD4011_DIP(U19)         // Quad 2-Input NAND Gates
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V12)

	TL082_DIP(U20)          // Op. Amp.
	NET_C(U20.8, I_V12)
	NET_C(U20.4, I_VM12)

	CD4024_DIP(U21)         // 7-Stage Ripple Binary Counter
	NET_C(U21.7, GND)
	NET_C(U21.14, I_V12)

	CD4011_DIP(U22)         // Quad 2-Input NAND Gates
	NET_C(U22.7, GND)
	NET_C(U22.14, I_V12)

	CD4024_DIP(U23)         // 7-Stage Ripple Binary Counter
	NET_C(U23.7, GND)
	NET_C(U23.14, I_V12)

	TTL_7406_DIP(U24)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U24.7, GND)
	NET_C(U24.14, I_V5)

#if (!HLE_BACKGROUND_VCO)
	LM566_DIP(U25)          // Voltage-Controlled Oscillator
#endif

	TTL_74LS00_DIP(U26)     // Quad 4-Input NAND Gate
	NET_C(U26.7, GND)
	NET_C(U26.14, I_V5)

	TTL_7407_DIP(U27)       // Hex Buffers with High Votage Open-Collector Outputs
	NET_C(U27.7, GND)
	NET_C(U27.14, I_V5)

	TTL_7407_DIP(U28)       // Hex Buffers with High Votage Open-Collector Outputs
	NET_C(U28.7, GND)
	NET_C(U28.14, I_V5)

	TL081_DIP(U29)          // Op. Amp.
	NET_C(U29.7, I_V12)
	NET_C(U29.4, I_VM12)

//  AY_3_8912_DIP(U30)      // PSG -- emulated by MAME

	CD4069_DIP(U31)         // Hex Inverter
	NET_C(U31.7, GND)
	NET_C(U31.14, I_V12)

	CD4069_DIP(U32)         // Hex Inverter
	NET_C(U32.7, GND)
	NET_C(U32.14, I_V12)

//  TTL_74LS125_DIP(U33)    // Quad 3-state buffer
//  NET_C(U33.7, GND)
//  NET_C(U33.14, I_V5)

//  TTL_74LS374_DIP(U34)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U34.10, GND)
//  NET_C(U34.20, I_V5)
//
//  TTL_74LS374_DIP(U35)    // Octal D-Type Transparent Latches And Edge-Triggered Flip-Flop
//  NET_C(U34.10, GND)
//  NET_C(U34.20, I_V5)

//  TTL_74LS74_DIP(U36)     // Dual D Flip Flop
//  NET_C(U36.7, GND)
//  NET_C(U36.14, I_V5)

//  TTL_74LS10_DIP(U37)     // Triple 3-Input NAND Gate
//  NET_C(U37.7, GND)
//  NET_C(U37.14, I_V5)

//  TTL_74LS14_DIP(U38)
//  NET_C(U38.7, GND)
//  NET_C(U38.14, I_V5)

//  TTL_74LS08_DIP(U39)     // Quad 2-Input AND Gates
//  NET_C(U39.7, GND)
//  NET_C(U39.14, I_V5)

//  TTL_74LS30_DIP(U40)     // 8-Input NAND Gate
//  NET_C(U40.7, GND)
//  NET_C(U40.14, I_V5)

//  TTL_74LS14_DIP(U41)
//  NET_C(U41.7, GND)
//  NET_C(U41.14, I_V5)

	//
	// Sheet 7, top-left/middle (Thrust)
	//

	NET_C(R58.1, GND)
	NET_C(R58.2, U12.3, U10.6, U11.6, U15.6)
	ALIAS(SUM_VCA, R58.2)
	NET_C(U12.2, R75.1, R76.2)
	NET_C(R76.1, GND)
	NET_C(R75.2, U12.6)
	ALIAS(BUFFER, U12.6)

	NET_C(I_V12, U4.4, C75.2)
	NET_C(C75.1, GND)
	NET_C(U4.1, GND)
	NET_C(U4.2, I_VM12, C76.2)
	NET_C(C76.1, GND)
	NET_C(U4.3, C77.1)
	NET_C(C77.2, R27.2, U3.3)
	NET_C(R27.1, GND)
	NET_C(U3.2, U3.6, R28.1, R32.1)
	ALIAS(WBN, U3.6)

	NET_C(R28.2, C17.2, R29.1)
	NET_C(C17.1, GND)
	NET_C(R29.2, C18.2, R30.1)
	NET_C(C18.1, GND)
	NET_C(R30.2, R31.2, U10.2)
	NET_C(R31.1, GND)
	NET_C(U10.3, R56.2)
	NET_C(R56.1, GND)

	NET_C(I_THRUST_LOW, U24.5)
	NET_C(U24.6, R99.1, R70.1)
	NET_C(R99.2, I_V12)
	NET_C(R70.2, R71.2, Q4.E)
	NET_C(I_THRUST_HI, U24.3)
	NET_C(U24.4, R100.1, R71.1)
	NET_C(R100.2, I_V12)
	NET_C(Q4.B, GND)
	NET_C(Q4.C, R69.1)
	NET_C(R69.2, U10.5)

	NET_C(R32.2, C19.2, R33.1)
	NET_C(C19.1, GND)
	NET_C(R33.2, C20.2, R34.1)
	NET_C(C20.1, GND)
	NET_C(R34.2, R35.2, U11.2)
	NET_C(R35.1, GND)
	NET_C(U11.3, R57.2)
	NET_C(R57.1, GND)

	NET_C(I_THRUST_LSB, U24.1)
	NET_C(U24.2, R77.1, R73.1)
	NET_C(R77.2, I_V12)
	NET_C(R73.2, R74.2, Q5.E)
	NET_C(I_THRUST_MSB, U24.13)
	NET_C(U24.12, R78.1, R74.1)
	NET_C(R78.2, I_V12)
	NET_C(Q5.B, GND)
	NET_C(Q5.C, R72.1)
	NET_C(R72.2, U11.5)

	//
	// Sheet 7, top-right (Skitter)
	//

	NET_C(I_SKITTER, U24.9)
	NET_C(U24.8, R25.2, D1.K, R17.1)
	NET_C(R17.2, I_V12)
	NET_C(D1.A, C21.1, R26.2, R25.1, Q3.B)
	NET_C(C21.2, GND)
	NET_C(R26.1, GND)
	NET_C(Q3.C, GND, Q2.C)
	NET_C(Q3.E, R23.1)
	NET_C(R23.2, R22.2)
	NET_C(R22.1, Q2.E)

#if (HLE_SKITTER_VCO)
	//
	// U1 is basically a standalone oscillator. Simulation at high
	// frequency puts it at around 45Hz. Samples from real machines
	// show it to be about 35Hz.
	//
	CLOCK(SKITTERCLK, 45)
	NET_C(SKITTERCLK.GND, GND)
	NET_C(SKITTERCLK.VCC, I_V12)
	AFUNC(SKITTERENV, 1, "if(A0<6,2,18)")
	NET_C(SKITTERENV.A0, SKITTERCLK.Q)
	NET_C(SKITTERENV.Q, Q2.B)
	NET_C(GND, C5.1, C5.2, C47.1, C47.2, R14.1, R14.2, R15.1, R15.2, R16.1, R16.2, R19.1, R19.2, U1.2, U1.3)
#else
	NET_C(Q2.B, R19.1)
	NET_C(R19.2, C5.1)
	NET_C(C5.2, C47.2, U1.2, R16.1)
	NET_C(C47.1, GND)
	NET_C(U1.3, R15.1, R14.2)
	NET_C(R14.1, GND)
	NET_C(R15.2, U1.6, R16.2)
#endif

	NET_C(C9.2, R21.1)
	NET_C(C9.1, GND)
	NET_C(R21.2, R18.1)
	NET_C(R18.2, I_V12)
	NET_C(I_V12, C8.1)
	NET_C(C8.2, GND)
	NET_C(U5.7, R18.1)
	NET_C(U5.5, R23.2)
	NET_C(U5.4, U24.8)
	NET_C(U5.2, U5.6, R21.1)
	NET_C(U5.8, I_V12)
	NET_C(U5.1, GND)
	NET_C(U5.3, R20.1)
	NET_C(R20.2, C7.2, C6.1)
	NET_C(C7.1, GND)
	ALIAS(SKITTER, C6.2)

	//
	// Sheet 7, middle-right (Enemy Ship)
	//

	NET_C(I_ENEMY_SHIP, U24.11)
	NET_C(U24.10, R60.1, U13.4)
	NET_C(R60.2, I_V12)
	NET_C(U13.7, R59.1, R61.2)
	NET_C(R59.2, I_V12)
	NET_C(R61.1, U13.2, U13.6, C30.2)
	NET_C(C30.1, GND)
	NET_C(U13.5, R145.1, R144.1)
	ALIAS(_10HZ, R144.2)
	NET_C(U13.1, GND)
	NET_C(U13.8, I_V12, C29.2)
	NET_C(C29.1, GND)
	NET_C(U13.3, C28.1)
	ALIAS(ENEMY_SHIP, C28.2)

	ALIAS(_100HZ, R135.1)
	NET_C(R145.2, R135.1)
	ALIAS(_50HZ, R134.1)
	ALIAS(_200HZA, R137.1)
	ALIAS(_200HZB, R138.1)
	NET_C(R138.2, R137.2, R134.2, R135.2, C61.2, R136.1)
	NET_C(C61.1, GND)
	NET_C(R136.2, Q11.E, U27.6)
	NET_C(U27.5, U26.8)
	NET_C(Q11.B, GND)
	NET_C(Q11.C, R131.1)
	NET_C(R131.2, U15.5)
	NET_C(U15.2, R106.2, R83.2)
	NET_C(R83.1, GND)
	NET_C(U15.3, R82.2)
	NET_C(R82.1, GND)

	//
	// Sheet 7, bottom-right (Background)
	//

	NET_C(I_BACKGROUND_LSB, U27.1, U26.10)
	NET_C(U27.2, R107.1, R108.1)
	NET_C(R107.2, I_V12)
	NET_C(R108.2, R101.1, U16.6, R109.2)
	NET_C(I_BACKGROUND_MSB, U27.3, U26.9)
	NET_C(U27.4, R110.1, R109.1)
	NET_C(R110.2, I_V12)
	NET_C(U16.5, GND)
	NET_C(U16.7, R101.2, R104.1)
	NET_C(R104.2, U16.2, R105.1, C56.2)
	NET_C(U16.3, R102.2, R103.1)
	NET_C(R103.2, I_V12)
	NET_C(R102.1, GND)
	NET_C(U16.1, R105.2, C56.1, C60.1)
	NET_C(C60.2, R130.1)
	NET_C(R130.2, D8.K, C59.1)
	NET_C(D8.A, I_V12)
	NET_C(C59.2, GND)

#if (HLE_BACKGROUND_VCO)
	//
	//    R2 = 0.96310: HP = (0.000237231*A0) - 0.000561399
	//    R2 = 0.99617: HP = (0.0000527091*A0*A0) - (0.000501828*A0) + 0.00197538
	//    R2 = 0.99867: HP = (0.0000174335*A0*A0*A0) - (0.000309878*A0*A0) + (0.00198163*A0) - 0.00362162
	//    R2 = 0.99898: HP = (0.00000748879*A0*A0*A0*A0) - (0.000189174*A0*A0*A0) + (0.00180942*A0*A0) - (0.00759439*A0) + 0.0124560
	//    R2 = 0.99899: HP = (0.00000135301*A0*A0*A0*A0*A0) - (0.0000390321*A0*A0*A0*A0) + (0.000446417*A0*A0*A0) - (0.00250299*A0*A0) + (0.00693360*A0) - 0.00698302
	//
	VARCLOCK(BGCLK, 1, "max(0.000001,min(0.1,(0.00000748879*A0*A0*A0*A0) - (0.000189174*A0*A0*A0) + (0.00180942*A0*A0) - (0.00759439*A0) + 0.0124560))")
	NET_C(BGCLK.GND, GND)
	NET_C(BGCLK.VCC, I_V5)
	NET_C(BGCLK.A0, U16.1)
	NET_C(BGCLK.Q, BGENV.A0)
	NET_C(GND, C57.1, C57.2, C58.1, C58.2)
	AFUNC(BGENV, 1, "if(A0>2.5,2.5,-2.5)")
	NET_C(BGENV.Q, R106.1)
#else
	NET_C(U16.1, U25.5)
	NET_C(C60.2, U25.6)
	NET_C(R130.2, U25.8)
	NET_C(U25.7, C58.2)
	NET_C(C58.1, GND)
	NET_C(U25.1, GND)
	NET_C(U25.3, C57.1)
	NET_C(C57.2, R106.1)
#endif

	//
	// Sheet 7, PSG input
	//

	NET_C(R_PSG_1.2, R_PSG_2.2, R_PSG_3.2, R133.2, C64.1)
	NET_C(R133.1, GND)
	NET_C(C64.2, R132.1)
	NET_C(R132.2, U29.2, R122.1, C63.1)
	NET_C(U29.3, GND)
	NET_C(C63.2, R122.2, U29.6)
	ALIAS(PSG, U29.6)

	//
	// Sheet 8, oscillators
	//
	// Pairs of CD4069 are used together with resistors and capacitors
	// to create oscillators. The formula for the frequency is
	// 1/(1.39*R*C). The following are simulated here:
	//
	// _200HZA: R142=330K, C68=0.01uF,  Freq=218.01Hz
	// _200HZB: R139=330K, C65=0.01uF,  Freq=218.01Hz
	//   _10HZ: R143=680K, C69=0.1uF,   Freq=10.58Hz
	//   _50HZ: R141=220K, C67=0.047uF, Freq=69.58Hz
	//  _100HZ: R140=270K, C66=0.022uF, Freq=121.12Hz
	//

	CLOCK(_200HZACLK, 218.01)
	NET_C(_200HZACLK.VCC, I_V12)
	NET_C(_200HZACLK.GND, GND)
	NET_C(_200HZA, _200HZACLK.Q)
	NET_C(R142.1, R142.2, C68.1, C68.2, U32.5, U32.9, GND)
	HINT(U32.6, NC)
	HINT(U32.8, NC)

	CLOCK(_200HZBCLK, 217.98)   // tweak frequency so this is out of phase
	NET_C(_200HZBCLK.VCC, I_V12)
	NET_C(_200HZBCLK.GND, GND)
	NET_C(_200HZB, _200HZBCLK.Q)
	NET_C(R139.1, R139.2, C65.1, C65.2, U32.13, U32.11, GND)
	HINT(U32.12, NC)
	HINT(U32.10, NC)

	CLOCK(_10HZCLK, 10.58)
	NET_C(_10HZCLK.VCC, I_V12)
	NET_C(_10HZCLK.GND, GND)
	NET_C(_10HZ, _10HZCLK.Q)
	NET_C(R143.1, R143.2, C69.1, C69.2, U32.1, U32.3, GND)
	HINT(U32.2, NC)
	HINT(U32.4, NC)

	CLOCK(_50HZCLK, 69.58)
	NET_C(_50HZCLK.VCC, I_V12)
	NET_C(_50HZCLK.GND, GND)
	NET_C(_50HZ, _50HZCLK.Q)
	NET_C(R141.1, R141.2, C67.1, C67.2, U31.1, U31.3, GND)
	HINT(U31.2, NC)
	HINT(U31.4, NC)

	// Note this oscillator is on Sheet 7, in the right-middle section
	CLOCK(_100HZCLK, 121.12)
	NET_C(_100HZCLK.VCC, I_V12)
	NET_C(_100HZCLK.GND, GND)
	NET_C(_100HZ, _100HZCLK.Q)
	NET_C(R140.1, R140.2, C66.1, C66.2, U31.5, U31.9, GND)
	HINT(U31.6, NC)
	HINT(U31.8, NC)

	//
	// Sheet 8, top middle (Explosion 1)
	//

	NET_C(WBN, R44.1)
	NET_C(R44.2, C13.2, R45.1)
	NET_C(C13.1, GND)
	NET_C(R45.2, C14.2, R46.1)
	NET_C(C14.1, GND)
	NET_C(R46.2, R47.2, U8.2)
	NET_C(R47.1, GND)
	NET_C(U8.3, R54.2)
	NET_C(R54.1, GND)
	NET_C(U8.6, SUM_VCA)

	NET_C(I_EXPLOSION_1, U28.11)
	NET_C(U28.10, R120.1, U19.6)
	NET_C(R120.2, I_V12)
	NET_C(U19.5, R116.1, C51.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_EXPLOSION_1)
	NET_C(D_EXPLOSION_1.A, U19.5)
	NET_C(D_EXPLOSION_1.K, I_V12)
#endif
	NET_C(R116.2, I_V12)
	NET_C(C51.2, U19.3)
	NET_C(U19.4, U19.1, U19.2, R87.1)
	NET_C(R87.2, Q9.E)
	NET_C(Q9.B, GND)
	NET_C(Q9.C, C37.1, R66.1)
	NET_C(C37.2, I_VM12)
	NET_C(R66.2, U8.5)

	//
	// Sheet 8, top right (Explosion 2)
	//

	NET_C(WBN, R48.1)
	NET_C(R48.2, C15.2, R49.1)
	NET_C(C15.1, GND)
	NET_C(R49.2, C16.2, R50.1)
	NET_C(C16.1, GND)
	NET_C(R50.2, R51.2, U9.2)
	NET_C(R51.1, GND)
	NET_C(U9.3, R55.2)
	NET_C(R55.1, GND)
	NET_C(U9.6, SUM_VCA)

	NET_C(I_EXPLOSION_2, U28.9)
	NET_C(U28.8, R119.1, U19.8)
	NET_C(R119.2, I_V12)
	NET_C(U19.9, R117.1, C52.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_EXPLOSION_2)
	NET_C(D_EXPLOSION_2.A, U19.9)
	NET_C(D_EXPLOSION_2.K, I_V12)
#endif
	NET_C(R117.2, I_V12)
	NET_C(C52.2, U19.11)
	NET_C(U19.10, U19.12, U19.13, R88.1)
	NET_C(R88.2, Q10.E)
	NET_C(Q10.B, GND)
	NET_C(Q10.C, C38.1, R68.1)
	NET_C(C38.2, I_VM12)
	NET_C(R68.2, U9.5)

	//
	// Sheet 8, bottom left (Explosion 3)

	NET_C(WBN, R37.1)
	NET_C(R37.2, C10.2, R38.1)
	NET_C(C10.1, GND)
	NET_C(R38.2, C11.2, R39.1)
	NET_C(C11.1, GND)
	NET_C(R39.2, R40.2, U6.2)
	NET_C(R40.1, GND)
	NET_C(U6.3, R52.2)
	NET_C(R52.1, GND)
	NET_C(U6.6, SUM_VCA)

	NET_C(I_EXPLOSION_3, U28.5)
	NET_C(U28.6, R118.1, U18.8)
	NET_C(R118.2, I_V12)
	NET_C(U18.9, R115.1, C50.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_EXPLOSION_3)
	NET_C(D_EXPLOSION_3.A, U18.9)
	NET_C(D_EXPLOSION_3.K, I_V12)
#endif
	NET_C(R115.2, I_V12)
	NET_C(C50.2, U18.11)
	NET_C(U18.10, U18.12, U18.13, R86.1)
	NET_C(R86.2, Q8.E)
	NET_C(Q8.B, GND)
	NET_C(Q8.C, C36.1, R62.1)
	NET_C(C36.2, I_VM12)
	NET_C(R62.2, U6.5)

	//
	// Sheet 8, bottom left (Fireball)
	//

	NET_C(WBN, R41.1)
	NET_C(R41.2, R43.2, U7.2, R42.2)
	NET_C(R43.1, GND)
	NET_C(R42.1, C12.2)
	NET_C(C12.1, _50HZ)
	NET_C(U7.3, R53.2)
	NET_C(R53.1, GND)
	NET_C(U7.6, SUM_VCA)

	NET_C(I_FIREBALL, U28.13)
	NET_C(U28.12, R111.1, U18.6)
	NET_C(R111.2, I_V12)
	NET_C(U18.5, R112.1, C49.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_FIREBALL)
	NET_C(D_FIREBALL.A, U18.5)
	NET_C(D_FIREBALL.K, I_V12)
#endif
	NET_C(R112.2, I_V12)
	NET_C(C49.2, U18.3)
	NET_C(U18.4, U18.1, U18.2, R85.1)
	NET_C(R85.2, Q7.E)
	NET_C(Q7.B, GND)
	NET_C(Q7.C, R67.1)
	NET_C(R67.2, C35.1, R64.1)
	NET_C(C35.2, I_VM12)
	NET_C(R64.2, U7.5)

	//
	// Sheet 8, bottom-ish middle (Bounce)
	//

	NET_C(_50HZ, C45.1)
	NET_C(C45.2, R81.1)
	NET_C(R81.2, R80.2, U14.2)
#if (SOUND_VARIANT == VARIANT_ELIMINATOR)
	NET_C(WBN, R79.1)
	NET_C(R79.2, R80.2)
#endif
	NET_C(R80.1, GND)
	NET_C(U14.3, R63.2)
	NET_C(R63.1, GND)
	NET_C(U14.6, SUM_VCA)

	NET_C(I_BOUNCE, U28.3)
	NET_C(U28.4, R114.1, U17.8)
	NET_C(R114.2, I_V12)
	NET_C(U17.9, R113.1, C48.1)
	NET_C(R113.2, I_V12)
	NET_C(C48.2, U17.11)
	NET_C(U17.10, U17.13, U17.12, R84.1)
	NET_C(R84.2, Q6.E)
	NET_C(Q6.B, GND)
	NET_C(Q6.C, C34.1, C33.1, R65.1)
	NET_C(C33.2, C34.2, I_VM12)
	NET_C(R65.2, U14.5)

	//
	// Sheet 8, bottom middle (Torpedo 1)
	//

	NET_C(I_TORPEDO_1, U27.9)
	NET_C(U27.8, R125.1, U22.8)
	NET_C(R125.2, I_V12)
	NET_C(U22.9, R124.1, C55.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_TORPEDO_1)
	NET_C(D_TORPEDO_1.A, U22.9)
	NET_C(D_TORPEDO_1.K, I_V12)
#endif
	NET_C(R124.2, I_V12)
	NET_C(C55.2, U22.11, U23.2)
	NET_C(U22.10, U22.12, U22.13, D4.K)
	NET_C(D4.A, R89.1)
	NET_C(R89.2, C39.2, D2.K)
	NET_C(C39.1, GND)

#if (HLE_TORPEDO1_VCO)
	//
	//    R2 = 0.99044: HP = (0.000104804*A0) - 0.00000213808
	//    R2 = 0.99913: HP = (0.0000155103*A0*A0) + (0.0000595259*A0) + 0.0000223086
	//    R2 = 0.99930: HP = (0.00000342725*A0*A0*A0) + (0.000000183693*A0*A0) + (0.000079091*A0) + 0.0000157375
	//    R2 = 0.99930: HP = (-0.000000204983*A0*A0*A0*A0) + (0.00000466123*A0*A0*A0) - (0.00000232484*A0*A0) + (0.000081060*A0) + 0.0000152562
	//    R2 = 0.99931: HP = (-0.00000210641*A0*A0*A0*A0*A0) + (0.0000157326*A0*A0*A0*A0) - (0.0000399041*A0*A0*A0) + (0.0000541020*A0*A0) + (0.0000495068*A0) + 0.0000213321
	//
	VARCLOCK(TORP1CLK, 1, "max(0.000001,min(0.1,(0.00000342725*A0*A0*A0) + (0.000000183693*A0*A0) + (0.000079091*A0) + 0.0000157375))")
	NET_C(TORP1CLK.GND, GND)
	NET_C(TORP1CLK.VCC, I_V5)
	NET_C(TORP1CLK.A0, D2.K)
	NET_C(TORP1CLK.Q, T1ENV.A0)
	AFUNC(T1ENV, 1, "if(A0>2.5,11.1,-11.1)")
	NET_C(T1ENV.Q, U23.1)
	NET_C(D2.A, D3.K, R91.1)
	NET_C(C46.2, R90.1)
	NET_C(R90.2, R91.2, R146.1)
	NET_C(GND, U20.3, U20.2)
#else
	NET_C(D2.A, D3.K, R91.1, U20.3)
	NET_C(U20.2, C46.2, R90.1)
	NET_C(R90.2, U20.1, R91.2, R146.1)
#endif
	NET_C(D3.A, GND)
	NET_C(C46.1, GND)
	NET_C(R146.2, U23.1)

	NET_C(U23.12, R129.1)
	NET_C(U23.11, R128.1)
	NET_C(U23.9, R126.1)
	NET_C(U23.6, R127.1)
	NET_C(R127.2, R126.2, R128.2, R129.2, C44.1)

	//
	// Sheet 8, middle (Torpedo 2)
	//

	NET_C(WBN, R36.1)
#if (DISABLE_TORPEDO2_NOISE)
	NET_C(R36.2, GND)   // noise source
#else
	NET_C(R36.2, D6.A)  // noise source
#endif

#if (HLE_TORPEDO2_VCO)
	//
	//    R2 = 0.98868: HP = (0.000142472*A0) - 0.0000121481
	//    R2 = 0.99851: HP = (0.0000304790*A0*A0) + (0.000072730*A0) + 0.0000185913
	//    R2 = 0.99875: HP = (0.0000104419*A0*A0*A0) - (0.00000603211*A0*A0) + (0.000109865*A0) + 0.0000082421
	//    R2 = 0.99875: HP = (0.000000506238*A0*A0*A0*A0) + (0.0000080617*A0*A0*A0) - (0.00000220640*A0*A0) + (0.000107441*A0) + 0.0000087378
	//    R2 = 0.99876: HP = (-0.0000095532*A0*A0*A0*A0*A0) + (0.0000569271*A0*A0*A0*A0) - (0.000116200*A0*A0*A0) + (0.000123414*A0*A0) + (0.0000501789*A0) + 0.0000180007
	//
	VARCLOCK(TORP2CLK, 1, "max(0.000001,min(0.1,(0.0000104419*A0*A0*A0) - (0.00000603211*A0*A0) + (0.000109865*A0) + 0.0000082421))")
	NET_C(TORP2CLK.GND, GND)
	NET_C(TORP2CLK.VCC, I_V5)
	NET_C(TORP2CLK.A0, D6.K)
	NET_C(TORP2CLK.Q, T2ENV.A0)
	AFUNC(T2ENV, 1, "if(A0>2.5,11.1,-11.1)")
	NET_C(T2ENV.Q, U21.1)
	NET_C(D6.A, D7.K, R93.1)
	NET_C(C43.2, R94.1)
	NET_C(R94.2, R93.2, R92.2)
	NET_C(GND, U20.6, U20.5)
#else
	NET_C(D6.A, U20.5, D7.K, R93.1)
	NET_C(U20.6, C43.2, R94.1)
	NET_C(R94.2, U20.7, R93.2, R92.2)
#endif

	NET_C(D7.A, GND)
	NET_C(D6.K, C53.2, R147.2)
	NET_C(C53.1, GND)
	NET_C(R147.1, D5.A)
	NET_C(U22.4, U22.1, U22.2, D5.K)
	NET_C(C43.1, GND)
	NET_C(R92.1, U21.1)

	NET_C(I_TORPEDO_2, U27.11)
	NET_C(U27.10, R123.1, U22.6)
	NET_C(R123.2, I_V12)
	NET_C(U22.5, R121.1, C54.1)
#if (ADD_CLIPPING_DIODES)
	// fast retriggering relies on clipping diodes which
	// aren't implemented by default for speed
	D_1N914(D_TORPEDO_2)
	NET_C(D_TORPEDO_2.A, U22.5)
	NET_C(D_TORPEDO_2.K, I_V12)
#endif
	NET_C(R121.2, I_V12)
	NET_C(C54.2, U22.3, U21.2)
	NET_C(U21.12, R98.1)
	NET_C(U21.11, R97.1)
	NET_C(U21.9, R95.1)
	NET_C(U21.6, R96.1)
	NET_C(R98.2, R97.2, R95.2, R96.2, C44.1)

	//
	// Sheet 8, middle right (Final sum)
	//

	NET_C(PSG, R10.1)
	NET_C(ENEMY_SHIP, R7.1)
	NET_C(SKITTER, R6.1)
	NET_C(BUFFER, R8.1)
	NET_C(C44.2, R9.1)
	NET_C(R10.2, R7.2, R6.2, R8.2, R9.2)
	ALIAS(OUTPUT, R9.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, U17.1, U17.2, U17.5, U17.6)
	NET_C(GND, U26.1, U26.2, U26.4, U26.5, U26.12, U26.13)
	NET_C(GND, U27.13, U28.1)
	NET_C(GND, U31.11, U31.13)

	//
	// Unconnected outputs
	//


#if (ENABLE_FRONTIERS)
#define RXX 384
	OPTIMIZE_FRONTIER(R10.1, RES_K(10), RXX)
	OPTIMIZE_FRONTIER(R7.1, RES_K(220), RXX)
	OPTIMIZE_FRONTIER(R6.1, RES_K(220), RXX)
	OPTIMIZE_FRONTIER(R8.1, RES_K(10), RXX)
#endif

NETLIST_END()

#endif
