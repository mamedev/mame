// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_74165_H_
#define NLD_74165_H_

#include "netlist/nl_setup.h"

// usage: TTL_74165(name, cCLK, cCLKINH, cSH_LDQ, cSER, cA, cB, cC, cD, cE, cF, cG, cH)
#define TTL_74165(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74165, __VA_ARGS__)

#endif /* NLD_74165_H_ */
