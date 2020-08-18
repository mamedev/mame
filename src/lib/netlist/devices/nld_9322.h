// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_9322_H_
#define NLD_9322_H_

#include "netlist/nl_setup.h"

#define TTL_9322_GATE(...)                                                \
		NET_REGISTER_DEVEXT(TTL_9322_GATE, __VA_ARGS__)

// usage: TTL_9322(name, cSELECT, cA1, cB1, cA2, cB2, cA3, cB3, cA4, cB4, cSTROBE)
#define TTL_9322(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_9322, __VA_ARGS__)

#endif /* NLD_9322_H_ */
