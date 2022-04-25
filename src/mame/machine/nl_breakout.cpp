// license:GPL-2.0+
// copyright-holders:DICE Team,Couriersud

/*
 * Changelog:
 *
 *      - Drop dice syntax (Couriersud)
 *      - Fix brick display (Couriersud)
 *      - Added led and lamp components (Couriersud)
 *      - Start2 works (Couriersud)
 *      - Added discrete paddle potentiometers (Couriersud)
 *      - Changes made to run in MAME (Couriersud)
 *      - Added bonus game dip switch (Couriersud)
 *      - Added discrete startup latch
 *      - Original version imported from DICE
 *
 * TODO:
 *      - lamp triacs?
 *
 * The MAME team has asked for and received written confirmation from the
 * author of DICE to use, modify and redistribute code under:
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   - DICE itself is licensed under version 3 of the GNU General Public License.
 *     Under no circumstances the exemptions listed above shall apply to any
 *     other code of DICE not contained in this file.
 *
 * The following is an extract from the DICE readme.
 *
 * ----------------------------------------------------------------------------
 *
 * DICE is a Discrete Integrated Circuit Emulator. It emulates computer systems
 * that lack any type of CPU, consisting only of discrete logic components.
 *
 * Project Page: http://sourceforge.net/projects/dice/
 * Email: dice.emulator@gmail.com
 *
 * License
 *
 * Copyright (C) 2008-2013 DICE Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

/*
 * Notes
 * FIXME: breakout generates some spurious hsync in the middle of line 27
 *
 */

#include "netlist/devices/net_lib.h"

#define SLOW_BUT_ACCURATE (0)

NETLIST_START(breakout)

#if (SLOW_BUT_ACCURATE)
	SOLVER(Solver, 16000)
	PARAM(Solver.RELTOL, 5e-4) // less accuracy and diode will not work
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e0)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-7)
	PARAM(Solver.METHOD, "MAT_CR")
#else
	SOLVER(Solver, 16000)
	PARAM(Solver.ACCURACY, 1e-4)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e-2)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-8)
	PARAM(Solver.METHOD, "MAT_CR")
#endif
	PARAM(NETLIST.USE_DEACTIVATE, 1)

	//----------------------------------------------------------------
	// Dip-Switch - Free game
	//----------------------------------------------------------------
	SWITCH(S1_1)
	SWITCH(S1_2)
	SWITCH(S1_3)
	SWITCH(S1_4)

	SWITCH2(COIN1) // Coin 1
	SWITCH2(COIN2) // Coin 2

	SWITCH(START1) // Start 1
	SWITCH(START2) // Start 2

	SWITCH(SERVE) // Start 2

	SWITCH(S2) // Cocktail
	SWITCH(S3) // 2 Plays / 25c
	SWITCH2(S4) // Three Balls / 5 Balls

	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5)

	ALIAS(GNDD ,GND.Q)
	ALIAS(P ,V5.Q)

	TTL_INPUT(ttlhigh, 1)
	TTL_INPUT(ttllow, 0)
	TTL_INPUT(antenna, 0)

	NET_C(VCC, ttlhigh.VCC, ttllow.VCC, antenna.VCC)
	NET_C(GND, ttlhigh.GND, ttllow.GND, antenna.GND)

	//----------------------------------------------------------------
	// Clock circuit
	//----------------------------------------------------------------

#if (SLOW_BUT_ACCURATE)
	MAINCLOCK(Y1, 14318000.0)
	TTL_9316_DIP(F1)
	NET_C(Y1.Q, F1.2)

	NET_C(F1.14, H1.13)
	NET_C(F1.13, H1.12)
	NET_C(F1.15, E1.5)
	NET_C(P, F1.1)
	NET_C(P, F1.7)
	NET_C(P, F1.10)
	NET_C(GNDD, F1.3)
	NET_C(P, F1.4)
	NET_C(GNDD, F1.5)
	NET_C(GNDD, F1.6)
	NET_C(E1.6, F1.9)

	ALIAS(CKBH, F1.13)
	ALIAS(CLOCK, H1.11)

#else
	/*
	 *  9316    2   3   4   5   6   7   8   9   10  11  12  13  14  15  2   3   4   5   6
	 *  A       0   1   0   1   0   1   0   1   0   1   0   1   0   1
	 *  B       1   1   0   0   1   1   0   0   1   1   0   0   1   1
	 * CKBH     1   1   0   0   1   1   0   0   1   1   0   0   1   1
	 *                          ^--- Pattern Start
	 * CLOCK    1   0   1   1   1   0   1   1   1   0   1   1   1   0
	 *                                  ^--- Pattern Start
	 *                          <------> 2 Clocks Offset
	 */
	//EXTCLOCK(Y2, 14318000.0, "2,6,2,6,2,2,2,6")
	EXTCLOCK(Y2, 14318000.0, "6,2,6,2,2,2,6,2")
	EXTCLOCK(Y1, 14318000.0, "4,4,4,4,8,4")
	PARAM(Y2.OFFSET, 1.047632350887E-07) // 1.5 clocks / 14318000.0
	ALIAS(CKBH, Y1.Q)
	ALIAS(CLOCK, Y2.Q)

	NET_C(ttlhigh, H1.13)
	NET_C(ttlhigh, H1.12)
	NET_C(ttlhigh, E1.5)
#endif

	//----------------------------------------------------------------
	// Startup / Antenna latch
	//----------------------------------------------------------------

	DIODE(CR3, "1N914")
	DIODE(CR4, "1N914")
	DIODE(CR5, "1N914")
	DIODE(CR7, "1N914")

	// No need to model capacitance
	QBJT_EB(Q1, "2N3644(CJC=0 CJE=0)")
	QBJT_EB(Q2, "2N3643(CJC=0 CJE=0)")
	QBJT_EB(Q3, "2N3643(CJC=0 CJE=0)")

	CAP(C19, CAP_U(0.1))
	CAP(C16, CAP_U(0.1))

	RES(R25, 100)
	RES(R26, 330)
	RES(R27, 100)
	RES(R31, 220)
	RES(R32, 100)

	NET_C(GND, CR5.A, Q2.E, C16.2, R25.2, Q3.E)
	NET_C(CR5.K, Q2.B, antenna)
	NET_C(Q2.C, C16.1, R25.1, Q3.B, R27.2)
	NET_C(R27.1, CR7.A, R31.2) //CR7.K == IN
	NET_C(R31.1, Q1.C)
	NET_C(Q3.C, R26.2, CR3.A, CR4.A, E9.5) // E9.6 = Q Q3.C=QQ CR3.K = COIN*1 CR4.K = COIN*2
	NET_C(R26.1, Q1.B, C19.2, R32.2)
	NET_C(Q1.E, C19.1, R32.1, V5)

	ALIAS(LAT_Q ,E9.6)
	ALIAS(Q_n ,Q3.C)
	ALIAS(COIN1_n ,F8.5)
	ALIAS(COIN2_n ,H9.5)

	NET_C(CR7.K, D8.11) //set
	NET_C(CR3.K, COIN1_n) //reset
	NET_C(CR4.K, COIN2_n) //reset


	//static CapacitorDesc c32_desc(CAP_U(0.1));
	//NETDEV_DELAY(C32) NETDEV_PARAMI(C32, L_TO_H, CAPACITOR_tc_lh((&c32_desc)->c, (&c32_desc)->r)) NETDEV_PARAMI(C32, H_TO_L, CAPACITOR_tc_hl((&c32_desc)->c, (&c32_desc)->r))

	CAP(C32, CAP_U(0.1))
	NET_C(GND, C32.2)

#if (SLOW_BUT_ACCURATE)
	CAP(C36, CAP_N(1.0)) //0.001uF = 1nF - determines horizontal gap between bricks
	NET_C(GND, C36.2)
#else
	NETDEV_DELAY(C36)
	PARAM(C36.L_TO_H, 93)
	PARAM(C36.H_TO_L, 2)
