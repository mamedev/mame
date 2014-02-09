/*
 * ne555_astable.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(ne555_astable)

    /*
     * Astable ne555
     *
     */

    /* Standard stuff */

    SOLVER(Solver)
    PARAM(Solver.FREQ, 48000)

    ANALOG_INPUT(V5, 5)  // 5V
    ANALOG_INPUT(VF, 2.5)  // 5V
    ANALOG_INPUT(VR, 5)  // 5V

    SN74LS629(OSC, 0.022e-6)

    NET_C(GND, OSC.GND)
    NET_C(VR, OSC.RNG)
    NET_C(VF, OSC.FC)
    NET_C(GND, OSC.ENQ)

    //LOG(log2, OSC.Y)

NETLIST_END()
