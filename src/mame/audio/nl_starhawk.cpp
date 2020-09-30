// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Star Hawk
//
// Derived from the schematics in the Star Hawk manual.
//
// Special thanks to:
//    * Jay Gallagher for verifying PCB components
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

#define HLE_SHIP_VCO (1)
#define HLE_LAZER_VCOS (1)


//
// Debugging/test - should be off for release
//

#define SLOW_SHIP_WOBBLE (0)


//
// Main netlist
//

NETLIST_START(starhawk)

#if (HLE_LAZER_VCOS && HLE_SHIP_VCO)
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
	TTL_INPUT(I_OUT_7, 0)               // active high

	NET_C(GND, I_OUT_0.GND, I_OUT_1.GND, I_OUT_2.GND, I_OUT_3.GND, I_OUT_4.GND, I_OUT_7.GND)
	NET_C(I_V5, I_OUT_0.VCC, I_OUT_1.VCC, I_OUT_2.VCC, I_OUT_3.VCC, I_OUT_4.VCC, I_OUT_7.VCC)

	CINEMAT_LOCAL_MODELS

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

	RES(R1, RES_K(10))
	RES(R2, RES_K(10))
	RES(R3, RES_K(20))
	RES(R4, RES_K(10))
	RES(R5, RES_K(1))
	RES(R6, RES_K(150))
	RES(R7, 330)
	RES(R8, RES_K(10))
	RES(R9, RES_K(47))
	RES(R10, RES_K(150))
	RES(R11, RES_K(100))
	RES(R12, RES_K(47))
	RES(R13, RES_K(10))
	RES(R14, RES_K(2.7))
	RES(R15, RES_K(2.7))
	RES(R16, RES_K(30))
	RES(R17, RES_K(510))
	RES(R18, RES_K(10))
	RES(R19, RES_K(33))
//  RES(R20, 150)       -- part of final amp (not emulated)
//  RES(R21, RES_K(22)) -- part of final amp (not emulated)
	RES(R22, RES_K(1))
//  RES(R23, RES_K(10)) -- part of final amp (not emulated)
//  RES(R24, 150)       -- part of final amp (not emulated)
//  POT(R25, RES_K(100))-- part of final amp (not emulated)
	RES(R26, RES_K(1))
	RES(R27, RES_K(1))
	RES(R28, RES_K(510))
	RES(R29, RES_K(10))     // PCB verified
//  RES(R30, ???)
	RES(R31, RES_K(47))     // PCB verified
	RES(R32, RES_M(3.3))    // PCB verified
	RES(R33, RES_M(1))
	RES(R34, RES_K(47))
	RES(R35, RES_M(1))
	RES(R36, RES_M(1))
	RES(R37, RES_M(1))
	RES(R38, RES_M(1))
	RES(R39, 150)
	RES(R40, RES_K(10))
	RES(R41, RES_K(20))
	RES(R42, RES_K(1))
	RES(R43, RES_M(1))
	RES(R44, RES_K(10))
	RES(R45, RES_K(10))
	RES(R46, 150)
	RES(R47, RES_K(20))
	RES(R48, RES_M(1))
	RES(R49, RES_K(10))
	RES(R50, RES_K(1))
	RES(R51, RES_K(10))
	RES(R52, RES_K(20))
	RES(R53, RES_K(39))
	RES(R54, RES_K(82))
	RES(R55, RES_K(2.2))
	RES(R56, RES_K(1))
	RES(R57, RES_K(10))
	RES(R58, RES_K(20))
	RES(R59, RES_K(39))
	RES(R60, RES_K(82))

//  CAP(C1, CAP_U(2.2))
//  CAP(C2, CAP_U(2.2))
//  CAP(C3, CAP_U(3.3))
//  CAP(C4, CAP_U(3.3))
	CAP(C5, CAP_P(100))
	CAP(C6, CAP_U(3.3))
	CAP(C7, CAP_U(0.01))
	CAP(C8, CAP_U(1))
	CAP(C9, CAP_U(0.022))
	CAP(C10, CAP_U(0.15))       // 15?
	CAP(C11, CAP_U(0.15))
	CAP(C12, CAP_U(15))
	CAP(C13, CAP_U(0.0033))
	CAP(C14, CAP_U(0.0047))
	CAP(C15, CAP_U(1))
