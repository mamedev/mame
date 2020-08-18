// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74174_H_
#define NLD_74174_H_

#include "netlist/nl_setup.h"

// usage: TTL_74174_GATE(name)
#define TTL_74174_GATE(...)                                                   \
		NET_REGISTER_DEVEXT(TTL_74174_GATE, __VA_ARGS__)

// usage: TTL_74174(name, cCLK, cD1, cD2, cD3, cD4, cD5, cD6, cCLRQ)
#define TTL_74174(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74174, __VA_ARGS__)

#endif /* NLD_74174_H_ */
