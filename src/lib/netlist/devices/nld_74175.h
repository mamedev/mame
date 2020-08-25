// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74175_H_
#define NLD_74175_H_

#include "../nl_setup.h"

// usage       : TTL_74175(name, pCLK, pD1, pD2, pD3, pD4, pCLRQ)
// auto connect: VCC, GND
#define TTL_74175(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74175, __VA_ARGS__)

#endif /* NLD_74175_H_ */
