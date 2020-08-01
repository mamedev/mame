// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
 * nld_2102A.h
 *
 *  2102: 1024 x 1-bit Static RAM
 *
 *          +--------------+
 *       A6 |1     ++    16| A7
 *       A5 |2           15| A8
 *      RWQ |3           14| A9
 *       A1 |4   82S16   13| CEQ
 *       A2 |5           12| DO
 *       A3 |6           11| DI
 *       A4 |7           10| VCC
 *       A0 |8            9| GND
 *          +--------------+
 *
 *
 *  Naming conventions follow Intel datasheet
 *
 */

#ifndef NLD_2102A_H_
#define NLD_2102A_H_

#include "netlist/nl_setup.h"

// expects: RAM_2102A(name, cCEQ, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cRWQ, cDI)
#define RAM_2102A(...)                                           \
	NET_REGISTER_DEVEXT(RAM_2102A, __VA_ARGS__)

#endif /* NLD_2102A_H_ */
