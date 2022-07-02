// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Space Wars
//
// Derived from the Audio Board Schematic redrawn by William J.
// Boucher at Biltronix.com. Part numbers are not present on the
// schematic, so the equivalent part numbers were taken from the
// Barrier schematics, which are quite similar.
//
// Known problems/issues:
//
//    * None.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_NOISE_GEN (1)
#define ENABLE_FRONTIERS (1)


//
// Main netlist
//

NETLIST_START(spacewar)
{

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)
//  PARAM(Solver.MIN_TS_TS, 2e-5)

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
//  RES(R12, 0)             -- not present on Space Wars
	RES(R13, RES_K(8.2))
	RES(R14, RES_K(120))
	RES(R15, RES_K(20))
	RES(R16, RES_M(10))
	RES(R17, RES_K(10))
	RES(R18, RES_K(47))
	RES(R19, 820)
//  POT(R20, RES_K(10))     -- part of final amp (not emulated)
//  RES(R21, 150)           -- part of final amp (not emulated), not present on Space Wars
//  RES(R22, 2.7)           -- part of final amp (not emulated), not present on Space Wars
//  RES(R23, 2.7)           -- part of final amp (not emulated), not present on Space Wars
	RES(R24, RES_K(47))
	RES(R25, 150)
	RES(R26, RES_K(160))
	RES(R27, 750)
//  RES(R28, RES_K(68))     -- part of final amp (not emulated), illegible on Space Wars
//  POT(R29, RES_K(10))     -- part of final amp (not emulated)
//  RES(R30, 750)           -- part of final amp (not emulated)
	RES(R31, 470)
	RES(R32, RES_K(1))
	RES(R33, RES_K(39))
	RES(R34, RES_K(6.8))
	RES(R35, RES_K(560))
	RES(R36, RES_M(1))
	RES(R37, RES_K(10))
	RES(R38, RES_K(10))
//  RES(R39, RES_K(120))
	RES(R40, RES_K(120))
	RES(R41, RES_K(20))
	RES(R42, RES_K(1))
	RES(R43, RES_K(10))

	CAP(C1, CAP_U(1))
	CAP(C2, CAP_U(1))
	CAP(C3, CAP_U(0.01))
	CAP(C4, CAP_U(0.01))
	CAP(C5, CAP_U(0.1))
//  CAP(C6, CAP_U(4.7))     // not needed
//  CAP(C7, 0)              // not present
	CAP(C8, CAP_U(1))
	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_P(220))
	CAP(C11, CAP_U(0.1))
//  CAP(C12, CAP_U(0.01))   -- part of final amp (not emulated)
//  CAP(C13, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C14, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C15, CAP_U(50))     -- not needed
//  CAP(C16, CAP_U(2.2))    -- not needed
	CAP(C17, CAP_U(0.01))
	CAP(C18, CAP_U(33))
//  CAP(C19, CAP_U(50))     -- not needed
//  CAP(C20, CAP_U(2.2))    -- not needed
	CAP(C21, CAP_U(0.02))
	CAP(C22, CAP_U(0.1))
	CAP(C23, CAP_U(0.1))
	CAP(C24, CAP_U(2.2))

	D_1N914(CR1)
	D_1N914(CR2)
	D_1N914(CR3)
	D_1N914(CR4)
	D_1N914(CR5)
	D_1N914(CR6)

#if !(HLE_NOISE_GEN)
	Q_2N3906(Q1)    // PNP
	Q_2N3904(Q2)    // NPN
#endif
	Q_2N6426(Q3)    // NPN Darlington
//  Q_2N6292(Q4)    // NPN -- not used
//  Q_2N6107(Q5)    // PNP -- not used
	Q_2N6426(Q6)    // NPN Darlington
	Q_2N3904(Q7)    // NPN

	TL081_DIP(U1)           // Op. Amp.
	NET_C(U1.4, I_VM15)
	NET_C(U1.7, I_V15)

	TTL_7406_DIP(U2)        // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

	TL081_DIP(U3)           // Op. Amp.
	NET_C(U3.4, I_VM15)
	NET_C(U3.7, I_V15)

//  TTL_7815_DIP(U4)        // +15V Regulator -- not needed

	TL182_DIP(U5)           // Analog switch
	NET_C(U5.6, I_V15)
	NET_C(U5.7, I_V5)
	NET_C(U5.8, GND)
	NET_C(U5.9, I_VM15)

//  TL081_DIP(U6)           // Op. Amp. -- part of final amp (not emulated)
//  NET_C(U6.4, I_VM15)
//  NET_C(U6.7, I_V15)

