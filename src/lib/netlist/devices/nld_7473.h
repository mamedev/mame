// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7473.h
 *
 *  DM7473: Dual Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *  DM7473A: Dual Negative-Edge-Triggered Master-Slave J-K Flip-Flops with Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLK1 |1     ++    14| J1
 *     CLR1 |2           13| QQ1
 *       K1 |3           12| Q1
 *      VCC |4    7473   11| GND
 *     CLK2 |5           10| K2
 *     CLR2 |6            9| Q2
 *       J2 |7            8| QQ2
 *          +--------------+
 *
 *
 *          Function table 73
 *
 *          +-----+-----+-----+---++---+-----+
 *          | CLR | CLK |  J  | K || Q | QQ  |
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
 *          | CLR | CLK |  J  | K || Q | QQ  |
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
 *  Naming conventions follow Fairchild Semiconductor datasheet
 *
 */

#ifndef NLD_7473_H_
#define NLD_7473_H_

#include "nl_setup.h"

#define TTL_7473A(name, cCLK, cJ, cK, cCLR)	\
		NET_REGISTER_DEV(TTL_7473A, name)	\
		NET_CONNECT(name, CLK, cCLK)		\
		NET_CONNECT(name, J, cJ)                                                \
		NET_CONNECT(name, K, cK)                                                \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74107(name, cCLK, cJ, cK, cCLRQ)                                    \
		TTL_74107A(name, cCLK, cJ, cK, cCLRQ)

#define TTL_74107_DIP(name)                                                     \
		NET_REGISTER_DEV(TTL_74107_DIP, name)

#endif /* NLD_74107_H_ */