#endif

	CAP(C37, CAP_P(330))
	NET_C(GND, C37.2)

	TTL_7474_DIP(A3)
	TTL_7408_DIP(A4)
	TTL_7400_DIP(A5)
	TTL_7474_DIP(A6)


	TTL_9602_DIP(A7)
	NET_C(VCC, A7.16)
	NET_C(GND, A7.8)
	RES(R45, RES_K(68))
	CAP(CC24, CAP_U(1))
	RES(R46, RES_K(22))
	NET_C(A7.1, CC24.1)
	NET_C(A7.2, CC24.2)
	NET_C(A7.2, R45.2)
	NET_C(VCC, R45.1)
	CAP(CC25, CAP_U(10))
	NET_C(A7.15, CC25.1)
	NET_C(A7.14, CC25.2)
	NET_C(A7.14, R46.2)
	NET_C(VCC, R46.1)

	TTL_9602_DIP(A8)
	NET_C(VCC, A8.16)
	NET_C(GND, A8.8)
	RES(R47, RES_K(27))
	CAP(CC26, CAP_U(1))
	RES(R48, RES_K(27))
	NET_C(A8.1, CC26.1)
	NET_C(A8.2, CC26.2)
	NET_C(A8.2, R47.2)
	NET_C(VCC, R47.1)
	CAP(CC27, CAP_U(1))
	NET_C(A8.15, CC27.1)
	NET_C(A8.14, CC27.2)
	NET_C(A8.14, R48.2)
	NET_C(VCC, R48.1)

	NE555_DIP(B2)
	RES(R55, 560)
	RES(R54, RES_M(1.8))
	CAP(C34, CAP_U(0.1))
	NET_C(B2.7, R55.1)
	NET_C(B2.7, R54.1)
	NET_C(B2.6, R54.2)
	NET_C(B2.6, C34.1)
	NET_C(B2.2, C34.1)
	NET_C(R55.2, V5)
	NET_C(C34.2, GND)
	NET_C(B2.8, V5)
	NET_C(B2.1, GND)
	TTL_7402_DIP(B3)
	TTL_9316_DIP(B4)
	TTL_74193_DIP(B5)
	TTL_7400_DIP(B6)
	TTL_9316_DIP(B7)
	TTL_9316_DIP(B8)
	TTL_7408_DIP(B9)

	TTL_7400_DIP(C2)
	TTL_7400_DIP(C3)
	TTL_7486_DIP(C4)
	TTL_7404_DIP(C5)
	TTL_7486_DIP(C6)
	TTL_9316_DIP(C7)
	TTL_9316_DIP(C8)

	NE555_DIP(C9)
	TTL_7432_DIP(D2)
	TTL_7474_DIP(D3)
	TTL_9316_DIP(D4)
	TTL_7474_DIP(D5)
	TTL_7408_DIP(D6)
	TTL_7411_DIP(D7)
	TTL_7400_DIP(D8)
	//  CHIP("D9", 4016)    //quad bilateral switch

	TTL_7404_DIP(E1)
	TTL_7486_DIP(E2)
	TTL_7402_DIP(E3)
	TTL_7432_DIP(E4)
	TTL_7408_DIP(E5)
	TTL_7474_DIP(E6)
	TTL_7402_DIP(E7)
	TTL_7474_DIP(E8)
	TTL_7404_DIP(E9)

	TTL_7411_DIP(F2)

	TTL_9602_DIP(F3)
	NET_C(VCC, F3.16)
	NET_C(GND, F3.8)
	RES(R22, RES_K(47))
	CAP(CC14, CAP_U(1))
	RES(R23, RES_K(47))
	NET_C(F3.1, CC14.1)
	NET_C(F3.2, CC14.2)
	NET_C(F3.2, R22.2)
	NET_C(VCC, R22.1)
	CAP(CC15, CAP_U(1))
	NET_C(F3.15, CC15.1)
	NET_C(F3.14, CC15.2)
	NET_C(F3.14, R23.2)
	NET_C(VCC, R23.1)

	TTL_7474_DIP(F4)
	TTL_7474_DIP(F5)
	TTL_74193_DIP(F6)
	TTL_74279_DIP(F7)
	TTL_7474_DIP(F8)
	TTL_7404_DIP(F9)

	TTL_7437_DIP(H1)
	TTL_7408_DIP(H2)
	TTL_7427_DIP(H3)
	TTL_7400_DIP(H4)
	TTL_9312_DIP(H5)
	TTL_9310_DIP(H6)
	TTL_7408_DIP(H7)

	TTL_7474_DIP(H8)
	TTL_7474_DIP(H9)

	TTL_74175_DIP(J1)
	TTL_7404_DIP(J2)
	TTL_7402_DIP(J3)
	TTL_9312_DIP(J4)
	TTL_7448_DIP(J5)
	TTL_9310_DIP(J6)
	TTL_7420_DIP(J7)
	TTL_74279_DIP(J8)
	TTL_7410_DIP(J9)

	TTL_9316_DIP(K1)
	TTL_7486_DIP(K2)
	TTL_7430_DIP(K3)
	TTL_7408_DIP(K4)
	TTL_9312_DIP(K5)
	TTL_9310_DIP(K6)
	TTL_7486_DIP(K7)
	TTL_7474_DIP(K8)
	TTL_74107_DIP(K9)

	TTL_9316_DIP(L1)
	TTL_7486_DIP(L2)
	TTL_82S16_DIP(L3) // Ram chip
	TTL_7411_DIP(L4)
	TTL_9312_DIP(L5)
	TTL_9310_DIP(L6)
	TTL_7486_DIP(L7)
	TTL_74193_DIP(L8)
	TTL_7400_DIP(L9)

	TTL_9316_DIP(M1)
	TTL_7483_DIP(M2)
	TTL_7486_DIP(M3)
	TTL_7410_DIP(M4)
	TTL_9312_DIP(M5)
	TTL_9310_DIP(M6)
	TTL_7427_DIP(M8)
	TTL_7404_DIP(M9)

	TTL_9316_DIP(N1)
	TTL_7483_DIP(N2)
	TTL_7486_DIP(N3)
	TTL_7411_DIP(N4)
	TTL_9312_DIP(N5)
	TTL_9310_DIP(N6)
	TTL_7408_DIP(N7)

	TTL_9602_DIP(N8)
	NET_C(VCC, N8.16)
	NET_C(GND, N8.8)
	RES(R2, RES_K(33))
	CAP(CC2, CAP_U(100))
	RES(R3, RES_K(5.6))
	NET_C(N8.1, CC2.1)
	NET_C(N8.2, CC2.2)
	NET_C(N8.2, R2.2)
	NET_C(VCC,  R2.1)
	NET_C(N8.14, R3.2)
	NET_C(VCC, R3.1)
	TTL_74192_DIP(N9)

	//LM380         //speaker amplifier - not emulated
	//LM323         //regulator - not emulated

	//----------------------------------------------------------------
	// HSYNC and VSYNC
	//----------------------------------------------------------------

	ALIAS(H1_d ,L1.14)
	ALIAS(H2_d ,L1.13)
	ALIAS(H4_d ,L1.12)
	ALIAS(H8_d ,L1.11)
	ALIAS(H8_n ,J2.2)
	ALIAS(H16_d ,K1.14)
	ALIAS(H16_n ,J2.6)
	ALIAS(H32_d ,K1.13)
	ALIAS(H32_n ,J2.4)
	ALIAS(H64_d ,K1.12)
	ALIAS(H128_d ,K1.11)

	ALIAS(V1_d ,M1.14)
	ALIAS(V2_d ,M1.13)
	ALIAS(V4_d ,M1.12)
	ALIAS(V8_d ,M1.11)
	ALIAS(V16_d ,N1.14)
	ALIAS(V16_n ,J2.10)
	ALIAS(V32_d ,N1.13)
	ALIAS(V64_d ,N1.12)
	ALIAS(V64I ,H7.11)
	ALIAS(V64_n ,M9.10)
	ALIAS(V128_d ,N1.11)

	ALIAS(H1 ,L2.8)
	ALIAS(H2 ,L2.11)
	ALIAS(H4 ,L2.3)
	ALIAS(H8 ,L2.6)
	ALIAS(H16 ,K2.8)
	ALIAS(H32 ,K2.11)
	ALIAS(H64 ,K2.3)
	ALIAS(H128 ,K2.6)

	ALIAS(V2 ,M3.3)
	ALIAS(V4 ,M3.6)
	ALIAS(V8 ,M3.11)
	ALIAS(V16 ,N3.8)
	ALIAS(V32 ,N3.3)
	ALIAS(V64 ,N3.6)
	ALIAS(V128 ,N3.11)

	ALIAS(HSYNC ,J1.2)
	ALIAS(HSYNC_n ,J1.3)
	ALIAS(VSYNC ,J1.7)
	ALIAS(VSYNC_n ,J1.6)
	ALIAS(PSYNC ,J1.11)
	ALIAS(PSYNC_n ,J1.10)
	ALIAS(BSYNC ,J1.15)
	ALIAS(BSYNC_n ,J1.14)

	ALIAS(BALL ,D7.6)
	ALIAS(BALL_DISPLAY ,A4.6)
	ALIAS(PLAYFIELD ,H4.3)
	ALIAS(SCORE ,D3.5)
	ALIAS(VERT_TRIG_n ,H1.8)

	ALIAS(SCLOCK ,K1.15)

	ALIAS(PAD_n ,K3.8)
	ALIAS(PAD_EN_n ,C2.8)

	ALIAS(COIN ,L9.6)
	ALIAS(CREDIT_1_OR_2 ,L9.3)
	ALIAS(CREDIT_1_OR_2_n ,F9.8)
	ALIAS(CREDIT2 ,F9.6)
	ALIAS(CREDIT2_n ,M8.8)
	ALIAS(CR_START1 ,E8.5)
	ALIAS(CR_START1_n ,E8.6) //Schematic shows E8.6 as positive CR_START1, but this can't be right?
	ALIAS(CR_START2 ,E8.9)
	ALIAS(CR_START2_n ,E8.8)
	ALIAS(CSW1 ,F9.12)
	ALIAS(CSW2 ,F9.2)

	ALIAS(P2_CONDITIONAL ,H1.3)
	ALIAS(P2_CONDITIONAL_dash ,H7.8)
	ALIAS(PLAYER_2 ,B4.14)
	ALIAS(PLAYER_2_n ,M9.8)

	ALIAS(START_GAME ,D8.6)
	ALIAS(START_GAME1_n ,M9.4)
	ALIAS(START_GAME_n ,M9.6)

	ALIAS(BG1_n ,K8.9)
	ALIAS(BG1 ,K8.8)
	ALIAS(BG2_n ,K8.5)
	ALIAS(BG2 ,K8.6)

	ALIAS(FREE_GAME_TONE ,N7.3)
	ALIAS(BONUS_COIN ,L9.11)

	ALIAS(SBD_n ,D2.11)

	ALIAS(PLAY_CP ,D2.8)
	ALIAS(PLGM2_n ,F7.7)
	ALIAS(VB_HIT_n ,A5.6)

	ALIAS(SERVE_n ,SERVE.1)
	ALIAS(SERVE_WAIT ,A3.9)
	ALIAS(SERVE_WAIT_n ,A3.8)

	ALIAS(BRICK_DISPLAY ,E3.1)
	ALIAS(BRICK_HIT ,E6.5)
	ALIAS(BRICK_HIT_n ,E6.6)

	//ALIAS(EGL       ,A4.3)
	ALIAS(EGL ,C37.1)
	ALIAS(EGL_n ,C5.2)

	ALIAS(RAM_PLAYER1 ,E7.4)
	ALIAS(A1 ,H6.14)
	ALIAS(B1 ,H6.13)
	ALIAS(C1 ,H6.12)
	ALIAS(D1 ,H6.11)
	ALIAS(E1 ,J6.14)
	ALIAS(F1 ,J6.13)
	ALIAS(G1 ,J6.12)
	ALIAS(H01 ,J6.11)
	ALIAS(I1 ,K6.14)
	ALIAS(J1 ,K6.13)
	ALIAS(K1 ,K6.12)
	ALIAS(L1 ,K6.11)
	ALIAS(A2 ,N6.14)
	ALIAS(B2 ,N6.13)
	ALIAS(C2 ,N6.12)
	ALIAS(D2 ,N6.11)
	ALIAS(E2s ,M6.14)
	ALIAS(F2 ,M6.13)
	ALIAS(G2 ,M6.12)
	ALIAS(H02 ,M6.11)
	ALIAS(I2 ,L6.14)
	ALIAS(J2 ,L6.13)
	ALIAS(K2 ,L6.12)
	ALIAS(L2 ,L6.11)

	ALIAS(CX0 ,C6.11)
	ALIAS(CX1 ,C6.6)
	ALIAS(X0 ,C5.10)
	ALIAS(X1 ,B6.3)
	ALIAS(X2 ,C6.3)
	ALIAS(Y0 ,B6.11)
	ALIAS(Y1 ,B6.6)
	ALIAS(Y2 ,A6.6)
	ALIAS(DN ,C4.3)
	ALIAS(PC ,D4.12)
	ALIAS(PD ,D4.11)
	ALIAS(SU_n ,D5.8)
	ALIAS(V_SLOW ,C5.8)

	ALIAS(PLNR ,E3.4)
	ALIAS(SCI_n ,H4.6)
	ALIAS(SFL_n ,E9.12) // Score Flash
	ALIAS(TOP_n ,E9.2)

	ALIAS(BP_HIT_n ,A5.8)
	ALIAS(BTB_HIT_n ,C3.3)

	ALIAS(SET_BRICKS ,D3.9)
	ALIAS(SET_BRICKS_n ,D3.8)

	ALIAS(BALL_A ,B4.13)
	ALIAS(BALL_B ,B4.12)
	ALIAS(BALL_C ,B4.11)

	ALIAS(FPD1 ,F3.10)
	ALIAS(FPD1_n ,F3.9)
	ALIAS(FPD2 ,F3.6)
	ALIAS(FPD2_n ,F3.7)

	ALIAS(COUNT ,N7.11)
	ALIAS(COUNT_1 ,N7.8)
	ALIAS(COUNT_2 ,N7.6)

	ALIAS(ATTRACT ,E6.8)
	ALIAS(ATTRACT_n ,E6.9)

	ALIAS(BRICK_SOUND ,B8.14)
	ALIAS(P_HIT_SOUND ,B7.12)
	ALIAS(VB_HIT_SOUND ,B7.11)

	ALIAS(LH_SIDE ,J3.13)
	ALIAS(RH_SIDE ,H2.3)
	ALIAS(TOP_BOUND ,K4.6)

	//----------------------------------------------------------------
	// Audio circuit
	//----------------------------------------------------------------

	NET_C(M9.2, F6.5)
	NET_C(M9.2, F7.15)
	NET_C(F6.13, F7.14)
	NET_C(START_GAME_n, F6.11)
	NET_C(P, F6.15)
	NET_C(P, F6.1)
	NET_C(P, F6.10)
	NET_C(P, F6.9)
	NET_C(GNDD, F6.14)
	NET_C(F7.13, J9.2)
	NET_C(VSYNC, J9.1)
	NET_C(A7.9, J9.13)
	NET_C(J9.12, F6.4)
	NET_C(J9.12, A7.11)
	NET_C(GNDD, A7.12)
	NET_C(ATTRACT_n, A7.13)
	NET_C(J9.12, A8.11)
	NET_C(GNDD, A8.12)
	NET_C(ATTRACT_n, A8.13)
	NET_C(VB_HIT_n, A7.5)
	NET_C(GNDD, A7.4)
	NET_C(ATTRACT_n, A7.3)
	NET_C(BP_HIT_n, A8.5)
	NET_C(GNDD, A8.4)
	NET_C(ATTRACT_n, A8.3)
	NET_C(A8.6, B9.13)
	NET_C(P_HIT_SOUND, B9.12)
	NET_C(A8.10, B9.10)
	NET_C(BRICK_SOUND, B9.9)
	NET_C(A7.6, B9.4)
	NET_C(VB_HIT_SOUND, B9.5)

	NET_C(S1_1.1, GND)
	NET_C(S1_2.1, GND)
	NET_C(S1_3.1, GND)
	NET_C(S1_4.1, GND)

	RES(R7, RES_K(2.7))
	RES(R8, RES_K(2.7))
	RES(R9, RES_K(2.7))
	RES(R10, RES_K(2.7))

	NET_C(R7.1, V5)
	NET_C(R8.1, V5)
	NET_C(R9.1, V5)
	NET_C(R10.1, V5)

	NET_C(S1_1.2, R7.2)
	NET_C(S1_2.2, R8.2)
	NET_C(S1_3.2, R9.2)
	NET_C(S1_4.2, R10.2)

	//----------------------------------------------------------------
	// Free Game
	//----------------------------------------------------------------

	NET_C(I1, K7.2)
	NET_C(S1_1.2, K7.1)
	NET_C(J1, K7.12)

	NET_C(S1_2.2, K7.13)
	NET_C(K1, K7.5)

	NET_C(S1_3.2, K7.4)
	NET_C(L1, K7.9)

	NET_C(S1_4.2, K7.10)

	NET_C(I2, L7.2)
	NET_C(S1_1.2, L7.1)
	NET_C(J2, L7.12)
	NET_C(S1_2.2, L7.13)
	NET_C(K2, L7.5)
	NET_C(S1_3.2, L7.4)
	NET_C(L2, L7.9)
	NET_C(S1_4.2, L7.10)

	NET_C(K7.3, J7.13)
	NET_C(K7.11, J7.12)
	NET_C(K7.6, J7.10)
	NET_C(K7.8, J7.9)

	NET_C(L7.3, J7.1)
	NET_C(L7.11, J7.2)
	NET_C(L7.6, J7.4)
	NET_C(L7.8, J7.5)

	NET_C(START_GAME1_n, J8.12)
	NET_C(BG1_n, J8.11)
	NET_C(J7.8, J8.10)

	NET_C(START_GAME1_n, J8.2)
	NET_C(BG2_n, J8.3)
	NET_C(J7.6, J8.1)

	NET_C(J8.9, K8.12)
	NET_C(EGL, K8.11)
	NET_C(P, K8.13)
	NET_C(LAT_Q, K8.10)

	NET_C(J8.4, K8.2)
	NET_C(EGL, K8.3)
	NET_C(P, K8.1)
	NET_C(LAT_Q, K8.4)

	NET_C(P, K9.8)
	NET_C(J8.9, K9.9)
	NET_C(GNDD, K9.11)
	NET_C(HSYNC_n, K9.10)

	NET_C(P, K9.1)
	NET_C(J8.4, K9.12)
	NET_C(GNDD, K9.4)
	NET_C(HSYNC_n, K9.13)

	NET_C(K9.6, L9.12)
	NET_C(K9.2, L9.13)

	NET_C(P, N8.5)
	NET_C(BONUS_COIN, N8.4)
	NET_C(ATTRACT_n, N8.3)

	NET_C(V4_d, N7.2)
	NET_C(N8.6, N7.1)

	NET_C(GNDD, M2.13)
	NET_C(V1_d, M2.10)
	NET_C(V2_d, M2.8)
	NET_C(V4_d, M2.3)
	NET_C(V8_d, M2.1)
	NET_C(GNDD, M2.11)
	NET_C(P2_CONDITIONAL, M2.7)
	NET_C(GNDD, M2.4)
	NET_C(GNDD, M2.16)

	NET_C(M2.14, N2.13)
	NET_C(V16_d, N2.10)
	NET_C(V32_d, N2.8)
	NET_C(V64_d, N2.3)
	NET_C(V128_d, N2.1)
	NET_C(GNDD, N2.11)
	NET_C(P2_CONDITIONAL, N2.7)
	NET_C(GNDD, N2.4)
	NET_C(GNDD, N2.16)

	NET_C(M2.6, M3.2)
	NET_C(P2_CONDITIONAL, M3.1)
	NET_C(M2.2, M3.5)
	NET_C(P2_CONDITIONAL, M3.4)
	NET_C(M2.15, M3.12)
	NET_C(P2_CONDITIONAL, M3.13)

	NET_C(P2_CONDITIONAL, N3.10)
	NET_C(N2.9, N3.9)
	NET_C(P2_CONDITIONAL, N3.1)
	NET_C(N2.6, N3.2)
	NET_C(P2_CONDITIONAL, N3.4)
	NET_C(N2.2, N3.5)
	NET_C(P2_CONDITIONAL, N3.13)
	NET_C(N2.15, N3.12)

	NET_C(H1_d, L2.9)
	NET_C(P2_CONDITIONAL, L2.10)
	NET_C(H2_d, L2.12)
	NET_C(P2_CONDITIONAL, L2.13)
	NET_C(H4_d, L2.2)
	NET_C(P2_CONDITIONAL, L2.1)
	NET_C(H8_d, L2.5)
	NET_C(P2_CONDITIONAL, L2.4)

	NET_C(P2_CONDITIONAL, K2.10)
	NET_C(H16_d, K2.9)
	NET_C(P2_CONDITIONAL, K2.13)
	NET_C(H32_d, K2.12)
	NET_C(P2_CONDITIONAL, K2.1)
	NET_C(H64_d, K2.2)
	NET_C(P2_CONDITIONAL, K2.4)
	NET_C(H128_d, K2.5)

	NET_C(V64, M9.11)
	NET_C(H16, J2.5)
	NET_C(H32, J2.3)
	NET_C(V16, J2.11)
	NET_C(H8, J2.1)

	NET_C(CLOCK, J1.9)
	NET_C(SCLOCK, J1.4)
	NET_C(N4.6, J1.5)
	NET_C(PAD_n, J1.12)
	NET_C(BALL_DISPLAY, J1.13)
	NET_C(P, J1.1)

	NET_C(P, K1.1)
	NET_C(P, K1.3)
	NET_C(P, K1.4)
	NET_C(P, K1.5)
	NET_C(P, K1.6)
	NET_C(P, K1.9)
	NET_C(P, K1.10)
	NET_C(CLOCK, K1.2)
	NET_C(L1.15, K1.7)

	NET_C(P, L1.1)
	NET_C(P, L1.3)
	NET_C(GNDD, L1.4)
	NET_C(P, L1.5)
	NET_C(GNDD, L1.6)
	NET_C(VERT_TRIG_n, L1.9)
	NET_C(P, L1.10)
	NET_C(CLOCK, L1.2)
	NET_C(P, L1.7)

	NET_C(P, N1.1)
	NET_C(P, N1.10)
	NET_C(P, N1.3)
	NET_C(P, N1.4)
	NET_C(P, N1.5)
	NET_C(P, N1.6)
	NET_C(P, N1.9)
	NET_C(CLOCK, N1.2)
	NET_C(H2.6, N1.7)

	NET_C(M1.15, H2.5)
	NET_C(L1.15, H2.4)

	NET_C(V128_d, N4.5)
	NET_C(V64_d, N4.3)
	NET_C(V32_d, N4.4)
	NET_C(N4.6, H1.10)
	NET_C(VSYNC_n, H1.9)

	NET_C(P, M1.1)
	NET_C(GNDD, M1.3)
	NET_C(GNDD, M1.4)
	NET_C(P, M1.5)
	NET_C(GNDD, M1.6)
	NET_C(VERT_TRIG_n, M1.9)
	NET_C(CLOCK, M1.2)
	NET_C(L1.15, M1.7)
	NET_C(K1.15, M1.10)

	NET_C(PLAYER_2, M9.9)
	NET_C(BALL_A, C5.5)
	NET_C(BALL_A, C4.13)
	NET_C(BALL_B, C4.12)
	NET_C(BALL_A, A4.13)
	NET_C(BALL_B, A4.12)
	NET_C(BALL_C, C4.10)
	NET_C(A4.11, C4.9)

	NET_C(A2, N5.1)
	NET_C(E2s, N5.2)
	NET_C(I2, N5.3)
	NET_C(C5.6, N5.4)
	NET_C(A1, N5.5)
	NET_C(E1, N5.6)
	NET_C(I1, N5.7)
	NET_C(PLAYER_2_n, N5.9)
	NET_C(H32_n, N5.10)
	NET_C(V16, N5.11)
	NET_C(V64, N5.12)
	NET_C(V128, N5.13)

	NET_C(B2, M5.1)
	NET_C(F2, M5.2)
	NET_C(J2, M5.3)
	NET_C(C4.11, M5.4)
	NET_C(B1, M5.5)
	NET_C(F1, M5.6)
	NET_C(J1, M5.7)
	NET_C(PLAYER_2, M5.9)
	NET_C(H32_n, M5.10)
	NET_C(V16, M5.11)
	NET_C(V64, M5.12)
	NET_C(V128, M5.13)

	NET_C(C2, L5.1)
	NET_C(G2, L5.2)
	NET_C(K2, L5.3)
	NET_C(C4.8, L5.4)
	NET_C(C1, L5.5)
	NET_C(G1, L5.6)
	NET_C(K1, L5.7)
	NET_C(GNDD, L5.9)
	NET_C(H32_n, L5.10)
	NET_C(V16, L5.11)
	NET_C(V64, L5.12)
	NET_C(V128, L5.13)

	NET_C(D2, K5.1)
	NET_C(H02, K5.2)
	NET_C(L2, K5.3)
	NET_C(GNDD, K5.4)
	NET_C(D1, K5.5)
	NET_C(H01, K5.6)
	NET_C(L1, K5.7)
	NET_C(GNDD, K5.9)
	NET_C(H32_n, K5.10)
	NET_C(V16, K5.11)
	NET_C(V64, K5.12)
	NET_C(V128, K5.13)

	NET_C(P, J5.4)
	NET_C(P, J5.3)
	NET_C(N5.15, J5.7)
	NET_C(M5.15, J5.1)
	NET_C(L5.15, J5.2)
	NET_C(K5.15, J5.6)
	NET_C(H32, J5.5)

	NET_C(J5.13, H5.1)
	NET_C(GNDD, H5.2)
	NET_C(GNDD, H5.3)
	NET_C(J5.14, H5.4)
	NET_C(GNDD, H5.5)
	NET_C(GNDD, H5.6)
	NET_C(J5.10, H5.7)
	NET_C(GNDD, H5.9)

	NET_C(V4, K4.12)
	NET_C(V8, K4.13)

	NET_C(K4.11, H5.10)
	NET_C(H2, H5.11)
	NET_C(H4, H5.12)
	NET_C(H8, H5.13)

	NET_C(H2, L4.3)
	NET_C(H4, L4.5)
	NET_C(H8, L4.4)

	NET_C(J5.12 , J4.1)
	NET_C(J5.11, J4.2)
	NET_C(GNDD, J4.3)
	NET_C(GNDD, J4.4)
	NET_C(J5.15, J4.5)
	NET_C(J5.9, J4.6)
	NET_C(GNDD, J4.7)
	NET_C(GNDD, J4.9)
	NET_C(L4.6, J4.10)
	NET_C(H8, J4.11)
	NET_C(V4, J4.12)
	NET_C(V8, J4.13)

	NET_C(H5.14, H4.13)
	NET_C(J4.14, H4.12)

	//----------------------------------------------------------------
	// Paddles
	//---------------------------------------------------------------

	NET_C(ATTRACT_n, B2.4)

	NET_C(B2.3, E9.13)
	NET_C(PLAYER_2_n, M3.9)
	NET_C(V128, M3.10)
	NET_C(H64, J3.8)
	NET_C(H128, J3.9)
	NET_C(V32, E3.5)
	NET_C(V16_n, E3.6)

	NET_C(SFL_n, M8.1)
	NET_C(M3.8, M8.2)
	NET_C(PLNR, M8.13)
	NET_C(J3.10, E9.1)
	NET_C(V64, E2.5)
	NET_C(V32, E2.4)
	NET_C(PLNR, E2.10)
	NET_C(H16, E2.9)

	NET_C(M8.12, M8.3)
	NET_C(TOP_n, M8.4)
	NET_C(TOP_n, M8.5)
	NET_C(H4.11, F2.11)
	NET_C(E2.6, F2.10)
	NET_C(E2.8, F2.9)

	NET_C(M8.6, H4.5)
	NET_C(F2.8, H4.4)

	// NOTE: Stabilizing CAP C20 not modelled.

	NET_C(PAD_EN_n, C9.4)
	NET_C(PAD_EN_n, C9.2)
	NET_C(C9.8, V5)
	NET_C(C9.1, GND)
	RES(R53, RES_K(12)) // 12k
	CAP(C21, CAP_U(1))
	NET_C(GND, C21.2, R53.2)
	NET_C(C21.1, R53.1, C9.6, C9.7)

	NET_C(BTB_HIT_n, C5.3)
	NET_C(P, F5.10)
	NET_C(P, F5.12)
	NET_C(C5.4, F5.11)
	NET_C(SERVE_WAIT_n, F5.13)
	NET_C(H64, E5.13)
	NET_C(F5.9, E5.12)
	NET_C(H128, E5.10)
	NET_C(F5.8, E5.9)
	NET_C(E5.11, E4.12)
	NET_C(E5.8, E4.13)
	NET_C(E4.11, D4.2)
	NET_C(P, D4.3)
	NET_C(P, D4.4)
	NET_C(P, D4.5)
	NET_C(P, D4.6)
	NET_C(P, D4.9)
	NET_C(P, D4.10)
	NET_C(C3.11, D4.7)
	NET_C(VSYNC_n, D4.1)

	NET_C(D4.15, E4.10)
	NET_C(H7.6, E4.9)
	NET_C(C9.3, H7.5)
	NET_C(PAD_EN_n, H7.4)
	NET_C(E4.8, C3.12)
	NET_C(ATTRACT_n, C3.13)
	NET_C(H8, J3.2)
	NET_C(H32, J3.3)

	NET_C(C3.11, K3.12)
	NET_C(H128, K3.5)
	NET_C(H64, K3.6)
	NET_C(H16, K3.11)
	NET_C(H4, K3.4)
	NET_C(J3.1, K3.1)
	NET_C(P, K3.3)
	NET_C(P, K3.2)

	NET_C(V16_d, D7.1)
	NET_C(V64_d, D7.13)
	NET_C(V128_d, D7.2)
	NET_C(D7.12, H1.4)
	NET_C(V8_d, H1.5)
	NET_C(H1.6, C2.4)
	NET_C(H1.6, C2.5)
	NET_C(V32_d, J2.9)
	NET_C(J2.8, C2.10)
	NET_C(C2.6, C2.9)

	//----------------------------------------------------------------
	// Score circuit
	//---------------------------------------------------------------

	NET_C(SCI_n, D3.4)
	NET_C(GNDD, D3.2)
	NET_C(GNDD, D3.3)
	NET_C(GNDD, D3.1)

	//----------------------------------------------------------------
	// Player 2
	//---------------------------------------------------------------

	NET_C(PLAYER_2, H7.10)
	NET_C(S2.2, GND)
	NET_C(S2.1, H7.9)
	RES(R18, RES_K(1))
	NET_C(R18.2, V5)
	NET_C(R18.1, S2.1)

	//A-L 1 and 2
	NET_C(SET_BRICKS_n, B3.2)
	NET_C(H2, B3.3)
	NET_C(B3.1, E7.6)
	NET_C(PLAYER_2, E7.5)
	NET_C(P, N6.9)
	NET_C(P, M6.9)
	NET_C(P, L6.9)
	NET_C(P, H6.9)
	NET_C(P, J6.9)
	NET_C(P, K6.9)

	NET_C(P, N6.10)
	NET_C(PLAYER_2, N6.7)
	NET_C(COUNT_2, N6.2)
	NET_C(START_GAME_n, N6.1)

	NET_C(N6.15, M6.10)
	NET_C(PLAYER_2, M6.7)
	NET_C(COUNT_2, M6.2)
	NET_C(START_GAME_n, M6.1)

	NET_C(M6.15, L6.10)
	NET_C(PLAYER_2, L6.7)
	NET_C(COUNT_2, L6.2)
	NET_C(START_GAME_n, L6.1)

	NET_C(P, H6.10)
	NET_C(RAM_PLAYER1, H6.7)
	NET_C(COUNT_1, H6.2)
	NET_C(START_GAME_n, H6.1)

	NET_C(H6.15, J6.10)
	NET_C(RAM_PLAYER1, J6.7)
	NET_C(COUNT_1, J6.2)
	NET_C(START_GAME_n, J6.1)

	NET_C(J6.15, K6.10)
	NET_C(RAM_PLAYER1, K6.7)
	NET_C(COUNT_1, K6.2)
	NET_C(START_GAME_n, K6.1)

	//CX0 and CX1
	NET_C(BRICK_HIT, H2.9)
	NET_C(H16_n, H2.10)
	NET_C(P, D5.10)
	NET_C(P, D5.12)
	NET_C(H2.8, D5.11)
	NET_C(SERVE_WAIT_n, D5.13)
	NET_C(X0, C6.13)
	NET_C(D5.9, C6.12)
	NET_C(D5.9, D6.12)
	NET_C(DN, D6.13)
	NET_C(D6.11, C6.4)
	NET_C(X1, C6.5)

	//----------------------------------------------------------------
	// COUNT1 and COUNT2
	//---------------------------------------------------------------

	NET_C(P, N8.11)
	NET_C(BRICK_HIT, N8.12)
	NET_C(ATTRACT_n, N8.13)
	NET_C(N8.9, N9.11)
	NET_C(P, N9.15)
	NET_C(P, N9.5)

	NET_C(COUNT, N9.4)
	NET_C(START_GAME, N9.14)
	NET_C(H8_n, N9.1)
	NET_C(H16_n, N9.10)
	NET_C(GNDD, N9.9)

	NET_C(N9.13, N7.13)
	NET_C(SCLOCK, N7.12)

	NET_C(PLAYER_2, N7.5)
	NET_C(COUNT, N7.4)

	NET_C(COUNT, M9.1)
	NET_C(COUNT, N7.10)
	NET_C(RAM_PLAYER1, N7.9)

	//----------------------------------------------------------------
	// DSW - Free Game and Ball Logic
	//---------------------------------------------------------------

	NET_C(P, C7.1)
	NET_C(P, C7.10)
	NET_C(P, C7.7)
	NET_C(CX0, C7.3)
	NET_C(CX1, C7.4)
	NET_C(X2, C7.5)
	NET_C(GNDD, C7.6)
	NET_C(D8.8, C7.9)
	NET_C(C7.15, C8.7)
	NET_C(C7.11, C8.10)
	NET_C(C7.12, D7.10)
	NET_C(C7.13, D7.11)
	NET_C(CLOCK, C7.2)

	NET_C(P, C8.1)
	NET_C(P, C8.3)
	NET_C(P, C8.4)
	NET_C(P, C8.5)
	NET_C(P, C8.6)
	NET_C(P, C8.9)
	NET_C(CLOCK, C8.2)
	NET_C(C8.15, B7.7)
	NET_C(C7.15, B7.10)

	NET_C(P, B7.1)
	NET_C(Y0, B7.3)
	NET_C(Y1, B7.4)
	NET_C(Y2, B7.5)
	NET_C(GNDD, B7.6)
	NET_C(D8.8, B7.9)
	NET_C(CLOCK, B7.2)
	NET_C(B7.15, B8.7)

	NET_C(VB_HIT_SOUND, D7.9)

	NET_C(P, B8.1)
	NET_C(P, B8.3)
	NET_C(P, B8.4)
	NET_C(P, B8.5)
	NET_C(P, B8.6)
	NET_C(P, B8.9)
	NET_C(C8.15, B8.10)
	NET_C(CLOCK, B8.2)

	NET_C(B8.15, D8.10)
	NET_C(B7.15, D8.9)

	NET_C(D7.8, D7.5)
	NET_C(P_HIT_SOUND, D7.4)
	NET_C(B8.15, D7.3)

	//----------------------------------------------------------------
	// RH and LH Sides
	//---------------------------------------------------------------

	NET_C(V128, N4.1)
	NET_C(V64, N4.2)
	NET_C(V16, N4.13)
	NET_C(N4.12, N4.11)
	NET_C(V8, N4.10)
	NET_C(V4, N4.9)
	NET_C(N4.8, H2.2)
	NET_C(V32, H2.1)
	NET_C(N4.8, J2.13)
	NET_C(J2.12, J3.11)
	NET_C(V32, J3.12)

	//----------------------------------------------------------------
	// Top Bound
	//---------------------------------------------------------------

	NET_C(H128, H3.4)
	NET_C(H64, H3.5)
	NET_C(H32, H3.3)
	NET_C(H3.6, L4.9)
	NET_C(H16, L4.10)
	NET_C(H8, L4.11)
	NET_C(L4.8, K4.5)
	NET_C(VSYNC_n, K4.4)

	//----------------------------------------------------------------
	// Cabinet type
	//
	// FIXME: missing?
	//---------------------------------------------------------------

	//----------------------------------------------------------------
	// Coin Circuit
	//---------------------------------------------------------------

	NET_C(COIN1.2, F9.13)
	NET_C(COIN1.1, F9.11)
	NET_C(COIN1.Q, GND)

	NET_C(CSW1, COIN1.1)
	NET_C(F9.10, COIN1.2)

	NET_C(V64, H7.12)
	NET_C(V64, H7.13)

	NET_C(CSW1, F8.10)
	NET_C(F9.10, F8.12)
	NET_C(V64I, F8.11)
	NET_C(P, F8.13)
	NET_C(P, F8.1)
	NET_C(V64I, F8.3)
	NET_C(F8.9, F8.2)
	NET_C(CSW1, F8.4)

	NET_C(F8.6, H8.12)
	NET_C(P, H8.10)
	NET_C(V16_d, H8.11)
	NET_C(P, H8.13)
	NET_C(H8.9, J8.15)
	NET_C(H8.9, J9.9)
	NET_C(V16_d, J8.14)
	NET_C(J8.13, J9.10)

	NET_C(V4_d, S3.1)
	NET_C(S3.2, J9.11)
	RES(R15, RES_K(1))
	NET_C(R15.1, V5)
	NET_C(R15.2, S3.2)

	NET_C(J9.8, L9.5)
	NET_C(J9.6, L9.4)

	NET_C(COIN2.2, F9.1)
	NET_C(COIN2.1, F9.3)
	NET_C(COIN2.Q, GND)

	NET_C(CSW2, COIN2.1)
	NET_C(CSW2, H9.10)
	NET_C(F9.4, H9.12)
	NET_C(F9.4, COIN2.2)
	NET_C(V64_n, H9.11)
	NET_C(V64_n, H9.3)
	NET_C(P, H9.13)
	NET_C(H9.9, H9.2)
	NET_C(CSW2, H9.4)
	NET_C(P, H9.1)

	NET_C(P, H8.4)
	NET_C(H9.6, H8.2)
	NET_C(V16_d, H8.3)
	NET_C(P, H8.1)
	NET_C(H8.5, J8.6)
	NET_C(V16_d, J8.5)
	NET_C(P, J9.3)
	NET_C(H8.5, J9.5)
	NET_C(J8.7, J9.4)

	NET_C(P2_CONDITIONAL_dash, E9.9)
	NET_C(E9.8, H1.1)
	NET_C(E9.8, H1.2)

	//----------------------------------------------------------------
	// Start 1 & 2 logic
	//---------------------------------------------------------------

	RES(R58, RES_K(1))
	NET_C(START2.2, GND)
	NET_C(R58.1, V5)
	NET_C(START2.1, R58.2, E9.11)

	NET_C(E9.10, E8.12)
	NET_C(P, E8.10)
	NET_C(V64I, E8.11)

	NET_C(V128_d, F7.2)
	NET_C(V128_d, F7.3)
	NET_C(CREDIT2_n, F7.1)
	NET_C(ATTRACT_n, E7.12)
	NET_C(F7.4, E7.11)
	NET_C(E7.13, E8.13)

	//Start1 Input
	RES(R57, RES_K(1))
	NET_C(START1.2, GND)
	NET_C(R57.1, V5)
	NET_C(START1.1, R57.2, E9.3)
	NET_C(E9.4, E8.2)
	NET_C(P, E8.4)
	NET_C(V64_d, E8.3)

	NET_C(CREDIT_1_OR_2_n, E7.2)
	NET_C(ATTRACT_n, E7.3)
	NET_C(E7.1, E8.1)

	NET_C(CR_START1_n, D8.4)
	NET_C(CR_START2_n, D8.5)

	NET_C(START_GAME, M9.3)
	NET_C(START_GAME, M9.5)

	NET_C(V32, D6.4)
	NET_C(ATTRACT, D6.5)
	NET_C(P, E6.10)
	NET_C(START_GAME, E6.12)
	NET_C(D6.6, E6.11)
	NET_C(D6.3, E6.13)

	NET_C(CREDIT_1_OR_2_n, D8.13)
	NET_C(EGL, D8.12)

	NET_C(LAT_Q, D6.1)
	NET_C(EGL_n, D6.2)

	//----------------------------------------------------------------
	// Serve logic
	//---------------------------------------------------------------

	RES(R30, RES_K(1))
	NET_C(SERVE.2, GND)
	NET_C(SERVE.1, R30.2)
	NET_C(R30.1, V5)

	NET_C(H64, J3.6)
	NET_C(H32, J3.5)
	NET_C(J3.4, L4.13)
	NET_C(H128, L4.2)
	NET_C(H16, L4.1)

	NET_C(BALL_DISPLAY, H2.13)
	NET_C(H128, H2.12)
	NET_C(H2.11, C3.9)
	NET_C(HSYNC, C3.10)
	NET_C(C3.8, B3.5)
	NET_C(H8_d, B3.6)
	NET_C(B3.4, C3.4)
	NET_C(H4, C3.5)
	NET_C(C3.6, A4.9)
	NET_C(START_GAME1_n, A4.10)

	NET_C(SERVE_WAIT_n, D2.13)

	NET_C(SERVE_n, D2.12)
	NET_C(SERVE_n, A3.4)
	NET_C(P, A3.2)
	NET_C(ATTRACT, A3.3)
	NET_C(SERVE_WAIT, A3.1)

	NET_C(BALL, E1.13)
	NET_C(E1.12, B3.8)
	NET_C(A3.6, B3.9)
	NET_C(B3.10, B3.11)
	NET_C(SERVE_WAIT_n, B3.12)
	NET_C(B3.13, A3.12)
	NET_C(A4.8, A3.10)
	NET_C(L4.12, A3.11)
	NET_C(P, A3.13)

	//----------------------------------------------------------------
	// Bricks logic
	//---------------------------------------------------------------

	NET_C(P, D3.10)
	NET_C(P, D3.12)
	NET_C(START_GAME, D3.11)
	NET_C(SERVE_n, D3.13)

	//----------------------------------------------------------------
	// Playfield logic
	//---------------------------------------------------------------

	NET_C(LH_SIDE, H3.1)
	NET_C(TOP_BOUND, H3.13)
	NET_C(RH_SIDE, H3.2)
	NET_C(H3.12, H4.2)
	NET_C(E1.2, C36.1)
