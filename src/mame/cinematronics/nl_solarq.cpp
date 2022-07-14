// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Solar Quest
//
// Derived from the schematics in the Solar Quest manual.
//
// Known problems/issues:
//
//    * This was the most challenging netlist so far to get
//       into realtime performance.
//
//    * The FIRE sound in particular uses a complex network
//       of op-amps (mix of TL081 and CA3080) with lots of
//       feedback. The default "fast" CA3080 is not sufficient
//       here because it uses an instantaneous AFUNC which
//       is bad in feedback loops, so instead we use the
//       much slower 15xBJT model. Frontiers are used to
//       isolate the two relevant CA3080s from the more
//       common CA3080s used for final mixing.
//
//    * A few important frontiers are used to isolate parts
//       of the circuit from one another.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"


//
// Optimizations
//

#define HLE_MUSIC_CLOCK (1)
#define HLE_CAPTURE_VCO (1)
#define HLE_PHOTON_VCO (1)
#define HLE_NOISE_CONVERT (1)
#define ENABLE_FRONTIERS (1)



//
// Main netlist
//

NETLIST_START(solarq)
{

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 4e-5)

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
	RES(R2, RES_K(1))
	RES(R3, RES_K(1))
	RES(R4, RES_K(2))
	RES(R5, RES_K(15))
	RES(R6, 390)
	RES(R7, RES_M(1))
	RES(R8, RES_K(4.7))
	RES(R9, 330)
	RES(R10, 330)
	RES(R11, RES_K(4.7))
	RES(R12, RES_K(20))
	RES(R13, RES_K(24))
	RES(R14, RES_M(1))
	RES(R15, RES_K(6.8))
	RES(R16, RES_K(18))
	RES(R17, RES_K(47))
	RES(R18, 390)
	RES(R19, 390)
	RES(R20, RES_K(4.7))
	RES(R21, RES_K(2.7))
	RES(R22, RES_K(4.7))
	RES(R23, RES_K(39))
	RES(R24, RES_K(12))
	RES(R25, RES_K(1))
	RES(R26, RES_K(75))
	RES(R27, RES_K(4.7))
	RES(R28, RES_K(2.7))
	RES(R29, RES_K(4.7))
	RES(R30, RES_K(39))
	RES(R31, RES_K(12))
	RES(R32, RES_K(1))
	RES(R33, RES_K(200))
	RES(R34, RES_K(8.2))
	RES(R35, RES_K(47))
	RES(R36, RES_K(22))
	RES(R37, RES_K(1))
	RES(R38, RES_K(1))
	RES(R39, RES_K(4.7))
	RES(R40, RES_K(2.7))
	RES(R41, RES_K(4.7))
	RES(R42, RES_K(39))
	RES(R43, RES_K(12))
	RES(R44, RES_K(1))
	RES(R45, RES_K(36))
	RES(R46, RES_K(100))
	RES(R47, RES_K(2))
	RES(R48, RES_M(1))
	RES(R49, RES_K(4.7))
	RES(R50, RES_K(2.7))
	RES(R51, RES_K(4.7))
	RES(R52, RES_K(39))
	RES(R53, RES_K(2.7))
	RES(R54, RES_K(12))
//  RES(R55, xx)           -- deleted
//  RES(R56, xx)           -- deleted
//  RES(R57, xx)           -- deleted
	RES(R58, RES_K(8.2))
	RES(R59, RES_K(51))
	RES(R60, RES_M(1))
	RES(R61, RES_K(20))
	RES(R62, RES_K(20))
	RES(R63, 100)
	RES(R64, RES_K(100))
	RES(R65, RES_K(20))
	RES(R66, RES_K(2))
	RES(R67, 100)
	RES(R68, RES_K(100))
	RES(R69, RES_K(20))
	RES(R70, RES_K(20))
	RES(R71, RES_K(13))
	RES(R72, RES_K(300))
	RES(R73, 100)
	RES(R74, 100)
	RES(R75, RES_K(4.7))
	RES(R76, RES_K(2.7))
	RES(R77, RES_K(4.7))
	RES(R78, RES_K(39))
	RES(R79, RES_K(12))
	RES(R80, RES_K(1))
	RES(R81, RES_K(360))
	RES(R82, RES_K(4.7))
	RES(R83, RES_K(2.7))
	RES(R84, RES_K(4.7))
	RES(R85, RES_K(39))
	RES(R86, RES_K(12))
	RES(R87, RES_K(2.7))
	RES(R88, RES_K(270))
	RES(R89, RES_K(51))
	RES(R90, RES_M(1))
	RES(R91, RES_K(10))
	RES(R92, RES_K(10))
	RES(R93, RES_K(1))
	RES(R94, RES_K(2))
	RES(R95, RES_K(10))
	RES(R96, RES_K(12))
	RES(R97, RES_K(1))
	RES(R98, RES_K(10))
	RES(R99, RES_K(11))
	RES(R100, RES_K(100))
	RES(R101, RES_K(100))
	RES(R102, RES_K(240))
	RES(R103, RES_K(100))
	RES(R104, RES_K(100))
	RES(R105, RES_K(100))
	RES(R106, 200)
	RES(R107, 200)
	RES(R108, RES_K(120))
	RES(R109, RES_M(10))
	RES(R110, RES_K(100))
	RES(R111, 200)
	RES(R112, 200)
	RES(R113, RES_K(4.7))
	RES(R114, RES_K(2.7))
	RES(R115, RES_K(4.7))
	RES(R116, RES_K(39))
	RES(R117, RES_K(12))
	RES(R118, RES_K(1))
	RES(R119, RES_K(120))
	RES(R120, RES_M(10))
	RES(R121, RES_K(4.7))
	RES(R122, RES_M(1.5))
	RES(R123, 390)
	RES(R124, 390)
	RES(R125, RES_K(4.7))
	RES(R126, RES_K(2.7))
	RES(R127, RES_K(4.7))
	RES(R128, RES_K(39))
	RES(R129, RES_K(12))
	RES(R130, RES_K(1))
//  RES(R131, xx)          -- deleted
	RES(R132, RES_K(75))
	RES(R133, RES_K(4.7))
	RES(R134, RES_K(2.7))
	RES(R135, RES_K(4.7))
	RES(R136, RES_K(39))
	RES(R137, RES_K(12))
	RES(R138, RES_K(1))
	RES(R139, RES_K(51))
	RES(R140, RES_K(100))
	RES(R141, RES_K(4.7))
	RES(R142, RES_K(2.7))
	RES(R143, RES_K(4.7))
	RES(R144, RES_K(39))
	RES(R145, RES_K(12))
	RES(R146, RES_K(1))
	RES(R147, RES_K(51))
	RES(R148, RES_K(200))
	RES(R149, RES_K(30))
	RES(R150, RES_K(8.2))
	RES(R151, RES_K(51))
