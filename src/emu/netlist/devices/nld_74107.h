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
 *          transferred when CLK falls.
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
 *  FIXME: Currently, only the 107A is implemented.
 *         The 107 uses the same model.
 *
 */

#ifndef NLD_74107_H_
#define NLD_74107_H_

#include "../nl_base.h"

#define TTL_74107A(_name, _CLK, _J, _K, _CLRQ)                                      \
		NET_REGISTER_DEV(TTL_74107A, _name)                                             \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, J,  _J)                                                  \
		NET_CONNECT(_name, K,  _K)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)

#define TTL_74107(_name, _CLK, _J, _K, _CLRQ)                                       \
		TTL_74107A(_name, _CLK, _J, _K, _CLRQ)

#define TTL_74107_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_74107_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_SUBDEVICE(74107Asub,
	logic_input_t m_clk;

	logic_output_t m_Q;
	logic_output_t m_QQ;

	netlist_sig_t m_Q1;
	netlist_sig_t m_Q2;
	netlist_sig_t m_F;

	ATTR_HOT void newstate(const netlist_sig_t state);

);

NETLIB_DEVICE(74107A,
public:
	NETLIB_NAME(74107Asub) sub;

	logic_input_t m_J;
	logic_input_t m_K;
	logic_input_t m_clrQ;

);

class NETLIB_NAME(74107) : public NETLIB_NAME(74107A)
{
public:
	NETLIB_NAME(74107) ()
	:   NETLIB_NAME(74107A) () {}

};

NETLIB_DEVICE(74107_dip,

	NETLIB_NAME(74107) m_1;
	NETLIB_NAME(74107) m_2;
);

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74107_H_ */
