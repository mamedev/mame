// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

#ifndef NLD_TMS4800_H_
#define NLD_TMS4800_H_

#include "netlist/nl_setup.h"

#define ROM_TMS4800(name, cAR, cOE1, cOE2, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cA10) \
		NET_REGISTER_DEV(ROM_TMS4800, name) \
		NET_CONNECT(name, AR,  cAR)     \
		NET_CONNECT(name, OE1, cOE1)    \
		NET_CONNECT(name, OE2, cOE2)    \
		NET_CONNECT(name, A0,  cA0)     \
		NET_CONNECT(name, A1,  cA1)     \
		NET_CONNECT(name, A2,  cA2)     \
		NET_CONNECT(name, A3,  cA3)     \
		NET_CONNECT(name, A4,  cA4)     \
		NET_CONNECT(name, A5,  cA5)     \
		NET_CONNECT(name, A6,  cA6)     \
		NET_CONNECT(name, A7,  cA7)     \
		NET_CONNECT(name, A8,  cA8)     \
		NET_CONNECT(name, A9,  cA9)     \
		NET_CONNECT(name, A10, cA10)    \
		NET_CONNECT(name, VCC, VCC)     \
		NET_CONNECT(name, GND, GND)

#define ROM_TMS4800_DIP(name)                                 \
		NET_REGISTER_DEV(ROM_TMS4800_DIP, name)

#endif /* NLD_TMS4800_H_ */
