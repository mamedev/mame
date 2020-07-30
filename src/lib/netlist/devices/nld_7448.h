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

#include "netlist/nl_setup.h"

#if !NL_AUTO_DEVICES
#if !(NL_USE_TRUTHTABLE_7448)
#define TTL_7448(...)                                 \
		NET_REGISTER_DEVEXT(TTL_7448, __VA_ARGS__)    \

#endif
#endif

#endif /* NLD_7448_H_ */
