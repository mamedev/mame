// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_DM9334_H_
#define NLD_DM9334_H_

#include "../nl_setup.h"

// usage       : TTL_9334(name, pCQ, pEQ, pD, pA0, pA1, pA2)
// auto connect: VCC, GND
#define TTL_9334(...)                                                  \
	NET_REGISTER_DEVEXT(TTL_9334, __VA_ARGS__)

#endif /* NLD_DM9334_H_ */
