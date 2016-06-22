// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74175.h
 *
 *  DM74175: Quad D Flip-Flops with Clear
 *
 *          +--------------+
 *      CLR |1     ++    16| VCC
 *       Q1 |2           15| Q4
 *      Q1Q |3           14| Q4Q
 *       D1 |4   74175   13| D4
 *       D2 |5           12| D3
 *      Q2Q |6           11| Q3Q
 *       Q2 |7           10| Q3
 *      GND |8            9| CLK
 *          +--------------+
 *
 *          +-----+-----+---++---+-----+
 *          | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+===++===+=====+
 *          |  0  |  X  | X || 0 |  1  |
 *          |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+---++---+-----+
 *
 *   Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -> 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#ifndef NLD_74175_H_
#define NLD_74175_H_

#include "nl_setup.h"

#define TTL_74175(name)                                     \
		NET_REGISTER_DEV(TTL_74175, name)
#define TTL_74175_DIP(name)                                 \
		NET_REGISTER_DEV(TTL_74175_DIP, name)


#endif /* NLD_74175_H_ */
