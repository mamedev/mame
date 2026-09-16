// license:CC0-1.0
// copyright-holders:m1macrophage

#include "netlist/devices/net_lib.h"

NETLIST_START(moogsource)
{
	SOLVER(Solver, 1000)  // Used for DC calculations, so a low frequency is fine.

	ANALOG_INPUT(VPLUS, 15)
	ANALOG_INPUT(VMINUS, -15)
	// Voltage at the CA3080's Iabc input. One diode drop above -15V.
	ANALOG_INPUT(CA3080_VABC, -14.3)


	// Voltage-to-exponential-current converter for the filter contour generator.
	// Generates the control current for U44 (CA3080A).

	// Inputs.
	ANALOG_INPUT(FLT_CNTR_RATE)
	POT(R201, RES_K(100))  // 'Range' pot.

	RES(R199, RES_K(43.2))  // 1% tolerance.
	RES(R200, RES_K(470))
	RES(R202, RES_M(1))
	RES(R203, RES_K(1))  // 1% tolerance.
	QBJT_EB(Q35, "2N3904")
	RES(R204, RES_K(100))
	QBJT_EB(Q36, "2N3906")
	RES(R198, RES_K(10))

	NET_C(FLT_CNTR_RATE, R199.1)
	NET_C(R199.2, R200.1, R202.1, R203.1, Q35.B)
	NET_C(R200.2, R201.2)
	NET_C(Q35.E, R204.1, Q36.B)
	NET_C(Q36.C, R198.1)
	NET_C(R198.2, CA3080_VABC)
	NET_C(VPLUS, Q35.C)
	NET_C(GND, R201.1, R203.2, Q36.E)
	NET_C(VMINUS, R201.3, R202.2, R204.2)

	ALIAS(FLT_CNTR_CV, R198.1)  // Output.


	// Voltage-to-exponential-current converter for the loudness contour generator.
	// Generates the control current for U42 (CA3080A). Identical to the circuit
	// above.

	// Inputs.
	ANALOG_INPUT(LOUD_CNTR_RATE, 0)
	POT(R179, RES_K(100))  // 'Range' pot.

	RES(R177, RES_K(43.2))  // 1% tolerance.
	RES(R178, RES_K(470))
	RES(R180, RES_M(1))
	RES(R181, RES_K(1))  // 1% tolerance.
	QBJT_EB(Q33, "2N3904")
	RES(R185, RES_K(100))
	QBJT_EB(Q34, "2N3906")
	RES(R186, RES_K(10))

	NET_C(LOUD_CNTR_RATE, R177.1)
	NET_C(R177.2, R178.1, R180.1, R181.1, Q33.B)
	NET_C(R178.2, R179.2)
	NET_C(Q33.E, R185.1, Q34.B)
	NET_C(Q34.C, R186.1)
	NET_C(R186.2, CA3080_VABC)
	NET_C(VPLUS, Q33.C)
	NET_C(GND, R179.1, R181.2, Q34.E)
	NET_C(VMINUS, R179.3, R180.2, R185.2)

	ALIAS(LOUD_CNTR_CV, R186.1)  // Output.


	// Voltage-to-exponential-current converter for the LFO. Generates the
	// control current for U49 (CA3080A). Almost identical to the circuit above,
	// except for a resistor value and the 'range' potentiometer being connected
	// to +15V instead of -15V.

	// Inputs.
	ANALOG_INPUT(MOD_RATE, 0)
	POT(R223, RES_K(100))  // 'Range' pot.

	RES(R221, RES_K(49.9))  // 1% tolerance.
	RES(R222, RES_K(470))
	RES(R224, RES_M(1))
	RES(R225, RES_K(1))  // 1% tolerance.
	QBJT_EB(Q38, "2N3904")
	RES(R226, RES_K(100))
	QBJT_EB(Q39, "2N3906")
	RES(R227, RES_K(10))

	NET_C(MOD_RATE, R221.1)
	NET_C(R221.2, R222.1, R224.1, R225.1, Q38.B)
	NET_C(R222.2, R223.2)
	NET_C(Q38.E, R226.1, Q39.B)
	NET_C(Q39.C, R227.1)
	NET_C(R227.2, CA3080_VABC)
	NET_C(VPLUS, R223.3, Q38.C)
	NET_C(GND, R223.1, R225.2, Q39.E)
	NET_C(VMINUS, R224.2, R226.2)

	ALIAS(MOD_CV, R227.1)  // Output.
}

