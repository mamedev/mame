// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Boxing Bugs
//
// Derived from the schematics in the Boxing Bugs manual.
//
// Known problems/issues:
//
//    * WIP.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"

//
// Hacks/workarounds
//

#define HACK_SIMPLIFY_INPUTS (1)
#define HACK_VOLTAGE_SWITCH (1)



//
// Optimizations
//

#define HLE_SFX_CLOCK (1)
#define HLE_MUSIC_CLOCK (1)
#define HLE_CHIRPING_VCO (1)
#define HLE_DYING_BUG_VCO (1)
#define HLE_CRACKING_VCO (1)
#define HLE_BEETLE_VCO (1)
#define HLE_BOUNCE_VCO (1)
#define HLE_CANNON_VCO (1)
#define ENABLE_FRONTIERS (1)



//
// Main netlist
//

NETLIST_START(boxingb)

	SOLVER(Solver, 1000)
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

	ANALOG_INPUT(I_V2_2, 2.2)
	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

//  RES(R4, 620)             -- part of voltage converter (not emulated)
//  RES(R5, 620)             -- part of voltage converter (not emulated)
//  RES(R6, 430)             -- part of 2.2V voltage converter (not emulated)
//  RES(R7, 560)             -- part of 2.2V voltage converter (not emulated)
//  RES(R8, RES_K(3.3))      -- part of 2.2V voltage converter (not emulated)
	RES(R9, RES_K(1))
	RES(R10, RES_K(4.7))
	RES(R11, RES_K(1))
	RES(R12, RES_K(39))
	RES(R13, RES_K(12))
	RES(R14, RES_K(1))
	RES(R15, RES_K(4.7))
	RES(R16, RES_K(75))
	RES(R17, RES_K(75))
	RES(R18, 200)
	RES(R19, 200)
	RES(R20, RES_K(200))
	RES(R21, RES_K(4.7))
	RES(R22, RES_K(4.7))
	RES(R23, RES_K(2.7))
	RES(R24, RES_K(4.7))
	RES(R25, RES_K(39))
	RES(R26, RES_K(12))
	RES(R27, RES_K(2.4))
	RES(R28, RES_K(51))
	RES(R29, RES_K(270))
	RES(R30, RES_M(1))
	RES(R31, RES_K(16))
	RES(R32, RES_K(10))
	RES(R33, RES_K(1))
	RES(R34, RES_K(4.7))
	RES(R35, RES_K(2.7))
	RES(R36, RES_K(4.7))
	RES(R37, RES_K(39))
	RES(R38, RES_K(12))
	RES(R39, RES_K(2.4))
	RES(R40, RES_K(51))
	RES(R41, RES_K(270))
	RES(R42, RES_M(1))
	RES(R43, RES_K(11))
	RES(R44, RES_K(10))
	RES(R45, RES_K(1))
	RES(R46, RES_K(2))
	RES(R47, RES_K(1))
	RES(R48, RES_K(12))
	RES(R49, RES_K(1))
	RES(R50, RES_K(20))
	RES(R51, RES_K(20))
	RES(R52, RES_K(20))
	RES(R53, RES_K(20))
	RES(R54, RES_K(2))
	RES(R55, RES_K(4.7))
	RES(R56, RES_K(2.7))
	RES(R57, RES_K(4.7))
	RES(R58, RES_K(39))
	RES(R59, RES_K(12))
	RES(R60, RES_K(2.4))
	RES(R61, RES_K(51))
	RES(R62, RES_K(270))
	RES(R63, RES_M(1))
	RES(R64, RES_K(6.2))
	RES(R65, RES_K(10))
	RES(R66, RES_K(1))
	RES(R67, RES_K(2))
	RES(R68, RES_K(22))
	RES(R69, RES_K(39))
	RES(R70, RES_K(47))
	RES(R71, RES_K(1))
	RES(R72, RES_K(10))
	RES(R73, RES_K(2))
	RES(R74, RES_K(910))
	RES(R75, 390)
#if (HACK_VOLTAGE_SWITCH)
	RES(R76, 100)
#else
	RES(R76, RES_K(4.7))
#endif
	RES(R77, RES_K(2.7))
	RES(R78, RES_K(4.7))
	RES(R79, RES_K(39))
	RES(R80, RES_K(12))
	RES(R81, RES_K(2.4))
	RES(R82, RES_K(51))
	RES(R83, RES_K(270))
	RES(R84, RES_M(1))
	RES(R85, RES_K(12))
	RES(R86, RES_K(10))
	RES(R87, RES_K(1))
	RES(R88, RES_K(2))
	RES(R89, RES_K(20))
	RES(R90, RES_K(2))
	RES(R91, RES_K(20))
#if (HACK_VOLTAGE_SWITCH)
	RES(R92, 100)
#else
	RES(R92, RES_K(4.7))
#endif
	RES(R93, RES_K(2.7))
	RES(R94, RES_K(4.7))
	RES(R95, RES_K(39))
	RES(R96, RES_K(12))
	RES(R97, RES_K(2.4))
	RES(R98, RES_K(51))
	RES(R99, RES_K(330))
	RES(R100, RES_K(1))
	RES(R101, RES_M(1))
	RES(R102, 47)
	RES(R103, RES_K(4.7))
	RES(R104, RES_K(1))
	RES(R105, RES_K(39))
	RES(R106, RES_K(12))
	RES(R107, RES_K(1))
	RES(R108, RES_K(6.8))
	RES(R109, RES_K(100))
	RES(R110, RES_K(270))
	RES(R111, 100)
	RES(R112, 100)
	RES(R113, RES_K(4.7))
	RES(R114, RES_K(1))
	RES(R115, RES_K(39))
	RES(R116, RES_K(12))
	RES(R117, RES_K(2.4))
	RES(R118, RES_K(51))
	RES(R119, RES_K(270))
	RES(R120, RES_M(1))
	RES(R121, RES_K(10))
	RES(R122, RES_K(10))
	RES(R123, RES_K(1))
	RES(R124, RES_K(2))
	RES(R125, RES_K(20))
	RES(R126, RES_K(2))
	RES(R127, RES_K(3.9))
	RES(R128, 10)
	RES(R129, RES_M(1))
	RES(R130, RES_K(6.8))
	RES(R131, RES_K(18))
	RES(R132, RES_K(47))
	RES(R133, 390)
	RES(R134, 390)
	RES(R135, RES_K(4.7))
	RES(R136, RES_K(2.7))
	RES(R137, RES_K(4.7))
	RES(R138, RES_K(39))
	RES(R139, RES_K(12))
	RES(R140, RES_K(1))
	RES(R141, RES_K(75))
	RES(R142, RES_K(4.7))
	RES(R143, RES_K(2.7))
	RES(R144, RES_K(4.7))
	RES(R145, RES_K(39))
	RES(R146, RES_K(12))
	RES(R147, RES_K(1))
	RES(R148, RES_K(240))
	RES(R149, RES_K(300))
	RES(R150, RES_K(300))
//  RES(R151, RES_K(15))    -- part of final amp (not emulated)
//  RES(R152, 150)          -- part of final amp (not emulated)
//  RES(R153, 150)          -- part of final amp (not emulated)
//  RES(R154, RES_K(22))    -- part of final amp (not emulated)
//  RES(R155, RES_K(390))   -- part of final amp (not emulated)
//  RES(R156, 0.51)         -- part of final amp (not emulated)
//  RES(R157, 0.51)         -- part of final amp (not emulated)
//  RES(R158, RES_K(390))   -- part of final amp (not emulated)
	RES(R159, RES_K(30))
	RES(R160, RES_K(8.2))
	RES(R161, RES_K(20))
//  RES(R162, RES_K(10))    -- part of shaft encoder (not emulated)
//  RES(R163, RES_K(100))   -- part of shaft encoder (not emulated)
//  RES(R164, RES_M(1.8))   -- part of shaft encoder (not emulated)
//  RES(R165, RES_K(10))    -- part of shaft encoder (not emulated)
//  RES(R166, RES_K(2.4))   -- part of shaft encoder (not emulated)
//  RES(R167, RES_K(1))     -- part of shaft encoder (not emulated)
//  RES(R168, RES_K(470))   -- part of shaft encoder (not emulated)
//  RES(R169, RES_K(10))    -- part of shaft encoder (not emulated)
//  RES(R170, RES_K(470))   -- part of shaft encoder (not emulated)
//  RES(R171, RES_K(100))   -- part of shaft encoder (not emulated)
//  RES(R172, RES_M(1.8))   -- part of shaft encoder (not emulated)
//  RES(R173, RES_K(10))    -- part of shaft encoder (not emulated)
//  RES(R174, RES_K(2.4))   -- part of shaft encoder (not emulated)
//  RES(R175, RES_K(1))     -- part of shaft encoder (not emulated)
//  RES(R176, RES_K(1))     -- part of shaft encoder (not emulated)
	RES(R177, RES_K(1))
	RES(R178, 330)
	RES(R179, 330)
	RES(R180, RES_K(1))
	RES(R181, RES_K(1))
	RES(R182, RES_K(1))
	RES(R183, RES_K(4.7))
	RES(R184, RES_K(510))
	RES(R185, 470)
	RES(R186, 470)
	RES(R187, RES_K(4.7))
	RES(R188, RES_K(2.7))
	RES(R189, RES_K(4.7))
	RES(R190, RES_K(39))
	RES(R191, RES_K(12))
	RES(R192, RES_K(1))
	RES(R193, RES_K(24))
	RES(R194, RES_K(100))
	RES(R195, RES_K(4.7))
	RES(R196, RES_K(2.7))
	RES(R197, RES_K(4.7))
	RES(R198, RES_K(39))
	RES(R199, RES_K(12))
	RES(R200, RES_K(1))
	RES(R201, RES_K(51))
	RES(R202, RES_K(36))
	RES(R203, RES_K(1))
	RES(R204, RES_K(1))
	RES(R205, RES_K(2))
	RES(R206, RES_K(1))
	RES(R207, RES_K(1))
	RES(R208, RES_K(1))
	RES(R209, RES_K(1))

