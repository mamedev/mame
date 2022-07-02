// license:CC0
// copyright-holders:Aaron Giles

//
// Netlist for Rip Off
//
// Derived from the schematics in the Rip Off manual.
//
// Known problems/issues:
//
//    * Voltage triggers for Motor 1 and Beep need a hack to
//       work (reducing resistance on one resistor from 4.7K to 100)
//       Need to understand why.
//
//    * Motor 1 and beep sounds dominate the output. They are
//       controlled by the current driven into the CCAs IC12 and
//       IC6. Not sure if this overdrive is related to the problem
//       with the switch above, but for now a hack is enabled to
//       multiply the resistance by 5x going into the CCA, which
//       seems to restore the balance to something reasonable.
//

#include "netlist/devices/net_lib.h"
#include "nl_cinemat_common.h"

//
// Optimizations
//

#define HLE_LASER_VCO (1)
#define HLE_TORPEDO_VCO (1)
#define HLE_BACKGROUND_VCOS (1)
#define ENABLE_FRONTIERS (1)


//
// Hacks
//

#define HACK_VOLTAGE_SWITCH (1)
#define HACK_CCA_RESISTANCE (1)


//
// Main netlist
//

NETLIST_START(ripoff)
{

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

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_V15, 15)
	ANALOG_INPUT(I_VM15, -15)

	RES(R1, RES_K(47))
	RES(R2, RES_K(47))
	RES(R3, RES_K(330))
	RES(R4, RES_K(10))
	RES(R5, RES_K(2.7))
	RES(R6, RES_K(2.7))
	RES(R7, RES_K(1))
	RES(R8, RES_K(470))
	RES(R9, 330)
	RES(R10, RES_K(20))
#if (HACK_VOLTAGE_SWITCH)
	RES(R11, 100)
#else
	RES(R11, RES_K(4.7))
#endif
	RES(R12, 300)
	RES(R13, RES_K(4.7))
	RES(R14, RES_K(8.2))
	RES(R15, RES_K(15))
	RES(R16, RES_K(2.7))
	RES(R17, RES_K(2.7))
#if (HACK_CCA_RESISTANCE)
	RES(R18, RES_K(4100))
#else
	RES(R18, RES_K(820))
#endif
	RES(R19, RES_K(27))
	RES(R20, RES_K(39))
	RES(R21, RES_K(10))
	RES(R22, 330)
	RES(R23, 160)
	RES(R24, RES_K(1))
	RES(R25, RES_K(1))
	RES(R26, RES_K(1))
	RES(R27, RES_K(2))
	RES(R28, RES_K(2))
	RES(R29, RES_K(4.7))
	RES(R30, RES_K(3.3))
	RES(R31, RES_K(68))
	RES(R32, RES_K(30))
	RES(R33, RES_K(15))
	RES(R34, RES_K(2.7))
	RES(R35, RES_K(2.7))
#if (HACK_CCA_RESISTANCE)
	RES(R36, RES_M(5))
#else
	RES(R36, RES_M(1))
#endif
	RES(R37, RES_K(56))
	RES(R38, RES_K(24))
	RES(R39, RES_K(15))
#if (HACK_VOLTAGE_SWITCH)
	RES(R40, 100)
#else
	RES(R40, RES_K(4.7))
#endif
	RES(R41, 300)
	RES(R42, RES_K(39))
	RES(R43, RES_K(10))
	RES(R44, 330)
	RES(R45, RES_K(27))
//  RES(R46, RES_K(12))  -- part of final amp (not emulated)
	RES(R47, RES_K(1))
	RES(R48, RES_K(2.7))
	RES(R49, 470)
