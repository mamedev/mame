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

#define RAM_2102A(name, cCEQ, cA0, cA1, cA2, cA3, cA4, cA5, cA6, cA7, cA8, cA9, cRWQ, cDI)  \
		NET_REGISTER_DEV(RAM_2102A, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CEQ, cCEQ)                                           \
		NET_CONNECT(name, A0,  cA0)                                            \
		NET_CONNECT(name, A1,  cA1)                                            \
		NET_CONNECT(name, A2,  cA2)                                            \
		NET_CONNECT(name, A3,  cA3)                                            \
		NET_CONNECT(name, A4,  cA4)                                            \
		NET_CONNECT(name, A5,  cA5)                                            \
		NET_CONNECT(name, A6,  cA6)                                            \
		NET_CONNECT(name, A7,  cA7)                                            \
		NET_CONNECT(name, A8,  cA8)                                            \
		NET_CONNECT(name, A9,  cA9)                                            \
		NET_CONNECT(name, RWQ, cRWQ)                                           \
		NET_CONNECT(name, DI, cDI)

#define RAM_2102A_DIP(name)                                                    \
		NET_REGISTER_DEV(RAM_2102A_DIP, name)

#endif /* NLD_2102A_H_ */
