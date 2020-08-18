// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_74193_H_
#define NLD_74193_H_

#include "netlist/nl_setup.h"

#define TTL_74193(name, cA, cB, cC, cD, cCLEAR, cLOADQ, cCU, cCD)              \
		NET_REGISTER_DEV(TTL_74193, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, A,     cA)                                           \
		NET_CONNECT(name, B,     cB)                                           \
		NET_CONNECT(name, C,     cC)                                           \
		NET_CONNECT(name, D,     cD)                                           \
		NET_CONNECT(name, CLEAR, cCLEAR)                                       \
		NET_CONNECT(name, LOADQ, cLOADQ)                                       \
		NET_CONNECT(name, CU,    cCU)                                          \
		NET_CONNECT(name, CD,    cCD)

#define TTL_74193_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74193_DIP, name)

#endif /* NLD_74193_H_ */
