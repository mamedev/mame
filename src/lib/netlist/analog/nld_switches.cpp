// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nlid_twoterm.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "solver/nld_solver.h"

// FIXME : convert to parameters

#define R_OFF   (plib::reciprocal(exec().gmin()))
#define R_ON    nlconst::magic(0.01)

namespace netlist
{
	namespace analog
	{
	// ----------------------------------------------------------------------------------------
	// SWITCH
	// ----------------------------------------------------------------------------------------

	NETLIB_BASE_OBJECT(switch1)
	{
		NETLIB_CONSTRUCTOR(switch1)
		, m_R(*this, "R")
		, m_POS(*this, "POS", false)
		{
			register_sub_alias("1", m_R.P());
			register_sub_alias("2", m_R.N());
		}

		NETLIB_RESETI()
		{
			m_R.set_R(R_OFF);
		}
		NETLIB_UPDATE_PARAMI()
		{
			m_R.change_state([this]()
			{
				m_R.set_R(m_POS() ? R_ON : R_OFF);
			});
		}

	private:
		analog::NETLIB_SUB(R_base) m_R;
		param_logic_t              m_POS;
	};

// ----------------------------------------------------------------------------------------
// SWITCH2
// ----------------------------------------------------------------------------------------

	NETLIB_BASE_OBJECT(switch2)
	{
		NETLIB_CONSTRUCTOR(switch2)
		, m_R1(*this, "R1")
		, m_R2(*this, "R2")
		, m_POS(*this, "POS", false)
		{
			connect(m_R1.N(), m_R2.N());

			register_sub_alias("1", m_R1.P());
			register_sub_alias("2", m_R2.P());

			register_sub_alias("Q", m_R1.N());
		}

		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();

	private:
		analog::NETLIB_SUB(R_base) m_R1;
		analog::NETLIB_SUB(R_base) m_R2;
		param_logic_t             m_POS;
	};

	NETLIB_RESET(switch2)
	{
		m_R1.set_R(R_ON);
		m_R2.set_R(R_OFF);
	}

#ifdef FIXMELATER
	NETLIB_UPDATE(switch2)
	{
		if (!m_POS())
		{
			m_R1.set_R(R_ON);
			m_R2.set_R(R_OFF);
		}
		else
		{
			m_R1.set_R(R_OFF);
			m_R2.set_R(R_ON);
		}
	}
#endif
	NETLIB_UPDATE_PARAM(switch2)
	{
		// R1 and R2 are connected. However this net may be a rail net.
		// The code here thus is a bit more complex.

		nl_fptype r1 = m_POS() ? R_OFF : R_ON;
		nl_fptype r2 = m_POS() ? R_ON : R_OFF;

		if (m_R1.solver() == m_R2.solver())
			m_R1.change_state([this, &r1, &r2]() { m_R1.set_R(r1); m_R2.set_R(r2); });
		else
		{
			m_R1.change_state([this, &r1]() { m_R1.set_R(r1); });
			m_R2.change_state([this, &r2]() { m_R2.set_R(r2); });
		}
	}

	} //namespace analog

	namespace devices {
		NETLIB_DEVICE_IMPL_NS(analog, switch1, "SWITCH",  "")
		NETLIB_DEVICE_IMPL_NS(analog, switch2, "SWITCH2", "")
	} // namespace devices
} // namespace netlist
