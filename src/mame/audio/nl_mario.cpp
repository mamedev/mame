// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "netlist/devices/net_lib.h"

/* ----------------------------------------------------------------------------
 *  Library section header START
 * ---------------------------------------------------------------------------*/

#ifndef __PLIB_PREPROCESSOR__

#endif

/* ----------------------------------------------------------------------------
 *  Library section header END
 * ---------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
 *  Mario schematics
 * ---------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------- */
/* mario sound                                                            */
/* ---------------------------------------------------------------------- */
static NETLIST_START(nl_mario_snd0)

	RES(R17, RES_K(27))               /* 20 according to parts list           */
										/* 27 verified, 30K in schematics       */
	CAP(C14, CAP_U(4.7))              /* verified                             */

	TTL_74123(2H_A)
	NET_C(2H_A.VCC, V5)
	NET_C(2H_A.GND, GND)
	NET_C(SOUND0.Q, 2H_A.B)
	NET_C(GND, 2H_A.A)
	NET_C(2H_A.CLRQ, ttlhigh)         /* NOT IN SCHEMATICS                    */
	DIODE(D1, "1N4148")               /* FIXME: try to identify */
	TTL_7404_INVERT(1H_A, 2H_A.QQ)
	NET_C(R17.1, V5)
	NET_C(R17.2, D1.A, C14.1)
	NET_C(D1.K, 2H_A.RC)
	NET_C(C14.2, 2H_A.C)

	RES(R6, RES_K(4.7))               /* verified                             */
	CAP(C3, CAP_U(10))                /* verified                             */

	NET_C(1H_A.Q, R6.1)
	NET_C(R6.2, C3.1, 1J_A.FC)
	NET_C(R6.2, 2J_A.FC)
	NET_C(C3.2, GND)

	// #define MR_C6       CAP_N(3.9)        /* verified                           */

	SN74LS629(1J_A, CAP_N(3.9))
	NET_C(1J_A.RNG, V5)
	NET_C(1J_A.ENQ, ttllow)
	NET_C(GND, 1J_A.GND)

	// #define MR_C17      CAP_N(22)        /* verified                            */

	SN74LS629(2J_A, CAP_N(22))
	NET_C(2J_A.RNG, V5)
	NET_C(2J_A.ENQ, ttllow)
	NET_C(GND, 2J_A.GND)

	TTL_7486_XOR(1K_A, 1J_A.Y, 2J_A.Y)
	TTL_7408_AND(2K_A, 2H_A.Q, 1K_A)
NETLIST_END()

/* ---------------------------------------------------------------------- */
/* luigi sound                                                            */
/* ---------------------------------------------------------------------- */
static NETLIST_START(nl_mario_snd1)

	RES(R18, RES_K(30))               /* 20 according to parts list           */
										/* 27 verified, 30K in schematics       */
	CAP(C15, CAP_U(4.7))              /* verified                             */

	TTL_74123(2H_B)
	NET_C(2H_B.VCC, V5)
	NET_C(2H_B.GND, GND)
	NET_C(SOUND1.Q, 2H_B.B)
	NET_C(GND, 2H_B.A)
	NET_C(2H_B.CLRQ, ttlhigh)         /* NOT IN SCHEMATICS                    */
	DIODE(D8, "1N4148")               /* FIXME: try to identify */
	TTL_7404_INVERT(1H_B, 2H_B.QQ)
	NET_C(R18.1, V5)
	NET_C(R18.2, D8.A, C15.1)
	NET_C(D8.K, 2H_B.RC)
	NET_C(C15.2, 2H_B.C)

	RES(R7, RES_K(4.7))               /* verified                             */
	CAP(C4, CAP_U(4.7))                /* verified                             */

	NET_C(1H_B.Q, R7.1)
	NET_C(R7.2, C4.1, 1J_B.FC)
	NET_C(R7.2, 2J_B.FC)
	NET_C(C4.2, GND)

	SN74LS629(1J_B, CAP_N(39))      /* C5 */
	NET_C(1J_B.RNG, V5)
	NET_C(1J_B.ENQ, ttllow)
	NET_C(GND, 1J_B.GND)

	SN74LS629(2J_B, CAP_N(6.8))     /* C16 */
	NET_C(2J_B.RNG, V5)
	NET_C(2J_B.ENQ, ttllow)
	NET_C(GND, 2J_B.GND)

	TTL_7486_XOR(1K_B, 1J_B.Y, 2J_B.Y)
	TTL_7408_AND(2K_B, 2H_B.Q, 1K_B)
