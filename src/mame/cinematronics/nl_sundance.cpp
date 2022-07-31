// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Sundance
//
// Derived from the schematics in the Sundance manual.
//
// Known problems/issues:
//
//    * The "whoosh" sound is close to correct, though there may
//       be some missing harmonics in the HLE.
//
//    * The "wideband noise generator" is simulated with a generic
//       noise source. This seems to work fine and doesn't sound
//       too different from recordings.
//
//    * The pitch of the pings and bongs seems a little lower than
//       some recordings, though it's hard to say if this is due
//       to aging as the frequencies are controlled by 555 timers
//       and RC networks.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_NOISE_GEN (1)
#define HLE_WHOOSH_MOD (1)


//
// Main netlist
//

NETLIST_START(sundance)
{

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	TTL_INPUT(I_OUT_0, 1)       // active low
	TTL_INPUT(I_OUT_1, 1)       // active low
	TTL_INPUT(I_OUT_2, 1)       // active low
	TTL_INPUT(I_OUT_3, 1)       // active low
	TTL_INPUT(I_OUT_4, 1)       // active low
	TTL_INPUT(I_OUT_7, 1)       // active low

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND, I_OUT_7.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC, I_OUT_7.VCC)

	CINEMAT_LOCAL_MODELS

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

	RES(R1, RES_M(1))
	RES(R2, 470)
	RES(R3, 470)
	RES(R4, RES_K(470))
	RES(R5, 100)
	RES(R6, RES_K(470))
	RES(R7, RES_K(10))
	RES(R8, RES_M(1))
	RES(R9, RES_K(10))
	RES(R10, RES_K(47))
	RES(R11, RES_K(100))
	RES(R12, RES_K(220))
	RES(R13, RES_M(3.3))
	RES(R14, RES_K(390))
	RES(R15, RES_K(56))
	RES(R16, RES_M(1))
	RES(R17, RES_K(100))
	RES(R18, RES_K(10))
	RES(R19, RES_K(3.3))
	RES(R20, RES_K(100))
	RES(R21, RES_K(47))
	RES(R22, RES_K(47))
	RES(R23, RES_K(750))
	RES(R24, 470)
	RES(R25, RES_K(220))
	RES(R26, 330)
	RES(R27, RES_K(10))
	RES(R28, RES_K(47))
	RES(R29, RES_K(330))
	RES(R30, RES_K(30))
	RES(R31, RES_K(10))
	RES(R32, RES_K(2.7))
	RES(R33, RES_K(2.7))
	RES(R34, RES_K(100))
	RES(R35, RES_K(10))
	RES(R36, RES_K(4.7))
	RES(R37, RES_K(8.2))
	RES(R38, RES_K(120))
	RES(R39, RES_M(3.3))
	RES(R40, RES_K(39))
	RES(R41, RES_K(2.7))
	RES(R42, RES_K(2.7))
	RES(R43, RES_K(47))
	RES(R44, RES_K(8.2))
	RES(R45, RES_K(2.7))
	RES(R46, RES_K(2.7))
	RES(R47, RES_M(10))
	RES(R48, RES_M(10))
	RES(R49, RES_K(1))
	RES(R50, 330)
	RES(R51, RES_K(510))
	RES(R52, RES_K(15))
	RES(R53, RES_K(20))
	RES(R54, RES_K(47))
	RES(R55, RES_K(2.7))
	RES(R56, RES_K(2.7))
	RES(R57, 330)
	RES(R58, RES_K(390))
	RES(R59, RES_K(15))
	RES(R60, RES_K(24))
	RES(R61, RES_K(56))
	RES(R62, RES_K(2.7))
	RES(R63, RES_K(2.7))
	RES(R64, 470)
