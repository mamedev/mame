/*
 * opamp.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(main)
{

	/* Standard stuff */

	CLOCK(clk, 10000) // 1000 Hz
	SOLVER(Solver, 48000)
	ANALOG_INPUT(V5, 5)
	PARAM(Solver.ACCURACY, 1e-6)
	PARAM(Solver.DYNAMIC_TS, 0)
	PARAM(Solver.SOR_FACTOR, 1.0)
	//PARAM(Solver.CONVERG, 1.0)
	PARAM(Solver.GS_LOOPS, 5)

	SUBMODEL(op1, opamp_fast)
	/* Wired as non - inverting amplifier like in LM3900 datasheet */

	RES(R1, 25000)
	RES(R2, 100000)
	RES(R3, 200000)
	CAP(C1, CAP_U(0.05))

	NET_C(clk, C1.1)
	NET_C(C1.2, R1.1)

	NET_C(op1.VM, GND)
	NET_C(op1.VP, V5    )
	NET_C(op1.PLUS, R1.2, R3.1)
	NET_C(R3.2, V5)
	//NET_C(R3.2, GND)
	NET_C(op1.MINUS, R2.2)
	NET_C(op1.OUT, R2.1)

	RES(RL, 10000)
	NET_C(RL.2, GND)
	NET_C(RL.1, op1.OUT)

	LOG(logX, op1.OUT)
	LOG(logY, clk)
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
	ALIAS(MINUS, G2.IN) // Negative input
	ALIAS(OUT, EBUF.OP) // Opamp output ...
	ALIAS(VM, EBUF.ON)  // V- terminal
	ALIAS(VP, DUMMY.I)  // V+ terminal

	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	CCCS(G1)
	CCCS(G2)
	//PARAM(G1.G, 100)  // typical OP-AMP amplification 100 * 1000 = 100000
	PARAM(G1.G, 1000000)  // typical OP-AMP amplification 100 * 1000 = 100000
	PARAM(G2.G, 1000000)  // typical OP-AMP amplification 100 * 1000 = 100000
	RES(RP1, 1000)
	CAP(CP1, 1.59e-6)   // <== change to 1.59e-3 for 10Khz bandwidth
	VCVS(EBUF)
	PARAM(EBUF.RO, 50)
	PARAM(EBUF.G, 1)

//    NET_C(EBUF.ON, GND)

	NET_C(G1.ON, GND)
	NET_C(G2.ON, GND)
	NET_C(RP1.2, GND)
	NET_C(CP1.2, GND)
	NET_C(EBUF.IN, GND)

	NET_C(G1.IN, G2.IP, GND)

	NET_C(RP1.1, G1.OP)
	NET_C(RP1.1, G2.OP)
	NET_C(CP1.1, RP1.1)
	NET_C(EBUF.IP, RP1.1)

}

NETLIST_START(opamp_fast)
{

	/*
	 *  Fast norton opamp model without bandwidth
	 */

	/* Terminal definitions for calling netlists */

	ALIAS(PLUS, R1.1) // Positive input
	ALIAS(MINUS, R2.1) // Negative input
	ALIAS(OUT, G1.OP) // Opamp output ...
	ALIAS(VM, G1.ON)  // V- terminal
	ALIAS(VP, DUMMY.I)  // V+ terminal

	DUMMY_INPUT(DUMMY)

	/* The opamp model */

	RES(R1, 1)
	RES(R2, 1)
	NET_C(R1.1, G1.IP)
	NET_C(R2.1, G1.IN)
	NET_C(R1.2, R2.2, G1.ON)
	VCVS(G1)
	PARAM(G1.G, 1000000)
	PARAM(G1.RO, RES_K(8))

}
