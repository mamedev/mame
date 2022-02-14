// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_4006.cpp
 *
 *  CD4006: CMOS 18-Stage Static Register
 *
 * Description
 *
 * CD4006BMS types are composed of 4 separate shift register sections: two
 * sections of four stages and two sections of five stages with an output tap
 * at the fourth stage. Each section has an independent single-rail data path.
 *
 * A common clock signal is used for all stages. Data are shifted to the next
 * stages on negative-going transitions of the clock. Through appropriate
 * connections of inputs and outputs, multiple register sections of 4, 5, 8,
 * and 9 stages or single register sections of 10, 12, 13, 14, 16, 17 and 18
 * stages can be implemented using one CD4006BMS package. Longer shift register
 * sections can be assembled by using more than one CD4006BMS.
 *
 * To facilitate cascading stages when clock rise and fall times are slow,
 * an optional output (D1 + 4’) that is delayed one-half clockcycle, is
 * provided.
 *
 *            +--------------+
 *       D1   |1     ++    14| VDD
 *      D1+4' |2           13| D1+4
 *      CLOCK |3           12| D2+5
 *         D2 |4    4006   11| D2+4
 *         D3 |5           10| D3+4
 *         D4 |6            9| D4+5
 *        VSS |7            8| D4+4
 *            +--------------+
 *
 *
 *  Naming conventions follow SYC datasheet
 *
 *  FIXME: Timing depends on VDD-VSS - partially done
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	// FIXME: optimize clock input

	NETLIB_OBJECT(CD4006)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4006, "CD4XXX")
		, m_CLOCK(*this, "CLOCK", NETLIB_DELEGATE(inputs))
		, m_I(*this, {"D1", "D2", "D3", "D4"}, NETLIB_DELEGATE(inputs))
		, m_Q(*this, {"D1P4", "D1P4S", "D2P4", "D2P5", "D3P4", "D4P4", "D4P5"})
		, m_d(*this, "m_d", 0)
		, m_last_clock(*this, "m_last_clock", 0)
		, m_tp(*this, "m_tp", netlist_time::from_nsec(200))
		, m_supply(*this, NETLIB_DELEGATE(vdd_vss))
		{
		}

	private:
		NETLIB_HANDLERI(inputs)
		{
			if (m_last_clock && !m_CLOCK())
			{
				m_d[0] >>= 1;
				m_d[1] >>= 1;
				m_d[2] >>= 1;
				m_d[3] >>= 1;
				// falling, output all but D1P4S
				m_Q[0].push(m_d[0] & 1, m_tp);
				m_Q[2].push((m_d[1] >> 1) & 1, m_tp); // D2 + 4
				m_Q[3].push( m_d[1]       & 1, m_tp); // D2 + 5
				m_Q[4].push( m_d[2]       & 1, m_tp); // D3 + 4
				m_Q[5].push((m_d[3] >> 1) & 1, m_tp); // D4 + 4
				m_Q[6].push( m_d[3]       & 1, m_tp); // D5 + 5
				m_last_clock = m_CLOCK();
			}
			else if (!m_last_clock && m_CLOCK())
			{
				// rising, output D1P4S
				m_Q[1].push(m_d[0] & 1, m_tp);
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

		NETLIB_HANDLERI(vdd_vss)
		{
			auto d = m_supply.VCC()() - m_supply.GND()();
			if (d > 0.1) // avoid unrealistic values
			{
				m_tp = netlist_time::from_nsec(gsl::narrow_cast<unsigned>(923.0 / d + 13.0)); // calculated from datasheet
			}
		}

		logic_input_t m_CLOCK;
		object_array_t<logic_input_t, 4>  m_I;
		object_array_t<logic_output_t, 7> m_Q;
		state_container<std::array<uint8_t, 4>> m_d;
		state_var<netlist_sig_t> m_last_clock;
		state_var<netlist_time> m_tp; // propagation time
		nld_power_pins m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4006, "CD4006", "+CLOCK,+D1,+D2,+D3,+D4,+D1P4,+D1P4S,+D2P4,+D2P5,+D3P4,+D4P4,+D4P5,@VCC,@GND")

} // namespace netlist::devices
