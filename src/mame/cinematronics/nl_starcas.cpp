// license:CC0
// copyright-holders:Aaron Giles

//NL_CONTAINS starcas wotw

//
// Netlist for Star Castle/War of the Worlds
//
// Derived from the schematics and parts list in the Star Castle
// manual.
//
// War of the Worlds uses effectively the same sound board, but
// with a swizzled set of inputs.
//
// Special thanks to:
//    * Frank Palazzolo for creating a schematic of the War of the
//       Worlds signal mapping
//
// Known problems/issues:
//
//    * The VCOs require high solver frequencies (100x+) to reach the
//       correct pitches. For this reason, HLE'ed versions are
//       provided that work correctly even at 48kHz.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_BACKGROUND_VCO (1)
#define HLE_LASER_VCO (1)
#define ENABLE_FRONTIERS (1)


//
// Initial compilation includes this section.
//

#ifndef SOUND_VARIANT


//
// Now include ourselves twice, once for Star Castle and
// once for War of the Worlds
//

#define VARIANT_STARCASTLE  0
#define VARIANT_WOTW        1

#define SOUND_VARIANT       (VARIANT_STARCASTLE)
#include "nl_starcas.cpp"

#undef SOUND_VARIANT
#define SOUND_VARIANT       (VARIANT_WOTW)
#include "nl_starcas.cpp"


#else


//
// Main netlist
//