//  CAP(C1, CAP_U(0.1))     -- part of voltage converter (not emulated)
//  CAP(C2, CAP_U(0.1))     -- part of voltage converter (not emulated)
//  CAP(C3, CAP_U(22))      -- part of voltage converter (not emulated)
//  CAP(C4, CAP_U(22))      -- part of voltage converter (not emulated)
//  CAP(C5, CAP_U(22))      -- part of voltage converter (not emulated)
//  CAP(C6, CAP_U(22))      -- part of voltage converter (not emulated)
//  CAP(C7, CAP_U(22))      -- part of voltage converter (not emulated)
//  CAP(C8, CAP_U(0.1))     -- part of voltage converter (not emulated)
//  CAP(C9, CAP_U(0.1))     -- part of voltage converter (not emulated)
//  CAP(C10, CAP_U(0.1))    -- part of 2.2V voltage converter (not emulated)
	CAP(C11, CAP_U(0.1))
	CAP(C12, CAP_U(0.1))
	CAP(C13, CAP_U(1))
	CAP(C14, CAP_U(2.2))
	CAP(C15, CAP_U(0.001))
	CAP(C16, CAP_U(0.001))
	CAP(C17, CAP_U(0.1))
	CAP(C18, CAP_U(2.2))
	CAP(C19, CAP_U(0.001))
	CAP(C20, CAP_U(0.001))
	CAP(C21, CAP_U(0.1))
	CAP(C22, CAP_U(0.01))
	CAP(C23, CAP_U(0.1))
	CAP(C24, CAP_U(0.68))
	CAP(C25, CAP_U(0.001))
	CAP(C26, CAP_U(0.0022))
	CAP(C27, CAP_U(0.1))
	CAP(C28, CAP_U(0.1))
	CAP(C29, CAP_U(0.1))
	CAP(C30, CAP_U(0.01))
	CAP(C31, CAP_U(4.7))
	CAP(C32, CAP_U(0.001))
	CAP(C33, CAP_U(0.01))
	CAP(C34, CAP_U(0.1))
	CAP(C35, CAP_U(0.1))
	CAP(C36, CAP_U(2.2))
	CAP(C37, CAP_U(0.1))
	CAP(C38, CAP_U(0.01))
	CAP(C39, CAP_U(0.001))
	CAP(C40, CAP_U(0.47))
	CAP(C41, CAP_U(1))
	CAP(C42, CAP_U(0.1))
	CAP(C43, CAP_U(0.47))
	CAP(C44, CAP_U(0.001))
	CAP(C45, CAP_U(0.1))
	CAP(C46, CAP_U(0.1))
	CAP(C47, CAP_U(100))
	CAP(C48, CAP_U(0.1))
	CAP(C49, CAP_U(0.1))
	CAP(C50, CAP_U(0.01))
	CAP(C51, CAP_U(4.7))
	CAP(C52, CAP_U(0.47))
	CAP(C53, CAP_U(0.1))
	CAP(C54, CAP_U(0.1))
//  CAP(C55, CAP_U(0.68))
//  CAP(C56, CAP_U(0.005))  -- part of final amp (not emulated)
//  CAP(C57, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C58, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C59, CAP_P(470))    -- part of final amp (not emulated)
	CAP(C60, CAP_U(0.33))
	CAP(C61, CAP_P(330))
//  CAP(C62, CAP_U(0.1))    -- part of shaft encoder (not emulated)
//  CAP(C63, CAP_P(330))    -- part of shaft encoder (not emulated)
//  CAP(C64, CAP_P(330))    -- part of shaft encoder (not emulated)
//  CAP(C65, CAP_P(330))    -- part of shaft encoder (not emulated)
//  CAP(C66, CAP_P(330))    -- part of shaft encoder (not emulated)
	CAP(C67, CAP_U(0.001))
	CAP(C68, CAP_P(680))
	CAP(C69, CAP_U(0.001))
	CAP(C70, CAP_U(0.001))
	CAP(C71, CAP_U(0.68))
	CAP(C72, CAP_U(0.22))
	CAP(C73, CAP_U(0.1))
	CAP(C74, CAP_U(0.1))
	CAP(C75, CAP_U(0.01))

//  Q_2N3904(Q1)            // NPN -- part of 2.2V voltage converter (not emulated)
	Q_2N3906(Q2)            // PNP
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3904(Q5)            // NPN
#if !(HLE_CHIRPING_VCO)
	Q_2N3904(Q6)            // NPN
#endif
	Q_2N3906(Q7)            // PNP
	Q_2N3904(Q8)            // NPN
#if !(HLE_DYING_BUG_VCO)
	Q_2N3904(Q9)            // NPN
#endif
	Q_2N3906(Q10)           // PNP
	Q_2N3904(Q11)           // NPN
#if !(HLE_CRACKING_VCO)
	Q_2N3904(Q12)           // NPN
#endif
	Q_2N3906(Q13)           // PNP
	Q_2N3904(Q14)           // NPN
#if !(HLE_BEETLE_VCO)
	Q_2N3904(Q15)           // NPN
#endif
	Q_2N3906(Q16)           // PNP
	Q_2N3904(Q17)           // NPN
	Q_2N3906(Q18)           // PNP
	Q_2N3906(Q19)           // PNP
	Q_2N3906(Q20)           // PNP
	Q_2N3904(Q21)           // NPN
#if !(HLE_CANNON_VCO)
	Q_2N3904(Q22)           // NPN
#endif
	Q_2N3906(Q23)           // PNP
	Q_2N3906(Q24)           // PNP
	Q_2N3906(Q25)           // PNP
	Q_2N3906(Q26)           // PNP
//  Q_2N6292(Q27)           // PNP -- part of final amp (not emulated)
//  Q_2N6107(Q28)           // NPN -- part of final amp (not emulated)
//  Q_2N5210(Q29)           // NPN -- not used
//  Q_2N5210(Q30)           // NPN -- not used
	Q_2N3906(Q31)           // PNP
	Q_2N3906(Q32)           // PNP
	Q_2N3906(Q33)           // PNP
	Q_2N3906(Q34)           // PNP

//  D_1N4003(D1)            -- part of voltage converter (not emulated)
//  D_1N4003(D2)            -- part of voltage converter (not emulated)
//  D_1N4003(D3)            -- part of voltage converter (not emulated)
//  D_1N4003(D4)            -- part of voltage converter (not emulated)
//  D_1N914(D5)             -- part of voltage converter (not emulated)
//  D_1N914(D6)             -- part of voltage converter (not emulated)
//  D_1N5236(D7)            -- part of voltage converter (not emulated)
//  D_1N5236(D8)            -- part of voltage converter (not emulated)
//  D_1N5236(D9)            -- part of voltage converter (not emulated)
//  D_1N5236(D10)           -- part of voltage converter (not emulated)
//  D_1N914(D11)            -- part of voltage converter (not emulated)
//  D_1N914(D12)            -- part of voltage converter (not emulated)
	D_1N914(D13)
	D_1N914(D14)
	D_1N914(D15)
	D_1N914(D16)
	D_1N914(D17)
	D_1N914(D18)
	D_1N914(D19)
	D_1N914(D20)
	D_1N914(D21)
	D_1N914(D22)
//  D_1N4003(D23)           -- part of final amp (not emulated)
//  D_1N4003(D24)           -- part of final amp (not emulated)
	D_1N914(D25)
	D_1N914(D26)
	D_1N914(D27)
	D_1N914(D28)
	D_1N914(D29)
	D_1N914(D30)

#if (!HLE_MUSIC_CLOCK)
	CLOCK(Y1, 20000000)
	NET_C(Y1.GND, GND)
	NET_C(Y1.VCC, I_V5)
#endif

	TTL_74LS393_DIP(U1)     // Dual 4-Stage Binary Counter
	NET_C(U1.7, GND)
	NET_C(U1.14, I_V5)

	TTL_74S04_DIP(U2)       // Hex Inverting Gates
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

//  TTL_74LS74_DIP(U7)      // Dual D Flip Flop -- part of shaft encoder (not emulated)
//  NET_C(U7.7, GND)
//  NET_C(U7.14, I_V5)

//  TTL_74LS74_DIP(U8)      // Dual D Flip Flop -- part of shaft encoder (not emulated)
//  NET_C(U8.7, GND)
//  NET_C(U8.14, I_V5)

	TTL_74LS107_DIP(U9)     // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U9.7, GND)
	NET_C(U9.14, I_V5)