#if (SLOW_BUT_ACCURATE)
	NET_C(C36.1, H4.1)
#else
	NET_C(C36.2, H4.1)
#endif
	NET_C(BALL_DISPLAY, A5.10)
	NET_C(PSYNC, A5.9)
	NET_C(BSYNC, C3.2)
	NET_C(TOP_BOUND, C3.1)

	NET_C(PC, C4.4)
	NET_C(PD, C4.5)
	NET_C(BP_HIT_n, C5.13)
	NET_C(PD, A5.1)
	NET_C(C5.12, A5.2)
	NET_C(BSYNC, A5.5)
	NET_C(VSYNC, A5.4)

	NET_C(C5.12, A5.13)
	NET_C(A5.3, A5.12)

	NET_C(BRICK_HIT, D5.3)
	NET_C(E5.3, D5.1)
	NET_C(D5.6, D5.2)
	NET_C(BP_HIT_n, D5.4)

	NET_C(P, A6.10)
	NET_C(C4.6, A6.12)
	NET_C(BP_HIT_n, A6.11)
	NET_C(P, A6.13)

	NET_C(A5.3, A6.4)
	NET_C(V16_d, A6.2)
	NET_C(VB_HIT_n, A6.3)
	NET_C(A5.11, A6.1)

	NET_C(P2_CONDITIONAL, C6.1)
	NET_C(D5.5, C6.2)
	NET_C(D5.6, C4.2)
	NET_C(P2_CONDITIONAL, C4.1)

	NET_C(V_SLOW, B6.12)
	NET_C(A6.8, B6.13)
	NET_C(V_SLOW, C6.10)
	NET_C(A6.5, C6.9)

	NET_C(Y0, B6.4)
	NET_C(C6.8, B6.5)

	NET_C(X2, D6.10)
	NET_C(B6.8, D6.9)
	NET_C(B5.7, B6.9)
	NET_C(B5.6, B6.10)
	NET_C(X0, B6.1)
	NET_C(X2, B6.2)

	NET_C(B5.6, C5.11)
	NET_C(B5.7, C5.9)

	NET_C(SU_n, B5.11)
	NET_C(P, B5.15)
	NET_C(P, B5.1)
	NET_C(P, B5.10)
	NET_C(P, B5.9)
	NET_C(P, B5.4)
	NET_C(D6.8, B5.5)
	NET_C(SERVE_WAIT, B5.14)

	NET_C(BTB_HIT_n, E5.1)
	NET_C(SBD_n, E5.2)

	NET_C(BP_HIT_n, E5.4)
	NET_C(BTB_HIT_n, E5.5)
	NET_C(E5.6, F7.11)
	NET_C(E5.6, F7.12)
	NET_C(BRICK_HIT_n, F7.10)
	NET_C(F7.9, C2.2)
	NET_C(BALL_DISPLAY, C2.1)
	NET_C(L3.6, E3.11)
	NET_C(C2.3, E3.12)

	NET_C(SET_BRICKS_n, E6.4)
	NET_C(E3.13, E6.2)
	NET_C(CKBH, E6.3)
	NET_C(E3.13, D2.2)
	NET_C(SET_BRICKS, D2.1)
	NET_C(D2.3, E6.1)

	NET_C(BRICK_DISPLAY, E1.1)
	NET_C(H1, K4.9)
	NET_C(H2, K4.10)
	NET_C(K4.8, E3.2)
	NET_C(L3.6, E3.3)

	NET_C(ATTRACT_n, C2.13)
	NET_C(SET_BRICKS_n, C2.12)
	NET_C(C2.11, H3.10)
	NET_C(FPD1, H3.9)
	NET_C(FPD2, H3.11)
	NET_C(H3.8, E1.3)
	NET_C(E1.4, C32.1)
	NET_C(C32.1, L3.13)
	NET_C(H4, L3.2)
	NET_C(H8, L3.1)
	NET_C(H16, L3.15)
	NET_C(V32, L3.14)
	NET_C(V64, L3.7)
	NET_C(V128, L3.9)
	NET_C(V16, L3.10)
	NET_C(RAM_PLAYER1, L3.11)
	NET_C(H32, L3.3)
	NET_C(H128, L3.4)
	NET_C(H4.8, L3.5)

	NET_C(V2, M4.5)
	NET_C(V4, M4.4)
	NET_C(V8, M4.3)
	NET_C(M4.6, H4.9)
	NET_C(VSYNC_n, K4.2)
	NET_C(H64, K4.1)
	NET_C(K4.3, H4.10)
	NET_C(FPD1_n, F2.13)
	NET_C(BRICK_HIT_n, F2.2)
	NET_C(FPD2_n, F2.1)

	NET_C(F2.12, L3.12)

	//----------------------------------------------------------------
	// Hit circuits and start logic
	//---------------------------------------------------------------

	NET_C(K2, M4.2)
	NET_C(G2, M4.13)
	NET_C(D2, M4.1)
	NET_C(K1, M4.9)
	NET_C(G1, M4.10)
	NET_C(D1, M4.11)
	NET_C(BP_HIT_n, E4.2)
	NET_C(M4.12, E4.1)
	NET_C(BP_HIT_n, E4.4)
	NET_C(M4.8, E4.5)

	NET_C(P, F4.4)
	NET_C(P, F4.2)
	NET_C(E4.3, F4.3)
	NET_C(START_GAME1_n, F4.1)

	NET_C(P, F4.10)
	NET_C(P, F4.12)
	NET_C(E4.6, F4.11)
	NET_C(START_GAME1_n, F4.13)

	NET_C(F4.6, F3.5)
	NET_C(GNDD, F3.4)
	NET_C(F4.8, F3.11)
	NET_C(GNDD, F3.12)

	NET_C(P, F3.3)
	NET_C(P, F3.13)

	//----------------------------------------------------------------
	// Credit counter logic
	//---------------------------------------------------------------

	NET_C(BONUS_COIN, E7.8)
	NET_C(COIN, E7.9)
	NET_C(CR_START1_n, H7.2)
	NET_C(V8, D8.1)
	NET_C(CR_START2, D8.2)
	NET_C(D8.3, H7.1)

	NET_C(L8.12, L8.11) // FIXME: not on schematic, on rollover load 16, keeps you from losing all credits
	NET_C(P, L8.15)
	NET_C(P, L8.1)
	NET_C(P, L8.10)
	NET_C(P, L8.9)
	NET_C(E7.10, L8.5)
	NET_C(H7.3, L8.4)

	NET_C(LAT_Q, L9.10)
	NET_C(L8.13, L9.9)
	NET_C(L9.8, L8.14)
	NET_C(L8.7, M8.9)
	NET_C(L8.6, M8.10)
	NET_C(L8.2, M8.11)
	NET_C(L8.3, M9.13)

	NET_C(CREDIT2_n, F9.5)
	NET_C(CREDIT2_n, L9.2)
	NET_C(M9.12, L9.1)
	NET_C(CREDIT_1_OR_2, F9.9)

	NET_C(CR_START1_n, F7.6)
	NET_C(CR_START2_n, F7.5)

	NET_C(PLGM2_n, F2.5)
	NET_C(PLAYER_2, F2.4)
	NET_C(H1, F2.3)

	NET_C(P, F5.4)
	NET_C(P, F5.2)
	NET_C(SERVE_WAIT, F5.3)
	NET_C(H128, F5.1)
	NET_C(F2.6, D2.9)
	NET_C(F5.5, D2.10)

	NET_C(P, B4.10)
	NET_C(P, B4.7)
	NET_C(P, B4.9)
	NET_C(PLAY_CP, B4.2)

	NET_C(EGL, C5.1)

	//----------------------------------------------------------------
	// Ball logic
	//---------------------------------------------------------------

	NET_C(START_GAME1_n, B4.1)
	NET_C(BALL_A, A4.2)
	NET_C(BALL_B, S4.1) // Three balls
	NET_C(BALL_C, S4.2) // Five balls
	NET_C(S4.Q, A4.1)
	NET_C(A4.3, C37.1)

	NET_C(SERVE_WAIT_n, A4.5)
	NET_C(BALL, A4.4)

	//----------------------------------------------------------------
	// Video output logic
	//---------------------------------------------------------------

	NET_C(V16_d, D2.4)
	NET_C(V8_d, D2.5)
	NET_C(D2.6, E3.8)
	NET_C(VSYNC_n, E3.9)
	NET_C(HSYNC_n, E2.12)
	NET_C(E3.10, E2.13)
	NET_C(PSYNC, B9.1)
	NET_C(VSYNC_n, B9.2)

	RES(R41, RES_K(3.9))
	RES(R42, RES_K(3.9))
	RES(R43, RES_K(3.9))
	RES(R51, RES_K(3.9))
	RES(R52, RES_K(3.9))