//  POT(R152, RES_K(10))  -- part of final amp (not emulated)
//  RES(R153, RES_K(15))  -- part of final amp (not emulated)
//  RES(R154, RES_K(390)) -- part of final amp (not emulated)
//  RES(R155, RES_K(150)) -- part of final amp (not emulated)
//  RES(R156, 150)        -- part of final amp (not emulated)
//  RES(R157, RES_K(22))  -- part of final amp (not emulated)
//  RES(R158, 0.51)       -- part of final amp (not emulated)
//  RES(R159, 0.51)       -- part of final amp (not emulated)
//  RES(R160, RES_K(390)) -- part of final amp (not emulated)
	RES(R161, RES_K(1))
	RES(R162, RES_K(1))
	RES(R163, RES_K(1))
	RES(R164, RES_K(1))

	CAP(C1, CAP_U(0.001))
	CAP(C2, CAP_U(0.01))
	CAP(C3, CAP_U(0.1))
	CAP(C4, CAP_P(680))
	CAP(C5, CAP_U(0.1))
	CAP(C6, CAP_U(0.1))
	CAP(C7, CAP_U(0.01))
	CAP(C8, CAP_U(3.3))
	CAP(C9, CAP_U(1))
	CAP(C10, CAP_U(0.22))
	CAP(C11, CAP_U(0.33))
	CAP(C12, CAP_U(4.7))
	CAP(C13, CAP_U(0.1))
	CAP(C14, CAP_U(0.01))
	CAP(C15, CAP_U(1))
	CAP(C16, CAP_U(0.1))
	CAP(C17, CAP_U(1))
	CAP(C18, CAP_U(0.47))
	CAP(C19, CAP_U(0.001))
	CAP(C20, CAP_U(0.01))
	CAP(C21, CAP_U(0.1))
	CAP(C22, CAP_U(0.1))
	CAP(C23, CAP_P(330))
	CAP(C24, CAP_U(0.68))
	CAP(C25, CAP_P(330))
	CAP(C26, CAP_U(2.2))
	CAP(C27, CAP_U(0.22))
	CAP(C28, CAP_U(2.2))
	CAP(C29, CAP_U(2.2))
	CAP(C30, CAP_U(0.1))
	CAP(C31, CAP_U(0.33))
//  CAP(C32, CAP_U(0.68))   -- part of final amp (not emulated)
//  CAP(C33, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C34, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C35, CAP_U(0.005))  -- part of final amp (not emulated)
//  CAP(C36, CAP_P(470))    -- part of final amp (not emulated)
//  CAP(C37, CAP_U(0.1))    -- part of voltage converter (not emulated)
//  CAP(C38, CAP_U(25))     -- part of voltage converter (not emulated)
//  CAP(C39, CAP_U(25))     -- part of voltage converter (not emulated)
//  CAP(C40, CAP_U(0.1))    -- part of voltage converter (not emulated)
//  CAP(C41, CAP_U(25))     -- part of voltage converter (not emulated)
//  CAP(C42, CAP_U(25))     -- part of voltage converter (not emulated)
//  CAP(C43, CAP_U(25))     -- part of voltage converter (not emulated)

//  D_1N4003(D1)            -- part of voltage converter (not emulated)
//  D_1N4003(D2)            -- part of voltage converter (not emulated)
//  D_1N4003(D3)            -- part of voltage converter (not emulated)
//  D_1N4003(D4)            -- part of voltage converter (not emulated)
	D_1N5240(D5)
	D_1N5240(D6)
	D_1N914(D7)
	D_1N914(D8)
//  D_1N4003(D9)            -- part of final amp (not emulated)
//  D_1N4003(D10)           -- part of final amp (not emulated)

	Q_2N3906(Q1)            // PNP
	Q_2N3906(Q2)            // PNP
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3906(Q5)            // PNP
	Q_2N3906(Q6)            // PNP
	Q_2N3906(Q7)            // PNP
	Q_2N3904(Q8)            // NPN
	Q_2N3906(Q9)            // PNP
	Q_2N3906(Q10)           // PNP
	Q_2N3906(Q11)           // PNP
	Q_2N3904(Q12)           // NPN
#if !(HLE_CAPTURE_VCO)
	Q_2N3904(Q13)           // NPN
#endif
	Q_2N3906(Q14)           // PNP
	Q_2N3906(Q15)           // PNP
	Q_2N3906(Q16)           // PNP
	Q_2N3906(Q17)           // PNP
	Q_2N3906(Q18)           // PNP
	Q_2N3906(Q19)           // PNP
	Q_2N3906(Q20)           // PNP
	Q_2N3906(Q21)           // PNP
//  Q_2N6292(Q22)           // NPN -- part of final amp (not emulated)
//  Q_2N6107(Q23)           // PNP -- part of final amp (not emulated)

#if (!HLE_MUSIC_CLOCK)
	CLOCK(Y1, 20000000)
	NET_C(Y1.GND, GND)
	NET_C(Y1.VCC, I_V5)
