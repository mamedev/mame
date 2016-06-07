// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7474.c
 *
 */

#include "nld_7474.h"

namespace netlist
{
	namespace devices
	{

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

	public:
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

	ATTR_HOT inline void NETLIB_NAME(7474sub)::newstate(const UINT8 stateQ, const UINT8 stateQQ)
	{
		// 0: High-to-low 40 ns, 1: Low-to-high 25 ns
		const netlist_time delay[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };
		OUTLOGIC(m_Q, stateQ, delay[stateQ]);
		OUTLOGIC(m_QQ, stateQQ, delay[stateQQ]);
	}

	NETLIB_UPDATE(7474sub)
	{
		//if (INP_LH(m_CLK))
		{
			newstate(m_nextD, !m_nextD);
			m_CLK.inactivate();
		}
	}

	NETLIB_UPDATE(7474)
	{
		if (INPLOGIC(m_PREQ) && INPLOGIC(m_CLRQ))
		{
			m_D.activate();
			sub.m_nextD = INPLOGIC(m_D);
			sub.m_CLK.activate_lh();
		}
		else if (!INPLOGIC(m_PREQ))
		{
			sub.newstate(1, 0);
			sub.m_CLK.inactivate();
			m_D.inactivate();
		}
		else if (!INPLOGIC(m_CLRQ))
		{
			sub.newstate(0, 1);
			sub.m_CLK.inactivate();
			m_D.inactivate();
		}
		else
		{
			sub.newstate(1, 1);
			sub.m_CLK.inactivate();
			m_D.inactivate();
		}
	}

	NETLIB_RESET(7474)
	{
		sub.do_reset();
	}

	NETLIB_RESET(7474sub)
	{
		m_CLK.set_state(logic_t::STATE_INP_LH);

		m_nextD = 0;
	}

	NETLIB_RESET(7474_dip)
	{
	//	m_1.do_reset();
		//m_2.do_reset();
	}

	NETLIB_UPDATE(7474_dip)
	{
		//m_1.update_dev();
		//m_2.update_dev();
	}

	NETLIB_DEVICE_IMPL(7474)
	NETLIB_DEVICE_IMPL(7474_dip)

	} //namespace devices
} // namespace netlist