//  TTL_7915_DIP(U7)        // -15V Regulator -- not needed

	TL081_DIP(U8)           // Op. Amp.
	NET_C(U8.4, I_VM15)
	NET_C(U8.7, I_V15)

	TL081_DIP(U9)           // Op. Amp.
	NET_C(U9.4, I_VM15)
	NET_C(U9.7, I_V15)

	TL182_DIP(U10)          // Analog switch
	NET_C(U10.6, I_V15)
	NET_C(U10.7, I_V5)
	NET_C(U10.8, GND)
	NET_C(U10.9, I_VM15)

	//
	// Top-left until output from U1
	//

#if (HLE_NOISE_GEN)
	CLOCK(NOISE_CLOCK, 2000)
	NET_C(NOISE_CLOCK.GND, GND)
	NET_C(NOISE_CLOCK.VCC, I_V5)

	SYS_NOISE_MT_N(NOISE, 0.0001)
	NET_C(NOISE.I, NOISE_CLOCK.Q)
	NET_C(NOISE.1, GND)
	NET_C(NOISE.2, C1.1)

	NET_C(GND, R1.1, R1.2, R2.1, R2.2, CR1.A, CR1.K, CR2.A, CR2.K)
#else
	NET_C(I_V15, CR1.A, R2.2)
	NET_C(CR1.K, CR2.A)
	NET_C(CR2.K, R1.2, Q1.B)
	NET_C(R1.1, Q2.C, GND)
	NET_C(R2.1, Q1.E)
	NET_C(Q2.E, Q1.C, C1.1)
#endif

	NET_C(C1.2, R3.2, U1.3)
	NET_C(R3.1, GND, R5.1)
	NET_C(U1.2, R5.2, R4.1)
	NET_C(R4.2, U1.6)

	//
	// Top-middle, from O1 until output from CR3
	//

	NET_C(I_OUT_1, U2.13)
	NET_C(U2.12, R6.1)
	NET_C(R6.2, R7.1, C2.1, Q3.B)
	NET_C(R7.2, I_V5, Q3.C)
	NET_C(C2.2, GND)
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
	NET_C(R10.1, CR4.K, GND, C5.1)
	NET_C(C4.2, C5.2, R13.1)

	//
	// Big middle section, from C8 until output from R15/R41/R37
	//

	NET_C(U1.6, C8.1)
	NET_C(C8.2, C24.2)
	NET_C(C24.1, R24.1)
	NET_C(R24.2, U8.2, C10.1, R16.1)
	NET_C(U8.3, GND)
	NET_C(U8.6, R16.2, C10.2, R31.2, R38.2)
	NET_C(R38.1, U5.14, U5.1)
	NET_C(I_OUT_2, U5.10)
	NET_C(U5.13, R14.1)
	NET_C(I_OUT_3, U5.5)
	NET_C(U5.2, R40.1)
	NET_C(R40.2, C23.2, R41.1)
	NET_C(R41.2, R13.2)
	NET_C(C23.1, GND)
	NET_C(R37.2, R13.2)
	NET_C(R14.2, C9.2, R15.1)
	NET_C(C9.1, GND)
	NET_C(R15.2, R13.2)
	NET_C(I_OUT_0, U2.9)
	NET_C(U2.8, R25.1)
	NET_C(R25.2, R26.1, C17.1, Q6.B, C18.1)
	NET_C(R26.2, C17.2, I_V5, Q6.C)
	NET_C(C18.2, GND)
	NET_C(Q6.E, R27.1)
	NET_C(R27.2, CR5.A)
	NET_C(CR5.K, R33.2, CR6.A, R34.2, C21.2, C22.1)
	NET_C(R31.1, R32.2, R33.1, Q7.E)
	NET_C(R32.1, Q7.B)
	NET_C(Q7.C, CR6.K, R34.1, C21.1, GND)
	NET_C(C22.2, R35.1)
	NET_C(R35.2, U9.2, R36.1)
	NET_C(U9.3, GND)
	NET_C(U9.6, R36.2, R37.1)

	//
	// Final stage
	//

	NET_C(U10.1, U10.14, R13.2)
	NET_C(I_OUT_4, U10.5, U2.1)
	NET_C(U2.2, R42.1, U10.10)
	NET_C(R42.2, I_V5)
	NET_C(U10.2, R17.2, C11.1)
	NET_C(U10.13, R43.2)
	NET_C(R43.1, GND)
	NET_C(R17.1, R18.1, GND)
	NET_C(C11.2, R18.2, R19.1)
	NET_C(R19.2, GND)
	ALIAS(OUTPUT, R18.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, U2.3, U2.5, U2.11)

	//
	// Frontier optimizations
	//

#if (ENABLE_FRONTIERS)
	// Separate each input into the summing network
	OPTIMIZE_FRONTIER(R13.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R15.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R41.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R37.1, RES_M(1), 50)

	// Decouple the Darlington BJTs from the sounds they enable
	OPTIMIZE_FRONTIER(R27.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R11.2, RES_M(1), 50)

	// Decouple the noise source from the downstream filters
	OPTIMIZE_FRONTIER(C3.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R24.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R38.2, RES_M(1), 50)
#endif

}
