// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Warrior
//
// Derived from the schematics in the Warrior manual.
//
// Known problems/issues:
//
//    * Not yet tested.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_NOISE_GEN (1)
#define HLE_PITFALL_VCO (1)



//
// Main netlist
//

NETLIST_START(warrior)

#if (HLE_PITFALL_VCO)
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

	RES(R1, RES_K(10))
	RES(R2, RES_K(47))
	RES(R3, RES_K(330))
	RES(R4, RES_K(10))
	RES(R5, RES_K(2.7))
	RES(R6, RES_K(2.7))
	RES(R7, RES_K(4.7))
	RES(R8, RES_K(8.2))
	RES(R9, RES_K(2.7))
	RES(R10, RES_K(2.7))
	RES(R11, 470)
	RES(R12, RES_K(470))
	RES(R13, 330)
	RES(R14, RES_K(30))
	RES(R15, 100)
	RES(R16, RES_M(1))
	RES(R17, RES_M(1))
	RES(R18, RES_K(470))
	RES(R19, 470)
	RES(R20, RES_K(470))
	RES(R21, RES_K(10))
	RES(R22, RES_K(10))
	RES(R23, RES_K(47))
	RES(R24, RES_K(1))
	RES(R25, RES_K(510))
	RES(R26, 330)
	RES(R27, 330)
	RES(R28, RES_K(120))
	RES(R29, RES_K(82))
	RES(R30, 330)
	RES(R31, RES_K(4.7))
	RES(R32, RES_K(910))
	RES(R33, RES_K(4.7))
	RES(R34, RES_K(2))
	RES(R35, RES_K(390))
	RES(R36, RES_K(4.7))
	RES(R37, RES_K(180))
	RES(R38, RES_K(20))
	RES(R39, RES_K(1))
	RES(R40, RES_K(10))
	RES(R41, RES_K(2.7))
	RES(R42, RES_K(2.7))
	RES(R43, RES_K(20))
	RES(R44, 150)
	RES(R45, RES_K(30))
	RES(R46, RES_K(360))
	RES(R47, RES_K(12))
	RES(R48, RES_M(1))
	RES(R49, RES_K(30))
	RES(R50, RES_K(10))
	RES(R51, RES_K(1))
	RES(R52, RES_K(4.7))
	RES(R53, RES_K(680))
	RES(R54, RES_K(30))
	RES(R55, RES_K(2.7))
	RES(R56, RES_K(2.7))
	RES(R57, RES_K(820))
	RES(R58, 470)
//  RES(R59, 150)        -- part of final amp (not emulated)
//  RES(R60, RES_K(2.2)) -- part of final amp (not emulated)
//  RES(R61, 150)        -- part of final amp (not emulated)
//  RES(R62, RES_K(47))  -- part of final amp (not emulated)
//  POT(R63, RES_K(100)) -- part of final amp (not emulated)

	CAP(C1, CAP_U(0.1))
	CAP(C2, CAP_U(0.1))
	CAP(C3, CAP_U(0.01))
	CAP(C4, CAP_U(0.1))
	CAP(C5, CAP_U(0.005))
	CAP(C6, CAP_U(0.1))
	CAP(C7, CAP_U(1))
	CAP(C8, CAP_U(15))
	CAP(C9, CAP_U(100))
	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.01))
	CAP(C12, CAP_U(0.1))
	CAP(C13, CAP_U(100))
	CAP(C14, CAP_U(0.33))
	CAP(C15, CAP_U(0.68))
	CAP(C16, CAP_U(0.01))
	CAP(C17, CAP_U(15))
	CAP(C18, CAP_U(4.7))
	CAP(C19, CAP_U(0.22))   // 22?
	CAP(C20, CAP_U(0.1))
	CAP(C21, CAP_U(0.1))
