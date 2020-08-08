// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_7450_H_
#define NLD_7450_H_

#include "netlist/nl_setup.h"

// usage: TTL_7450_ANDORINVERT(name, cI1, cI2, cI3, cI4)
#define TTL_7450_ANDORINVERT(...)                                              \
		NET_REGISTER_DEVEXT(TTL_7450_ANDORINVERT, __VA_ARGS__)

#endif /* NLD_7450_H_ */
