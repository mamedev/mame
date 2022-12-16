// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Speed Freak
//
// Derived from the utterly terrible schematics in the Speed Freak
// manual. Newly-drawn schematics are available upon request.
//
// Special thanks to:
//    * Frank Palazzolo for helping to verify connections and parts
//    * Brian Tarachi for supplying his corrected schematics
//
// Known problems/issues:
//
//    * Works pretty well. Needs more detailed comparison with clean
//       audio recordings from PCBs, but it's pretty close.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_CLOCK_GENERATOR (1)
#define HLE_CLOCK_INPUT (1)


//
// Main netlist
//

NETLIST_START(speedfrk)
{

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	TTL_INPUT(I_OUT_0, 1)   // active low
	TTL_INPUT(I_OUT_1, 1)   // active low
	TTL_INPUT(I_OUT_2, 1)   // active low
	TTL_INPUT(I_OUT_3, 1)   // active low
	TTL_INPUT(I_OUT_4, 1)   // active low
	TTL_INPUT(I_OUT_7, 1)   // active low

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND, I_OUT_7.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC, I_OUT_7.VCC)

	CINEMAT_LOCAL_MODELS

	ANALOG_INPUT(I_V5, 5)

//  RES(R1, 2.7)
//  RES(R2, 2.7)
//  RES(R3, 2.7)
//  RES(R4, 2.7)
//  RES(R5, 150)            // PCB verified
//  RES(R6, 150)
//  RES(R7, RES_K(10))      // PCB verified
//  RES(R8, RES_K(68))      // PCB verified
//  RES(R9, RES_K(2.2))     // PCB verified
//  RES(R10, 820)           // PCB verified
//  RES(R11, RES_K(47))     // PCB verified
//  RES(R12, RES_K(1))   ??
	RES(R13, 150)
	RES(R14, RES_K(2.2))    // PCB verified
	RES(R15, RES_K(10))     // PCB verified
	RES(R16, RES_K(2.2))    // PCB verified
	RES(R17, RES_K(1))      // PCB verified
	RES(R18, RES_K(8.2))    // PCB verified
	RES(R19, RES_K(3.9))    // PCB verified
	RES(R20, RES_K(4.7))    // PCB verified
	RES(R21, RES_K(3.3))    // PCB verified
	RES(R22, RES_K(10))     // PCB verified
	RES(R23, RES_K(4.7))    // PCB verified
	RES(R24, RES_K(10))
	RES(R25, RES_K(18))     // PCB verified
	RES(R26, RES_K(18))     // PCB verified
	RES(R27, RES_K(6.8))    // PCB verified
	RES(R28, RES_K(10))     // PCB verified
	RES(R29, RES_K(2.2))    // PCB verified
	RES(R30, 330)           // PCB verified
	RES(R31, 330)           // PCB verified
	RES(R32, RES_K(1))      // PCB verified
	RES(R33, RES_K(1))      // PCB verified
	RES(R34, RES_K(1))      // PCB verified
//  RES(R35, 0)             // PCB verified: not populated
	RES(R36, RES_K(1))      // PCB verified
	RES(R37, RES_K(1))      // PCB verified
	RES(R38, RES_K(1))      // PCB verified
	RES(R39, RES_K(1))      // PCB verified
	RES(R40, RES_K(1))      // PCB verified
	RES(R41, RES_K(1))      // PCB verified
	RES(R42, RES_K(1))      // PCB verified
	RES(R43, RES_K(1))      // PCB verified
	RES(R44, RES_K(30))     // PCB verified
	RES(R45, RES_K(4.7))    // PCB verified
	RES(R46, RES_K(10))     // PCB verified

//  CAP(C4, CAP_U(4.7))
//  CAP(C5, CAP_U(4.7))
	CAP(C12, CAP_U(0.001))
	CAP(C13, CAP_U(0.001))
	CAP(C17, CAP_U(0.02))
	CAP(C20, CAP_U(0.1))
	CAP(C23, CAP_U(0.1))

//  CAP(C1, CAP_U(50))
//  CAP(C2, CAP_U(50))
//  CAP(C3, CAP_U(4.7))
//  CAP(C6, CAP_U(0.002))
//  CAP(C7, CAP_U(0.002))
//  CAP(C8, CAP_U(0.01))
//  CAP(C9, CAP_U(0.1))
//  CAP(C10, CAP_U(0.1))
//  CAP(C11, CAP_U(0.02))

//  D_1N914B(CR1)   // OK
//  D_1N914B(CR2)   // OK
	D_1N914B(CR3)   // OK

