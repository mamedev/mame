// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_82S115_H_
#define NLD_82S115_H_

#include "netlist/nl_setup.h"

// expects: PROM_82S115(name, cCE1Q, cCE2, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cSTROBE)
#define PROM_82S115(...)                                           \
	NET_REGISTER_DEVEXT(PROM_82S115, __VA_ARGS__)

#endif /* NLD_82S115_H_ */
