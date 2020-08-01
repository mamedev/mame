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
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(inputs))
		, m_J(*this, "J", NETLIB_DELEGATE(inputs))
		, m_K(*this, "K", NETLIB_DELEGATE(inputs))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(inputs))
		, m_last_CLK(*this, "m_last_CLK", 0)
		, m_q(*this, "m_q", 0)
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_last_CLK = 0;
		}

	public:
		NETLIB_HANDLERI(inputs)
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

		logic_input_t m_CLK;
		logic_input_t m_J;
		logic_input_t m_K;
		logic_input_t m_CLRQ;

		state_var<unsigned> m_last_CLK;
		state_var<unsigned> m_q;

		logic_output_t m_Q;
		logic_output_t m_QQ;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(7473A, 7473)
	{
	public:
		NETLIB_CONSTRUCTOR(7473A) { }

	};

	NETLIB_DEVICE_IMPL(7473, "TTL_7473", "+CLK,+J,+K,+CLRQ,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7473A, "TTL_7473A", "+CLK,+J,+K,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
