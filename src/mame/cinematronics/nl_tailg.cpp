// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Tail Gunner
//
// Derived from the schematics and parts list in the
// Tail Gunner manual.
//
// Known problems/issues:
//
//    * The diodes (D8/D9) connected to LS123 Rext/Cext pins
//       cause major performance issues. Snipping them out
//       seems to have no effect apart from giving back all
//       the lost performance.
//
//    * The "wideband noise generator" is simulated with a generic
//       noise source. This seems to work fine and doesn't sound
//       too different from recordings.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_NOISE_GEN (1)
#define HLE_HYPERSPACE_VCO (1)
#define HLE_LASER_VCO (1)
#define REMOVE_LS123_DIODES (1)
#define ADD_LS125_PULLUPS (1)


//
// Main netlist
//

NETLIST_START(tailg)
{

#if (HLE_HYPERSPACE_VCO && HLE_LASER_VCO)
	SOLVER(Solver, 1000)
#else
	SOLVER(Solver, 48000000)
#endif
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	TTL_INPUT(I_OUT_0, 0)               // active high
	TTL_INPUT(I_OUT_1, 0)               // active high
	TTL_INPUT(I_OUT_2, 0)               // active high
	TTL_INPUT(I_OUT_3, 0)               // active high
	TTL_INPUT(I_OUT_4, 0)               // active high

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC)

	CINEMAT_LOCAL_MODELS

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

	RES(R1, RES_M(1))
	RES(R2, RES_K(470))
	RES(R3, 470)
	RES(R4, RES_K(470))
	RES(R5, 100)
	RES(R6, RES_K(10))
	RES(R7, RES_M(1))
	RES(R8, RES_K(10))
	RES(R9, RES_K(47))
	RES(R10, RES_K(1))
	RES(R11, RES_K(330))
	RES(R12, RES_K(9.1))
	RES(R13, RES_M(1))
	RES(R14, RES_K(20))
	RES(R15, RES_K(10))
	RES(R16, RES_K(1))
	RES(R17, RES_K(10))
	RES(R18, RES_K(20))
	RES(R19, RES_K(39))
	RES(R20, RES_K(82))
	RES(R21, RES_K(2.7))
	RES(R22, RES_K(1))
	RES(R23, RES_K(10))
	RES(R24, RES_K(47))
	RES(R25, RES_K(330))
	RES(R26, RES_K(10))
	RES(R27, RES_K(2.7))
	RES(R28, RES_K(2.7))
	RES(R29, 150)
	RES(R30, RES_K(10))
	RES(R31, RES_M(1))
	RES(R32, RES_K(20))
	RES(R33, RES_K(10))
	RES(R34, RES_K(1))
	RES(R35, RES_K(10))
	RES(R36, RES_K(20))
	RES(R37, RES_K(39))
	RES(R38, RES_K(82))
	RES(R39, RES_K(100))
	RES(R40, RES_K(68))
	RES(R41, 330)
	RES(R42, 330)
	RES(R43, RES_K(560))
	RES(R44, RES_K(30))
	RES(R45, RES_K(1))
	RES(R46, 330)
	RES(R47, RES_K(220))
	RES(R48, RES_K(2.7))
	RES(R49, RES_K(2.7))
	RES(R50, RES_K(820))
	RES(R51, RES_K(360))
	RES(R52, RES_K(330))
	RES(R53, RES_K(150))
	RES(R54, RES_K(4.7))
	RES(R55, RES_K(390))
	RES(R56, RES_K(910))
	RES(R57, RES_K(4.7))
	RES(R58, RES_K(2))
	RES(R59, RES_K(10))
	RES(R60, RES_K(1))
	RES(R61, RES_K(4.7))
	RES(R62, RES_K(20))
	RES(R63, RES_K(180))
	RES(R64, RES_K(10))
	RES(R65, RES_K(20))
	RES(R66, RES_K(15))
	RES(R67, RES_K(10))
	RES(R68, RES_K(20))
	RES(R69, RES_K(30))
	RES(R70, 470)
