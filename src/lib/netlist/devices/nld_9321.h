// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_9321_H_
#define NLD_9321_H_

#include "../nl_setup.h"

// usage: TTL_9321(name, cAE, cA0, cA1, cBE, cB0, cB1)
#define TTL_9321(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_9321, __VA_ARGS__)

#endif /* NLD_9321_H_ */
