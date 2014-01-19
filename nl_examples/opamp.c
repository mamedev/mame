/*
 * opamp.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(opamp)

    /* Opamp model from
     *
     * http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm
     *
     * Bandwidth 10Mhz
     *
     * This one is connected as a impedance changer
     */

    /* Standard stuff */

    CLOCK(clk)
    PARAM(clk.FREQ, 1000) // 1000 Hz
    SOLVER(Solver)
    PARAM(Solver.FREQ, 48000)
    PARAM(Solver.ACCURACY, 1e-6)

    /* Wiring up the opamp */

    NET_C(PLUS, clk)
    NET_C(MINUS, OUT)

    /* The opamp model */

    NETDEV_VCCS(G1)
    PARAM(G1.G, 100)  // typical OP-AMP amplification 100 * 1000 = 100000
    RES(RP1, 1000)
    CAP(CP1, 1.59e-6)   // <== change to 1.59e-3 for 10Khz bandwidth
    NETDEV_VCVS(EBUF)
    PARAM(EBUF.RO, 50)
    PARAM(EBUF.G, 1)

    NET_ALIAS(PLUS, G1.IP) // Positive input
    NET_ALIAS(MINUS, G1.IN) // Negative input
    NET_ALIAS(OUT, EBUF.OP) // Opamp output ...

    NET_C(EBUF.ON, GND)

    NET_C(G1.ON, GND)
    NET_C(RP1.2, GND)
    NET_C(CP1.2, GND)
    NET_C(EBUF.IN, GND)

    NET_C(RP1.1, G1.OP)
    NET_C(CP1.1, RP1.1)
    NET_C(EBUF.IP, RP1.1)

    //LOG(logX, OUT)
    //LOG(logY, 4V)
NETLIST_END()
