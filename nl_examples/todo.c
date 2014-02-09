/*
 * todo.c
 *
 */

#if 0
    RES(R1, 10)
    RES(R2, 10)
    RES(R3, 10)
    NET_C(V5,R1.1)
    NET_C(R1.2, R2.1)
    NET_C(R2.2, R3.1)
    NET_C(R3.2, GND)
#endif
#if 0
    RES(R4, 1000)
    CAP(C1, 1e-6)
    NET_C(V5,R4.1)
    NET_C(R4.2, C1.1)
    NET_C(C1.2, GND)
    //LOG(log1, C1.1)
#endif

#if 0
    RES(R5, 1000)
    NETDEV_1N914(D1)
    NET_C(V5, R5.1)
    NET_C(R5.2, D1.A)
    NET_C(D1.K, GND)
    //LOG(log1, D1.A)
#endif

#if 0
#endif

#if 0
    NETDEV_VCVS(VV)
    RES(R1, 1000)
    RES(R2, 10000)

    NET_C(V5, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R2.1, VV.OP)
    NET_C(R2.2, VV.IN)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    LOG(logX, VV.OP)

#endif

#if 0
#endif

#if 0
    NETDEV_VCVS(VV)
    PARAM(VV.G, 100000)  // typical OP-AMP amplification
    PARAM(VV.RO, 50)  // typical OP-AMP amplification
    RES(R1, 1000)
    RES(R3, 10000) // ==> 10x amplification (inverting)

    NET_C(4V, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    LOG(logX, VV.OP)
    LOG(logY, 4V)

#endif

#if 0
    // Impedance converter with resistor
    NETDEV_VCVS(VV)
    PARAM(VV.G, 100000)  // typical OP-AMP amplification
    PARAM(VV.RO, 50)  // typical OP-AMP amplification
    RES(R3, 10000)

    NET_C(4V, VV.IP)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(VV.ON, GND)
    LOG(logX, VV.OP)
    LOG(logY, 4V)

#endif

#if 0
    // Impedance converter without resistor
    NETDEV_VCVS(VV)
    PARAM(VV.G, 100000)  // typical OP-AMP amplification
    PARAM(VV.RO, 50)  // typical OP-AMP amplification

    NET_C(4V, VV.IP)
    NET_C(VV.IN, VV.OP)
    NET_C(VV.ON, GND)
    LOG(logX, VV.OP)
    LOG(logY, 4V)

#endif
d
