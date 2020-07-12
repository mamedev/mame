// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_4316.c
 *
 */

#include "nld_4316.h"
#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"

namespace netlist { namespace devices {

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
			m_R.set_R(plib::reciprocal(exec().gmin()));
		}

		NETLIB_HANDLERI(inputs)
		{
			m_R.change_state([this]()
				{
					if (m_S() && !m_E())
						m_R.set_R(m_base_r());
					else
						m_R.set_R(plib::reciprocal(exec().gmin()));
				}
				, NLTIME_FROM_NS(1));
		}

		NETLIB_UPDATEI()
		{
			inputs();
		}

	private:
		analog::NETLIB_SUB(R_base) m_R;

		logic_input_t              m_S;
		logic_input_t              m_E;
		param_fp_t             m_base_r;
		nld_power_pins             m_supply;
	};

	NETLIB_DEVICE_IMPL(CD4316_GATE, "CD4316_GATE", "")

} // namespace devices
 } // namespace netlist