#endif

	TTL_7414_DIP(U1)        // Hex Inverter
	NET_C(U1.7, GND)
	NET_C(U1.14, I_V5)

	TTL_7414_DIP(U2)        // Hex Inverter
	NET_C(U2.7, GND)
	NET_C(U2.14, I_V5)

	TTL_74LS164_DIP(U3)     // 8-bit Shift Reg.
	NET_C(U3.7, GND)
	NET_C(U3.14, I_V5)

	TTL_74LS164_DIP(U4)     // 8-bit Shift Reg.
	NET_C(U4.7, GND)
	NET_C(U4.14, I_V5)

	TTL_74LS377_DIP(U5)     // Octal D Flip Flop
	NET_C(U5.10, GND)
	NET_C(U5.20, I_V5)

	TTL_74LS377_DIP(U6)     // Octal D Flip Flop
	NET_C(U6.10, GND)
	NET_C(U6.20, I_V5)

	TTL_74LS377_DIP(U7)     // Octal D Flip Flop
	NET_C(U7.10, GND)
	NET_C(U7.20, I_V5)

	TTL_74LS163_DIP(U8)     // Binary Counter
	NET_C(U8.8, GND)
	NET_C(U8.16, I_V5)

	TTL_74LS163_DIP(U9)     // Binary Counter
	NET_C(U9.8, GND)
	NET_C(U9.16, I_V5)

	TTL_74LS163_DIP(U10)    // Binary Counter
	NET_C(U10.8, GND)
	NET_C(U10.16, I_V5)

	TTL_74LS163_DIP(U11)    // Binary Counter
	NET_C(U11.8, GND)
	NET_C(U11.16, I_V5)

	TTL_74LS163_DIP(U12)    // Binary Counter
	NET_C(U12.8, GND)
	NET_C(U12.16, I_V5)

	LM555_DIP(U13)

	TTL_74LS74_DIP(U14)     // Dual D Flip Flop
	NET_C(U14.7, GND)
	NET_C(U14.14, I_V5)

	TTL_74LS74_DIP(U15)     // Dual D Flip Flop
	NET_C(U15.7, GND)
	NET_C(U15.14, I_V5)

	TTL_74LS107_DIP(U16)    // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U16.7, GND)
	NET_C(U16.14, I_V5)

	TTL_74LS393_DIP(U17)    // Dual 4-Stage Binary Counter
	NET_C(U17.7, GND)
	NET_C(U17.14, I_V5)

	TTL_74LS86_DIP(U18)     // Quad 2-Input XOR Gates
	NET_C(U18.7, GND)
	NET_C(U18.14, I_V5)

	TTL_74LS74_DIP(U19)     // Dual D Flip Flop
	NET_C(U19.7, GND)
	NET_C(U19.14, I_V5)

	TTL_74LS74_DIP(U20)     // Dual D Flip Flop
	NET_C(U20.7, GND)
	NET_C(U20.14, I_V5)

	TTL_74LS393_DIP(U21)    // Dual 4-Stage Binary Counter
	NET_C(U21.7, GND)
	NET_C(U21.14, I_V5)

	TTL_74LS02_DIP(U22)     // Quad 2-input Nor Gate
	NET_C(U22.7, GND)
	NET_C(U22.14, I_V5)

	TTL_74LS74_DIP(U23)     // Dual D Flip Flop
	NET_C(U23.7, GND)
	NET_C(U23.14, I_V5)

	TTL_74S04_DIP(U24)      // Hex Inverting Gates
	NET_C(U24.7, GND)
	NET_C(U24.14, I_V5)

	TTL_74S113A_DIP(U25)    // Dual JK Negative Edge-Trigged Flip Flop
	NET_C(U25.7, GND)
	NET_C(U25.14, I_V5)

	TTL_74LS163_DIP(U26)    // Binary Counter
	NET_C(U26.8, GND)
	NET_C(U26.16, I_V5)

	TTL_74LS107_DIP(U27)    // DUAL J-K FLIP-FLOPS WITH CLEAR
	NET_C(U27.7, GND)
	NET_C(U27.14, I_V5)

	TTL_74LS164_DIP(U28)    // 8-bit Shift Reg.
	NET_C(U28.7, GND)
	NET_C(U28.14, I_V5)

	TTL_74LS164_DIP(U29)    // 8-bit Shift Reg.
	NET_C(U29.7, GND)
	NET_C(U29.14, I_V5)

	TTL_74LS164_DIP(U30)    // 8-bit Shift Reg.
	NET_C(U30.7, GND)
	NET_C(U30.14, I_V5)

	TTL_74LS393_DIP(U31)    // Dual 4-Stage Binary Counter
	NET_C(U31.7, GND)
	NET_C(U31.14, I_V5)

	TL081_DIP(U32)          // Op. Amp.
	NET_C(U32.7, I_V15)
	NET_C(U32.4, I_VM15)

	CA3080_DIP(U33)         // Op. Amp.
	NET_C(U33.4, I_VM15)
	NET_C(U33.7, I_V15)

	CA3080_DIP(U34)         // Op. Amp.
	NET_C(U34.4, I_VM15)
	NET_C(U34.7, I_V15)

	LM555_DIP(U35)

	TL081_DIP(U36)          // Op. Amp.
	NET_C(U36.7, I_V15)
	NET_C(U36.4, I_VM15)

	TL081_DIP(U37)          // Op. Amp.
	NET_C(U37.7, I_V15)
	NET_C(U37.4, I_VM15)

	TL081_DIP(U38)          // Op. Amp.
	NET_C(U38.7, I_V15)
	NET_C(U38.4, I_VM15)

	TL081_DIP(U39)          // Op. Amp.
	NET_C(U39.7, I_V15)
	NET_C(U39.4, I_VM15)

	TL081_DIP(U40)          // Op. Amp.
	NET_C(U40.7, I_V15)
	NET_C(U40.4, I_VM15)

	CA3080_DIP(U41)         // Op. Amp.
	NET_C(U41.4, I_VM15)
	NET_C(U41.7, I_V15)

#if (!HLE_CAPTURE_VCO)
	LM566_DIP(U42)
#endif

	TL081_DIP(U43)          // Op. Amp.
	NET_C(U43.7, I_V15)
	NET_C(U43.4, I_VM15)

	CA3080_SLOW_DIP(U44)        // Op. Amp.
	NET_C(U44.4, I_VM15)
	NET_C(U44.7, I_V15)

	TL081_DIP(U45)          // Op. Amp.
	NET_C(U45.7, I_V15)
	NET_C(U45.4, I_VM15)

	CA3080_SLOW_DIP(U46)        // Op. Amp.
	NET_C(U46.4, I_VM15)
	NET_C(U46.7, I_V15)

	TL081_DIP(U47)          // Op. Amp.
	NET_C(U47.7, I_V15)
	NET_C(U47.4, I_VM15)

	CA3080_DIP(U48)         // Op. Amp.
	NET_C(U48.4, I_VM15)
	NET_C(U48.7, I_V15)

	TL081_DIP(U49)          // Op. Amp.
	NET_C(U49.7, I_V15)
	NET_C(U49.4, I_VM15)

	TL081_DIP(U50)          // Op. Amp.
	NET_C(U50.7, I_V15)
	NET_C(U50.4, I_VM15)

//  TL081_DIP(U51)          // Op. Amp. -- part of final amp (not emulated)
//  NET_C(U51.7, I_V15)
//  NET_C(U51.4, I_VM15)

