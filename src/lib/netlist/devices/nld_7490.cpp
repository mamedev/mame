// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_7490.cpp
 *
 *  DM7490: Decade Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7490   11| QD
 *      VCC |5           10| GND
 *      R91 |6            9| QB
 *      R92 |7            8| QC
 *          +--------------+
 *
 *          Counter Sequence
 *
 *          +-------++----+----+----+----+
 *          | COUNT || QD | QC | QB | QA |
 *          +=======++====+====+====+====+
 *          |    0  ||  0 |  0 |  0 |  0 |
 *          |    1  ||  0 |  0 |  0 |  1 |
 *          |    2  ||  0 |  0 |  1 |  0 |
 *          |    3  ||  0 |  0 |  1 |  1 |
 *          |    4  ||  0 |  1 |  0 |  0 |
 *          |    5  ||  0 |  1 |  0 |  1 |
 *          |    6  ||  0 |  1 |  1 |  0 |
 *          |    7  ||  0 |  1 |  1 |  1 |
 *          |    8  ||  1 |  0 |  0 |  0 |
 *          |    9  ||  1 |  0 |  0 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note A Output QA is connected to input B for BCD count
 *
 *          Reset Count Function table
 *
 *          +-----+-----+-----+-----++----+----+----+----+
 *          | R01 | R02 | R91 | R92 || QD | QC | QB | QA |
 *          +=====+=====+=====+=====++====+====+====+====+
 *          |  1  |  1  |  0  |  X  ||  0 |  0 |  0 |  0 |
 *          |  1  |  1  |  X  |  0  ||  0 |  0 |  0 |  0 |
 *          |  X  |  X  |  1  |  1  ||  1 |  0 |  0 |  1 |
 *          |  X  |  0  |  X  |  0  ||       COUNT       |
 *          |  0  |  X  |  0  |  X  ||       COUNT       |
 *          |  0  |  X  |  X  |  0  ||       COUNT       |
 *          |  X  |  0  |  0  |  X  ||       COUNT       |
 *          +-----+-----+-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"

namespace netlist::devices {

	static constexpr const std::array<const netlist_time, 4> delay =
	{
			NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(36) - NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(54) - NLTIME_FROM_NS(18),
			NLTIME_FROM_NS(72) - NLTIME_FROM_NS(18)
	};

	NETLIB_OBJECT(7490)
	{
		NETLIB_CONSTRUCTOR(7490)
		, m_A(*this, "A", NETLIB_DELEGATE(inputs))
		, m_B(*this, "B", NETLIB_DELEGATE(inputs))
		, m_R1(*this, "R1", NETLIB_DELEGATE(inputs))
		, m_R2(*this, "R2", NETLIB_DELEGATE(inputs))
		, m_R91(*this, "R91", NETLIB_DELEGATE(inputs))
		, m_R92(*this, "R92", NETLIB_DELEGATE(inputs))
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

			if (m_R91() & m_R92())
			{
				m_cnt = 9;
				m_Q.push(9, delay);
			}
			else if (m_R1() & m_R2())
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
					if (m_cnt >= 10)
						m_cnt &= 1; /* Output A is not reset! */
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
		logic_input_t m_R91;
		logic_input_t m_R92;

		state_var_u8 m_cnt;
		state_var<netlist_sig_t> m_last_A;
		state_var<netlist_sig_t> m_last_B;

		object_array_t<logic_output_t, 4> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7490,     "TTL_7490",        "+A,+B,+R1,+R2,+R91,+R92,@VCC,@GND")

} // namespace netlist::devices
