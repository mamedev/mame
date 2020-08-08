// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74393_H_
#define NLD_74393_H_

#include "netlist/nl_setup.h"

// usage: TTL_74393(name, cCP, cMR)
#define TTL_74393(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74393, __VA_ARGS__)

#endif /* NLD_74193_H_ */
