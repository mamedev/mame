// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef NLD_74365_H_
#define NLD_74365_H_

#include "netlist/nl_setup.h"

#define TTL_74365(name, cG1Q, cG2Q, cA1, cA2, cA3, cA4, cA5, cA6)              \
		NET_REGISTER_DEV(TTL_74365, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, G1Q,   cG1Q)                                         \
		NET_CONNECT(name, G2Q,   cG2Q)                                         \
		NET_CONNECT(name, A1,    cA1)                                          \
		NET_CONNECT(name, A2,    cA2)                                          \
		NET_CONNECT(name, A3,    cA3)                                          \
		NET_CONNECT(name, A4,    cA4)                                          \
		NET_CONNECT(name, A5,    cA5)                                          \
		NET_CONNECT(name, A6,    cA6)

#define TTL_74365_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74365_DIP, name)

#endif /* NLD_74365_H_ */
