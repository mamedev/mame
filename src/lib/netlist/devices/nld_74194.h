// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef NLD_74194_H_
#define NLD_74194_H_

#include "netlist/nl_setup.h"

#define TTL_74194(name, cCLK, cS0, cS1, cSRIN, cA, cB, cC, cD, cSLIN, cCLRQ)   \
		NET_REGISTER_DEV(TTL_74194, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CLK,  cCLK)                                          \
		NET_CONNECT(name, S0,   cS0)                                           \
		NET_CONNECT(name, S1,   cS1)                                           \
		NET_CONNECT(name, SRIN, cSRIN)                                         \
		NET_CONNECT(name, A,    cA)                                            \
		NET_CONNECT(name, B,    cB)                                            \
		NET_CONNECT(name, C,    cC)                                            \
		NET_CONNECT(name, D,    cD)                                            \
		NET_CONNECT(name, SLIN, cSLIN)                                         \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74194_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74194_DIP, name)

#endif /* NLD_74194_H_ */
