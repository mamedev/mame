// license:CC0
// copyright-holders:Couriersud
/*
 * bjt.c
 *
 */

#include "netlist/devices/net_lib.h"

// Run this with ... ./nltool -c run -t 1 -n cmos_inverter ../examples/cmos_inverter_dk.cpp

static NETLIST_START(CD4069_ANALOG_GATE)
{

	//MOSFET(P, "PMOS(VTO=-1.0 KP=2e-3 LAMBDA=2E-2)")
	//MOSFET(M, "NMOS(VTO=1.0 KP=2e-3 LAMBDA=2E-2)")
	// https://www.youtube.com/watch?v=jayFN7XqPJw
	MOSFET(P, "PMOS(VTO=-1.22 KP=0.044 LAMBDA=0.05 GAMMA=0.25 L=4.22e-6 W=30e-6)")
	MOSFET(M, "NMOS(VTO=1.22 KP=0.044 LAMBDA=0.05 GAMMA=0.25 L=4.22e-6 W=30e-6)")

	ALIAS(VDD, P.S)
	ALIAS(VSS, M.S)
	ALIAS(Q, M.D)
	ALIAS(A, M.G)

#if 0
	// No real difference
	DIODE(D1, "1N4148")
	NET_C(D1.K, A)
	NET_C(D1.A, VSS)

	DIODE(D2, "1N4148")
	NET_C(D2.A, A)
	NET_C(D2.K, VDD)
#endif
	NET_C(P.D, M.D)
	NET_C(M.G, P.G)

}

//- Identifier: CD4069_ANALOG_DIP
//- Title: CD4069UBM/CD4069UBC Inverter Circuits
//- Pinalias: A1,Y1,A2,Y2,A3,Y3,VSS,Y4,A4,Y5,A5,Y6,A6,VDD
//- Package: DIP
//- NamingConvention: Naming conventions follow National Semiconductor datasheet
//- FunctionTable:
//-    http://pdf.datasheetcatalog.com/datasheets/150/206783_DS.pdf
//-
//-    This uses two NMOS/PMOS transistors to model
//     the input to output voltage transfer function.
//
static NETLIST_START(CD4069_ANALOG_DIP)
{
	CD4069_ANALOG_GATE(A)
	CD4069_ANALOG_GATE(B)
	CD4069_ANALOG_GATE(C)
	CD4069_ANALOG_GATE(D)
	CD4069_ANALOG_GATE(E)
	CD4069_ANALOG_GATE(F)

	NET_C(A.VDD, B.VDD, C.VDD, D.VDD, E.VDD, F.VDD)
	NET_C(A.VSS, B.VSS, C.VSS, D.VSS, E.VSS, F.VSS)

	DIPPINS(   /*     +--------------+     */
		  A.A, /*  A1 |1     ++    14| VDD */ A.VDD,
		  A.Q, /*  Y1 |2           13| A6  */ F.A,
		  B.A, /*  A2 |3           12| Y6  */ F.Q,
		  B.Q, /*  Y2 |4    4069   11| A5  */ E.A,
		  C.A, /*  A3 |5           10| Y5  */ E.Q,
		  C.Q, /*  Y3 |6            9| A4  */ D.A,
		A.VSS, /* VSS |7            8| Y4  */ D.Q
			   /*     +--------------+     */
	)
}

NETLIST_START(cmos_inverter)
{
	/* Standard stuff */

	//EXTERNAL_SOURCE(modules_lib)

	//INCLUDE(modules_lib)

	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-7)
	PARAM(Solver.NR_LOOPS, 50)
	PARAM(Solver.METHOD, "MAT_CR")
	ANALOG_INPUT(V5, 5)

	RTEST(X)
	NET_C(X.1, V5)
	NET_C(X.2, GND)

	LOCAL_LIB_ENTRY(CD4069_ANALOG_GATE)
	//SUBMODEL(CD4069_ANALOG_GATE, GATE)
	CD4069_ANALOG_GATE(G1)
	CD4069_ANALOG_GATE(G2)
	CD4069_ANALOG_GATE(G3)

	NET_C(V5, G1.VDD, G2.VDD, G3.VDD)
	NET_C(GND, G1.VSS, G2.VSS, G3.VSS)

	RES(R1, 18000)
	RES(R2, 3300000)
	CAP(C, CAP_U(10)) // is 20!
	NET_C(G1.Q, G2.A)
	NET_C(G2.Q, G3.A, C.1)
	NET_C(G3.Q, R1.1)
	NET_C(C.2, R1.2, R2.1)
	NET_C(R2.2, G1.A)

	RES(R47,10000)
	RES(R46,1000)
	RES(R48,2000) // is 1000!, but assume 50:50 on modulated signal
	CAP(C45, CAP_U(22))
	QBJT_EB(Q,"2SC1815")
	NET_C(G1.Q, R47.1)
	NET_C(R47.2, Q.B)
	NET_C(V5, Q.C)
	NET_C(R46.1, Q.E)
	NET_C(R46.2, C45.1)
	NET_C(R48.1, C45.1)
	NET_C(GND, C45.2, R48.2)


	// capacitance over D - S
#if 0
	CAP(C, CAP_N(1))
	NET_C(M.D, C.1)
	NET_C(M.S, C.2)
#endif
	//LOG(log_G, IN.P)
	//LOG(log_D, G1.Q)
	LOG(log_D, C45.1)

}
