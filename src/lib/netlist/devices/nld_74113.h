// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74113.h
 *
 *  74113: Dual Master-Slave J-K Flip-Flops with Set and Complementary Outputs
 *  74113A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Set and Complementary Outputs
 *
 *          +----------+
 *     1CLK |1   ++  14| VCC
 *       1K |2       13| 2CLK
 *       1J |3       12| 2K
 *    1SETQ |4 74113 11| 2J
 *       1Q |5       10| 2SETQ
 *      1QQ |6        9| 2Q
 *      GND |7        8| 2QQ
 *          +----------+
 *
 *
 *          Function table 113
 *
 *          +-----+-----+-----+---++---+-----+
 *          | SETQ| CLK |  J  | K || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  X  |  X  | X || 1 |  0  |
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
 *          Function table 113A
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
 *          THe 113A is negative triggered.
 *
 *  Naming conventions follow Texas instruments datasheet
 *
 *  FIXME: Currently, only the 113 is implemented.
 *         The 113A uses the same model.
 *
 */

#ifndef NLD_74113_H_
#define NLD_74113_H_

#include "netlist/nl_setup.h"

// usage: TTL_74113(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74113(...)                                                        \
		NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

// usage: TTL_74113A(name, cCLK, cJ, cK, cCLRQ)
#define TTL_74113A(...)                                                       \
		NET_REGISTER_DEVEXT(TTL_74113, __VA_ARGS__)

#endif /* NLD_74113_H_ */
