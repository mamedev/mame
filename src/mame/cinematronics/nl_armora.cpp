// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Armor Attack
//
// Derived from the schematics in the Armor Attack manual.
//
// Known problems/issues:
//
//    * Worked pretty well the first time.
//
//    * The squeak on the tank treads is not right, need to
//       understand what's going on.
//
//    * Entire schematic needs a verification pass.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_TANK_VCO (1)



//
// Main netlist
//

NETLIST_START(armora)
{

#if (HLE_TANK_VCO)
	SOLVER(Solver, 1000)
#else
	SOLVER(Solver, 4800000)
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

	RES(R1, RES_K(1))
	RES(R2, RES_K(1.3))
	RES(R3, RES_K(1.3))
	RES(R4, RES_K(2))
	RES(R5, RES_K(2))
	RES(R6, RES_K(12))
	RES(R7, RES_K(24))
	RES(R8, RES_K(1))
	RES(R9, RES_K(2))
	RES(R10, RES_K(1))
	RES(R11, RES_K(3.6))
	RES(R12, RES_K(10))
	RES(R13, RES_K(1))
	RES(R14, RES_K(240))
	RES(R15, RES_K(22))
	RES(R16, RES_K(22))
	RES(R17, RES_K(22))
	RES(R18, RES_K(20))
	RES(R19, RES_K(430))
	RES(R20, RES_M(1.5))
	RES(R21, RES_K(390))
	RES(R22, RES_M(1.5))
	RES(R23, RES_K(51))
//  RES(R24, RES_K(390))  -- part of final amp (not emulated)
//  RES(R25, RES_K(390))  -- part of final amp (not emulated)
	RES(R26, RES_M(1))
	RES(R27, RES_K(4.7))
	RES(R28, RES_K(2.7))
	RES(R29, RES_K(4.7))
	RES(R30, RES_K(39))
	RES(R31, RES_K(12))
	RES(R32, RES_K(1))
	RES(R33, RES_K(240))
	RES(R34, RES_K(18))
	RES(R35, RES_K(16))
	RES(R36, RES_K(7.5))
	RES(R37, 620)
	RES(R38, 620)
	RES(R39, RES_K(4.7))
	RES(R40, RES_K(2.7))
	RES(R41, RES_K(4.7))
	RES(R42, RES_K(39))
	RES(R43, RES_K(12))
	RES(R44, RES_K(1))
	RES(R45, RES_K(150))
	RES(R46, RES_K(20))
	RES(R47, RES_K(22))
	RES(R48, RES_K(30))
	RES(R49, 680)
	RES(R50, 680)
	RES(R51, RES_K(4.7))
	RES(R52, RES_K(2.7))
	RES(R53, RES_K(4.7))
	RES(R54, RES_K(39))
	RES(R55, RES_K(12))
	RES(R56, RES_K(1))
	RES(R57, RES_K(220))
	RES(R58, RES_K(10))
	RES(R59, RES_K(47))
	RES(R60, 750)
	RES(R61, 750)
	RES(R62, RES_K(4.7))
	RES(R63, RES_K(2.7))
	RES(R64, RES_K(4.7))
	RES(R65, RES_K(39))
	RES(R66, RES_K(12))
	RES(R67, RES_K(1))
	RES(R68, RES_K(910))
	RES(R69, RES_K(8.2))
	RES(R70, RES_K(43))
	RES(R71, RES_K(22))
	RES(R72, 750)
	RES(R73, 750)
	RES(R74, RES_K(4.7))
	RES(R75, RES_K(2.7))
	RES(R76, RES_K(4.7))
	RES(R77, RES_K(39))
	RES(R78, RES_K(12))
	RES(R79, RES_K(1))
	RES(R80, RES_K(750))
	RES(R81, RES_K(20))
	RES(R82, RES_K(8.2))
	RES(R83, RES_K(330))
//  RES(R84, RES_K(15))   -- part of final amp (not emulated)
//  RES(R85, 150)         -- part of final amp (not emulated)
//  RES(R86, RES_K(22))   -- part of final amp (not emulated)
//  RES(R87, 150)         -- part of final amp (not emulated)
//  RES(R88, 0.51)        -- part of final amp (not emulated)
//  RES(R89, 0.51)        -- part of final amp (not emulated)
	RES(R90, RES_K(100))
	RES(R91, RES_K(8.2))
	RES(R92, RES_K(20))
//  POT(R93, RES_K(10))   -- part of final amp (not emulated)
	RES(R94, RES_K(30))
	RES(R95, RES_K(4.7))
	RES(R96, RES_K(2.7))
	RES(R97, RES_K(39))
	RES(R98, RES_K(12))
	RES(R99, RES_K(2.4))
	RES(R100, RES_K(4.7))
	RES(R101, RES_K(2.7))
	RES(R102, RES_K(39))
	RES(R103, RES_K(12))
	RES(R104, RES_K(2.4))
	RES(R105, RES_K(4.7))
	RES(R106, RES_K(2.7))
	RES(R107, RES_K(39))
	RES(R108, RES_K(12))
	RES(R109, RES_K(2.4))
	RES(R110, RES_K(1))

//  CAP(C1, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C2, CAP_U(22))    -- part of voltage converter (not emulated)
//  CAP(C3, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C4, CAP_U(22))    -- part of voltage converter (not emulated)
//  CAP(C5, CAP_U(22))    -- part of voltage converter (not emulated)
//  CAP(C6, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C7, CAP_U(22))    -- part of voltage converter (not emulated)
//  CAP(C8, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C9, CAP_U(22))    -- part of voltage converter (not emulated)
//  CAP(C10, CAP_U(0.1))  -- part of voltage converter (not emulated)
	CAP(C11, CAP_U(0.047))
	CAP(C12, CAP_U(0.01))
	CAP(C13, CAP_U(0.047))
	CAP(C14, CAP_U(0.47))
	CAP(C15, CAP_U(0.001))
	CAP(C16, CAP_U(0.1))
	CAP(C17, CAP_U(0.0047))
	CAP(C18, CAP_U(2.2))
	CAP(C19, CAP_U(0.1))
	CAP(C20, CAP_U(100))
	CAP(C21, CAP_U(0.1))
	CAP(C22, CAP_U(2.2))
	CAP(C23, CAP_U(0.22))
	CAP(C24, CAP_U(0.22))
	CAP(C25, CAP_U(3.3))
	CAP(C26, CAP_U(0.1))
	CAP(C27, CAP_U(0.047))
//  CAP(C28, CAP_U())   -- don't see it anywhere
	CAP(C29, CAP_U(0.047))
	CAP(C30, CAP_U(0.047))
	CAP(C31, CAP_U(0.22))
	CAP(C32, CAP_U(0.22))
	CAP(C33, CAP_U(0.0047))
	CAP(C34, CAP_U(1))
	CAP(C35, CAP_U(0.1))
	CAP(C36, CAP_U(0.1))
	CAP(C37, CAP_U(0.01))
//  CAP(C38, CAP_U(0.68))  -- part of final amp (not emulated)
//  CAP(C39, CAP_P(470))   -- part of final amp (not emulated)
//  CAP(C40, CAP_P(470))   -- part of final amp (not emulated)
//  CAP(C41, CAP_U(0.005)) -- part of final amp (not emulated)
//  CAP(C42, CAP_P(470))   -- part of final amp (not emulated)
	CAP(C43, CAP_U(0.33))


//  D_1N4003(D1)        -- part of voltage converter (not emulated)
//  D_1N4003(D2)        -- part of voltage converter (not emulated)
//  D_1N4003(D3)        -- part of voltage converter (not emulated)
//  D_1N4003(D4)        -- part of voltage converter (not emulated)
	D_1N914(D5)
//  D_1N4003(D6)        -- part of final amp (not emulated)
//  D_1N4003(D7)        -- part of final amp (not emulated)
	D_1N914(D8)


//  Q_2N3904(Q1)            // NPN -- not used
	Q_2N3906(Q2)            // PNP
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3906(Q5)            // PNP
	Q_2N3906(Q6)            // PNP
	Q_2N3906(Q7)            // PNP
	Q_2N3906(Q8)            // PNP
	Q_2N3906(Q9)            // PNP
	Q_2N3906(Q10)           // PNP
	Q_2N3906(Q11)           // PNP
//  Q_2N6292(Q12)           // NPN -- part of final amp (not emulated)
//  Q_2N6107(Q13)           // PNP -- part of final amp (not emulated)
	Q_2N3906(Q14)           // PNP
	Q_2N3904(Q15)           // NPN
	Q_2N3906(Q16)           // PNP
	Q_2N3904(Q17)           // NPN
	Q_2N3906(Q18)           // PNP
	Q_2N3904(Q19)           // NPN

	TTL_7414_DIP(IC1)       // Hex Inverter
	NET_C(IC1.7, GND)
	NET_C(IC1.14, I_V5)

	TTL_74LS164_DIP(IC2)    // 8-bit Shift Reg.
	NET_C(IC2.7, GND)
	NET_C(IC2.14, I_V5)

	TTL_74LS377_DIP(IC3)    // Octal D Flip Flop
	NET_C(IC3.10, GND)
	NET_C(IC3.20, I_V5)

//  TTL_7815_DIP(IC4)       // +15V Regulator -- part of voltage converter (not emulated)
//  TTL_7915_DIP(IC5)       // -15V Regulator -- part of voltage converter (not emulated)

	LM555_DIP(IC6)

	TTL_7406_DIP(IC7)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC7.7, GND)
	NET_C(IC7.14, I_V5)

	TTL_74LS163_DIP(IC8)    // Binary Counter (schems say can sub a 74161)
	NET_C(IC8.8, GND)
	NET_C(IC8.16, I_V5)

	TTL_74LS00_DIP(IC9)     // Quad 4-Input NAND Gate
	NET_C(IC9.7, GND)
	NET_C(IC9.14, I_V5)

	TTL_74LS393_DIP(IC10)   // Dual 4-Stage Binary Counter
	NET_C(IC10.7, GND)
	NET_C(IC10.14, I_V5)

	TTL_74LS163_DIP(IC11)   // Binary Counter (schems say can sub a 74161)
	NET_C(IC11.8, GND)
	NET_C(IC11.16, I_V5)

	// IC12 was deleted from schematics

	TTL_74LS393_DIP(IC13)   // Dual 4-Stage Binary Counter
	NET_C(IC13.7, GND)
	NET_C(IC13.14, I_V5)

	// IC14 was deleted from schematics