//  RES(R71, 150)        -- part of final amp (not emulated)
//  RES(R72, RES_K(22))  -- part of final amp (not emulated)
//  RES(R73, 150)        -- part of final amp (not emulated)
//  RES(R74, RES_K(47))  -- part of final amp (not emulated)
//  POT(R75, RES_K(100)) -- part of final amp (not emulated)
//  PARAM(R75.DIAL, 0.5) -- part of final amp (not emulated)
	RES(R76, RES_K(47))
	RES(R77, RES_K(47))
	RES(R78, RES_K(2.7))
	RES(R79, RES_K(2.7))
	RES(R80, RES_K(100))
	RES(R81, RES_K(1))
	RES(R82, RES_K(330))
	RES(R83, RES_K(10))
	RES(R84, RES_K(91))

	CAP(C1, CAP_U(100))
	CAP(C2, CAP_U(0.1))
	CAP(C3, CAP_U(0.01))
	CAP(C4, CAP_U(0.1))
	CAP(C5, CAP_U(100))
	CAP(C6, CAP_U(0.68))
	CAP(C7, CAP_U(0.1))
	CAP(C8, CAP_U(1))
	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.005))
	CAP(C12, CAP_U(0.1))
	CAP(C13, CAP_U(0.1))
	CAP(C14, CAP_U(0.05))
	CAP(C15, CAP_U(10))
	CAP(C16, CAP_U(10))
	CAP(C17, CAP_U(0.002))
	CAP(C18, CAP_U(0.01))
	CAP(C19, CAP_U(0.01))
	CAP(C20, CAP_U(0.01))
	CAP(C21, CAP_U(0.1))
	CAP(C22, CAP_U(0.1))
	CAP(C23, CAP_U(0.01))
	CAP(C24, CAP_U(15))
	CAP(C25, CAP_U(0.22))
	CAP(C26, CAP_U(4.7))
	CAP(C27, CAP_U(0.1))
	CAP(C28, CAP_U(0.47))
	CAP(C29, CAP_U(0.47))
	CAP(C30, CAP_U(0.01))
	CAP(C31, CAP_U(0.05))
	CAP(C32, CAP_U(0.1))
	CAP(C33, CAP_U(0.1))
//  CAP(C34, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C35, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C36, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C37, CAP_U(3.3))
//  CAP(C38, CAP_U(3.3))
//  CAP(C39, CAP_U(3.3))
//  CAP(C40, CAP_U(3.3))
	CAP(C41, CAP_U(0.005))
	CAP(C42, CAP_U(0.1))
	CAP(C43, CAP_U(10))
	CAP(C44, CAP_U(2.2))

	D_1N5240(D1)
	D_1N914(D2)
	D_1N914(D3)
	D_1N914(D4)
	D_1N914(D5)
	D_1N914(D6)
	D_1N914(D7)
	D_1N914(D8)
	D_1N914(D9)

//  Q_2N3904(Q1)            // NPN -- not used
//  Q_2N3904(Q2)            // NPN -- not used
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3906(Q5)            // PNP
//  Q_2N6292(Q6)            // NPN -- part of final amp (not emulated)
//  Q_2N6107(Q7)            // PNP -- part of final amp (not emulated)
	Q_2N3906(Q8)            // PNP

	TL081_DIP(IC1)          // Op. Amp.