//  CAP(C16, CAP_P(470))    -- part of final amp (not emulated)
	CAP(C17, CAP_U(22))
//  CAP(C18, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C19, CAP_P(470))    -- part of final amp (not emulated)
	CAP(C20, CAP_U(1))
#if (SLOW_SHIP_WOBBLE)
	CAP(C21, CAP_U(22))     // discovered by accident, makes HLE analysis easier
#else
	CAP(C21, CAP_U(0.22))   // PCB verified
#endif
	CAP(C22, CAP_U(0.1))
	CAP(C23, CAP_U(0.0027))
	CAP(C24, CAP_U(0.1))
	CAP(C25, CAP_U(0.0027))
	CAP(C26, CAP_U(1))
	CAP(C27, CAP_U(0.1))
//  CAP(C39, CAP_U(1))
//  CAP(C40, CAP_U(1))

	D_1N914(CR1)
	D_1N914(CR2)
	D_1N914(CR3)
	D_1N914(CR4)
	D_1N914(CR5)
	D_1N914(CR6)
	D_1N914(CR7)
	D_1N914(CR8)
	D_1N914(CR9)
	D_1N914(CR10)

	Q_2N3906(Q1)            // PNP
//  Q_2N6292(Q2)            // NPN -- part of final amp (not emulated)
//  Q_2N6107(Q3)            // PNP -- part of final amp (not emulated)
#if (!HLE_LAZER_VCOS)
	Q_2N3904(Q4)            // NPN
	Q_2N3904(Q5)            // NPN
#endif

	TL182_DIP(IC3A)         // Analog switch
	NET_C(IC3A.6, I_V15)
	NET_C(IC3A.7, I_V5)
	NET_C(IC3A.8, GND)
	NET_C(IC3A.9, I_VM15)

//  TTL_7815_DIP(IC2D)      // +15V Regulator -- not needed
//  TTL_7915_DIP(IC2C)      // -15V Regulator -- not needed

	TL081_DIP(IC4A)         // Op. Amp.
	NET_C(IC4A.4, I_VM15)
	NET_C(IC4A.7, I_V15)

	TL081_DIP(IC4B)         // Op. Amp.
	NET_C(IC4B.4, I_VM15)
	NET_C(IC4B.7, I_V15)