#if (!HLE_TANK_VCO)
	LM566_DIP(IC15)
#endif

	// IC16 was deleted from schematics

	AMI_S2688(IC17)         // Noise generator

	TL081_DIP(IC18)         // Op. Amp.
	NET_C(IC18.7, I_V15)
	NET_C(IC18.4, I_VM15)

	CA3080_DIP(IC19)        // Op. Amp.
	NET_C(IC19.4, I_VM15)
	NET_C(IC19.7, I_V15)

	CA3080_DIP(IC20)        // Op. Amp.
	NET_C(IC20.4, I_VM15)
	NET_C(IC20.7, I_V15)

	CA3080_DIP(IC21)        // Op. Amp.
	NET_C(IC21.4, I_VM15)
	NET_C(IC21.7, I_V15)

	CA3080_DIP(IC22)        // Op. Amp.
	NET_C(IC22.4, I_VM15)
	NET_C(IC22.7, I_V15)

	LM555_DIP(IC23)

	TL081_DIP(IC24)         // Op. Amp.
	NET_C(IC24.7, I_V15)
	NET_C(IC24.4, I_VM15)

	TL081_DIP(IC25)         // Op. Amp.
	NET_C(IC25.7, I_V15)
	NET_C(IC25.4, I_VM15)

	TL081_DIP(IC26)         // Op. Amp.
	NET_C(IC26.7, I_V15)
	NET_C(IC26.4, I_VM15)

	TTL_7414_DIP(IC27)      // Hex Inverter
	NET_C(IC27.7, GND)
	NET_C(IC27.14, I_V5)

	//
	// Page 1: inputs and shift register
	//

	NET_C(I_OUT_1, IC27.13)
	ALIAS(TANK_EN, IC27.12)
	NET_C(I_OUT_2, IC27.9)
	ALIAS(BEEP_EN, IC27.8)
	NET_C(I_OUT_3, IC27.3)
	ALIAS(CHOPPER_SW, IC27.4)

	NET_C(I_V5, R1.1)
	NET_C(R1.2, IC2.9, IC2.1)
	ALIAS(HI, R1.2)
	NET_C(I_OUT_7, IC1.13)
	NET_C(IC1.12, IC1.1)
	NET_C(IC1.2, IC2.2)
	NET_C(I_OUT_4, IC1.11)
	NET_C(IC1.10, IC1.5)
	NET_C(IC1.6, IC2.8)
	NET_C(IC2.3, IC3.3)
	NET_C(IC2.4, IC3.4)
	NET_C(IC2.5, IC3.7)
	NET_C(IC2.6, IC3.8)
	NET_C(IC2.10, IC3.13)
	NET_C(IC2.11, IC3.14)
	NET_C(IC2.12, IC3.17)
	NET_C(IC2.13, IC3.18)

	NET_C(I_OUT_0, IC27.11)
	NET_C(IC27.10, IC27.5)
	NET_C(IC27.6, IC3.11)
	NET_C(IC3.1, GND)
	ALIAS(TANK_FIRE, IC3.2)
	ALIAS(HI_EXP, IC3.5)
	ALIAS(JEEP_FIRE, IC3.6)
	ALIAS(LO_EXP, IC3.9)
	NET_C(IC3.12, IC8.6)
	NET_C(IC3.15, IC8.5)
	NET_C(IC3.16, IC8.4)
	NET_C(IC3.19, IC8.3)

	//
	// Page 1: Tank EN
	//

	NET_C(I_V5, IC6.8, R2.2)
	NET_C(R2.1, IC6.7, R3.2)
	NET_C(R3.1, IC6.2, IC6.6, C11.1)
	NET_C(C11.2, GND)
	NET_C(TANK_EN, IC6.4, IC7.9)
	NET_C(IC6.1, GND)
	NET_C(IC6.5, C12.1)
	NET_C(C12.2, GND)
	NET_C(IC6.3, IC1.9)
	NET_C(IC1.8, IC8.2)
	NET_C(IC8.7, IC8.10, IC8.1, HI)
	NET_C(IC8.15, IC9.1, IC9.2, IC11.2, IC10.1)
	NET_C(IC8.9, IC9.3)
	NET_C(IC10.2, GND)
	NET_C(IC10.6, IC9.9, IC13.13)

	NET_C(IC11.1, IC11.10, IC11.7, IC11.3, IC11.4, HI)
	NET_C(IC11.5, IC11.6, GND)
	NET_C(IC11.15, IC1.3)
	NET_C(IC1.4, IC11.9)
	NET_C(IC11.11, IC9.10)
	NET_C(IC9.8, IC10.13)
	NET_C(IC10.12, GND)
	NET_C(IC10.11, R6.1)
	NET_C(R6.2, C13.1, C14.2)
	NET_C(C13.2, GND)
	NET_C(C14.1, R7.1)
	ALIAS(SJ, R7.2)

	NET_C(IC7.8, R8.1, IC13.12)
	NET_C(R8.2, I_V5)
	NET_C(IC13.8, R5.1, IC7.5)
	ALIAS(SH2_1, IC13.8)
	NET_C(R5.2, I_V5)
	NET_C(IC13.9, IC7.11, R9.1, IC7.13)
	ALIAS(SH2_3, IC13.9)
	NET_C(R9.2, I_V5)
	NET_C(IC7.10, IC7.6, R10.1)
	ALIAS(SH2_5, R10.1)
	NET_C(R10.2, I_V5)

	ALIAS(SH2_2, R19.1)
	ALIAS(SH2_4, R20.1)
	ALIAS(SH2_6, R21.1)
	NET_C(R19.2, R20.2, R21.2, R22.2, C18.1, R23.1)
	NET_C(R22.1, GND)
	NET_C(C18.2, GND)
	NET_C(R23.2, I_V15)