//  TTL_7815_DIP(U52)       // +15V Regulator -- part of voltage converter (not emulated)
//  TTL_7915_DIP(U53)       // -15V Regulator -- part of voltage converter (not emulated)

	//
	// Page 1, top left
	//

	NET_C(I_OUT_7, U1.1)
	NET_C(U1.2, U1.3)
	NET_C(U1.4, U3.1)

	NET_C(I_OUT_4, U1.5)
	NET_C(U1.6, U1.13)
	NET_C(U1.12, U3.8, U4.8)

	NET_C(I_V5, R1.1, R2.1, R3.1)
	ALIAS(HIA_P, R1.2)
	ALIAS(HIB_P, R2.2)
	ALIAS(HIC_P, R3.2)

	NET_C(HIA_P, U3.9, U3.2)
	NET_C(U3.3, U5.3, U7.3)
	NET_C(U3.4, U5.4, U7.4)
	NET_C(U3.5, U5.7, U7.7)
	NET_C(U3.6, U5.8, U7.8)
	NET_C(U3.10, U5.13, U7.13)
	NET_C(U3.11, U5.14, U7.14)
	NET_C(U3.12, U5.17, U7.17)
	NET_C(U3.13, U5.18, U7.18, U4.1)

	NET_C(HIA_P, U4.2, U4.9)
	NET_C(U4.3, U6.3)
	NET_C(U4.4, U6.4)
	NET_C(U4.5, U6.7)
	NET_C(U4.6, U6.8)
	NET_C(U4.10, U6.13)
	NET_C(U4.11, U6.14)
	NET_C(U4.12, U6.17)
	NET_C(U4.13, U6.18)

	ALIAS(MEN_P, U5.2)
	ALIAS(AS2_M, U5.5)
	ALIAS(AS1_M, U5.6)
	ALIAS(AS0_M, U5.9)
	ALIAS(FS11_P, U5.12)
	ALIAS(FS10_P, U5.15)
	ALIAS(FS09_P, U5.16)
	ALIAS(FS08_P, U5.19)
	ALIAS(MLATCH_P, U5.11)
	NET_C(U5.1, GND)

	ALIAS(FS07_P, U6.2)
	ALIAS(FS06_P, U6.5)
	ALIAS(FS05_P, U6.6)
	ALIAS(FS04_P, U6.9)
	ALIAS(FS03_P, U6.12)
	ALIAS(FS02_P, U6.15)
	ALIAS(FS01_P, U6.16)
	ALIAS(FS00_P, U6.19)
	NET_C(U6.11, MLATCH_P)
	NET_C(U6.1, GND)

	//
	// Page 1, bottom
	//

	NET_C(I_OUT_0, U2.3)
	NET_C(U2.4, U2.5)
	ALIAS(LATCH_CLK_P, U2.6)

	NET_C(I_OUT_1, U1.11)
	NET_C(U1.10, U1.9)
	NET_C(U1.8, U7.11)

	ALIAS(LOUD_EXP_M, U7.2)
	ALIAS(SOFT_EXP_M, U7.5)
	ALIAS(THRUST_M, U7.6)
	ALIAS(FIRE_M, U7.9)
	ALIAS(CAPTURE_M, U7.12)
	ALIAS(NUKE_P, U7.15)
	ALIAS(PHOTON_M, U7.16)
	HINT(U7.19, NC)
	NET_C(U7.1, GND)

	NET_C(HIA_P, U19.4, U19.1)
	ALIAS(MACLK_M, U19.2)
	ALIAS(_227KC_P, U19.3)
	ALIAS(MBCLK_P, U19.5)
	ALIAS(MBCLK_M, U19.6)

	NET_C(HIA_P, U19.10, U19.13)
	NET_C(MBCLK_P, U19.12)
	ALIAS(_227KC_M, U19.11)
	ALIAS(MACLK_P, U19.9)
	NET_C(U19.8, MACLK_M, U20.11)

	NET_C(HIA_P, U16.8, U16.11, U16.10)
	NET_C(U16.9, U20.9, U22.8)
	ALIAS(DMUSIC_P, U16.5)
	HINT(U16.6, NC)

	NET_C(HIA_P, U20.10)
	ALIAS(MCARRY_P, U20.12)
	NET_C(R162.1, I_V5)
	NET_C(R162.2, U20.13)
	ALIAS(MLOAD_M, U20.8)

	NET_C(MBCLK_M, U21.13)
	HINT(U21.8, NC)
	NET_C(U21.9, U22.11, U22.12)
	HINT(U21.10, NC)
	HINT(U21.11, NC)
	NET_C(U21.12, U20.6, U22.9)

	NET_C(HIA_P, U20.2, U20.4)
	NET_C(LATCH_CLK_P, U20.3)
	ALIAS(KILL_FLAG_M, U20.1)
	HINT(U20.5, NC)

	NET_C(U22.13, U23.10)
	NET_C(U22.10, U23.12)
	NET_C(MBCLK_P, U23.11)
	NET_C(HIA_P, U23.13)
	HINT(U23.8, NC)
	NET_C(U23.9, U23.2, MLATCH_P)

	NET_C(HIA_P, U23.4)
	NET_C(MACLK_P, U23.3)
	NET_C(I_V5, R163.1)
	NET_C(R163.2, U23.1)
	HINT(U23.5, NC)
	NET_C(U23.6, KILL_FLAG_M)

	//
	// Page 1, middle
	//

	NET_C(FS11_P, U8.6)
	NET_C(FS10_P, U8.5)
	NET_C(FS09_P, U8.4)
	NET_C(FS08_P, U8.3)
	NET_C(HIB_P, U8.1)
	NET_C(MEN_P, U8.7)
	NET_C(MLOAD_M, U8.9)
	NET_C(MBCLK_P, U8.2)
	NET_C(U8.10, U9.15)
	NET_C(U8.15, MCARRY_P)
	HINT(U8.11, NC)
	HINT(U8.12, NC)
	HINT(U8.13, NC)
	HINT(U8.14, NC)

	NET_C(FS07_P, U9.6)
	NET_C(FS06_P, U9.5)
	NET_C(FS05_P, U9.4)
	NET_C(FS04_P, U9.3)
	NET_C(HIB_P, U9.1)
	NET_C(MEN_P, U9.7)
	NET_C(MLOAD_M, U9.9)
	NET_C(MACLK_P, U9.2)
	NET_C(U9.10, U10.15)
	HINT(U9.11, NC)
	HINT(U9.12, NC)
	HINT(U9.13, NC)
	HINT(U9.14, NC)

	NET_C(FS03_P, U10.6)
	NET_C(FS02_P, U10.5)
	NET_C(FS01_P, U10.4)
	NET_C(FS00_P, U10.3)
	NET_C(HIB_P, U10.1)
	NET_C(MLOAD_M, U10.9)
	NET_C(MBCLK_M, U10.2)
	NET_C(MEN_P, U10.7, U10.10)
	HINT(U10.11, NC)
	HINT(U10.12, NC)
	HINT(U10.13, NC)
	HINT(U10.14, NC)

	//
	// Page 1, top-right
	//

	NET_C(GND, U11.6, U11.4, U11.3)
	NET_C(HIB_P, U11.5, U11.1, U11.7)
	ALIAS(NLOAD_M, U11.9)
	ALIAS(NBCLK_P, U11.2)
	NET_C(U11.10, U12.15)
	NET_C(U11.15, U15.12)
	HINT(U11.11, NC)
	HINT(U11.12, NC)
	HINT(U11.13, NC)
	HINT(U11.14, NC)

	NET_C(GND, U12.6, U12.5, U12.4)
	NET_C(HIB_P, U12.3, U12.1, U12.7, U12.10)
	NET_C(NLOAD_M, U12.9)
	ALIAS(NACLK_M, U12.2)
	HINT(U12.11, NC)
	HINT(U12.12, NC)
	HINT(U12.13, NC)
	HINT(U12.14, NC)

	NET_C(HIB_P, U15.10)
	ALIAS(NBCLK_M, U15.11)
	NET_C(I_V5, R164.1)
	NET_C(R164.2, U15.13)
	NET_C(U15.9, U16.12)
	NET_C(U15.8, NLOAD_M)

	NET_C(U16.1, U16.4, U16.13, HIB_P)
	NET_C(U16.3, U18.2)
	HINT(U16.2, NC)

	NET_C(GND, U17.2)
	NET_C(NBCLK_P, U17.1)
	NET_C(U17.6, U17.13)
	HINT(U17.5, NC)
	HINT(U17.4, NC)
	HINT(U17.3, NC)

	NET_C(GND, U17.12)
	HINT(U17.8, NC)
	NET_C(U17.9, U18.1)
	HINT(U17.10, NC)
	HINT(U17.11, NC)

	NET_C(U18.3, R7.1, R8.2)
	NET_C(R7.2, C3.1)
	ALIAS(SJ, C3.2)
	NET_C(R8.1, I_V5)

	//
	// Page 1, middle-right
	//

	NET_C(I_V5, U13.8, R4.2)
	NET_C(R4.1, U13.7, R5.2)
	NET_C(R5.1, U13.2, U13.6, C1.2)
	NET_C(C1.1, GND)
	NET_C(NUKE_P, U13.4)
	NET_C(U13.1, GND)
	NET_C(U13.5, C2.2)
	NET_C(C2.1, GND)
	NET_C(U13.3, R6.2, U2.1)
	NET_C(R6.1, GND)
	NET_C(U2.2, U2.13, U14.3)

	NET_C(HIB_P, U14.1, U14.13)
	NET_C(U14.6, NACLK_M)
	NET_C(U14.5, U14.12)
	NET_C(U14.2, U14.8, NBCLK_M)
	NET_C(HIB_P, U14.4, U14.10)
	NET_C(U14.9, NBCLK_P)
	NET_C(U2.12, U14.11)

