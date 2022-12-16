// license:CC0
// copyright-holders:Couriersud
/*
 * opamp.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(main)
{

	/* Standard stuff */

	CLOCK(clk, 1000) // 1000 Hz
	SOLVER(Solver, 480000)
	PARAM(Solver.ACCURACY, 1e-10)
	PARAM(Solver.NR_LOOPS, 30000    )
	//PARAM(Solver.CONVERG, 1.0)
	PARAM(Solver.GS_LOOPS, 30)

	// Tie up +5 to opamps thought it's not currently needed
	// Stay compatible
	ANALOG_INPUT(V5, 5)
	NET_C(op.VCC, V5)
	NET_C(op1.VCC, V5)

	/* Opamp wired as impedance changer */
	SUBMODEL(opamp, op)

	NET_C(op.GND, GND)
	NET_C(op.PLUS, clk)
	NET_C(op.MINUS, op.OUT)

	SUBMODEL(opamp, op1)
	/* Wired as inverting amplifier connected to output of first opamp */

	RES(R1, 100000)
	RES(R2, 200000)

	NET_C(op1.GND, GND)
	NET_C(op1.PLUS, GND)
	NET_C(op1.MINUS, R2.2)
	NET_C(op1.MINUS, R1.2)

	NET_C(op.OUT, R1.1)
	NET_C(op1.OUT, R2.1)

	RES(RL, 1000)
	NET_C(RL.2, GND)
	NET_C(RL.1, op1.OUT)

	LOG(log_X, op1.OUT)
	LOG(log_Y, clk)
}

NETLIST_START(opamp)
{

	/* Opamp model from
	 *
	 * http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm
	 *
	 * Bandwidth 10Mhz
	 *
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, G1.IP) // Positive input
	ALIAS(MINUS, G1.IN) // Negative input
	ALIAS(OUT, EBUF.OP) // Opamp output ...

	ALIAS(GND, EBUF.ON) // GND terminal
	ALIAS(VCC, DUMMY.I) // VCC terminal
	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	LVCCS(G1)
	PARAM(G1.G, 0.0021)
	PARAM(G1.CURLIM, 0.002)
	RES(RP1, 1e7)
	CAP(CP1, 0.00333e-6)
	VCVS(EBUF)
	PARAM(EBUF.RO, 50)
	PARAM(EBUF.G, 1)

//    NET_C(EBUF.ON, GND)

	NET_C(G1.ON, GND)
	NET_C(RP1.2, GND)
	NET_C(CP1.2, GND)
	NET_C(EBUF.IN, GND)

	NET_C(RP1.1, G1.OP)
	NET_C(CP1.1, RP1.1)
	NET_C(EBUF.IP, RP1.1)

}

NETLIST_START(opamp_mod)
{

	/* Opamp model from
	 *
	 * http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm
	 *
	 * MB3614 Unit Gain frequency is about 500 kHz and the first pole frequency
	 * about 5 Hz. We have to keep the Unity Gain Frequency below our sampling
	 * frequency of 24 Khz.
	 *
	 * Simple Opamp Model Calculation
	 *
	 * First Pole Frequency           5 Hz
	 * Unity Gain Frequency      11,000 Hz
	 * RP                       100,000 Ohm
	 * DC Gain / Aol               2200
	 * CP                         0.318 uF
	 * KG                        0.022
	 *
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, G1.IP) // Positive input
	ALIAS(MINUS, G1.IN) // Negative input
	ALIAS(OUT, EBUF.OP) // Opamp output ...

	AFUNC(fUH, 1, "A0 1.2 -")
	AFUNC(fUL, 1, "A0 1.2 +")

	ALIAS(VCC, fUH.A0) // VCC terminal
	ALIAS(GND, fUL.A0) // VGND terminal

	AFUNC(fVREF, 2, "A0 A1 + 0.5 *")
	NET_C(fUH.A0, fVREF.A0)
	NET_C(fUL.A0, fVREF.A1)

	NET_C(EBUF.ON, fVREF)
	/* The opamp model */

	LVCCS(G1)
	PARAM(G1.RI, RES_K(1000))
#if 0
	PARAM(G1.G, 0.0022)
	RES(RP1, 1e6)
	CAP(CP1, 0.0318e-6)
#else
	PARAM(G1.G, 0.002)
	PARAM(G1.CURLIM, 0.002)
	RES(RP1, 9.5e6)
	CAP(CP1, 0.0033e-6)
#endif
	VCVS(EBUF)
	PARAM(EBUF.RO, 50)
	PARAM(EBUF.G, 1)

	NET_C(G1.ON, fVREF)
	NET_C(RP1.2, fVREF)
	NET_C(CP1.2, fVREF)
	NET_C(EBUF.IN, fVREF)

	NET_C(RP1.1, G1.OP)
	NET_C(CP1.1, RP1.1)

	DIODE(DP,"D(IS=1e-15 N=1)")
	DIODE(DN,"D(IS=1e-15 N=1)")
#if 1
	NET_C(DP.K, fUH.Q)
	NET_C(fUL.Q, DN.A)
	NET_C(DP.A, DN.K, RP1.1)
#else
	/*
	 * This doesn't add any performance by decreasing iteration loops.
	 * To the contrary, it significantly decreases iterations
	 */
	RES(RH1, 0.1)
	RES(RL1, 0.1)
	NET_C(DP.K, RH1.1)
	NET_C(RH1.2, fUH.Q)
	NET_C(fUL.Q, RL1.1)
	NET_C(RL1.2, DN.A)
	NET_C(DP.A, DN.K, RP1.1)

#endif
	NET_C(EBUF.IP, RP1.1)

}
