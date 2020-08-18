// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74175_H_
#define NLD_74175_H_

#include "netlist/nl_setup.h"

#define TTL_74175(name, cCLK, cD1, cD2, cD3, cD4, cCLRQ)                       \
		NET_REGISTER_DEV(TTL_74175, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CLK,  cCLK)                                          \
		NET_CONNECT(name, D1,   cD1)                                           \
		NET_CONNECT(name, D2,   cD2)                                           \
		NET_CONNECT(name, D3,   cD3)                                           \
		NET_CONNECT(name, D4,   cD4)                                           \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74175_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74175_DIP, name)


#endif /* NLD_74175_H_ */
