// license:GPL-2.0+
// copyright-holders:Ryan Holtz
/*
 * nld_7442.cpp
 *
 */

#include "nld_7442.h"
#include "netlist/nl_base.h"

namespace netlist
{
namespace devices
{
	static C14CONSTEXPR const netlist_time delay = NLTIME_FROM_NS(30); // Worst-case through 3 levels of logic

	NETLIB_OBJECT(7442)
	{
		NETLIB_CONSTRUCTOR(7442)
		, m_A(*this, "A")
		, m_B(*this, "B")
		, m_C(*this, "C")
		, m_D(*this, "D")
		, m_val(*this, "m_val", 0)
		, m_last_val(*this, "m_last_val", 0)
		, m_Q(*this, {"Q0", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8", "Q9"})
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_UPDATEI();
		NETLIB_RESETI();

		void update_outputs() noexcept
		{
			for (std::size_t i=0; i<10; i++)
				m_Q[i].push(i == m_val ? 0 : 1, delay);
		}

		logic_input_t m_A;
		logic_input_t m_B;
		logic_input_t m_C;
		logic_input_t m_D;

		state_var_u8 m_val;
		state_var_u8 m_last_val;

		object_array_t<logic_output_t, 10> m_Q;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(7442_dip)
	{
		NETLIB_CONSTRUCTOR(7442_dip)
		, A(*this, "A")
		{
			register_subalias("1", "A.Q0");
			register_subalias("2", "A.Q1");
			register_subalias("3", "A.Q2");
			register_subalias("4", "A.Q3");
			register_subalias("5", "A.Q4");
			register_subalias("6", "A.Q5");
			register_subalias("7", "A.Q6");
			register_subalias("8", "A.GND");

			register_subalias("9", "A.Q7");
			register_subalias("10", "A.Q8");
			register_subalias("11", "A.Q9");
			register_subalias("12", "A.D");
			register_subalias("13", "A.C");
			register_subalias("14", "A.B");
			register_subalias("15", "A.A");
			register_subalias("16", "A.VCC");
		}
		NETLIB_RESETI() {}
		NETLIB_UPDATEI() {}
	private:
		NETLIB_SUB(7442) A;
	};

	NETLIB_RESET(7442)
	{
		m_val = 0;
		m_last_val = 0;
	}

	NETLIB_UPDATE(7442)
	{
		const netlist_sig_t new_A = m_A();
		const netlist_sig_t new_B = m_B();
		const netlist_sig_t new_C = m_C();
		const netlist_sig_t new_D = m_D();

		m_last_val = m_val;
		m_val = static_cast<uint8_t>((new_D << 3) | (new_C << 2) | (new_B << 1) | new_A);

		if (m_last_val != m_val)
		{
			update_outputs();
		}
	}

	NETLIB_DEVICE_IMPL(7442,     "TTL_7442",        "+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7442_dip, "TTL_7442_DIP",    "")

} // namespace devices
} // namespace netlist