#if (HLE_TANK_VCO)
	//
	//    R2 = 0.98110: HP = (0.00000599036*A0) - 0.0000565124
	//    R2 = 0.99782: HP = (0.00000194885*A0*A0) - (0.0000415989*A0) + 0.000233746
	//    R2 = 0.99811: HP = (0.000000646112*A0*A0*A0) - (0.0000215063*A0*A0) + (0.000242010*A0) - 0.000908469
	//    R2 = 0.99589: HP = (0.000000217354*A0*A0*A0*A0) - (0.0000098166*A0*A0*A0) + (0.000167248*A0*A0) - (0.00127054*A0) + 0.00363402
	//    R2 = 0.92249: HP = (0.00000000630602*A0*A0*A0*A0*A0) - (0.000000220145*A0*A0*A0*A0) + (0.00000210638*A0*A0*A0) + (0.00000707526*A0*A0) - (0.000207037*A0) + 0.000836264
	//
	VARCLOCK(TANKCLK, 1, "max(0.000001,min(0.1,(0.000000646112*A0*A0*A0) - (0.0000215063*A0*A0) + (0.000242010*A0) - 0.000908469))")
	NET_C(TANKCLK.GND, GND)
	NET_C(TANKCLK.VCC, I_V5)
	NET_C(R19.2, TANKCLK.A0)
	NET_C(TANKCLK.Q, IC13.1)
	NET_C(GND, R4.1, R4.2, R11.1, R11.2, R12.1, R12.2, R13.1, R13.2, C15.1, C15.2, C16.1, C16.2, C17.1, C17.2, D5.A, D5.K, D8.A, D8.K)
