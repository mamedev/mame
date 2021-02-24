// license:CC0
// copyright-holders: Ivan Vangelista
//
// Netlist for Atari's Night Driver
//
// Derived from the schematics in the manual.
//
// TODO:
// * Everything should be triple checked
// * Explore using frontiers
// * All sounds play at start-up

#include "netlist/devices/net_lib.h"

NETLIST_START(nitedrvr)

	SOLVER(Solver, 48000)
	ANALOG_INPUT(V11, 11)
	ANALOG_INPUT(V5, 5)
	ALIAS(VCC, V5)
	CLOCK(V4, 15750.0 / 2 / 4)
	NET_C(VCC, V4.VCC)
	NET_C(GND, V4.GND)

	TTL_INPUT(SPEED1, 1)
	TTL_INPUT(SPEED2, 1)
	TTL_INPUT(SPEED3, 1)
	TTL_INPUT(SPEED4, 1)
	TTL_INPUT(SKID1, 1)
	TTL_INPUT(SKID2, 1)
	TTL_INPUT(CRASH, 1)
	TTL_INPUT(ATTRACT, 1)
	NET_C(VCC, SPEED1.VCC, SPEED2.VCC, SPEED3.VCC, SPEED4.VCC, SKID1.VCC, SKID2.VCC, CRASH.VCC, ATTRACT.VCC)
	NET_C(GND, SPEED1.GND, SPEED2.GND, SPEED3.GND, SPEED4.GND, SKID1.GND, SKID2.GND, CRASH.GND, ATTRACT.GND)

	RES(R8,   RES_K(220)) // Resistor, Carbon Comp, 1/4w, 5%, 220K ohm
	RES(R9,   RES_K(470)) // Resistor, Carbon Comp, 1/4w, 5%, 470K ohm
	RES(R10,  RES_M(1))   // Resistor, Carbon Comp, 1/4w, 5%, 1Megohm
	RES(R11,  RES_M(2.2)) // Resistor, Carbon Comp, 1/4w, 5%, 2.2Megohm
	RES(R12,  RES_K(100)) // Resistor, Carbon Comp, 1/4w, 5%, 100K ohm
	RES(R13,  RES_K(220)) // Resistor, Carbon Comp, 1/4w, 5%, 220K ohm
	POT(R14,  RES_K(100)) // Potentiometer, 100K
	RES(R15,  RES_K(6.8)) // Resistor, Carbon Comp, 1/4w, 5%, 6.8K ohm
	RES(R16,  100)        // Resistor, Carbon Comp, 1/4w, 5%, 100 ohm
	RES(R17,  RES_K(18))  // Resistor, Carbon Comp, 1/4w, 5%, 18K ohm
	RES(R42,  100)        // Resistor, Carbon Comp, 1/4w, 5%, 100 ohm
	RES(R43,  RES_K(9.1)) // Resistor, Carbon Comp, 1/4w, 5%, 9.1K ohm
	RES(R44,  RES_K(10))  // Resistor, Carbon Comp, 1/4w, 5%, 10K ohm
	RES(R45,  RES_K(3.3)) // Resistor, Carbon Comp, 1/4w, 5%, 3.3K ohm
	RES(R46,  RES_K(8.2)) // Resistor, Carbon Comp, 1/4w, 5%, 8.2K ohm
	RES(R53,  RES_K(1))   // Resistor, Carbon Comp, 1/4w, 5%, 1K ohm
	RES(R54,  RES_K(2.2)) // Resistor, Carbon Comp, 1/4w, 5%, 2.2K ohm
	RES(R55,  RES_K(8.2)) // Resistor, Carbon Comp, 1/4w, 5%, 8.2K ohm
	RES(R56,  RES_K(3.9)) // Resistor, Carbon Comp, 1/4w, 5%, 3.9K ohm
	RES(R58,  RES_K(47))  // Resistor, Carbon Comp, 1/4w, 5%, 47K ohm
	RES(R59,  RES_K(47))  // Resistor, Carbon Comp, 1/4w, 5%, 47K ohm
	RES(R60,  RES_K(10))  // Resistor, Carbon Comp, 1/4w, 5%, 10K ohm
	RES(R61,  RES_K(6.8)) // Resistor, Carbon Comp, 1/4w, 5%, 6.8K ohm
	RES(R105, RES_K(2.2)) // Resistor, Carbon Comp, 1/4w, 5%, 2.2K ohm
	RES(R106, RES_K(2.2)) // Resistor, Carbon Comp, 1/4w, 5%, 2.2K ohm
	RES(R107, 390)        // Resistor, Carbon Comp, 1/4w, 5%, 390 ohm - Missing from parts list, 390 ohm on schematics
	RES(R108, 270)        // Resistor, Carbon Comp, 1/4w, 5%, 270 ohm
	RES(R109, 330)        // Resistor, Carbon Comp, 1/4w, 5%, 330 ohm
	RES(R110, RES_K(180)) // Resistor, Carbon Comp, 1/4w, 5%, 180K ohm
	POT(R111, RES_K(5))   // Potentiometer, 100K

	CAP(C3,  CAP_U(2.2))   // Capacitor, Electrolytic, 25V, 2.2uf
	CAP(C4,  CAP_U(1))     // Capacitor, Tantalum, 35V, 1uf
	CAP(C5,  CAP_U(0.001)) // Capacitor, Ceramic Disc, 25V, .001uf
	CAP(C6,  CAP_U(0.22))  // Capacitor, Mylar, 100V, .22uf
	CAP(C7,  CAP_U(0.001)) // Capacitor, Ceramic Disc, 25V, .001uf
	CAP(C8,  CAP_U(47))    // Capacitor, Electrolytic, 25V, 47uf
	CAP(C9,  CAP_U(0.001)) // Capacitor, Ceramic Disc, 25V, .001uf
	CAP(C10, CAP_U(10))    // Capacitor, Electrolytic, 25V, 10uf
	CAP(C11, CAP_U(0.22))  // Capacitor, Mylar, 100V, .22uf
	CAP(C12, CAP_U(0.33))  // Capacitor, Ceramic Disc, 25V, .33uf
	CAP(C15, CAP_U(2.2))   // Capacitor, Electrolytic, 25V, 2.2uf
	CAP(C16, CAP_U(2.2))   // Capacitor, Electrolytic, 25V, 2.2uf
	CAP(C17, CAP_U(1))     // Capacitor, Electrolytic, 25V, 1uf
	CAP(C18, CAP_U(0.01))  // Capacitor, Ceramic Disc, 25V, .01uf

	NE566_DIP(IC_A5)     // I.C. 566
	NE566_DIP(IC_A6)     // I.C. 566
	NE566_DIP(IC_A7)     // I.C. 566
	LM324_DIP(IC_B5)     // I.C. LM324
	MC3340_DIP(IC_B6)    // I.C. MFC3040
	TTL_7408_DIP(IC_B8)  // I.C. 74083
	TTL_7486_DIP(IC_C6)  // I.C. 7486
	TTL_7400_DIP(IC_C7)  // I.C. 7400
	TTL_74193_DIP(IC_C8) // I.C. 7419
	NE555_DIP(IC_C9)     // I.C. 555
	TTL_74164_DIP(IC_D6) // I.C. 74164
	TTL_74164_DIP(IC_D7) // I.C. 74164
	TTL_74107_DIP(IC_D8) // I.C. 74107
	TTL_7404_DIP(IC_E4)  // I.C. 7404
	TTL_7414_DIP(IC_E9)  // I.C. 7414
	TTL_7406_DIP(IC_N9)  // Missing from parts list, 06 on schematics

	NET_C(V11, IC_A5.8, IC_A6.8, IC_A7.8, IC_B5.4, IC_B6.8)
	NET_C(VCC, IC_B8.14, IC_C6.14, IC_C7.14, IC_C8.16, IC_C9.4, IC_C9.8, IC_D6.14, IC_D7.14, IC_D8.14, IC_E4.14, IC_E9.14, IC_N9.14) // IC_C9.4 -> P?
	NET_C(GND, IC_A5.1, IC_A6.1, IC_A7.1, IC_B5.11, IC_B6.3, IC_B8.7, IC_C6.7, IC_C7.7, IC_C8.8, IC_C9.1, IC_D6.7, IC_D7.7, IC_D8.7, IC_E4.7, IC_E9.7, IC_N9.7)
	NET_C(GND, IC_B5.5, IC_B5.6, IC_B5.9, IC_B5.10, IC_B5.12, IC_B5.13, IC_C6.4, IC_C6.5, IC_C6.9, IC_C6.10, IC_C6.12, IC_C6.13, IC_C7.4, IC_C7.5, IC_C7.9, IC_C7.10, IC_D8.8, IC_D8.9, IC_D8.10, IC_D8.11,
		  IC_E4.1, IC_E4.3, IC_E4.5, IC_E4.9, IC_E4.11, IC_E9.2, IC_E9.8,  IC_E9.10, IC_E9.12, IC_N9.1, IC_N9.3, IC_N9.9, IC_N9.11, IC_N9.13) // not connected terminals (GND for now, investigate)

	// Attract
	NET_C(ATTRACT.Q, IC_N9.5)

	// Motor Sound
	NET_C(SPEED1.Q, R11.1)
	NET_C(SPEED2.Q, R10.1)
	NET_C(SPEED3.Q, R9.1)
	NET_C(SPEED4.Q, R8.1)
	NET_C(R11.2, R10.2, R9.2, R8.2, C3.1, IC_B5.3)
	NET_C(C3.2, GND)

	NET_C(IC_B5.1, R13.1, R14.1)
	NET_C(R13.2, R12.1, IC_B5.2)
	NET_C(R12.2, GND)
	PARAM(R14.REVERSE, 1)

	NET_C(R14.2, R17.1, R14.3, IC_N9.6, C9.1, IC_A7.5, C5.1, IC_A5.5, C7.1, IC_A6.5)
	NET_C(R17.2, V11)

	NET_C(C5.2, IC_A5.6, R15.1)
	NET_C(R15.2, V11)
	NET_C(IC_A5.7, C4.1)
	NET_C(C4.2, GND)
	NET_C(IC_A5.4, R16.1)
	NET_C(R16.2, C8.1, R42.2)
	NET_C(C8.2, IC_B6.1)

	NET_C(C7.2, IC_A6.6, R43.1)
	NET_C(R43.2, V11)
	NET_C(IC_A6.7, C6.1)
	NET_C(C6.2, GND)
	NET_C(IC_A6.4, R42.1)

	NET_C(C9.2, IC_A7.6, R46.1)
	NET_C(R46.2, V11)
	NET_C(IC_A7.7, C11.1)
	NET_C(C11.2, GND)
	NET_C(IC_A7.4, C10.1)
	NET_C(C10.2, R44.1, R45.1, IC_B6.2)
	NET_C(R44.2, V11)
	NET_C(R45.2, GND)

	NET_C(IC_B6.7, R60.1)

	// Crash
	NET_C(CRASH.Q, IC_D6.9, IC_C8.11, IC_D7.9, IC_D8.13)

	NET_C(V4.Q, IC_D6.8, IC_D7.8)

	NET_C(IC_D6.3, IC_C6.2)
	NET_C(IC_D6.13, IC_D7.2, IC_D7.1)

	NET_C(IC_D7.12, IC_C6.1)
	NET_C(IC_D7.13, R105.1, R106.1, IC_B8.10, IC_B8.4, IC_B8.1, IC_B8.13)

	NET_C(IC_C6.3, IC_E4.13)
	NET_C(IC_E4.12, IC_D6.2, IC_D6.1)

	NET_C(V5, IC_C8.5, IC_C8.9, IC_C8.10, IC_C8.1, IC_C8.15) // P?
	NET_C(IC_C8.4, IC_C9.3)
	NET_C(IC_C8.3, IC_B8.12)
	NET_C(IC_C8.2, IC_B8.2)
	NET_C(IC_C8.6, IC_B8.5)
	NET_C(IC_C8.7, IC_B8.9)
	NET_C(IC_C8.13, IC_D8.12)

	NET_C(IC_C9.5, C18.1)
	NET_C(C18.2, GND)
	NET_C(IC_C9.2, IC_C9.6, C17.1, R109.1)
	NET_C(C17.2, GND)
	NET_C(R109.2, IC_C9.7, R110.1)
	NET_C(R110.2, V5)

	NET_C(IC_D8.1, V5) // P?
	NET_C(IC_D8.4, GND)
	NET_C(IC_D8.3, IC_C8.14)

	// Digital Noise
	NET_C(IC_B8.11, R55.1)
	NET_C(IC_B8.3, R56.1)
	NET_C(IC_B8.6, R54.1)
	NET_C(IC_B8.8, R53.1)
	NET_C(R55.2, R56.2, R54.2, R53.2, C12.1, R61.1)
	NET_C(C12.2, GND)

	NET_C(R106.2, C15.1, R108.1, IC_E9.3)
	NET_C(C15.2, GND)
	NET_C(IC_E9.4, R108.2, IC_C7.12)
	NET_C(SKID1.Q, IC_C7.13)
	NET_C(IC_C7.11, R59.1)

	NET_C(R105.2, C16.1, R107.1, IC_E9.5)
	NET_C(C16.2, GND)
	NET_C(IC_E9.6, R107.2, IC_C7.2)
	NET_C(SKID2.Q, IC_C7.1)
	NET_C(IC_C7.3, R58.1)

	// Mixer
	NET_C(R61.2, R60.2, R59.2, R58.2, R111.1)
	NET_C(R111.3, GND)
	PARAM(R111.REVERSE, 1)
	ALIAS(OUTPUT, R111.2)

	// Unconnected pins
	HINT(IC_C6.6, NC)
	HINT(IC_C6.8, NC)
	HINT(IC_C6.11, NC)
	HINT(IC_C7.6, NC)
	HINT(IC_C7.8, NC)
	HINT(IC_C8.12, NC)
	HINT(IC_D6.4, NC)
	HINT(IC_D6.5, NC)
	HINT(IC_D6.6, NC)
	HINT(IC_D6.10, NC)
	HINT(IC_D6.11, NC)
	HINT(IC_D6.12, NC)
	HINT(IC_D7.3, NC)
	HINT(IC_D7.4, NC)
	HINT(IC_D7.5, NC)
	HINT(IC_D7.6, NC)
	HINT(IC_D7.10, NC)
	HINT(IC_D7.11, NC)
	HINT(IC_D8.2, NC)
	HINT(IC_D8.5, NC)
	HINT(IC_D8.6, NC)
	HINT(IC_E4.2, NC)
	HINT(IC_E4.4, NC)
	HINT(IC_E4.6, NC)
	HINT(IC_E4.8, NC)
	HINT(IC_E4.10, NC)
	HINT(IC_N9.2, NC)
	HINT(IC_N9.4, NC)
	HINT(IC_N9.8, NC)
	HINT(IC_N9.10, NC)
	HINT(IC_N9.12, NC)
NETLIST_END()
