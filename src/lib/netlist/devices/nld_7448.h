// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7448.h
 *
 *  DM7448: BCD to 7-Segment decoders/drivers
 *
 *           +--------------+
 *         B |1     ++    16| VCC
 *         C |2           15| f
 * LAMP TEST |3           14| g
 *    BI/RBQ |4    7448   13| a
 *       RBI |5           12| b
 *         D |6           11| c
 *         A |7           10| d
 *       GND |8            9| e
 *           +--------------+
 *
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_7448_H_
#define NLD_7448_H_

#include "nl_base.h"
#include "nld_truthtable.h"

#define TTL_7448(name, cA0, cA1, cA2, cA3, cLTQ, cBIQ, cRBIQ)                   \
		NET_REGISTER_DEV(TTL_7448, name)                                        \
		NET_CONNECT(name, A, cA0)                                               \
		NET_CONNECT(name, B, cA1)                                               \
		NET_CONNECT(name, C, cA2)                                               \
		NET_CONNECT(name, D, cA3)                                               \
		NET_CONNECT(name, LTQ, cLTQ)                                            \
		NET_CONNECT(name, BIQ, cBIQ)                                            \
		NET_CONNECT(name, RBIQ, cRBIQ)

#define TTL_7448_DIP(name)                                                      \
		NET_REGISTER_DEV(TTL_7448_DIP, name)

#endif /* NLD_7448_H_ */