#else
	NET_C(IC15.5, R19.2, C15.2)
	NET_C(IC15.7, C17.1)
	NET_C(C17.2, GND)
	NET_C(IC15.1, GND)
	NET_C(IC15.6, C15.1, R11.1)
	NET_C(R11.2, IC15.8, I_V15)
	NET_C(IC15.3, C16.2)
	NET_C(C16.1, R12.1, D5.K)
	NET_C(D5.A, GND)
	NET_C(R12.2, Q1.B)
	NET_C(Q1.E, D8.K, R4.2)
	NET_C(D8.A, GND)
	NET_C(R4.1, I_VM15)
	NET_C(Q1.C, IC13.1, R13.1)
	NET_C(R13.2, I_V5)
#endif

	NET_C(IC7.12, R110.1, IC13.2)
	NET_C(R110.2, I_V5)
	NET_C(IC13.5, R15.1)
	NET_C(IC13.4, R16.1)
	NET_C(IC13.3, R17.1)
	NET_C(R15.2, R16.2, R17.2, R18.2, C19.2)
	NET_C(R18.1, GND)
	NET_C(C19.1, R14.1)
	NET_C(R14.2, SJ)

	//
	// Page 2 stuff
	//

	NET_C(SH2_1, R96.1)
	NET_C(R96.2, R95.1, Q14.B)
	NET_C(R95.2, I_V5, Q14.E)
	NET_C(Q14.C, R97.2, R98.2, R99.1)
	NET_C(R97.1, I_VM15)
	NET_C(R98.1, GND, Q15.E)
	NET_C(R99.2, Q15.B)
	NET_C(Q15.C, SH2_2)

	NET_C(SH2_3, R101.1)
	NET_C(R101.2, R100.1, Q16.B)
	NET_C(R100.2, I_V5, Q16.E)
	NET_C(Q16.C, R102.2, R103.2, R104.1)
	NET_C(R102.1, I_VM15)
	NET_C(R103.1, GND, Q17.E)
	NET_C(R104.2, Q17.B)
	NET_C(Q17.C, SH2_4)

	NET_C(SH2_5, R106.1)
	NET_C(R106.2, R105.1, Q18.B)
	NET_C(R105.2, I_V5, Q18.E)
	NET_C(Q18.C, R107.2, R108.2, R109.1)
	NET_C(R107.1, I_VM15)
	NET_C(R108.1, GND, Q19.E)
	NET_C(R109.2, Q19.B)
	NET_C(Q19.C, SH2_6)

	//
	// Page 3
	//

	NET_C(I_V15, C20.1, IC17.4)
	NET_C(C20.2, GND)
	NET_C(IC17.2, IC17.1, GND, R26.1)
	NET_C(IC17.3, C21.2)
	NET_C(C21.1, R26.2, IC18.3)
	NET_C(IC18.2, IC18.6)
	ALIAS(NOISE, IC18.6)

	//
	// TANK FIRE
	//

	NET_C(TANK_FIRE, R27.1, R28.1)
	NET_C(R27.2, I_V5, R29.2, Q2.E)
	NET_C(R28.2, R29.1, Q2.B)
	NET_C(Q2.C, R30.2, R31.2, Q3.E)
	NET_C(R30.1, I_VM15)
	NET_C(R31.1, GND, R32.1)
	NET_C(R32.2, Q3.B)
	NET_C(Q3.C, C22.1, R33.1)
	NET_C(C22.2, I_VM15)
	NET_C(R33.2, IC19.5)
	NET_C(NOISE, R34.1)
	NET_C(R34.2, C23.1, R35.1)
	NET_C(R35.2, C24.1, R36.1)
	NET_C(C23.2, C24.2, GND, R37.1, R38.1)
	NET_C(R36.2, R37.2, IC19.2)
	NET_C(R38.2, IC19.3)
	NET_C(IC19.6, IC25.3)

	//
	// LO EXP
	//

	NET_C(LO_EXP, R39.1, R40.1)
	NET_C(R39.2, I_V5, R41.2, Q4.E)
	NET_C(R40.2, R41.1, Q4.B)
	NET_C(Q4.C, R42.2, R43.2, Q5.E)
	NET_C(R42.1, I_VM15)
	NET_C(R43.1, GND, R44.1)
	NET_C(R44.2, Q5.B)
	NET_C(Q5.C, C25.1, R45.1)
	NET_C(C25.2, I_VM15)
	NET_C(R45.2, IC20.5)
	NET_C(NOISE, R46.1)
	NET_C(R46.2, C26.1, R47.1)
	NET_C(R47.2, C27.1, R48.1)
	NET_C(C26.2, C27.2, GND, R49.1, R50.1)
	NET_C(R48.2, R49.2, IC20.2)
	NET_C(R50.2, IC20.3)
	NET_C(IC20.6, IC25.3)

	//
	// CHOPPER SW
	//

	NET_C(CHOPPER_SW, IC7.3)
	NET_C(IC7.4, R51.1, R52.1)
	NET_C(R51.2, I_V5, R53.2, Q6.E)
	NET_C(R52.2, R53.1, Q6.B)
	NET_C(Q6.C, R54.2, R55.2, Q7.E)
	NET_C(R54.1, I_VM15)
	NET_C(R55.1, R56.1, GND)
	NET_C(R56.2, Q7.B)
	NET_C(Q7.C, R57.1)
	NET_C(R57.2, IC21.5)
	NET_C(NOISE, R58.1)
	NET_C(R58.2, C29.1, R59.1)
	NET_C(R59.2, C30.1, R94.1)
	NET_C(C29.2, C30.2, GND, R60.1, R61.1)
	NET_C(R94.2, R60.2, IC21.2)
	NET_C(R61.2, IC21.3)
	NET_C(IC21.6, IC25.3)

	//
	// JEEP FIRE
	//

	NET_C(JEEP_FIRE, R62.1, R63.1)
	NET_C(R62.2, I_V5, R64.2, Q8.E)
	NET_C(R63.2, R64.1, Q8.B)
	NET_C(Q8.C, R65.2, R66.2, Q9.E)
	NET_C(R65.1, I_VM15)
	NET_C(R66.1, GND, R67.1)
	NET_C(R67.2, Q9.B)
	NET_C(Q9.C, C31.1, R68.1)
	NET_C(C31.2, I_VM15)
	NET_C(R68.2, R80.2, IC22.5)
	NET_C(NOISE, R69.1)
	NET_C(R69.2, C32.1, R70.1)
	NET_C(R70.2, C33.1, R71.1)
	NET_C(C32.2, C33.2, GND, R72.1, R73.1)
	NET_C(R71.2, R72.2, IC22.2)
	NET_C(R73.2, IC22.3)
	NET_C(IC22.6, IC25.3)

	//
	// HI EXP
	//

	NET_C(HI_EXP, R74.1, R75.1)
	NET_C(R74.2, I_V5, R76.2, Q10.E)
	NET_C(R75.2, R76.1, Q10.B)
	NET_C(Q10.C, R77.2, R78.2, Q11.E)
	NET_C(R77.1, I_VM15)
	NET_C(R78.1, GND, R79.1)
	NET_C(R79.2, Q11.B)
	NET_C(Q11.C, C34.1, R80.1)
	NET_C(C34.2, I_VM15)

	//
	// BEEP EN
	//

	NET_C(BEEP_EN, IC23.4)
	NET_C(I_V5, R81.2, IC23.8)
	NET_C(R81.1, R82.1, IC23.6, IC23.2, C35.1)
	NET_C(R82.2, IC23.7)
	NET_C(C35.2, GND)
	NET_C(IC23.1, GND)
	NET_C(IC23.5, C37.1)
	NET_C(C37.2, GND)
	NET_C(IC23.3, R83.1)
	NET_C(R83.2, C36.1)
	NET_C(C36.2, SJ)

	//
	// Final mix
	//

	NET_C(R90.2, IC25.3)
	NET_C(R90.1, GND)
	NET_C(IC25.2, IC25.6, C43.1)
	NET_C(C43.2, R91.1)
	NET_C(R91.2, IC26.2, SJ, R92.1)
	NET_C(IC26.3, GND)
	NET_C(IC26.6, R92.2)
	ALIAS(OUTPUT, R92.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC7.1, IC9.4, IC9.5, IC9.12, IC9.13, IC27.1, IC27.2)
	NET_C(GND, IC24.2, IC24.3)  // part of final amp

	//
	// Unconnected outputs
	//

	HINT(IC7.2, NC)
	HINT(IC8.11, NC)    // QD
	HINT(IC8.12, NC)    // QC
	HINT(IC8.13, NC)    // QB
	HINT(IC8.14, NC)    // QA
	HINT(IC9.6, NC)
	HINT(IC9.11, NC)
	HINT(IC10.3, NC)    // QA
	HINT(IC10.4, NC)    // QB
	HINT(IC10.5, NC)    // QC
	HINT(IC10.8, NC)    // QD
	HINT(IC10.9, NC)    // QC
	HINT(IC10.10, NC)   // QB
	HINT(IC11.12, NC)   // QC
	HINT(IC11.13, NC)   // QB
	HINT(IC11.14, NC)   // QA
	HINT(IC13.6, NC)
	HINT(IC13.10, NC)
	HINT(IC13.11, NC)
//    HINT(IC27.2, NC)

}
