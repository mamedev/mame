// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_74166_H_
#define NLD_74166_H_

#include "netlist/nl_setup.h"

// usage: TTL_74166(name, cCLK, cCLKINH, cSH_LDQ, cSER, cA, cB, cC, cD, cE, cF, cG, cH, cCLRQ)
#define TTL_74166(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74166, __VA_ARGS__)

#endif /* NLD_74166_H_ */
