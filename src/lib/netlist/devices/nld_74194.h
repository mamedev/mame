// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef NLD_74194_H_
#define NLD_74194_H_

#include "../nl_setup.h"

// usage       : TTL_74194(name, pCLK, pS0, pS1, pSRIN, pA, pB, pC, pD, pSLIN, pCLRQ)
// auto connect: VCC, GND
#define TTL_74194(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74194, __VA_ARGS__)

#endif /* NLD_74194_H_ */