//  POT(R50, RES_K())    -- listed as optional on schematics
//  RES(R51, RES_K(1.5)) -- part of final amp (not emulated)
//  RES(R52, 150)        -- part of final amp (not emulated)
//  RES(R53, RES_K(22))  -- part of final amp (not emulated)
//  RES(R54, 150)        -- part of final amp (not emulated)
//  RES(R55, RES_K(39))  -- part of final amp (not emulated)
	RES(R56, 150)
	RES(R57, RES_K(2.7))
	RES(R58, RES_M(1))
	RES(R59, RES_K(20))
	RES(R60, RES_K(10))
	RES(R61, RES_K(1))
	RES(R62, RES_K(10))
	RES(R63, RES_K(20))
	RES(R64, RES_K(39))
	RES(R65, RES_K(82))
	RES(R66, RES_K(2.7))
	RES(R67, RES_K(3.9))
	RES(R68, RES_M(1))
	RES(R69, RES_K(20))
	RES(R70, RES_K(10))
	RES(R71, RES_K(1))
	RES(R72, RES_K(10))
	RES(R73, RES_K(20))
	RES(R74, RES_K(39))
	RES(R75, RES_K(82))
	RES(R76, 470)
	RES(R77, RES_M(1.5))
	RES(R78, RES_K(20))
	RES(R79, RES_M(1))
	RES(R80, RES_K(43))
	RES(R81, RES_K(20))
	RES(R82, RES_M(1))
	RES(R83, RES_K(20))
	RES(R84, RES_K(10))
	RES(R85, RES_K(1))
	RES(R86, RES_K(10))
	RES(R87, RES_K(20))
	RES(R88, RES_K(39))
	RES(R89, RES_K(82))
	RES(R90, RES_K(1))
	RES(R91, RES_K(2.7))

	CAP(C1, CAP_U(100))
	CAP(C2, CAP_U(0.1))
	CAP(C3, CAP_U(0.1))
	CAP(C4, CAP_U(0.02))
	CAP(C5, CAP_U(0.1))
	CAP(C6, CAP_U(0.68))
	CAP(C7, CAP_U(0.01))
	CAP(C8, CAP_U(4.7))
	CAP(C9, CAP_U(0.047))
	CAP(C10, CAP_U(0.01))
	CAP(C11, CAP_U(0.1))
	CAP(C12, CAP_U(0.1))
//  CAP(C13, CAP_U())      -- not used according to schematics
	CAP(C14, CAP_U(0.22))
	CAP(C15, CAP_U(0.01))
	CAP(C16, CAP_U(0.1))
	CAP(C17, CAP_U(0.1))
	CAP(C18, CAP_U(0.01))
	CAP(C19, CAP_U(0.1))
//  CAP(C20, CAP_U())      -- not used according to schematics
	CAP(C21, CAP_U(0.68))
//  CAP(C22, CAP_U(0.005)) -- part of final amp (not emulated)
//  CAP(C23, CAP_P(470))   -- part of final amp (not emulated)
//  CAP(C24, CAP_P(470))   -- part of final amp (not emulated)
//  CAP(C25, CAP_P(470))   -- part of final amp (not emulated)
	CAP(C26, CAP_U(0.1))
	CAP(C27, CAP_U(0.1))
	CAP(C28, CAP_U(0.01))
	CAP(C29, CAP_U(0.047))
	CAP(C30, CAP_U(0.22))
	CAP(C31, CAP_U(0.1))
	CAP(C32, CAP_U(0.68))
//  CAP(C33, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C34, CAP_U(25))    -- part of voltage converter (not emulated)
//  CAP(C35, CAP_U(25))    -- part of voltage converter (not emulated)
//  CAP(C36, CAP_U())      -- part of voltage converter (not emulated)
//  CAP(C37, CAP_U(0.1))   -- part of voltage converter (not emulated)
//  CAP(C38, CAP_U(25))    -- part of voltage converter (not emulated)
//  CAP(C39, CAP_U(25))    -- part of voltage converter (not emulated)
//  CAP(C40, CAP_U())      -- part of voltage converter (not emulated)
//  CAP(C41, CAP_U(25))    -- part of voltage converter (not emulated)
//  CAP(C42, CAP_U(0.1))   -- part of voltage converter (not emulated)

	D_1N5240(D1)
	D_1N914(D2)
	D_1N914(D3)
	D_1N914(D4)
	D_1N914(D5)
	D_1N914(D6)
	D_1N914(D7)
	D_1N914(D8)
	D_1N914(D9)
	D_1N914(D10)
	D_1N914(D11)
	D_1N5240(D12)
	D_1N5240(D13)

	Q_2N3906(Q1)            // PNP
	Q_2N3906(Q2)            // PNP
	Q_2N3906(Q3)            // PNP
	Q_2N3906(Q4)            // PNP
	Q_2N3906(Q5)            // PNP
