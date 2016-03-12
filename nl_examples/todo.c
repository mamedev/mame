// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * todo.c
 *
 */

#include "netlist/devices/net_lib.h"

NETLIST_START(7400_TTL)
	NET_REGISTER_DEV(7400, s1)
	NET_REGISTER_DEV(7400, s2)
	NET_REGISTER_DEV(7400, s3)
	NET_REGISTER_DEV(7400, s4)

	ALIAS(1, s1.A);
	ALIAS(2, s1.B);
	ALIAS(3, s1.Q);

	ALIAS(4, s2.A);
	ALIAS(5, s2.B);
	ALIAS(6, s2.Q);

	ALIAS(9, s3.A);
	ALIAS(10, s3.B)
	ALIAS(8, s3.Q);

	ALIAS(12, s4.A);
	ALIAS(13, s4.B);
	ALIAS(11, s4.Q);

NETLIST_END()

NETLIST_START(lib)
	TRUTHTABLE_START(7400A, 2, 1, 0, "+A,B")
		TT_HEAD(" A , B | Q ")
		TT_LINE(" 0 , X | 1 |22")
		TT_LINE(" X , 0 | 1 |22")
		TT_LINE(" 1 , 1 | 0 |15")
	TRUTHTABLE_END()
NETLIST_END()



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

