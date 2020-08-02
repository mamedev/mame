// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74174.cpp
 *
 */

#include "nld_74174.h"
#include "netlist/nl_base.h"

namespace netlist
{
namespace devices
{
	NETLIB_OBJECT(74174_GATE)
	{
		NETLIB_CONSTRUCTOR(74174_GATE)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, "Q")
		, m_clrq(*this, "m_clr", 0)
		, m_data(*this, "m_data", 0)
		, m_D(*this, "D", NETLIB_DELEGATE(other))
		, m_CLRQ(*this, "CLRQ", NETLIB_DELEGATE(other))
		, m_power_pins(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_CLK.set_state(logic_t::STATE_INP_LH);
			m_clrq = 0;
			m_data = 0xFF;
		}

		NETLIB_HANDLERI(other)
		{
			netlist_sig_t d = m_D();
			m_clrq = m_CLRQ();
			if (!m_clrq)
			{
				m_Q.push(0, NLTIME_FROM_NS(40));
				m_data = 0;
			} else if (d != m_data)
			{
				m_data = d;
				m_CLK.activate_lh();
			}
		}

	private:
		NETLIB_HANDLERI(clk)
		{
			if (m_clrq)
			{
				m_Q.push(m_data, NLTIME_FROM_NS(25));
				m_CLK.inactivate();
			}
		}

		logic_input_t m_CLK;
		logic_output_t m_Q;

		state_var<netlist_sig_t> m_clrq;
		state_var<netlist_sig_t> m_data;

		logic_input_t m_D;
		logic_input_t m_CLRQ;
		nld_power_pins m_power_pins;
	};

	NETLIB_OBJECT(74174)
	{
		NETLIB_CONSTRUCTOR(74174)
		, A(*this, "A")
		, B(*this, "B")
		, C(*this, "C")
		, D(*this, "D")
		, E(*this, "E")
		, F(*this, "F")
		{
			register_subalias("CLRQ", "A.CLRQ");
			connect("A.CLRQ", "B.CLRQ");
			connect("A.CLRQ", "C.CLRQ");
			connect("A.CLRQ", "D.CLRQ");
			connect("A.CLRQ", "E.CLRQ");
			connect("A.CLRQ", "F.CLRQ");

			register_subalias("CLK", "A.CLK");
			connect("A.CLK", "B.CLK");
			connect("A.CLK", "C.CLK");
			connect("A.CLK", "D.CLK");
			connect("A.CLK", "E.CLK");
			connect("A.CLK", "F.CLK");

			register_subalias("D1", "A.D");
			register_subalias("Q1", "A.Q");

			register_subalias("D2", "B.D");
			register_subalias("Q2", "B.Q");

			register_subalias("D3", "C.D");
			register_subalias("Q3", "C.Q");

			register_subalias("D4", "D.D");
			register_subalias("Q4", "D.Q");

			register_subalias("D5", "E.D");
			register_subalias("Q5", "E.Q");

			register_subalias("D6", "F.D");
			register_subalias("Q6", "F.Q");

			register_subalias("GND", "A.GND");
			connect("A.GND", "B.GND");
			connect("A.GND", "C.GND");
			connect("A.GND", "D.GND");
			connect("A.GND", "E.GND");
			connect("A.GND", "F.GND");

			register_subalias("VCC", "A.VCC");
			connect("A.VCC", "B.VCC");
			connect("A.VCC", "C.VCC");
			connect("A.VCC", "D.VCC");
			connect("A.VCC", "E.VCC");
			connect("A.VCC", "F.VCC");
		}
		//NETLIB_RESETI() {}
	private:
		NETLIB_SUB(74174_GATE) A;
		NETLIB_SUB(74174_GATE) B;
		NETLIB_SUB(74174_GATE) C;
		NETLIB_SUB(74174_GATE) D;
		NETLIB_SUB(74174_GATE) E;
		NETLIB_SUB(74174_GATE) F;
	};

	NETLIB_DEVICE_IMPL(74174_GATE, "TTL_74174_GATE", "")
	NETLIB_DEVICE_IMPL(74174,      "TTL_74174", "+CLK,+D1,+D2,+D3,+D4,+D5,+D6,+CLRQ,@VCC,@GND")

	} //namespace devices
} // namespace netlist