//  Q_2N6292(Q6)            // PNP -- part of final amp (not emulated)
//  Q_2N6107(Q7)            // PNP -- part of final amp (not emulated)
#if !(HLE_LASER_VCO)
	Q_2N3904(Q8)            // NPN
#endif
#if !(HLE_TORPEDO_VCO)
	Q_2N3904(Q9)            // NPN
#endif
#if !(HLE_BACKGROUND_VCOS)
	Q_2N3904(Q10)           // NPN
#endif

	AMI_S2688(IC1)          // Noise generator

	TL081_DIP(IC2)          // Op. Amp.
	NET_C(IC2.7, I_V15)
	NET_C(IC2.4, I_VM15)

	CA3080_DIP(IC3)         // Op. Amp.
	NET_C(IC3.4, I_VM15)
	NET_C(IC3.7, I_V15)

	LM555_DIP(IC4)

	LM555_DIP(IC5)

	CA3080_DIP(IC6)         // Op. Amp.
	NET_C(IC6.4, I_VM15)
	NET_C(IC6.7, I_V15)

	TL081_DIP(IC7)          // Op. Amp.
	NET_C(IC7.7, I_V15)
	NET_C(IC7.4, I_VM15)

	TTL_74LS164_DIP(IC8)    // 8-bit Shift Reg.
	NET_C(IC8.7, GND)
	NET_C(IC8.14, I_V5)

	TTL_74LS377_DIP(IC9)    // Octal D Flip Flop
	NET_C(IC9.10, GND)
	NET_C(IC9.20, I_V5)

	TTL_7406_DIP(IC10)      // Hex inverter -- currently using a clone of 7416, no open collector behavior
	NET_C(IC10.7, GND)
	NET_C(IC10.14, I_V5)

	LM555_DIP(IC11)

	CA3080_DIP(IC12)        // Op. Amp.
	NET_C(IC12.4, I_VM15)
	NET_C(IC12.7, I_V15)

	LM555_DIP(IC13)

	TL081_DIP(IC14)         // Op. Amp.
	NET_C(IC14.7, I_V15)
	NET_C(IC14.4, I_VM15)

	TL081_DIP(IC15)         // Op. Amp.
	NET_C(IC15.7, I_V15)
	NET_C(IC15.4, I_VM15)

	TL081_DIP(IC16)         // Op. Amp.
	NET_C(IC16.7, I_V15)
	NET_C(IC16.4, I_VM15)

	TL081_DIP(IC17)         // Op. Amp.
	NET_C(IC17.7, I_V15)
	NET_C(IC17.4, I_VM15)

	TTL_74LS393_DIP(IC18)   // Dual 4 Bit B.C.
	NET_C(IC18.7, GND)
	NET_C(IC18.14, I_V5)

	TL081_DIP(IC19)         // Op. Amp.
	NET_C(IC19.7, I_V15)
	NET_C(IC19.4, I_VM15)

	TL081_DIP(IC20)         // Op. Amp.
	NET_C(IC20.7, I_V15)
	NET_C(IC20.4, I_VM15)

	TL081_DIP(IC21)         // Op. Amp.
	NET_C(IC21.7, I_V15)
	NET_C(IC21.4, I_VM15)

	TTL_74LS393_DIP(IC22)   // Dual 4 Bit B.C.
	NET_C(IC22.7, GND)
	NET_C(IC22.14, I_V5)