//  NET_C(IC1.7, I_V15)     // (indirectly via R5)
	NET_C(IC1.4, I_VM15)

	TL081_DIP(IC2)          // Op. Amp.
	NET_C(IC2.4, I_VM15)
	NET_C(IC2.7, I_V15)

	TL081_DIP(IC3)          // Op. Amp.
	NET_C(IC3.4, I_VM15)
	NET_C(IC3.7, I_V15)

	TTL_74LS125_DIP(IC4)    // Quad 3-state Buffers
	NET_C(IC4.7, GND)
	NET_C(IC4.14, I_V5)

	TTL_7404_DIP(IC5)       // Hex Inverting Gates
	NET_C(IC5.7, GND)
	NET_C(IC5.14, I_V5)

	TTL_7406_DIP(IC6)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC6.7, GND)
	NET_C(IC6.14, I_V5)

	TL081_DIP(IC7)          // Op. Amp.
	NET_C(IC7.4, I_VM15)
	NET_C(IC7.7, I_V15)

	TTL_74LS393_DIP(IC8)    // Dual 4-Stage Binary Counter
	NET_C(IC8.7, GND)
	NET_C(IC8.14, I_V5)

	TL081_DIP(IC9)          // Op. Amp.
	NET_C(IC9.4, I_VM15)
	NET_C(IC9.7, I_V15)

	CA3080_DIP(IC10)        // Op. Amp.
	NET_C(IC10.4, I_VM15)
	NET_C(IC10.7, I_V15)

	TL081_DIP(IC11)         // Op. Amp.
	NET_C(IC11.4, I_VM15)
	NET_C(IC11.7, I_V15)

	TTL_74LS123_DIP(IC12)   // Retriggerable Monostable Multivibrators
	NET_C(IC12.8, GND)
	NET_C(IC12.16, I_V5)

	CA3080_DIP(IC13)        // Op. Amp.
	NET_C(IC13.4, I_VM15)
	NET_C(IC13.7, I_V15)

	LM555_DIP(IC14)

	LM555_DIP(IC15)

	LM555_DIP(IC16)

	LM555_DIP(IC17)

	TTL_74LS393_DIP(IC18)   // Dual 4-Stage Binary Counter
	NET_C(IC18.7, GND)
	NET_C(IC18.14, I_V5)

	TL081_DIP(IC19)         // Op. Amp.
	NET_C(IC19.4, I_VM15)
	NET_C(IC19.7, I_V15)

//  TTL_7915_DIP(IC20)      // -15V Regulator -- not needed
//  TTL_7815_DIP(IC21)      // +15V Regulator -- not needed

	CA3080_DIP(IC22)        // Op. Amp.
	NET_C(IC22.4, I_VM15)
	NET_C(IC22.7, I_V15)

	TTL_74LS259_DIP(IC23)
	NET_C(IC23.8, GND)
	NET_C(IC23.16, I_V5)

#if (HLE_NOISE_GEN)
	//
	// The "wideband noise gen" relies on properties
	// of the components to create noise. Not only
	// does this simulate poorly, but it would be too
	// slow for realtime, so HLE it with some quality
	// noise.
	//
	// Note that Sundance has the exact same circuit.
	//
	CLOCK(NOISE_CLOCK, 10000)
	NET_C(NOISE_CLOCK.GND, GND)
	NET_C(NOISE_CLOCK.VCC, I_V5)

	SYS_NOISE_MT_U(NOISE, 3)
	NET_C(NOISE.I, NOISE_CLOCK.Q)
	NET_C(NOISE.1, GND)
	NET_C(NOISE.2, R9.2, R23.2, R55.2, R76.1)

	NET_C(GND, C1.1, C1.2, C2.1, C2.2, C3.1, C3.2, C4.1, C4.2, C5.1, C5.2)
	NET_C(GND, D1.A, D1.K, D2.A, D2.K, D3.A, D3.K)
	NET_C(GND, R1.1, R1.2, R2.1, R2.2, R3.1, R3.2, R4.1, R4.2, R5.1, R5.2, R6.1, R6.2, R7.1, R7.2, R8.1, R8.2, R9.1)
	NET_C(GND, IC1.2, IC1.3, IC1.7, IC2.2, IC2.3, IC3.2, IC3.3)
#else
	NET_C(C1.1, GND)
	NET_C(C1.2, C2.2, D1.K, IC1.7, R5.1)
	NET_C(C2.1, GND)
	NET_C(D1.A, C3.1, R1.2)
	NET_C(R1.1, GND)
	NET_C(C3.2, R2.2, IC1.2)
	NET_C(R3.1, GND)
	NET_C(R2.1, C4.1, R3.2)
	NET_C(C4.2, IC1.3, R4.1)
	NET_C(R4.2, IC1.6, R6.1)
	NET_C(R5.2, C5.1, I_V15)
	NET_C(C5.2, GND)

	NET_C(R6.2, IC2.2, R7.1, D3.A, D2.K)
	NET_C(IC2.3, GND)
	NET_C(IC2.6, R7.2, D3.K, D2.A, R8.1)
	NET_C(R8.2, IC3.2, R9.1)
	NET_C(IC3.3, GND)
	NET_C(IC3.6, R9.2, R23.2, R55.2, R76.1)
