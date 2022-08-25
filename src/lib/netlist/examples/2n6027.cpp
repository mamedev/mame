// license:CC0
// copyright-holders:Couriersud

#include "netlist/devices/net_lib.h"

/*
 * Run with
 * ./nltool -t 1 -f nl_examples/2N6027.cpp -l PUT.A -l PUT.K -l PUT.G -v
 */

/* ----------------------------------------------------------------------------
 *  Library section header START
 * ---------------------------------------------------------------------------*/

#ifndef __PLIB_PREPROCESSOR__

#define PUT_2N6027(_name)                                                     \
		NET_REGISTER_DEV(G501534_DIP, _name)

//NETLIST_EXTERNAL(ex2N6027)
NETLIST_EXTERNAL(loc_lib)

#endif

/* ----------------------------------------------------------------------------
 *  Library section header END
 * ---------------------------------------------------------------------------*/

NETLIST_START(ex2N6027)
{

	SOLVER(Solver, 48000)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-9)
	PARAM(Solver.METHOD, "MAT_CR")

	LOCAL_SOURCE(loc_lib)
	INCLUDE(loc_lib)

	ANALOG_INPUT(VB, 10)

	PUT_2N6027(PUT)

	// Figure 3 from datasheet
	RES(R1, RES_K(510))
	RES(R2, RES_K(16))
	RES(R3, RES_K(27))
	RES(R4, 20)
	CAP(C, CAP_U(0.1))

	NET_C(VB, R1.1, R2.1)
	NET_C(R1.2, C.1, PUT.A)
	NET_C(PUT.K, R4.1)
	NET_C(PUT.G, R2.2, R3.1)

	NET_C(GND, C.2, R4.2, R3.2)

}


NETLIST_START(PUT_2N6027)
{

	NET_MODEL("2N6027_NPN NPN(IS=5E-15 VAF=100 IKF=0.3 ISE=1.85E-12 NE=1.45 RE=0.15 RC=0.15 CJE=7E-10 TF=0.6E-8 CJC=2.2E-10 TR=4.76E-8 XTB=3)")
	NET_MODEL("2N6027_PNP PNP(IS=2E-15 VAF=100 IKF=0.3 ISE=1.90E-12 NE=1.5 RE=0.15 RC=0.15 CJE=7E-10 TF=1.6E-8 CJC=2.2E-10 TR=5.1E-8 XTB=3)")

	QBJT_EB(Q1, "2N6027_NPN")
	QBJT_EB(Q2, "2N6027_PNP")

	/* The netlist transistor model currently doesn't support
	 * BE and BC capacitances.
	 * Adding those here significantly reduces NR loops.
	 * FIXME: Needs to be removed when added to the
	 * transistor EB model.
	 */
#if 0
	CAP(CJE1, CAP_N(1))
	CAP(CJE2, CAP_N(1))

	NET_C(CJE1.1, Q1.B)
	NET_C(CJE1.2, Q1.E)
	NET_C(CJE2.1, Q2.B)
	NET_C(CJE2.2, Q2.E)
#endif
	NET_C(Q1.C, Q2.B)
	NET_C(Q1.B, Q2.C)

	ALIAS(G, Q2.B)
	ALIAS(A, Q2.E)
	ALIAS(K, Q1.E)

}

NETLIST_START(loc_lib)
{

	LOCAL_LIB_ENTRY(PUT_2N6027)

}