//  TTL_7915_DIP(IC23)      // -15V Regulator -- not emulated
//  TTL_7815_DIP(IC24)      // +15V Regulator -- not emulated

	TTL_7414_DIP(IC25)      // Hex Inverter
	NET_C(IC25.7, GND)
	NET_C(IC25.14, I_V5)

	//
	// Explosion
	//

	NET_C(I_OUT_7, R7.1, IC4.2)
	NET_C(IC4.8, IC4.4, I_V5)   // pin 4 not documented in schematics
	NET_C(R7.2, I_V5)
	NET_C(R8.2, I_V5)
	NET_C(R8.1, IC4.6, IC4.7, C6.1)
	NET_C(C6.2, GND)
	NET_C(IC4.5, C7.2)
	NET_C(C7.1, GND)
	NET_C(IC4.1, GND)
	NET_C(IC4.3, Q1.E)
	NET_C(Q1.B, R9.2)
	NET_C(R9.1, GND)
	NET_C(Q1.C, C8.1, R10.1)
	NET_C(C8.2, I_VM15)
	NET_C(R10.2, IC3.5)

	NET_C(C1.1, IC1.4, I_V15)
	NET_C(C1.2, GND)
	NET_C(IC1.1, IC1.2, GND)
	NET_C(IC1.3, R1.1)
	NET_C(R1.2, C2.2, R2.1)
	NET_C(C2.1, GND)
	NET_C(R2.2, C3.1)
	NET_C(C3.2, IC2.2, C4.1, R3.1)
	NET_C(IC2.3, GND)
	NET_C(IC2.6, C4.2, R3.2, R4.1)
	NET_C(R4.2, C5.1)
	NET_C(C5.2, R5.2, IC3.2)
	NET_C(R5.1, GND)
	NET_C(IC3.3, R6.2)
	NET_C(R6.1, GND)
	NET_C(IC3.6, IC7.3, R19.2)
	NET_C(R19.1, GND)

	//
	// Shift register
	//

	NET_C(I_OUT_0, IC25.1)
	NET_C(IC25.2, IC25.13)
	NET_C(IC25.12, IC8.2)
	NET_C(I_OUT_1, IC25.3)
	NET_C(IC25.4, IC25.11)
	NET_C(IC25.10, IC8.8)
	NET_C(R24.1, I_V5)
	NET_C(R24.2, IC8.9, IC8.1)
	NET_C(I_OUT_2, IC25.5)
	NET_C(IC25.6, IC25.9)
	NET_C(IC25.8, IC9.11)
	NET_C(IC9.1, GND)
	NET_C(IC8.3, IC9.3)
	NET_C(IC8.4, IC9.4)
	NET_C(IC8.5, IC9.7)
	NET_C(IC8.6, IC9.8)
	NET_C(IC8.10, IC9.13)
	NET_C(IC8.11, IC9.14)

	//
	// Background
	//

	NET_C(IC9.9, IC22.2)
	NET_C(IC9.2, IC10.1)
	NET_C(IC10.2, R25.2, R26.1)
	NET_C(IC9.5, IC10.3)
	NET_C(IC10.4, R27.2, R28.1)
	NET_C(IC9.6, IC10.5)
	NET_C(IC10.6, R29.2, R30.1)
	NET_C(R29.1, R27.1, R25.1, D1.K, R23.1) // also R50.2 if present
	NET_C(R23.2, I_V15)
	NET_C(D1.A, GND)
	NET_C(R26.2, R28.2, R30.2, R76.1, IC19.2) // also R50.1 if present
	NET_C(IC19.3, GND)
	NET_C(IC19.6, R76.2, D8.A, D6.A)

