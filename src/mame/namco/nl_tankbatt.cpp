// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

//
// Netlist for Tank Battalion
//
// Derived from the schematics in the manual.
//
// Known problems/issues:
//
//    * None.
//

#include "netlist/devices/net_lib.h"

//
// Main netlist
//

NETLIST_START(tankbatt)
{

	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5)

	CLOCK(2V, 4000) // 18.432MHz / (3 * 384 * 2)
	CLOCK(4V, 2000) // 18.432MHz / (3 * 384 * 4)

	TTL_INPUT(S1, 0)        // active high
	TTL_INPUT(S2, 0)        // active high
	TTL_INPUT(OFF, 1)       // active high
	TTL_INPUT(ENGINE_HI, 0) // active high
	TTL_INPUT(SHOOT, 0)     // active high
	TTL_INPUT(HIT, 0)       // active high

	RES(R31, 470)
	RES(R32, 470)
	RES(R35, RES_K(4.7))
	RES(R36, RES_K(4.7))
	RES(R37, RES_K(6.8))
	RES(R38, RES_K(10))
	//RES(R41, RES_K(22))
	RES(R42, RES_K(22))
	RES(R43, RES_K(22))
	RES(R44, RES_K(33))
	RES(R45, RES_K(47))
	RES(R46, RES_K(4.7)) // Possible schematic or scan-quality error: Schematic says 47K
	RES(R47, RES_K(4.7))
	RES(R48, RES_K(4.7))
	RES(R49, RES_K(4.7)) // Possible schematic or scan-quality error: Schematic says 47K
	RES(R50, RES_K(150))
	RES(R51, RES_K(150))
	RES(R52, RES_K(220))
	RES(R53, RES_K(470))
	RES(R54, RES_K(470))
	RES(R56_1, RES_K(1))
	RES(R56_2, RES_K(1))
	RES(R57, RES_K(10))
	RES(R58, RES_K(1))
	RES(R59, RES_K(1))
	RES(R60, RES_K(1))
	RES(R61, RES_K(1))

	CAP(C10, CAP_U(2.2))
	CAP(C11, CAP_U(2.2))
	CAP(C12, CAP_U(2.2))
	CAP(C13, CAP_U(2.2))
	CAP(C14, CAP_U(0.1))
	CAP(C15, CAP_U(0.01))
	CAP(C16, CAP_U(0.01))
	CAP(C17, CAP_U(0.01))
	CAP(C18, CAP_U(0.01))
	CAP(C42, CAP_U(0.1))

	DIODE(D8, "D") // Generic diodes, types not listed on schematic
	DIODE(D9, "D")
	DIODE(D10, "D")
	DIODE(D11, "D")

	CD4006(_6F)
	CD4066_DIP(_6L)
	LM324_DIP(_6J)
	TTL_7486_DIP(_6K)
	TTL_7492_DIP(_5K)

	NET_C(VCC, _5K.5, _6F.VDD, _6J.4, _6K.7, _6L.14, S1.VCC, S2.VCC, OFF.VCC, ENGINE_HI.VCC, SHOOT.VCC, HIT.VCC, 2V.VCC, 4V.VCC)
	NET_C(GND, _5K.10, _6F.VSS, _6J.11, _6K.14, _6L.7, S1.GND, S2.GND, OFF.GND, ENGINE_HI.GND, SHOOT.GND, HIT.GND, 2V.GND, 4V.GND)

	// Noise generation (presumably)
	NET_C(_6F.CLOCK, 2V)
	NET_C(_6F.D1P4, _6F.D3)
	NET_C(_6F.D3P4, _6F.D4, _6L.12, _6L.6)
	NET_C(_6F.D1, _6F.D2P5, _6K.A.A)
	NET_C(_6F.D4P4, _6K.A.B)
	NET_C(_6K.A.Q, R38.1)
	NET_C(_6F.D2, R38.2, D9.K)
	NET_C(_6F.D4P5, D8.A, R53.1)
	NET_C(D8.K, R53.2, C42.1, _6J.2)
	NET_C(_6J.1, C42.2, D9.A)
	NET_C(_6J.3, C12.1, R43.1, R44.1, _6J.12, _6J.10)
	NET_C(C12.2, GND)
	NET_C(R43.2, GND)
	NET_C(R44.2, V5)

	// S1
	NET_C(S1.Q, _6L.5)
	NET_C(4V, _6L.4)
	NET_C(_6L.3, R35.1)

	// S2
	NET_C(S2.Q, _6L.13)
	NET_C(2V, _6L.1)
	NET_C(_6L.2, R36.1)

	// Hit
	NET_C(HIT.Q, R31.1)
	NET_C(R31.2, D10.A)
	NET_C(D10.K, C10.1, _6L.8)
	NET_C(C10.2, GND)
	NET_C(_6L.9, R50.1)
	NET_C(R50.2, R42.1, C15.1, C16.1)
	NET_C(R42.2, GND)
	NET_C(C16.2, R54.1, _6J.9)
	NET_C(C15.2, R54.2, _6J.8, R56_1.1)

	// Shoot
	NET_C(SHOOT.Q, R32.1)
	NET_C(R32.2, D11.A)
	NET_C(D11.K, C11.1, _6L.11)
	NET_C(C11.2, GND)
	NET_C(_6L.10, R45.1)
	NET_C(R45.2, R37.1, C17.1, C18.1)
	NET_C(R37.2, GND)
	NET_C(C18.2, R51.1, _6J.13)
	NET_C(C17.2, R51.2, _6J.14, R56_2.1)

	// Engine Rumble
	NET_C(ENGINE_HI.Q, _6K.B.A)
	NET_C(_6K.B.B, _6K.C.Q)
	NET_C(_6K.B.Q, R52.1, _5K.1)
	NET_C(R52.2, R46.1, _6J.6, C14.1)
	NET_C(C14.2, GND)
	NET_C(_6J.5, R47.1, R48.1, R49.1)
	NET_C(R47.2, V5)
	NET_C(R48.2, GND)
	NET_C(R49.2, R57.1, _6J.7, R46.2, _6K.C.A)
	NET_C(R57.2, GND)
	NET_C(_6K.C.B, VCC)
	NET_C(_5K.7, VCC)
	NET_C(_5K.6, OFF.Q)
	NET_C(_5K.11, R60.1, _6K.D.A)
	NET_C(_5K.8, R58.1, _6K.D.B)
	NET_C(_6K.D.Q, _5K.14)
	NET_C(_5K.12, R61.1)
	NET_C(R58.2, R60.2, R61.2, C13.1, R59.1)
	NET_C(C13.2, GND)

	// Mixing
	//NET_C(R56_1.2, R56_2.2)
	//NET_C(R35.2, R36.2)
	//NET_C(R56_2.2, R59.2)
	NET_C(R35.2, R36.2, R56_1.2, R56_2.2, R59.2)
	//NET_C(R41.2, GND)
}