//  TTL_74LS74_DIP(U10)     // Dual D Flip Flop -- part of shaft encoder (not emulated)
//  NET_C(U10.7, GND)
//  NET_C(U10.14, I_V5)

	TTL_74LS164_DIP(U11)    // 8-bit Shift Reg.
	NET_C(U11.7, GND)
	NET_C(U11.14, I_V5)

	TTL_74LS86_DIP(U12)     // Quad 2-Input XOR Gates
	NET_C(U12.7, GND)
	NET_C(U12.14, I_V5)

	TTL_74LS393_DIP(U13)    // Dual 4-Stage Binary Counter
	NET_C(U13.7, GND)
	NET_C(U13.14, I_V5)

	TTL_74LS393_DIP(U15)    // Dual 4-Stage Binary Counter
	NET_C(U15.7, GND)
	NET_C(U15.14, I_V5)

//  TTL_74LS191_DIP(U16)    // Presettable 4-bit Binary Up/Down Counter -- part of shaft encoder (not emulated)
//  NET_C(U16.8, GND)
//  NET_C(U16.16, I_V5)

//  TTL_74LS191_DIP(U17)    // Presettable 4-bit Binary Up/Down Counter -- part of shaft encoder (not emulated)
//  NET_C(U17.8, GND)
//  NET_C(U17.16, I_V5)

//  TTL_74LS157_DIP(U18)    // Quad 2-Input Multiplexor -- part of shaft encoder (not emulated)
//  NET_C(U18.8, GND)
//  NET_C(U18.16, I_V5)

	LM555_DIP(U19)

	TTL_7414_DIP(U20)       // Hex Inverter
	NET_C(U20.7, GND)
	NET_C(U20.14, I_V5)

	TTL_74LS393_DIP(U21)    // Dual 4-Stage Binary Counter
	NET_C(U21.7, GND)
	NET_C(U21.14, I_V5)

	TTL_74LS393_DIP(U22)    // Dual 4-Stage Binary Counter
	NET_C(U22.7, GND)
	NET_C(U22.14, I_V5)

	TTL_74S113_DIP(U23)     // Dual JK Negative Edge-Trigged Flip Flop
	NET_C(U23.7, GND)
	NET_C(U23.14, I_V5)

	TTL_74LS74_DIP(U24)     // Dual D Flip Flop
	NET_C(U24.7, GND)
	NET_C(U24.14, I_V5)

	TTL_74LS74_DIP(U25)     // Dual D Flip Flop
	NET_C(U25.7, GND)
	NET_C(U25.14, I_V5)

	TTL_74LS393_DIP(U26)    // Dual 4-Stage Binary Counter
	NET_C(U26.7, GND)
	NET_C(U26.14, I_V5)

	TTL_74LS163_DIP(U27)    // Binary Counter
	NET_C(U27.8, GND)
	NET_C(U27.16, I_V5)

	TTL_74LS163_DIP(U28)    // Binary Counter
	NET_C(U28.8, GND)
	NET_C(U28.16, I_V5)

	TTL_74LS74_DIP(U29)     // Dual D Flip Flop
	NET_C(U29.7, GND)
	NET_C(U29.14, I_V5)

	TTL_7414_DIP(U30)       // Hex Inverter
	NET_C(U30.7, GND)
	NET_C(U30.14, I_V5)

	TTL_74LS04_DIP(U31)     // Hex Inverting Gates
	NET_C(U31.7, GND)
	NET_C(U31.14, I_V5)

	TTL_74LS393_DIP(U32)    // Dual 4-Stage Binary Counter
	NET_C(U32.7, GND)
	NET_C(U32.14, I_V5)

	TTL_74LS107_DIP(U33)    // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U33.7, GND)
	NET_C(U33.14, I_V5)

	TTL_74LS107_DIP(U34)    // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U34.7, GND)
	NET_C(U34.14, I_V5)

	TTL_74LS02_DIP(U35)     // Quad 2-input Nor Gate
	NET_C(U35.7, GND)
	NET_C(U35.14, I_V5)

	TTL_74LS163_DIP(U36)    // Binary Counter
	NET_C(U36.8, GND)
	NET_C(U36.16, I_V5)

	TTL_74LS163_DIP(U37)    // Binary Counter
	NET_C(U37.8, GND)
	NET_C(U37.16, I_V5)

	TTL_74LS377_DIP(U38)    // Octal D Flip Flop
	NET_C(U38.10, GND)
	NET_C(U38.20, I_V5)

	TTL_74LS164_DIP(U39)    // 8-bit Shift Reg.
	NET_C(U39.7, GND)
	NET_C(U39.14, I_V5)

	TTL_7406_DIP(U40)       // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(U40.7, GND)
	NET_C(U40.14, I_V5)

	TTL_74LS393_DIP(U41)    // Dual 4-Stage Binary Counter
	NET_C(U41.7, GND)
	NET_C(U41.14, I_V5)

	TTL_74LS393_DIP(U42)    // Dual 4-Stage Binary Counter
	NET_C(U42.7, GND)
	NET_C(U42.14, I_V5)

	TTL_74LS163_DIP(U43)    // Binary Counter
	NET_C(U43.8, GND)
	NET_C(U43.16, I_V5)

	TTL_74LS74_DIP(U44)     // Dual D Flip Flop
	NET_C(U44.7, GND)
	NET_C(U44.14, I_V5)

	TTL_74LS74_DIP(U45)     // Dual D Flip Flop
	NET_C(U45.7, GND)
	NET_C(U45.14, I_V5)

	TTL_74LS163_DIP(U46)    // Binary Counter
	NET_C(U46.8, GND)
	NET_C(U46.16, I_V5)

	TTL_74LS377_DIP(U47)    // Octal D Flip Flop
	NET_C(U47.10, GND)
	NET_C(U47.20, I_V5)

	TTL_74LS377_DIP(U48)    // Octal D Flip Flop
	NET_C(U48.10, GND)
	NET_C(U48.20, I_V5)

	TTL_74LS164_DIP(U49)    // 8-bit Shift Reg.
	NET_C(U49.7, GND)
	NET_C(U49.14, I_V5)

	TTL_7414_DIP(U50)       // Hex Inverter
	NET_C(U50.7, GND)
	NET_C(U50.14, I_V5)

#if (!HLE_CHIRPING_VCO)
	LM566_DIP(U51)          // 566 VCO
#endif

#if (!HLE_DYING_BUG_VCO)
	LM566_DIP(U52)          // 566 VCO
#endif

#if (!HLE_CRACKING_VCO)
	LM566_DIP(U53)          // 566 VCO
#endif

	TL081_DIP(U54)          // Op. Amp.
	NET_C(U54.7, I_V15)
	NET_C(U54.4, I_VM15)

	TL081_DIP(U55)          // Op. Amp.
	NET_C(U55.7, I_V15)
	NET_C(U55.4, I_VM15)

#if (!HLE_BEETLE_VCO)
	LM566_DIP(U56)          // 566 VCO
#endif

#if (!HLE_BOUNCE_VCO)
	LM566_DIP(U57)          // 566 VCO
#endif

	LM555_DIP(U58)

	CA3080_DIP(U59)         // Op. Amp.
	NET_C(U59.4, I_VM15)
	NET_C(U59.7, I_V15)

#if (!HLE_CANNON_VCO)
	LM566_DIP(U60)          // 566 VCO
#endif

	CA3080_DIP(U61)         // Op. Amp.
	NET_C(U61.4, I_VM15)
	NET_C(U61.7, I_V15)

	AMI_S2688(U62)          // Noise generator

	TL081_DIP(U63)          // Op. Amp.
	NET_C(U63.7, I_V15)
	NET_C(U63.4, I_VM15)

	CA3080_DIP(U64)         // Op. Amp.
	NET_C(U64.4, I_VM15)
	NET_C(U64.7, I_V15)

//  TL081_DIP(U65)          // Op. Amp. -- part of shaft encoder (not emulated)
//  NET_C(U65.7, I_V15)
//  NET_C(U65.4, I_VM15)

//  TL081_DIP(U66)          // Op. Amp. -- part of shaft encoder (not emulated)
//  NET_C(U66.7, I_V15)
//  NET_C(U66.4, I_VM15)

	CA3080_DIP(U67)         // Op. Amp.
	NET_C(U67.4, I_VM15)
	NET_C(U67.7, I_V15)

//  TTL_7815_DIP(U68)       // +15V Regulator -- part of voltage converter (not emulated)
//  TTL_7915_DIP(U69)       // -15V Regulator -- part of voltage converter (not emulated)

