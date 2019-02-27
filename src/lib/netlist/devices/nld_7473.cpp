// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7473.c
 *
 */

#include "nld_7473.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(7473)
	{
		NETLIB_CONSTRUCTOR(7473)
		, m_CLK(*this, "CLK")
		, m_J(*this, "J")
		, m_K(*this, "K")
		, m_CLRQ(*this, "CLRQ")
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_q(*this, "m_q", 0)
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		logic_input_t m_CLK;
		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_CLRQ;

		state_var<unsigned> m_last_CLK;
		state_var<unsigned> m_q;

		logic_output_t m_Q;
		logic_output_t m_QQ;
	};

	NETLIB_OBJECT_DERIVED(7473A, 7473)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(7473A, 7473) { }

	};

	NETLIB_OBJECT(7473_dip)
	{
		NETLIB_CONSTRUCTOR(7473_dip)
		, m_1(*this, "1")
		, m_2(*this, "2")
		{
			register_subalias("1", m_1.m_CLK);
			register_subalias("2", m_1.m_CLRQ);
			register_subalias("3", m_1.m_K);
			//register_subalias("4", ); ==> VCC
			register_subalias("5", m_2.m_CLK);
			register_subalias("6", m_2.m_CLRQ);
			register_subalias("7", m_2.m_J);

			register_subalias("8", m_2.m_QQ);
			register_subalias("9", m_2.m_Q);
			register_subalias("10", m_2.m_K);
			//register_subalias("11", ); ==> VCC
			register_subalias("12", m_2.m_Q);
			register_subalias("13", m_1.m_QQ);
			register_subalias("14", m_1.m_J);
		}

	private:
		NETLIB_SUB(7473) m_1;
		NETLIB_SUB(7473) m_2;
	};

	NETLIB_OBJECT(7473A_dip)
	{
		NETLIB_CONSTRUCTOR(7473A_dip)
		, m_1(*this, "1")
		, m_2(*this, "2")
		{
			register_subalias("1", m_1.m_CLK);
			register_subalias("2", m_1.m_CLRQ);
			register_subalias("3", m_1.m_K);
			//register_subalias("4", ); ==> VCC
			register_subalias("5", m_2.m_CLK);
			register_subalias("6", m_2.m_CLRQ);
			register_subalias("7", m_2.m_J);

			register_subalias("8", m_2.m_QQ);
			register_subalias("9", m_2.m_Q);
			register_subalias("10", m_2.m_K);
			//register_subalias("11", ); ==> VCC
			register_subalias("12", m_2.m_Q);
			register_subalias("13", m_1.m_QQ);
			register_subalias("14", m_1.m_J);
		}

	private:
		NETLIB_SUB(7473A) m_1;
		NETLIB_SUB(7473A) m_2;
	};

	NETLIB_RESET(7473)
	{
		m_last_CLK = 0;
	}

	NETLIB_UPDATE(7473)
	{
		const auto JK = (m_J() << 1) | m_K();

		if (m_CLRQ())
		{
			if (!m_CLK() && m_last_CLK)
			{
				switch (JK)
				{
					case 1:             // (!m_J) & m_K))
						m_q = 0;
						break;
					case 2:             // (m_J) & !m_K))
						m_q = 1;
						break;
					case 3:             // (m_J) & m_K))
						m_q ^= 1;
						break;
					default:
					case 0:
						break;
				}
			}
		}

		m_last_CLK = m_CLK();

		m_Q.push(m_q, NLTIME_FROM_NS(20)); // FIXME: timing
		m_QQ.push(m_q ^ 1, NLTIME_FROM_NS(20)); // FIXME: timing
	}

	NETLIB_DEVICE_IMPL(7473, "TTL_7473", "+CLK,+J,+K,+CLRQ")
	NETLIB_DEVICE_IMPL(7473A, "TTL_7473A", "+CLK,+J,+K,+CLRQ")
	NETLIB_DEVICE_IMPL(7473_dip, "TTL_7473_DIP", "")
	NETLIB_DEVICE_IMPL(7473A_dip, "TTL_7473A_DIP", "")

	} //namespace devices
} // namespace netlist
