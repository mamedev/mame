// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7492.cpp
 *
 */

#include "nld_7492.h"
#include "netlist/nl_base.h"

namespace netlist
{
namespace devices
{

	static constexpr const std::array<netlist_time, 4> delay =
	{
			NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(36) - NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(54) - NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(72) - NLTIME_FROM_NS(18)
	};

	NETLIB_OBJECT(7492)
	{
		NETLIB_CONSTRUCTOR(7492)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_R1(*this, "R1", NETLIB_DELEGATE(inputs))
		, m_R2(*this, "R2", NETLIB_DELEGATE(inputs))
		, m_cnt(*this, "m_cnt", 0)
		, m_last_A(*this, "m_last_A", 0)
		, m_last_B(*this, "m_last_B", 0)
		, m_Q(*this, {"QA", "QB", "QC", "QD"})
		, m_power_pins(*this)
		{
		}

	private:

		NETLIB_HANDLERI(inputs)
		{
			const netlist_sig_t new_A = m_A();
			const netlist_sig_t new_B = m_B();

			if (m_R1() & m_R2())
			{
				m_cnt = 0;
				m_Q.push(0, delay);
			}
			else
			{
				if (m_last_A && !new_A)  // High - Low
				{
					m_cnt ^= 1;
					m_Q[0].push(m_cnt & 1, delay[0]);
				}
				if (m_last_B && !new_B)  // High - Low
				{
					m_cnt += 2;
					if (m_cnt == 6)
						m_cnt = 8;
					if (m_cnt == 14)
						m_cnt = 0;
					m_Q.push(m_cnt, delay);
				}
			}
			m_last_A = new_A;
			m_last_B = new_B;
		}

		NETLIB_RESETI()
		{
			m_cnt = 0;
			m_last_A = 0;
			m_last_B = 0;
		}

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_R1;
		logic_input_t m_R2;

		state_var_u8 m_cnt;
		state_var<netlist_sig_t> m_last_A;
		state_var<netlist_sig_t> m_last_B;

		object_array_t<logic_output_t, 4> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7492,     "TTL_7492",        "+A,+B,+R1,+R2,@VCC,@GND")

} // namespace devices
} // namespace netlist
