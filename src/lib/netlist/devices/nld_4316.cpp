// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_4316.c
 *
 */

#include "nld_4316.h"
#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"
#include "nlid_cmos.h"

namespace netlist { namespace devices {

	NETLIB_OBJECT(CD4316_GATE)
	{
		NETLIB_CONSTRUCTOR(CD4316_GATE)
		NETLIB_FAMILY("CD4XXX")
		, m_supply(*this, "PS")
		, m_R(*this, "_R")
		, m_S(*this, "S")
		, m_E(*this, "E")
		, m_base_r(*this, "BASER", 45.0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		NETLIB_SUB(vdd_vss)        m_supply;
		analog::NETLIB_SUB(R_base) m_R;

		logic_input_t              m_S;
		logic_input_t              m_E;
		param_double_t             m_base_r;
	};

	NETLIB_RESET(CD4316_GATE)
	{
		m_R.set_R(plib::constants<nl_double>::one() / exec().gmin());
	}

	NETLIB_UPDATE(CD4316_GATE)
	{
		m_R.update();
		if (m_S() && !m_E())
			m_R.set_R(m_base_r());
		else
			m_R.set_R(plib::constants<nl_double>::one() / exec().gmin());
		m_R.solve_later(NLTIME_FROM_NS(1));
	}

	NETLIB_DEVICE_IMPL(CD4316_GATE, "CD4316_GATE", "")

} // namespace devices
 } // namespace netlist
