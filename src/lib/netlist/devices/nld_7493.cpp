// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_7493.cpp
 *
 */

#include "nld_7493.h"
#include "netlist/nl_base.h"
#include "nlid_system.h"

namespace netlist
{
	namespace devices
	{

	static constexpr const netlist_time out_delay = NLTIME_FROM_NS(18);
	static constexpr const netlist_time out_delay2 = NLTIME_FROM_NS(36);
	static constexpr const netlist_time out_delay3 = NLTIME_FROM_NS(54);

	NETLIB_OBJECT(7493)
	{
		NETLIB_CONSTRUCTOR(7493)
		, m_R1(*this, "R1")
		, m_R2(*this, "R2")
		, m_a(*this, "_m_a", 0)
		, m_bcd(*this, "_m_b", 0)
		, m_CLKA(*this, "CLKA", NETLIB_DELEGATE(7493, updA))
		, m_CLKB(*this, "CLKB", NETLIB_DELEGATE(7493, updB))
		, m_QA(*this, "QA")
		, m_QB(*this, "QB")
		, m_QC(*this, "QC")
		, m_QD(*this, "QD")
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

		NETLIB_UPDATEI()
		{
			if (!(m_R1() & m_R2()))
			{
				m_CLKA.activate_hl();
				m_CLKB.activate_hl();
			}
			else
			{
				m_CLKA.inactivate();
				m_CLKB.inactivate();
				m_QA.push(0, NLTIME_FROM_NS(40));
				m_QB.push(0, NLTIME_FROM_NS(40));
				m_QC.push(0, NLTIME_FROM_NS(40));
				m_QD.push(0, NLTIME_FROM_NS(40));
				m_a = m_bcd = 0;
			}
		}

		NETLIB_HANDLERI(updA)
		{
			m_a ^= 1;
			m_QA.push(m_a, out_delay);
		}

		NETLIB_HANDLERI(updB)
		{
			auto cnt = (++m_bcd &= 0x07);
			m_QD.push((cnt >> 2) & 1, out_delay3);
			m_QC.push((cnt >> 1) & 1, out_delay2);
			m_QB.push(cnt & 1, out_delay);
		}

		logic_input_t m_R1;
		logic_input_t m_R2;

		state_var_sig m_a;
		state_var_u8  m_bcd;

		logic_input_t m_CLKA;
		logic_input_t m_CLKB;

		logic_output_t m_QA;
		logic_output_t m_QB;
		logic_output_t m_QC;
		logic_output_t m_QD;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT_DERIVED(7493_dip, 7493)
	{
		NETLIB_CONSTRUCTOR_DERIVED(7493_dip, 7493)
		{
			register_subalias("1", "CLKB");
			register_subalias("2", "R1");
			register_subalias("3", "R2");

			// register_subalias("4", ); --> NC
			register_subalias("5", "VCC");
			// register_subalias("6", ); --> NC
			// register_subalias("7", ); --> NC

			register_subalias("8", "QC");
			register_subalias("9", "QB");
			register_subalias("10", "GND");
			register_subalias("11", "QD");
			register_subalias("12", "QA");
			// register_subalias("13", ); -. NC
			register_subalias("14", "CLKA");
		}
	};


	NETLIB_DEVICE_IMPL(7493,        "TTL_7493", "+CLKA,+CLKB,+R1,+R2,@VCC,@GND")
	NETLIB_DEVICE_IMPL(7493_dip,    "TTL_7493_DIP", "")

	} //namespace devices
} // namespace netlist
