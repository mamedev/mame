// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * Script to analyze opamp amplification as a function of frequency.
 *
 * ./nltool -t 0.5 -f nl_examples/opamp_amplification_curve.cpp
 *
 * t=0.0: 10 Hz
 * t=0.1: 100 Hz
 * t=0.2: 1000 Hz
 * t=0.3: 10000 Hz
 * t=0.4: 100000 Hz
 * ....
 *
 * ./plot_nl.sh --log Y Z
 */


#include "netlist/devices/net_lib.h"

#define OPAMP_TEST "MB3614(FPF=10 UGF=1000k)"

NETLIST_START(main)

    /* Standard stuff */

    //VARCLOCK(clk, "0.5 / pow(10, 1 + T * 4)")
    //CLOCK(clk, 1000)
    SOLVER(Solver, 48000)
    PARAM(Solver.ACCURACY, 1e-7)
    PARAM(Solver.NR_LOOPS, 300)
    PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-7)

	VS(vs, 0)
	PARAM(vs.FUNC, "0.001 * sin(6.28 * pow(10, 1 + 10*T) * T)")
	//PARAM(vs.FUNC, "0.001 * sin(6.28 * pow(10, trunc((1 + T * 4)*2)/2) * T)")
	//PARAM(vs.FUNC, "1.001 * sin(6.28 * 100 * T)")
	PARAM(vs.R, 0.001)
	ALIAS(clk, vs.1)
	NET_C(vs.2, GND)
    ANALOG_INPUT(V12, 12)
    ANALOG_INPUT(VM12, -12)

    OPAMP(op,OPAMP_TEST)

    NET_C(op.GND, VM12)
    NET_C(op.VCC, V12)

    /* Opamp B wired as inverting amplifier connected to output of first opamp */

    RES(R1,   0.1)
    RES(R2, 10000)

    NET_C(op.PLUS, GND)
    NET_C(op.MINUS, R2.2)
    NET_C(op.MINUS, R1.2)

 	NET_C(clk, R1.1)
    NET_C(op.OUT, R2.1)

    RES(RL, 2000)
    NET_C(RL.2, GND)
    NET_C(RL.1, op.OUT)

	AFUNC(f, 1, "A0 * 1000")
	NET_C(f.A0, op.OUT)
#if 1
    LOG(log_Y, R1.1)
    LOG(log_Z, f)
#endif
NETLIST_END()