#if (SLOW_BUT_ACCURATE)
	DIODE(CR6, "1N914")
	NET_C(E2.11, CR6.K)

	NET_C(CR6.A, R41.1, R42.1, R43.1, R51.1, R52.1)
#else
	SYS_DSW(CR6, E2.11, R41.1, GND)
	PARAM(CR6.RON, 1e20)
	PARAM(CR6.ROFF, 1)
	NET_C(R41.1, R42.1, R43.1, R51.1, R52.1)
#endif

	NET_C(R51.2, PLAYFIELD)
	NET_C(R43.2, BSYNC)
	NET_C(R52.2, SCORE)

	NET_C(R41.2, B9.3)
	NET_C(R42.2, V5)

	ALIAS(videomix, R41.1)

	//----------------------------------------------------------------
	// Audio logic
	//---------------------------------------------------------------

	RES(R36, RES_K(47))
	RES(R37, RES_K(47))
	RES(R38, RES_K(47))
	RES(R39, RES_K(47))
	NET_C(R36.2, B9.11)
	NET_C(R38.2, B9.8)
	NET_C(R39.2, FREE_GAME_TONE)
	NET_C(R37.2, B9.6)
	NET_C(R36.1, R37.1, R38.1, R39.1)
	ALIAS(sound, R36.1)

	//----------------------------------------------------------------
	// Potentiometer logic
	//---------------------------------------------------------------

	POT2(POTP1, RES_K(5)) // 5k
	PARAM(POTP1.DIALLOG, 1) // Log Dial ...
	PARAM(POTP1.REVERSE, 1) // Log Dial ...
	NET_C(POTP1.1, V5)

	POT2(POTP2, RES_K(5)) // 5k
	PARAM(POTP2.DIALLOG, 1) // Log Dial ...
	PARAM(POTP2.REVERSE, 1) // Log Dial ...
	NET_C(POTP2.1, V5)

	RES(R33, 47)

	CD4016_DIP(D9)
	NET_C(D9.7, GND)
	NET_C(D9.14, V5)

	NET_C(P2_CONDITIONAL_dash, D9.6)
	NET_C(D9.12, E9.8)
	NET_C(D9.8, POTP2.2) // Connect P2 dial here
	NET_C(D9.11, POTP1.2)
	NET_C(D9.9, D9.10, R33.1)
	NET_C(R33.2, C9.6)

	//----------------------------------------------------------------
	// Serve Leds
	//----------------------------------------------------------------

	RES(R40, 150)
	RES(R21, 150)
	DIODE(LED1, "LedRed")

	/* Below is the upright cabinet configuration
	 * cocktail has two leds connected to R40.1 */
	NET_C(SERVE_WAIT_n, R21.2)
	NET_C(R21.1, R40.2)
	NET_C(LED1.K, R40.1)
	NET_C(LED1.A, V5)
	ALIAS(CON_P, R40.1)

	//----------------------------------------------------------------
	// Credit lamps
	//----------------------------------------------------------------

	/* The credit lamp circuit uses thyristors and 6VAC. This is
	 * currently not modeled and instead the CREDIT_1_OR_2 and CREDIT2
	 * signals are used.
	 */

	ALIAS(CON_CREDIT1, L9.3) // CREDIT_1_OR_2
	ALIAS(CON_CREDIT2, F9.6) // CREDIT2

	//----------------------------------------------------------------
	// Coin Counter
	//----------------------------------------------------------------

	NET_C(CSW1, E2.1)
	NET_C(CSW2, E2.2)
	RES(R14, 150)
	QBJT_SW(Q6, "2N5190")
	DIODE(CR8, "1N4001")
	NET_C(E2.3, R14.1)
	NET_C(R14.2, Q6.B)
	NET_C(GND, Q6.E)
	NET_C(Q6.C, CR8.A)
	NET_C(CR8.K, V5)
	ALIAS(CON_T, Q6.C)

	// Not on PCB: Coincounter

	RES(CC_R, 20) // this is connected
	NET_C(CC_R.1, CON_T)
	NET_C(CC_R.2, V5)

	//----------------------------------------------------------------
	// Not connected pins
	//----------------------------------------------------------------

	NET_C(ttlhigh, B4.3, B4.4, B4.5, B4.6)
	NET_C(ttlhigh, N6.3, N6.4, N6.5, N6.6)
	NET_C(ttlhigh, M6.3, M6.4, M6.5, M6.6)
	NET_C(ttlhigh, L6.3, L6.4, L6.5, L6.6)

	NET_C(ttlhigh, H6.3, H6.4, H6.5, H6.6)
	NET_C(ttlhigh, K6.3, K6.4, K6.5, K6.6)
	NET_C(ttlhigh, J6.3, J6.4, J6.5, J6.6)

	NET_C(ttlhigh, E1.9, E1.11)

	NET_C(GND, D9.1, D9.2, D9.13, D9.3, D9.4, D9.5)

	//----------------------------------------------------------------
	// Power Pins
	//----------------------------------------------------------------

	NET_C(V5,                A3.14, A4.14, A5.14, A6.14,
							 B3.14, B4.16, B5.16, B6.14, B7.16, B8.16, B9.14,
					  C2.14, C3.14, C4.14, C5.14, C6.14, C7.16, C8.16,
					  D2.14, D3.14, D4.16, D5.14, D6.14, D7.14, D8.14,
			   E1.14, E2.14, E3.14, E4.14, E5.14, E6.14, E7.14, E8.14, E9.14,
					  F2.14,        F4.14, F5.14, F6.16, F7.16, F8.14, F9.14,
			   H1.14, H2.14, H3.14, H4.14, H5.16, H6.16, H7.14, H8.14, H9.14,
			   J1.16, J2.14, J3.14, J4.16, J5.16, J6.16, J7.14, J8.16, J9.14,
			   K1.16, K2.14, K3.14, K4.14, K5.16, K6.16, K7.14, K8.14, K9.14,
			   L1.16, L2.14, L3.16, L4.14, L5.16, L6.16, L7.14, L8.16, L9.14,
			   M1.16, M2.5,  M3.14, M4.14, M5.16, M6.16,        M8.14, M9.14,
			   N1.16, N2.5,  N3.14, N4.14, N5.16, N6.16, N7.14,        N9.16)
	NET_C(GND,               A3.7,  A4.7,  A5.7,  A6.7,
							 B3.7,  B4.8,  B5.8,  B6.7,  B7.8,  B8.8,  B9.7,
					  C2.7,  C3.7,  C4.7,  C5.7,  C6.7,  C7.8,  C8.8,
					  D2.7,  D3.7,  D4.8,  D5.7,  D6.7,  D7.7,  D8.7,
			   E1.7,  E2.7,  E3.7,  E4.7,  E5.7,  E6.7,  E7.7,  E8.7,  E9.7,
					  F2.7,         F4.7,  F5.7,  F6.8,  F7.8,  F8.7,  F9.7,
			   H1.7,  H2.7,  H3.7,  H4.7,  H5.8,  H6.8,  H7.7,  H8.7,  H9.7,
			   J1.8,  J2.7,  J3.7,  J4.8,  J5.8,  J6.8,  J7.7,  J8.8,  J9.7,
			   K1.8,  K2.7,  K3.7,  K4.7,  K5.8,  K6.8,  K7.7,  K8.7,  K9.7,
			   L1.8,  L2.7,  L3.8,  L4.7,  L5.8,  L6.8,  L7.7,  L8.8,  L9.7,
			   M1.8,  M2.12, M3.7,  M4.7,  M5.8,  M6.8,         M8.7,  M9.7,
			   N1.8,  N2.12, N3.7,  N4.7,  N5.8,  N6.8,  N7.7,         N9.8 )

