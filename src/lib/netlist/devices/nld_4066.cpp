// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.c
 *
 */

#include "nld_4066.h"
#include "nld_cmos.h"

namespace netlist
{
	namespace devices
	{

	NETLIB_OBJECT(CD4066_GATE)
	{
		NETLIB_CONSTRUCTOR(CD4066_GATE)
		NETLIB_FAMILY("CD4XXX")
		, m_supply(*this, "PS")
		, m_R(*this, "R")
		, m_base_r(*this, "BASER", 270.0)
		{
			enregister("CTL", m_control);
		}

		NETLIB_RESETI() { }
		NETLIB_UPDATEI();

	public:
		NETLIB_SUB(vdd_vss) m_supply;
		NETLIB_SUB(R) m_R;

		analog_input_t m_control;
		param_double_t m_base_r;
	};

	NETLIB_UPDATE(CD4066_GATE)
	{
		nl_double sup = (m_supply.vdd() - m_supply.vss());
		nl_double low = NL_FCONST(0.45) * sup;
		nl_double high = NL_FCONST(0.55) * sup;
		nl_double in = INPANALOG(m_control) - m_supply.vss();
		nl_double rON = m_base_r * NL_FCONST(5.0) / sup;
		nl_double R = -1.0;

		if (in < low)
		{
			R = NL_FCONST(1.0) / netlist().gmin();
		}
		else if (in > high)
		{
			R = rON;
		}
		if (R > NL_FCONST(0.0))
		{
			// We only need to update the net first if this is a time stepping net
			if (1) // m_R.m_P.net().as_analog().solver().is_timestep())
			{
				m_R.update_dev();
				m_R.set_R(R);
				m_R.m_P.schedule_after(NLTIME_FROM_NS(1));
			}
			else
			{
				m_R.set_R(R);
				m_R.update_dev();
			}
		}
	}

	NETLIB_DEVICE_IMPL(CD4066_GATE)

	} //namespace devices
} // namespace netlist
