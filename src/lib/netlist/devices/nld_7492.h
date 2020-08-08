// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7492_H_
#define NLD_7492_H_

#include "netlist/nl_setup.h"

// usage: TTL_7492(name, cA, cB, cR1, cR2)
#define TTL_7492(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7492, __VA_ARGS__)

#endif /* NLD_7492_H_ */
