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

#include "nl_base.h"

#define TTL_74175(name)                                     \
		NET_REGISTER_DEV(TTL_74175, name)
#define TTL_74175_DIP(name)                                 \
		NET_REGISTER_DEV(TTL_74175_DIP, name)

namespace netlist
{
	namespace devices
	{

NETLIB_OBJECT(74175_sub)
{
	NETLIB_CONSTRUCTOR(74175_sub)
	, m_data(0)
	{
		enregister("CLK",   m_CLK);

		enregister("Q1",   m_Q[0]);
		enregister("Q1Q",  m_QQ[0]);
		enregister("Q2",   m_Q[1]);
		enregister("Q2Q",  m_QQ[1]);
		enregister("Q3",   m_Q[2]);
		enregister("Q3Q",  m_QQ[2]);
		enregister("Q4",   m_Q[3]);
		enregister("Q4Q",  m_QQ[3]);

		save(NLNAME(m_clrq));
		save(NLNAME(m_data));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	logic_input_t m_CLK;
	logic_output_t m_Q[4];
	logic_output_t m_QQ[4];

	netlist_sig_t m_clrq;
	UINT8 m_data;
};

NETLIB_OBJECT(74175)
{
	NETLIB_CONSTRUCTOR(74175)
	, m_sub(*this, "sub")
	{
		register_subalias("CLK",   m_sub.m_CLK);

		enregister("CLRQ",  m_CLRQ);

		enregister("D1",    m_D[0]);
		register_subalias("Q1",   m_sub.m_Q[0]);
		register_subalias("Q1Q",  m_sub.m_QQ[0]);

		enregister("D2",    m_D[1]);
		register_subalias("Q2",   m_sub.m_Q[1]);
		register_subalias("Q2Q",  m_sub.m_QQ[1]);

		enregister("D3",    m_D[2]);
		register_subalias("Q3",   m_sub.m_Q[2]);
		register_subalias("Q3Q",  m_sub.m_QQ[2]);

		enregister("D4",    m_D[3]);
		register_subalias("Q4",   m_sub.m_Q[3]);
		register_subalias("Q4Q",  m_sub.m_QQ[3]);
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

protected:
	NETLIB_SUB(74175_sub) m_sub;
	logic_input_t m_D[4];
	logic_input_t m_CLRQ;
};

NETLIB_OBJECT_DERIVED(74175_dip, 74175)
{
	NETLIB_CONSTRUCTOR_DERIVED(74175_dip, 74175)
	{
		register_subalias("9", m_sub.m_CLK);
		register_subalias("1",  m_CLRQ);

		register_subalias("4",    m_D[0]);
		register_subalias("2",   m_sub.m_Q[0]);
		register_subalias("3",  m_sub.m_QQ[0]);

		register_subalias("5",    m_D[1]);
		register_subalias("7",   m_sub.m_Q[1]);
		register_subalias("6",  m_sub.m_QQ[1]);

		register_subalias("12",    m_D[2]);
		register_subalias("10",   m_sub.m_Q[2]);
		register_subalias("11",  m_sub.m_QQ[2]);

		register_subalias("13",    m_D[3]);
		register_subalias("15",   m_sub.m_Q[3]);
		register_subalias("14",  m_sub.m_QQ[3]);
	}
};

	} //namespace devices
} // namespace netlist

#endif /* NLD_74175_H_ */