//  TL081_DIP(IC4C)         // Op. Amp. -- part of final amp (not emulated)
//  NET_C(IC4C.4, I_VM15)
//  NET_C(IC4C.7, I_V15)

	TTL_74LS393_DIP(IC4E)   // Dual 4-Stage Binary Counter
	NET_C(IC4E.7, GND)
	NET_C(IC4E.14, I_V5)

	TL081_DIP(IC5A)         // Op. Amp.
	NET_C(IC5A.4, I_VM15)
	NET_C(IC5A.7, I_V15)

	TL081_DIP(IC5B)         // Op. Amp.
	NET_C(IC5B.4, I_VM15)
	NET_C(IC5B.7, I_V15)

	LM556_DIP(IC5D)

	PROM_74S287_DIP(IC5E)   // 1024-bit PROM -- dump needed
	PARAM(IC5E.A.ROM, "2085.5e8e")
	NET_C(IC5E.8, GND)
	NET_C(IC5E.16, I_V5)

	CA3080_DIP(IC6A)        // Trnscndt. Op. Amp.
	NET_C(IC6A.7, I_V15)
	NET_C(IC6A.4, I_VM15)

	TL081_DIP(IC6B)         // Op. Amp.
	NET_C(IC6B.4, I_VM15)
	NET_C(IC6B.7, I_V15)

	TTL_74LS04_DIP(IC6C)    // Hex Inverting Gates
	NET_C(IC6C.7, GND)
	NET_C(IC6C.14, I_V5)

	LM556_DIP(IC6D)

	TL081_DIP(IC6E)         // Op. Amp.
	NET_C(IC6E.4, I_VM15)
	NET_C(IC6E.7, I_V15)

	TL081_DIP(IC6F)         // Op. Amp.
	NET_C(IC6F.4, I_VM15)
	NET_C(IC6F.7, I_V15)

	TTL_7406_DIP(IC7C)      // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC7C.7, GND)
	NET_C(IC7C.14, I_V5)

	TTL_74LS393_DIP(IC7E)   // Dual 4-Stage Binary Counter
	NET_C(IC7E.7, GND)
	NET_C(IC7E.14, I_V5)

	TTL_74LS164_DIP(IC8C)   // 8-bit Shift Reg.
	NET_C(IC8C.7, GND)
	NET_C(IC8C.14, I_V5)

	TTL_74LS164_DIP(IC8D)   // 8-bit Shift Reg.
	NET_C(IC8D.7, GND)
	NET_C(IC8D.14, I_V5)

	PROM_74S287_DIP(IC8E)   // 1024-bit PROM -- dump needed
	PARAM(IC8E.A.ROM, "2085.5e8e")
	NET_C(IC8E.8, GND)
	NET_C(IC8E.16, I_V5)

	TTL_74LS164_DIP(IC9C)   // 8-bit Shift Reg.
	NET_C(IC9C.7, GND)
	NET_C(IC9C.14, I_V5)

	TTL_74LS164_DIP(IC9D)   // 8-bit Shift Reg.
	NET_C(IC9D.7, GND)
	NET_C(IC9D.14, I_V5)

	TTL_74LS163_DIP(IC9E)   // Binary Counter (schems say can sub a 74161)
	NET_C(IC9E.8, GND)
	NET_C(IC9E.16, I_V5)

	TTL_74LS86_DIP(IC10C)   // Quad 2-Input XOR Gates
	NET_C(IC10C.7, GND)
	NET_C(IC10C.14, I_V5)

	TTL_74LS21_DIP(IC10D)   // Dual 4-Input AND Gates
	NET_C(IC10D.7, GND)
	NET_C(IC10D.14, I_V5)

	TTL_74LS393_DIP(IC10E)  // Dual 4-Stage Binary Counter
	NET_C(IC10E.7, GND)
	NET_C(IC10E.14, I_V5)

	//
	// Top-left noise generator
	//

	NET_C(I_V5, R1.2, IC6D.14, IC6D.4, R3.2)
	NET_C(R1.1, IC6D.1, R2.2)
	NET_C(R2.1, IC6D.2, IC6D.6, C7.2)
	NET_C(C7.1, GND)
	NET_C(IC6D.7, GND)
	NET_C(IC6D.3, C8.1, R3.1, R4.1)
	NET_C(C8.2, GND)
	NET_C(R4.2, IC8D.4)
	NET_C(IC6D.5, IC8D.8, IC9D.8, IC10E.1, IC9E.2)

	NET_C(IC9E.9, IC9E.7, IC9E.10, I_V5)
	NET_C(IC9E.1, I_V5)
	NET_C(IC9E.15, IC10E.13, IC8C.8, IC9C.8)

	NET_C(IC10E.2, IC8D.3)
	NET_C(IC10E.3, IC10D.1)
	NET_C(IC10E.4, IC10D.2)
	NET_C(IC10E.5, IC10D.4)
	NET_C(IC10E.6, IC10D.5)

	NET_C(IC10D.6, IC10C.5)
	NET_C(IC10C.4, IC10C.11)
	NET_C(IC10C.6, IC8D.1, IC8D.2)

	NET_C(IC8D.9, I_V5)
	NET_C(IC8D.13, IC9D.1, IC9D.2)
	NET_C(IC9D.10, IC10C.13)
	NET_C(IC9D.12, IC10C.12)
	NET_C(IC9D.13, R8.2)
	NET_C(IC9D.9, I_V5)

	NET_C(R8.1, C10.2, R9.1)
	NET_C(C10.1, GND)
	NET_C(R9.2, CR1.K, CR2.A)

	NET_C(IC10E.12, IC8C.3)
	NET_C(IC10E.8, IC10D.9)
	NET_C(IC10E.9, IC10D.10)
	NET_C(IC10E.10, IC10D.12)
	NET_C(IC10E.11, IC10D.13)

	NET_C(IC10D.8, IC10C.1)
	NET_C(IC10C.2, IC10C.8)
	NET_C(IC10C.3, IC8C.1, IC8C.2)

	NET_C(IC8C.9, I_V5)
	NET_C(IC8C.13, IC9C.1, IC9C.2)
	NET_C(IC9C.10, IC10C.10)
	NET_C(IC9C.12, IC10C.9)
	NET_C(IC9C.13, R10.1)
	NET_C(IC9C.9, I_V5)

	NET_C(R10.2, C11.2, R11.1)
	NET_C(C11.1, GND)
	NET_C(R11.2, CR2.K, CR1.A, C12.1)

	NET_C(C12.2, IC6B.2, R12.1, C13.1)
	NET_C(IC6B.3, GND)
	NET_C(IC6B.6, C13.2, R12.2, R13.1)
	NET_C(R13.2, C15.1)
	NET_C(C15.2, R14.2, IC6A.2)
	NET_C(R14.1, GND)
	NET_C(IC6A.3, R15.2)
	NET_C(R15.1, GND)
	NET_C(IC6A.5, R16.2)

	//
	// Explosion
	//

	NET_C(I_OUT_0, IC6C.11)
	NET_C(IC6C.10, R5.1)
	NET_C(R5.2, C5.2, IC6D.8)
	NET_C(C5.1, GND)
	NET_C(IC6D.10, R6.2, I_V5)
	NET_C(IC6D.12, IC6D.13, R6.1, C6.1)
	NET_C(C6.2, GND)
	NET_C(IC6D.9, Q1.E)
	NET_C(Q1.B, R7.2)
	NET_C(R7.1, GND)
	NET_C(Q1.C, R17.2, C17.1, R16.1)
	NET_C(R17.1, GND)
	NET_C(C17.2, I_VM15)

	//
	// On/off switches
	//

	NET_C(I_OUT_3, IC3A.5, IC6C.3)
	NET_C(I_OUT_4, IC3A.10)

	NET_C(IC3A.1, IC4A.2, R18.1, C14.1)
	NET_C(IC4A.3, GND)
	NET_C(C14.2, R18.2, IC4A.6, R19.1)
	NET_C(R19.2, IC3A.14, IC6A.6)

