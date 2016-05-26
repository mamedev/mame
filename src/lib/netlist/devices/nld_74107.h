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

#include "nl_base.h"

#define TTL_74107A(name, cCLK, cJ, cK, cCLRQ)                                      \
		NET_REGISTER_DEV(TTL_74107A, name)                                             \
		NET_CONNECT(name, CLK, cCLK)                                               \
		NET_CONNECT(name, J, cJ)                                                  \
		NET_CONNECT(name, K, cK)                                                  \
		NET_CONNECT(name, CLRQ, cCLRQ)

#define TTL_74107(name, cCLK, cJ, cK, cCLRQ)                                       \
		TTL_74107A(name, cCLK, cJ, cK, cCLRQ)

#define TTL_74107_DIP(name)                                                         \
		NET_REGISTER_DEV(TTL_74107_DIP, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(74107Asub)
{
	NETLIB_CONSTRUCTOR(74107Asub)
	, m_Q1(0)
	, m_Q2(0)
	, m_F(0)

	{
		enregister("CLK", m_clk);
		enregister("Q", m_Q);
		enregister("QQ", m_QQ);

		save(NLNAME(m_Q1));
		save(NLNAME(m_Q2));
		save(NLNAME(m_F));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	logic_input_t m_clk;

	logic_output_t m_Q;
	logic_output_t m_QQ;

	netlist_sig_t m_Q1;
	netlist_sig_t m_Q2;
	netlist_sig_t m_F;

	ATTR_HOT void newstate(const netlist_sig_t state);

};

NETLIB_OBJECT(74107A)
{
	NETLIB_CONSTRUCTOR(74107A)
	, m_sub(*this, "sub")
	{

		register_subalias("CLK", m_sub.m_clk);
		enregister("J", m_J);
		enregister("K", m_K);
		enregister("CLRQ", m_clrQ);
		register_subalias("Q", m_sub.m_Q);
		register_subalias("QQ", m_sub.m_QQ);
	}

	//NETLIB_RESETI();
	NETLIB_UPDATEI();
public:
	NETLIB_SUB(74107Asub) m_sub;

	logic_input_t m_J;
	logic_input_t m_K;
	logic_input_t m_clrQ;

};

NETLIB_OBJECT_DERIVED(74107, 74107A)
{
public:
	NETLIB_CONSTRUCTOR_DERIVED(74107, 74107A) { }

};

NETLIB_OBJECT(74107_dip)
{
	NETLIB_CONSTRUCTOR(74107_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	{

		register_subalias("1", m_1.m_J);
		register_subalias("2", m_1.m_sub.m_QQ);
		register_subalias("3", m_1.m_sub.m_Q);

		register_subalias("4", m_1.m_K);
		register_subalias("5", m_2.m_sub.m_Q);
		register_subalias("6", m_2.m_sub.m_QQ);

		// register_subalias("7", ); ==> GND

		register_subalias("8", m_2.m_J);
		register_subalias("9", m_2.m_sub.m_clk);
		register_subalias("10", m_2.m_clrQ);

		register_subalias("11", m_2.m_K);
		register_subalias("12", m_1.m_sub.m_clk);
		register_subalias("13", m_1.m_clrQ);

		// register_subalias("14", ); ==> VCC

	}
	//NETLIB_RESETI();
	//NETLIB_UPDATEI();

private:
	NETLIB_SUB(74107) m_1;
	NETLIB_SUB(74107) m_2;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_74107_H_ */