//  CAP(C22, CAP_U(3.3)) -- part of voltage converter (not emulated)
//  CAP(C23, CAP_U(3.3)) -- part of voltage converter (not emulated)
//  CAP(C24, CAP_U(3.3)) -- part of voltage converter (not emulated)
//  CAP(C25, CAP_U(3.3)) -- part of voltage converter (not emulated)
//  CAP(C26, CAP_U(3.3)) -- part of voltage converter (not emulated)
	CAP(C27, CAP_U(0.047))
	CAP(C28, CAP_U(0.01))
	CAP(C29, CAP_U(0.47))
	CAP(C30, CAP_U(0.47))
	CAP(C31, CAP_U(1))
	CAP(C32, CAP_U(0.1))
	CAP(C33, CAP_U(0.47))
	CAP(C34, CAP_U(0.05))
	CAP(C35, CAP_U(0.05))
	CAP(C36, CAP_U(0.01))
	CAP(C37, CAP_U(0.1))
//  CAP(C38, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C39, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C40, CAP_P(470)) -- part of final amp (not emulated)

	D_1N5240(D1)
	D_1N914(D2)
	D_1N914(D3)
	D_1N914(D4)
	D_1N914(D5)

	Q_2N3906(Q1)            // PNP
	Q_2N3906(Q2)            // PNP
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
#if !(HLE_PITFALL_VCO)
	Q_2N3904(Q5)            // NPN
#endif
//  Q_2N5878(Q6)            // NPN -- part of final amp (not emulated)
//  Q_2N5876(Q7)            // PNP -- part of final amp (not emulated)

	TL081_DIP(IC1)          // Op. Amp.
	NET_C(IC1.7, I_V15)
	NET_C(IC1.4, I_VM15)

	CA3080_DIP(IC2)         // Op. Amp.
	NET_C(IC2.4, I_VM15)
	NET_C(IC2.7, I_V15)

	CA3080_DIP(IC3)         // Op. Amp.
	NET_C(IC3.4, I_VM15)
	NET_C(IC3.7, I_V15)

	LM555_DIP(IC4)

	TL081_DIP(IC5)          // Op. Amp.
//  NET_C(IC5.7, I_V15)     // (indirectly via R15)
	NET_C(IC5.4, I_VM15)

	TL081_DIP(IC6)          // Op. Amp.
	NET_C(IC6.4, I_VM15)
	NET_C(IC6.7, I_V15)

	TL081_DIP(IC7)          // Op. Amp.
	NET_C(IC7.4, I_VM15)
	NET_C(IC7.7, I_V15)

	TTL_74LS125_DIP(IC8)    // Quad 3-state Buffers
	NET_C(IC8.7, GND)
	NET_C(IC8.14, I_V5)

	LM555_DIP(IC9)

//  TTL_7815_DIP(IC10)      // +15V Regulator -- not emulated
//  TTL_7915_DIP(IC11)      // -15V Regulator -- not emulated

	LM555_DIP(IC12)

	CA3080_DIP(IC13)        // Op. Amp.
	NET_C(IC13.4, I_VM15)
	NET_C(IC13.7, I_V15)

	TTL_74121_DIP(IC14)     // Monostable multivibrators with Schmitt-trigger inputs
	NET_C(IC14.7, GND)
	NET_C(IC14.14, I_V5)

	TTL_7406_DIP(IC15)      // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC15.7, GND)
	NET_C(IC15.14, I_V5)

	TL081_DIP(IC16)         // Op. Amp.
	NET_C(IC16.4, I_VM15)
	NET_C(IC16.7, I_V15)

	LM555_DIP(IC17)

	CA3080_DIP(IC18)        // Op. Amp.
	NET_C(IC18.4, I_VM15)
	NET_C(IC18.7, I_V15)

//  TL081_DIP(IC19)         // Op. Amp. -- part of final amp (not emulated)
//  NET_C(IC19.4, I_VM15)
//  NET_C(IC19.7, I_V15)

