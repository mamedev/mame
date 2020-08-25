// license:CC0
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
	NET_C(I_SP0250.VCC, I_V5)
	ALIAS(I_SPEECH, I_SP0250.Q)

	ANALOG_INPUT(I_V5, 5)
	ANALOG_INPUT(I_VM5, -5)

	//
	// There are two schematic drawings of the speech board
	// that show fairly different outputs from the SP0250.
	// Both have their problems.
	//
	// The simpler one is included in the Astro Blaster and
	// Space Fury manuals. This is largely correct, except
	// that it is believed (not verified) that R20 should be
	// 4.7K instead of 470 Ohms. This schematic does not show
	// the CD4053 mixer.
	//
	// The more complex schematic is included in the G-80
	// schematics package, and in the Star Trek and Zektor
	// manuals. It has several significant errors (all verified
	// from a working PCB):
	//
	//    1. U8 pins 2 and 3 are swapped
	//    2. The connection from R20 to GND should be removed
	//        (this also disconnected C50 and R21 from GND)
	//    3. R13 should be 220k, not 22k
	//
	// With the fixes above, the output sections of the two
	// schematics line up, minus the mixing section, which is
	// only shown on the more complex schematic.
	//
	// The mixing section is trivial, with 3 bits from a control
	// port controlling three switches in the CD4053. Bit 3
	// enables/disables speech. Bit 4 enables/disables an
	// unconnected source. And bit 5 enables/disables incoming
	// external sound. The incoming sound is also routed through
	// a pot to enable control over the relative volume.
	//
	// For purposes of this netlist, and since it runs at high
	// speeds, we tap the speech output before it hits the
	// CD4053 and manually manage the speech output.
	//
	// Since we use MAME to manage the mixing, the control bits
	// are managed directly there, rather than in this netlist.
	//

	//
	// This represents the schematic drawing from Astro Blaster
	// and Space Fury; there is no control register and it
	// works.
	//
	RES(R17, RES_K(10))
	RES(R18, RES_K(22))
	RES(R19, RES_K(250))
	RES(R20, RES_K(4.7))    // schematic shows 470Ohm, but a real PCB had 4.7k here
	RES(R21, RES_K(10))

	CAP(C9, CAP_U(0.1))
	CAP(C10, CAP_U(0.047))
	CAP(C50, CAP_U(0.003))

	TL081_DIP(U8)           // Op. Amp.
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

	//
	// In the more complex case, this would feed into a
	// CD4053 for switching. We rely on this being managed
	// at the driver level.
	//

NETLIST_END()