//  Q_2N6292(Q1)    // NPN
//  Q_2N6107(Q2)    // PNP
	Q_2N3904(Q3)    // NPN
//  Q_2N3904(Q3)    // NPN -- unknown type

	TTL_74LS04_DIP(U2)      // Hex Inverting Gates
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

	TL081_DIP(U3)           // Op. Amp.
	NET_C(U3.4, GND)
	NET_C(U3.7, I_V5)

	TTL_74LS163_DIP(U4)     // Synchronous 4-Bit Counters
	NET_C(U4.8, GND)
	NET_C(U4.16, I_V5)

	TTL_74LS107_DIP(U5)     // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U5.7, GND)
	NET_C(U5.14, I_V5)

	TTL_74LS08_DIP(U6)      // Quad 2-Input AND Gates
	NET_C(U6.7, GND)
	NET_C(U6.14, I_V5)

	TTL_74LS163_DIP(U7)     // Synchronous 4-Bit Counters
	NET_C(U7.8, GND)
	NET_C(U7.16, I_V5)

	TTL_74LS163_DIP(U8)     // Synchronous 4-Bit Counters
	NET_C(U8.8, GND)
	NET_C(U8.16, I_V5)

	TTL_74LS163_DIP(U9)     // Synchronous 4-Bit Counters
	NET_C(U9.8, GND)
	NET_C(U9.16, I_V5)

//  TTL_7915_DIP(U8)        // -15V Regulator -- not needed
//  TTL_7815_DIP(U9)        // +15V Regulator -- not needed

	TTL_74LS04_DIP(U10)     // Hex Inverting Gates
	NET_C(U10.7, GND)
	NET_C(U10.14, I_V5)

	TTL_74LS08_DIP(U11)     // Quad 2-Input AND Gates
	NET_C(U11.7, GND)
	NET_C(U11.14, I_V5)

	TTL_74LS75_DIP(U12)     // 4-Bit Bistable Latches with Complementary Outputs
	NET_C(U12.12, GND)
	NET_C(U12.5, I_V5)

	TTL_74LS164_DIP(U13)    // 8-bit parallel-out serial shift registers
	NET_C(U13.7, GND)
	NET_C(U13.14, I_V5)

	TTL_74LS164_DIP(U14)    // 8-bit parallel-out serial shift registers
	NET_C(U14.7, GND)
	NET_C(U14.14, I_V5)

	TTL_74LS163_DIP(U15)    // Synchronous 4-Bit Counters
	NET_C(U15.8, GND)
	NET_C(U15.16, I_V5)

	TTL_74LS107_DIP(U17)    // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U17.7, GND)
	NET_C(U17.14, I_V5)

	TTL_74LS393_DIP(U18)    // Dual 4-Stage Binary Counter
	NET_C(U18.7, GND)
	NET_C(U18.14, I_V5)

	TTL_74LS86_DIP(U19)     // Quad 2-Input XOR Gates
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V5)

	TTL_74LS164_DIP(U20)    // 8-bit parallel-out serial shift registers
	NET_C(U20.7, GND)
	NET_C(U20.14, I_V5)

	LM555_DIP(U22)          // 5-5-5 Timer

	TTL_74LS163_DIP(U23)    // Dual 4-Stage Binary Counter
	NET_C(U23.8, GND)
	NET_C(U23.16, I_V5)

	TTL_74LS164_DIP(U24)    // 8-bit parallel-out serial shift registers
	NET_C(U24.7, GND)
	NET_C(U24.14, I_V5)

	//
	// 76kHz coming from the logic PCB
	//
	CLOCK(J4_2, 76000)
	NET_C(J4_2.GND, GND)
	NET_C(J4_2.VCC, I_V5)

#if (HLE_CLOCK_GENERATOR)
	//
	// Skip the clock generator and just do it directly
	//
	CLOCK(C2MHZ, 2000000)
	NET_C(C2MHZ.GND, GND)
	NET_C(C2MHZ.VCC, I_V5)
	NET_C(GND, R30.1, R30.2, R31.1, R31.2, C12.1, C12.2, C13.1, C13.2, U10.1, U10.3, U10.5)
#else
	NET_C(R30.1, U10.1, C13.1)
	NET_C(R30.2, U10.2, C12.1)
	NET_C(C12.2, U10.3, R31.1)
	NET_C(R31.2, U10.4, C13.2, U10.5)
	ALIAS(C2MHZ, U10.6)
#endif