#if (HLE_SHIP_VCO)
	//
	// This VCO is tricky to simulate, as there is both a frequency aspect
	// and an envelope aspect. The frequency is relatively straightforward.
	// Pick up the voltage from the anode of CR4 and map to a frequency
	// with a polynominal derived from the LLE implementation.
	//
	// Here is the mapping I get for CR4.A vs. IC3A.5 (using a -0.1-0.1 threshold):
	//    R2 = 0.99832: HP = (-0.000160539*A0) + 0.0000331984
	//    R2 = 0.99927: HP = (0.00000397232*A0*A0) - (0.000129210*A0) + 0.000083240
	//    R2 = 0.99927: HP = (-0.000000185528*A0*A0*A0) + (0.00000154157*A0*A0) - (0.000138889*A0) + 0.000071961
	//    R2 = 0.99929: HP = (-0.000000215021*A0*A0*A0*A0) - (0.00000393775*A0*A0*A0) - (0.0000213646*A0*A0) - (0.000196307*A0) + 0.0000221998
	//    R2 = 0.99931: HP = (0.000000153378*A0*A0*A0*A0*A0) + (0.00000317059*A0*A0*A0*A0) + (0.0000244935*A0*A0*A0) + (0.000091436*A0*A0) + (0.0000138241*A0) + 0.000169151
	//
	VARCLOCK(SHIPCLK, 1, "max(0.000001,min(0.1,(0.00000397232*A0*A0) - (0.000129210*A0) + 0.000083240))")
	NET_C(SHIPCLK.GND, GND)
	NET_C(SHIPCLK.VCC, I_V5)
	NET_C(SHIPCLK.A0, CR4.A)
	//
	// The envelope is trickier. When the signal is OFF (3A pin 5 is HIGH),
	// the envelope tracks the voltage from CR4. When the signal is ON
	// (3A pin 5 is LOW), the raw clock from SHIPCLK is clamped to -1..1
	// and scaled down by a constant factor with a small additional
	// envelope from the CR4 anode voltage.
	//
	AFUNC(SHIPENV, 3, "if(A2>2.5, -A1, (0.07-(0.005*A1))*if(A0>2.5,1,-1))")
	NET_C(SHIPENV.A0, SHIPCLK.Q)
	NET_C(SHIPENV.A1, CR4.A)
	NET_C(SHIPENV.A2, IC3A.5)
	NET_C(SHIPENV.Q, IC3A.2)
	NET_C(R34.2, R35.1)
	NET_C(R35.2, CR4.K)
	NET_C(R36.1, IC5B.6, R38.2, CR4.A)
	NET_C(GND, R34.1, R36.2, R37.1, R37.2, C9.1, C9.2, CR5.A, CR5.K, IC4B.2, IC4B.3, IC5A.2, IC5A.3)
