// license:GPL-2.0+
// copyright-holders:Sergey Svishchev

#ifndef NLD_7497_H_
#define NLD_7497_H_

#include "netlist/nl_setup.h"

// usage: TTL_7497(name, cCLK, cSTRB, cEN, cUNITY, cCLR, cB0, cB1, cB2, cB3, cB4, cB5)
#define TTL_7497(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7497, __VA_ARGS__)

#endif /* NLD_7497_H_ */
