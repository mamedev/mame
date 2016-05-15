// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7474.h
 *
 *  DM7474: Dual Positive-Edge-Triggered D Flip-Flops
 *          with Preset, Clear and Complementary Outputs
 *
 *          +--------------+
 *     CLR1 |1     ++    14| VCC
 *       D1 |2           13| CLR2
 *     CLK1 |3           12| D2
 *      PR1 |4    7474   11| CLK2
 *       Q1 |5           10| PR2
 *      Q1Q |6            9| Q2
 *      GND |7            8| Q2Q
 *          +--------------+
 *
 *          +-----+-----+-----+---++---+-----+
 *          | PR  | CLR | CLK | D || Q | QQ  |
 *          +=====+=====+=====+===++===+=====+
 *          |  0  |  1  |  X  | X || 1 |  0  |
 *          |  1  |  0  |  X  | X || 0 |  1  |
 *          |  0  |  0  |  X  | X || 1 |  1  | (*)
 *          |  1  |  1  |  R  | 1 || 1 |  0  |
 *          |  1  |  1  |  R  | 0 || 0 |  1  |
 *          |  1  |  1  |  0  | X || Q0| Q0Q |
 *          +-----+-----+-----+---++---+-----+
 *
 *  (*) This configuration is not stable, i.e. it will not persist
 *  when either the preset and or clear inputs return to their inactive (high) level
 *
 *  Q0 The output logic level of Q before the indicated input conditions were established
 *
 *  R:  0 -. 1
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  FIXME: Check that (*) is emulated properly
 */

#ifndef NLD_7474_H_
#define NLD_7474_H_

#include "nl_base.h"

#define TTL_7474(_name, _CLK, _D, _CLRQ, _PREQ)                                     \
		NET_REGISTER_DEV(TTL_7474, _name)                                               \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, D,  _D)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)                                            \
		NET_CONNECT(_name, PREQ,  _PREQ)

#define TTL_7474_DIP(_name)                                                         \
		NET_REGISTER_DEV(TTL_7474_DIP, _name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(7474sub)
{
	NETLIB_CONSTRUCTOR(7474sub)
	, m_nextD(0)
	{
		enregister("CLK",  m_CLK);

		enregister("Q",   m_Q);
		enregister("QQ",  m_QQ);

		save(NLNAME(m_nextD));
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

	logic_input_t m_CLK;
	logic_output_t m_Q;
	logic_output_t m_QQ;
	INT8 m_nextD;

	ATTR_HOT inline void newstate(const UINT8 stateQ, const UINT8 stateQQ);

private:

};

NETLIB_OBJECT(7474)
{
	NETLIB_CONSTRUCTOR(7474)
	, sub(*this, "sub")
	{
		register_subalias("CLK",    sub.m_CLK);
		enregister("D",         m_D);
		enregister("CLRQ",      m_CLRQ);
		enregister("PREQ",      m_PREQ);

		register_subalias("Q",      sub.m_Q);
		register_subalias("QQ",     sub.m_QQ);
	}

	NETLIB_RESETI();
	NETLIB_UPDATEI();

public:
	NETLIB_SUB(7474sub) sub;

	logic_input_t m_D;
	logic_input_t m_CLRQ;
	logic_input_t m_PREQ;
};

NETLIB_OBJECT(7474_dip)
{
	NETLIB_CONSTRUCTOR(7474_dip)
	, m_1(*this, "1")
	, m_2(*this, "2")
	{

		register_subalias("1", m_1.m_CLRQ);
		register_subalias("2", m_1.m_D);
		register_subalias("3", m_1.sub.m_CLK);
		register_subalias("4", m_1.m_PREQ);
		register_subalias("5", m_1.sub.m_Q);
		register_subalias("6", m_1.sub.m_QQ);
		// register_subalias("7", ); ==> GND

		register_subalias("8", m_2.sub.m_QQ);
		register_subalias("9", m_2.sub.m_Q);
		register_subalias("10", m_2.m_PREQ);
		register_subalias("11", m_2.sub.m_CLK);
		register_subalias("12", m_2.m_D);
		register_subalias("13", m_2.m_CLRQ);
		// register_subalias("14", ); ==> VCC
	}
	NETLIB_UPDATEI();
	NETLIB_RESETI();

private:
	NETLIB_SUB(7474) m_1;
	NETLIB_SUB(7474) m_2;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_7474_H_ */