#if (HLE_CLOCK_INPUT)
	//
	// The clock input from the main PCB is run through a voltage
	// converter which eats a lot of time and is unnecessary since
	// we're just generating a TTL signal already.
	//
	NET_C(J4_2.Q, U5.12)
	NET_C(GND, R27.1, R27.2, R28.1, R28.2, R29.1, R29.2, CR3.A, CR3.K, U3.2, U3.3)
#else
	NET_C(J4_2.Q, U3.3)
	NET_C(R27.1, GND)
	NET_C(R27.2, U3.2, R28.1)
	NET_C(R28.2, I_V5)
	NET_C(U3.6, R29.1)
	NET_C(R29.2, CR3.K, U5.12)
#endif

	NET_C(CR3.A, GND)
	NET_C(U5.1, R41.1)
	NET_C(R41.2, I_V5)
	NET_C(U5.4, GND)
	NET_C(U5.13, U5.10, U10.9, U4.15)
	NET_C(U5.3, U5.8)
	NET_C(U5.2, U5.11)
	NET_C(U5.6, U4.9)
	NET_C(U5.9, U4.2, C2MHZ.Q)

	NET_C(U10.8, U4.7)
	NET_C(U4.3, U4.1, I_V5)
	NET_C(U4.6, R36.1)
	NET_C(U4.4, U4.5, GND)
	NET_C(U4.14, U18.1, U6.13, U7.2)
	NET_C(U4.10, I_V5)          // need to verify

	NET_C(U18.6, U18.13)
	NET_C(U18.2, U18.12, GND)
	NET_C(U18.8, U17.12)
	NET_C(U18.10, U20.8, U24.8)
	NET_C(U18.11, U23.2)

	NET_C(I_OUT_0, U2.13)
	NET_C(U2.12, U2.1)
	ALIAS(STEERING, U2.12)
	NET_C(U2.2, U6.4)
	NET_C(U6.6, U19.13, U19.5)
	NET_C(U19.12, R39.1)
	NET_C(R39.2, I_V5)

	NET_C(I_OUT_1, U6.9)
	NET_C(U6.10, R32.2)
	NET_C(R32.1, I_V5)
	NET_C(U6.8, U2.5, U6.12, U12.4, U12.13)
	NET_C(U2.6, R14.2, U6.5)
	NET_C(R14.1, Q3.B)
	NET_C(Q3.E, GND)
	NET_C(Q3.C, R13.1)
	NET_C(R13.2, GND)
	ALIAS(LAMP, R13.2)

	NET_C(U19.11, U20.1)
	NET_C(U20.9, R42.1)