#if (HLE_BACKGROUND_VCOS)
	//
	// The two background VCOs are done with diodes and op-amps,
	// but end up generating a quite linear voltage-to-period
	// mapping. There is a low-frequency VCO and a high-frequency
	// one. They are combined and sent to an LS393 counter as a
	// clock after going through a voltage converter. Here we
	// skip the whole lot.
	//
	// First VCO, vs IC19.6:
	//    R2 = 0.99406: HP = (-0.0235033*A0) + 0.0179360
	//    R2 = 0.99415: HP = (0.000193041*A0*A0) - (0.0227932*A0) + 0.0182436
	//    R2 = 0.99418: HP = (-0.000106682*A0*A0*A0) - (0.000443621*A0*A0) - (0.0237264*A0) + 0.0180362
	//    R2 = 0.99419: HP = (0.000069781*A0*A0*A0*A0) + (0.000470159*A0*A0*A0) + (0.00104384*A0*A0) - (0.0224827*A0) + 0.0182202
	//    R2 = 0.99419: HP = (-0.0000242172*A0*A0*A0*A0*A0) - (0.000186286*A0*A0*A0*A0) - (0.000483269*A0*A0*A0) - (0.000431519*A0*A0) - (0.0233265*A0) + 0.0181209
	//
	// Second VCO, vs IC19.6:
	//    R2 = 0.99969: HP = (-0.000308955*A0) + 0.000256399
	//    R2 = 0.99986: HP = (0.00000356099*A0*A0) - (0.000295774*A0) + 0.000262224
	//    R2 = 0.99986: HP = (-0.000000646439*A0*A0*A0) - (0.000000309249*A0*A0) - (0.000301475*A0) + 0.000260938
	//    R2 = 0.99986: HP = (0.0000000315553*A0*A0*A0*A0) - (0.000000385224*A0*A0*A0) + (0.000000365474*A0*A0) - (0.000300909*A0) + 0.000261022
	//    R2 = 0.99986: HP = (-0.0000000265364*A0*A0*A0*A0*A0) - (0.000000249089*A0*A0*A0*A0) - (0.00000143038*A0*A0*A0) - (0.00000125235*A0*A0) - (0.000301835*A0) + 0.000260913
	//
	VARCLOCK(BGCLK1, 1, "max(0.000001,min(0.1,(-0.0235033*A0) + 0.0179360))")
	NET_C(BGCLK1.GND, GND)
	NET_C(BGCLK1.VCC, I_V15)
	NET_C(BGCLK1.A0, IC19.6)
	NET_C(BGCLK1.Q, BGCOMBINE.A0)
	NET_C(GND, R77.1, R77.2, R78.1, R78.2, R79.1, R79.2, C30.1, C30.2, D6.K, D7.A, D7.K, IC20.2, IC20.3)

	VARCLOCK(BGCLK2, 1, "max(0.000001,min(0.1,(-0.000308955*A0) + 0.000256399))")
	NET_C(BGCLK2.GND, GND)
	NET_C(BGCLK2.VCC, I_V15)
	NET_C(BGCLK2.A0, IC19.6)
	NET_C(BGCLK2.Q, BGCOMBINE.A1)
	NET_C(GND, R80.1, R80.2, R81.1, R81.2, R82.1, R82.2, C31.1, C31.2, D8.K, D9.A, D9.K, IC21.2, IC21.3)

	AFUNC(BGCOMBINE, 2, "max(A0,A1)")
	NET_C(BGCOMBINE.Q, IC22.1)
	NET_C(GND, R83.1, R83.2, R84.1, R84.2, R85.1, R85.2, D10.A, D10.K, D11.A, D11.K)
#else
	NET_C(D6.K, D7.A, R79.1, IC20.3)
	NET_C(D7.K, GND)
	NET_C(IC20.2, C30.2, R77.1)
	NET_C(C30.1, GND)
	NET_C(IC20.6, R77.2, D10.A, R78.1, R79.2)
	NET_C(D10.K, R78.2, R83.1, R81.2, D11.K)
	NET_C(D11.A, R82.2, IC21.6, R80.2, R81.1)
	NET_C(IC21.3, R82.1, D9.A, D8.K)
	NET_C(D9.K, GND)
	NET_C(IC21.2, C31.2, R80.1)
	NET_C(C31.1, GND)
	NET_C(R83.2, R84.2, Q10.B)
	NET_C(R84.1, GND)
	NET_C(Q10.E, GND)
	NET_C(Q10.C, R85.1, IC22.1)
	NET_C(R85.2, I_V5)
