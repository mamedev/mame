// license:BSD-3-Clause
// copyright-holders:Aaron Giles

//
// Netlist for Sega Speech board
//
// Derived from the schematics in the Sega G-80 manual.
//
// Known problems/issues:
//
//    * Mostly works. Not 100% sure about using a current source
//       for input.
//

#include "netlist/devices/net_lib.h"
#include "nl_segaspeech.h"

//
// Optimizations
//




//
// Main netlist
//

NETLIST_START(segaspeech)

	SOLVER(Solver, 1000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 2e-5)

	TTL_INPUT(I_SP0250, 0)
	PARAM(I_SP0250.MODEL, "74XXOC")
	NET_C(I_SP0250.GND, GND)
	NET_C(I_SP0250.VCC, I_VSP)
	ALIAS(I_SPEECH, I_SP0250.Q)

	TTL_INPUT(I_CONTROL_D3, 0)
	NET_C(I_CONTROL_D3.GND, GND)
	NET_C(I_CONTROL_D3.VCC, I_V5)

	ANALOG_INPUT(I_VSP, 5)
	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_VM5, -5)

#if 1

	//
	// This represents the schematic drawing from Astro Blaster
	// and Space Fury; there is no control register and it
	// works.
	//
	RES(R17, RES_K(10))
	RES(R18, RES_K(22))
	RES(R19, RES_K(250))
	RES(R20, 470)
	RES(R21, RES_K(10))

	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_U(0.047))
	CAP(C50, CAP_U(0.003))

	TL081_DIP(U8)			// Op. Amp.
	NET_C(U8.7, I_V5)
	NET_C(U8.4, I_VM5)

	NET_C(I_SPEECH, R17.1, C9.1)
	NET_C(R17.2, I_V5)
	NET_C(C9.2, R18.1)
	NET_C(R18.2, C10.1, R19.2, U8.3)
	NET_C(R19.1, C10.2, GND)
	NET_C(R20.1, GND)
	NET_C(R20.2, U8.2, R21.2, C50.2)
	NET_C(U8.6, R21.1, C50.1)
	ALIAS(OUTPUT, U8.6)

#else

	//
	// This represents the schematic drawing from the G-80
	// manual, Star Trek, and Zektor. There is a lightly
	// used control register, but the wiring in the schematic
	// seems to be wrong and produces distorted results.
	//
	RES(R14, RES_K(22))
	RES(R17, RES_K(10))
	RES(R18, RES_K(22))
	RES(R19, RES_K(270))
	RES(R20, RES_K(4.7))
	RES(R21, RES_K(10))
	RES(R22, RES_K(22))
	RES(R30, RES_K(68))

	CAP(C2, CAP_U(4.7))
	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_U(0.047))
	CAP(C50, CAP_U(0.003))

	TL081_DIP(U8)			// Op. Amp.
	NET_C(U8.7, I_V5)
	NET_C(U8.4, I_VM5)

	TL082_DIP(U11)			// Op. Amp.
	NET_C(U11.8, I_V5)
	NET_C(U11.4, I_VM5)

	CD4053_DIP(U12)			// 3x analog demuxer
	NET_C(U12.16, I_V5)
	NET_C(U12.6, GND)		// INH
	NET_C(U12.7, I_VM5)		// VEE
	NET_C(U12.8, GND)

	NET_C(I_SPEECH, R17.2, C9.1)
	NET_C(R17.1, I_V5)
	NET_C(C9.2, R18.1)
	NET_C(R18.2, R19.2, C10.1, U8.2)
	NET_C(R19.1, C10.2, GND, R20.1, R21.1, C50.1)
	NET_C(R20.2, U8.3)
	NET_C(U8.6, R21.2, C50.2, R22.1)

	NET_C(U12.14, R22.2)
	NET_C(U12.11, I_CONTROL_D3)
	NET_C(U12.13, R14.1)
	NET_C(R14.2, R30.1, U11.2)
	NET_C(U11.3, GND)
	NET_C(U11.1, C2.2, R30.2)
	ALIAS(OUTPUT, C2.1)

	RES(ROUT, RES_M(1))
	NET_C(C2.1, ROUT.1)
	NET_C(ROUT.2, GND)

	//
	// Unconnected inputs
	//

	NET_C(GND, U11.5, U11.6)
	NET_C(GND, U12.1, U12.2, U12.3, U12.5, U12.9, U12.10, U12.12)
#endif

NETLIST_END()
