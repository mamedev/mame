// license:GPL-2.0+
// copyright-holders:DICE Team,couriersud
/*
 * Changelog:
 *      - Added discrete paddle potentiometers (couriersud)
 *      - Changes made to run in MAME (couriersud)
 *      - Original version imported from DICE
 *
 * TODO:
 *      - implement trimmers
 *
 * The MAME team has asked for and received written confirmation from the
 * author of DICE to use, modify and redistribute code under:
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

// identify unknown devices in IDE

//#define NETLIST_DEVELOPMENT 1

#include "netlist/nl_dice_compat.h"

static Mono555Desc a3_555_desc(K_OHM(100.0), U_FARAD(0.1));

static Mono555Desc a10_555_desc(K_OHM(70.0), U_FARAD(0.1));  // actually 56k + 50k trimmer
static Mono555Desc b10_555_desc(K_OHM(70.0), U_FARAD(0.1));  // actually 56k + 50k trimmer

static Mono555Desc b9a_555_desc(K_OHM(70.0), U_FARAD(0.1));  // actually 56k + 50k trimmer
static Mono555Desc b9b_555_desc(K_OHM(70.0), U_FARAD(0.1));  // actually 56k + 50k trimmer

static Mono555Desc f5_555_desc(K_OHM(330.0), U_FARAD(4.7));
static Mono555Desc g5_555_desc(K_OHM(220.0), U_FARAD(1.0));

static SeriesRCDesc c33_desc(K_OHM(1.0), U_FARAD(0.1)); // Capacitor C33, Resistor R30

#if 0
static Paddle1VerticalDesc pad1_desc(17000.0, 145000.0, &a10_555_desc);
static Paddle2VerticalDesc pad2_desc(17000.0, 145000.0, &b10_555_desc);
static Paddle3VerticalDesc pad3_desc(17000.0, 145000.0, &b9b_555_desc);
static Paddle4VerticalDesc pad4_desc(17000.0, 145000.0, &b9a_555_desc);

static DipswitchDesc dipswitch1_desc("winning_score", "Winning Score", 0, "11", "15");
#endif

CIRCUIT_LAYOUT( pongdoubles )
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7) // works and is sufficient
	//CHIP("CLOCK", CLOCK_14_318_MHZ)
	MAINCLOCK(CLOCK, 14318000.0)

	ANALOG_INPUT(V5, 5)
#define VCC "V5", Q
#define GND "GND", Q

	CHIP("F9",  7493)
	CHIP("F10", 7493)
	CHIP("F7", 74107)
	CHIP("F8", 7430)
	CHIP("E8", 7474)

	CHIP("E9", 7493)
	CHIP("E10", 7493)
	CHIP("D10", 74107)
	CHIP("D9", 7410)

	CHIP("G6", 7410)
	CHIP("H6", 7400)
	CHIP("F6", 7402)

	CHIP("F4", 74107)
	CHIP("G4", 7400)
	CHIP("G3", 7427)

	CHIP("E4", 7427)
	CHIP("E3", 7410)
	CHIP("D3", 7402)
	CHIP("G2", 7402)
	CHIP("F3", 7425)
	CHIP("C6", 7448)
	CHIP("C7", 74153)
	CHIP("D7", 74153)

	CHIP("C4", 7400)
	CHIP("E5", 7404)
	CHIP("E6", 7427)
	CHIP("C5", 7410)
	CHIP("D5", 7410)
	CHIP("D6", 7410)
	CHIP("D4", 7430)

	//CHIP("B10", 555_Mono, &b10_555_desc)
	CHIP_555_Mono(B10, &b10_555_desc)

	// NETLIST - analog start
	POT(B10_POT, RES_K(1))     // This is a guess!!
	PARAM(B10_POT.DIALLOG, 1)  // Log Dial ...
	RES(B10_RPRE, 470)

	NET_C(B10_POT.1, V5)
	NET_C(B10_POT.3, GND)
	NET_C(B10_POT.2, B10_RPRE.1)
	NET_C(B10_RPRE.2, B10.5)
	// NETLIST - analog end


	CHIP("C10", 7404)
	CHIP("A7", 7493)
	CHIP("B8", 7400)
	CHIP("A8", 7420)

	//CHIP("A10", 555_Mono, &a10_555_desc)
	CHIP_555_Mono(A10, &a10_555_desc)

	// NETLIST - analog start
	POT(A10_POT, RES_K(1))     // This is a guess!!
	PARAM(A10_POT.DIALLOG, 1)  // Log Dial ...
	RES(A10_RPRE, 470)

	NET_C(A10_POT.1, V5)
	NET_C(A10_POT.3, GND)
	NET_C(A10_POT.2, A10_RPRE.1)
	NET_C(A10_RPRE.2, A10.5)
	// NETLIST - analog end

	CHIP("A9", 7493)

	CHIP("H4", 7474)

	CHIP("B3", 7400)
	CHIP("D2", 7404)

	CHIP("G8", 9316)
	CHIP("H8", 9316)
	CHIP("G7", 74107)
	CHIP("H7", 7420)
	CHIP("E2", 7400)

	CHIP("B4", 9316)
	CHIP("A4", 9316)
	CHIP("B5", 7483)

	CHIP("B6", 7474)
	CHIP("H5", 7400)
	CHIP("E7", 7400)
	//CHIP("F5", 555_Mono, &f5_555_desc)
	CHIP_555_Mono(F5, &f5_555_desc)

	//CHIP("G5", 555_Mono, &g5_555_desc)
	CHIP_555_Mono(G5, &g5_555_desc)
	CHIP("C3", 7474)
	CHIP("C2", 7400)

	CHIP("F2", 7493)
	CHIP("H2", 7400)
	CHIP("H3", 74107)

	CHIP("B7", 7450)
	CHIP("A6", 7474)
	CHIP("A5", 7486)
	//CHIP("D10", 74107)

	CHIP("C8", 7490)
	CHIP("D8", 7490)
	CHIP("C9", 74107)

	CHIP("A1", 74153)
	CHIP("B1", 74153)

	CHIP("B2", 7474)
	CHIP("H1", 7402)
	CHIP("G1", 7404)

	CHIP("E1", 7400)
	CHIP("A2", 7420)
	CHIP("D1", 7493)
	CHIP("C1", 7493)

	CHIP("J1", 7474)
	CHIP("F1", 7427)

	CHIP("J10", 7400)
	CHIP("H10", 7474)
	CHIP("G10", 7474)
	//CHIP("A3", 555_Mono, &a3_555_desc)
	CHIP_555_Mono(A3, &a3_555_desc)

	//CHIP("B9A", 555_Mono, &b9a_555_desc)
	CHIP_555_Mono(B9A, &b9a_555_desc)

	// NETLIST - analog start
	POT(B9A_POT, RES_K(1))     // This is a guess!!
	PARAM(B9A_POT.DIALLOG, 1)  // Log Dial ...
	RES(B9A_RPRE, 470)

	NET_C(B9A_POT.1, V5)
	NET_C(B9A_POT.3, GND)
	NET_C(B9A_POT.2, B9A_RPRE.1)
	NET_C(B9A_RPRE.2, B9A.5)
	// NETLIST - analog end

	//CHIP("B9B", 555_Mono, &b9b_555_desc)
	CHIP_555_Mono(B9B, &b9b_555_desc)

	// NETLIST - analog start
	POT(B9B_POT, RES_K(1))     // This is a guess!!
	PARAM(B9B_POT.DIALLOG, 1)  // Log Dial ...
	RES(B9B_RPRE, 470)

	NET_C(B9B_POT.1, V5)
	NET_C(B9B_POT.3, GND)
	NET_C(B9B_POT.2, B9B_RPRE.1)
	NET_C(B9B_RPRE.2, B9B.5)
	// NETLIST - analog end

	CHIP_SERIES_RC(C33, &c33_desc)

#if 0
	CHIP("PAD1", PADDLE1_VERTICAL_INPUT, &pad1_desc)
	PADDLE_CONNECTION("PAD1", "A10")

	CHIP("PAD2", PADDLE2_VERTICAL_INPUT, &pad2_desc)
	PADDLE_CONNECTION("PAD2", "B10")

	CHIP("PAD3", PADDLE3_VERTICAL_INPUT, &pad3_desc)
	PADDLE_CONNECTION("PAD3", "B9B")

	CHIP("PAD4", PADDLE4_VERTICAL_INPUT, &pad4_desc)
	PADDLE_CONNECTION("PAD4", "B9A")

	CHIP("LATCH", LATCH)
	CHIP("COIN", COIN_INPUT)
	CHIP("START", START_INPUT)
	CHIP("DIPSW1", DIPSWITCH, &dipswitch1_desc)

	VIDEO(pongdoubles)
#endif

	//CHIP("COIN", COIN_INPUT)
	CHIP_INPUT_ACTIVE_LOW(COIN)
	//CHIP("LATCH", LATCH)
	CHIP_LATCH(LATCH)
	//CHIP("START", START_INPUT)
	CHIP_INPUT_ACTIVE_HIGH(START)
	//CHIP("DIPSW1", DIPSWITCH, &dipswitch1_desc)
	SWITCH2(DIPSW1)
	SWITCH2(DIPSW2)
#ifdef DEBUG
	CHIP("LOG1", VCD_LOG, &vcd_log_desc)
#endif

	// Not used, need to be connected
	CONNECTION(GND, "C2", 1)
	CONNECTION(GND, "C2", 2)
	CONNECTION(GND, "A5", 1)
	CONNECTION(GND, "A5", 2)
	CONNECTION(GND, "G7", 4)
	CONNECTION(GND, "G7", 1)
	CONNECTION(GND, "G7",13)
	CONNECTION(GND, "G7",12)
	CONNECTION(GND, "J10",13)
	CONNECTION(GND, "J10",12)
	CONNECTION(GND, "C10",9)
	CONNECTION(GND, "C10",11)
	CONNECTION(GND, "C10",13)
	CONNECTION(GND, "B1",10)
	CONNECTION(GND, "B1",11)
	CONNECTION(GND, "B1",12)
	CONNECTION(GND, "B1",13)
	CONNECTION(GND, "E7",12)
	CONNECTION(GND, "E7",13)
	CONNECTION(GND, "H1",11)
	CONNECTION(GND, "H1",12)
	CONNECTION(GND, "G1",9)

	// HRESET Circuit

	CONNECTION("CLOCK", Q, "F7", 12)
	CONNECTION(VCC, "F7", 1)
	CONNECTION(VCC, "F7", 4)
	CONNECTION(VCC, "F7", 13)

	#define CLK "F7", 3

	CONNECTION("F9", 12, "F9", 1)
	CONNECTION(CLK, "F9", 14)
	CONNECTION("E8", 8, "F9", 2)
	CONNECTION("E8", 8, "F9", 3)

	CONNECTION("F10", 12, "F10", 1)
	CONNECTION("F9", 11, "F10", 14)
	CONNECTION("E8", 8, "F10", 2)
	CONNECTION("E8", 8, "F10", 3)

	CONNECTION(VCC, "F7", 8)
	CONNECTION(VCC, "F7", 11)
	CONNECTION("F10", 11, "F7", 9)
	CONNECTION("E8", 9, "F7", 10)

	CONNECTION("F7", 5, "F8", 1)
	CONNECTION("F10", 11, "F8", 11)
	CONNECTION("F10", 8, "F8", 12)
	CONNECTION("F9", 8, "F8", 5)
	CONNECTION("F9", 9, "F8", 6)
	CONNECTION(VCC, "F8", 2)
	CONNECTION(VCC, "F8", 3)
	CONNECTION(VCC, "F8", 4)

	CONNECTION(CLK, "E8", 11)
	CONNECTION("F8", 8, "E8", 12)
	CONNECTION(VCC, "E8", 13)
	CONNECTION(VCC, "E8", 10)

	#define H1 "F9", 12
	#define H2 "F9", 9
	#define H4 "F9", 8
	#define H8 "F9", 11
	#define H16 "F10", 12
	#define H32 "F10", 9
	#define H64 "F10", 8
	#define H128 "F10", 11
	#define H256 "F7", 5
	#define H256_n "F7", 6
	#define HRESET "E8", 8
	#define HRESET_n "E8", 9


	// VRESET Circuit
	CONNECTION("E9", 12, "E9", 1)
	CONNECTION(HRESET, "E9", 14)
	CONNECTION("E8", 6, "E9", 2)
	CONNECTION("E8", 6, "E9", 3)

	CONNECTION("E10", 12, "E10", 1)
	CONNECTION("E9", 11, "E10", 14)
	CONNECTION("E8", 6, "E10", 2)
	CONNECTION("E8", 6, "E10", 3)

	CONNECTION(VCC, "D10", 8)
	CONNECTION(VCC, "D10", 11)
	CONNECTION("E10", 11, "D10", 9)
	CONNECTION("E8", 5, "D10", 10)

	CONNECTION("D10", 5, "D9", 9)
	CONNECTION("E9", 12, "D9", 10)
	CONNECTION("E9", 8, "D9", 11)

	CONNECTION(HRESET, "E8", 3)
	CONNECTION("D9", 8, "E8", 2)
	CONNECTION(VCC, "E8", 1)
	CONNECTION(VCC, "E8", 4)

	#define V1 "E9", 12
	#define V2 "E9", 9
	#define V4 "E9", 8
	#define V8 "E9", 11
	#define V16 "E10", 12
	#define V32 "E10", 9
	#define V64 "E10", 8
	#define V128 "E10", 11
	#define V256 "D10", 5
	#define V256_n "D10", 6
	#define VRESET "E8", 6
	#define VRESET_n "E8", 5


	// HSync Logic
	CONNECTION(H16, "G6", 3)
	CONNECTION(H64, "G6", 4)
	CONNECTION(H64, "G6", 5)

	CONNECTION("G6", 6, "H6", 4)
	CONNECTION(HRESET_n, "H6", 10)
	CONNECTION("H6", 6, "H6", 9)
	CONNECTION("H6", 8, "H6", 5)

	CONNECTION(H64, "D2", 9)

	CONNECTION(H64, "D3", 3)
	CONNECTION("H6", 6, "D3", 2)

	CONNECTION("D2", 8, "C2", 9)
	//CONNECTION("D2", 8, "C2", 10)
	CONNECTION(H32, "C2", 10) // Not shown. Accurate?

	CONNECTION(VCC, "B2", 10)
	CONNECTION("C2", 8, "B2", 12)
	CONNECTION(H16, "B2", 11)
	CONNECTION("D3", 1, "B2", 13)

	#define HBLANK "H6", 8
	#define HBLANK_n "H6", 6
	#define HSYNC_n "B2", 8

	// VSync Logic
	CONNECTION(VRESET, "F6", 8)
	CONNECTION(V16, "F6", 12)
	CONNECTION("F6", 10, "F6", 11)
	CONNECTION("F6", 13, "F6", 9)

	CONNECTION(V8, "H6", 1)
	CONNECTION(V8, "H6", 2)

	CONNECTION("F6", 13, "G6", 1)
	CONNECTION(V4, "G6", 2)
	CONNECTION("H6", 3, "G6", 13)

	#define VBLANK "F6", 13
	#define VBLANK_n "F6", 10


	// Net circuit
	CONNECTION(H256, "F4", 8)
	CONNECTION(H256_n, "F4", 11)
	CONNECTION(CLK, "F4", 9)
	CONNECTION(VCC, "F4", 10)

	CONNECTION("F4", 6, "G4", 5)
	CONNECTION(H256, "G4", 4)

	CONNECTION(V4, "G3", 3)
	CONNECTION(VBLANK, "G3", 4)
	CONNECTION("G4", 6, "G3", 5)

	#define NET "G3", 6


	// Score decoding circuit
	CONNECTION("C9", 2, "D7", 6)
	CONNECTION("C8", 8, "D7", 5)
	CONNECTION("C9", 6, "D7", 4)
	CONNECTION("D8", 8, "D7", 3)
	CONNECTION("C9", 2, "D7", 10)
	CONNECTION("C8", 11, "D7", 11)
	CONNECTION("C9", 6, "D7", 12)
	CONNECTION("D8", 11, "D7", 13)
	CONNECTION(H32, "D7", 14)
	CONNECTION(H64, "D7", 2)
	CONNECTION(GND, "D7", 1)
	CONNECTION(GND, "D7", 15)

	CONNECTION(VCC, "C7", 6)
	CONNECTION("C8", 12, "C7", 5)
	CONNECTION(VCC, "C7", 4)
	CONNECTION("D8", 12, "C7", 3)
	CONNECTION("C9", 2, "C7", 10)
	CONNECTION("C8", 9, "C7", 11)
	CONNECTION("C9", 6, "C7", 12)
	CONNECTION("D8", 9, "C7", 13)
	CONNECTION(H32, "C7", 14)
	CONNECTION(H64, "C7", 2)
	CONNECTION(GND, "C7", 1)
	CONNECTION(GND, "C7", 15)

	CONNECTION(H128, "E4", 1)
	CONNECTION(H128, "E4", 2)
	CONNECTION(H128, "E4", 13)

	CONNECTION("E4", 12, "E4", 5)
	CONNECTION(H64, "E4", 4)
	CONNECTION(H256, "E4", 3)

	CONNECTION("E4", 12, "E3", 9)
	CONNECTION(H64, "E3", 10)
	CONNECTION(H256, "E3", 11)

	CONNECTION("E3", 8, "E4", 9)
	CONNECTION("E3", 8, "E4", 10)
	CONNECTION("E3", 8, "E4", 11)

	CONNECTION("E4", 6, "D3", 8)
	CONNECTION("E4", 8, "D3", 9)

	CONNECTION(V32, "G2", 2)
	CONNECTION(V32, "G2", 3)

	CONNECTION("G2", 1, "F3", 1)
	CONNECTION(V64, "F3", 2)
	CONNECTION(V128, "F3", 4)
	CONNECTION("D3", 10, "F3", 5)

	CONNECTION("C7", 7, "C6", 7)
	CONNECTION("C7", 9, "C6", 1)
	CONNECTION("D7", 7, "C6", 2)
	CONNECTION("D7", 9, "C6", 6)
	CONNECTION("F3", 6, "C6", 4)
	CONNECTION(VCC, "C6", 3)
	CONNECTION(VCC, "C6", 5)


	// Score display circuit
	CONNECTION(H16, "E5", 3)

	CONNECTION(H4, "C4", 12)
	CONNECTION(H8, "C4", 13)

	CONNECTION("E5", 4, "E6", 9)
	CONNECTION(H4, "E6", 10)
	CONNECTION(H8, "E6", 11)

	CONNECTION("C4", 11, "D3", 5)
	CONNECTION("E5", 4, "D3", 6)

	CONNECTION(V8, "E6", 3)
	CONNECTION(V4, "E6", 4)
	CONNECTION("E5", 4, "E6", 5)

	CONNECTION(V4, "E3", 1)
	CONNECTION(V8, "E3", 2)
	CONNECTION(H16, "E3", 13)

	CONNECTION("E5", 1, "E3", 12)

	CONNECTION(V16, "E5", 5)

	CONNECTION("E5", 6, "D5", 1)
	CONNECTION("C6", 15, "D5", 2)
	CONNECTION("E6", 8, "D5", 13)

	CONNECTION("C6", 9, "D6", 9)
	CONNECTION(V16, "D6", 10)
	CONNECTION("E6", 8, "D6", 11)

	CONNECTION("D3", 4, "C5", 9)
	CONNECTION("E5", 6, "C5", 10)
	CONNECTION("C6", 12, "C5", 11)

	CONNECTION("D3", 4, "D6", 1)
	CONNECTION("C6", 11, "D6", 2)
	CONNECTION(V16, "D6", 13)

	CONNECTION("C6", 13, "D5", 9)
	CONNECTION("E5", 6, "D5", 10)
	CONNECTION("E6", 6, "D5", 11)

	CONNECTION("C6", 14, "D5", 3)
	CONNECTION("E5", 2, "D5", 4)
	CONNECTION("E5", 6, "D5", 5)

	CONNECTION("E5", 2, "D6", 3)
	CONNECTION(V16, "D6", 4)
	CONNECTION("C6", 10, "D6", 5)

	CONNECTION("D5", 12, "D4", 12)
	CONNECTION("D6", 8, "D4", 5)
	CONNECTION("C5", 8, "D4", 4)
	CONNECTION(VCC, "D4", 1)
	CONNECTION("D6", 12, "D4", 2)
	CONNECTION("D5", 8, "D4", 11)
	CONNECTION("D5", 6, "D4", 6)
	CONNECTION("D6", 6, "D4", 3)


	// Coin / Start Logic
	CONNECTION("COIN", 1, "C10", 5)

	// Capacitor C33 used to drive a ~0.1ms low pulse
	// onto A3 pin 2 at the falling edge of COIN.
	CONNECTION("COIN", 1, "C33", 1)
	CONNECTION(VCC, "C33", 2)

	CONNECTION(VCC, "A3", 4)
	CONNECTION("C33", 3, "A3", 2)

	CONNECTION("A3", 3, "D2", 11)

	CONNECTION(VCC, "B2", 4)
	CONNECTION("D2", 10, "B2", 3)
	CONNECTION("C10", 6, "B2", 2)
	CONNECTION("C10", 6, "B2", 1)

	CONNECTION("J10", 3, "LATCH", 1)
	CONNECTION("B2", 6, "LATCH", 2)

	CONNECTION("LATCH", 3, "H6", 12)
	CONNECTION("LATCH", 3, "H6", 13) // Wrong pins on schematic

	CONNECTION("START", 1, "J10", 9)
	CONNECTION("START", 1, "J10", 10)

	CONNECTION(VCC, "C3", 10)
	CONNECTION("H10", 6, "C3", 12)
	CONNECTION("B2", 5, "C3", 11)
	CONNECTION(VCC, "C3", 13)

	CONNECTION("H6", 11, "H10", 4)
	CONNECTION(VCC, "H10", 2)
	CONNECTION("B2", 6, "H10", 1)
	CONNECTION("G10", 9, "H10", 3)

	CONNECTION("H10", 5, "J10", 2)
	CONNECTION("G10", 9, "J10", 1)

	CONNECTION(VCC, "G10", 4)
	CONNECTION("G10", 9, "G10", 2)
	CONNECTION("J10", 8, "G10", 3)
	CONNECTION("H10", 8, "G10", 1)

	CONNECTION(VCC, "H10", 10)
	CONNECTION("G10", 5, "H10", 12)
	CONNECTION(V256, "H10", 11)
	CONNECTION(VCC, "H10", 13)

	CONNECTION("H10", 9, "J10", 4)
	CONNECTION("H10", 6, "J10", 5)

	CONNECTION("H6", 11, "G10", 10)
	CONNECTION(VCC, "G10", 12)
	CONNECTION("B3", 3, "G10", 11)
	CONNECTION("J10", 6, "G10", 13) // Schematic shows pin 10

	CONNECTION("J10", 6, "E5", 9) // Schematic says E6?

	#define ATTRACT_n "D2", 4
	#define ATTRACT   "G10", 9

	#define SRST "E5", 8
	#define SRST_n "J10", 6


	// Game Control Logic
	CONNECTION("H7", 6, "E7", 9)
	CONNECTION("H6", 8, "E7", 10)

	CONNECTION("E7", 8, "D2", 13)

	CONNECTION("D2", 12, "E2", 1)
	CONNECTION(ATTRACT_n, "E2", 2)

	CONNECTION(SRST_n, "E7", 4)
	CONNECTION("E7", 8, "E7", 5)

	CONNECTION("E7", 6, "E7", 1)
	CONNECTION("E7", 6, "E7", 2)

	CONNECTION("E7", 3, "F5", 2)
	CONNECTION(VCC, "F5", 4)

	CONNECTION(ATTRACT, "E6", 1)
	CONNECTION("B3", 3, "E6", 2)
	CONNECTION("F5", 3, "E6", 13)

	CONNECTION("E6", 12, "B6", 12)
	CONNECTION("E6", 12, "B6", 13)
	CONNECTION(VCC, "B6", 10)
	CONNECTION("G3", 8, "B6", 11)

	CONNECTION(ATTRACT, "D2", 3)



	// Horizontal Ball Counter
	CONNECTION(ATTRACT_n, "E2", 4)
	CONNECTION("B6", 8, "E2", 5)

	CONNECTION("H5", 8, "G8", 3)
	CONNECTION("H5", 6, "G8", 4)
	CONNECTION(GND, "G8", 5)
	CONNECTION(VCC, "G8", 6)
	CONNECTION(CLK, "G8", 2)
	CONNECTION("E2", 6, "G8", 1)
	CONNECTION(VCC, "G8", 7)
	CONNECTION("H6", 6, "G8", 10)
	CONNECTION("G6", 8, "G8", 9)

	CONNECTION(GND, "H8", 3)
	CONNECTION(GND, "H8", 4)
	CONNECTION(GND, "H8", 5)
	CONNECTION(VCC, "H8", 6)
	CONNECTION(CLK, "H8", 2)
	CONNECTION("E2", 6, "H8", 1)
	CONNECTION("G8", 15, "H8", 7)
	CONNECTION(VCC, "H8", 10)
	CONNECTION("G6", 8, "H8", 9)

	CONNECTION(VCC, "G7", 8)
	CONNECTION(VCC, "G7", 11)
	CONNECTION("H8", 15, "G7", 9)
	CONNECTION("E2", 6, "G7", 10)

	CONNECTION("G8", 12, "H7", 9)
	CONNECTION("G8", 11, "H7", 10)
	CONNECTION("H8", 15, "H7", 12)
	CONNECTION("G7", 5, "H7", 13)

	#define HVID_n "H7", 8

	CONNECTION("H8", 15, "G6", 9)
	CONNECTION("G8", 15, "G6", 10)
	CONNECTION("G7", 5, "G6", 11)

	CONNECTION(HVID_n, "H7", 1)
	CONNECTION(HVID_n, "H7", 2)
	CONNECTION(HVID_n, "H7", 4)
	CONNECTION(HVID_n, "H7", 5)

	#define HVID "H7", 6


	// Vertical Ball Counter
	CONNECTION("B5", 9, "B4", 3)
	CONNECTION("B5", 6, "B4", 4)
	CONNECTION("B5", 2, "B4", 5)
	CONNECTION("B5", 15, "B4", 6)
	CONNECTION("B2", 8, "B4", 2)
	CONNECTION(VCC, "B4", 1)
	CONNECTION(VCC, "B4", 7)
	CONNECTION("F6", 10, "B4", 10)
	CONNECTION("B3", 6, "B4", 9)

	CONNECTION(GND, "A4", 3)
	CONNECTION(GND, "A4", 4)
	CONNECTION(GND, "A4", 5)
	CONNECTION(GND, "A4", 6)
	CONNECTION("B2", 8, "A4", 2)
	CONNECTION(VCC, "A4", 1)
	CONNECTION("B4", 15, "A4", 7)
	CONNECTION(VCC, "A4", 10)
	CONNECTION("B3", 6, "A4", 9)

	CONNECTION("B4", 15, "B3", 5)
	CONNECTION("A4", 15, "B3", 4)

	CONNECTION("B4", 12, "E3", 3)
	CONNECTION("B4", 11, "E3", 4)
	CONNECTION("A4", 15, "E3", 5)

	CONNECTION("E3", 6, "D3", 11)
	CONNECTION("E3", 6, "D3", 12)

	#define VVID_n "E3", 6
	#define VVID   "D3", 13


	// Hit Logic
	CONNECTION(HVID_n, "G2", 5)
	CONNECTION(VVID_n, "G2", 6)

	CONNECTION("G1", 10, "G4", 1)
	CONNECTION("G2", 4, "G4", 2)

	CONNECTION("G1", 12, "G4", 13)
	CONNECTION("G2", 4, "G4", 12)

	CONNECTION("G4", 3, "B3", 9)
	CONNECTION("G4", 11, "B3", 10)

	CONNECTION("B3", 8, "B3", 12)
	CONNECTION("B3", 8, "B3", 13)

	#define HIT   "B3", 8
	#define HIT_n "B3", 11


	// Horizontal Ball Control
	CONNECTION("E2", 8, "E2", 12)
	CONNECTION("C3", 6, "E2", 13)

	CONNECTION("E7", 6, "F2", 2)
	CONNECTION("E7", 6, "F2", 3)
	CONNECTION("E2", 11, "F2", 14)
	CONNECTION("F2", 12, "F2", 1)

	CONNECTION("F2", 8, "G2", 11)
	CONNECTION("F2", 11, "G2", 12)

	CONNECTION("F2", 8, "E2", 9)
	CONNECTION("F2", 11, "E2", 10)

	CONNECTION("G2", 13, "H2", 1)
	CONNECTION("G2", 13, "H2", 2)

	CONNECTION("H2", 3, "H2", 12)
	CONNECTION("E2", 8, "H2", 13)

	CONNECTION("H2", 11, "H2", 9)
	CONNECTION("E8", 6, "H2", 10)

	CONNECTION("H2", 3, "H2", 4)
	CONNECTION("E8", 6, "H2", 5)

	CONNECTION(H256_n, "G2", 8)
	CONNECTION("E8", 6, "G2", 9)

	CONNECTION(VCC, "H3", 8)
	CONNECTION("G2", 10, "H3", 9)
	CONNECTION("H5", 3, "H3", 11)
	CONNECTION("H2", 8, "H3", 10)

	CONNECTION("H3", 5, "H3", 1)
	CONNECTION("G2", 10, "H3", 12)
	CONNECTION(GND, "H3", 4)
	CONNECTION("H2", 6, "H3", 13)

	CONNECTION("H3", 5, "H5", 1)
	CONNECTION("H3", 3, "H5", 2)

	CONNECTION("G5", 3, "C2", 12)
	CONNECTION(ATTRACT, "C2", 13)

	CONNECTION("C2", 11, "D2", 1)

	CONNECTION("H4", 8, "H4", 12)
	CONNECTION("D2", 2, "H4", 11)
	CONNECTION("G4", 11, "H4", 13)
	CONNECTION("G4", 3, "H4", 10)

	CONNECTION("H5", 3, "H5", 12)
	CONNECTION("H4", 9, "H5", 13)

	CONNECTION("H5", 3, "H5", 4)
	CONNECTION("H4", 8, "H5", 5)

	CONNECTION("H5", 11, "H5", 10)
	CONNECTION("H5", 6, "H5", 9)



	// Vertical Ball Control

	CONNECTION("A7", 9, "B1", 6)
	CONNECTION("D1", 9, "B1", 5)
	CONNECTION("A9", 9, "B1", 4)
	CONNECTION("C1", 9, "B1", 3)
	CONNECTION(H64, "B1", 14)
	CONNECTION(H256, "B1", 2)
	CONNECTION(GND, "B1", 1)
	CONNECTION(GND, "B1", 15) // Added

	CONNECTION("A7", 8, "A1", 6)
	CONNECTION("D1", 8, "A1", 5)
	CONNECTION("A9", 8, "A1", 4)
	CONNECTION("C1", 8, "A1", 3)
	CONNECTION("A7", 11, "A1", 10)
	CONNECTION("D1", 11, "A1", 11)
	CONNECTION("A9", 11, "A1", 12)
	CONNECTION("C1", 11, "A1", 13)
	CONNECTION(H64, "A1", 14)
	CONNECTION(H256, "A1", 2)
	CONNECTION(GND, "A1", 1)
	CONNECTION(GND, "A1", 15)

	CONNECTION("A7", 11, "B7", 3)
	CONNECTION(H256_n, "B7", 2)
	CONNECTION("A9", 11, "B7", 4)
	CONNECTION(H256, "B7", 5)

	CONNECTION(ATTRACT, "D2", 5)

	CONNECTION("B1", 7, "A6", 12)
	CONNECTION(HIT, "A6", 11)
	CONNECTION("D2", 6, "A6", 13)
	CONNECTION(VCC, "A6", 10)

	CONNECTION("A1", 7, "A6", 2)
	CONNECTION(HIT, "A6", 3)
	CONNECTION("D2", 6, "A6", 1)
	CONNECTION(VCC, "A6", 4)

	CONNECTION("A1", 9, "B6", 2)
	CONNECTION(HIT, "B6", 3)
	CONNECTION("D2", 6, "B6", 1)
	CONNECTION(VCC, "B6", 4)

	CONNECTION(VVID, "D10", 1)
	CONNECTION("F6", 13, "D10", 12)
	CONNECTION(VVID, "D10", 4)
	CONNECTION("B3", 11, "D10", 13)

	CONNECTION("A6", 8, "A5", 9)
	CONNECTION("D10", 3, "A5", 10)

	CONNECTION("D10", 3, "A5", 4)
	CONNECTION("A6", 6, "A5", 5)

	CONNECTION("D10", 3, "B7", 1)
	CONNECTION("B6", 6, "B7", 13)
	CONNECTION("B6", 5, "B7", 10) // Pins 5&6 swapped compared to Pong
	CONNECTION("D10", 2, "B7", 9)

	CONNECTION("B7", 8, "C5", 1)
	CONNECTION("B7", 8, "C5", 2)
	CONNECTION("B7", 8, "C5", 13)

	CONNECTION("A5", 8, "B5", 10)
	CONNECTION("A5", 6, "B5", 8)
	CONNECTION("B7", 8, "B5", 3)
	CONNECTION(GND, "B5", 1)
	CONNECTION("C5", 12, "B5", 11)
	CONNECTION(VCC, "B5", 7)
	CONNECTION(VCC, "B5", 4)
	CONNECTION(GND, "B5", 16)
	CONNECTION(GND, "B5", 13)



	// Paddle 1 Vertical
	CONNECTION(V256_n, "B10", 2)
	CONNECTION(VCC, "B10", 4)

	CONNECTION("B10", 3, "C10", 3)

	CONNECTION(HSYNC_n, "B8", 4)
	CONNECTION("A8", 8, "B8", 5)

	CONNECTION("B10", 3, "A7", 2)
	CONNECTION("B10", 3, "A7", 3)
	CONNECTION("B8", 6, "A7", 14)
	CONNECTION("A7", 12, "A7", 1)

	CONNECTION("A7", 12, "A8", 10)
	CONNECTION("A7", 9, "A8", 9)
	CONNECTION("A7", 8, "A8", 12)
	CONNECTION("A7", 11, "A8", 13)

	CONNECTION("C10", 4, "B8", 1)
	CONNECTION("A8", 8, "B8", 2)

	// Paddle 1N Vertical
	CONNECTION(V256_n, "B9A", 2)
	CONNECTION(VCC, "B9A", 4)

	CONNECTION("B9A", 3, "G1", 1)

	CONNECTION(HSYNC_n, "E1", 1)
	CONNECTION("A2", 8, "E1", 2)

	CONNECTION("B9A", 3, "D1", 2)
	CONNECTION("B9A", 3, "D1", 3)
	CONNECTION("E1", 3, "D1", 14)
	CONNECTION("D1", 12, "D1", 1)

	CONNECTION("D1", 12, "A2", 10)
	CONNECTION("D1", 9, "A2", 9)
	CONNECTION("D1", 8, "A2", 12)
	CONNECTION("D1", 11, "A2", 13)

	CONNECTION("G1", 2, "E1", 12)
	CONNECTION("A2", 8, "E1", 13)



	// Paddle 2 Vertical
	CONNECTION(V256_n, "A10", 2)
	CONNECTION(VCC, "A10", 4)

	CONNECTION("A10", 3, "C10", 1)

	CONNECTION("A8", 6, "B8", 9)
	CONNECTION(HSYNC_n, "B8", 10)

	CONNECTION("A10", 3, "A9", 2)
	CONNECTION("A10", 3, "A9", 3)
	CONNECTION("B8", 8, "A9", 14)
	CONNECTION("A9", 12, "A9", 1)

	CONNECTION("A9", 12, "A8", 2)
	CONNECTION("A9", 9, "A8", 1)
	CONNECTION("A9", 8, "A8", 4)
	CONNECTION("A9", 11, "A8", 5)

	CONNECTION("C10", 2, "B8", 13)
	CONNECTION("A8", 6, "B8", 12)

	// Paddle 2N Vertical
	CONNECTION(V256_n, "B9B", 2)
	CONNECTION(VCC, "B9B", 4)

	CONNECTION("B9B", 3, "G1", 3)

	CONNECTION(HSYNC_n, "E1", 4)
	CONNECTION("A2", 6, "E1", 5)

	CONNECTION("B9B", 3, "C1", 2)
	CONNECTION("B9B", 3, "C1", 3)
	CONNECTION("E1", 6, "C1", 14)
	CONNECTION("C1", 12, "C1", 1)

	CONNECTION("C1", 12, "A2", 5)
	CONNECTION("C1", 9, "A2", 4)
	CONNECTION("C1", 8, "A2", 2)
	CONNECTION("C1", 11, "A2", 1)

	CONNECTION("G1", 4, "E1", 9)
	CONNECTION("A2", 6, "E1", 10)


	// Paddle N Horizontal
	CONNECTION(H64, "G1", 5)

	CONNECTION(VCC, "J1", 4)
	CONNECTION("G1", 6, "J1", 2)
	CONNECTION(H4, "J1", 3)
	CONNECTION(ATTRACT_n, "J1", 1)

	CONNECTION("C3", 9, "J1", 10)
	CONNECTION(VCC, "J1", 12)
	CONNECTION("G4", 8, "J1", 11)
	CONNECTION(H256, "J1", 13)

	CONNECTION("J1", 9, "F1", 4)
	CONNECTION("G1", 6, "F1", 3)
	CONNECTION("J1", 6, "F1", 5)

	CONNECTION("F1", 6, "H1", 8)
	CONNECTION("F1", 6, "H1", 9)

	CONNECTION("H1", 10, "F1", 13)
	CONNECTION(H256, "F1", 2) // Schematic says 256V
	CONNECTION("E1", 11, "F1", 1)

	CONNECTION("H1", 10, "F1", 11)
	CONNECTION(H256_n, "F1", 9)
	CONNECTION("E1", 8, "F1", 10)


	// Paddle Horizontal
	CONNECTION(VCC, "H4", 1)
	CONNECTION("D2", 4, "H4", 4)
	CONNECTION(H4, "H4", 3)
	CONNECTION(H128, "H4", 2)

	CONNECTION(H128, "G4", 10)
	CONNECTION("H4", 6, "G4", 9)

	CONNECTION("G4", 8, "G3", 9)
	CONNECTION(H256, "G3", 10)
	CONNECTION("B8", 3, "G3", 11)

	CONNECTION("B8", 11, "G3", 1)
	CONNECTION(H256_n, "G3", 2)
	CONNECTION("G4", 8, "G3", 13)

	CONNECTION("F1", 12, "H1", 2)
	CONNECTION("G3", 8, "H1", 3)

	CONNECTION("F1", 8, "H1", 5)
	CONNECTION("G3", 12, "H1", 6)

	CONNECTION("H1", 1, "G1", 13)
	CONNECTION("H1", 4, "G1", 11)


	// Sound
	CONNECTION("E7", 8, "G5", 2)
	CONNECTION(VCC, "G5", 4)

	CONNECTION("G5", 3, "C4", 10)
	CONNECTION(V32, "C4", 9)

	CONNECTION(VCC, "C3", 2)
	CONNECTION(VCC, "C3", 4)
	CONNECTION(HIT_n, "C3", 1)
	CONNECTION("A4", 15, "C3", 3)

	CONNECTION("C3", 6, "C4", 1)
	CONNECTION("A4", 14, "C4", 2)

	CONNECTION(VVID, "F4", 1)
	CONNECTION("F6", 13, "F4", 12)
	CONNECTION(VVID_n, "F4", 4)
	CONNECTION("B6", 9, "F4", 13)

	CONNECTION("F4", 3, "C4", 5)
	CONNECTION("A4", 13, "C4", 4)

	CONNECTION("C4", 6, "C5", 3)
	CONNECTION("C4", 3, "C5", 4)
	CONNECTION("C4", 8, "C5", 5)




	// Score Counters
	CONNECTION("H4", 9, "F6", 5)
	CONNECTION("E2", 3, "F6", 6)

	CONNECTION("H4", 8, "F6", 2)
	CONNECTION("E2", 3, "F6", 3)

	CONNECTION("F6", 4, "C8", 14)
	CONNECTION("C8", 12, "C8", 1)
	CONNECTION(SRST, "C8", 2)
	CONNECTION(SRST, "C8", 3)
	CONNECTION(GND, "C8", 6)
	CONNECTION(GND, "C8", 7)

	CONNECTION("F6", 1, "D8", 14)
	CONNECTION("D8", 12, "D8", 1)
	CONNECTION(SRST, "D8", 2)
	CONNECTION(SRST, "D8", 3)
	CONNECTION(GND, "D8", 6)
	CONNECTION(GND, "D8", 7)

	CONNECTION("C8", 11, "C9", 12)
	CONNECTION(VCC, "C9", 1)
	CONNECTION(VCC, "C9", 4)
	CONNECTION(SRST_n, "C9", 13)

	CONNECTION("D8", 11, "C9", 9)
	CONNECTION(VCC, "C9", 8)
	CONNECTION(VCC, "C9", 11)
	CONNECTION(SRST_n, "C9", 10)

	CONNECTION(VCC, "DIPSW1", 1)
	CONNECTION("C8", 8, "DIPSW1", 2)

	CONNECTION("C8", 12, "D9", 1)
	//CONNECTION("DIPSW1", 3, "D9", 2)
	CONNECTION("DIPSW1", Q, "D9", 2)
	CONNECTION("C9", 3, "D9", 13)

	//CONNECTION(VCC, "DIPSW1", 4)
	//CONNECTION("D8", 8, "DIPSW1", 5)
	CONNECTION(VCC, "DIPSW2", 1)
	CONNECTION("D8", 8, "DIPSW2", 2)

	CONNECTION("D8", 12, "D9", 3)
	//CONNECTION("DIPSW1", 6, "D9", 4)
	CONNECTION("DIPSW2", Q, "D9", 4)
	CONNECTION("C9", 5, "D9", 5)

	CONNECTION("D9", 12, "B3", 1)
	CONNECTION("D9", 6, "B3", 2)

	// Missing Logic in DICE

	NET_C(G6.12, A5.13)
	NET_C(B2.8, A5.12)
	NET_C(A5.11, E5.11)

	// Video Summing
	CONNECTION("F3", 13, "G2", 4)  // Ball
	CONNECTION("F3", 12, "G1", 10)
	CONNECTION("F3", 10, NET)
	CONNECTION("F3", 9, "G1", 12)

	CONNECTION("F3", 8, "E5", 13)

	// FIXME: pong doubles uses different resistor values!
	RES(RV1, 1000)
	RES(RV2, 1200)
	RES(RV3, 22000)
	NET_C(E5.12, RV1.1)     //Video
	NET_C(D4.8, RV2.1)      //Score
	NET_C(E5.10, RV3.1)

	NET_C(RV1.2, RV2.2)
	NET_C(RV2.2, RV3.2)

	ALIAS(videomix, RV3.2)

#if 0
	CONNECTION("VIDEO", 1, "E5", 12) // VIDEO
	CONNECTION("VIDEO", 2, "D4", 8)  // SCORE

	CONNECTION("VIDEO", Video::HBLANK_PIN, HBLANK)
	CONNECTION("VIDEO", Video::VBLANK_PIN, VBLANK)
#endif

	// Audio Summing
	CONNECTION(ATTRACT_n, "C2", 4)
	CONNECTION("C5", 6, "C2", 5)

	//CONNECTION("AUDIO", 1, "C2", 6)
	ALIAS(AUDIO, C2.6)

#ifdef DEBUG
	CONNECTION("LOG1", 1, "COIN", 1)
	CONNECTION("LOG1", 2, "A3", 3)
	CONNECTION("LOG1", 3, "B2", 5)
	CONNECTION("LOG1", 4, "H10", 5)
	CONNECTION("LOG1", 5, "C3", 9)
	CONNECTION("LOG1", 6, "G10", 5)
	CONNECTION("LOG1", 7, "E2", 6)
	CONNECTION("LOG1", 8, "B3", 3)
#endif


CIRCUIT_LAYOUT_END
