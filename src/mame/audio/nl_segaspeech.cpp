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

#if (USE_CURRENT_SOURCE)
	CS(I_SP0250, 0) // Fed through stream ...
	NET_C(I_SP0250.1, I_V5)
#else
	ANALOG_INPUT(I_SP0250, 0)
#endif

	TTL_INPUT(I_CONTROL_D3, 0)
	NET_C(I_CONTROL_D3.GND, GND)
	NET_C(I_CONTROL_D3.VCC, I_V5)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_VM5, -5)

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

#if (USE_CURRENT_SOURCE)
	NET_C(I_SP0250.2, R17.2, C9.1)
#else
	NET_C(I_SP0250, R17.2, C9.1)
#endif
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
NETLIST_END()