#endif
	NET_C(IC22.3, R86.1)
	NET_C(IC22.4, R87.1)
	NET_C(IC22.5, R88.1)
	NET_C(IC22.6, R89.1)
	NET_C(R86.2, R87.2, R88.2, R89.2, R90.2, R91.1)
	NET_C(R90.1, GND)
	NET_C(R91.2, C32.1)
	NET_C(C32.2, R49.1)
	NET_C(R49.2, R45.1)

	//
	// Beep
	//

	NET_C(IC9.12, Q2.B, R11.1)
	NET_C(R11.2, R12.2, I_V5)
	NET_C(Q2.E, R12.1)
	NET_C(Q2.C, R20.2, R21.2, Q3.E)
	NET_C(R20.1, I_VM15)
	NET_C(R21.1, GND)
	NET_C(Q3.B, R22.2)
	NET_C(R22.1, GND)
	NET_C(Q3.C, R18.1)
	NET_C(R18.2, IC6.5)

	NET_C(R13.2, IC5.4, IC5.8, I_V5)
	NET_C(R13.1, IC5.7, R14.2)
	NET_C(R14.1, IC5.6, IC5.2, C9.2)
	NET_C(C9.1, GND)
	NET_C(IC5.1, GND)
	NET_C(IC5.5, C10.2)
	NET_C(C10.1, GND)
	NET_C(IC5.3, R15.1)
	NET_C(R15.2, C11.1)
	NET_C(C11.2, R16.2, IC6.2)
	NET_C(R16.1, GND)
	NET_C(IC6.3, R17.2)
	NET_C(R17.1, GND)
	NET_C(IC6.6, IC7.3)

	NET_C(IC7.6, IC7.2, C12.1)
	NET_C(C12.2, R45.2)
	ALIAS(OUTPUT, R45.1)

	//
	// Motor 1
	//

	NET_C(IC9.15, R40.1, Q4.B)
	NET_C(R40.2, R41.1, I_V5)
	NET_C(R41.2, Q4.E)
	NET_C(Q4.C, R42.2, R43.2, Q5.E)
	NET_C(R42.1, I_VM15)
	NET_C(R43.1, GND)
	NET_C(Q5.B, R44.2)
	NET_C(R44.1, GND)
	NET_C(Q5.C, R36.1)
	NET_C(R36.2, IC12.5)

	NET_C(R31.2, IC11.4, IC11.8, I_V5)
	NET_C(R31.1, IC11.7, R32.2)
	NET_C(R32.1, IC11.6, IC11.2, C14.2)
	NET_C(C14.1, GND, IC11.1, C15.1)
	NET_C(C15.2, IC11.5)
	NET_C(IC11.3, R33.1)
	NET_C(R33.2, C16.1)
	NET_C(C16.2, R34.2, IC12.2, C19.2)
	NET_C(R34.1, GND)
	NET_C(IC12.3, R35.2)
	NET_C(R35.1, GND)
	NET_C(IC12.6, D12.A, IC7.3)
	NET_C(D12.K, D13.K)
	NET_C(D13.A, GND)

	NET_C(R37.2, IC13.4, IC13.8, I_V5)
	NET_C(R37.1, IC13.7, R38.2)
	NET_C(R38.1, IC13.6, IC13.2, C17.2)
	NET_C(C17.1, GND)
	NET_C(IC13.1, GND)
	NET_C(IC13.5, C18.2)
	NET_C(C18.1, GND)
	NET_C(IC13.3, R39.1)
	NET_C(R39.2, C19.1)

	//
	// Laser
	//

	NET_C(I_OUT_4, IC10.13, IC18.12)
	NET_C(IC10.12, R56.1)
	NET_C(R56.2, C26.2, D2.K)
	NET_C(C26.1, GND)

