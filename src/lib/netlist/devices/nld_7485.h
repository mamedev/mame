// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_7485_H_
#define NLD_7485_H_

#include "netlist/nl_setup.h"

// usage: TTL_7485(name, cA0, cA1, cA2, cA3, cB0, cB1, cB2, cB3, cLTIN, cEQIN, cGTIN)
#define TTL_7485(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7485, __VA_ARGS__)

#endif /* NLD_7485_H_ */