#endif

	//
	// Input mux
	//

	NET_C(I_OUT_0, IC23.1)
	NET_C(I_OUT_1, IC23.2)
	NET_C(I_OUT_2, IC23.3)
	NET_C(I_OUT_3, IC23.13)
	NET_C(I_OUT_4, IC23.14)
	NET_C(IC23.15, I_V5)

	NET_C(R23.1, R24.1, C9.2)
	NET_C(C9.1, GND)
	NET_C(R24.2, C10.1)
	NET_C(C10.2, IC9.2, R25.1, C11.1)
	NET_C(IC9.3, GND)
	NET_C(IC9.6, R25.2, C11.2, R26.1)
	NET_C(R26.2, C12.1)
	NET_C(C12.2, R27.2, IC10.2)
	NET_C(R27.1, GND)
	NET_C(IC10.3, R28.2)
	NET_C(R28.1, GND)
	NET_C(IC10.6, R70.1)

	//
	// Explosion
	//

	NET_C(IC23.4, IC12.1)
	NET_C(IC12.14, C15.2)

#if (REMOVE_LS123_DIODES)
	//
	// The diodes connected to the Rext/Cext pins on the
	// LS123 (monostable multivibrators) absolutely tank
	// performance for reasons yet not understood. Their
	// purpose is unclear, and removing them seems to have
	// no effect on the sound, while fixing the performance
	// so we'll just snip them out for now.
	//
	NET_C(IC12.15, C15.1, R39.1)
	NET_C(GND, D8.A, D8.K)
#else
	NET_C(IC12.15, D8.K)
	NET_C(D8.A, C15.1, R39.1)
#endif

	NET_C(R39.2, IC12.2, IC12.3, I_V5)
	NET_C(IC12.13, Q3.E)
	NET_C(Q3.B, R42.2)
	NET_C(R42.1, GND)
	NET_C(Q3.C, R43.2, C16.1, R44.1)
	NET_C(C16.2, I_VM15)
	NET_C(R44.2, IC10.5)

	//
	// Rumble
	//

	NET_C(IC23.5, IC6.5)
	NET_C(IC6.6, R40.1, Q4.E)
	NET_C(R40.2, I_V5)
	NET_C(Q4.B, R41.2)
	NET_C(R41.1, GND)
	NET_C(Q4.C, R43.1)

	//
	// Shield
	//

	NET_C(IC23.7, IC4.13)
	NET_C(I_V5, R45.1)
	NET_C(R45.2, IC4.12)
	NET_C(IC4.11, Q5.E)
	NET_C(Q5.B, R46.2)
	NET_C(R46.1, GND)
	NET_C(Q5.C, R47.1)
	NET_C(R47.2, IC13.5)
	NET_C(IC13.6, R70.1)

	NET_C(I_V15, R54.2, IC16.4, IC16.8)
	NET_C(R54.1, IC16.7, R56.2)
	NET_C(R56.1, IC16.6, IC16.2, C22.2)
	NET_C(C22.1, IC16.1, C23.1, GND)
	NET_C(C23.2, IC16.5)
	NET_C(IC16.3, R57.1)
	NET_C(R57.2, C24.1, R58.1)
	NET_C(C24.2, GND)
	NET_C(R58.2, C26.1, C25.1)
	NET_C(C26.2, GND)
	NET_C(C25.2, C27.1, R48.2, IC13.2, C21.1)
	NET_C(R48.1, GND)
	NET_C(IC13.3, R49.2)
	NET_C(R49.1, GND)
	NET_C(C27.2, R59.1)
	NET_C(R59.2, C28.1, R60.1)
	NET_C(C28.2, GND)
	NET_C(R60.2, C29.1, R62.1)
	NET_C(C29.2, GND)
	NET_C(R62.2, IC17.3)
	NET_C(IC17.7, R61.2, R63.2)
	NET_C(R61.1, IC17.4, IC17.8, I_V15)
	NET_C(R63.1, IC17.6, IC17.2, C31.2)
	NET_C(C31.1, IC17.1, C30.1, GND)
	NET_C(C30.2, IC17.5)
	NET_C(C21.2, R55.1)

	//
	// Hyperspace
	//

	NET_C(IC23.10, IC4.1)
	NET_C(IC4.2, GND)

