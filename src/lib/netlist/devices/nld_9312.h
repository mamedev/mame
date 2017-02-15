// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_9312.h
 *
 *  DM9312: One of Eight Line Data Selectors/Multiplexers
 *
 *          +--------------+
 *       D0 |1     ++    16| VCC
 *       D1 |2           15| Y
 *       D2 |3           14| YQ
 *       D3 |4    9312   13| C
 *       D4 |5           12| B
 *       D5 |6           11| A
 *       D6 |7           10| G   Strobe
 *      GND |8            9| D7
 *          +--------------+
 *                  __
 *          +---+---+---+---++---+---+
 *          | C | B | A | G || Y | YQ|
 *          +===+===+===+===++===+===+
 *          | X | X | X | 1 ||  0| 1 |
 *          | 0 | 0 | 0 | 0 || D0|D0Q|
 *          | 0 | 0 | 1 | 0 || D1|D1Q|
 *          | 0 | 1 | 0 | 0 || D2|D2Q|
 *          | 0 | 1 | 1 | 0 || D3|D3Q|
 *          | 1 | 0 | 0 | 0 || D4|D4Q|
 *          | 1 | 0 | 1 | 0 || D5|D5Q|
 *          | 1 | 1 | 0 | 0 || D6|D6Q|
 *          | 1 | 1 | 1 | 0 || D7|D7Q|
 *          +---+---+---+---++---+---+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_9312_H_
#define NLD_9312_H_

#include "../nl_setup.h"

#define TTL_9312(name, cA, cB, cC, cD0, cD1, cD2, cD3, cD4, cD5, cD6, cD7, cSTROBE)     \
		NET_REGISTER_DEV(TTL_9312, name)    \
		NET_CONNECT(name, A,  cA)       \
		NET_CONNECT(name, B,  cB)       \
		NET_CONNECT(name, C,  cC)       \
		NET_CONNECT(name, D0, cD0)      \
		NET_CONNECT(name, D1, cD1)      \
		NET_CONNECT(name, D2, cD2)      \
		NET_CONNECT(name, D3, cD3)      \
		NET_CONNECT(name, D4, cD4)      \
		NET_CONNECT(name, D5, cD5)      \
		NET_CONNECT(name, D6, cD6)      \
		NET_CONNECT(name, D7, cD7)      \
		NET_CONNECT(name, G,  cSTROBE)

#define TTL_9312_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_9312_DIP, name)

#endif /* NLD_9312_H_ */
