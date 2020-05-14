// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74377.c
 *
 */

#include "nld_74377.h"
#include "netlist/nl_base.h"

namespace netlist
{
	namespace devices
	{

	constexpr const std::array<netlist_time, 2> delay = { NLTIME_FROM_NS(25), NLTIME_FROM_NS(25) };

	NETLIB_OBJECT(74377_GATE)
	{
		NETLIB_CONSTRUCTOR(74377_GATE)
		, m_E(*this, "E")
		, m_D(*this, "D")
		, m_CP(*this, "CP")
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_cp(*this, "m_cp", 0)
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
		}
		NETLIB_UPDATEI()
		{
			netlist_sig_t last_cp = m_cp;

			m_cp = m_CP();

			if (!m_E() && !last_cp && m_cp)
			{
				netlist_sig_t d = m_D();
				m_Q.push(d, delay[d]);
				m_QQ.push(d ^ 1, delay[d ^ 1]);
			}
		}

	private:
		logic_input_t m_E;
		logic_input_t m_D;
		logic_input_t m_CP;
		logic_output_t m_Q;
		logic_output_t m_QQ;

		state_var_sig m_cp;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(74377_GATE, "TTL_74377_GATE", "")

	} //namespace devices
} // namespace netlist