//  RES(R65, 150)       -- part of final amp (not emulated)
//  RES(R66, RES_K(22)) -- part of final amp (not emulated)
//  RES(R67, 150)       -- part of final amp (not emulated)
	RES(R68, 330)
	RES(R69, RES_K(390))
	RES(R70, RES_K(15))
	RES(R71, RES_K(30))
	RES(R72, RES_K(68))
	RES(R73, RES_K(2.7))
	RES(R74, RES_K(2.7))
//  RES(R75, RES_K(10)) -- part of final amp (not emulated)
//  POT(R76, RES_K(100)) -- part of final amp (not emulated)
//  PARAM(R76.DIAL, 0.500000) -- part of final amp (not emulated)
	RES(R77, 330)
	RES(R78, RES_K(220))

	CAP(C1, CAP_U(0.1))
	CAP(C2, CAP_U(100))
	CAP(C3, CAP_U(0.01))
	CAP(C4, CAP_U(0.1))
	CAP(C5, CAP_U(100))
	CAP(C6, CAP_U(1))
	CAP(C7, CAP_U(1))
	CAP(C8, CAP_U(10))
	CAP(C9, CAP_U(0.05))
	CAP(C10, CAP_U(0.1))
	CAP(C11, CAP_U(0.01))
	CAP(C12, CAP_U(0.1))
	CAP(C13, CAP_U(0.001))
	CAP(C14, CAP_U(0.005))
	CAP(C15, CAP_U(10))
//  CAP(C16, CAP_U(3.3)) -- not needed
//  CAP(C17, CAP_U(3.3)) -- not needed
//  CAP(C18, CAP_U(3.3)) -- not needed
//  CAP(C19, CAP_U(3.3)) -- not needed
//  CAP(C20, CAP_U(3.3)) -- not needed
	CAP(C21, CAP_U(0.1))
	CAP(C22, CAP_U(0.005))
	CAP(C23, CAP_U(0.1))
	CAP(C24, CAP_U(0.1))
	CAP(C25, CAP_U(0.1))
	CAP(C26, CAP_U(0.01))
	CAP(C27, CAP_U(0.1))
	CAP(C28, CAP_U(0.15))
	CAP(C29, CAP_U(0.1))
	CAP(C30, CAP_U(0.01))
	CAP(C31, CAP_U(0.1))
//  CAP(C32, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C33, CAP_P(470)) -- part of final amp (not emulated)
//  CAP(C34, CAP_P(470)) -- part of final amp (not emulated)
	CAP(C35, CAP_U(0.15))
	CAP(C36, CAP_U(0.1))
	CAP(C37, CAP_U(0.01))
	CAP(C38, CAP_U(0.1))
	CAP(C39, CAP_U(1))

	D_1N5240(D1)
	D_1N914(D2)
	D_1N914(D3)

	Q_2N3904(Q1)            // NPN
	Q_2N3904(Q2)            // NPN
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3906(Q5)            // PNP
	Q_2N3906(Q6)            // PNP
//  Q_2N6292(Q7)            // NPN -- part of final amp (not emulated)
//  Q_2N6107(Q9)            // PNP -- part of final amp (not emulated)
	Q_2N3906(Q8)            // PNP
	Q_2N3906(Q10)           // PNP

	TL081_DIP(IC1)          // Op. Amp.
//  NET_C(IC1.7, I_V15)     // (indirectly via R5)
	NET_C(IC1.4, I_VM15)

	TL081_DIP(IC2)          // Op. Amp.
	NET_C(IC2.7, I_V15)
	NET_C(IC2.4, I_VM15)

	TL081_DIP(IC3)          // Op. Amp.
	NET_C(IC3.7, I_V15)
	NET_C(IC3.4, I_VM15)

	LM3900_DIP(IC4)
	NET_C(IC4.7, GND)
	NET_C(IC4.14, I_V15)

