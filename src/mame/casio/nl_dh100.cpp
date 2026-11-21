// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*
    Casio DH-100 netlist sound

    Current issues:
    - VCF filter stages really use JFETs, but are currently faked using variable resistors.
      The overall frequency response currently sounds okay, but not entirely accurate
    - Audible distortion and aliasing at higher pitches; better solver params probably needed
*/

#include "netlist/devices/net_lib.h"
#include "nl_dh100.h"

// VCF section is separate so it can be easily used with other models later
static NETLIST_START(dh100_filter)
{
	MC1558_DIP(IC2) // actually BA4558
	NET_C(VDA, IC2.8)
	NET_C(GND, IC2.4)

	// input opamp
	RES(R4, RES_K(2.7))
	RES(R5, RES_K(2.7))
	RES(R13, RES_K(39))
	RES(R14, RES_K(10))
	RES(R15, RES_K(10))

	CAP(C37, CAP_U(1))
	CAP(C39, CAP_U(33))

	NET_C(VDA, R4.2)
	NET_C(GND, R5.2, C39.2)
	NET_C(R15.2, C37.1)
	NET_C(IC2.6, R13.2, R14.2, C37.2)
	NET_C(IC2.5, R4.1, R5.1, C39.1)
	ALIAS(VCF_IN, R15.1)
	ALIAS(VREF, IC2.5)

	// filter stages
	// 4x 2SK425, currently modeled as variable resistors instead (resistor value is somewhat arbitrary)
	POT2(T1, RES_R(200))
	POT2(T2, RES_R(200))
	POT2(T3, RES_R(200))
	POT2(T4, RES_R(200))
	PARAM(T1.REVERSE, 1)
	PARAM(T2.REVERSE, 1)
	PARAM(T3.REVERSE, 1)
	PARAM(T4.REVERSE, 1)

	CAP(C2, CAP_N(100))
	CAP(C3, CAP_N(100))
	CAP(C4, CAP_N(100))
	CAP(C6, CAP_N(6.8))
	CAP(C7, CAP_N(100))

	NET_C(IC2.7, R14.1, T4.1)
	NET_C(T4.2, C7.1, T3.1)
	NET_C(T3.2, C2.1, T1.1)
	NET_C(T1.2, C3.1, T2.1)
	NET_C(T2.2, C4.1, C6.1)
	NET_C(VREF, C2.2, C3.2, C4.2, C7.2)

	// output opamp
	RES(R6, RES_K(10))
	RES(R7, RES_K(100))
	RES(R8, RES_K(220))
	RES(MAIN_VR, RES_K(10))

	CAP(C5, CAP_P(100))
	CAP(C29, CAP_U(1))
	CAP(C31, CAP_U(1))

	NET_C(IC2.3, R8.1, C6.2)
	NET_C(IC2.2, R6.1, R7.2, C5.2)
	NET_C(IC2.1, R7.1, C5.1, C29.1, C31.1)
	NET_C(VREF, R6.2, R8.2)

	NET_C(C29.2, MAIN_VR.1)
	NET_C(MAIN_VR.2, GND)
	ALIAS(OUTPUT, MAIN_VR.1)

	// feedback
	CD4066_GATE(IC5C)
	NET_C(VDA, IC5C.VDD)
	NET_C(GND, IC5C.VSS)
	ALIAS(VCF_FB, IC5C.CTL)

	RES(R76, RES_K(10))
	RES(R78, RES_R(220))
	RES(R79, RES_M(2.2))

	CAP(C1, CAP_N(10))

	NET_C(C31.2, R76.1)
	NET_C(R76.2, R78.1, R13.1, C1.1)
	NET_C(R78.2, R79.1, IC5C.R.1)
	NET_C(VREF, R79.2, C1.2, IC5C.R.2)
}

