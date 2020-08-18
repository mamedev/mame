// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74153_H_
#define NLD_74153_H_

#include "netlist/nl_setup.h"

// usage: TTL_74153(name, cC0, cC1, cC2, cC3, cA, cB, cG)
#define TTL_74153(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74153, __VA_ARGS__)

#endif /* NLD_74153_H_ */
