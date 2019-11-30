// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7473.h
 *
 *  7473: Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *  7473A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *
 *          +----------+
 *     1CLK |1   ++  14| 1J
 *    1CLRQ |2       13| 1QQ
 *       1K |3       12| 1Q
 *      VCC |4  7473 11| GND
 *     2CLK |5       10| 2K
 *    2CLRQ |6        9| 2Q
 *       2J |7        8| 2QQ
 *          +----------+
 *
 *
 *          Function table 73
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
 *          transferred when CLK falls.
 *
 *          Function table 73A
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
 *          THe 73A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  FIXME: Currently, only the 73 is implemented.
 *         The 73A uses the same model.
 *
 */

#ifndef NLD_7473_H_
#define NLD_7473_H_

#include "netlist/nl_setup.h"

#define TTL_7473(name, cCLK, cJ, cK, cCLRQ)                                    \
		NET_REGISTER_DEV(TTL_7473, name)                                       \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, CLK, cCLK)                                           \
		NET_CONNECT(name, J, cJ)                                               \
		NET_CONNECT(name, K, cK)                                               \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_7473A(name, cCLK, cJ, cK, cCLRQ)                                   \
		TTL_7473(name, cCLK, cJ, cK, cCLRQ)

#define TTL_7473_DIP(name)                      \
		NET_REGISTER_DEV(TTL_7473_DIP, name)


#define TTL_7473A_DIP(name)                     \
		NET_REGISTER_DEV(TTL_7473A_DIP, name)

#endif /* NLD_7473_H_ */
