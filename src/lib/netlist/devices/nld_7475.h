// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_7475_H_
#define NLD_7475_H_

#include "netlist/nl_setup.h"

#define TTL_7475_GATE(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7475_GATE, __VA_ARGS__)

#define TTL_7477_GATE(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7477_GATE, __VA_ARGS__)

#endif /* NLD_7475_H_ */
