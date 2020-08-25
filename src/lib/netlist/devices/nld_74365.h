// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef NLD_74365_H_
#define NLD_74365_H_

#include "../nl_setup.h"

// usage       : TTL_74365(name, pG1Q, pG2Q, pA1, pA2, pA3, pA4, pA5, pA6)
// auto connect: VCC, GND
#define TTL_74365(...)                                                 \
	NET_REGISTER_DEVEXT(TTL_74365, __VA_ARGS__)

#endif /* NLD_74365_H_ */