#if (HLE_LASER_VCO)
	//
	// This is a typical Cinemtraonics VCO, driving a TTL counter.
	// Netlist simulation requires a very small step which is not
	// realtime performant, so we model it offline with the small
	// step count, and then for realtime performance replace it
	// with a mapping.
	//
	// Here is the mapping between 26.2 and the TTL clock IC18.13
	// when the circuit is present:
	//
	//    R2 = 0.99566: HP = (0.0000211106*A0) + 0.0000233926
	//    R2 = 0.99925: HP = (0.000000616538*A0*A0) + (0.0000166899*A0) + 0.0000239800
	//    R2 = 0.99946: HP = (0.000000065592*A0*A0*A0) - (0.000000207181*A0*A0) + (0.0000190091*A0) + 0.0000237318
	//    R2 = 0.99946: HP = (0.00000000399629*A0*A0*A0*A0) - (0.00000000517200*A0*A0*A0) + (0.000000172470*A0*A0) + (0.0000184103*A0) + 0.0000237906
	//    R2 = 0.99946: HP = (0.0000000000207619*A0*A0*A0*A0*A0) + (0.00000000352382*A0*A0*A0*A0) - (0.00000000145304*A0*A0*A0) + (0.000000160778*A0*A0) + (0.0000184225*A0) + 0.0000237894
	//
	// And here is the mapping when the circuit is removed:
	//
	//    R2 = 0.98806: HP = (0.00245939*A0) - 0.000220173
	//    R2 = 0.99179: HP = (0.00545522*A0*A0) + (0.00103499*A0) - 0.000132203
	//    R2 = 0.99913: HP = (-0.102004*A0*A0*A0) + (0.0406935*A0*A0) - (0.00277839*A0) - 0.00000140594
	//    R2 = 0.99941: HP = (0.651650*A0*A0*A0*A0) - (0.377655*A0*A0*A0) + (0.078834*A0*A0) - (0.00454290*A0) + 0.00000408830
	//    R2 = 0.99941: HP = (2.264700*A0*A0*A0*A0*A0) - (0.633172*A0*A0*A0*A0) - (0.108936*A0*A0*A0) + (0.0542944*A0*A0) - (0.00371696*A0) + 0.00000401368
	//
	VARCLOCK(LASERCLK, 1, "max(0.000001,min(0.1,(-0.102004*A0*A0*A0) + (0.0406935*A0*A0) - (0.00277839*A0) - 0.00000140594))")
	NET_C(LASERCLK.GND, GND)
	NET_C(LASERCLK.VCC, I_V5)
	NET_C(LASERCLK.A0, C26.2)
	NET_C(LASERCLK.Q, IC18.13)
	NET_C(GND, R57.1, R57.2, R58.1, R58.2, R59.1, R59.2, R60.1, R60.2, R61.1, R61.2, C27.1, C27.2, D2.A, D3.A, D3.K, IC16.2, IC16.3)
#else
	NET_C(D2.A, IC16.3, D3.K, R58.1)
	NET_C(D3.A, GND)
	NET_C(IC16.2, R57.1, C27.2)
	NET_C(C27.1, GND)
	NET_C(IC16.6, R57.2, R58.2, R59.1)
	NET_C(R59.2, R60.2, Q8.B)
	NET_C(R60.1, GND)
	NET_C(Q8.E, GND)
	NET_C(Q8.C, R61.1, IC18.13)
	NET_C(R61.2, I_V5)
#endif

	NET_C(IC18.9, R62.1)
	NET_C(IC18.8, R63.1)
	NET_C(IC18.10, R64.1)
	NET_C(IC18.11, R65.1)
	NET_C(R62.2, R63.2, R64.2, R65.2, R47.2, R48.1)
	NET_C(R47.1, GND)
	NET_C(R48.2, C21.1)
	NET_C(C21.2, R49.1)

	//
	// Torpedo
	//

	NET_C(I_OUT_3, IC10.11, IC18.2)
	NET_C(IC10.10, R66.1)
	NET_C(R66.2, C28.2, D4.K)
	NET_C(C28.1, GND)