#if (HLE_NOISE_GEN)
	//
	// The "wideband noise gen" relies on properties
	// of the components to create noise. Not only
	// does this simulate poorly, but it would be too
	// slow for realtime, so HLE it with some quality
	// noise.
	//
	// Note that Sundance and Tail Gunner have the
	// exact same circuit.
	//
	CLOCK(NOISE_CLOCK, 10000)
	NET_C(NOISE_CLOCK.GND, GND)
	NET_C(NOISE_CLOCK.VCC, I_V5)

	SYS_NOISE_MT_U(NOISE, 3)
	NET_C(NOISE.I, NOISE_CLOCK.Q)
	NET_C(NOISE.1, GND)
	NET_C(NOISE.2, R1.2, R7.1, R35.1)

	NET_C(GND, C9.1, C9.2, C10.1, C10.2, C11.1, C11.2, C12.1, C12.2, C13.1, C13.2)
	NET_C(GND, D1.A, D1.K, D2.A, D2.K, D3.A, D3.K)
	NET_C(GND, R15.1, R15.2, R16.1, R16.2, R17.1, R17.2, R18.1, R18.2, R19.1, R19.2, R20.1, R20.2, R21.1, R21.2, R22.1, R22.2, R23.1, R23.2)
	NET_C(GND, IC5.2, IC5.3, IC5.7, IC6.2, IC6.3, IC7.2, IC7.3)
#else
	NET_C(C9.2, C10.1, GND)
	NET_C(C9.1, C10.2, D1.K, R15.1, IC5.7)
	NET_C(R15.2, C13.1, I_V15)
	NET_C(C13.2, GND)
	NET_C(D1.A, C11.1, R17.2)
	NET_C(R17.1, GND)
	NET_C(C11.2, R18.2, IC5.3)
	NET_C(R18.1, C12.1, R19.2)
	NET_C(R19.1, GND)
	NET_C(C12.2, IC5.2, R20.1)
	NET_C(R20.2, IC5.6, R21.1)
	NET_C(R21.2, IC6.2, R16.1, D3.A, D2.K)
	NET_C(D2.A, A3.K, R16.2, IC6.6)
	NET_C(IC6.3, GND)
	NET_C(R22.2, R23.1, IC7.2)
	NET_C(IC7.3, GND)
	NET_C(R23.2, IC7.6, R1.2, R7.1, R35.1)
