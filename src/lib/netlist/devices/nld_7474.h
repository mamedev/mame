// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7474_H_
#define NLD_7474_H_

#include "netlist/nl_setup.h"

// usage: TTL_7474(name, cCLK, cD, cCLRQ, cPREQ)
#define TTL_7474(...)                                             \
		NET_REGISTER_DEVEXT(TTL_7474, __VA_ARGS__)

#endif /* NLD_7474_H_ */