NETLIST_START(dh100)
{
	SOLVER(Solver, 48000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_LTE, 1e-2)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-8)

	ANALOG_INPUT(VDA, 5)

	LOCAL_SOURCE(dh100_filter)
	INCLUDE(dh100_filter)

	// inputs from CPU
	TTL_INPUT(PC6, 0)
	TTL_INPUT(PC7, 0)
	TTL_INPUT(PF4, 0)
	TTL_INPUT(PF5, 0)
	TTL_INPUT(PF6, 0)
	TTL_INPUT(PF7, 0)
	NET_C(VDA, PC6.VCC, PC7.VCC, PF4.VCC, PF5.VCC, PF6.VCC, PF7.VCC)
	NET_C(GND, PC6.GND, PC7.GND, PF4.GND, PF5.GND, PF6.GND, PF7.GND)

	CD4066_GATE(IC5A)
	CD4066_GATE(IC5B)
	CD4066_GATE(IC5D)
	NET_C(VDA, IC5A.VDD, IC5B.VDD, IC5D.VDD)
	NET_C(GND, IC5A.VSS, IC5B.VSS, IC5D.VSS)

	NET_C(PF4, IC5B.CTL)
	NET_C(PF5, VCF_FB)
	NET_C(PF6, IC5D.CTL)
	NET_C(PF7, IC5A.CTL)

	// VCA
	R2R_DAC(DAC, 5, RES_K(25), 8)

	QBJT_EB(T5, "2SC2412")

	RES(R10, RES_K(10))
	RES(R11, RES_K(10))

	CAP(C40, CAP_U(1))

	NET_C(VDA, T5.C)
	NET_C(GND, DAC.VGND, R11.2, C40.2)
	NET_C(DAC.VOUT, C40.1, T5.B)
	NET_C(T5.E, R10.1)
	NET_C(R10.2, R11.1)

	ALIAS(VCA, T5.E)

	// 8' tone generator (PC6)
	RES(R65, RES_K(10))
	RES(R66, RES_K(47))
	RES(R67, RES_M(2.2))
	RES(R68, RES_M(2.2)) // misprinted as 2.2k on schematic
	RES(R71, RES_R(100))
	RES(R73, RES_K(47))
	RES(R77, RES_K(470))
	RES(R82, RES_K(68))

	CAP(C33, CAP_N(10))
	CAP(C35, CAP_N(1))
	CAP(C38, CAP_N(3.3))

	DIODE(D6, "D") // DA116

	QBJT_EB(T8, "2SC2412")

	NET_C(VCA, R82.1)
	NET_C(GND, D6.A, T8.E, R66.2, C38.2)
	NET_C(PC6, R65.1)
	NET_C(IC5A.R.1, R65.2, R67.1, C33.1)
	NET_C(IC5A.R.2, T8.B, D6.K, R66.1, R67.2, C33.2)
	NET_C(T8.C, R71.1)
	NET_C(IC5B.R.1, R68.1, C35.1)
	NET_C(IC5B.R.2, IC5D.R.2, R68.2, R71.2, R73.1, R77.1, R82.2)
	NET_C(IC5D.R.1, R77.2, C38.1)

	// 16' tone generator (PC7)
	RES(R69, RES_K(10))
	RES(R70, RES_K(47))
	RES(R72, RES_R(100))
	RES(R74, RES_K(22))
	RES(R83, RES_K(47))

	CAP(C34, CAP_N(33))
	CAP(C36, CAP_N(10))

	QBJT_EB(T9, "2SC2412")

	NET_C(VCA, R83.1)
	NET_C(GND, T9.E, R70.2, C36.2)
	NET_C(PC7, C34.1)
	NET_C(C34.2, R69.1)
	NET_C(T9.B, R69.2, R70.1)
	NET_C(T9.C, R72.1)
	NET_C(R72.2, R74.1, R83.2, C36.1)

	// mixing
	RES(R80, RES_K(1))
	NET_C(R80.1, R73.2, R74.2, C35.2, VCF_IN)
	NET_C(GND, R80.2)
}