#endif

	//
	// Explosion
	//

	NET_C(I_OUT_2, R11.1, IC4.2)
	NET_C(R11.2, R12.2, IC4.8, IC4.4, I_V5) // IC4.4 not listed
	NET_C(R12.1, IC4.6, IC4.7, C7.1)
	NET_C(C7.2, GND)
	NET_C(IC4.1, GND)
	NET_C(IC4.3, Q1.E)
	NET_C(Q1.B, R13.2)
	NET_C(R13.1, GND)
	NET_C(Q1.C, C8.1, R14.1)
	NET_C(C8.2, I_VM15)
	NET_C(R14.2, IC2.5)

	NET_C(C1.1, GND)
	NET_C(C1.2, R2.1, R1.1)
	NET_C(R2.2, C2.1)
	NET_C(C2.2, IC1.2, R3.1, C3.1)
	NET_C(IC1.3, GND)
	NET_C(C3.2, R3.2, IC1.6, R4.1)
	NET_C(R4.2, C4.1)
	NET_C(C4.2, R5.2, IC2.2)
	NET_C(R5.1, GND)
	NET_C(IC2.3, R6.2)
	NET_C(R6.1, GND)
	NET_C(IC2.6, R58.1)

	//
	// Reappearance hiss
	//

	NET_C(I_OUT_4, IC8.13)
	NET_C(IC8.12, R24.1)
	NET_C(R24.2, I_V5)
	NET_C(IC8.11, Q2.E)
	NET_C(Q2.B, R26.2)
	NET_C(R26.1, GND)
	NET_C(Q2.C, C14.1, R25.1)
	NET_C(C14.2, I_VM15)
	NET_C(R25.2, IC3.5)

	NET_C(R7.2, C5.1)
	NET_C(C5.2, R8.2, C6.1)
	NET_C(R8.1, GND)
	NET_C(C6.2, R9.2, IC3.2)
	NET_C(R9.1, GND)
	NET_C(IC3.3, R10.2)
	NET_C(R10.1, GND)
	NET_C(IC3.6, R58.1)

	//
	// Hi level / Normal level sword hum
	//

	NET_C(I_OUT_1, IC8.4)
	NET_C(IC8.5, R24.1)
	NET_C(IC8.6, Q3.E)
	NET_C(Q3.B, R27.2)
	NET_C(R27.1, GND)
	NET_C(Q3.C, R28.1, R29.1)
	NET_C(R28.2, IC13.5)
	NET_C(R29.2, Q4.C)
	NET_C(Q4.B, R30.2)
	NET_C(R30.1, GND)
	NET_C(Q4.E, IC8.3)
	NET_C(IC8.2, R24.1)
	NET_C(I_OUT_0, IC8.1)

	NET_C(R36.2, IC12.4, IC12.8, I_V15)
	NET_C(R36.1, IC12.7, R37.2)
	NET_C(R37.1, IC12.6, IC12.2, C27.2)
	NET_C(C27.1, GND, IC12.1, C28.1)
	NET_C(C28.2, IC12.5)
	NET_C(IC12.3, R38.1)
	NET_C(R38.2, C29.1, R39.1)
	NET_C(C29.2, GND)
	NET_C(R39.2, C30.1, R40.1)
	NET_C(C30.2, GND)

	NET_C(R31.2, IC9.4, IC9.8, I_V15)
	NET_C(R31.1, IC9.7, R32.2)
	NET_C(R32.1, IC9.6, IC9.2, C15.1)
	NET_C(C15.2, IC9.1, GND, C16.1)
	NET_C(C16.2, IC9.5)
	NET_C(IC9.3, R33.1)
	NET_C(R33.2, C17.1, R34.1)
	NET_C(C17.2, GND)
	NET_C(R34.2, C18.1, C19.1)
	NET_C(C18.2, GND)
	NET_C(C19.2, C20.2, C21.2, R41.2, IC13.2)
	NET_C(R41.1, GND)
	NET_C(R35.2, C20.1)
	NET_C(C21.1, R40.2)
	NET_C(IC13.3, R42.2)
	NET_C(R42.1, GND)
	NET_C(IC13.6, R58.1)
	NET_C(R58.2, GND)
	ALIAS(OUTPUT, R58.1)

	//
	// Pit fall
	//

	NET_C(I_OUT_3, IC8.10, IC14.4)
	NET_C(R43.2, I_V5)
	NET_C(R43.1, IC14.11, C31.1)
	NET_C(C31.2, IC14.10)
	NET_C(IC14.5, IC14.3, I_V5)
	NET_C(IC14.6, IC15.1)
	NET_C(IC15.2, R44.1)
	NET_C(R44.2, D4.K, C33.1)
	NET_C(C33.2, GND)
	NET_C(D4.A, R46.1)

	NET_C(R52.2, IC17.4, IC17.8, I_V15)
	NET_C(R52.1, IC17.7, R53.2)
	NET_C(R53.1, IC17.6, IC17.2, C35.2)
	NET_C(C35.1, IC17.1, C36.1, GND)
	NET_C(C36.2, IC17.5)
	NET_C(IC17.3, C32.2)
	NET_C(C32.1, R45.2)

