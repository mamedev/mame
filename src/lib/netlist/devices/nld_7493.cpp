// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_7493.cpp
 *
 *  DM7493: Binary Counters
 *
 *          +--------------+
 *        B |1     ++    14| A
 *      R01 |2           13| NC
 *      R02 |3           12| QA
 *       NC |4    7493   11| QD
 *      VCC |5           10| GND
 *       NC |6            9| QB
 *       NC |7            8| QC
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
 *          |   10  ||  1 |  0 |  1 |  0 |
 *          |   11  ||  1 |  0 |  1 |  1 |
 *          |   12  ||  1 |  1 |  0 |  0 |
 *          |   13  ||  1 |  1 |  0 |  1 |
 *          |   14  ||  1 |  1 |  1 |  0 |
 *          |   15  ||  1 |  1 |  1 |  1 |
 *          +-------++----+----+----+----+
 *
 *          Note C Output QA is connected to input B
 *
 *          Reset Count Function table
 *
 *          +-----+-----++----+----+----+----+
 *          | R01 | R02 || QD | QC | QB | QA |
 *          +=====+=====++====+====+====+====+
 *          |  1  |  1  ||  0 |  0 |  0 |  0 |
 *          |  0  |  X  ||       COUNT       |
 *          |  X  |  0  ||       COUNT       |
 *          +-----+-----++----+----+----+----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 */

#include "nl_base.h"


namespace netlist::devices {

	static constexpr const std::array<const netlist_time, 3> out_delay { NLTIME_FROM_NS(18), NLTIME_FROM_NS(36), NLTIME_FROM_NS(54) };

	NETLIB_OBJECT(7493)
	{
		NETLIB_CONSTRUCTOR(7493)
		, m_CLKA(*this, "CLKA", NETLIB_DELEGATE(updA))
		, m_CLKB(*this, "CLKB", NETLIB_DELEGATE(updB))
		, m_QA(*this, "QA")
		, m_QBCD(*this, {"QB", "QC", "QD"})
		, m_a(*this, "m_a", 0)
		, m_bcd(*this, "m_b", 0)
		, m_R1(*this, "R1", NETLIB_DELEGATE(inputs))
		, m_R2(*this, "R2", NETLIB_DELEGATE(inputs))
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_a = m_bcd = 0;
			m_CLKA.set_state(logic_t::STATE_INP_HL);
			m_CLKB.set_state(logic_t::STATE_INP_HL);
		}

		NETLIB_HANDLERI(inputs)
		{
			if (!(m_R1() && m_R2()))
			{
				m_CLKA.activate_hl();
				m_CLKB.activate_hl();
			}
			else
			{
				m_CLKA.inactivate();
				m_CLKB.inactivate();
				m_QA.push(0, NLTIME_FROM_NS(40));
				m_QBCD.push(0, NLTIME_FROM_NS(40));
				m_a = m_bcd = 0;
			}
		}

		NETLIB_HANDLERI(updA)
		{
			m_a ^= 1;
			m_QA.push(m_a, out_delay[0]);
		}

		NETLIB_HANDLERI(updB)
		{
			const auto cnt(++m_bcd &= 0x07);
			m_QBCD.push(cnt, out_delay);
		}

		logic_input_t m_CLKA;
		logic_input_t m_CLKB;

		logic_output_t m_QA;
		object_array_t<logic_output_t, 3> m_QBCD;

		state_var<unsigned> m_a;
		state_var<unsigned> m_bcd;

		logic_input_t m_R1;
		logic_input_t m_R2;

		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7493,        "TTL_7493", "+CLKA,+CLKB,+R1,+R2,@VCC,@GND")

} // namespace netlist::devices