#if (ADD_LS125_PULLUPS)
	//
	// The 74LS125 is being abuse here, assuming the tristate
	// will pull high instead of low. This is not how it is
	// modelled, so add a pullup resistor between the output
	// and the TTL source.
	//
	RES(RTEMP1, RES_K(1))
	NET_C(RTEMP1.1, I_V5)
	NET_C(IC4.3, RTEMP1.2)
#endif

	NET_C(IC4.3, IC5.5, IC8.12)
	NET_C(IC5.6, IC6.1)
	NET_C(IC6.2, R10.1, R11.1)
	NET_C(R10.2, I_V5)
	NET_C(R11.2, D4.K, C6.1)
	NET_C(C6.2, GND)

#if (HLE_HYPERSPACE_VCO)
	//
	// The hyperspace VCO is troublesome to emulate without
	// cranking up the solver frequency, so model it instead.
	// Take the voltage at C6.1 and map it to the TTL
	// frequency at IC8.13, then remove the circuit in favor
	// of a variable clock.
	//
	// Here is the mapping I get for C6.1 vs IC8.13 half-period:
	//    R2 = 0.99919: HP = (0.000070760*A0) + 0.0000496821
	//    R2 = 0.99991: HP = (0.00000141986*A0*A0) + (0.0000592676*A0) + 0.000067917
	//    R2 = 0.99995: HP = (0.000000176200*A0*A0*A0) - (0.000000394224*A0*A0) + (0.000064351*A0) + 0.000063955
	//    R2 = 0.99995: HP = (-0.00000000225454*A0*A0*A0*A0) + (0.000000207009*A0*A0*A0) - (0.000000529599*A0*A0) + (0.000064557*A0) + 0.000063885
	//    R2 = 0.99995: HP = (0.0000000099503*A0*A0*A0*A0*A0) - (0.000000173841*A0*A0*A0*A0) + (0.00000127300*A0*A0*A0) - (0.00000339043*A0*A0) + (0.000067595*A0) + 0.000063196
	//
	// However, when we clip the circuit, the C6.1 values change,
	// so here is a mapping for the clipped C6.1 vs. the original
	// IC8.13 half-period:
	//
	//    R2 = 0.97319: HP = (0.0000572507*A0) + 0.000194746
	//    R2 = 0.97692: HP = (0.00000283778*A0*A0) + (0.0000473318*A0) + 0.000197383
	//    R2 = 0.97785: HP = (-0.00000116165*A0*A0*A0) + (0.0000100018*A0*A0) + (0.0000372162*A0) + 0.000199308
	//    R2 = 0.97830: HP = (-0.000000661779*A0*A0*A0*A0) + (0.00000465155*A0*A0*A0) - (0.00000545641*A0*A0) + (0.0000499499*A0) + 0.000197353
	//    R2 = 0.97930: HP = (-0.000000814292*A0*A0*A0*A0*A0) + (0.0000086074*A0*A0*A0*A0) - (0.0000317523*A0*A0*A0) + (0.0000522027*A0*A0) + (0.0000173329*A0) + 0.000201604
	//
	VARCLOCK(HYPERCLK, 1, "max(0.000001,min(0.1,(-0.000000814292*A0*A0*A0*A0*A0) + (0.0000086074*A0*A0*A0*A0) - (0.0000317523*A0*A0*A0) + (0.0000522027*A0*A0) + (0.0000173329*A0) + 0.000201604))")
	NET_C(HYPERCLK.GND, GND)
	NET_C(HYPERCLK.VCC, I_V5)
	NET_C(HYPERCLK.Q, IC8.13)
	NET_C(HYPERCLK.A0, C6.1)
	NET_C(GND, R12.1, R12.2, R13.1, R13.2, R14.1, R14.2, R15.1, R15.2, R16.1, R16.2, C7.1, C7.2, D4.A, D5.A, D5.K, IC7.2, IC7.3, IC5.1)
	HINT(IC5.2, NC)
#else
	NET_C(D4.A, IC7.3, D5.K, R13.1)
	NET_C(IC7.2, C7.2, R12.1)
	NET_C(C7.1, GND)
	NET_C(R12.2, IC7.6, R13.2, R14.1)
	NET_C(D5.A, GND)
	NET_C(R14.2, R15.2, Q1.B)
	NET_C(R15.1, GND)
	NET_C(Q1.E, GND)
	NET_C(Q1.C, R16.1, IC5.1)
	NET_C(R16.2, I_V5)
	NET_C(IC5.2, IC8.13)
