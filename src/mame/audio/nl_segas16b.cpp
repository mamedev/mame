// license:CC0
// copyright-holders:Couriersud
#include "netlist/devices/net_lib.h"

//NL_CONTAINS segas16b_audio

NETLIST_START(segas16b_audio)
	SOLVER(Solver, 48000)
	//PARAM(Solver.DYNAMIC_TS, 1)

	ANALOG_INPUT(VP5, 5)
	ANALOG_INPUT(VP12, 12)

	/* UPD7759 has resistor of 47K on reference pin
	 * ==> IREF ~30 uA
	 * According to datasheet maximum current sink
	 * is 34 * IREF.
	 *
	 * MAME must provide range of 0 to 1020 uA to the current sink.
	 *
	 */
#if 0
	CLOCK(T0, 10)
	NET_C(GND, T0.GND)
	NET_C(VP5, T0.VCC)
	RES(T0R, RES_M(10000000))
	NET_C(T0, T0R.1)

	CS(T1, 0.0)
	//PARAM(T1.FUNC, "0.001020 * (0.5 + 0.5 * sin(6280 * T))")
	RES(T2, 1000)
	NET_C(GND, T2.2, T1.2)
	NET_C(T1.1, T2.1, T0R.2)
#endif

	CS(SPEECH, 0)
	//PARAM(SPEECH.FUNC, "0.001020 * (0.5 + 0.5 * sin(6280 * T))")
	NET_C(SPEECH.2, GND)

	ANALOG_INPUT(CH1, 0)
	ANALOG_INPUT(CH2, 0)

#if 0
	VS(CH1VS, 0)
	PARAM(CH1VS.FUNC, "2.5 + 1.25 * sin(6280 * T)")
	NET_C(CH1VS.2, GND)
	ALIAS(CH2, CH1VS.1)
#endif

	/* The YM3012 uses only one dac for the two channels.
	 * Sample-and-hold is used to two toggle between the two channels.
	 * This is not emulated here since the YM2151 signals are provided
	 * externally.
	 *
	 * Therefore two RCOM resistors are used for now directly connected
	 * to the channels.
	 *
	 * The YM3012 datasheet gives a formula for digital to VOUT:
	 * VOUT = 0.5 VDD + 0.25 * VDD *(-1 + D9 + d8/2 .. + d0/512 + 1/1024)/pow(2,N)
	 * N= S2Q*4+S1Q*2+S0Q
	 * 3 bits ignored (I0,I1,I2)
	 * Serial data stream: I0,I1,I2,D0,..., D9,S0,S1,S2
	 *
	 * Basically output is between 0.25 * VDD and 0.75 * VDD.
	 * This needs to be done in MAME interface
	 */

	TL084_DIP(D20)
	LM324_DIP(XC20)

	RES(RCOM1, 390)
	RES(RCOM2, 390)

	CAP(C21, CAP_U(2.2))
	CAP(C22, CAP_U(2.2)) // Needs verification
	CAP(C29, CAP_P(1500))
	CAP(C28, CAP_P(1500))

	// ------------------------------------
	// YM2151/YM3012 inputs
	// ------------------------------------

	NET_C(CH1, RCOM1.1)
	NET_C(RCOM1.2, C29.1, D20.5)
	NET_C(D20.7, D20.6, C22.1)

	NET_C(CH2, RCOM2.1)
	NET_C(RCOM2.2, C28.1, D20.10)
	NET_C(D20.8, D20.9, C21.1)

	NET_C(GND, C29.2, C28.2)

	// OPAMPS, no information on schematics, assume 12V since
	// TL084 with 5V would be outside specifications
	// No negative voltages found on schematics

	NET_C(VP12, XC20.4, D20.4)
	NET_C(GND, XC20.11, D20.11)

	// ------------------------------------
	// Speech inputs
	// ------------------------------------

	RES(R11, RES_K(1))
	RES(R14, RES_K(5.6))
	RES(R16, RES_K(6.8))
	RES(R15, RES_K(12))
	RES(R12, RES_K(1.3))
	RES(R13, RES_K(6.2))
	CAP(C25, CAP_U(0.0022))
	CAP(C26, CAP_U(0.022))
	CAP(C23, CAP_U(0.022))
	CAP(C24, CAP_U(0.01))
	CAP(C62, CAP_U(2.2))

	NET_C(VP5, R11.1)
	NET_C(SPEECH.1, R11.2, R14.1)
	NET_C(R14.2, R16.1, C26.1, R15.1)
	NET_C(R15.2, C25.1, XC20.5)
	NET_C(XC20.7, XC20.6, C26.2, R12.1)

	NET_C(R12.2, C23.1, R13.1)
	NET_C(R13.2, C24.1, XC20.3)
	NET_C(XC20.1, XC20.2, C23.2, C62.1)

	NET_C(GND, R16.2, C25.2, C24.2)

	// ------------------------------------
	// Mixing and pre amplifier
	// ------------------------------------

	RES(R6, RES_K(10))
	RES(R9, RES_K(47))
	RES(R10, RES_K(47))
	RES(R4, RES_K(10))
	RES(R7, RES_K(47))
	RES(R3, RES_K(10))
	RES(R5, RES_K(10))
	RES(VR1, RES_K(2))    // FIXME: Actually a potentiometer
	CAP(C12, CAP_U(10))
	CAP(C63, CAP_U(2.2))
	CAP(C20, CAP_P(1000))
	CAP(C19, CAP_U(47))  // FIXME: Needs verification
	RES(AMP_IN, RES_K(100))

	NET_C(C62.2, R6.1)
	NET_C(C22.2, R9.1)
	NET_C(C21.2, R10.1)
	NET_C(R6.2, R9.2, R10.2, XC20.9, C20.1, R7.1) // FIXME: Schematics read "9" (XC20.9) - other AMP?
	NET_C(R7.2, C20.2, C63.1, XC20.8) // FIXME: Schematics read "7" (XC20.7) - other AMP?

	NET_C(VP5, R4.1)
	NET_C(R4.2, C19.1, R5.1, XC20.10) // FIXME: Schematics read "8" (XC20.8) - other AMP?

	NET_C(C63.2, R3.1)
	NET_C(R3.2, VR1.1, C12.1)
	NET_C(C12.2, AMP_IN.1)

	NET_C(GND, VR1.2, C19.2, R5.2, AMP_IN.2)

	// OUTPUT

	ALIAS(OUT, AMP_IN.1)

	// Not connected

	NET_C(GND, XC20.13, XC20.12, D20.13, D20.12, D20.2, D20.3)
	NET_C(GND, XC20.14, D20.14, D20.1)
NETLIST_END()

