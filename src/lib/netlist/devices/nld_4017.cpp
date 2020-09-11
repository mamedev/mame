// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4017.cpp
 *
 *  CD4017: Decade Counter/Divider with 10 Decoded Outputs
 *
 *          +--------------+
 *       Q5 |1     ++    16| VDD
 *       Q1 |2           15| RESET
 *       Q0 |3           14| CLOCK
 *       Q2 |4    4017   13| CLOCK ENABLE
 *       Q6 |5           12| CARRY OUT
 *       Q7 |6           11| Q9
 *       Q3 |7           10| Q4
 *      VSS |8            9| Q8
 *          +--------------+
 *
 *
 *  CD4022: Divide-by-8 Counter/Divider with 8 Decoded Outputs
 *
 *          +--------------+
 *       Q1 |1     ++    16| VDD
 *       Q0 |2           15| RESET
 *       Q2 |3           14| CLOCK
 *       Q5 |4    4022   13| CLOCK ENABLE
 *       Q6 |5           12| CARRY OUT
 *       NC |6           11| Q4
 *       Q3 |7           10| Q7
 *      VSS |8            9| NC
 *          +--------------+
 *
 *  Naming conventions follow Fairchild datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *         This needs a cmos d-a/a-d proxy implementation.
 *
 */

#include "nld_4017.h"
#include "nl_base.h"
#include "nl_factory.h"

namespace netlist
{
	namespace devices
	{

	template <std::size_t MaxCount>
	NETLIB_OBJECT(CD4017_base)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4017_base, "CD4XXX")
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_CLKEN(*this, "CLKEN", NETLIB_DELEGATE(inputs))
		, m_RESET(*this, "RESET", NETLIB_DELEGATE(inputs))
		, m_CO(*this, "CO")
		, m_Q(*this, 0, "Q{}")
		, m_cnt(*this, "m_cnt", 0)
		, m_supply(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_cnt = 0;
		}

		NETLIB_HANDLERI(clk)
		{
			++m_cnt;
			update_outputs(m_cnt);
		}

		NETLIB_HANDLERI(inputs)
		{
			if (m_RESET())
			{
				m_CLK.inactivate();
				m_cnt = 0;
				update_outputs(m_cnt);
			}
			else if (m_CLKEN())
			{
				m_CLK.inactivate();
			}
			else
			{
				m_CLK.activate_lh();
			}
		}

		void update_outputs(const unsigned cnt) noexcept
		{
			for (std::size_t i = 0; i < MaxCount; i++)
				m_Q[i].push(i == cnt, NLTIME_FROM_NS(200));
			m_CO.push(cnt < MaxCount / 2, NLTIME_FROM_NS(160));
		}
		logic_input_t m_CLK;
		logic_input_t m_CLKEN;
		logic_input_t m_RESET;
		logic_output_t m_CO;
		object_array_t<logic_output_t, MaxCount> m_Q;

		state_var<unsigned> m_cnt;
		nld_power_pins m_supply;
	};

	using NETLIB_NAME(CD4017) = NETLIB_NAME(CD4017_base)<10>;
	using NETLIB_NAME(CD4022) = NETLIB_NAME(CD4017_base)<8>;


	NETLIB_DEVICE_IMPL(CD4017, "CD4017", "")
	NETLIB_DEVICE_IMPL(CD4022, "CD4022", "")

	} //namespace devices
} // namespace netlist
