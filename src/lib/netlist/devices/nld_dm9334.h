// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_DM9334_H_
#define NLD_DM9334_H_

#include "netlist/nl_setup.h"

#define TTL_9334(name, cCQ, cEQ, cD, cA0, cA1, cA2)                            \
		NET_REGISTER_DEV(TTL_9334, name)                                       \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CQ,  cCQ)                                            \
		NET_CONNECT(name, EQ,  cEQ)                                            \
		NET_CONNECT(name, D,   cD)                                             \
		NET_CONNECT(name, A0,  cA0)                                            \
		NET_CONNECT(name, A1,  cA1)                                            \
		NET_CONNECT(name, A2,  cA2)

#define TTL_9334_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_9334_DIP, name)

#endif /* NLD_DM9334_H_ */
