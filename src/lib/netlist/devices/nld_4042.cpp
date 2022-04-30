// license:BSD-3-Clause
// copyright-holders:Couriersud, Jonathan Gevaryahu
/*
 * nld_4042.cpp
 *
 *  CD4042BM/CD4042BC Quad Clocked D Latch
 *
 *       +--------------+
 *    Q4 |1     ++    16| VDD
 *    Q1 |2           15| Q4Q
 *   Q1Q |3           14| D4
 *    D1 |4    4042   13| D3
 *   CLK |5           12| Q3Q
 *   POL |6           11| Q3
 *    D2 |7           10| Q2
 *   VSS |8            9| Q2Q
 *       +--------------+
 *
 *
 *          Function table
 *          +-----+-----+----++----+-----+
 *          | POL | CLK | Dx || Qx | QQx |
 *          +=====+=====+====++====+=====+
 *          |  0  |  0  |  X || Qx | /Qx |
 *          |  0  |  1  |  D ||  D | /D  |
 *          |  1  |  1  |  X || Qx | /Qx |
 *          |  1  |  0  |  D ||  D | /D  |
 *          +-----+-----+----++----+-----+
 *          Note that since this is a level triggered transparent latch,
 *          as long as POL ^ CLK == true, the latch is transparent, and
 *          if D changes Q and QQ(/Q) will instantly change.
 *
 *  Naming conventions follow National Semiconductor datasheet
 *
 *  National Semiconductor Datasheet: http://pdf.datasheetcatalog.com/datasheet/nationalsemiconductor/DS005966.PDF
 *  TI Datasheet: https://www.ti.com/lit/ds/symlink/cd4042b.pdf
 */

#include "nl_base.h"

namespace netlist::devices {


	NETLIB_OBJECT(CD4042)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4042, "CD4XXX")
		, m_D(*this, {"D1", "D2", "D3", "D4"}, NETLIB_DELEGATE(inputs))
		, m_POL(*this, "POL", NETLIB_DELEGATE(clk))
		, m_CLK(*this, "CLK", NETLIB_DELEGATE(clk))
		, m_Q(*this, {"Q1", "Q2", "Q3", "Q4"})
		, m_QQ(*this, {"Q1Q", "Q2Q", "Q3Q", "Q4Q"})
		, m_latch(*this, "m_latch", 0)
		, m_tpdq(*this, "m_tpd", netlist_time::from_nsec(175))
		, m_tpdqq(*this, "m_tpd", netlist_time::from_nsec(150))
		, m_tpcq(*this, "m_tpc", netlist_time::from_nsec(250))
		, m_tpcqq(*this, "m_tpc", netlist_time::from_nsec(250))
		, m_power_pins(*this, NETLIB_DELEGATE(vdd_vss))
		{
		}

	private:
		NETLIB_HANDLERI(clk)
		{
			if (m_POL() ^ m_CLK()) // are we in transparent mode? if so latch the data and push it.
			{
				m_latch = m_D()&0xf;
				m_Q.push(m_latch&0xf, m_tpcq);
				m_QQ.push((~m_latch)&0xf, m_tpcqq);
			}
			// if not, the data inputs are ignored and just do nothing
		}

		NETLIB_HANDLERI(inputs)
		{
			if ((m_POL() ^ m_CLK())&&(m_latch != (m_D()&0xf))) // are we in transparent mode? if so latch the data and push it. only do this if the data actually changed
			{
				m_latch = m_D()&0xf;
				m_Q.push(m_latch&0xf, m_tpdq);
				m_QQ.push((~m_latch)&0xf, m_tpdqq);
			}
		}

		NETLIB_HANDLERI(vdd_vss)
		{
			auto d = m_power_pins.VCC()() - m_power_pins.GND()();
			if (d > 0.1) // avoid unrealistic values
			{
				m_tpdq = netlist_time::from_nsec(gsl::narrow_cast<unsigned>(894.0 / d - 6.0));
				m_tpdqq = netlist_time::from_nsec(gsl::narrow_cast<unsigned>(750.0 / d + 0.0));
				m_tpcq = netlist_time::from_nsec(gsl::narrow_cast<unsigned>(1327.0 / d - 18.8));
				m_tpcqq = netlist_time::from_nsec(gsl::narrow_cast<unsigned>(1234.5 / d + 0.8));
			}
		}

		NETLIB_RESETI()
		{
			m_latch = 0;
		}

		object_array_t<logic_input_t, 4> m_D;
		logic_input_t m_POL;
		logic_input_t m_CLK;

		object_array_t<logic_output_t, 4> m_Q;
		object_array_t<logic_output_t, 4> m_QQ;
		state_var_u8 m_latch;
		state_var<netlist_time> m_tpdq; // propagation time for data alone when in transparent mode, Q
		state_var<netlist_time> m_tpdqq; // propagation time for data alone when in transparent mode, /Q
		state_var<netlist_time> m_tpcq; // propagation time for data vs CLK, Q
		state_var<netlist_time> m_tpcqq; // propagation time for data vs CLK, /Q
		nld_power_pins m_power_pins;
	};

	NETLIB_DEVICE_IMPL(CD4042,     "CD4042",        "+D1,+D2,+D3,+D4,+POL,+CLK,@VCC,@GND")

} // namespace netlist::devices
