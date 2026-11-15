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

	// clkA->QA can be slower than the ripple stages on real silicon: TI 1976
	// databook gives 46 ns typ for the 7493A. The 18 ns default keeps the
	// stock race behaviour; netlists where QA must lose a race (a gated-NAND
	// latch on the same clock edge glitching downstream clock lines) set
	// QA_DELAY per instance.
	static constexpr const std::array<const netlist_time, 3> out_delay { NLTIME_FROM_NS(18), NLTIME_FROM_NS(36), NLTIME_FROM_NS(54) };

	NETLIB_OBJECT(7493)
	{
		NETLIB_CONSTRUCTOR(7493)
		, m_CLKA(*this, "CLKA", NETLIB_DELEGATE(inputs))
		, m_CLKB(*this, "CLKB", NETLIB_DELEGATE(inputs))
		, m_QA(*this, "QA")
		, m_QB(*this, "QB")
		, m_QC(*this, "QC")
		, m_QD(*this, "QD")
		, m_a(*this, "m_a", 0)
		, m_bcd(*this, "m_b", 0)
		, m_last_A(*this, "m_lA", 0)
		, m_last_B(*this, "m_lB", 0)
		, m_qa_delay(*this, "QA_DELAY", 18.0)
		, m_R1(*this, "R1", NETLIB_DELEGATE(inputs))
		, m_R2(*this, "R2", NETLIB_DELEGATE(inputs))
		, m_power_pins(*this)
		{
		}

	private:
	NETLIB_RESETI()
	{
		m_a = m_bcd = 0;
		m_last_A = 0;
		m_last_B = 0;
	}

	// level-tracked inputs following the proven nld_7490 pattern.
	// edge-triggered delegates lost QC/QD when clocked from another
	// counter's delayed output net.
	NETLIB_HANDLERI(inputs)
	{
		const auto new_A(m_CLKA());
		const auto new_B(m_CLKB());
		if (m_R1() & m_R2())
		{
			m_a = 0;
			m_bcd = 0;
			const auto delay(NLTIME_FROM_NS(40));
			m_QA.push(0, delay);
			m_QB.push(0, delay);
			m_QC.push(0, delay);
			m_QD.push(0, delay);
		}
		else
		{
			if (m_last_A && !new_A)  // High - Low
			{
				m_a ^= 1;
				m_QA.push(m_a, NLTIME_FROM_NS(static_cast<unsigned>(m_qa_delay())));
			}
			if (m_last_B && !new_B)  // High - Low
			{
				const auto cnt(++m_bcd &= 0x07);
				m_QB.push(cnt & 1, out_delay[0]);
				m_QC.push((cnt >> 1) & 1, out_delay[1]);
				m_QD.push((cnt >> 2) & 1, out_delay[2]);
			}
		}
		m_last_A = new_A;
		m_last_B = new_B;
	}

		logic_input_t m_CLKA;
		logic_input_t m_CLKB;

		logic_output_t m_QA;
		logic_output_t m_QB;
		logic_output_t m_QC;
		logic_output_t m_QD;

		state_var<unsigned> m_a;
		state_var<unsigned> m_bcd;
		state_var<unsigned> m_last_A;
		state_var<unsigned> m_last_B;

		param_fp_t m_qa_delay;

		logic_input_t m_R1;
		logic_input_t m_R2;

		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(7493,        "TTL_7493", "+CLKA,+CLKB,+R1,+R2,@VCC,@GND")

} // namespace netlist::devices