NETLIST_END()

/* ---------------------------------------------------------------------- */
/* skid sound                                                            */
/* ---------------------------------------------------------------------- */

// FIXME: Diodes are 1S953
static NETLIST_START(nl_mario_snd7)

	RES(R61, RES_K(47))
	CAP(C41, CAP_U(4.7))              /* verified                             */

	TTL_74123(4L_A)
	NET_C(4L_A.VCC, V5)
	NET_C(4L_A.GND, GND)
	NET_C(SOUND7.Q, 4L_A.B)
	NET_C(GND, 4L_A.A)
	NET_C(4L_A.CLRQ, ttlhigh)         /* NOT IN SCHEMATICS                    */
	DIODE(D10, "1N4148")               /* FIXME: try to identify */
	TTL_7404_INVERT(4J_A, 4L_A.Q)
	NET_C(R61.1, V5)
	NET_C(R61.2, D10.A, C41.1)
	NET_C(D10.K, 4L_A.RC)
	NET_C(C41.2, 4L_A.C)

	RES(R65, RES_K(10))
	CAP(C44, CAP_U(3.3))              /* verified                             */

	SN74LS629(4K_A, CAP_U(0.022))
	NET_C(4K_A.RNG, V5)
	NET_C(4K_A.ENQ, ttllow)
	NET_C(GND, 4K_A.GND)
	NET_C(R65.1, 4J_A.Q)
	NET_C(R65.2, 4K_A.FC, C44.1)
	NET_C(C44.2, GND)

	CD4020_WI(3H, 4K_B.Y, ttllow, V5, GND)
	TTL_7404_INVERT(4J_B, 3H.Q12)

	RES(R64, RES_K(20))
	CAP(C43, CAP_U(3.3))              /* verified                             */

	SN74LS629(4K_B, CAP_U(0.0047))
	NET_C(4K_B.RNG, V5)
	NET_C(4K_B.ENQ, ttllow)
	NET_C(GND, 4K_B.GND)
	NET_C(R64.1, 4J_B.Q)
	NET_C(R64.2, 4K_B.FC, C43.1)
	NET_C(C43.2, GND)

	TTL_7486_XOR(1K_C, 3H.Q4, 4K_A.Y)
	TTL_7408_AND(2K_C, 4L_A.Q, 1K_C)

NETLIST_END()

/* ---------------------------------------------------------------------- */
/* DAC sound                                                            */
/* ---------------------------------------------------------------------- */
static NETLIST_START(nl_mario_dac)
	RES(R34, RES_M(2))
	RES(R35, RES_M(1))
	RES(R36, RES_M(1.8))
	LM3900(3M_1)
	NET_C(3M_1.GND, GND)
	NET_C(3M_1.VCC, V5)

	NET_C(DAC.VOUT, R34.1)
	NET_C(3M_1.MINUS, R34.2, R35.2)
	NET_C(3M_1.OUT, R35.1)
	NET_C(3M_1.PLUS, R36.1)
	NET_C(R36.2, V5)

	RES(R21, RES_M(1.8))
	RES(R23, RES_K(10))
	RES(R25, RES_K(10))
	RES(R37, RES_K(750))
	RES(R38, RES_K(360))
	RES(R39, RES_K(750))

	CAP(C18, CAP_P(100))
	CAP(C19, CAP_U(10))
	CAP(C20, CAP_U(1))
	CAP(C30, CAP_P(100))

	LM3900(3M_2)
	NET_C(3M_2.GND, GND)
	NET_C(3M_2.VCC, V5)

	NET_C(R35.1, C20.1)
	NET_C(C20.2, R37.1)
	NET_C(R37.2, R38.2, C18.1, R39.2)

	NET_C(C18.2, GND)
	NET_C(R38.1, C30.2, 3M_2.MINUS)
	NET_C(3M_2.OUT, R39.1, C30.1)

	NET_C(R21.1, 3M_2.PLUS)
	NET_C(R21.2, C19.1, R25.2, R23.1)
	NET_C(C19.2, R23.2, GND)
	NET_C(R25.1, V5)