#if (HLE_MUSIC_CLOCK)
	//
	// The 20MHz clock (Y1) is divided by 4 via a pair
	// of JK flip-flops (U25) to 5MHz. That signal is only
	// used to clock a 74LS163 counter (U26) that divides
	// the clock by 9 via a preset value. It then goes
	// through another JK flip-flop (U27) for another
	// divide by 2, ending up at 277778Hz. No sense in
	// running all this manually.
	//
	CLOCK(MUSICCLK, 277778)
	NET_C(MUSICCLK.VCC, I_V5)
	NET_C(MUSICCLK.GND, GND)
	NET_C(_227KC_P, MUSICCLK.Q)
	NET_C(MUSICCLK.Q, U24.13)
	NET_C(_227KC_M, U24.12)
	NET_C(GND, R9.1, R9.2, R10.1, R10.2, R161.1, R161.2, C4.1, C4.2)
	NET_C(GND, U15.1, U15.2, U15.3, U15.4)
	NET_C(GND, U24.1, U24.5)
	NET_C(GND, U25.1, U25.2, U25.3, U25.4, U25.10, U25.11, U25.12, U25.13)
	NET_C(GND, U26.1, U26.2, U26.3, U26.4, U26.5, U26.6, U26.7, U26.9, U26.10)
	NET_C(GND, U27.1, U27.4, U27.12)
	NET_C(HIC_P, U27.13)

#else

	//
	// Page 2, top-left (clock)
	//

	//
	// This is just here for documentation; the crystal is
	// not modelled for this circuit.
	//
	NET_C(U24.5, R9.1, C4.1)
	NET_C(U24.6, R9.2, Y1.1)
	NET_C(Y1.2, R10.1, U24.1)
	NET_C(R10.2, U24.2, C4.2, U24.13)
	NET_C(U24.12, U25.1)
	NET_C(HIC_P, U25.3, U25.2, U25.4, U25.11, U25.12, U25.10)
	NET_C(U25.5, U25.13)
	ALIAS(_5MC_P, U25.9)
	ALIAS(_5MC_M, U25.8)

	//
	// Page 2, middle-left
	//

	NET_C(HIC_P, U26.5, U26.3, U26.1, U26.10, U26.7)
	NET_C(GND, U26.6, U26.4)
	NET_C(_5MC_P, U26.2)
	HINT(U26.11, NC)
	HINT(U26.12, NC)
	HINT(U26.13, NC)
	HINT(U26.14, NC)
	NET_C(U26.9, U27.12, U15.6)
	NET_C(U26.15, U15.2)

	NET_C(_5MC_M, U15.3)
	NET_C(HIC_P, U15.4)
	NET_C(I_V5, R161.1)
	NET_C(R161.2, U15.1)

	NET_C(HIC_P, U27.1, U27.4, U27.13)
	NET_C(_227KC_P, U27.3)
	NET_C(_227KC_M, U27.2)
#endif

	//
	// Page 2, top-middle
	//

	NET_C(MACLK_P, U29.8)
	ALIAS(IN_M, U29.2)
	NET_C(U29.2, U29.1)
	ALIAS(RN_M, U29.9)
	NET_C(U29.9, U28.9, U30.9)
	NET_C(U29.13, U28.1, U28.2)
	HINT(U29.12, NC)
	HINT(U29.11, NC)
	HINT(U29.10, NC)
	HINT(U29.6, NC)
	HINT(U29.5, NC)
	HINT(U29.4, NC)
	HINT(U29.3, NC)

	NET_C(MACLK_P, U28.8)
	NET_C(U28.13, U30.1, U30.2)
	HINT(U28.12, NC)
	HINT(U28.11, NC)
	HINT(U28.10, NC)
	HINT(U28.6, NC)
	HINT(U28.5, NC)
	HINT(U28.4, NC)
	HINT(U28.3, NC)

	NET_C(MACLK_P, U30.8)
	NET_C(U30.13, U18.10)
	NET_C(U30.6, U18.5)
	NET_C(U30.5, U18.4)
	NET_C(U30.3, U18.9)
	HINT(U30.12, NC)
	HINT(U30.11, NC)
	HINT(U30.10, NC)
	HINT(U30.4, NC)