//  NET_C(R42.2, I_V5)
	NET_C(U20.2, U19.3)
	NET_C(U20.12, U19.1)
	NET_C(U20.13, U19.4)

	NET_C(U19.6, U24.1, U24.2)
	NET_C(U24.12, U19.2)
	NET_C(U24.9, R43.1)
	NET_C(R43.2, I_V5)
	NET_C(U24.13, R44.1, U11.12, U11.9, U11.5, U11.2)

	NET_C(U23.1, U23.3, U23.5, U23.7, R40.2)
	NET_C(U23.10, R40.2)        // need to verify
	NET_C(R40.1, I_V5)
	NET_C(U23.4, U23.6, GND)
	NET_C(U23.9, U19.8, U17.9)
	NET_C(U23.15, U19.10)
	NET_C(U19.9, R39.1)

	NET_C(I_OUT_7, U2.9)
	NET_C(U2.8, U17.10, U17.13)
	NET_C(U17.8, U17.11, U17.1, U17.4, R34.1)
	NET_C(R34.2, I_V5)
	NET_C(U17.5, R25.1)
	NET_C(U17.3, R26.1)

	NET_C(R44.2, U22.7, R45.1, R46.2)
	NET_C(R45.2, U22.4, U22.8, I_V5)
	NET_C(R46.1, U22.6, U22.2, C23.2)
	NET_C(C23.1, GND)
	NET_C(U22.1, GND)
	NET_C(U22.5, C17.2)
	NET_C(C17.1, GND)

	NET_C(I_OUT_4, U6.2)
	NET_C(U22.3, U6.1)
	NET_C(U6.3, R15.1)

	NET_C(U6.11, U15.2, U9.2, U8.2)
	NET_C(U15.1, U15.7, R42.1)
	NET_C(R42.2, I_V5)
	NET_C(U9.1, U9.7, U8.1, U8.7, R37.1)
	NET_C(R37.2, I_V5)
	NET_C(U15.15, U10.11, U7.10)
	NET_C(U15.6, U14.3)
	NET_C(U15.5, U14.4)
	NET_C(U15.4, U14.5)
	NET_C(U15.3, U14.6)
	NET_C(U15.10, U9.15)
	NET_C(U15.9, U9.9, U8.9, U10.10)

	NET_C(U9.6, U14.10)
	NET_C(U9.5, U14.11)
	NET_C(U9.4, U14.12)
	NET_C(U9.3, U14.13, U13.1, U13.2)
	NET_C(U9.10, U8.15)

	NET_C(U8.6, U13.3)
	NET_C(U8.5, U13.4)
	NET_C(U8.4, U13.5)
	NET_C(U8.3, U13.6)
	NET_C(U8.10, R36.1)
	NET_C(R36.2, I_V5)

	NET_C(I_OUT_2, U2.11)
	NET_C(I_OUT_3, U2.3)
	NET_C(U2.10, U14.1, U14.2)
	NET_C(U2.4, U14.8, U13.8)
	NET_C(U14.9, U13.9, R38.1)
	NET_C(R38.2, I_V5)

	NET_C(U12.9, U11.13)
	NET_C(U12.10, U11.10)
	NET_C(U12.15, U11.4)
	NET_C(U12.16, U11.1)
	NET_C(U12.7, U13.10)
	NET_C(U12.6, U13.11)
	NET_C(U12.3, U13.12)
	NET_C(U12.2, U13.13)

	NET_C(U11.11, R17.1)
	NET_C(U11.8, R16.1)
	NET_C(U11.6, R19.1)
	NET_C(U11.3, R18.1)
	NET_C(R17.2, R16.2, R19.2, R18.2, R20.1)

	NET_C(U7.1, U7.5, R33.2)
	NET_C(R33.1, I_V5)
	NET_C(U7.7, STEERING)
	NET_C(U7.9, U10.12)
	NET_C(U7.15, U10.13)
	NET_C(U7.3, U7.4, U7.6, GND)
	NET_C(U7.11, R22.1)
	NET_C(U7.12, R23.1)
	NET_C(U7.13, R24.1)
	NET_C(R22.2, R23.2, R24.2, C20.2, R21.1)
	NET_C(C20.1, GND)

	NET_C(R15.2, R25.2, R26.2, R21.2, R20.2)
	ALIAS(OUTPUT, R20.2)

	//
	// Unconnected outputs
	//

	HINT(U4.11, NC)     // Q3
	HINT(U4.12, NC)     // Q2
	HINT(U4.13, NC)     // Q1
	HINT(U5.5, NC)      // Q2
	HINT(U7.14, NC)     // Q0
	HINT(U8.11, NC)     // Q3
	HINT(U8.12, NC)     // Q2
	HINT(U8.13, NC)     // Q1
	HINT(U8.14, NC)     // Q0
	HINT(U9.11, NC)     // Q3
	HINT(U9.12, NC)     // Q2
	HINT(U9.13, NC)     // Q1
	HINT(U9.14, NC)     // Q0
	HINT(U10.2, NC)     // QQ1 -- part of 2MHz clock gen
	HINT(U10.4, NC)     // QQ2 -- part of 2MHz clock gen
	HINT(U10.6, NC)     // QQ3 -- part of 2MHz clock gen
	HINT(U12.1, NC)     // QQ0
	HINT(U12.8, NC)     // QQ3
	HINT(U12.11, NC)    // QQ2
	HINT(U12.14, NC)    // QQ1
	HINT(U15.11, NC)    // Q3
	HINT(U15.12, NC)    // Q2
	HINT(U15.13, NC)    // Q1
	HINT(U15.14, NC)    // Q0
	HINT(U17.2, NC)     // QQ1
	HINT(U17.6, NC)     // QQ2
	HINT(U18.3, NC)     // Q0
	HINT(U18.4, NC)     // Q1
	HINT(U18.5, NC)     // Q2
	HINT(U18.9, NC)     // Q2
	HINT(U20.3, NC)     // Q0
	HINT(U20.4, NC)     // Q1
	HINT(U20.5, NC)     // Q2
	HINT(U20.6, NC)     // Q3
	HINT(U20.10, NC)    // Q4
	HINT(U20.11, NC)    // Q5
	HINT(U23.11, NC)    // Q3
	HINT(U23.12, NC)    // Q2
	HINT(U23.13, NC)    // Q1
	HINT(U23.14, NC)    // Q0
	HINT(U24.3, NC)     // Q0
	HINT(U24.4, NC)     // Q1
	HINT(U24.5, NC)     // Q2
	HINT(U24.6, NC)     // Q3
	HINT(U24.10, NC)    // Q4
	HINT(U24.11, NC)    // Q5

}
