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

	// FIXME: optimize clock input

	NETLIB_OBJECT(CD4006)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4006, "CD4XXX")
		, m_CLOCK(*this, "CLOCK", NETLIB_DELEGATE(inputs))
		, m_I(*this, {"D1", "D2", "D3", "D4"}, NETLIB_DELEGATE(inputs))
		, m_Q(*this, {"D1P4", "D1P4S", "D2P4", "D2P5", "D3P4", "D4P4", "D4P5"})
		, m_d(*this, "m_d", 0)
		, m_last_clock(*this, "m_last_clock", 0)
		, m_supply(*this)
		{
		}

		NETLIB_HANDLERI(inputs)
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

		friend class NETLIB_NAME(CD4006_dip);
	private:
		logic_input_t m_CLOCK;
		object_array_t<logic_input_t, 4>  m_I;
		object_array_t<logic_output_t, 7> m_Q;
		state_container<std::array<uint8_t, 4>> m_d;
		state_var<netlist_sig_t> m_last_clock;
		nld_power_pins m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4006, "CD4006", "+CLOCK,+D1,+D2,+D3,+D4,+D1P4,+D1P4S,+D2P4,+D2P5,+D3P4,+D4P4,+D4P5,@VCC,@GND")

	} //namespace devices
} // namespace netlist