#if (SLOW_BUT_ACCURATE)
	NET_C(VCC, F1.16)
	NET_C(GND, F1.8)
#endif

	// 163% -- manually optimized
	HINT(B6.C, NO_DEACTIVATE)
	HINT(C4.C, NO_DEACTIVATE)
	HINT(C4.D, NO_DEACTIVATE)
	HINT(C5.C, NO_DEACTIVATE)
	HINT(C5.D, NO_DEACTIVATE)
	HINT(E2.B, NO_DEACTIVATE)
	HINT(E3.B, NO_DEACTIVATE)
	HINT(E5.D, NO_DEACTIVATE)
	HINT(E9.F, NO_DEACTIVATE)
	HINT(H2.A, NO_DEACTIVATE)
	HINT(H3.A, NO_DEACTIVATE)
	HINT(J3.D, NO_DEACTIVATE)
	HINT(J5, NO_DEACTIVATE) // 7448 needs to be disabled in all cases
	HINT(J6, NO_DEACTIVATE)
	HINT(J8.A, NO_DEACTIVATE)
	HINT(J8.C, NO_DEACTIVATE)
	HINT(K4.D, NO_DEACTIVATE)
	HINT(M3.B, NO_DEACTIVATE)
	HINT(M3.D, NO_DEACTIVATE)
	HINT(M4.B, NO_DEACTIVATE)
	HINT(M6, NO_DEACTIVATE)
	HINT(M8.A, NO_DEACTIVATE)
	HINT(N7.C, NO_DEACTIVATE)