//  TTL_7815_DIP(IC5)       // +15V Regulator -- not needed
//  TTL_7915_DIP(IC6)       // -15V Regulator -- not needed

	LM555_DIP(IC7)

	TL081_DIP(IC8)          // Op. Amp.
	NET_C(IC8.7, I_V15)
	NET_C(IC8.4, I_VM15)

	CA3080_DIP(IC9)
	NET_C(IC9.7, I_V15)
	NET_C(IC9.4, I_VM15)

	CA3080_DIP(IC10)
	NET_C(IC10.7, I_V15)
	NET_C(IC10.4, I_VM15)

	LM555_DIP(IC11)

	CA3080_DIP(IC12)
	NET_C(IC12.7, I_V15)
	NET_C(IC12.4, I_VM15)

	TTL_74LS125_DIP(IC13)   // Quad 3-state buffer
	NET_C(IC13.7, GND)
	NET_C(IC13.14, I_V5)

	LM555_DIP(IC14)

	LM555_DIP(IC15)

	LM555_DIP(IC16)

	CA3080_DIP(IC17)
	NET_C(IC17.7, I_V15)
	NET_C(IC17.4, I_VM15)

	CA3080_DIP(IC18)
	NET_C(IC18.7, I_V15)
	NET_C(IC18.4, I_VM15)

	CA3080_DIP(IC19)
	NET_C(IC19.7, I_V15)
	NET_C(IC19.4, I_VM15)

	TL081_DIP(IC20)         // Op. Amp.
	NET_C(IC20.7, I_V15)
	NET_C(IC20.4, I_VM15)

#if (HLE_NOISE_GEN)
	//
	// The "wideband noise gen" relies on properties
	// of the components to create noise. Not only
	// does this simulate poorly, but it would be too
	// slow for realtime, so HLE it with some quality
	// noise.
	//
	// Note that Tail Gunner has the exact same
	// circuit.
	//
	CLOCK(NOISE_CLOCK, 10000)
	NET_C(NOISE_CLOCK.GND, GND)
	NET_C(NOISE_CLOCK.VCC, I_V5)

	SYS_NOISE_MT_U(NOISE, 3)
	NET_C(NOISE.I, NOISE_CLOCK.Q)
	NET_C(NOISE.1, GND)
	NET_C(NOISE.2, R27.2, R36.1, R43.1)

	NET_C(GND, C1.1, C1.2, C2.1, C2.2, C3.1, C3.2, C4.1, C4.2, C5.1, C5.2)
	NET_C(GND, D1.A, D1.K, D2.A, D2.K, D3.A, D3.K)
	NET_C(GND, R1.1, R1.2, R3.1, R3.2, R4.1, R4.2, R5.1, R5.2, R6.1, R6.2, R7.1, R7.2, R8.1, R8.2, R9.1, R9.2, R10.1, R10.2)
	NET_C(GND, IC1.2, IC1.3, IC1.7, IC2.2, IC2.3, IC3.2, IC3.3)
#else
	NET_C(C1.1, C2.2, R1.1, R3.1, C5.2, GND)
	NET_C(C1.2, C2.1, D1.K, R5.1, IC1.7)
	NET_C(R1.2, C3.1, D1.A)
	NET_C(R3.2, R4.1, C4.1)
	NET_C(R4.2, C3.2, IC1.3)
	NET_C(C4.2, IC1.2, R6.1)
	NET_C(R6.2, IC1.6, R7.1)
	NET_C(R5.2, C5.1, I_V15)
	NET_C(R7.2, IC2.2, R8.1, D3.A, D2.K)
	NET_C(IC2.3, GND)
	NET_C(IC2.6, R8.2, D3.K, D2.A, R9.1)
	NET_C(R9.2, R10.1, IC3.2)
	NET_C(IC3.3, GND)
	NET_C(IC3.6, R10.2, R43.1, R27.2, R36.1)
