// license:GPL-2.0+
// copyright-holders:Couriersud,DICE Team,
/*
 * The work herein is based on the "Rebound computer service manual"
 * available on archive.org:
 *
 *    https://archive.org/details/ArcadeGameManualRebound
 *
 * Changelog:
 *
 *      - Migrated from DICE (Couriersud)
 *      - Added all missing elements (Couriersud)
 *        - Added discrete startup logic
 *        - Added credit led
 *        - Added missing start and coin discrete elements
 *        - Added missing paddle elements
 *        - Fixed bugs in start and coin logic and ensured it matches schematics
 *      - Replaced HLE by discrete elements (Couriersud)
 *      - Documented PCB connector (Couriersud)
 *
 * TODO:
 *      - Add switch to pull Connector 3 (CON3) to GND
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

#ifndef __PLIB_PREPROCESSOR__
	#define NL_PROHIBIT_BASEH_INCLUDE   1
	#include "netlist/devices/net_lib.h"
#endif

static NETLIST_START(rebound_schematics)

	 ANALOG_INPUT(V5, 5)

	/* -----------------------------------------------------------------------
	 * External interface section
	 * -----------------------------------------------------------------------*/

	SWITCH2(COIN1_SW)
	SWITCH2(START_SW)

	TTL_INPUT(antenna, 0)
	NET_C(V5,  antenna.VCC)
	NET_C(GND, antenna.GND)

	// DSW1a/b are actually only a 1 contact switches.
	// The logic is needed to force high level on
	// inputs since proxies have zero input impedance.
	SWITCH2(DSW1a)
	NET_C(V5, DSW1a.1)
	SWITCH2(DSW1b)
	NET_C(V5, DSW1b.1)
	SWITCH2(DSW2)

	// POTS
	POT(POTP1, RES_K(1)) // 1k ... pure guess - not documented
	PARAM(POTP1.DIALLOG, 0)
	PARAM(POTP1.REVERSE, 1)
	NET_C(POTP1.1, V5)
	NET_C(POTP1.3, GND)
	ALIAS(CON14, POTP1.2)

	POT(POTP2, RES_K(1)) // 1k ... pure guess - not documented
	PARAM(POTP2.DIALLOG, 0)
	PARAM(POTP2.REVERSE, 1)
	NET_C(POTP2.1, V5)
	NET_C(POTP2.3, GND)
	ALIAS(CON18, POTP2.2)

	NET_C(START_SW.1, CON5)
	NET_C(START_SW.2, CON7)
	NET_C(START_SW.Q, GND)

	NET_C(COIN1_SW.1, CON12)
	NET_C(COIN1_SW.2, CON10)
	NET_C(COIN1_SW.Q, GND)

	NET_C(CON15, antenna)

	// Coin Counter is connected to CON10 as well. Simulate using resistor

	RES(RCOINCOUNTER, 10)
	NET_C(RCOINCOUNTER.1, CON10)
	NET_C(RCOINCOUNTER.2, V5)

	/* -----------------------------------------------------------------------
	 * Real netlist start
	 * -----------------------------------------------------------------------*/

	MAINCLOCK(CLOCK, 14318000.0) //7159000.0*2

	TTL_7400_DIP(A1)

	TTL_7400_DIP(B1)
	TTL_7474_DIP(B2)
	TTL_74107_DIP(B3)
	TTL_9316_DIP(B4)
	TTL_7404_DIP(B5)
	TTL_9322_DIP(B6)
	TTL_9316_DIP(B7)
	TTL_7410_DIP(B8)
	TTL_7493_DIP(C1)
	TTL_7450_DIP(C2)
	TTL_7410_DIP(C3)
	TTL_9316_DIP(C4)
	TTL_7486_DIP(C5)
	TTL_7474_DIP(C6)
	TTL_9316_DIP(C7)
	TTL_7402_DIP(C8)
	TTL_7402_DIP(D2)
	TTL_9316_DIP(D3)
	TTL_9316_DIP(D4)
	TTL_7400_DIP(D5)
	TTL_7474_DIP(D6)
	TTL_7400_DIP(D7)
	TTL_7474_DIP(D8)

	TTL_7400_DIP(E1)
	TTL_7483_DIP(E2)
	TTL_9316_DIP(E3)
	TTL_74193_DIP(E4)
	TTL_7408_DIP(E5)
	TTL_7402_DIP(E6)
	TTL_7474_DIP(E7)
	TTL_7474_DIP(E8)
	TTL_7404_DIP(E9)

	TTL_7474_DIP(F1)
	TTL_7408_DIP(F2)
	TTL_74107_DIP(F3)
	TTL_7420_DIP(F4)
	TTL_7493_DIP(F5)
	TTL_7410_DIP(F6)
	TTL_7408_DIP(F7)
	TTL_7410_DIP(F8)
	TTL_7404_DIP(F9)

	TTL_7400_DIP(H1)
	TTL_7410_DIP(H2)
	TTL_7493_DIP(H3)
	TTL_74107_DIP(H4)
	TTL_7493_DIP(H5)
	TTL_7427_DIP(H6)
	TTL_7410_DIP(H7)
	TTL_7430_DIP(H8)
	TTL_74107_DIP(H9)

	TTL_7474_DIP(J1)
	TTL_7474_DIP(J2)
	TTL_7493_DIP(J3)
	TTL_7474_DIP(J4)
	TTL_7486_DIP(J5)
	TTL_7400_DIP(J6)
	TTL_7410_DIP(J7)
	TTL_74153_DIP(J8)
	TTL_7490_DIP(J9)

	TTL_7493_DIP(K2)
	TTL_9316_DIP(K3)
	TTL_7400_DIP(K4)
	TTL_7474_DIP(K5)
	TTL_7402_DIP(K6)
	TTL_7448_DIP(K7)
	TTL_74153_DIP(K8)
	TTL_7490_DIP(K9)

	/* -----------------------------------------------------------------------
	 * Paddle 2
	 * -----------------------------------------------------------------------*/

	// FIXME: Hack, schematic shows 100k (100 to be exact which makes even less sense)
	// One way or the other, according to schematics both horizontal position logic for
	// P1 and P2 start at hreset_n. This limits the usable range for P2.

	RES(R24, 47)
	CAP(CC27, CAP_U(0.1))
	NET_C(CC27.2, GND)
	NET_C(B9.5, CC27.1, R24.2)
	NET_C(R24.1, CON18)

	NE555_DIP(B9)
	RES(R23, RES_K(200.0))
	CAP(CC26, CAP_P(220.0))

	NET_C(B9.6, B9.7)
	NET_C(B9.6, R23.1)
	NET_C(B9.6, CC26.1)
	NET_C(R23.2, V5)
	NET_C(CC26.2, GND)
	NET_C(B9.8, V5)
	NET_C(B9.1, GND)

	/* -----------------------------------------------------------------------
	 * Paddle 1
	 * -----------------------------------------------------------------------*/

	RES(R17, 47)
	CAP(CC23, CAP_U(0.1))
	NET_C(CC23.2, GND)
	NET_C(C9.5, CC23.1, R17.2)
	NET_C(R17.1, CON14)

	NE555_DIP(C9)
	NET_C(C9.6, C9.7)
	RES(R16, RES_K(100.0))
	CAP(CC22, CAP_P(220.0))
	NET_C(C9.6, R16.1)
	NET_C(C9.6, CC22.1)
	NET_C(R16.2, V5)
	NET_C(CC22.2, GND)
	NET_C(C9.8, V5)
	NET_C(C9.1, GND)

	TTL_9602_DIP(D1)

	RES(R10, RES_K(47.0))
	CAP(CC16, CAP_U(5.0))
	RES(R9, RES_K(47.0))
	CAP(CC21, CAP_U(10.0))

	NET_C(V5, D1.16)
	NET_C(GND, D1.8)
	NET_C(D1.1, CC16.1)
	NET_C(D1.2, CC16.2)
	NET_C(D1.2, R10.2)
	NET_C(V5, R10.1)
	NET_C(D1.15, CC21.1)
	NET_C(D1.14, CC21.2)
	NET_C(D1.14, R9.2)
	NET_C(V5, R9.1)

	TTL_9602_DIP(K1)

	RES(R2, RES_K(33.0))
	CAP(CC2, CAP_U(47.0))
	RES(R1, RES_K(47.0))
	CAP(CC3, CAP_U(10.0))

	NET_C(V5, K1.16)
	NET_C(GND, K1.8)
	NET_C(K1.1, CC2.1)
	NET_C(K1.2, CC2.2)
	NET_C(K1.2, R2.2)
	NET_C(V5, R2.1)
	NET_C(K1.15, CC3.1)
	NET_C(K1.14, CC3.2)
	NET_C(K1.14, R1.2)
	NET_C(V5, R1.1)

	/* -----------------------------------------------------------------------
	 * Start circuit
	 * -----------------------------------------------------------------------*/

	RES(R30, RES_K(1))
	RES(R31, RES_K(1))

	NET_C(V5, R31.1, R30.1)
	NET_C(B1.3, B1.4, A1.5, R30.2)
	NET_C(B1.6, B1.2)
	NET_C(R31.2, E9.1)
	NET_C(E9.2, B1.1)

	ALIAS(CON5, R30.2)
	ALIAS(CON7, B1.2)
	ALIAS(CON3, R31.2) // Not explained in manual

	/* -----------------------------------------------------------------------
	 * Coin circuit
	 * -----------------------------------------------------------------------*/

	DIODE(CR3, "1N916")
	DIODE(CR4, "1N4001")
	CAP(CC20, CAP_U(0.1))
	CAP(CC17, CAP_U(0.1))
	CAP(CC18, CAP_U(0.1))
	CAP(CC19, CAP_U(0.1))
	RES(R12, RES_K(1))
	RES(R13, RES_K(100))

	NE555_DIP(D9)

	NET_C(V5, CR4.K, R12.1, D9.8, D9.4, R13.1, E8.10)
	NET_C(CR3.K, CR4.A)
	NET_C(E9.10, E9.13, B1.5, CC20.2, CR3.A)
	NET_C(E9.12, E9.11, CC17.1, C8.2, E8.13, E8.12)
	NET_C(CC20.1, R12.2, D9.2)
	NET_C(CC19.1, D9.5)
	NET_C(CC18.1, D9.6, D9.7, R13.2)
	NET_C(D9.3, E9.9)
	NET_C(E9.8, E8.11)
	NET_C(CR8.K, E8.8, D8.1, D8.13)

	NET_C(GND, CC17.2, CC19.2, D9.1, CC18.2)

	ALIAS(CON12, E9.11)
	ALIAS(CON10, CR3.K)

	/* -----------------------------------------------------------------------
	 * Power on clear
	 * -----------------------------------------------------------------------*/

	QBJT_EB(Q1, "2N3643")
	QBJT_EB(Q2, "2N3644")
	QBJT_EB(Q3, "2N3643")
	DIODE(CR6, "1N914")
	DIODE(CR7, "1N914")
	DIODE(CR8, "1N914") // not readable on schematic, guessing CR8

	CAP(CC15, CAP_U(0.1))
	CAP(CC24, CAP_U(0.1))
	RES(R8,  100)
	RES(R14, 330)
	RES(R15, 100)
	RES(R18, 100)
	RES(R19, 220)

	NET_C(D7.3, CR6.K)
	NET_C(CR6.A, R19.1, R15.2)
	NET_C(R15.1, Q2.C)
	NET_C(V5, Q2.E, CC15.1, R8.1)
	NET_C(Q2.B, CC15.2, R8.2, R14.1)
	NET_C(R14.2, Q3.C, CR8.A, E9.3)
	NET_C(Q1.C, R19.2, CC24.1, R18.1, Q3.B)
	NET_C(GND, Q1.E, CC24.2, R18.2, Q3.E, CR7.A)
	NET_C(CR7.K, Q1.B)

	/* -----------------------------------------------------------------------
	 * Antenna
	 * -----------------------------------------------------------------------*/

	ALIAS(CON15, CR7.K)

	/* -----------------------------------------------------------------------
	 * Credit LED
	 * -----------------------------------------------------------------------*/

	RES(R7, 150)
	NET_C(D8.8, E9.5)
	NET_C(E9.6, R7.2)

	ALIAS(CON11, R7.1)

	DIODE(CR_LED, "LedRed")
	NET_C(CR_LED.K, CON11)
	NET_C(CR_LED.A, V5)

	/* -----------------------------------------------------------------------
	 * Aliases
	 * -----------------------------------------------------------------------*/

	ALIAS(CLK ,J4.5)
	ALIAS(CLK_n ,J4.6)

	ALIAS(H1 ,H5.12)
	ALIAS(H2 ,H5.9)
	ALIAS(H4 ,H5.8)
	ALIAS(H8 ,H5.11)

	ALIAS(H16 ,F5.12)
	ALIAS(H32 ,F5.9)
	ALIAS(H64 ,F5.8)
	ALIAS(H128 ,F5.11)

	ALIAS(H256 ,H4.3)
	ALIAS(H256_n ,H4.2)

	ALIAS(HRESET ,J4.8)
	ALIAS(HRESET_n ,J4.9)

	ALIAS(HBLANK ,K5.5)
	ALIAS(HBLANK_n ,K5.6)

	ALIAS(HSYNC ,K5.9)
	ALIAS(HSYNC_n ,K5.8)

	ALIAS(V1 ,J3.12)
	ALIAS(V2 ,J3.9)
	ALIAS(V4 ,J3.8)
	ALIAS(V8 ,J3.11)

	ALIAS(V16 ,H3.12)
	ALIAS(V32 ,H3.9)
	ALIAS(V64 ,H3.8)
	ALIAS(V128 ,H3.11)

	ALIAS(V256 ,H4.5)
	ALIAS(V256_n ,H4.6)

	ALIAS(VRESET ,J2.8)
	ALIAS(VRESET_n ,J2.9)

	ALIAS(VSYNC_n ,H1.11)

	ALIAS(NET_n ,B8.12)
	ALIAS(PADDLE_12 ,K6.10)
	ALIAS(NET_HEIGHT_n ,B5.10)

	ALIAS(STOP1_n ,F7.8)
	ALIAS(STOP2_n ,F7.6)

	ALIAS(DISP_PAD1_n ,B8.6)
	ALIAS(DISP_PAD2_n ,B8.8)

	ALIAS(BACKWARDS_n ,D6.5)
	ALIAS(DP ,C6.5)
	ALIAS(CP_n ,D6.8)
	ALIAS(BP_n ,C6.8)

	ALIAS(V64_n ,C8.13)

	ALIAS(HIT ,E6.10)
	ALIAS(HIT_n ,D5.3)

	ALIAS(SERVE ,J1.8)
	ALIAS(SERVE_n ,J1.9)
	ALIAS(SERVE_TIME ,K1.6)
	ALIAS(SERVE_TIME_n ,K1.7)

	ALIAS(RIGHT_SCORE ,A1.11)
	ALIAS(LEFT_SCORE ,A1.3)

	ALIAS(BALL_RETURN ,J5.3)
	ALIAS(MISS_n ,D2.1)

	ALIAS(BALL ,E6.4)
	ALIAS(BALL_n ,E6.1)

	ALIAS(LSB1 ,K9.12)
	ALIAS(LSB2 ,J9.12)
	ALIAS(STOP ,J6.8)

	ALIAS(ATTRACT ,E8.5)
	ALIAS(ATTRACT_n ,E8.6)

	ALIAS(START ,E7.9)
	ALIAS(START_n ,E7.8)


	/* -----------------------------------------------------------------------
	 * HRESET
	 * -----------------------------------------------------------------------*/

	NET_C(CLOCK.Q, J4.3)
	NET_C(J4.6, J4.2)
	NET_C(V5, J4.4)
	NET_C(V5, J4.1)

	NET_C(CLK, H5.14)
	NET_C(J4.8, H5.2)
	NET_C(J4.8, H5.3)
	NET_C(H5.12, H5.1)

	NET_C(H5.11, F5.14)
	NET_C(J4.8, F5.2)
	NET_C(J4.8, F5.3)
	NET_C(F5.12, F5.1)

	NET_C(F5.11, H4.12)
	NET_C(J4.9, H4.13)
	NET_C(V5, H4.1)
	NET_C(V5, H4.4)

	NET_C(H256, F4.5)
	NET_C(H128, F4.2)
	NET_C(H64, F4.1)
	NET_C(H4, F4.4)

	NET_C(CLK_n, J4.11)
	NET_C(V5, J4.10)
	NET_C(V5, J4.13)
	NET_C(F4.6, J4.12)

	/* -----------------------------------------------------------------------
	 * HBLANK/HSYNC
	 * -----------------------------------------------------------------------*/

	NET_C(H16, K4.2)
	NET_C(H64, K4.1)

	NET_C(HRESET_n, K5.4)
	NET_C(GND, K5.2)
	NET_C(GND, K5.3)
	NET_C(K4.3, K5.1)

	NET_C(H64, B5.3)

	NET_C(H32, K4.13)
	NET_C(B5.4, K4.12)

	NET_C(B5.4, F7.1)
	NET_C(K5.5, F7.2)

	NET_C(V5, K5.10)
	NET_C(K4.11, K5.12)
	NET_C(H16, K5.11)
	NET_C(F7.3, K5.13)

	/* -----------------------------------------------------------------------
	 * VRESET
	 * -----------------------------------------------------------------------*/

	NET_C(J4.8, J3.14)
	NET_C(J2.8, J3.2)
	NET_C(J2.8, J3.3)
	NET_C(J3.12, J3.1)

	NET_C(J3.11, H3.14)
	NET_C(J2.8, H3.2)
	NET_C(J2.8, H3.3)
	NET_C(H3.12, H3.1)

	NET_C(H3.11, H4.9)
	NET_C(J2.9, H4.10)
	NET_C(V5, H4.8)
	NET_C(V5, H4.11)

	NET_C(V256, H2.1)
	NET_C(V4, H2.2)
	NET_C(V1, H2.13)

	NET_C(J4.8, J2.11)
	NET_C(V5, J2.10)
	NET_C(V5, J2.13)
	NET_C(H2.12, J2.12)

	/* -----------------------------------------------------------------------
	 * VSYNC
	 * -----------------------------------------------------------------------*/

	NET_C(V8, H2.3)
	NET_C(V4, H2.4)
	NET_C(V2, H2.5)

	NET_C(J2.9, H1.1)
	NET_C(H1.11, H1.2)

	NET_C(H1.3, H1.12)
	NET_C(H2.6, H1.13)

	/* -----------------------------------------------------------------------
	 * Volleyball net
	 * -----------------------------------------------------------------------*/

	NET_C(SERVE_n, K1.11)
	NET_C(BALL_RETURN, K1.12)
	NET_C(V5, K1.13)

	NET_C(J1.6, J1.2)
	NET_C(K1.9, J1.3)
	NET_C(V5, J1.4)
	NET_C(H1.6, J1.1)

	NET_C(SERVE, K2.2)
	NET_C(SERVE, K2.3)
	NET_C(J1.5, K2.14)
	NET_C(K2.12, K2.1)

	NET_C(K2.9, H1.4)
	NET_C(K2.11, H1.5)

	NET_C(V128, K3.9)
	NET_C(V5, K3.3)
	NET_C(K2.9, K3.4)
	NET_C(K2.8, K3.5)
	NET_C(K2.11, K3.6)
	NET_C(K4.8, K3.7)
	NET_C(V5, K3.10)
	NET_C(V4, K3.2)
	NET_C(V5, K3.1) //Not shown on schematic

	NET_C(V64, H2.11)
	NET_C(V32, H2.10)
	NET_C(V128, H2.9)

	NET_C(V16, B1.12)
	NET_C(V8, B1.13)

	NET_C(H2.8, K6.9)
	NET_C(B1.11, K6.8)

	NET_C(V128, K4.10)
	NET_C(K3.15, K4.9)

	NET_C(H2.8, K4.4)
	NET_C(K4.8, K4.5)

	NET_C(K4.6, B5.11)

	NET_C(K4.6, B2.10)
	NET_C(H256, B2.12)
	NET_C(H2, B2.11)
	NET_C(V5, B2.13)

	NET_C(V4, B8.2)
	NET_C(H256, B8.1)
	NET_C(B2.8, B8.13)

	/* -----------------------------------------------------------------------
	 * Stop signals
	 * -----------------------------------------------------------------------*/

	NET_C(H256, F6.3)
	NET_C(H128, F6.5)
	NET_C(H16, F6.4)

	NET_C(H64, F6.11)
	NET_C(H128, F6.9)
	NET_C(H32, F6.10)

	NET_C(F6.6, F7.5)
	NET_C(ATTRACT_n, F7.4)

	NET_C(F6.8, F7.9)
	NET_C(ATTRACT_n, F7.10)

	/* -----------------------------------------------------------------------
	 * Paddle 1
	 * -----------------------------------------------------------------------*/

	NET_C(HRESET_n, C9.2)
	NET_C(STOP1_n, C9.4)

	NET_C(K5.5, C8.6)
	NET_C(C9.3, C8.5)

	NET_C(V5, C7.10)
	NET_C(D7.11, C7.7)
	NET_C(H1, C7.2)
	NET_C(GND, C7.3)
	NET_C(GND, C7.4)
	NET_C(GND, C7.5)
	NET_C(GND, C7.6)
	NET_C(V5, C7.1)
	NET_C(C8.4, C7.9)

	NET_C(ATTRACT_n, D7.12)
	NET_C(C7.15, D7.13)

	NET_C(D7.11, B8.4)
	NET_C(PADDLE_12, B8.3)
	NET_C(C8.4, B8.5)

	/* -----------------------------------------------------------------------
	 * Paddle 2
	 * -----------------------------------------------------------------------*/

	NET_C(HRESET_n, B9.2)
	NET_C(STOP2_n, B9.4)

	NET_C(H256_n, C8.8)
	NET_C(B9.3, C8.9)

	NET_C(V5, B7.10)
	NET_C(D7.8, B7.7)
	NET_C(H1, B7.2)
	NET_C(GND, B7.3)
	NET_C(GND, B7.4)
	NET_C(GND, B7.5)
	NET_C(GND, B7.6)
	NET_C(V5, B7.1)
	NET_C(C8.10, B7.9)

	NET_C(ATTRACT_n, D7.10)
	NET_C(B7.15, D7.9)

	NET_C(D7.8, B8.10)
	NET_C(PADDLE_12, B8.11)
	NET_C(C8.10, B8.9)

	/* -----------------------------------------------------------------------
	 * Ball direction and speed
	 * -----------------------------------------------------------------------*/

	NET_C(C7.11, B6.2)
	NET_C(B7.11, B6.3)
	NET_C(C7.12, B6.5)
	NET_C(B7.12, B6.6)
	NET_C(C7.13, B6.14)
	NET_C(B7.13, B6.13)
	NET_C(B8.6, B6.11)
	NET_C(B8.8, B6.10)
	NET_C(H256, B6.1)
	NET_C(GND, B6.15)

	NET_C(B6.9, B5.1)

	NET_C(B5.2, D5.2)
	NET_C(BALL, D5.1)

	NET_C(H1, E6.9)
	NET_C(D5.3, E6.8)

	NET_C(E6.10, E6.11)
	NET_C(GND, E6.12)

	NET_C(E6.13, E5.13)
	NET_C(SERVE_TIME_n, E5.12)

	NET_C(H256_n, C5.5)
	NET_C(B6.4, C5.4)

	NET_C(LSB1, C5.9)
	NET_C(LSB2, C5.10)

	NET_C(SERVE, D5.10)
	NET_C(C5.8, D5.9)

	NET_C(C5.8, B5.5)

	NET_C(SERVE, D5.12)
	NET_C(B5.6, D5.13)

	NET_C(V5, D6.4)
	NET_C(C5.6, D6.2)
	NET_C(E6.10, D6.3)
	NET_C(SERVE_TIME_n, D6.1)

	NET_C(D5.8, C6.4)
	NET_C(B6.4, C6.2)
	NET_C(E6.10, C6.3)
	NET_C(D5.11, C6.1)

	NET_C(D5.11, D6.10)
	NET_C(B6.7, D6.12)
	NET_C(E6.10, D6.11)
	NET_C(D5.8, D6.13)

	NET_C(D5.8, C6.10)
	NET_C(B6.12, C6.12)
	NET_C(E6.10, C6.11)
	NET_C(D5.11, C6.13)

	NET_C(D6.9, C5.1)
	NET_C(C6.5, C5.2)

	NET_C(C6.9, C5.13)
	NET_C(C6.5, C5.12)

	NET_C(C5.3, E5.4)
	NET_C(SERVE_n, E5.5)

	NET_C(B4.15, B5.9)

	NET_C(V5, B4.10)
	NET_C(V5, B4.7)
	NET_C(V64, B4.2)
	NET_C(V5, B4.3)
	NET_C(V5, B4.4)
	NET_C(GND, B4.5)
	NET_C(GND, B4.6)
	NET_C(B5.8, B4.9)
	NET_C(D5.3, B4.1)

	NET_C(V5, E4.5)
	NET_C(B4.15, E4.4)
	NET_C(E5.11, E4.11)
	NET_C(C5.11, E4.15)
	NET_C(E5.6, E4.1)
	NET_C(SERVE_n, E4.10)
	NET_C(V5, E4.9)
	NET_C(GND, E4.14)

	/* -----------------------------------------------------------------------
	 * Score
	 * -----------------------------------------------------------------------*/

	NET_C(V64, C8.11)
	NET_C(V64, C8.12)

	NET_C(V128, F9.5)

	NET_C(V32, F7.12)
	NET_C(F9.6, F7.13)

	NET_C(H256_n, J5.9)
	NET_C(H64, J5.10)

	NET_C(H256, J5.13)
	NET_C(H128, J5.12)

	NET_C(C8.13, J6.2)
	NET_C(F7.11, J6.1)

	NET_C(J5.8, J6.5)
	NET_C(J5.11, J6.4)

	NET_C(J6.3, K6.5)
	NET_C(J6.6, K6.6)

	/* -----------------------------------------------------------------------
	 * Score decoder
	 * -----------------------------------------------------------------------*/

	NET_C(K6.4, K7.4)
	NET_C(V5, K7.3) //Not on schematic
	NET_C(V5, K7.5) //Not on schematic
	NET_C(K8.7, K7.7)
	NET_C(K8.9, K7.1)
	NET_C(J8.7, K7.2)
	NET_C(J8.9, K7.6)

	NET_C(H4, J6.13)
	NET_C(H8, J6.12)

	NET_C(H16, K6.2)
	NET_C(GND, K6.3)

	NET_C(V4, F6.2)
	NET_C(V8, F6.1)
	NET_C(H16, F6.13)

	NET_C(H8, H6.5)
	NET_C(H4, H6.4)
	NET_C(K6.1, H6.3)

	NET_C(V16, F9.3)

	NET_C(J6.11, K6.11)
	NET_C(K6.1, K6.12)

	NET_C(K6.1, H6.2)
	NET_C(V8, H6.1)
	NET_C(V4, H6.13)

	NET_C(F6.12, F9.9)

	NET_C(K7.15, J7.2)
	NET_C(H6.6, J7.13)
	NET_C(F9.4, J7.1)

	NET_C(K7.9, J7.9)
	NET_C(H6.6, J7.10)
	NET_C(V16, J7.11)

	NET_C(F9.4, H7.3)
	NET_C(K6.13, H7.4)
	NET_C(K7.12, H7.5)

	NET_C(V16, J7.3)
	NET_C(K6.13, J7.4)
	NET_C(K7.11, J7.5)

	NET_C(K7.13, F8.9)
	NET_C(F9.4, F8.10)
	NET_C(H6.12, F8.11)

	NET_C(F9.4, H7.1)
	NET_C(K7.14, H7.2)
	NET_C(F9.8, H7.13)

	NET_C(K7.10, H7.9)
	NET_C(V16, H7.10)
	NET_C(F9.8, H7.11)

	NET_C(J7.12, H8.4)
	NET_C(J7.8, H8.6)
	NET_C(H7.6, H8.3)
	NET_C(J7.6, H8.5)
	NET_C(NET_n, H8.1)
	NET_C(F8.8, H8.12)
	NET_C(H7.12, H8.11)
	NET_C(H7.8, H8.2)

	/* -----------------------------------------------------------------------
	 * Score counters
	 * -----------------------------------------------------------------------*/

	NET_C(LEFT_SCORE, K9.14)
	NET_C(K9.1, K9.12)
	NET_C(GND, K9.6)
	NET_C(GND, K9.7)
	NET_C(START, K9.2)
	NET_C(START, K9.3)

	NET_C(V5, H9.8)
	NET_C(K9.11, H9.9)
	NET_C(V5, H9.11)
	NET_C(START_n, H9.10)

	NET_C(RIGHT_SCORE, J9.14)
	NET_C(J9.1, J9.12)
	NET_C(GND, J9.6)
	NET_C(GND, J9.7)
	NET_C(START, J9.2)
	NET_C(START, J9.3)

	NET_C(V5, H9.1)
	NET_C(J9.11, H9.12)
	NET_C(V5, H9.4)
	NET_C(START_n, H9.13)

	NET_C(K9.8, DSW1a.2)

	NET_C(H9.5, F8.4)
	NET_C(K9.12, F8.5)
	NET_C(DSW1a.Q, F8.3)

	NET_C(J9.8, DSW1b.2)

	NET_C(H9.3, F8.2)
	NET_C(J9.12, F8.1)
	NET_C(DSW1b.Q, F8.13)

	NET_C(F8.6, J6.10)
	NET_C(F8.12, J6.9)

	NET_C(H32, K8.14)
	NET_C(H64, K8.2)
	NET_C(K9.12, K8.5)
	NET_C(K9.9, K8.11)
	NET_C(V5, K8.6)
	NET_C(V5, K8.4)
	NET_C(J9.12, K8.3)
	NET_C(J9.9, K8.13)
	NET_C(H9.2, K8.12)
	NET_C(H9.6, K8.10)
	NET_C(GND, K8.1)
	NET_C(GND, K8.15)

	NET_C(H32, J8.14)
	NET_C(H64, J8.2)
	NET_C(K9.8, J8.5)
	NET_C(K9.11, J8.11)
	NET_C(H9.6, J8.6)
	NET_C(H9.2, J8.4)
	NET_C(J9.8, J8.3)
	NET_C(J9.11, J8.13)
	NET_C(H9.2, J8.12)
	NET_C(H9.6, J8.10)
	NET_C(GND, J8.1)
	NET_C(GND, J8.15)

	/* -----------------------------------------------------------------------
	 * Serve logic
	 * -----------------------------------------------------------------------*/

	NET_C(H128, C3.10)
	NET_C(H64, C3.11)
	NET_C(H256_n, C3.9)

	NET_C(V64_n, H6.10)
	NET_C(V128, H6.9)
	NET_C(C3.8, H6.11)

	NET_C(START, K1.4)
	NET_C(D1.7, K1.5)
	NET_C(V5, K1.3)

	NET_C(V5, J1.10)
	NET_C(K1.7, J1.12)
	NET_C(H6.8, J1.11)
	NET_C(K1.7, J1.13)

	/* -----------------------------------------------------------------------
	 * Out of bounds checks
	 * -----------------------------------------------------------------------*/

	NET_C(H256, B3.1)
	NET_C(BALL, B3.12)
	NET_C(H256_n, B3.4)
	NET_C(SERVE_n, B3.13)

	NET_C(SERVE_n, J2.4)
	NET_C(B3.3, J2.2)
	NET_C(H2, J2.3)
	NET_C(V5, J2.1)

	NET_C(B3.3, J5.2)
	NET_C(J2.5, J5.1)

	NET_C(V5, D1.11)
	NET_C(F1.8, D1.12)
	NET_C(V5, D1.13)

	NET_C(J5.3, C1.2)
	NET_C(J5.3, C1.3)
	NET_C(D1.9, C1.14)
	NET_C(C1.12, C1.1)

	NET_C(HBLANK_n, B1.9)
	NET_C(NET_HEIGHT_n, B1.10)

	NET_C(J5.3, E5.1)
	NET_C(B1.8, E5.2)

	NET_C(C1.8, D2.8)
	NET_C(E5.3, D2.9)

	NET_C(D2.10, D2.11)
	NET_C(SERVE, D2.12)

	NET_C(BALL_n, D2.5)
	NET_C(V256_n, D2.6)

	NET_C(D2.4, D2.3)
	NET_C(D2.13, D2.2)

	NET_C(V5, B2.4)
	NET_C(H256_n, B2.2)
	NET_C(HIT, B2.3)
	NET_C(V5, B2.1)

	NET_C(D2.4, C2.1)
	NET_C(H256_n, C2.13)
	NET_C(D2.13, C2.10)
	NET_C(B2.5, C2.9)

	NET_C(D2.4, C2.2)
	NET_C(H256, C2.3)
	NET_C(D2.13, C2.4)
	NET_C(B2.6, C2.5)

	NET_C(C2.8, A1.13)
	NET_C(ATTRACT_n, A1.12)

	NET_C(ATTRACT_n, A1.2)
	NET_C(C2.6, A1.1)

	/* -----------------------------------------------------------------------
	 * Horizontal motion
	 * -----------------------------------------------------------------------*/

	NET_C(V5, F1.4)
	NET_C(F1.6, F1.2)
	NET_C(VRESET, F1.3)
	NET_C(BACKWARDS_n, F1.1)

	NET_C(F1.6, F2.5)
	NET_C(VRESET, F2.4)

	NET_C(BP_n, F2.1)
	NET_C(F2.6, F2.2)

	NET_C(CP_n, F2.13)
	NET_C(F2.6, F2.12)

	NET_C(DP, H1.9)
	NET_C(F2.6, H1.10)

	NET_C(H1.8, E1.12)
	NET_C(H1.8, E1.13)

	NET_C(F2.3, E2.10)
	NET_C(F2.11, E2.8)
	NET_C(H1.8, E2.3)
	NET_C(GND, E2.1)
	NET_C(E1.11, E2.11)
	NET_C(V5, E2.7)
	NET_C(V5, E2.4)
	NET_C(GND, E2.16)
	NET_C(GND, E2.13)

	NET_C(E2.15, E3.6)
	NET_C(E2.2, E3.5)
	NET_C(E2.6, E3.4)
	NET_C(E2.9, E3.3)
	NET_C(C3.6, E3.9)
	NET_C(SERVE_n, E3.1)
	NET_C(V5, E3.10)
	NET_C(V5, E3.7)
	NET_C(CLK, E3.2)

	NET_C(GND, D3.6)
	NET_C(GND, D3.5)
	NET_C(V5, D3.4)
	NET_C(V5, D3.3)
	NET_C(C3.6, D3.9)
	NET_C(SERVE_n, D3.1)
	NET_C(V5, D3.10)
	NET_C(E3.15, D3.7)
	NET_C(CLK, D3.2)

	NET_C(V5, F3.1)
	NET_C(D3.15, F3.12)
	NET_C(V5, F3.4)
	NET_C(SERVE_n, F3.13)

	NET_C(E3.15, C3.5)
	NET_C(D3.15, C3.3)
	NET_C(F3.3, C3.4)

	NET_C(F3.3, F4.9)
	NET_C(D3.15, F4.10)
	NET_C(E3.11, F4.12)
	NET_C(E3.12, F4.13)

	/* -----------------------------------------------------------------------
	 * Vertical motion
	 * -----------------------------------------------------------------------*/

	NET_C(E4.7, D4.6)
	NET_C(E4.6, D4.5)
	NET_C(E4.2, D4.4)
	NET_C(E4.3, D4.3)
	NET_C(D5.6, D4.9)
	NET_C(SERVE_n, D4.1)
	NET_C(VSYNC_n, D4.10)
	NET_C(V5, D4.7)
	NET_C(HSYNC_n, D4.2)

	NET_C(GND, C4.6)
	NET_C(GND, C4.5)
	NET_C(GND, C4.4)
	NET_C(GND, C4.3)
	NET_C(D5.6, C4.9)
	NET_C(SERVE_n, C4.1)
	NET_C(V5, C4.10)
	NET_C(D4.15, C4.7)
	NET_C(HSYNC_n, C4.2)

	NET_C(D4.15, D5.5)
	NET_C(C4.15, D5.4)

	NET_C(C4.15, C3.13)
	NET_C(D4.11, C3.2)
	NET_C(D4.12, C3.1)

	NET_C(F4.8, E6.5)
	NET_C(C3.12, E6.6)

	NET_C(E6.4, E6.3)
	NET_C(E6.4, E6.2)

	/* -----------------------------------------------------------------------
	 * Sound
	 * -----------------------------------------------------------------------*/

	NET_C(D2.1, D1.5)
	NET_C(GND, D1.4)
	NET_C(V5, D1.3)

	NET_C(V32, E1.10)
	NET_C(D1.6, E1.9)

	NET_C(J1.9, F1.10)
	NET_C(HIT_n, F1.12)
	NET_C(C4.15, F1.11)
	NET_C(HIT_n, F1.13)

	NET_C(F1.8, E1.2)
	NET_C(C4.14, E1.1)

	NET_C(ATTRACT_n, F2.10)
	NET_C(E1.8, F2.9)

	NET_C(F2.8, E1.5)
	NET_C(E1.3, E1.4)


	/* -----------------------------------------------------------------------
	 * Coin and Start switches continued
	 * -----------------------------------------------------------------------*/

	NET_C(GND, DSW2.1)
	NET_C(V5, DSW2.2)

	NET_C(V5, D8.2)
	NET_C(DSW2.Q, D8.4)
	NET_C(ATTRACT, D8.3)

	NET_C(D8.5, D8.12)
	NET_C(E9.4, D8.10)
	NET_C(ATTRACT, D8.11)

	NET_C(D8.9, C8.3)

	NET_C(C8.1, A1.4)

	NET_C(V5, E7.4)
	NET_C(E8.5, E7.2)
	NET_C(A1.6, E7.3)
	NET_C(E7.8, E7.1)

	NET_C(V5, E7.10)
	NET_C(E7.5, E7.12)
	NET_C(V256, E7.11)
	NET_C(C8.1, E7.13)

	NET_C(E7.9, D7.5)
	NET_C(D8.8, D7.4)

	NET_C(E9.4, E8.4)
	NET_C(V5, E8.2)
	NET_C(STOP, E8.3)
	NET_C(D7.6, E8.1)

	NET_C(D8.9, D7.2)
	NET_C(ATTRACT, D7.1)

	/* -----------------------------------------------------------------------
	 * Video signal
	 * -----------------------------------------------------------------------*/

	NET_C(V4, E5.9)
	NET_C(B5.2, E5.10)

	NET_C(J5.4, H1.11) // VSYNC
	NET_C(J5.5, K5.9) // HSYNC

	RES(R29, 1000)
	RES(R25, 330)
	RES(R26, 330)
	RES(R27, 330)
	RES(R28, 330)

	NET_C(R29.1, V5)
	NET_C(H8.8, R25.1)
	NET_C(J5.6, R26.1)
	NET_C(E6.4, R27.1) // BALL
	NET_C(E5.8, R28.1)

	NET_C(R29.2, R25.2, R26.2, R27.2, R28.2)

	ALIAS(videomix, R29.2)

	/* -----------------------------------------------------------------------
	 * Audio signal
	 * -----------------------------------------------------------------------*/

	ALIAS(sound, E1.6)

	/* -----------------------------------------------------------------------
	 * Inputs not used
	 * -----------------------------------------------------------------------*/

	NET_C(V5, A1.C.A, A1.C.B, B5.F.A)
	NET_C(V5, B3.B.CLK, B3.B.CLRQ, B3.B.J, B3.B.K)
	NET_C(V5, F3.B.CLK, F3.B.CLRQ, F3.B.J, F3.B.K)
	NET_C(V5, F9.A.A, F9.E.A, F9.F.A)

	/* -----------------------------------------------------------------------
	 * Power terminals
	 * -----------------------------------------------------------------------*/

	NET_C(V5,  A1.14,
			   B1.14, B2.14, B3.14, B4.16, B5.14, B6.16, B7.16, B8.14,
			   C1.5,  C2.14, C3.14, C4.16, C5.14, C6.14, C7.16, C8.14,
					  D2.14, D3.16, D4.16, D5.14, D6.14, D7.14, D8.14,
			   E1.14, E2.5,  E3.16, E4.16, E5.14, E6.14, E7.14, E8.14, E9.14,
			   F1.14, F2.14, F3.14, F4.14, F5.5,  F6.14, F7.14, F8.14, F9.14,
			   H1.14, H2.14, H3.5,  H4.14, H5.5,  H6.14, H7.14, H8.14, H9.14,
			   J1.14, J2.14, J3.5,  J4.14, J5.14, J6.14, J7.14, J8.16, J9.5,
					  K2.5,  K3.16, K4.14, K5.14, K6.14, K7.16, K8.16, K9.5)
	NET_C(GND, A1.7,
			   B1.7,  B2.7,  B3.7,  B4.8,  B5.7,  B6.8,  B7.8,  B8.7,
			   C1.10, C2.7,  C3.7,  C4.8,  C5.7,  C6.7,  C7.8,  C8.7,
					  D2.7,  D3.8,  D4.8,  D5.7,  D6.7,  D7.7,  D8.7,
			   E1.7,  E2.12, E3.8,  E4.8,  E5.7,  E6.7,  E7.7,  E8.7,  E9.7,
			   F1.7,  F2.7,  F3.7,  F4.7,  F5.10, F6.7,  F7.7,  F8.7,  F9.7,
			   H1.7,  H2.7,  H3.10, H4.7,  H5.10, H6.7,  H7.7,  H8.7,  H9.7,
			   J1.7,  J2.7,  J3.10, J4.7,  J5.7,  J6.7,  J7.7,  J8.8,  J9.10,
					  K2.10, K3.8,  K4.7,  K5.7,  K6.7,  K7.8,  K8.8,  K9.10)
NETLIST_END()

NETLIST_START(rebound)

	LOCAL_SOURCE(rebound_schematics)
	SOLVER(Solver, 480)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e-6)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 5e-7)

	PARAM(Solver.PARALLEL, 0) // Don't do parallel solvers
	PARAM(NETLIST.USE_DEACTIVATE, 1)
	HINT(F9.B, NO_DEACTIVATE)
	INCLUDE(rebound_schematics)

NETLIST_END()
