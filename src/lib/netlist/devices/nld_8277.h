// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_8277_H_
#define NLD_8277_H_

#include "netlist/nl_setup.h"

#define TTL_8277(name, cRESET, cCLK, cCLKA, cD0A, cD1A, cDSA, cCLKB, cD0B, cD1B, cDSB)  \
		NET_REGISTER_DEV(TTL_8277, name)    \
		NET_CONNECT(name, VCC, VCC)         \
		NET_CONNECT(name, GND, GND)         \
		NET_CONNECT(name, RESET, cRESET)    \
		NET_CONNECT(name, CLK, cCLK)        \
		NET_CONNECT(name, CLKA, cCLKA)      \
		NET_CONNECT(name, CLKB, cCLKB)      \
		NET_CONNECT(name, D0A, cD0A)        \
		NET_CONNECT(name, D0B, cD0B)        \
		NET_CONNECT(name, D1A, cD1A)        \
		NET_CONNECT(name, D1B, cD1B)        \
		NET_CONNECT(name, DSA, cDSA)        \
		NET_CONNECT(name, DSB, cDSB)

#define TTL_8277_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_8277_DIP, name)

#endif /* NLD_2877_H_ */