#if (HLE_NOISE_CONVERT)
	//
	// The TTL-to-analog conversion takes a noticeable
	// amount of time, so just do it directly. The 4.2
	// P-P value is observed from the original netlist.
	//
	AFUNC(NOISECONV, 1, "if(A0>2.5,-4.2,4.2)")
	NET_C(U30.13, NOISECONV.A0)
	ALIAS(NOISE, NOISECONV.Q)
	NET_C(GND, C5.1, C5.2, R11.1, R11.2, R12.1, R12.2, R13.1, R13.2, R14.1, R14.2, U32.2, U32.3)
#else
	NET_C(U30.13, R11.1, C5.1)
	NET_C(R11.2, I_V5)
	NET_C(C5.2, R14.2, U32.3)
	NET_C(R14.1, GND)
	NET_C(U32.2, R12.2, R13.1)
	NET_C(R12.1, GND)
	NET_C(R13.2, U32.6)
	ALIAS(NOISE, R13.2)
#endif

	NET_C(U18.6, U18.12)
	NET_C(U18.8, U18.13)
	NET_C(U18.11, U31.12, U31.2, U24.9)
	NET_C(U24.8, IN_M)

	NET_C(MACLK_M, U31.1)
	NET_C(U31.6, U31.13)
	HINT(U31.5, NC)
	HINT(U31.4, NC)
	HINT(U31.3, NC)
	NET_C(U31.10, U24.11)
	HINT(U31.8, NC)
	HINT(U31.9, NC)
	HINT(U31.11, NC)
	NET_C(U24.10, RN_M)

	//
	// Page 2, bottom-middle and top-right (fire)
	//

	NET_C(FIRE_M, R113.1, R114.1)
	NET_C(R113.2, I_V5, R115.2, Q14.E)
	NET_C(R114.2, R115.1, Q14.B)
	NET_C(Q14.C, R116.2, R117.2, Q15.E)
	NET_C(R116.1, I_VM15)
	NET_C(R117.1, R118.1, GND)
	NET_C(R118.2, Q15.B)
	NET_C(Q15.C, C26.1, R108.1, R119.1)
	NET_C(C26.2, I_VM15)
	NET_C(R119.2, U46.5)
	NET_C(R108.2, U44.5)

	NET_C(NOISE, R98.2)
	NET_C(R98.1, R99.2, R100.1)
	NET_C(R99.1, GND)
	NET_C(R100.2, U43.2, R104.1, R102.1, R101.1)
	NET_C(U43.3, GND)
	NET_C(R104.2, U43.6, R105.1)
	NET_C(R105.2, R106.2, U44.3)
	NET_C(R106.1, GND)
	NET_C(U44.2, R107.2)
	NET_C(R107.1, GND)
	NET_C(U44.6, C23.2, R109.2, U45.3)
	NET_C(C23.1, GND)
	NET_C(R109.1, GND)
	NET_C(U45.6, U45.2, R110.1, R103.1, R102.2)
	NET_C(R103.2, C24.1)
	NET_C(C24.2, SJ)
	NET_C(R110.2, R111.2, U46.3)
	NET_C(R111.1, R112.1, GND)
	NET_C(R112.2, U46.2)
	NET_C(U46.6, C25.2, R120.2, U47.3)
	NET_C(C25.1, R120.1, GND)
	NET_C(U47.2, U47.6, R101.2)

	//
	// Page 2, bottom-right (AS0-2)
	//

	NET_C(I_V5, R121.1)
	NET_C(R121.2, DMUSIC_P, R122.1)
	NET_C(R122.2, C27.1)
	NET_C(C27.2, R123.2, U48.2)
	NET_C(R123.1, GND, R124.1)
	NET_C(R124.2, U48.3)
	NET_C(R148.2, R140.2, R132.2, U48.5)
	ALIAS(CS, U48.6)

	NET_C(AS2_M, R125.1, R126.1)
	NET_C(R125.2, I_V5, R127.2, Q16.E)
	NET_C(R126.2, R127.1, Q16.B)
	NET_C(Q16.C, R128.2, R129.2, Q17.E)
	NET_C(R128.1, I_VM15)
	NET_C(R129.1, R130.1, GND)
	NET_C(R130.2, Q17.B)
	NET_C(Q17.C, C28.1, R132.1)
	NET_C(C28.2, I_VM15)

	NET_C(AS1_M, R133.1, R134.1)
	NET_C(R133.2, I_V5, R135.2, Q18.E)
	NET_C(R134.2, R135.1, Q18.B)
	NET_C(Q18.C, R136.2, R137.2, Q19.E)
	NET_C(R136.1, I_VM15)
	NET_C(R137.1, R138.1, GND)
	NET_C(R138.2, Q19.B)
	NET_C(Q19.C, R139.1)
	NET_C(R139.2, C29.1, R140.1)
	NET_C(C29.2, I_VM15)

	NET_C(AS0_M, R141.1, R142.1)
	NET_C(R141.2, I_V5, R143.2, Q20.E)
	NET_C(R142.2, R143.1, Q20.B)
	NET_C(Q20.C, R144.2, R145.2, Q21.E)
	NET_C(R144.1, I_VM15)
	NET_C(R145.1, R146.1, GND)
	NET_C(R146.2, Q21.B)
	NET_C(Q21.C, R147.1)
	NET_C(R147.2, C30.1, R148.1)
	NET_C(C30.2, I_VM15)

	//
	// Page 3, top-left (explosions)
	//

	NET_C(NOISE, R15.1)
	NET_C(R15.2, C6.2, R16.1)
	NET_C(C6.1, GND)
	NET_C(R16.2, C7.2, R17.1)
	NET_C(C7.1, GND)
	NET_C(R17.2, R18.2, U33.2)
	NET_C(R18.1, GND)
	NET_C(U33.3, R19.2)
	NET_C(R19.1, GND)
	NET_C(CS, U33.6)

	NET_C(LOUD_EXP_M, R20.1, R21.1)
	NET_C(R20.2, I_V5, R22.2, Q1.E)
	NET_C(R21.2, R22.1, Q1.B)
	NET_C(Q1.C, R23.2, R24.2, Q2.E)
	NET_C(R23.1, I_VM15)
	NET_C(R24.1, GND)
	NET_C(Q2.B, R25.2)
	NET_C(R25.1, GND)
	NET_C(Q2.C, C8.1, R26.1)
	NET_C(C8.2, I_VM15)
	NET_C(R26.2, U33.5)

	NET_C(SOFT_EXP_M, R27.1, R28.1)
	NET_C(R27.2, I_V5, R29.2, Q3.E)
	NET_C(R28.2, R29.1, Q3.B)
	NET_C(Q3.C, R30.2, R31.2, Q4.E)
	NET_C(R30.1, I_VM15)
	NET_C(R31.1, GND)
	NET_C(Q4.B, R32.2)
	NET_C(R32.1, GND)
	NET_C(Q4.C, C9.1, R33.1)
	NET_C(C9.2, I_VM15)
	NET_C(R33.2, U33.5)

	//
	// Page 3, bottom-left (thrust)
	//

	NET_C(NOISE, R34.1)
	NET_C(R34.2, C10.2, R35.1)
	NET_C(C10.1, GND)
	NET_C(R35.2, C11.2, R36.1)
	NET_C(C11.1, GND)
	NET_C(R36.2, R37.2, U34.2)
	NET_C(R37.1, GND)
	NET_C(U34.3, R38.2)
	NET_C(R38.1, GND)
	NET_C(U34.6, CS)

	NET_C(THRUST_M, R39.1, R40.1)
	NET_C(R39.2, I_V5, R41.2, Q5.E)
	NET_C(R40.2, R41.1, Q5.B)
	NET_C(Q5.C, R42.2, R43.2, Q6.E)
	NET_C(R42.1, I_VM15)
	NET_C(R43.1, GND)
	NET_C(Q6.B, R44.2)
	NET_C(R44.1, GND)
	NET_C(Q6.C, R45.1)
	NET_C(R45.2, C12.1, R46.1)
	NET_C(C12.2, I_VM15)
	NET_C(R46.2, U34.5)

	//
	// Page 3, top-middle/right (capture)
	//

	NET_C(CAPTURE_M, R82.1, R83.1, U21.2)
	NET_C(R82.2, I_V5, R84.2, Q11.E)
	NET_C(R83.2, R84.1, Q11.B)
	NET_C(Q11.C, R85.2, R86.2, R87.1)
	NET_C(R85.1, I_VM15)
	NET_C(R86.1, GND)
	NET_C(R87.2, Q12.B)
	NET_C(Q12.E, GND)
	NET_C(Q12.C, R88.1)
	NET_C(R88.2, R89.1, C18.2, R90.2, C19.1)
	NET_C(R89.2, I_V15)
	NET_C(C18.1, GND)
	NET_C(R90.1, GND)

