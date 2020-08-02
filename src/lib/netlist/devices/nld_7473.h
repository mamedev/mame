// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7473_H_
#define NLD_7473_H_

#include "netlist/nl_setup.h"

// usage: TTL_7473(name, cCLK, cJ, cK, cCLRQ)
#define TTL_7473(...)                                                          \
		NET_REGISTER_DEVEXT(TTL_7473, __VA_ARGS__)

#define TTL_7473A(...)                                                         \
		TTL_7473(__VA_ARGS__)

#endif /* NLD_7473_H_ */