#endif

	NET_C(I_OUT_1, R2.1, IC11.2)
	NET_C(R2.2, I_V5)
	NET_C(IC11.8, IC11.4, I_V5)     // -- IC11.4 not documented
	NET_C(IC11.3, R35.1)
	NET_C(IC11.1, GND)
	NET_C(R34.1, I_V5)
	NET_C(R34.2, IC11.6, IC11.7, C13.2)
	NET_C(C13.1, GND)

	NET_C(R35.2, R17.1, IC4.13)
	NET_C(R11.1, R14.1, R17.2, IC4.9)
	NET_C(R11.2, IC4.3)
	NET_C(R12.1, IC4.2)
	NET_C(R12.2, R16.2, R18.2, Q2.C, R22.2, I_V15)
	NET_C(IC4.4, R13.1)
	NET_C(R13.2, C6.2, IC4.6)
	NET_C(R14.2, IC4.1)
	NET_C(IC4.5, C6.1, R15.1, Q1.B)
	NET_C(R15.2, R16.1, IC4.8)

	NET_C(Q1.C, R18.1, Q2.B)
	NET_C(Q1.E, R19.2)
	NET_C(R19.1, GND)
	NET_C(Q2.E, Q3.B, R20.2)
	NET_C(R20.1, I_VM15)
	NET_C(Q3.E, R21.1)
	NET_C(R21.2, R22.1, R23.2)
	NET_C(R23.1, GND)
	NET_C(Q3.C, IC12.5)

	NET_C(R36.2, C14.1)
	NET_C(C14.2, R37.2, R40.2, C15.1)
	NET_C(R37.1, GND)

#if (HLE_WHOOSH_MOD)
	//
	// The "whoosh" sound has a noise modulator that is a steady
	// clock ~64.5Hz, generated by an LM3900 and an RC network.
	// When run at 48kHz, this network does not clock at the
	// correct frequency, so HLE this with a basic clock.
	//
	CLOCK(WHOOSH_CLK, 64.5)
	NET_C(WHOOSH_CLK.GND, GND)
	NET_C(WHOOSH_CLK.VCC, I_V15)
	NET_C(WHOOSH_CLK.Q, R40.1)
	NET_C(GND, C21.1, C21.2, R38.1, R38.2, R39.1, R39.2, R47.1, R47.2, R48.1, R48.2, IC4.11, IC4.12)
#else
	NET_C(R40.1, R38.2, IC4.10, R48.2)
	NET_C(C21.2, R38.1, R39.2)
	NET_C(C21.1, GND)
	NET_C(R39.1, IC4.11)
	NET_C(I_V15, R47.1)
	NET_C(R47.2, IC4.12, R48.1)