#endif

	NET_C(IC8.9, R17.1)
	NET_C(IC8.8, R18.1)
	NET_C(IC8.10, R19.1)
	NET_C(IC8.11, R20.1)
	NET_C(R17.2, R18.2, R19.2, R20.2, R22.2, R21.1)
	NET_C(R22.1, GND)
	NET_C(R21.2, C8.1)
	NET_C(C8.2, R70.1)

	//
	// Laser
	//

	NET_C(IC23.6, IC4.4)
	NET_C(IC4.5, GND)

#if (ADD_LS125_PULLUPS)
	//
	// see previous comment about why this is necessary
	//
	RES(RTEMP2, RES_K(1))
	NET_C(RTEMP2.1, I_V5)
	NET_C(IC4.6, RTEMP2.2)
#endif

	NET_C(IC4.6, IC8.2, IC6.3)
	NET_C(IC6.4, R29.1)
	NET_C(R29.2, D6.K, C13.2)
	NET_C(C13.1, GND)

#if (HLE_LASER_VCO)
	//
	// The laser VCO is almost identical to the hyperspace VCO,
	// apart from the component values. The same approach is used
	// for HLE.
	//
	// Here is the mapping I get for C13.2 vs IC8.1 half-period:
	//    R2 = 0.99489: HP = (0.0000346649*A0) + 0.0000391143
	//    R2 = 0.99489: HP = (0.0000000539278*A0*A0) + (0.0000345904*A0) + 0.0000391217
	//    R2 = 0.99491: HP = (0.00000116177*A0*A0*A0) - (0.00000273810*A0*A0) + (0.0000362549*A0) + 0.0000389782
	//    R2 = 0.99502: HP = (-0.00000728180*A0*A0*A0*A0) + (0.0000256184*A0*A0*A0) - (0.0000288539*A0*A0) + (0.0000456890*A0) + 0.0000382619
	//    R2 = 0.99526: HP = (0.0000241755*A0*A0*A0*A0*A0) - (0.000110184*A0*A0*A0*A0) + (0.000181339*A0*A0*A0) - (0.000128682*A0*A0) + (0.000070619*A0) + 0.0000366068
	//
	// And here is the mapping for the clipped C13.2 vs the original
	// IC8.1 half-period:
	//    R2 = 0.10697: HP = (0.000449852*A0) - 0.000000764127
	//    R2 = 0.99164: HP = (0.0445612*A0*A0) - (0.00441596*A0) + 0.0000386659
	//    R2 = 0.99165: HP = (0.0576325*A0*A0*A0) + (0.0325274*A0*A0) - (0.00378669*A0) + 0.0000384468
	//    R2 = 0.99422: HP = (-37.92188*A0*A0*A0*A0) + (11.77137*A0*A0*A0) - (1.173779*A0*A0) + (0.0377702*A0) + 0.0000242577
	//    R2 = 0.99536: HP = (-1435.864*A0*A0*A0*A0*A0) + (418.8404*A0*A0*A0*A0) - (37.19112*A0*A0*A0) + (0.653446*A0*A0) + (0.0314777*A0) + 0.0000261637
	//
	VARCLOCK(LASERCLK, 1, "max(0.000001,min(0.1,(0.0576325*A0*A0*A0) + (0.0325274*A0*A0) - (0.00378669*A0) + 0.0000384468))")
	NET_C(LASERCLK.GND, GND)
	NET_C(LASERCLK.VCC, I_V5)
	NET_C(LASERCLK.Q, IC8.1)
	NET_C(LASERCLK.A0, C13.2)
	NET_C(GND, R30.1, R30.2, R31.1, R31.2, R32.1, R32.2, R33.1, R33.2, R34.1, R34.2, C14.1, C14.2, D6.A, D7.A, D7.K, IC11.2, IC11.3, IC5.3)
	HINT(IC5.4, NC)
