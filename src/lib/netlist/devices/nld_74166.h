// license:BSD-3-Clause
// copyright-holders:Ryan 161
/*
 * nld_74166.h
 *
 *  74166: Parallel-Load 8-Bit Shift Register
 *
 *          +--------------+
 *      SER |1     ++    16| VCC
 *        A |2           15| SH/LDQ
 *        B |3           14| H
 *        C |4    74166  13| QH
 *        D |5           12| G
 *   CLKINH |6           11| F
 *      CLK |7           10| E
 *      GND |8            9| CLRQ
 *          +--------------+
 *
 * SH/LDQ: Shift / !Load
 * CLKINH: Clock Inhibit
 *
 *  Naming convention attempts to follow Texas Instruments datasheet
 *
 */

#ifndef NLD_74166_H_
#define NLD_74166_H_

#include "nl_setup.h"

#define TTL_74166(name)                                                         \
		NET_REGISTER_DEV(TTL_74166, name)

#define TTL_74166_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74166_DIP, name)

#endif /* NLD_74166_H_ */