#else
	NET_C(IC3A.2, C9.2)
	NET_C(C9.1, IC4B.2, R34.1)
	NET_C(IC4B.6, R34.2, R35.1)
	NET_C(IC4B.3, R35.2, CR4.K, CR5.A)
	NET_C(R36.1, IC5B.6, R38.2, CR4.A)
	NET_C(R36.2, IC5A.2, R37.1)
	NET_C(IC5A.3, GND)
	NET_C(R37.2, IC5A.6, CR5.K)
#endif

	NET_C(IC3A.13, R22.1)
	NET_C(R22.2, GND)
	ALIAS(OUTPUT, R22.1)

	//
	// K exit
	//

	NET_C(I_OUT_7, IC7C.1)
	NET_C(IC7C.2, R27.1, CR6.A)
	NET_C(R27.2, I_V5)

	NET_C(IC6C.4, IC7C.11)
	NET_C(IC7C.10, CR3.A, R26.1)
	NET_C(R26.2, I_V15, IC5D.14, IC5D.4, R29.2)
	NET_C(CR3.K, R28.2)
	NET_C(R28.1, CR6.K, C20.1, R33.1)
	NET_C(C20.2, GND)
	NET_C(R33.2, C27.1, R38.1, IC5B.2)
	NET_C(IC5D.1, R29.1, R31.2)
	NET_C(IC5D.2, IC5D.6, R31.1, R32.1, C21.1)
	NET_C(R32.2, C27.2)
	NET_C(C21.2, GND)
	NET_C(IC5B.3, GND)
	NET_C(IC5D.7, GND)

	// pin 5 (OUTPUT) of the 555 timer is not connected;
	// use this kludge to simulate that
	RES(RDUMMY, RES_K(100))
	NET_C(IC5D.5, RDUMMY.1)
	NET_C(RDUMMY.2, GND)

	//
	// Lazer 1
	//

	NET_C(I_OUT_1, IC6C.5)
	NET_C(IC6C.6, IC8E.14, IC7C.9)
	NET_C(IC7C.8, R39.1)
	NET_C(R39.2, C22.2, CR7.K)
	NET_C(C22.1, GND)

