// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7483.h
 *
 *  DM7483: 4-Bit Binary Adder with Fast Carry
 *
 *          +--------------+
 *       A4 |1     ++    16| B4
 *       S3 |2           15| S4
 *       A3 |3           14| C4
 *       B3 |4    7483   13| C0
 *      VCC |5           12| GND
 *       S2 |6           11| B1
 *       B2 |7           10| A1
 *       A2 |8            9| S1
 *          +--------------+
 *
 *          S = (A + B + C0) & 0x0f
 *
 *          C4 = (A + B + C) > 15 ? 1 : 0
 *
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_7483_H_
#define NLD_7483_H_

#include "netlist/nl_setup.h"

#define TTL_7483(name, cA1, cA2, cA3, cA4, cB1, cB2, cB3, cB4, cCI)             \
		NET_REGISTER_DEV(TTL_7483, name)                                        \
		NET_CONNECT(name, A1, cA1)                                              \
		NET_CONNECT(name, A2, cA2)                                              \
		NET_CONNECT(name, A3, cA3)                                              \
		NET_CONNECT(name, A4, cA4)                                              \
		NET_CONNECT(name, B1, cB1)                                              \
		NET_CONNECT(name, B2, cB2)                                              \
		NET_CONNECT(name, B3, cB3)                                              \
		NET_CONNECT(name, B4, cB4)                                              \
		NET_CONNECT(name, C0, cCI)

#define TTL_7483_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_7483_DIP, name)

#endif /* NLD_7483_H_ */
