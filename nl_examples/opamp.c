/*
 * opamp.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(main)

    /* Standard stuff */

    CLOCK(clk, 1000) // 1000 Hz
    SOLVER(Solver, 48000)
    //PARAM(Solver.ACCURACY, 1e-3)
    //PARAM(Solver.CONVERG, 1.0)
    //PARAM(Solver.GS_LOOPS, 30)

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

    //LOG(logX, op1.OUT)
    //LOG(logY, clk)
NETLIST_END()

NETLIST_START(opamp)

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

    VCCS(G1)
    PARAM(G1.G, 100)  // typical OP-AMP amplification 100 * 1000 = 100000
    RES(RP1, 1000)
    CAP(CP1, 1.59e-6)   // <== change to 1.59e-3 for 10Khz bandwidth
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

NETLIST_END()
