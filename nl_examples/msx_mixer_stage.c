/*
 * bjt.c
 *
 */


#include "netlist/devices/net_lib.h"

NETLIST_START(msx)
    /* Standard stuff */

    NETDEV_CLOCK(clk)
    NETDEV_PARAM(clk.FREQ, 1000) // 1000 Hz
    NETDEV_SOLVER(Solver)
    NETDEV_PARAM(Solver.FREQ, 48000)
    NETDEV_PARAM(Solver.ACCURACY, 1e-4)

    NETDEV_R(RAY8910, 2345)     // Max Voltage

    NETDEV_ANALOG_INPUT(V5, 5)
    NETDEV_ANALOG_INPUT(V12, 12)

    NETDEV_ANALOG_INPUT(SOUND, 5)
    NETDEV_ANALOG_INPUT(SND, 5)

    NET_MODEL(".model ss9014 NPN(is=2.87599e-14 bf=377.5 vaf=123 ikf=1.1841 ise=4.7863e-15 ne=1.5 br=4.79 var=11.29 ikr=0.275423 isc=1.44544e-14 nc=1.5 rb=200 irb=1e-5 rbm=10 re=0.56 rc=5 cje=1.7205e-11 vje=0.6905907 mje=0.3193434 tf=5.89463e-10 cjc=6.2956p vjc=0.4164212 mjc=0.2559546 xcjc=0.451391 xtb=1.8881 eg=1.2415 xti=3 fc=0.5 Vceo=45 Icrating=0.1 mfg=Fairchild)")

    NETDEV_R(R24, RES_K(51))
    NETDEV_R(R23, RES_K(5))
    NETDEV_R(R21, RES_K(51))
    NETDEV_R(R20, RES_K(1))
    NETDEV_R(R9,  RES_K(10))
    NETDEV_R(R8,  330)

    NETDEV_C(C55, CAP_U(5))     // Guessed

    //NET_C(RAY8910.1, SND)
    NET_C(RAY8910.1, clk)

    //NET_C(C55.1, SOUND)
    NET_C(C55.1, V5)
    NET_C(C55.2, R24.1)
    NET_C(R24.2, R23.2)
    NET_C(R23.1, RAY8910.2)
    NET_C(R23.1, R20.1)
    NET_C(R20.2, GND)

    NET_C(R21.1, V5)
    NET_C(R21.2, R23.2)

    NETDEV_QBJT_EB(T2, "ss9014")

    NET_C(R9.1, V12)
    NET_C(R9.2, T2.C)
    NET_C(R21.2, T2.B)
    NET_C(R8.1, T2.E)
    NET_C(R8.2, GND)

    //NETDEV_LOG(logB, T2.B)
    //NETDEV_LOG(logC, T2.C)

NETLIST_END()