//  TL081_DIP(U70)          // Op. Amp. --  part of final amp (not emulated)
//  NET_C(U70.7, I_V15)
//  NET_C(U70.4, I_VM15)

	//
	// Page 1, top right
	//

	ALIAS(HIB_P, U27.6)
	NET_C(HIB_P, U27.5, U27.4, U27.1, U27.7)
	NET_C(GND, U27.3)
	ALIAS(BLOAD_M, U27.9)
	ALIAS(BCLK_P, U27.2)
	NET_C(U27.2, U26.1)
	NET_C(U27.10, U28.15)
	NET_C(U27.15, U24.12)
	HINT(U27.11, NC)
	HINT(U27.12, NC)
	HINT(U27.13, NC)
	HINT(U27.14, NC)

	NET_C(HIB_P, U28.6, U28.3, U28.1, U28.7, U28.10)
	NET_C(GND, U28.4, U28.5)
	NET_C(BLOAD_M, U28.9)
	ALIAS(ACLK_M, U28.2)
	HINT(U28.11, NC)
	HINT(U28.12, NC)
	HINT(U28.13, NC)
	HINT(U28.14, NC)

	ALIAS(HIA_P, U24.10)
	ALIAS(BCLK_M, U24.11)
	NET_C(U24.13, R9.2)
	NET_C(R9.1, I_V5)
	NET_C(U24.9, U34.12)
	NET_C(BLOAD_M, U24.8)

	NET_C(HIA_P, U34.1, U34.4, U34.13)
	NET_C(U34.3, R21.1, C11.1)
	ALIAS(_588USEC_P, U34.3)
	NET_C(R21.2, I_V5)
	HINT(U34.2, NC)

	NET_C(U26.2, GND)
	NET_C(U26.6, U26.13)
	HINT(U26.5, NC)
	HINT(U26.4, NC)
	HINT(U26.3, NC)
	NET_C(U26.12, GND)
	ALIAS(_327USEC_P, U26.8)
	HINT(U26.9, NC)
	HINT(U26.10, NC)
	NET_C(U26.11, R15.1, C12.2)
	NET_C(R15.2, I_V5)
	NET_C(C12.1, R16.1)
	NET_C(R16.2, R17.1, R18.2, U61.2)
	NET_C(R17.2, C11.2)
	NET_C(R18.1, GND)

	NET_C(U61.3, R19.2)
	NET_C(R19.1, GND)

	ALIAS(BELL_EN_M, R10.1)
	NET_C(R10.1, R11.1)
	NET_C(R10.2, I_V5)
	NET_C(R11.2, Q2.B)
	NET_C(Q2.E, I_V2_2)
	NET_C(Q2.C, R12.2, R13.2, Q3.E)
	NET_C(R12.1, I_VM15)
	NET_C(R13.1, GND)
	NET_C(Q3.B, R14.2)
	NET_C(R14.1, GND)
	NET_C(Q3.C, C13.1, R20.1)
	NET_C(C13.2, I_VM15)
	NET_C(R20.2, U61.5)
	ALIAS(CS, U61.6)

	//
	// Page 1, bottom-right
	//

#if (HLE_SFX_CLOCK)
	//
	// A 20MHz crystal (Y1) is divided by 4 by a pair of
	// JK flip flops (U23) to 5MHz. This is fed to a
	// 74LS393 counter (U13) and the divide-by-16 output
	// is divided again by a JK flip flop (U9) into a
	// 156250Hz counter. Skip the work of dividing this
	// manually and just create a clock directly.
	//
	CLOCK(SFXCLOCK, 156250)
	NET_C(SFXCLOCK.GND, GND)
	NET_C(SFXCLOCK.VCC, I_V5)
	NET_C(SFXCLOCK.Q, U29.3, U2.5)
	NET_C(U2.6, U29.11)
	NET_C(GND, U13.12, U13.13)
	NET_C(GND, U9.9, U9.10, U9.11)
#else
	NET_C(U25.5, GND)   // unused inverter borrowed
	HINT(U2.6, NC)      // for HLE SFX clock

	ALIAS(_5MHZ_M, U13.13)
	NET_C(U13.12, GND)
	NET_C(U13.8, U9.9)
	HINT(U13.9, NC)
	HINT(U13.10, NC)
	HINT(U13.11, NC)

	NET_C(U9.8, U9.10, U9.11)
	NET_C(U9.5, U29.3)
	NET_C(U9.6, U29.11)
#endif
	ALIAS(HID_P, U9.8)

	NET_C(U29.2, BCLK_M)
	NET_C(HIB_P, U29.4, U29.1)
	NET_C(U29.5, U29.12)
	ALIAS(ACLK_P, U29.5)
	NET_C(ACLK_M, U29.6)

	NET_C(HIB_P, U29.10, U29.13)
	NET_C(BCLK_P, U29.9)
	NET_C(BCLK_M, U29.8)

	//
	// Page 2, top (noise generator)
	//

	NET_C(_588USEC_P, U1.1)
	NET_C(U1.2, GND)
	NET_C(U1.6, U1.13)
	HINT(U1.5, NC)
	HINT(U1.4, NC)
	HINT(U1.3, NC)
	NET_C(U1.12, GND)
	HINT(U1.8, NC)
	HINT(U1.9, NC)
	NET_C(U1.10, U11.8, U32.1)
	HINT(U1.11, NC)

	NET_C(U11.9, U35.1)
	NET_C(U11.1, U11.2, U12.8)
	NET_C(U11.13, U12.1)
	ALIAS(RANDOM_NOISE_P, U11.13)
	HINT(U11.12, NC)
	HINT(U11.11, NC)
	HINT(U11.10, NC)
	NET_C(U11.6, U12.2)
	NET_C(U11.5, U12.5)
	NET_C(U11.4, U12.4)
	HINT(U11.3, NC)

	NET_C(U12.3, U12.13)
	NET_C(U12.6, U12.12)
	NET_C(U12.11, U32.2, U12.9)
	NET_C(U12.10, HID_P)
	ALIAS(IN_M, U12.8)

	NET_C(U32.6, U35.2, U35.3)
	HINT(U32.5, NC)
	HINT(U32.4, NC)
	HINT(U32.3, NC)
	ALIAS(ERRCLR_M, U35.1)

	//
	// Page 2, middle (Chirping Birds)
	//

	NET_C(RANDOM_NOISE_P, U31.5)
	NET_C(U31.6, R22.1, R23.1)
	NET_C(R22.2, I_V5, R24.2, Q4.E)
	NET_C(R23.2, R24.1, Q4.B)
	NET_C(Q4.C, R25.2, R26.2, R27.1)
	NET_C(R25.1, I_VM15)
	NET_C(R26.1, GND)
	NET_C(R27.2, Q5.B)
	NET_C(Q5.E, GND)
	NET_C(Q5.C, R29.1)
	NET_C(R29.2, R28.1, C14.1, R30.2, C15.2)
	NET_C(R28.2, I_V15)
	NET_C(C14.2, GND)
	NET_C(R30.1, GND)
	NET_C(C15.1, R31.1)
	NET_C(R31.2, I_V15)

#if (HLE_CHIRPING_VCO)
	//
	// Standard mapping:
	//    R2 = 0.91650: HP = (0.0000123028*A0) - 0.000136645
	//    R2 = 0.98600: HP = (0.00000750231*A0*A0) - (0.000183288*A0) + 0.00113698
	//    R2 = 0.99008: HP = (0.00000414207*A0*A0*A0) - (0.000154885*A0*A0) + (0.00193729*A0) - 0.0080873
	//    R2 = 0.98800: HP = (0.00000603082*A0*A0*A0*A0) - (0.000311647*A0*A0*A0) + (0.00604258*A0*A0) - (0.0520898*A0) + 0.168437
	//    R2 = 0.98586: HP = (0.000000081416*A0*A0*A0*A0*A0) - (0.00000284650*A0*A0*A0*A0) + (0.0000136226*A0*A0*A0) + (0.000571759*A0*A0) - (0.0083734*A0) + 0.0333905
	//
	VARCLOCK(CHIRPCLK, 1, "max(0.000001,min(0.1,(0.000000081416*A0*A0*A0*A0*A0) - (0.00000284650*A0*A0*A0*A0) + (0.0000136226*A0*A0*A0) + (0.000571759*A0*A0) - (0.0083734*A0) + 0.0333905))")
	NET_C(CHIRPCLK.GND, GND)
	NET_C(CHIRPCLK.VCC, I_V5)
	NET_C(CHIRPCLK.A0, C15.2)
	NET_C(CHIRPCLK.Q, U41.1)
	NET_C(GND, R32.1, R32.2, R33.1, R33.2, R205.1, R205.2, C16.1, C16.2, C17.1, C17.2, D13.A, D13.K, D14.A, D14.K)
#else
	NET_C(R29.2, U51.5)
	NET_C(C15.1, U51.6)
	NET_C(R31.2, U51.8)
	NET_C(U51.7, C16.1)
	NET_C(C16.2, GND)
	NET_C(U51.1, GND)
	HINT(U51.4, NC)
	NET_C(U51.3, C17.2)
	NET_C(C17.1, D13.K, R32.1)
	NET_C(D13.A, GND)
	NET_C(R32.2, Q6.B)
	NET_C(Q6.E, R205.2, D14.K)
	NET_C(R205.1, I_VM15)
	NET_C(D14.A, GND)
	NET_C(Q6.C, R33.1, U41.1)
	NET_C(R33.2, I_V5)
#endif

	NET_C(U41.2, RANDOM_NOISE_P)
	HINT(U41.6, NC)
	HINT(U41.5, NC)
	HINT(U41.4, NC)
	NET_C(U41.3, U41.13)

	ALIAS(CHIRPING_BIRDS_M, U41.12)
	HINT(U41.8, NC)
	HINT(U41.9, NC)
	NET_C(U41.10, R47.1, R48.1)
	HINT(U41.11, NC)
	NET_C(R47.2, I_V5)
	NET_C(R48.2, R49.2, C22.2)
	NET_C(R49.1, GND)
	ALIAS(SJ, C22.1)

	//
	// Page 2, bottom (Dying Bug)
	//

	ALIAS(BUG_DYING_M, U31.3)
	NET_C(U31.4, R34.1, R35.1)
	NET_C(R34.2, I_V5, R36.2, Q7.E)
	NET_C(R35.2, R36.1, Q7.B)
	NET_C(Q7.C, R37.2, R38.2, R39.1)
	NET_C(R37.1, I_VM15)
	NET_C(R38.1, GND)
	NET_C(R39.2, Q8.B)
	NET_C(Q8.E, GND)
	NET_C(Q8.C, R41.1)
	NET_C(R41.2, R40.1, C18.1, R42.2, C19.2)
	NET_C(R40.2, I_V15)
	NET_C(C18.2, GND)
	NET_C(R42.1, GND)
	NET_C(R43.2, I_V15)
	NET_C(C19.1, R43.1)