#if (HLE_CAPTURE_VCO)
	//
	// The capture VCO actually doesn't sound bad at default
	// settings, but still takes up a lot of horsepower, so
	// HLE it as usual. The mappings aren't as good as they
	// usually are, but the sound is short and the result is
	// pretty indistinguishable from reality, so we'll go
	// with it.
	//
	//    R2 = 0.87013: HP = (0.000085296*A0) - 0.000965124
	//    R2 = 0.91754: HP = (0.0000395501*A0*A0) - (0.000953905*A0) + 0.00585023
	//    R2 = 0.91858: HP = (-0.0000119561*A0*A0*A0) + (0.000513446*A0*A0) - (0.00720853*A0) + 0.0333386
	//    R2 = 0.93063: HP = (-0.000085348*A0*A0*A0*A0) + (0.00450954*A0*A0*A0) - (0.089245*A0*A0) + (0.784128*A0) - 2.580951
	//    R2 = 0.84993: HP = (0.00000301512*A0*A0*A0*A0*A0) - (0.000286000*A0*A0*A0*A0) + (0.0098476*A0*A0*A0) - (0.160207*A0*A0) + (1.255519*A0) - 3.832746
	//
	VARCLOCK(CAPTURECLK, 1, "max(0.000001,min(0.1,(-0.0000119561*A0*A0*A0) + (0.000513446*A0*A0) - (0.00720853*A0) + 0.0333386))")
	NET_C(CAPTURECLK.GND, GND)
	NET_C(CAPTURECLK.VCC, I_V5)
	NET_C(CAPTURECLK.Q, U21.1)
	NET_C(CAPTURECLK.A0, C19.1)
	NET_C(GND, R91.1, R91.2, R92.1, R92.2, R93.1, R93.2, R94.1, R94.2, C19.2, C20.1, C20.2, C21.1, C21.2, D7.A, D7.K, D8.A, D8.K)
#else
	NET_C(R88.2, U42.5)
	NET_C(C19.2, U42.6, R91.1)
	NET_C(R91.2, I_V15, U42.8)
	NET_C(U42.7, C20.2)
	NET_C(C20.1, GND)
	NET_C(U42.1, GND)
	NET_C(U42.3, C21.1)
	NET_C(C21.2, D7.K, R92.1)
	NET_C(D7.A, GND)
	NET_C(R92.2, Q13.B)
	NET_C(Q13.C, R93.1, U21.1)
	NET_C(R93.2, I_V5)
	NET_C(Q13.E, D8.K, R94.2)
	NET_C(D8.A, GND)
	NET_C(R94.1, I_VM15)
#endif

	NET_C(U21.3, R95.1)
	HINT(U21.6, NC)
	HINT(U21.5, NC)
	HINT(U21.4, NC)
	NET_C(R95.2, R97.2, R96.1)
	NET_C(R97.1, GND)
	NET_C(R96.2, C22.1)
	NET_C(C22.2, SJ)

	//
	// Page 3, bottom-middle+right (photon)
	//

	NET_C(U35.1, GND)
	NET_C(U35.6, U35.2, R48.1, C13.2)
	NET_C(C13.1, GND)
	NET_C(R48.2, U35.7, R47.1)
	NET_C(R47.2, U35.8, U35.4, R49.2, R51.2, I_V5, Q7.E)
	NET_C(U35.5, C14.2)
	NET_C(C14.1, GND)
	NET_C(U35.3, R49.1, R50.1)
	NET_C(R50.2, R51.1, Q7.B)
	NET_C(Q7.C, R52.2, R54.2, R53.1)
	NET_C(R52.1, I_VM15)
	NET_C(R54.1, GND)
	NET_C(R53.2, Q8.B)
	NET_C(Q8.E, GND)
	NET_C(Q8.C, R58.1)
	NET_C(R58.2, R59.1, R60.2, C15.1, U36.3)
	NET_C(R59.2, I_V15)
	NET_C(R60.1, GND)
	NET_C(C15.2, GND)
	NET_C(U36.2, U36.6, R61.2)
	ALIAS(VP, R61.2)
	NET_C(R61.1, R62.1, U37.2)
	NET_C(R62.2, U37.6)
	ALIAS(VN, R62.2)
	NET_C(U37.3, GND)

	NET_C(VP, R63.1)
	NET_C(R63.2, U38.3, R64.1)
	NET_C(R64.2, U38.6, R65.1)
	NET_C(U38.2, R71.1, C16.2, U39.2)
	NET_C(C16.1, GND)