#if (HLE_PITFALL_VCO)
	//
	// This VCO is very tricky to HLE. There is an rising curve
	// controlling the VCO that is modulated by a 555 timer. To
	// approximate this, the 555 was disconnected from the ciruit
	// and full simulation recorded at 1000x rate to find the
	// mapping between the input (taken from R46.2) and the final
	// TTL output at IC8.9.
	//
	// Map from original R46.2 to IC8.9 with 555 timer removed:
	//    R2 = 0.97609: HP = (0.000065653*A0) - 0.000100931
	//    R2 = 0.99674: HP = (0.00000448319*A0*A0) - (0.00000588451*A0) + 0.000158263
	//    R2 = 0.99695: HP = (0.000000153649*A0*A0*A0) + (0.000000894331*A0*A0) + (0.0000204462*A0) + 0.000098055
	//    R2 = 0.99808: HP = (0.000000086668*A0*A0*A0*A0) - (0.00000223815*A0*A0*A0) + (0.0000235819*A0*A0) - (0.000064560*A0) + 0.000198836
	//    R2 = 0.99808: HP = (-0.000000000510037*A0*A0*A0*A0*A0) + (0.000000103515*A0*A0*A0*A0) - (0.00000243946*A0*A0*A0) + (0.0000246047*A0*A0) - (0.000066382*A0) + 0.000198653
	//
	// As usual this mapping changed once the VCO was removed
	// from the ciruit, so this is the mapping that is used:
	//
	// Map from clipped R46.2 to IC8.9 when the 555 timer is removed:
	//    R2 = 0.99759: HP = (0.00369938*A0) - 0.000165421
	//    R2 = 0.99774: HP = (-0.00117988*A0*A0) + (0.00409590*A0) - 0.000196405
	//    R2 = 0.99774: HP = (0.00262450*A0*A0*A0) - (0.00252128*A0*A0) + (0.00431521*A0) - 0.000207837
	//    R2 = 0.99774: HP = (-0.00269118*A0*A0*A0*A0) + (0.00447056*A0*A0*A0) - (0.00298272*A0*A0) + (0.00436492*A0) - 0.000209783
	//    R2 = 0.99774: HP = (1.393570*A0*A0*A0*A0*A0) - (1.202353*A0*A0*A0*A0) + (0.408585*A0*A0*A0) - (0.069485*A0*A0) + (0.0097063*A0) - 0.000377210
	//
	// Then the 555 timer modulation is added back in via an
	// AFUNC. This is not 100% perfect, since we are modulating
	// the clipped value not the original, but it provides a
	// reasonable approximation.
	//
	AFUNC(PITFALLMOD, 2, "A0 + (A1-7.5)/150")
	NET_C(R46.2, PITFALLMOD.A0)
	NET_C(R45.1, PITFALLMOD.A1)
	VARCLOCK(PITFALLCLK, 1, "max(0.000001,min(0.1,(0.00262450*A0*A0*A0) - (0.00252128*A0*A0) + (0.00431521*A0) - 0.000207837))")
	NET_C(PITFALLCLK.GND, GND)
	NET_C(PITFALLCLK.VCC, I_V5)
	NET_C(PITFALLCLK.Q, IC8.9)
	NET_C(PITFALLCLK.A0, PITFALLMOD.Q)
	NET_C(GND, R47.1, R47.2, R48.1, R48.2, R49.1, R49.2, R50.1, R50.2, R51.1, R51.2, C34.1, C34.2, D5.A, D5.K, IC16.2, IC16.3)
#else
	NET_C(R45.1, D4.K)
	NET_C(R46.2, IC16.3, D5.K, R48.1)
	NET_C(D5.A, GND)
	NET_C(R48.2, IC16.6, R47.2, R49.1)
	NET_C(R47.1, IC16.2, C34.2)
	NET_C(C34.1, GND)
	NET_C(R49.2, R50.2, Q5.B)
	NET_C(R50.1, GND)
	NET_C(Q5.E, GND)
	NET_C(Q5.C, R51.1)
	NET_C(Q5.C, IC8.9)
	NET_C(R51.2, I_V5)
#endif

	NET_C(IC8.8, C37.1)
	NET_C(C37.2, R54.1)
	NET_C(R54.2, R55.2, IC18.2)
	NET_C(R55.1, GND)
	NET_C(IC18.3, R56.2)
	NET_C(R56.1, GND)
	NET_C(R57.2, IC18.5)
	NET_C(R57.1, GND)
	NET_C(IC18.6, R58.1)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC15.3, IC15.5, IC15.9, IC15.11, IC15.13)

	//
	// Unconnected outputs
	//

/*
    HINT(IC5.4, NC)     // Q1
    HINT(IC5.6, NC)     // Q2
    HINT(IC5.8, NC)     // Q3
    HINT(IC5.10, NC)    // Q4
    HINT(IC5.12, NC)    // Q5
*/

NETLIST_END()
