// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74192_H_
#define NLD_74192_H_

#include "../nl_setup.h"

// usage       : TTL_74192(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)
// auto connect: VCC, GND
#define TTL_74192(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74192, __VA_ARGS__)

#endif /* NLD_74192_H_ */
