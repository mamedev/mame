// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74107.h
 *
 *  DM74107: DUAL J-K FLIP-FLOPS WITH CLEAR
 *
 *          +--------------+
 *       1J |1     ++    14| VCC
 *      1QQ |2           13| 1CLRQ
 *       1Q |3           12| 1CLK
 *       1K |4    74107  11| 2K
 *       2Q |5           10| 2CLRQ
 *      2QQ |6            9| 2CLK
 *      GND |7            8| 2J
 *          +--------------+
 *
 *
 *          Function table 107
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
 *          |  1  |  *  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  *  |  1  | 0 || 1 |  0  |
 *          |  1  |  *  |  0  | 1 || 0 |  1  |
 *          |  1  |  *  |  1  | 1 || TOGGLE  |
 *          +-----+-----+-----+---++---+-----+
 *                _
 *          * = _| |_
 *
 *          This is positive triggered, J and K
 *          are latched during clock high and
 *          transferred when CLK falls. The
 *          datasheet requires J and K to be
 *          stable during clock high.
 *
 *          Function table 107A
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLRQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 0 |  1  |
 *          |  1  |  F  |  0  | 0 || Q0| Q0Q |
 *          |  1  |  F  |  1  | 0 || 1 |  0  |
 *          |  1  |  F  |  0  | 1 || 0 |  1  |
 *          |  1  |  F  |  1  | 1 || TOGGLE  |
 *          |  1  |  1  |  X  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *          THe 107A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  TODO:  Currently, only the 107A is implemented.
 *         The 107 uses the same model, but different timings.
 *         The requirement that J and K must be stable during
 *         clock high indicates that the chip may exhibit undefined
 *         behaviour.
 *
 */

#ifndef NLD_74107_H_
#define NLD_74107_H_

#include "netlist/nl_setup.h"

#define TTL_74107A(name, cCLK, cJ, cK, cCLRQ)                                   \
		NET_REGISTER_DEV(TTL_74107A, name)                                      \
		NET_CONNECT(name, CLK, cCLK)                                            \
		NET_CONNECT(name, J, cJ)                                                \
		NET_CONNECT(name, K, cK)                                                \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74107(name, cCLK, cJ, cK, cCLRQ)                                    \
		TTL_74107A(name, cCLK, cJ, cK, cCLRQ)

#define TTL_74107_DIP(name)                             \
		NET_REGISTER_DEV(TTL_74107_DIP, name)

#endif /* NLD_74107_H_ */
