// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_2102A_H_
#define NLD_2102A_H_

#include "../nl_setup.h"

// expects: RAM_2102A(name, cCEQ, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cRWQ, cDI)
#define RAM_2102A(...)                                           \
	NET_REGISTER_DEVEXT(RAM_2102A, __VA_ARGS__)

#endif /* NLD_2102A_H_ */