#if (HLE_TORPEDO_VCO)
	//
	// Another tricky Cinematronics VCO. Here is the mapping between
	// C28.2 and the TTL clock IC18.1 when the circuit is present:
	//
	//    R2 = 0.97255: HP = (0.0000327662*A0) + 0.0000149365
	//    R2 = 0.99171: HP = (0.00000331016*A0*A0) - (0.0000125849*A0) + 0.0000206090
	//    R2 = 0.99613: HP = (0.000000536978*A0*A0*A0) - (0.0000078207*A0*A0) + (0.0000390037*A0) + 0.0000148210
	//    R2 = 0.99748: HP = (0.000000092476*A0*A0*A0*A0) - (0.00000204069*A0*A0*A0) + (0.0000139512*A0*A0) - (0.0000143884*A0) + 0.0000205567
	//    R2 = 0.99798: HP = (0.0000000168890*A0*A0*A0*A0*A0) - (0.000000498868*A0*A0*A0*A0) + (0.00000512183*A0*A0*A0) - (0.0000205655*A0*A0) + (0.0000394849*A0) + 0.0000149619
	//
	// And here is the mapping when the circuit is removed:
	//
	//    R2 = 0.83356: HP = (0.000387263*A0) - 0.0000161072
	//    R2 = 0.96482: HP = (-0.000459844*A0*A0) + (0.00110697*A0) - 0.000085961
	//    R2 = 0.99210: HP = (0.000512521*A0*A0*A0) - (0.00183716*A0*A0) + (0.00202778*A0) - 0.000165571
	//    R2 = 0.99614: HP = (-0.000450996*A0*A0*A0*A0) + (0.00218565*A0*A0*A0) - (0.00381123*A0*A0) + (0.00280823*A0) - 0.000225737
	//    R2 = 0.99625: HP = (-0.000161974*A0*A0*A0*A0*A0) + (0.000309626*A0*A0*A0*A0) + (0.000915096*A0*A0*A0) - (0.00291505*A0*A0) + (0.00256587*A0) - 0.000209241
	//
	VARCLOCK(TORPEDOCLK, 1, "max(0.000001,min(0.1,(0.000512521*A0*A0*A0) - (0.00183716*A0*A0) + (0.00202778*A0) - 0.000165571))")
	NET_C(TORPEDOCLK.GND, GND)
	NET_C(TORPEDOCLK.VCC, I_V5)
	NET_C(TORPEDOCLK.A0, C28.2)
	NET_C(TORPEDOCLK.Q, IC18.1)
	NET_C(GND, R67.1, R67.2, R68.1, R68.2, R69.1, R69.2, R70.1, R70.2, R71.1, R71.2, C29.1, C29.2, D4.A, D5.A, D5.K, IC17.2, IC17.3)
#else
	NET_C(D4.A, IC17.3, D5.K, R68.1)
	NET_C(D5.A, GND)
	NET_C(IC17.2, C29.2, R67.1)
	NET_C(C29.1, GND)
	NET_C(IC17.6, R67.2, R68.2, R69.1)
	NET_C(R69.2, R70.2, Q9.B)
	NET_C(R70.1, GND)
	NET_C(Q9.E, GND)
	NET_C(Q9.C, R71.1, IC18.1)
	NET_C(R71.2, I_V5)
#endif

	NET_C(IC18.5, R72.1)
	NET_C(IC18.6, R73.1)
	NET_C(IC18.4, R74.1)
	NET_C(IC18.3, R75.1)
	NET_C(R72.2, R73.2, R74.2, R75.2, R48.1)

	//
	// Unconnected inputs
	//

	NET_C(GND, IC9.17, IC9.18, IC10.9, IC22.12, IC22.13)
	NET_C(GND, IC14.2, IC14.3, IC15.2, IC15.3)  // part of final amp

	//
	// Unconnected outputs
	//

	HINT(IC9.16, NC)        // Q6
	HINT(IC9.19, NC)        // Q7
	HINT(IC10.8, NC)        // QD
	HINT(IC22.11, NC)       // Q0
	HINT(IC22.10, NC)       // Q1
	HINT(IC22.9, NC)        // Q2
	HINT(IC22.8, NC)        // Q3

#if (ENABLE_FRONTIERS)
	//
	// Split explosion/beep/motor from other sources
	//
	OPTIMIZE_FRONTIER(R45.2, RES_M(1), 50)
	OPTIMIZE_FRONTIER(IC7.3, RES_M(1), 50)

	//
	// Split noise generator from consumers
	//
	OPTIMIZE_FRONTIER(R1.1, RES_M(1), 50)
#endif

}
