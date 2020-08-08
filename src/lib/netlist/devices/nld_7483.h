// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7483_H_
#define NLD_7483_H_

#include "netlist/nl_setup.h"

// usage: TTL_7483(name, cA1, cA2, cA3, cA4, cB1, cB2, cB3, cB4, cCI)
#define TTL_7483(...)                                                     \
		NET_REGISTER_DEVEXT(TTL_7483, __VA_ARGS__)

#endif /* NLD_7483_H_ */
