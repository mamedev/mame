// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74193_H_
#define NLD_74193_H_

#include "../nl_setup.h"

// usage       : TTL_74193(name, pA, pB, pC, pD, pCLEAR, pLOADQ, pCU, pCD)
// auto connect: VCC, GND
#define TTL_74193(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74193, __VA_ARGS__)

#endif /* NLD_74193_H_ */
