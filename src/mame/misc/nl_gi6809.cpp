// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

/***********************************************

  General Instruments dual 6809 hardware
  Discrete sound system.

***********************************************/

#include "netlist/devices/net_lib.h"

NETLIST_START(gi6809)
{
	SOLVER(Solver, 48000)
	ANALOG_INPUT(V12, 12)
	ANALOG_INPUT(V5, 5)

	ALIAS(VCC, V5)
	ALIAS(VDD, V12)

	TTL_INPUT(PA0, 0)  // -> r5
	TTL_INPUT(PA1, 0)  // -> r6 r8
	TTL_INPUT(PA2, 0)  // -> r7 r9
	TTL_INPUT(PA3, 0)  // -> inversor -> reset (active low)

	NET_C(VCC, PA0.VCC, PA1.VCC, PA2.VCC, PA3.VCC)
	NET_C(GND, PA0.GND, PA1.GND, PA2.GND, PA3.GND)

	// 7406 to power
	TTL_7406_GATE(U1_F)
	NET_C(VCC, U1_F.VCC)
	NET_C(GND, U1_F.GND)

	// ne555 to power
	NE555(U2)
	NET_C(GND, U2.GND)
	NET_C(VCC, U2.VCC)


	// Discrete components
	CAP(C1, CAP_U(4.7))
	CAP(C2, CAP_U(1))
	CAP(C3, CAP_U(0.1))

	RES(R1, RES_K(20))
	RES(R2, RES_K(2.2))
	RES(R3, RES_K(1))
	RES(R4, RES_K(10))
	RES(R5, RES_K(33))
	RES(R6, RES_K(18))
	RES(R7, RES_K(10))
	RES(R8, RES_K(47))
	RES(R9, RES_K(220))

	// Inputs pullups
	NET_C(VDD, R8.1)
	NET_C(VDD, R9.1)

	// inputs to resnet -> 555 control
	NET_C(PA0, R5.1)  // PA0 input to resnet

	NET_C(PA1, R8.2)  // pullup
	NET_C(PA1, R6.1)  // PA1 input to resnet

	NET_C(PA2, R9.2)  // pullup
	NET_C(PA2, R7.1)  // PA2 input to resnet

	//resnet r5,r6,r7
	NET_C(U2.CONT, R5.2) //resnet -> 555 control
	NET_C(U2.CONT, R6.2) //resnet -> 555 control
	NET_C(U2.CONT, R7.2) //resnet -> 555 control

	NET_C(U2.CONT, C1.1)
	NET_C(GND, C1.2)

	//wiring 555
	NET_C(R3.1, U2.VCC)
	NET_C(R3.2, U2.DISCH)

	NET_C(R4.1, U2.DISCH)
	NET_C(R4.2, U2.TRIG)
	NET_C(R4.2, U2.THRESH)

	NET_C(U2.TRIG, C3.1)
	NET_C(C3.2, GND)

	// PA3 input - > cd4069 -> ne555-reset
	NET_C(PA3, U1_F.A)
	NET_C(U1_F.Y, U2.RESET)

	// Ne555 - Output net
	NET_C(U2.OUT, R1.1)
	NET_C(R1.2, C2.1)
	NET_C(C2.2, R2.1)
	NET_C(GND, R2.2)

	ALIAS(OUTPUT, R2.1)
}