#endif

	NET_C(C15.2, R41.2, IC12.2)
	NET_C(R41.1, R42.1, GND)
	NET_C(R42.2, IC12.3)
	NET_C(IC12.6, IC10.6, IC9.6, IC17.6, IC18.6, IC19.6, R64.1)

	NET_C(I_OUT_2, R24.1, IC7.2)
	NET_C(R25.1, IC7.6, IC7.7, C7.1)
	NET_C(C7.2, IC7.1, GND)
	NET_C(R24.2, I_V5)
	NET_C(R25.2, IC7.8, IC7.4, I_V5)    // IC7.4 -- not documented
	NET_C(IC7.3, Q4.E)
	NET_C(Q4.B, R26.2)
	NET_C(R26.1, GND)
	NET_C(Q4.C, C8.1, R30.1)
	NET_C(C8.2, I_VM15)
	NET_C(R30.2, IC10.5)

	NET_C(R27.1, C9.2, R28.1)
	NET_C(C9.1, GND)
	NET_C(R28.2, C10.1)
	NET_C(C10.2, C11.1, R29.1, IC8.2)
	NET_C(IC8.3, GND)
	NET_C(IC8.6, C11.2, R29.2, R31.1)
	NET_C(R31.2, C12.1)
	NET_C(C12.2, R32.2, IC10.2)
	NET_C(R32.1, R33.1, GND)
	NET_C(R33.2, IC10.3)

	NET_C(R43.2, C22.1)
	NET_C(C22.2, R44.2, C23.1)
	NET_C(R44.1, GND)
	NET_C(C23.2, R45.2, IC9.2)
	NET_C(R45.1, GND)
	NET_C(R46.1, GND)
	NET_C(R46.2, IC9.3)

	NET_C(I_OUT_7, IC13.1)
	NET_C(I_V5, R49.2)
	NET_C(R49.1, IC13.2, IC13.5, IC13.9, IC13.12)
	NET_C(IC13.3, Q5.E)
	NET_C(Q5.B, R50.2)
	NET_C(R50.1, GND)
	NET_C(Q5.C, C24.2, R51.1)
	NET_C(C24.1, I_VM15)
	NET_C(R51.2, IC9.5)

	NET_C(I_OUT_3, IC13.4)
	NET_C(IC13.6, Q6.E)
	NET_C(Q6.B, R57.2)
	NET_C(R57.1, GND)
	NET_C(Q6.C, C28.1, R58.1)
	NET_C(C28.2, I_VM15)
	NET_C(R58.2, IC17.5)
	NET_C(IC14.4, IC14.8, I_V5)
	NET_C(IC14.3, R52.1)
	NET_C(R52.2, C25.1)
	NET_C(C25.2, R55.2, IC17.2)
	NET_C(IC14.2, IC14.6, R54.1, R53.2, C27.2)
	NET_C(R54.2, I_V5)
	NET_C(C27.1, GND)
	NET_C(IC14.7, R53.1)
	NET_C(IC14.5, C26.2)
	NET_C(C26.1, GND)
	NET_C(IC14.1, GND)
	NET_C(R55.1, GND)
	NET_C(R56.1, GND)
	NET_C(R56.2, IC17.3)

	NET_C(I_OUT_4, IC13.10)
	NET_C(IC13.8, Q8.E)
	NET_C(Q8.B, R68.2)
	NET_C(R68.1, GND)
	NET_C(Q8.C, C35.1, R69.1)
	NET_C(C35.2, I_VM15)
	NET_C(R69.2, IC18.5)
	NET_C(IC15.4, IC15.8, I_V5)
	NET_C(IC15.3, R59.1)
	NET_C(R59.2, C29.1)
	NET_C(C29.2, R62.2, IC18.2)
	NET_C(IC15.2, IC15.6, R61.1, R60.2, C31.2)
	NET_C(R61.2, I_V5)
	NET_C(C31.1, GND)
	NET_C(IC15.7, R60.1)
	NET_C(IC15.5, C30.2)
	NET_C(C30.1, GND)
	NET_C(IC15.1, GND)
	NET_C(R62.1, GND)
	NET_C(R63.1, GND)
	NET_C(R63.2, IC18.3)

	NET_C(I_OUT_0, IC13.13)
	NET_C(IC13.11, Q10.E)
	NET_C(Q10.B, R77.2)
	NET_C(R77.1, GND)
	NET_C(Q10.C, C39.1, R78.1)
	NET_C(C39.2, I_VM15)
	NET_C(R78.2, IC19.5)
	NET_C(IC16.4, IC16.8, I_V5)
	NET_C(IC16.3, R70.1)
	NET_C(R70.2, C36.1)
	NET_C(C36.2, R73.2, IC19.2)
	NET_C(IC16.2, IC16.6, R72.1, R71.2, C38.2)
	NET_C(R72.2, I_V5)
	NET_C(C38.1, GND)
	NET_C(IC16.7, R71.1)
	NET_C(IC16.5, C37.2)
	NET_C(C37.1, GND)
	NET_C(IC16.1, GND)
	NET_C(R73.1, GND)
	NET_C(R74.1, GND)
	NET_C(R74.2, IC19.3)

	ALIAS(OUTPUT, R64.1)
	NET_C(R64.2, GND)


	//
	// Unconnected pins
	//

	NET_C(GND, IC20.2, IC20.3)  // part of final amp

//  NET_C(GND, IC6.3, IC28.8, IC28.9, IC28.10, IC28.11)

}