#if (HLE_LAZER_VCOS)
	//
	// This VCO is very difficult to simulate without cranking the speed up
	// and killing performance. Even at 1000x frequency, we still get failures
	// to converge. Here we clip the circuit at the diode CR7 and substitute
	// a VARCLOCK that directly drives the counter at 7E, skipping the analog
	// to TTL conversion logic after the VCO.
	//
	// Here is the mapping I get for C22.2 vs. IC7E.1 half-period:
	//
	//    R2 = 0.97399: HP = (0.00000249069*A0) + 0.00000439991
	//    R2 = 0.99638: HP = (0.000000142614*A0*A0) + (0.00000104196*A0) + 0.00000471406
	//    R2 = 0.99932: HP = (0.0000000174880*A0*A0*A0) - (0.000000159311*A0*A0) + (0.00000222025*A0) + 0.00000455815
	//    R2 = 0.99978: HP = (0.00000000229608*A0*A0*A0*A0) - (0.0000000377974*A0*A0*A0) + (0.000000245492*A0*A0) + (0.00000134883*A0) + 0.00000465357
	//    R2 = 0.99985: HP = (0.000000000286259*A0*A0*A0*A0*A0) - (0.00000000650969*A0*A0*A0*A0) + (0.0000000560876*A0*A0*A0) - (0.000000154720*A0*A0) + (0.00000190838*A0) + 0.0000045976
	//
	// One additional wrinkle is that when we clip the circuit, the voltage
	// input to the VCO changes from a curve to linear, so to compute the
	// mapping below, we had to map the C22.2 value from the clipped circuit
	// against the frequency. Fortunately, the relationship still held, and
	// in fact became almost linear.
	//
	// Here is the mapping for the clipped C22.2 vs. the original IC7E.1:
	//    R2 = 0.99947: HP = (0.000226684*A0) - 0.0000178774
	//    R2 = 0.99947: HP = (-0.0000111790*A0*A0) + (0.000230333*A0) - 0.0000181329
	//    R2 = 0.99958: HP = (-0.00124814*A0*A0*A0) + (0.000641071*A0*A0) + (0.000124020*A0) - 0.0000127688
	//    R2 = 0.99976: HP = (0.0197918*A0*A0*A0*A0) - (0.0148601*A0*A0*A0) + (0.00399181*A0*A0) - (0.000225308*A0) + 0.000000287255
	//    R2 = 0.99979: HP = (-0.095062*A0*A0*A0*A0*A0) + (0.093556*A0*A0*A0*A0) - (0.0361209*A0*A0*A0) + (0.00677578*A0*A0) - (0.000384926*A0) + 0.00000324102
	//
	VARCLOCK(LAZER1CLK, 1, "max(0.000001,min(0.1,(0.000226684*A0) - 0.0000178774))")
	NET_C(LAZER1CLK.GND, GND)
	NET_C(LAZER1CLK.VCC, I_V5)
	NET_C(LAZER1CLK.Q, IC7E.1)
	NET_C(LAZER1CLK.A0, C22.2)
	NET_C(GND, R40.1, R40.2, R41.1, R41.2, R42.1, R42.2, R43.1, R43.2, R44.1, R44.2, C23.1, C23.2, CR7.A, CR8.A, CR8.K, IC6C.9, IC6F.2, IC6F.3)
#else
	NET_C(CR7.A, IC6F.3, CR8.K, R43.1)
	NET_C(CR8.A, GND)
	NET_C(IC6F.2, C23.2, R40.1)
	NET_C(C23.1, GND)
	NET_C(IC6F.6, R40.2, R43.2, R41.1)
	NET_C(R41.2, R44.2, Q4.B)
	NET_C(R44.1, GND)
	NET_C(Q4.E, GND)
	NET_C(Q4.C, R42.1, IC6C.9)
	NET_C(R42.2, I_V5)
	NET_C(IC6C.8, IC7E.1)
#endif

	NET_C(IC7E.2, IC7E.12, GND)
	NET_C(IC7E.3, IC8E.7)
	NET_C(IC7E.4, IC8E.4)
	NET_C(IC7E.5, IC8E.6)
	NET_C(IC7E.6, IC8E.5, IC7E.13)
	NET_C(IC7E.8, IC8E.15)
	NET_C(IC7E.10, IC8E.2)
	NET_C(IC7E.11, IC8E.3)

	NET_C(IC8E.1, GND)
	NET_C(IC8E.13, GND)
	NET_C(IC8E.12, R51.1)
	NET_C(IC8E.11, R52.1)
	NET_C(IC8E.10, R53.1)
	NET_C(IC8E.9, R54.1)
	NET_C(R51.2, R52.2, R53.2, R54.2, R55.1, R56.2)
	NET_C(R56.1, GND)
	NET_C(R55.2, C26.1)
	NET_C(C26.2, IC6A.6)

	//
	// Lazer 2
	//

	NET_C(I_OUT_2, IC6C.1)
	NET_C(IC6C.2, IC5E.14, IC7C.13)
	NET_C(IC7C.12, R46.1)
	NET_C(R46.2, C24.2, CR9.K)
	NET_C(C24.1, GND)