#if (HLE_DYING_BUG_VCO)
	//
	// Standard mapping:
	//    R2 = 0.94234: HP = (0.0000126953*A0) - 0.000142795
	//    R2 = 0.99216: HP = (0.0000085544*A0*A0) - (0.000211995*A0) + 0.00132411
	//    R2 = 0.99610: HP = (0.00000512381*A0*A0*A0) - (0.000194224*A0*A0) + (0.00245729*A0) - 0.0103626
	//    R2 = 0.99646: HP = (0.00000347185*A0*A0*A0*A0) - (0.000178231*A0*A0*A0) + (0.00343236*A0*A0) - (0.0293814*A0) + 0.094321
	//    R2 = 0.99644: HP = (0.000000064673*A0*A0*A0*A0*A0) - (0.00000153002*A0*A0*A0*A0) - (0.0000269128*A0*A0*A0) + (0.00118283*A0*A0) - (0.0128917*A0) + 0.0465240
	//
	VARCLOCK(DYINGCLK, 1, "max(0.000001,min(0.1,(0.000000064673*A0*A0*A0*A0*A0) - (0.00000153002*A0*A0*A0*A0) - (0.0000269128*A0*A0*A0) + (0.00118283*A0*A0) - (0.0128917*A0) + 0.0465240))")
	NET_C(DYINGCLK.GND, GND)
	NET_C(DYINGCLK.VCC, I_V5)
	NET_C(DYINGCLK.A0, C19.2)
	NET_C(DYINGCLK.Q, U42.1)
	NET_C(GND, R44.1, R44.2, R45.1, R45.2, R46.1, R46.2, C20.1, C20.2, C21.1, C21.2, D15.A, D15.K, D16.A, D16.K)
#else
	NET_C(C19.2, U52.5)
	NET_C(C19.1, U52.6)
	NET_C(R43.2, U52.8)
	NET_C(U52.7, C20.1)
	NET_C(C20.2, GND)
	NET_C(U52.1, GND)
	HINT(U52.4, NC)
	NET_C(U52.3, C21.2)
	NET_C(C21.1, D15.K, R44.1)
	NET_C(D15.A, GND)
	NET_C(R44.2, Q9.B)
	NET_C(Q9.E, R46.2, D16.K)
	NET_C(R46.1, I_VM15)
	NET_C(D16.A, GND)
	NET_C(Q9.C, R45.1, U42.1)
	NET_C(R45.2, I_V5)
#endif

	NET_C(_327USEC_P, U32.13)
	NET_C(U32.12, GND)
	ALIAS(_5232USEC_P, U32.8)
	ALIAS(_2616USEC_P, U32.9)
	NET_C(U32.9, U42.2)
	HINT(U32.10, NC)
	HINT(U32.11, NC)

	HINT(U42.6, NC)
	HINT(U42.5, NC)
	HINT(U42.4, NC)
	NET_C(U42.3, U42.13)
	NET_C(U42.12, BUG_DYING_M)
	NET_C(U42.8, R51.1)
	NET_C(U42.9, R52.1)
	NET_C(U42.10, R53.1)
	HINT(U42.11, NC)
	NET_C(R54.2, R53.2, R52.2, R51.2, R50.1)
	NET_C(R54.1, GND)
	NET_C(R50.2, C23.2)
	NET_C(C23.1, SJ)

	//
	// Page 3, top (Egg Cracking)
	//

	ALIAS(EGG_CRACKING_M, U31.13)
	NET_C(U31.12, R55.1, R56.1)
	NET_C(R55.2, I_V5, R57.2, Q10.E)
	NET_C(R56.2, R57.1, Q10.B)
	NET_C(Q10.C, R58.2, R59.2, R60.1)
	NET_C(R58.1, I_VM15)
	NET_C(R59.1, GND)
	NET_C(Q11.E, GND)
	NET_C(R60.2, Q11.B)
	NET_C(Q11.C, R62.1)
	NET_C(R62.2, R61.1, C24.1, R63.2, C25.2)
	NET_C(C24.2, GND)
	NET_C(R63.1, GND)
	NET_C(R61.2, R64.2, I_V15)
	NET_C(C25.1, R64.1)

#if (HLE_CRACKING_VCO)
	//
	// Standard mapping:
	//    R2 = 0.89585: HP = (0.0000175988*A0) - 0.000207278
	//    R2 = 0.98810: HP = (0.0000104596*A0*A0) - (0.000261248*A0) + 0.00164623
	//    R2 = 0.99788: HP = (0.00000629669*A0*A0*A0) - (0.000240194*A0*A0) + (0.00305980*A0) - 0.0129994
	//    R2 = 0.99873: HP = (0.00000350292*A0*A0*A0*A0) - (0.000179267*A0*A0*A0) + (0.00344234*A0*A0) - (0.0293869*A0) + 0.094097
	//    R2 = 0.99653: HP = (0.000000075716*A0*A0*A0*A0*A0) - (0.00000154725*A0*A0*A0*A0) - (0.0000446414*A0*A0*A0) + (0.00164941*A0*A0) - (0.0174578*A0) + 0.0623765
	//
	VARCLOCK(CRACKINGCLK, 1, "max(0.000001,min(0.1,(0.000000064673*A0*A0*A0*A0*A0) - (0.00000153002*A0*A0*A0*A0) - (0.0000269128*A0*A0*A0) + (0.00118283*A0*A0) - (0.0128917*A0) + 0.0465240))")
	NET_C(CRACKINGCLK.GND, GND)
	NET_C(CRACKINGCLK.VCC, I_V5)
	NET_C(CRACKINGCLK.A0, C25.2)
	NET_C(CRACKINGCLK.Q, U22.13)
	NET_C(GND, R65.1, R65.2, R66.1, R66.2, R67.1, R67.2, C26.1, C26.2, C27.1, C27.2, D17.A, D17.K, D18.A, D18.K)
#else
	NET_C(R61.2, U53.8)
	NET_C(C25.1, U53.6)
	NET_C(R62.2, U53.5)
	NET_C(U53.7, C26.1)
	NET_C(C26.2, GND)
	NET_C(U53.1, GND)
	HINT(U53.4, NC)
	NET_C(U53.3, C27.2)
	NET_C(C27.1, D17.K, R65.1)
	NET_C(D17.A, GND)
	NET_C(R65.2, Q12.B)
	NET_C(Q12.E, R67.2, D18.K)
	NET_C(R67.1, I_VM15)
	NET_C(D18.A, GND)
	NET_C(Q12.C, R66.1, U22.13)
	NET_C(R66.2, I_V5)
#endif

	NET_C(EGG_CRACKING_M, U22.12)
	NET_C(U22.8, R68.1)
	NET_C(U22.9, R69.1)
	NET_C(U22.10, R70.1)
	HINT(U22.11, NC)
	NET_C(R68.2, R69.2, R70.2, R71.2, R72.1)
	NET_C(R71.1, GND)
	NET_C(R72.2, C28.2)
	NET_C(C28.1, SJ)

	//
	// Page 3, middle (Beetle on screen)
	//

	ALIAS(BEETLE_ON_SCREEN_M, U31.1)
	NET_C(U31.2, U19.4)
	NET_C(U19.8, R73.2, I_V5)
	NET_C(R73.1, U19.7, R74.2)
	NET_C(R74.1, U19.6, U19.2, C29.1)
	NET_C(C29.2, GND)
	NET_C(U19.1, GND)
	NET_C(U19.5, C30.1)
	NET_C(C30.2, GND)
	NET_C(U19.3, R75.2, U20.1)
	NET_C(R75.1, GND)

	NET_C(U20.2, R77.1, R76.1)
	NET_C(R76.2, I_V5, R78.2, Q13.E)
	NET_C(R77.2, R78.1, Q13.B)
	NET_C(Q13.C, R79.2, R80.2, R81.1)
	NET_C(R79.1, I_VM15)
	NET_C(R80.1, GND)
	NET_C(R81.2, Q14.B)
	NET_C(Q14.E, GND)
	NET_C(Q14.C, R83.1)
	NET_C(R83.2, R82.1, C31.1, R84.2, C32.2)
	NET_C(C31.2, GND)
	NET_C(R84.1, GND)
	NET_C(R82.2, I_V15, R85.2)
	NET_C(R85.1, C32.1)