NETLIST_END()

/*
 * MCR106-2 model from http://www.duncanamps.com/
 * MCR106-1 are used to drive lamps for player 1 and player 2
 * These have a BV of 30 ("-2" has 60, see comments below
 * Not yet modeled.
 *
* MCR106-2  SCR  A G K  MCE  7-17-95
*MCE MCR106-2  60V  4A  pkg:TO-225AA
.SUBCKT XMCR1062 1 2 3
QP  6 4 1  POUT OFF
QN  4 6 5  NOUT OFF
RF  6 4    15MEG
RR  1 4    10MEG
RGK 6 5    6.25K
RG  2 6    46.2
RK  3 5    16.2M
DF  6 4    ZF
DR  1 4    ZR
DGK 6 5    ZGK
.MODEL ZF   D (IS=1.6F IBV=800N BV=60 RS=2.25MEG) // BV=30
.MODEL ZR   D (IS=1.6F IBV=800N BV=80) // BV=80/60*30
.MODEL ZGK  D (IS=1.6F IBV=800N BV=6)
.MODEL POUT PNP (IS=1.6P BF=1 CJE=60.3P)
.MODEL NOUT NPN (IS=1.6P BF=100 RC=65M
+ CJE=60.3P CJC=12P TF=126N TR=18U)
.ENDS XMCR1062
 */