#else
	NET_C(D6.A, IC11.3, D7.K, R31.1)
	NET_C(D7.A, GND)
	NET_C(IC11.2, C14.2, R30.1)
	NET_C(C14.1, GND)
	NET_C(IC11.6, R30.2, R31.2, R32.1)
	NET_C(R32.2, R33.2, Q2.B)
	NET_C(R33.1, GND)
	NET_C(Q2.E, GND)
	NET_C(Q2.C, R34.1, IC5.3)
	NET_C(R34.2, I_V5)
	NET_C(IC5.4, IC8.1)
#endif

	NET_C(IC8.5, R35.1)
	NET_C(IC8.6, R36.1)
	NET_C(IC8.4, R37.1)
	NET_C(IC8.3, R38.1)
	NET_C(R35.2, R36.2, R37.2, R38.2, R21.1)

	//
	// Shield Bounce
	//

	NET_C(R50.2, I_V5, IC14.4, IC14.8)
	NET_C(R50.1, R51.1, IC14.6, IC14.2, C17.2)
	NET_C(R51.2, IC14.7)
	NET_C(C17.1, IC14.1, C18.1, GND)
	NET_C(C18.2, IC14.5)
	NET_C(IC14.3, IC18.1)
	NET_C(IC18.2, GND)
	NET_C(IC18.4, R64.2)
	NET_C(R64.1, R65.1, C32.2)
	NET_C(IC18.6, R65.2)
	NET_C(C32.1, R66.2)
	NET_C(R66.1, R69.1, IC22.2, R78.2, C42.2)

	NET_C(R52.2, I_V5, IC15.4, IC15.8)
	NET_C(R52.1, R53.1, IC15.6, IC15.2, C19.2)
	NET_C(R53.2, IC15.7)
	NET_C(C19.1, IC15.1, C20.1, GND)
	NET_C(C20.2, IC15.5)
	NET_C(IC15.3, IC18.13)
	NET_C(IC18.12, GND)
	NET_C(IC18.8, R67.2)
	NET_C(R67.1, R68.1, C33.2)
	NET_C(IC18.11, R68.2)
	NET_C(C33.1, R69.2)

	NET_C(C42.1, R77.2, C41.2)
	NET_C(R77.1, GND)
	NET_C(C41.1, R76.2)

	NET_C(R78.1, GND)
	NET_C(IC22.3, R79.2)
	NET_C(R79.1, GND)

	NET_C(IC23.9, IC12.9)
	NET_C(IC12.6, C43.2)

#if (REMOVE_LS123_DIODES)
	//
	// A second instance of problematic diode; see above
	// comment for an explanation.
	//
	NET_C(C43.1, R80.1, IC12.7)
	NET_C(GND, D9.A, D9.K)
#else
	NET_C(C43.1, D9.A, R80.1)
	NET_C(D9.K, IC12.7)
#endif

	NET_C(R80.2, IC12.10, IC12.11, I_V5)

	NET_C(IC12.12, IC4.10)
	NET_C(IC4.9, R81.2)
	NET_C(R81.1, I_V5)
	NET_C(IC4.8, Q8.E)
	NET_C(Q8.B, R82.2)
	NET_C(R82.1, GND)
	NET_C(Q8.C, R83.1)
	NET_C(R83.2, C44.1, R84.1)
	NET_C(C44.2, I_VM15)
	NET_C(R84.2, IC22.5)
	NET_C(IC22.6, R70.1)

	//
	// Final amp
	//

//NET_C(R70.1, GND) // temp
	ALIAS(OUTPUT, R70.1)
	NET_C(R70.2, GND)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC19.2, IC19.3)  // part of final amp
	NET_C(GND, IC5.9, IC5.11, IC5.13, IC6.9, IC6.11, IC6.13)

	//
	// Unconnected outputs
	//

	HINT(IC5.8, NC)     // QC
	HINT(IC5.10, NC)    // QD
	HINT(IC5.12, NC)    // QE
	HINT(IC6.8, NC)     // QC
	HINT(IC6.10, NC)    // QD
	HINT(IC6.12, NC)    // QE
	HINT(IC12.4, NC)    // /QA
	HINT(IC12.5, NC)    // QB
	HINT(IC18.3, NC)    // Q0A
	HINT(IC18.5, NC)    // Q2A
	HINT(IC18.9, NC)    // Q2B
	HINT(IC18.10, NC)   // Q1B
	HINT(IC23.11, NC)   // Q6
	HINT(IC23.12, NC)   // Q7

}