#if (SOUND_VARIANT == VARIANT_STARCASTLE)
NETLIST_START(starcas)
{
#else // (SOUND_VARIANT == VARIANT_WOTW)
NETLIST_START(wotw)
{
#endif

	// 192k is not high enough to make the laser and background pitches high enough
#if (HLE_BACKGROUND_VCO && HLE_LASER_VCO)
	SOLVER(Solver, 1000)
#else
	SOLVER(Solver, 4800000)
#endif
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	TTL_INPUT(I_OUT_0, 0)       // active low
	TTL_INPUT(I_OUT_1, 1)       // active low
	TTL_INPUT(I_OUT_2, 1)       // active low
	TTL_INPUT(I_OUT_3, 1)       // active low
	TTL_INPUT(I_OUT_4, 0)       // active low
	TTL_INPUT(I_OUT_7, 0)       // active low

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND, I_OUT_7.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC, I_OUT_7.VCC)

	CINEMAT_LOCAL_MODELS

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

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
//  RES(R120, RES_K(390)) -- part of final amp (not emulated)
//  RES(R121, RES_K(15))  -- part of final amp (not emulated)
//  RES(R122, 150)        -- part of final amp (not emulated)
//  RES(R123, RES_K(22))  -- part of final amp (not emulated)
//  RES(R124, 150)        -- part of final amp (not emulated)
	RES(R125, RES_K(8.2))
	RES(R126, RES_K(20))
	RES(R127, RES_K(30))
	POT(R128, RES_K(10))
	PARAM(R128.DIAL, 0.500000)

//  CAP(C2, CAP_U(25))      // electrolytic
//  CAP(C4, CAP_U(25))      // electrolytic
//  CAP(C5, CAP_U(25))      // electrolytic
//  CAP(C7, CAP_U(25))      // electrolytic
//  CAP(C9, CAP_U(25))      // electrolytic
	CAP(C11, CAP_U(0.68))   // film
	CAP(C12, CAP_U(0.001))  // disk
	CAP(C13, CAP_U(0.0022)) // film
	CAP(C14, CAP_U(0.1))    // film
	CAP(C15, CAP_U(0.1))    // film
	CAP(C16, CAP_U(0.1))    // disk*
	CAP(C17, CAP_U(100))    // electrolytic
	CAP(C18, CAP_U(0.1))    // film
	CAP(C19, CAP_U(0.1))    // disk*
	CAP(C20, CAP_U(0.1))    // film
	CAP(C21, CAP_U(0.01))   // disk
	CAP(C22, CAP_U(0.68))   // film
	CAP(C23, CAP_U(0.001))  // disk
	CAP(C24, CAP_U(0.0047)) // film
	CAP(C25, CAP_U(0.1))    // film
	CAP(C26, CAP_U(0.1))    // film
	CAP(C27, CAP_U(2.2))    // electrolytic
	CAP(C28, CAP_U(0.22))   // film
	CAP(C29, CAP_U(0.1))    // film
	CAP(C30, CAP_U(4.7))    // electrolytic
	CAP(C31, CAP_U(0.1))    // film
	CAP(C32, CAP_U(0.01))   // film
	CAP(C33, CAP_U(0.68))   // film
	CAP(C34, CAP_U(3.3))    // electrolytic
	CAP(C35, CAP_U(0.22))   // film
	CAP(C36, CAP_U(0.33))   // film
	CAP(C37, CAP_U(0.47))   // film
	CAP(C38, CAP_U(0.01))   // disk
	CAP(C39, CAP_U(0.68))   // film
	CAP(C40, CAP_U(0.1))    // film
	CAP(C41, CAP_U(0.01))   // disk
	CAP(C42, CAP_U(0.1))    // film
//  CAP(C43, CAP_U(0.68))   // film -- part of final amp (not emulated)
//  CAP(C44, CAP_P(470))    // disk -- part of final amp (not emulated)
//  CAP(C45, CAP_P(470))    // disk -- part of final amp (not emulated)
//  CAP(C46, CAP_P(470))    // disk -- part of final amp (not emulated)
//  CAP(C47, CAP_U(0.005))  // disk -- part of final amp (not emulated)
	CAP(C48, CAP_U(0.33))   // film

//  D_1N4003(D1)            // not needed
//  D_1N4003(D2)            // not needed
//  D_1N4003(D3)            // not needed
//  D_1N4003(D4)            // not needed
	D_1N5240B(D5)
	D_1N5236B(D6)
	D_1N914B(D7)
	D_1N914B(D8)
	D_1N914B(D9)
	D_1N914B(D10)

#if !(HLE_BACKGROUND_VCO)
	Q_2N3904(Q1)            // NPN
#endif
	Q_2N3904(Q2)            // NPN
	Q_2N3906(Q3)            // PNP
	Q_2N3904(Q4)            // NPN
#if !(HLE_LASER_VCO)
	Q_2N3904(Q5)            // NPN
#endif
	Q_2N3906(Q6)            // PNP
	Q_2N3906(Q7)            // PNP
	Q_2N3906(Q8)            // PNP
	Q_2N3906(Q9)            // PNP
	Q_2N3906(Q10)           // PNP
	Q_2N3906(Q11)           // PNP
	Q_2N3906(Q12)           // PNP
	Q_2N3906(Q13)           // PNP
	Q_2N3906(Q14)           // PNP
	Q_2N3906(Q15)           // PNP
	Q_2N3906(Q16)           // PNP
//  Q_2N6107(Q17)           // PNP -- part of final amp (not emulated)
//  Q_2N6292(Q18)           // NPN -- part of final amp (not emulated)

	TTL_7414_DIP(IC1)       // Hex Inverter
	NET_C(IC1.7, GND)
	NET_C(IC1.14, I_V5)

	TTL_74LS164_DIP(IC2)    // 8-bit Shift Reg.
	NET_C(IC2.7, GND)
	NET_C(IC2.14, I_V5)

	TTL_74LS377_DIP(IC3)    // Octal D Flip Flop
	NET_C(IC3.10, GND)
	NET_C(IC3.20, I_V5)

//  TTL_7815_DIP(IC4)       // +15V Regulator -- not needed
//  TTL_7915_DIP(IC5)       // -15V Regulator -- not needed

	TTL_7406_DIP(IC6)       // Hex Inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC6.7, GND)
	NET_C(IC6.14, I_V5)

	TL081_DIP(IC7)          // Op. Amp.
	NET_C(IC7.7, I_V15)
	NET_C(IC7.4, I_VM15)

	TL081_DIP(IC8)          // Op. Amp.
	NET_C(IC8.7, I_V15)
	NET_C(IC8.4, I_VM15)

#if (!HLE_BACKGROUND_VCO)
	LM566_DIP(IC9)          // 566 VCO
#endif

	TTL_74LS163_DIP(IC10)   // Binary Counter (schems say can sub a 74161)
	NET_C(IC10.8, GND)
	NET_C(IC10.16, I_V5)

	TTL_74LS163_DIP(IC11)   // Binary Counter (schems say can sub a 74161)
	NET_C(IC11.8, GND)
	NET_C(IC11.16, I_V5)

	TTL_74LS393_DIP(IC12)   // Dual 4 Bit B.C.
	NET_C(IC12.7, GND)
	NET_C(IC12.14, I_V5)

	TTL_74LS393_DIP(IC13)   // Dual 4 Bit B.C.
	NET_C(IC13.7, GND)
	NET_C(IC13.14, I_V5)

	AMI_S2688(IC14)         // Noise generator

	TL081_DIP(IC15)         // Op. Amp.
	NET_C(IC15.7, I_V15)
	NET_C(IC15.4, I_VM15)

	LM555_DIP(IC16)         // Timer

#if (!HLE_LASER_VCO)
	LM566_DIP(IC17)         // 566 VCO
#endif

	CA3080_DIP(IC18)        // Trnscndt. Op. Amp.
	NET_C(IC18.7, I_V15)
	NET_C(IC18.4, I_VM15)

	CA3080_DIP(IC19)        // Trnscndt. Op. Amp.
	NET_C(IC19.7, I_V15)
	NET_C(IC19.4, I_VM15)

	CA3080_DIP(IC20)        // Trnscndt. Op. Amp.
	NET_C(IC20.7, I_V15)
	NET_C(IC20.4, I_VM15)

	CA3080_DIP(IC21)        // Trnscndt. Op. Amp.
	NET_C(IC21.7, I_V15)
	NET_C(IC21.4, I_VM15)

	CA3080_DIP(IC22)        // Trnscndt. Op. Amp.
	NET_C(IC22.7, I_V15)
	NET_C(IC22.4, I_VM15)

	LM555_DIP(IC23)         // Timer

	LM555_DIP(IC24)         // Timer

//  TL081_DIP(IC25)         // Op. Amp. -- part of final amp (not emulated)
//  NET_C(IC25.7, I_V15)
//  NET_C(IC25.4, I_VM15)

	TL081_DIP(IC26)         // Op. Amp.
	NET_C(IC26.7, I_V15)
	NET_C(IC26.4, I_VM15)

	TL081_DIP(IC27)         // Op. Amp.
	NET_C(IC27.7, I_V15)
	NET_C(IC27.4, I_VM15)

	TTL_74LS107_DIP(IC28)   // Dual J-K Flip Flop
	NET_C(IC28.7, GND)
	NET_C(IC28.14, I_V5)

	//
	// Sheet 1, shift register (top left)
	//

	NET_C(I_V5, R1.1)
	ALIAS(HI, R1.2)

	NET_C(R1.2, IC2.9, IC2.2)
	NET_C(I_OUT_7, IC1.13)
	NET_C(IC1.12, IC1.1)
	NET_C(IC1.2, IC2.1)
	NET_C(I_OUT_4, IC1.9)
	NET_C(IC1.8, IC1.5)
	NET_C(IC1.6, IC2.8)
	NET_C(I_OUT_0, IC1.11)
	NET_C(IC1.10, IC1.3)
	NET_C(IC1.4, IC3.11)

	NET_C(IC2.3, IC3.3)
	NET_C(IC2.4, IC3.4)
	NET_C(IC2.5, IC3.7)
	NET_C(IC2.6, IC3.8)
	NET_C(IC2.10, IC3.13)
	NET_C(IC2.11, IC3.14)
	NET_C(IC2.12, IC3.17)
	NET_C(IC2.13, IC3.18)
	NET_C(GND, IC3.1)

#if (SOUND_VARIANT == VARIANT_STARCASTLE)

	ALIAS(FIREBALL_EN, IC3.2)
	ALIAS(SHIELD_EN, IC3.5)
	ALIAS(STAR_EN, IC3.6)
	ALIAS(THRUST_EN, IC3.9)
	ALIAS(BACKGROUND_EN, IC3.12)
	ALIAS(BL2, IC3.15)
	ALIAS(BL1, IC3.16)
	ALIAS(BL0, IC3.19)

#else // (SOUND_VARIANT == VARIANT_WOTW)

	ALIAS(BACKGROUND_EN, IC3.2)
	ALIAS(BL2, IC3.6)
	ALIAS(BL1, IC3.6)
	ALIAS(BL0, IC3.6)
	ALIAS(SHIELD_EN, IC3.9)
	ALIAS(FIREBALL_EN, IC3.12)
	RES(REXTRA, RES_K(1))
	NET_C(IC3.16, IC6.3)
	NET_C(IC6.4, REXTRA.1)
	NET_C(REXTRA.2, I_V5)
	ALIAS(STAR_EN, IC6.4)
	ALIAS(THRUST_EN, IC3.19)

#endif

	//
	// Sheet 1, BACKGROUND (top portion)
	//

	NET_C(GND, D5.A, D6.K, R13.1, R16.1, R17.1, IC7.3)
	NET_C(I_V15, R2.1)
	NET_C(I_VM15, R12.1, R18.1)
	NET_C(R2.2, D5.K, R3.1, R5.1, R7.1)
	NET_C(BL2, IC6.9)
	NET_C(IC6.8, R3.2, R4.1)
	NET_C(BL1, IC6.11)
	NET_C(IC6.10, R5.2, R6.1)
	NET_C(BL0, IC6.13)
	NET_C(IC6.12, R7.2, R8.1)
	NET_C(R4.2, R6.2, R8.2, IC7.2, R9.1)
	NET_C(IC7.6, R9.2, R10.1)
	NET_C(R10.2, IC8.2, R11.1, C11.1)
	NET_C(R12.2, R13.2, IC8.3)
	NET_C(IC8.6, C11.2, R11.2, R14.1)
	NET_C(R14.2, D6.A, R15.1)

#if (HLE_BACKGROUND_VCO)
	//
	// The background clock is a VCO controlled via a 566 timer.
	// Getting the frequency high enough to not miss clocking
	// the downstream dividers requires increasing the solver
	// frequency too much for realtime.
	//
	// Instead, clip out the circuit from the control voltage
	// coming into IC9 (pin 5), through the TTL converter, and
	// directly output the clock into IC10 pin 2. The equation
	// for the clock frequency is computed from running the
	// full emulation at 1000x frequency and fitting a curve
	// to the resulting dataset.
	//
	VARCLOCK(BGCLK, 1, "max(0.000001,min(0.1,(0.00000215073*A0*A0*A0*A0) + (0.0000224782*A0*A0*A0) + (0.000090697*A0*A0) + (0.000175878*A0) + 0.000163685))")
	NET_C(BGCLK.GND, GND)
	NET_C(BGCLK.VCC, I_V5)
	NET_C(R17.2, R15.2, R18.2, BGCLK.A0, C12.2)
	NET_C(C12.1, R16.2)
	ALIAS(CLK, BGCLK.Q)
	NET_C(C13.1, C13.2, C14.1, C14.2, R19.1, R19.2, R20.1, R20.2, R21.1, R21.2, D7.A, D7.K, D8.A, D8.K, GND)
#else
	NET_C(GND, IC9.8, D7.A, D8.A)
	NET_C(I_V5, R20.1)
	NET_C(I_VM15, IC9.1, C13.2, R21.1)
	NET_C(R17.2, R15.2, R18.2, IC9.5, C12.2)
	NET_C(C13.1, IC9.7)
	NET_C(C12.1, IC9.6, R16.2)
	NET_C(IC9.3, C14.1)
	NET_C(C14.2, D7.K, R19.1)
	NET_C(R19.2, Q1.B)
	NET_C(Q1.E, R21.2, D8.K)
	NET_C(Q1.C, R20.2)
	ALIAS(CLK, R20.2)
#endif

	//
	// Sheet 1, BACKGROUND (bottom portion)
	//

	NET_C(GND, R27.1, IC10.4, IC10.5, IC10.6, IC11.3, IC11.4, IC12.12, IC13.12, IC28.4)
	NET_C(I_V5, R22.1)
	NET_C(CLK, IC10.2, IC11.2, IC13.1, IC28.12)
	NET_C(BACKGROUND_EN, IC6.1, IC13.2)
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
	NET_C(I_V15, C16.1, C17.1, IC14.4)
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

	NET_C(GND, C22.2, C24.2, Q4.E, R38.1, R42.1, R50.1)
	NET_C(I_V5, R34.1, R36.1, Q3.E)
	NET_C(I_V15, R39.1, R43.1)
	NET_C(I_VM15, R37.1)
	NET_C(I_OUT_3, IC6.5, IC12.2)
	NET_C(IC6.6, R36.2, R35.1)
	NET_C(R35.2, Q3.B, R34.2)
	NET_C(Q3.C, R37.2, R38.2, R40.1)
	NET_C(R40.2, Q4.B)
	NET_C(R39.2, R41.1, C22.1, R42.2, C23.2)
	NET_C(Q4.C, R41.2)
	NET_C(C23.1, R43.2)

#if (HLE_LASER_VCO)
	//
	// The laser VCO is the same story as the background VCO,
	// requiring a large multiplier to the solver frequency
	// to clock downstream gates. Again, just replace it
	// with a VARCLOCK tuned based on output from running
	// the full simulation.
	//
	VARCLOCK(LASERCLK, 1, "max(0.000001,min(0.1,(0.00000385462*A0*A0*A0*A0) - (0.000195567*A0*A0*A0) + (0.00372371*A0*A0) - (0.0315254*A0) + 0.100119))")
	NET_C(LASERCLK.GND, GND)
	NET_C(LASERCLK.VCC, I_V5)
	NET_C(C23.2, LASERCLK.A0)
	NET_C(LASERCLK.Q, IC12.1)
	NET_C(GND, C24.1, C25.1, C25.2, D9.A, D9.K, D10.A, D10.K, R44.1, R44.2, R45.1, R45.2, R46.1, R46.2)
#else
	NET_C(GND, D9.A, D10.A, IC17.1)
	NET_C(I_V15, IC17.8)
	NET_C(C23.2, IC17.5)
	NET_C(C23.1, IC17.6)
	NET_C(C24.1, IC17.7)
	NET_C(IC17.3, C25.1)
	NET_C(C25.2, D9.K, R44.1)
	NET_C(R44.2, Q5.B)
	NET_C(I_VM15, R46.1)
	NET_C(Q5.E, D10.K, R46.2)
	NET_C(I_V5, R45.1)
	NET_C(Q5.C, R45.2, IC12.1)
#endif

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
	NET_C(I_VM15, C27.2, R54.1)
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
	NET_C(I_VM15, C30.2, R65.1)
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
	NET_C(I_VM15, C33.2, R77.1)
	NET_C(FIREBALL_EN, R74.2, R75.1)
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
	NET_C(I_VM15, R91.1)
	NET_C(SHIELD_EN, R85.2, R86.1)
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
	NET_C(I_VM15, C34.2, R102.1)
	NET_C(THRUST_EN, R99.2, R100.1)
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
	NET_C(STAR_EN, IC23.4, IC24.4)
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
	NET_C(IC18.6, IC19.6, IC20.6, IC21.6, IC22.6, R127.2, IC26.3)
	NET_C(IC26.2, IC26.6, C48.1)
	NET_C(C48.2, R125.1)
	NET_C(R125.2, SJ, IC27.2, R126.1)
	NET_C(R126.2, IC27.6, R128.3)

	//
	// Sheet 2, final amp
	//

	ALIAS(OUTPUT, R126.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC28.8, IC28.9, IC28.10, IC28.11)
#if (SOUND_VARIANT == VARIANT_STARCASTLE)
	NET_C(GND, IC6.3)
#endif

	//
	// Unconnected outputs
	//

	HINT(IC28.3, NC)
	HINT(IC28.5, NC)
	HINT(IC28.6, NC)

#if (SOUND_VARIANT == VARIANT_STARCASTLE)
	HINT(IC6.4, NC)
#endif
	HINT(IC10.11, NC)
	HINT(IC10.12, NC)
	HINT(IC10.13, NC)
	HINT(IC10.14, NC)
	HINT(IC11.11, NC)
	HINT(IC11.12, NC)
	HINT(IC11.13, NC)
	HINT(IC11.14, NC)
	HINT(IC12.5, NC)
	HINT(IC12.8, NC)
	HINT(IC12.9, NC)
	HINT(IC12.10, NC)
	HINT(IC13.3, NC)
	HINT(IC13.4, NC)
	HINT(IC13.5, NC)
	HINT(IC13.8, NC)
	HINT(IC13.10, NC)
	HINT(IC13.11, NC)

#if (ENABLE_FRONTIERS)
	//
	// Disconnect noise source from consumers
	//
	OPTIMIZE_FRONTIER(IC15.3, RES_M(1), 50)

	//
	// Split noise outputs from output outputs before the mixer
	//
	OPTIMIZE_FRONTIER(IC26.3, RES_M(1), 50)
#endif

}

#endif
