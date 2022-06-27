// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_4316.cpp
 *
 *  CD74HC4316: Quad Analog Switch with Level Translation
 *
 *          +--------------+
 *       1Z |1     ++    16| VCC
 *       1Y |2           15| 1S
 *       2Y |3           14| 4S
 *       2Z |4    4316   13| 4Z
 *       2S |5           12| 4Y
 *       3S |6           11| 3Y
 *       /E |7           10| 3Z
 *      GND |8            9| VEE
 *          +--------------+
 *
 *  FIXME: These devices are slow (can be over 200 ns in HC types). This is currently not reflected
 *
 *  Naming conventions follow Texas Instruments datasheet
 *
 */

#include "analog/nlid_twoterm.h"
#include "solver/nld_solver.h"

namespace netlist::devices {

	// FIXME: tristate outputs?

	NETLIB_OBJECT(CD4316_GATE)
	{
		NETLIB_CONSTRUCTOR_MODEL(CD4316_GATE, "CD4XXX")
		, m_R(*this, "_R")
		, m_S(*this, "S", NETLIB_DELEGATE(inputs))
		, m_E(*this, "E", NETLIB_DELEGATE(inputs))
		, m_base_r(*this, "BASER", nlconst::magic(45.0))
		, m_supply(*this)
		{
		}

		NETLIB_RESETI()
		{
			m_R().set_R(plib::reciprocal(exec().gmin()));
		}

		NETLIB_HANDLERI(inputs)
		{
			m_R().change_state([this]()
				{
					if (m_S() && !m_E())
						m_R().set_R(m_base_r());
					else
						m_R().set_R(plib::reciprocal(exec().gmin()));
				});
		}

	private:
		NETLIB_SUB_NS(analog, R_base) m_R;

		logic_input_t                 m_S;
		logic_input_t                 m_E;
		param_fp_t                    m_base_r;
		nld_power_pins                m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4316_GATE, "CD4316_GATE", "")

} // namespace netlist::devices
