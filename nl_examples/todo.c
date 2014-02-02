/*
 * todo.c
 *
 */

#if 0
    NETDEV_R(R1, 10)
    NETDEV_R(R2, 10)
    NETDEV_R(R3, 10)
    NET_C(V5,R1.1)
    NET_C(R1.2, R2.1)
    NET_C(R2.2, R3.1)
    NET_C(R3.2, GND)
#endif
#if 0
    NETDEV_R(R4, 1000)
    NETDEV_C(C1, 1e-6)
    NET_C(V5,R4.1)
    NET_C(R4.2, C1.1)
    NET_C(C1.2, GND)
    //NETDEV_LOG(log1, C1.1)
#endif

#if 0
    NETDEV_R(R5, 1000)
    NETDEV_1N914(D1)
    NET_C(V5, R5.1)
    NET_C(R5.2, D1.A)
    NET_C(D1.K, GND)
    //NETDEV_LOG(log1, D1.A)
#endif

#if 0
#endif

#if 0
    NETDEV_VCVS(VV)
    NETDEV_R(R1, 1000)
    NETDEV_R(R2, 10000)

    NET_C(V5, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R2.1, VV.OP)
    NET_C(R2.2, VV.IN)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    NETDEV_LOG(logX, VV.OP)

#endif

#if 0
    NETDEV_VCCS(VV)
    NETDEV_PARAM(VV.G, 100000)  // typical OP-AMP amplification
    NETDEV_R(R1, 1000)
    NETDEV_R(R2, 1)
    NETDEV_R(R3, 10000)

    NET_C(4V, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R2.1, VV.OP)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(R2.2, GND)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    //NETDEV_LOG(logX, VV.OP)
    //NETDEV_LOG(logY, 4V)

#endif

#if 0
    NETDEV_VCVS(VV)
    NETDEV_PARAM(VV.G, 100000)  // typical OP-AMP amplification
    NETDEV_PARAM(VV.RO, 50)  // typical OP-AMP amplification
    NETDEV_R(R1, 1000)
    NETDEV_R(R3, 10000) // ==> 10x amplification (inverting)

    NET_C(4V, R1.1)
    NET_C(R1.2, VV.IN)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(VV.ON, GND)
    NET_C(VV.IP, GND)
    NETDEV_LOG(logX, VV.OP)
    NETDEV_LOG(logY, 4V)

#endif

#if 0
    // Impedance converter with resistor
    NETDEV_VCVS(VV)
    NETDEV_PARAM(VV.G, 100000)  // typical OP-AMP amplification
    NETDEV_PARAM(VV.RO, 50)  // typical OP-AMP amplification
    NETDEV_R(R3, 10000)

    NET_C(4V, VV.IP)
    NET_C(R3.1, VV.IN)
    NET_C(R3.2, VV.OP)
    NET_C(VV.ON, GND)
    NETDEV_LOG(logX, VV.OP)
    NETDEV_LOG(logY, 4V)

#endif

#if 0
    // Impedance converter without resistor
    NETDEV_VCVS(VV)
    NETDEV_PARAM(VV.G, 100000)  // typical OP-AMP amplification
    NETDEV_PARAM(VV.RO, 50)  // typical OP-AMP amplification

    NET_C(4V, VV.IP)
    NET_C(VV.IN, VV.OP)
    NET_C(VV.ON, GND)
    NETDEV_LOG(logX, VV.OP)
    NETDEV_LOG(logY, 4V)

#endif
d