#if (HLE_BEETLE_VCO)
	//
	// Standard mapping:
	//    R2 = 0.92906: HP = (0.000127227*A0) - 0.00149583
	//    R2 = 0.97372: HP = (0.000086592*A0*A0) - (0.00218553*A0) + 0.0139420
	//    R2 = 0.97589: HP = (0.000063925*A0*A0*A0) - (0.00248748*A0*A0) + (0.0323532*A0) - 0.140489
	//    R2 = 0.97153: HP = (-0.00000615887*A0*A0*A0*A0) + (0.000395273*A0*A0*A0) - (0.0091707*A0*A0) + (0.092250*A0) - 0.341743
	//    R2 = 0.96812: HP = (0.00000145563*A0*A0*A0*A0*A0) - (0.0000505655*A0*A0*A0*A0) + (0.000151855*A0*A0*A0) + (0.0134189*A0*A0) - (0.189302*A0) + 0.763397
	//
	VARCLOCK(BEETLECLK, 1, "max(0.000001,min(0.1,(0.00000145563*A0*A0*A0*A0*A0) - (0.0000505655*A0*A0*A0*A0) + (0.000151855*A0*A0*A0) + (0.0134189*A0*A0) - (0.189302*A0) + 0.763397))")
	NET_C(BEETLECLK.GND, GND)
	NET_C(BEETLECLK.VCC, I_V5)
	NET_C(BEETLECLK.A0, C32.2)
	NET_C(BEETLECLK.Q, U22.1)
	NET_C(GND, R86.1, R86.2, R87.1, R87.2, R88.1, R88.2, C33.1, C33.2, C34.1, C34.2, D19.A, D19.K, D20.A, D20.K)
#else
	NET_C(R83.2, U56.5)
	NET_C(R82.2, U56.8)
	NET_C(R85.1, U56.6)
	NET_C(U56.7, C33.1)
	NET_C(C33.2, GND)
	NET_C(U56.1, GND)
	HINT(U56.4, NC)
	NET_C(U56.3, C34.2)
	NET_C(C34.1, D19.K, R86.1)
	NET_C(D19.A, GND)
	NET_C(R86.2, Q15.B)
	NET_C(Q15.E, R88.2, D20.K)
	NET_C(R88.1, I_VM15)
	NET_C(D20.A, GND)
	NET_C(Q15.C, R87.1, U22.1)
	NET_C(R87.2, I_V5)
#endif

	NET_C(BEETLE_ON_SCREEN_M, U22.2)
	HINT(U22.6, NC)
	HINT(U22.5, NC)
	NET_C(U22.4, R89.1)
	HINT(U22.3, NC)
	NET_C(R89.2, R90.2, R91.1)
	NET_C(R90.1, GND)
	NET_C(R91.2, C35.2)
	NET_C(C35.1, SJ)

	//
	// Page 3, bottom (BOUNCE)
	//

	ALIAS(BOUNCE_EN_P, R93.1)
	NET_C(R93.1, R92.1)
	NET_C(R92.2, R94.2, I_V5, Q16.E)
	NET_C(R93.2, R94.1, Q16.B)
	NET_C(Q16.C, R95.2, R96.2, R97.1)
	NET_C(R95.1, I_VM15)
	NET_C(R96.1, GND)
	NET_C(R97.2, Q17.B)
	NET_C(Q17.E, GND)
	NET_C(Q17.C, R99.1)
	NET_C(R99.2, R98.1, C36.1, R101.2, C39.2, R102.2)
	NET_C(R98.2, I_V15)
	NET_C(C36.2, GND)
	NET_C(R101.1, GND)

	NET_C(_5232USEC_P, R100.1, U58.6, U58.2)
	NET_C(R100.2, I_V5)
	NET_C(U58.7, GND)   // not connected
	NET_C(U58.4, U58.8, I_V5)
	NET_C(U58.1, GND)
	NET_C(U58.5, C38.1)
	NET_C(C38.2, GND)
	NET_C(U58.3, C37.2)
	NET_C(C37.1, R102.1)

	NET_C(C39.1, R108.1)
	NET_C(R108.2, I_V15)
#if (HLE_BOUNCE_VCO)
	//
	// Unlike all the other VCOs, this one doesn't go directly into a
	// TTL device. The actual square wave is tapered from the top and
	// slightly modulated with the frequency, but it makes little
	// practical difference, so we just scale the output square wave
	// to +/-5V as input to R109.
	//
	// Standard mapping:
	//    R2 = 0.89933: HP = (0.00476268*A0) - 0.0576442
	//    R2 = 0.97773: HP = (0.00311005*A0*A0) - (0.079281*A0) + 0.509096
	//    R2 = 0.97985: HP = (0.00115078*A0*A0*A0) - (0.0435380*A0*A0) + (0.550407*A0) - 2.321385
	//    R2 = 0.97985: HP = (0.0000369937*A0*A0*A0*A0) - (0.000849582*A0*A0*A0) - (0.00300315*A0*A0) + (0.185593*A0) - 1.090973
	//    R2 = 0.24613: HP = (-0.000199982*A0*A0*A0*A0*A0) + (0.0134882*A0*A0*A0*A0) - (0.362557*A0*A0*A0) + (4.857498*A0*A0) - (32.45284*A0) + 86.5278
	//
	VARCLOCK(BOUNCECLK, 1, "max(0.000001,min(0.1,((0.00115078*A0*A0*A0) - (0.0435380*A0*A0) + (0.550407*A0) - 2.321385)))")
	NET_C(BOUNCECLK.GND, GND)
	NET_C(BOUNCECLK.VCC, I_V5)
	NET_C(BOUNCECLK.A0, C39.2)
	NET_C(BOUNCECLK.Q, BOUNCEENV.A0)
	AFUNC(BOUNCEENV, 1, "if(A0>2.5,5,-5)")
	NET_C(BOUNCEENV, R109.1)
	NET_C(GND, C40.1, C40.2)
	NET_C(GND, C42.1, C42.2)
#else
	NET_C(C39.1, U57.6)
	NET_C(R108.2, U57.8)
	NET_C(R99.2, U57.5)
	NET_C(C39.1, U57.6, R108.1)
	NET_C(R108.2, I_V15, U57.8)
	NET_C(U57.7, C40.1)
	NET_C(C40.2, GND)
	NET_C(U57.1, GND)
	HINT(U57.4, NC)
	NET_C(U57.3, C42.2)
	NET_C(C42.1, R109.1)
#endif

	NET_C(R109.2, R111.2, U59.2)
	NET_C(R111.1, GND)
	NET_C(U59.3, R112.2)
	NET_C(R112.1, GND)

	ALIAS(BOUNCE_EN_M, R104.1)
	NET_C(R104.1, R103.1)
	NET_C(R103.2, I_V5)
	NET_C(R104.2, Q18.B)
	NET_C(Q18.E, I_V2_2)
	NET_C(Q18.C, R105.2, R106.2, Q19.E)
	NET_C(R105.1, I_VM15)
	NET_C(R106.1, GND)
	NET_C(Q19.B, R107.2)
	NET_C(R107.1, GND)
	NET_C(Q19.C, C41.1, R110.1)
	NET_C(C41.2, I_VM15)
	NET_C(R110.2, U59.5)
	NET_C(U59.6, CS)

	//
	// Page 4, top (Cannon)
	//

	ALIAS(CANNON_M, R114.1)
	NET_C(R114.1, R113.1)
	NET_C(R113.2, I_V5)
	NET_C(R114.2, Q20.B)
	NET_C(Q20.E, I_V2_2)
	NET_C(Q20.C, R115.2, R116.2, R117.1)
	NET_C(R115.1, I_VM15)
	NET_C(R116.1, GND)
	NET_C(R117.2, Q21.B)
	NET_C(Q21.E, GND)
	NET_C(Q21.C, R119.1)
	NET_C(R119.2, R118.1, C43.1, R120.2, C44.2)
	NET_C(R118.2, I_V15, R121.2)
	NET_C(C43.2, GND)
	NET_C(R120.1, GND)
	NET_C(C44.1, R121.1)

#if (HLE_CANNON_VCO)
	//
	// Standard mapping:
	//    R2 = 0.96910: HP = (0.000125667*A0) - 0.00142938
	//    R2 = 0.99026: HP = (0.000076462*A0*A0) - (0.00189006*A0) + 0.0117720
	//    R2 = 0.99181: HP = (0.0000429163*A0*A0*A0) - (0.00161661*A0*A0) + (0.0203246*A0) - 0.085173
	//    R2 = 0.99203: HP = (0.0000167801*A0*A0*A0*A0) - (0.000839548*A0*A0*A0) + (0.0157634*A0*A0) - (0.131604*A0) + 0.412204
	//    R2 = 0.99209: HP = (0.00000283657*A0*A0*A0*A0*A0) - (0.000141443*A0*A0*A0*A0) + (0.00257240*A0*A0*A0) - (0.0192167*A0*A0) + (0.0334224*A0) + 0.148265
	//
	VARCLOCK(CANNONCLK, 1, "max(0.000001,min(0.1,(0.00000283657*A0*A0*A0*A0*A0) - (0.000141443*A0*A0*A0*A0) + (0.00257240*A0*A0*A0) - (0.0192167*A0*A0) + (0.0334224*A0) + 0.148265))")
	NET_C(CANNONCLK.GND, GND)
	NET_C(CANNONCLK.VCC, I_V5)
	NET_C(CANNONCLK.A0, C44.2)
	NET_C(CANNONCLK.Q, U13.1)
	NET_C(GND, R122.1, R122.2, R123.1, R123.2, R124.1, R124.2, C75.1, C75.2, C45.1, C45.2, D21.A, D21.K, D22.A, D22.K)