#if (HLE_LAZER_VCOS)
	//
	// This VCO is identical to the one above, just using different components
	//
	VARCLOCK(LAZER2CLK, 1, "max(0.000001,min(0.1,(0.000226684*A0) - 0.0000178774))")
	NET_C(LAZER2CLK.GND, GND)
	NET_C(LAZER2CLK.VCC, I_V5)
	NET_C(LAZER2CLK.Q, IC4E.1)
	NET_C(LAZER2CLK.A0, C24.2)
	NET_C(GND, R45.1, R45.2, R47.1, R47.2, R48.1, R48.2, R49.1, R49.2, R50.1, R50.2, C25.1, C25.2, CR9.A, CR10.A, CR10.K, IC6C.13, IC6E.2, IC6E.3)
#else
	NET_C(CR9.A, IC6E.3, CR10.K, R48.1)
	NET_C(CR10.A, GND)
	NET_C(IC6E.2, C25.2, R45.1)
	NET_C(C25.1, GND)
	NET_C(IC6E.6, R45.2, R48.2, R47.1)
	NET_C(R47.2, R49.2, Q5.B)
	NET_C(R49.1, GND)
	NET_C(Q5.E, GND)
	NET_C(Q5.C, R50.1, IC6C.13)
	NET_C(R50.2, I_V5)
	NET_C(IC6C.12, IC4E.1)
#endif

	NET_C(IC4E.2, IC4E.12, GND)
	NET_C(IC4E.3, IC5E.7)
	NET_C(IC4E.4, IC5E.4)
	NET_C(IC4E.5, IC5E.6)
	NET_C(IC4E.6, IC5E.5, IC4E.13)
	NET_C(IC4E.8, IC5E.15)
	NET_C(IC4E.10, IC5E.2)
	NET_C(IC4E.11, IC5E.3)

	NET_C(IC5E.1, GND)
	NET_C(IC5E.13, GND)
	NET_C(IC5E.12, R57.1)
	NET_C(IC5E.11, R58.1)
	NET_C(IC5E.10, R59.1)
	NET_C(IC5E.9, R60.1)
	NET_C(R57.2, R58.2, R59.2, R60.2, R55.1)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC5D.8, IC5D.9, IC5D.10, IC5D.12, IC5D.13, IC7C.3, IC7C.5, IC9E.3, IC9E.4, IC9E.5, IC9E.6)

	//
	// Unconnected outputs
	//

	HINT(IC4E.9, NC)    // Q3
#if (HLE_LAZER_VCOS)
	HINT(IC6C.8, NC)    // QD
	HINT(IC6C.12, NC)   // QF
#endif
	HINT(IC7C.4, NC)    // QB
	HINT(IC7C.6, NC)    // QC
	HINT(IC7E.9, NC)    // Q3
	HINT(IC8C.4, NC)    // Q1
	HINT(IC8C.5, NC)    // Q2
	HINT(IC8C.6, NC)    // Q3
	HINT(IC8C.10, NC)   // Q4
	HINT(IC8C.11, NC)   // Q5
	HINT(IC8C.12, NC)   // Q6
	HINT(IC8D.5, NC)    // Q2
	HINT(IC8D.6, NC)    // Q3
	HINT(IC8D.10, NC)   // Q4
	HINT(IC8D.11, NC)   // Q5
	HINT(IC8D.12, NC)   // Q6
	HINT(IC9C.3, NC)    // Q0
	HINT(IC9C.4, NC)    // Q1
	HINT(IC9C.5, NC)    // Q2
	HINT(IC9C.6, NC)    // Q3
	HINT(IC9C.11, NC)   // Q5
	HINT(IC9D.3, NC)    // Q0
	HINT(IC9D.4, NC)    // Q1
	HINT(IC9D.5, NC)    // Q2
	HINT(IC9D.6, NC)    // Q3
	HINT(IC9D.11, NC)   // Q5
	HINT(IC9E.11, NC)   // Q3
	HINT(IC9E.12, NC)   // Q2
	HINT(IC9E.13, NC)   // Q1
	HINT(IC9E.14, NC)   // Q0

NETLIST_END()
