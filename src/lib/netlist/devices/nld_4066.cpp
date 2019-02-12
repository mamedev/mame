// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.c
 *
 */

#include "nld_4066.h"

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/solver/nld_solver.h"
#include "nlid_cmos.h"

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
		, m_control(*this, "CTL")
		, m_base_r(*this, "BASER", 270.0)
		{
		}

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	public:
		NETLIB_SUB(vdd_vss)        m_supply;
		analog::NETLIB_SUB(R_base) m_R;

		analog_input_t             m_control;
		param_double_t             m_base_r;
	};

	NETLIB_RESET(CD4066_GATE)
	{
		// Start in off condition
		// FIXME: is ROFF correct?
		m_R.set_R(plib::constants<nl_double>::one() / exec().gmin());

	}

	NETLIB_UPDATE(CD4066_GATE)
	{
		nl_double sup = (m_supply.vdd() - m_supply.vss());
		nl_double low = plib::constants<nl_double>::cast(0.45) * sup;
		nl_double high = plib::constants<nl_double>::cast(0.55) * sup;
		nl_double in = m_control() - m_supply.vss();
		nl_double rON = m_base_r() * plib::constants<nl_double>::cast(5.0) / sup;
		nl_double R = -1.0;

		if (in < low)
		{
			R = plib::constants<nl_double>::one() / exec().gmin();
		}
		else if (in > high)
		{
			R = rON;
		}
		if (R > plib::constants<nl_double>::zero())
		{
			m_R.update();
			m_R.set_R(R);
			m_R.solve_later();
		}
	}

	NETLIB_DEVICE_IMPL(CD4066_GATE,         "CD4066_GATE",            "")

	} //namespace devices
} // namespace netlist
