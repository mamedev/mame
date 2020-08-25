// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

#ifndef NLD_TMS4800_H_
#define NLD_TMS4800_H_

#include "../nl_setup.h"

// usage       : ROM_TMS4800(name, pAR, pOE1, pOE2, pA0, pA1, pA2, pA3, pA4, pA5, pA6, pA7, pA8, pA9, pA10)
// auto connect: VCC, GND
#define ROM_TMS4800(...)                                               \
	NET_REGISTER_DEVEXT(ROM_TMS4800, __VA_ARGS__)

#endif /* NLD_TMS4800_H_ */
