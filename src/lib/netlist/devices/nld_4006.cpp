// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4006.c
 *
 */

#include "nld_4006.h"
#include "nl_base.h"

namespace netlist
{
	namespace devices
	{
	NETLIB_OBJECT(CD4006)
	{
		NETLIB_CONSTRUCTOR(CD4006)
		NETLIB_FAMILY("CD4XXX")
		, m_CLOCK(*this, "CLOCK")
		, m_I(*this, {"D1", "D2", "D3", "D4"})
		, m_Q(*this, {"D1P4", "D1P4S", "D2P4", "D2P5", "D3P4", "D4P4", "D3P5"})
		, m_d(*this, "m_d", 0)
		, m_last_clock(*this, "m_last_clock", 0)
		, m_supply(*this, "VDD", "VSS")
		{
		}

		NETLIB_RESETI()
		{
		}

		NETLIB_UPDATEI()
		{
			if (m_last_clock && !m_CLOCK())
			{
				m_d[0] >>= 1;
				m_d[1] >>= 1;
				m_d[2] >>= 1;
				m_d[3] >>= 1;
				// falling, output all but D1P4S
				m_Q[0].push(m_d[0] & 1, netlist_time::from_nsec(200));
				m_Q[2].push((m_d[1] >> 1) & 1, netlist_time::from_nsec(200)); // D2 + 4
				m_Q[3].push( m_d[1]       & 1, netlist_time::from_nsec(200)); // D2 + 5
				m_Q[4].push( m_d[2]       & 1, netlist_time::from_nsec(200)); // D3 + 4
				m_Q[5].push((m_d[3] >> 1) & 1, netlist_time::from_nsec(200)); // D4 + 4
				m_Q[6].push( m_d[3]       & 1, netlist_time::from_nsec(200)); // D5 + 5
				m_last_clock = m_CLOCK();
			}
			else if (!m_last_clock && m_CLOCK())
			{
				// rising, output D1P4S
				m_Q[1].push(m_d[0] & 1, netlist_time::from_nsec(200));
				m_last_clock = m_CLOCK();
			}
			else
			{
				m_d[0] = static_cast<uint8_t>((m_d[0] & 0x0f) | (m_I[0]() << 4));
				m_d[1] = static_cast<uint8_t>((m_d[1] & 0x1f) | (m_I[1]() << 5));
				m_d[2] = static_cast<uint8_t>((m_d[2] & 0x0f) | (m_I[2]() << 4));
				m_d[3] = static_cast<uint8_t>((m_d[3] & 0x1f) | (m_I[3]() << 5));
			}
		}

	protected:
		logic_input_t m_CLOCK;
		object_array_t<logic_input_t, 4>  m_I;
		object_array_t<logic_output_t, 7> m_Q;
		state_container<std::array<uint8_t, 4>> m_d;
		state_var<netlist_sig_t> m_last_clock;
		nld_power_pins m_supply;
	};

	NETLIB_OBJECT_DERIVED(CD4006_dip, CD4006)
	{
		NETLIB_CONSTRUCTOR_DERIVED(CD4006_dip, CD4006)
		{
			register_subalias("1", m_I[0]);
			register_subalias("2", m_Q[1]);
			register_subalias("3", m_CLOCK);
			register_subalias("4", m_I[1]);
			register_subalias("5", m_I[2]);
			register_subalias("6", m_I[3]);
			register_subalias("7", "VSS");

			register_subalias("8", m_Q[5]);
			register_subalias("9", m_Q[6]);
			register_subalias("10", m_Q[4]);
			register_subalias("11", m_Q[2]);
			register_subalias("12", m_Q[3]);
			register_subalias("13", m_Q[0]);
			register_subalias("14", "VDD");

		}
	};

	NETLIB_DEVICE_IMPL(CD4006, "CD4006", "+CLOCK,+D1,+D2,+D3,+D4,+D1P4,+D1P4S,+D2P4,+D2P5,+D3P4,+D4P4,+D3P5,@VCC,@GND")
	NETLIB_DEVICE_IMPL(CD4006_dip, "CD4006_DIP", "")

	} //namespace devices
} // namespace netlist