NETLIST_END()

NETLIST_START(mario)

	LOCAL_SOURCE(nl_mario_snd0)
	LOCAL_SOURCE(nl_mario_snd1)
	LOCAL_SOURCE(nl_mario_snd7)
	LOCAL_SOURCE(nl_mario_dac)

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.SOR_FACTOR, 1.0)
	PARAM(Solver.GS_LOOPS, 1)
	/* Dynamic timestepping avoids excessive newton loops on startup */
	PARAM(Solver.DYNAMIC_LTE, 5e-2)
	PARAM(Solver.DYNAMIC_TS,  0)

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5) // no-ttl-dip devices need VCC!

	TTL_INPUT(SOUND0, 1)
	INCLUDE(nl_mario_snd0)

	TTL_INPUT(SOUND1, 1)
	INCLUDE(nl_mario_snd1)

	TTL_INPUT(SOUND7, 1)
	INCLUDE(nl_mario_snd7)

	R2R_DAC(DAC, 3.4, 10000.0, 8)
	NET_C(DAC.VGND, GND)

	INCLUDE(nl_mario_dac)

	/* ---------------------------------------------------------------------- */
	/* mixing                                                                 */
	/* ---------------------------------------------------------------------- */

	RES(R20, RES_K(22))               /* verified                             */
	RES(R19, RES_K(22))               /* verified                             */
	RES(R40, RES_K(22))               /* verified                             */
	RES(R41, RES_K(100))              /* verified                             */
	CAP(C31, CAP_U(0.022))            /*                             */

	NET_C(2K_A.Q, R20.1)
	NET_C(2K_B.Q, R19.1)
	NET_C(2K_C.Q, R41.1)

	NET_C(R39.1, R40.1)

	NET_C(R20.2, R19.2, R40.2, R41.2, C31.1)
	NET_C(C31.2, GND)

	CAP(C32, CAP_U(1))                /* verified                             */
	RES(R42, RES_K(43))               /* verified                             */
	RES(R43, RES_K(100))              /* verified                             */

	NET_C(C31.1, C32.1)
	NET_C(C32.2, R42.1, R43.2, Q10.B)
	//NET_C(C32.2, R42.1, R43.2)
	NET_C(R43.1, V5)
	NET_C(R42.2, GND)
#if 1
	RES(R63, RES_K(1))                /*                                      */
	RES(R62, 150)                     /*                                      */

	QBJT_EB(Q10, "2SC1815")

	NET_C(R62.2, GND)
	NET_C(R62.1, Q10.E)

	NET_C(R63.1, V5)
	NET_C(R63.2, Q10.C)

	CAP(C42, CAP_U(0.1))
	CAP(C47, CAP_U(4.7))
	RES(VR1, RES_K(10))

	NET_C(C42.1, C47.1, R62.1)
	NET_C(C42.2, GND)
	NET_C(C47.2, VR1.1)
	NET_C(VR1.2, GND)
#endif
	/* ---------------------------------------------------------------------- */
	/* Output                                                                 */
	/* ---------------------------------------------------------------------- */

	RES(ROUT, 1000000)

	//NET_C(Q10.C, ROUT.1)
	//NET_C(R43.2, ROUT.1)
	NET_C(VR1.1, ROUT.1)

	NET_C(GND, ROUT.2)

	OPTIMIZE_FRONTIER(R40.1, RES_K(22), 50)
NETLIST_END()