#else
	NET_C(R119.2, U60.5)
	NET_C(R118.2, U60.8)
	NET_C(C44.1, U60.6)
	NET_C(U60.7, C75.1)
	NET_C(C75.2, GND)
	NET_C(U60.1, GND)
	HINT(U60.4, NC)
	NET_C(U60.3, C45.2)
	NET_C(C45.1, D21.K, R122.1)
	NET_C(D21.A, GND)
	NET_C(R122.2, Q22.B)
	NET_C(Q22.E, R124.2, D22.K)
	NET_C(R124.1, I_VM15)
	NET_C(D22.A, GND)
	NET_C(Q22.C, R123.1, U13.1)
	NET_C(R123.2, I_V5)
#endif

	NET_C(CANNON_M, U13.2)
	HINT(U13.6, NC)
	HINT(U13.5, NC)
	NET_C(U13.4, R125.1)
	HINT(U13.3, NC)
	NET_C(R125.2, R126.1, R127.2)
	NET_C(R126.2, GND)
	NET_C(R127.1, C46.2)
	NET_C(C46.1, SJ)

	//
	// Page 4, bottom-left (explosions)
	//

	NET_C(U62.2, GND)
	NET_C(U62.1, GND)
	NET_C(U62.4, R128.1, C47.1, C74.1)
	NET_C(R128.2, I_V15)
	NET_C(C47.2, GND)
	NET_C(C74.2, GND)
	NET_C(U62.3, C48.2)
	NET_C(C48.1, R129.2, U63.3)
	NET_C(R129.1, GND)
	NET_C(U63.2, U63.6, R130.1)
	NET_C(R130.2, C49.1, R131.1)
	NET_C(C49.2, GND)
	NET_C(R131.2, C50.1, R132.1)
	NET_C(C50.2, GND)
	NET_C(R132.2, R133.2, U64.2)
	NET_C(R133.1, GND)
	NET_C(U64.3, R134.2)
	NET_C(R134.1, GND)
	NET_C(U64.6, CS)

	ALIAS(LOUD_EXPL_M, R136.1)
	NET_C(R136.1, R135.1)
	NET_C(R135.2, I_V5, R137.2, Q23.E)
	NET_C(R136.2, R137.1, Q23.B)
	NET_C(Q23.C, R138.2, R139.2, Q24.E)
	NET_C(R138.1, I_VM15)
	NET_C(R139.1, GND)
	NET_C(Q24.B, R140.2)
	NET_C(R140.1, GND)
	NET_C(Q24.C, C51.1, R141.1)
	NET_C(C51.2, I_VM15)
	NET_C(R141.2, R148.2, U64.5)

	ALIAS(SOFT_EXPL_M, R143.1)
	NET_C(R143.1, R142.1)
	NET_C(R142.2, I_V5, R144.2, Q25.E)
	NET_C(R143.2, R144.1, Q25.B)
	NET_C(Q25.C, R145.2, R146.2, Q26.E)
	NET_C(R145.1, I_VM15)
	NET_C(R146.1, GND)
	NET_C(Q26.B, R147.2)
	NET_C(R147.1, GND)
	NET_C(Q26.C, C52.1, R148.1)
	NET_C(C52.2, I_VM15)

	//
	// Page 4, bottom-right (bug pushing and final mix)
	//

	ALIAS(BUG_PUSHING_A_P, U31.9)
	NET_C(U31.8, U21.2)
	NET_C(_588USEC_P, U21.1)
	HINT(U21.6, NC)
	NET_C(U21.5, R149.1)
	HINT(U21.4, NC)
	HINT(U21.3, NC)
	NET_C(R149.2, C53.1)

	ALIAS(BUG_PUSHING_B_P, U31.11)
	NET_C(U31.10, U21.12)
	NET_C(_588USEC_P, U21.13)
	NET_C(U21.8, R150.1)
	HINT(U21.9, NC)
	HINT(U21.10, NC)
	HINT(U21.11, NC)
	NET_C(R150.2, C54.2)

	NET_C(C53.2, C54.1, SJ, U54.2, R160.2, R161.1, C61.1)
	NET_C(C61.2, R161.2)
	ALIAS(OUTPUT, R161.2)
	NET_C(R161.2, U54.6)
	NET_C(U54.3, GND)
	NET_C(CS, U55.3, R159.2)
	NET_C(R159.1, GND)
	NET_C(U55.2, U55.6, C60.2)
	NET_C(C60.1, R160.1)

	//
	// Page 5, shaft encoder -- not emulated
	//

	//
	// Page 6, top left
	//

	NET_C(I_V5, R206.1, R207.1, R208.1, R209.1)
	NET_C(R206.2, HIA_P)
	NET_C(R207.2, HIB_P)
	ALIAS(HIC_P, R208.2)
	NET_C(R209.2, HID_P)

	NET_C(I_OUT_7, D25.K, U50.9)
	NET_C(D25.A, GND)
	NET_C(U50.8, U50.1)
	NET_C(U50.2, U49.1)
	ALIAS(DATA_P, U50.2)

#if (HACK_SIMPLIFY_INPUTS)
	//
	// Several of the inputs go through several rounds of inverters
	// diodes and pullups, and eventually something goes wrong.
	// Bypassing these extra devices helps make the inputs reliable.
	//
	NET_C(I_OUT_4, U30.13)
	NET_C(GND, D26.A, D26.K, U50.13, U50.12, U40.9, R177.1, R177.2, C67.1, C67.2)
#else
	NET_C(I_OUT_4, D26.K, U50.13)
	NET_C(D26.A, GND)
	NET_C(U50.12, U40.9)
	NET_C(U40.8, C67.2, R177.1, U30.13)
	NET_C(C67.1, GND)
	NET_C(R177.2, I_V5)
#endif
	NET_C(U30.12, U30.1)
	NET_C(U30.2, U49.8, U39.8)

	NET_C(U49.2, U49.9, U39.2, U39.9, HIA_P)
	NET_C(U49.3, U48.3, U47.3)
	NET_C(U49.4, U48.4, U47.4)
	NET_C(U49.5, U48.7, U47.7)
	NET_C(U49.6, U48.8, U47.8)
	NET_C(U49.10, U48.13, U47.13)
	NET_C(U49.11, U48.14, U47.14)
	NET_C(U49.12, U48.17, U47.17)
	NET_C(U49.13, U48.18, U47.18, U39.1)

	NET_C(U39.3, U38.3)
	NET_C(U39.4, U38.4)
	NET_C(U39.5, U38.7)
	NET_C(U39.6, U38.8)
	NET_C(U39.10, U38.13)
	NET_C(U39.11, U38.14)
	NET_C(U39.12, U38.17)
	NET_C(U39.13, U38.18)

	ALIAS(MEN_P, U48.2)
	NET_C(CANNON_M, U48.5)
	ALIAS(AS1_M, U48.6)
	ALIAS(AS0_M, U48.9)
	ALIAS(FS11_P, U48.12)
	ALIAS(FS10_P, U48.15)
	ALIAS(FS09_P, U48.16)
	ALIAS(FS08_P, U48.19)
	NET_C(U48.1, GND)
	ALIAS(MLATCH_P, U48.11)

	ALIAS(FS07_P, U38.2)
	ALIAS(FS06_P, U38.5)
	ALIAS(FS05_P, U38.6)
	ALIAS(FS04_P, U38.9)
	ALIAS(FS03_P, U38.12)
	ALIAS(FS02_P, U38.15)
	ALIAS(FS01_P, U38.16)
	ALIAS(FS00_P, U38.19)
	NET_C(U38.1, GND)
	NET_C(MLATCH_P, U38.11)

	NET_C(SOFT_EXPL_M, U47.2)
	NET_C(LOUD_EXPL_M, U47.5)
	NET_C(CHIRPING_BIRDS_M, U47.6)
	NET_C(EGG_CRACKING_M, U47.9)
	NET_C(BUG_PUSHING_A_P, U47.12)
	NET_C(BUG_PUSHING_B_P, U47.15)
	NET_C(BUG_DYING_M, U47.16)
	NET_C(BEETLE_ON_SCREEN_M, U47.19)
	NET_C(U47.11, U30.6)
	NET_C(U47.1, GND)

	//
	// Page 6, middle-left
	//

#if (HLE_MUSIC_CLOCK)
	//
	// The 20MHz clock (Y1) is divided by 4 via a pair
	// of JK flip-flops (U23) to 5MHz. That signal is
	// used to clock a 74LS163 counter (U43) that divides
	// the clock by 9 via a preset value. It then goes
	// through another JK flip-flop (U33) for another
	// divide by 2, ending up at 277778Hz. No sense in
	// running all this manually.
	//
	CLOCK(MUSICCLK, 277778)
	NET_C(MUSICCLK.VCC, I_V5)
	NET_C(MUSICCLK.GND, GND)
	ALIAS(_227KC_P, MUSICCLK.Q)
	NET_C(MUSICCLK.Q, U2.13)
	ALIAS(_227KC_M, U2.12)
	NET_C(GND, R178.1, R178.2, R179.1, R179.2, R204.1, R204.2, C68.1, C68.2)
	NET_C(GND, U2.1, U2.3)
	NET_C(GND, U23.1, U23.2, U23.3, U23.4, U23.10, U23.11, U23.12, U23.13)