#if (HLE_PHOTON_VCO)
	//
	// The PHOTON VCO is modulated by the 555 timer U35,
	// fed through some capacitors and an op-amp to
	// produce a periodic charge/discharge curve. The
	// output of the op-amp U36 tracks the final
	// frequency pretty closely, but only if charge and
	// discharge curves are considered separately. Use
	// the raw U35.3 output as a switch to pick the
	// appropriate curve.
	//
	// U35.3 on:
	//    R2 = 0.95303: HP = (0.000454865*A0) - 0.000727100
	//    R2 = 0.97158: HP = (0.0000320398*A0*A0) + (0.000096187*A0) + 0.000101501
	//    R2 = 0.99149: HP = (0.0000173739*A0*A0*A0) - (0.000265948*A0*A0) + (0.00163589*A0) - 0.00222259
	//    R2 = 0.99201: HP = (-0.00000151250*A0*A0*A0*A0) + (0.0000522525*A0*A0*A0) - (0.000547752*A0*A0) + (0.00256844*A0) - 0.00327534
	//    R2 = 0.99549: HP = (0.00000220778*A0*A0*A0*A0*A0) - (0.000065312*A0*A0*A0*A0) + (0.000753266*A0*A0*A0) - (0.00418112*A0*A0) + (0.0113844*A0) - 0.0112392
	//
	// U35.3 off:
	//    R2 = 0.18174: HP = (0.000065384*A0) + 0.000468783
	//    R2 = 0.83543: HP = (-0.0000468095*A0*A0) + (0.000655196*A0) - 0.000641642
	//    R2 = 0.99434: HP = (-0.0000088969*A0*A0*A0) + (0.000127464*A0*A0) - (0.000276787*A0) + 0.000661774
	//    R2 = 0.99978: HP = (-0.000000655767*A0*A0*A0*A0) + (0.0000088937*A0*A0*A0) - (0.0000336865*A0*A0) + (0.000281719*A0) + 0.0000419612
	//    R2 = 0.99987: HP = (0.0000000525584*A0*A0*A0*A0*A0) - (0.00000241385*A0*A0*A0*A0) + (0.0000304913*A0*A0*A0) - (0.000153608*A0*A0) + (0.000579685*A0) - 0.000223579
	//
	VARCLOCK(PHOTONCLK, 2, "max(0.000001,min(0.1,if(A1>2.5,(0.0000173739*A0*A0*A0) - (0.000265948*A0*A0) + (0.00163589*A0) - 0.00222259,(-0.0000088969*A0*A0*A0) + (0.000127464*A0*A0) - (0.000276787*A0) + 0.000661774)))")
	NET_C(PHOTONCLK.GND, GND)
	NET_C(PHOTONCLK.VCC, I_V5)
	NET_C(PHOTONCLK.Q, PHOTONENV.A0)
	NET_C(PHOTONCLK.A0, U36.6)
	NET_C(PHOTONCLK.A1, U35.3)
	AFUNC(PHOTONENV, 1, "if(A0>2.5,0.0038,-0.0038)")
	NET_C(PHOTONENV.Q, U41.2)
	NET_C(GND, U40.2, U40.3, R65.2, R66.1, R66.2, R69.2, R70.1, R70.2, R71.2, R72.1, R72.2, R73.2, D5.A, D5.K, D6.A, D6.K)
#else
	NET_C(R65.2, U40.3, R70.1, R69.2)
	NET_C(U40.2, GND, D5.A)
	NET_C(D5.K, D6.K)
	NET_C(D6.A, R71.2, R72.2, R66.2)
	NET_C(U40.6, R66.1, R70.2)
	NET_C(R72.1, R73.2, U41.2)
#endif

	NET_C(R73.1, GND)
	NET_C(U41.3, R74.2)
	NET_C(R74.1, GND)
	NET_C(VN, R67.1)
	NET_C(R67.2, U39.3, R68.1)
	NET_C(R68.2, U39.6, R69.1)

	NET_C(PHOTON_M, R75.1, R76.1)
	NET_C(R75.2, I_V5, R77.2, Q9.E)
	NET_C(R76.2, R77.1, Q9.B)
	NET_C(Q9.C, R78.2, R79.2, Q10.E)
	NET_C(R78.1, I_VM15)
	NET_C(R79.1, GND)
	NET_C(Q10.B, R80.2)
	NET_C(R80.1, GND)
	NET_C(Q10.C, C17.1, R81.1)
	NET_C(C17.2, I_VM15)
	NET_C(R81.2, U41.5)

	NET_C(U41.6, CS, R149.2, U49.3)
	NET_C(R149.1, GND)
	NET_C(U49.2, U49.6, C31.1)
	NET_C(C31.2, R150.1)
	NET_C(R150.2, SJ, U50.2, R151.1)
	NET_C(U50.3, GND)
	NET_C(R151.2, U50.6)
	ALIAS(OUTPUT, R151.2)

	//
	// Unconnected inputs
	//

	NET_C(GND, U2.8, U2.10, U22.2, U22.3, U22.5, U22.6, U24.3, U27.8, U27.9, U27.10, U27.11)

	//
	// Unconnected outputs
	//

	HINT(U2.9, NC)
	HINT(U2.11, NC)
	HINT(U24.4, NC)

#if (ENABLE_FRONTIERS)
	//
	// Isolate the NOISE consumers from one another; the first one
	// in particular is a big win
	//
	OPTIMIZE_FRONTIER(R100.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R15.1, RES_M(1), 50)
	OPTIMIZE_FRONTIER(R34.1, RES_M(1), 50)

	//
	// Isolate the CS sounds from the rest of the mixer (huge!)
	//
	OPTIMIZE_FRONTIER(U49.3, RES_M(1), 50)
#endif

}
