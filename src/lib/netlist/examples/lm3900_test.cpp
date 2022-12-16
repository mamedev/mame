// license:CC0
// copyright-holders:Couriersud
/*
 * Script to analyze opamp amplification as a function of frequency.
 *
 * ./nltool -t 0.5 -f nl_examples/LM9000_test.cpp
 *
 * t=0.0: 10 Hz
 * t=0.1: 100 Hz
 * t=0.2: 1000 Hz
 * t=0.3: 10000 Hz
 * t=0.4: 100000 Hz
 * ....
 *
 * ./plot_nl.sh --db Y Z
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(main)
{

	/* Standard stuff */

	//VARCLOCK(clk, "0.5 / pow(10, 1 + T * 4)")
	//CLOCK(clk, 1000)
	SOLVER(Solver, 48000)
	PARAM(Solver.ACCURACY, 1e-10)
	PARAM(Solver.NR_LOOPS, 300)
	PARAM(Solver.DYNAMIC_TS, 1)
	PARAM(Solver.DYNAMIC_MIN_TIMESTEP, 1e-7)

	VS(vs, 0)
	/*
	 * Having f(t)=sin(g(t)*t) the derivative becomes
	 *
	 * f'(t) = d/dt(g(t)*t) * cos(g(t)*t)
	 *
	 * w(t) = d/dt(g(t)*t) = 2*pi*freq(t)  is the angular frequency at time t
	 *
	 * choosing freq(t) = pow(10, a+b*t) and integrating we get
	 *
	 * g(t)*t = 2 * pi /(b*ln(10)) * (pow(10, a+b*t)-pow(10,a))
	 */

	PARAM(vs.FUNC, "0.001 * sin(0.2728752708 * (pow(10, 1 + 10*T) - 10))")

	/*
	 * Stepwise ... same as above
	 */
	//PARAM(vs.FUNC, "0.001 * sin(6.28 * pow(10, trunc((1 + T * 10)*10)/10) * T)")

	/*
	 * Fixed Frequency:
	 * PARAM(vs.FUNC, "1.001 * sin(6.28 * 100 * T)")
	 */
	PARAM(vs.R, 0.1)
	ALIAS(clk, vs.1)
	NET_C(vs.2, GND)
	ANALOG_INPUT(V9, 9)
	ANALOG_INPUT(V12, 12)
	ANALOG_INPUT(VM12, -12)

	//OPAMP(op, OPAMP_TEST)
	LM3900(op)

	NET_C(op.GND, VM12)
	NET_C(op.VCC, V12)

	/* Opamp B wired as inverting amplifier connected to output of first opamp */

	RES(R1, 50)
	RES(RP, 500000) // Set to R2 * 2 if VPlus == VCC (see datasheet)
	RES(R2, 500000)

	NET_C(op.PLUS, RP.1)

	//NET_C(RP.2, V9)
	NET_C(RP.2, GND)
	//NET_C(RP.2, V12)

	NET_C(op.MINUS, R2.2)
	NET_C(op.MINUS, R1.2)

	CAP(C, 10e-6)

	NET_C(clk, C.1)
	NET_C(C.2, R1.1)
	NET_C(op.OUT, R2.1)

	CAP(CL, 10.0e-6)
	RES(RL, 2000)
	NET_C(RL.2, GND)
	NET_C(RL.1, CL.2)
	NET_C(CL.1, op.OUT)

	AFUNC(f, 1, "A0 * 1000")
	NET_C(f.A0, RL.1)
#if 1
	LOG(log_Y, R1.1)
	LOG(log_Z, f)
#endif
}