#else
	//
	// This is just here for documentation; the crystal is
	// not modelled for this circuit.
	//
	NET_C(U2.3, R178.1, C68.1)
	NET_C(R178.2, U2.4, Y1.1)
	NET_C(Y1.2, R179.1, U2.1)
	NET_C(R179.2, U2.2, U2.13, C68.2)
	NET_C(U2.12, U23.1)
	NET_C(HIC_P, U23.3, U23.2, U23.4, U23.11, U23.12, U23.10)
	NET_C(U23.5, U23.13)
	HINT(U23.6, NC)
	ALIAS(_5MHZ_P, U23.9)
	NET_C(_5MHZ_M, U23.8)
#endif

	//
	// Page 6, bottom-left
	//

#if (HACK_SIMPLIFY_INPUTS)
	NET_C(I_OUT_1, U30.9)
	NET_C(GND, D27.A, D27.K, U50.3, U50.4, U40.13, R180.1, R180.2, C69.1, C69.2)
#else
	NET_C(I_OUT_1, U50.3, D27.K)
	NET_C(D27.A, GND)
	NET_C(U50.4, U40.13)
	NET_C(U40.12, R180.1, C69.2, U30.9)
	NET_C(R180.2, I_V5)
	NET_C(C69.1, GND)
#endif
	NET_C(U30.8, U30.5)
	ALIAS(SLATCH_P, U30.6)

#if (HACK_SIMPLIFY_INPUTS)
	NET_C(I_OUT_0, U30.11)
	NET_C(GND, D28.A, D28.K, U50.11, U50.10, U40.11, R181.1, R181.2, C70.1, C70.2)
#else
	NET_C(I_OUT_0, U50.11, D28.K)
	NET_C(D28.A, GND)
	NET_C(U50.10, U40.11)
	NET_C(U40.10, R181.1, C70.2, U30.11)
	NET_C(R181.2, I_V5)
	NET_C(C70.1, GND)
#endif
	NET_C(U30.10, U30.3)
	ALIAS(LATCH_CLK_P, U30.4)

	NET_C(I_OUT_2, U50.5, BOUNCE_EN_M, D29.K)
	NET_C(D29.A, GND)
	NET_C(U50.6, BOUNCE_EN_P)

	NET_C(I_OUT_3, D30.K, BELL_EN_M)
	NET_C(D30.A, GND)

	//
	// Page 6, bottom-middle
	//

#if (HLE_MUSIC_CLOCK)
	NET_C(_227KC_P, U44.3)
	NET_C(_227KC_M, U44.11)
	NET_C(GND, U24.1, U24.2, U24.3, U24.4)
	NET_C(GND, U33.1, U33.4, U33.12)
	NET_C(GND, U43.1, U43.2, U43.3, U43.4, U43.5, U43.6, U43.7, U43.9, U43.10)
	NET_C(HIC_P, U33.13)
#else
	NET_C(HIC_P, U43.5, U43.3, U43.1, U43.10, U43.7)
	NET_C(GND, U43.6, U43.4)
	NET_C(_5MHZ_P, U43.2)
	NET_C(U43.9, U33.12, U24.6)
	HINT(U43.11, NC)
	HINT(U43.12, NC)
	HINT(U43.13, NC)
	HINT(U43.14, NC)
	NET_C(U43.15, U24.2)

	NET_C(U24.4, HIC_P)
	HINT(U24.5, NC)
	NET_C(U24.3, _5MHZ_M)
	NET_C(U24.1, R204.2)
	NET_C(R204.1, I_V5)

	NET_C(HIC_P, U33.1, U33.4, U33.13)
	NET_C(U33.3, U44.3)
	ALIAS(_227KC_P, U33.3)
	NET_C(U33.2, U44.11)
	ALIAS(_227KC_M, U33.2)
#endif

	NET_C(HIA_P, U44.4, U44.1, U44.10, U44.13)
	ALIAS(MACLK_M, U44.2)
	NET_C(U44.5, U44.12)
	ALIAS(MBCLK_P, U44.5)
	ALIAS(MBCLK_M, U44.6)
	ALIAS(MACLK_P, U44.9)
	NET_C(MACLK_M, U44.8, U25.11)

	ALIAS(MCARRY_P, U25.12)
	NET_C(U25.13, R182.2)
	NET_C(R182.1, I_V5)
	ALIAS(MLOAD_M, U25.8)
	NET_C(U25.9, U35.8, U34.9)
	ALIAS(MLOAD_P, U25.9)
	NET_C(HIA_P, U25.10)

	NET_C(HIA_P, U34.8, U34.11, U34.10)
	ALIAS(DMUSIC_P, U34.5)
	HINT(U34.6, NC)

	NET_C(HIA_P, U25.2, U25.4)
	NET_C(LATCH_CLK_P, U25.3)
	NET_C(U25.1, U45.6)
	HINT(U25.5, NC)
	NET_C(U25.6, U35.9, U15.12)

	NET_C(MBCLK_M, U15.13)
	HINT(U15.8, NC)
	NET_C(U15.9, U35.11, U35.12)
	HINT(U15.10, NC)
	HINT(U15.11, NC)

	NET_C(U35.13, U45.10)
	NET_C(U35.10, U45.12)
	NET_C(MBCLK_P, U45.11)
	NET_C(HIA_P, U45.13)
	HINT(U45.8, NC)
	NET_C(U45.9, MLATCH_P, U45.2)
	NET_C(HIA_P, U45.4)
	HINT(U45.5, NC)
	NET_C(U45.1, R203.2)
	NET_C(R203.1, I_V5)
	NET_C(MACLK_P, U45.3)

	//
	// Page 6, right (music generator)
	//

	NET_C(FS11_P, U46.6)
	NET_C(FS10_P, U46.5)
	NET_C(FS09_P, U46.4)
	NET_C(FS08_P, U46.3)
	NET_C(HIA_P, U46.1)
	NET_C(MEN_P, U46.7)
	NET_C(MLOAD_M, U46.9)
	NET_C(MBCLK_P, U46.2)
	NET_C(U46.15, MCARRY_P)
	NET_C(U46.10, U36.15)
	HINT(U46.11, NC)
	HINT(U46.12, NC)
	HINT(U46.13, NC)
	HINT(U46.14, NC)

	NET_C(FS07_P, U36.6)
	NET_C(FS06_P, U36.5)
	NET_C(FS05_P, U36.4)
	NET_C(FS04_P, U36.3)
	NET_C(HIA_P, U36.1)
	NET_C(MEN_P, U36.7)
	NET_C(MLOAD_M, U36.9)   // schems say MLOAD_P, but solarq says MLOAD_M
	NET_C(MACLK_P, U36.2)
	NET_C(U36.10, U37.15)
	HINT(U36.11, NC)
	HINT(U36.12, NC)
	HINT(U36.13, NC)
	HINT(U36.14, NC)

	NET_C(FS03_P, U37.6)
	NET_C(FS02_P, U37.5)
	NET_C(FS01_P, U37.4)
	NET_C(FS00_P, U37.3)
	NET_C(HIA_P, U37.1)
	NET_C(MEN_P, U37.7, U37.10)
	NET_C(MLOAD_M, U37.9)
	NET_C(MBCLK_M, U37.2)
	HINT(U37.11, NC)
	HINT(U37.12, NC)
	HINT(U37.13, NC)
	HINT(U37.14, NC)

	NET_C(DMUSIC_P, R183.1, R184.1)
	NET_C(R183.2, I_V5)
	NET_C(R184.2, C71.1)
	NET_C(C71.2, R185.1, U67.2)
	NET_C(R185.2, R186.1, GND)
	NET_C(R186.2, U67.3)
	NET_C(U67.6, CS)

	NET_C(AS1_M, R187.1, R188.1)
	NET_C(R187.2, I_V5, R189.2, Q31.E)
	NET_C(R188.2, R189.1, Q31.B)
	NET_C(Q31.C, R190.2, R191.1, Q32.E)
	NET_C(R190.1, I_VM15)
	NET_C(R191.2, R192.1, GND)
	NET_C(R192.2, Q32.B)
	NET_C(Q32.C, R193.1)
	NET_C(R193.2, C72.2, R194.1)
	NET_C(C72.1, I_VM15)
	NET_C(R194.2, U67.5, R202.2)

	NET_C(AS0_M, R195.1, R196.1)
	NET_C(R195.2, I_V5, R197.2, Q33.E)
	NET_C(R196.2, R197.1, Q33.B)
	NET_C(Q33.C, R198.2, R199.1, Q34.E)
	NET_C(R198.1, I_VM15)
	NET_C(R199.2, R200.1, GND)
	NET_C(R200.2, Q34.B)
	NET_C(Q34.C, R201.1)
	NET_C(R201.2, C73.2, R202.1)
	NET_C(C73.1, I_VM15)

	//
	// Unconnected inputs
	//

	NET_C(U2.9, U2.11, U9.1, U9.4, U9.12, U9.13, U15.1, U15.2, U20.4, U20.6, U20.8, U20.10, U20.12, U33.8, U33.9, U33.10, U33.11, U35.5, U35.6, U40.1, U40.3, U40.5)

	//
	// Unconnected outputs
	//

	HINT(U2.8, NC)
	HINT(U2.10, NC)
	HINT(U40.2, NC)
	HINT(U40.4, NC)
	HINT(U40.6, NC)

//    HINT(U2.9, NC)
//    HINT(U2.11, NC)
//    HINT(U24.4, NC)

#if (ENABLE_FRONTIERS)
	//
	// Isolate the CS sounds from the rest of the mixer
	//
	OPTIMIZE_FRONTIER(U55.3, RES_M(1), 50)
#endif

NETLIST_END()
