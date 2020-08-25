// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_8277_H_
#define NLD_8277_H_

#include "../nl_setup.h"

// usage       : TTL_8277(name, pRESET, pCLK, pCLKA, pD0A, pD1A, pDSA, pCLKB, pD0B, pD1B, pDSB)
// auto connect: VCC, GND
#define TTL_8277(...)                                                \
	NET_REGISTER_DEVEXT(TTL_8277, __VA_ARGS__)

#endif /* NLD_2877_H_ */
