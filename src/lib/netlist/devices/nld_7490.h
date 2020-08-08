// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7490_H_
#define NLD_7490_H_

#include "netlist/nl_setup.h"

// usage: TTL_7490(name, cA, cB, cR1, cR2, cR91, cR92)
#define TTL_7490(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7490, __VA_ARGS__)

#endif /* NLD_7490_H_ */
