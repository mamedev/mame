// license:GPL-2.0+
// copyright-holders:Couriersud, Jonathan Gevaryahu
/*
 * nld_4076.cpp
 *
 *  CD4076BM/CD4076BC TRI-STATE(R) Quad D Flip-Flop
 *
 *       +--------------+
 *   OD1 |1     ++    16| VDD
 *   OD2 |2           15| CLR
 *    OA |3           14| IA
 *    OB |4    4076   13| IB
 *    OC |5           12| IC
 *    OD |6           11| ID
 *   CLK |7           10| ID2
 *   VSS |8            9| ID1
 *       +--------------+
 *
 *
 *          Function table for ID1/2 pins
 *          +-----+-----+-----+----+-----++-----+
 *          | ID1 | ID2 | CLR | Ix | CLK || iOx |
 *          +=====+=====+=====+====+=====++=====+
 *          |  X  |  X  |  0  |  X |  0  || iOx |
 *          |  X  |  X  |  0  |  X |  1  || iOx |
 *          |  1  |  X  |  0  |  X | 0>1 || iOx |
 *          |  X  |  1  |  0  |  X | 0>1 || iOx |
 *          |  0  |  0  |  0  |  0 | 0>1 ||  0  |
 *          |  0  |  0  |  0  |  1 | 0>1 ||  1  |
 *          |  X  |  X  |  1  |  X |  X  ||  0  |
 *          +-----+-----+-----+----+-----++-----+
 *          Note: iOX is an internal signal, the output of each D-latch
 *
 *          Function table for OD1/2 pins vs output of the internal D-latches
 *          +-----+-----+-----++-----+
 *          | OD1 | OD2 | iOx ||  Ox |
 *          +=====+=====+=====++=====+
 *          |  1  |  X  |  X  ||  Z  |
 *          |  X  |  1  |  X  ||  Z  |
 *          |  0  |  0  |  0  ||  0  |
 *          |  0  |  0  |  1  ||  1  |
 *          +-----+-----+-----++-----+
 *
 *  Naming conventions follow National Semiconductor datasheet
 *  http://www.bitsavers.org/components/national/_dataBooks/1981_Natonal_CMOS_Databook.pdf 5-186 (pdf page 567)
 *
 *  TODO: the 74C173 is 100% identical to this CMOS part.
 */

#include "nl_base.h"

namespace netlist::devices {

	NETLIB_OBJECT(CD4076)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4076, "CD4XXX")
		, m_I(*this, {"IA", "IB", "IC", "ID"}, NETLIB_DELEGATE(id)) // if the d-pins change absolutely nothing happens, as they are clock-latched.
		, m_ID1(*this, "ID1", NETLIB_DELEGATE(id))
		, m_ID2(*this, "ID2", NETLIB_DELEGATE(id))
		, m_enable_in(*this, "m_enable_in", true)
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_clk_old(*this, "m_clk_old", 0)
		, m_OD1(*this, "OD1", NETLIB_DELEGATE(od)) // if the OD pins change nothing about the internal state changes directly, but the outputs can change from driven to tri-state or vice-versa
		, m_OD2(*this, "OD2", NETLIB_DELEGATE(od)) // ""
		, m_enable_out(*this, "m_enable_out", true)
		, m_O(*this, {"OA", "OB", "OC", "OD"})
		, m_latch(*this, "m_latch", 0)
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_HANDLERI(id)
		{
			m_enable_in = (!(m_ID1() || m_ID2()));
		}

		NETLIB_HANDLERI(clk)
		{
			if ((!m_clk_old) && m_CLK() && m_enable_in) // clock rising edge and input is enabled; otherwise the latch just re-latches its own value
			{
				m_latch = m_I()&0xf;
			}
			m_clk_old = m_CLK();
			// update the pin output state
			for (std::size_t i=0; i<4; i++)
			{
				m_O.set_tristate(m_enable_out, NLTIME_FROM_NS(170), NLTIME_FROM_NS(170));
				m_O[i].push((m_latch >> i) & 1, NLTIME_FROM_NS(220));
			}
		}

		NETLIB_HANDLERI(od)
		{
			m_enable_out = (!(m_OD1() || m_OD2()));
			// update the pin output state
			for (std::size_t i=0; i<4; i++)
			{
				m_O.set_tristate(m_enable_out, NLTIME_FROM_NS(170), NLTIME_FROM_NS(170));
				m_O[i].push((m_latch >> i) & 1, NLTIME_FROM_NS(220));
			}
		}

		NETLIB_RESETI()
		{
			m_latch = 0;
			m_enable_in = true;
			m_enable_out = true;
		}

		object_array_t<logic_input_t, 4> m_I;
		logic_input_t m_ID1;
		logic_input_t m_ID2;
		state_var<bool> m_enable_in;
		logic_input_t m_CLK;
		state_var<netlist_sig_t> m_clk_old;
		logic_input_t m_OD1;
		logic_input_t m_OD2;
		state_var<bool> m_enable_out;
		object_array_t<logic_output_t, 4> m_O;
		state_var_u8 m_latch;
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(CD4076,     "CD4076",        "+I1,+I2,+I3,+I4,+ID1,+ID2,+OD1,+OD2,@VCC,@GND")

} // namespace netlist::devices
