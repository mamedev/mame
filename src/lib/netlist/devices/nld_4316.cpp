// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 * nld_4316.c
 *
 */

#include <devices/nlid_cmos.h>
#include "analog/nld_twoterm.h"
#include "nld_4316.h"

namespace netlist { namespace devices {

	NETLIB_OBJECT(CD4316_GATE)
	{
		NETLIB_CONSTRUCTOR(CD4316_GATE)
		NETLIB_FAMILY("CD4XXX")
		, m_supply(*this, "PS")
		, m_R(*this, "R")
		, m_S(*this, "S")
		, m_E(*this, "E")
		, m_base_r(*this, "BASER", 45.0)
		{
		}

		NETLIB_RESETI() { }
		NETLIB_UPDATEI();

	public:
		NETLIB_SUB(vdd_vss) m_supply;
		NETLIB_SUB(R) m_R;

		logic_input_t m_S, m_E;
		param_double_t m_base_r;
	};

	NETLIB_UPDATE(CD4316_GATE)
	{
		m_R.update_dev();
		if (m_S() && !m_E())
			m_R.set_R(m_base_r());
		else
			m_R.set_R(NL_FCONST(1.0) / netlist().gmin());
		m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
	}

	NETLIB_DEVICE_IMPL(CD4316_GATE);

} } // namesapce netlist::devices
