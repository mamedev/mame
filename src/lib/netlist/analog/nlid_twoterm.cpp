// license:GPL-2.0+
// copyright-holders:Couriersud

#include "netlist/solver/nld_solver.h"

#include "netlist/nl_factory.h"
#include "nlid_twoterm.h"

namespace netlist
{
namespace analog
{

	// ----------------------------------------------------------------------------------------
	// nld_twoterm
	// ----------------------------------------------------------------------------------------

	void NETLIB_NAME(twoterm)::solve_now()
	{
		// we only need to call the non-rail terminal
		if (m_P.has_net() && !m_P.net().isRailNet())
			m_P.solve_now();
		else if (m_N.has_net() && !m_N.net().isRailNet())
			m_N.solve_now();
	}

	void NETLIB_NAME(twoterm)::solve_later(netlist_time delay)
	{
		// we only need to call the non-rail terminal
		if (m_P.has_net() && !m_P.net().isRailNet())
			m_P.schedule_solve_after(delay);
		else if (m_N.has_net() && !m_N.net().isRailNet())
			m_N.schedule_solve_after(delay);
	}


	NETLIB_UPDATE(twoterm)
	{
		// only called if connected to a rail net ==> notify the solver to recalculate
		solve_now();
	}

	// ----------------------------------------------------------------------------------------
	// nld_R_base
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(R_base)
	{
		NETLIB_NAME(twoterm)::reset();
		set_R(plib::reciprocal(exec().gmin()));
	}

	// ----------------------------------------------------------------------------------------
	// nld_POT
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(POT)
	{
		nl_fptype v = m_Dial();
		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one()) / (plib::exp(nlconst::one()) - nlconst::one());

		m_R1.set_R(std::max(m_R() * v, exec().gmin()));
		m_R2.set_R(std::max(m_R() * (nlconst::one() - v), exec().gmin()));
	}

	NETLIB_UPDATE_PARAM(POT)
	{
		m_R1.solve_now();
		m_R2.solve_now();

		nl_fptype v = m_Dial();
		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one()) / (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;
		m_R1.set_R(std::max(m_R() * v, exec().gmin()));
		m_R2.set_R(std::max(m_R() * (nlconst::one() - v), exec().gmin()));

	}

	// ----------------------------------------------------------------------------------------
	// nld_POT2
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(POT2)
	{
		nl_fptype v = m_Dial();

		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one()) / (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;
		m_R1.set_R(std::max(m_R() * v, exec().gmin()));
	}


	NETLIB_UPDATE_PARAM(POT2)
	{
		m_R1.solve_now();

		nl_fptype v = m_Dial();

		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one()) / (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;
		m_R1.set_R(std::max(m_R() * v, exec().gmin()));
	}

	// ----------------------------------------------------------------------------------------
	// nld_L
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(L)
	{
		m_gmin = exec().gmin();
		m_I = nlconst::zero();
		m_G = m_gmin;
		set_mat( m_G, -m_G, -m_I,
				-m_G,  m_G,  m_I);
		//set(1.0/NETLIST_GMIN, 0.0, -5.0 * NETLIST_GMIN);
	}

	NETLIB_UPDATE_PARAM(L)
	{
	}

	NETLIB_TIMESTEP(L)
	{
		// Gpar should support convergence
		m_I += m_I + m_G * deltaV();
		m_G = step / m_L() + m_gmin;
		set_mat( m_G, -m_G, -m_I,
				-m_G,  m_G,  m_I);
	}

	// ----------------------------------------------------------------------------------------
	// nld_D
	// ----------------------------------------------------------------------------------------

	NETLIB_RESET(D)
	{
		nl_fptype Is = m_model.m_IS;
		nl_fptype n = m_model.m_N;

		m_D.set_param(Is, n, exec().gmin(), nlconst::T0());
		set_G_V_I(m_D.G(), nlconst::zero(), m_D.Ieq());
	}

	NETLIB_UPDATE_PARAM(D)
	{
		nl_fptype Is = m_model.m_IS;
		nl_fptype n = m_model.m_N;

		m_D.set_param(Is, n, exec().gmin(), nlconst::T0());
	}

	NETLIB_UPDATE_TERMINALS(D)
	{
		m_D.update_diode(deltaV());
		const nl_fptype G = m_D.G();
		const nl_fptype I = m_D.Ieq();
		set_mat( G, -G, -I,
				-G,  G,  I);
		//set(m_D.G(), 0.0, m_D.Ieq());
	}


} //namespace analog

namespace devices {
	NETLIB_DEVICE_IMPL_NS(analog, R,    "RES",   "R")
	NETLIB_DEVICE_IMPL_NS(analog, POT,  "POT",   "R")
	NETLIB_DEVICE_IMPL_NS(analog, POT2, "POT2",  "R")
	NETLIB_DEVICE_IMPL_NS(analog, C,    "CAP",   "C")
	NETLIB_DEVICE_IMPL_NS(analog, L,    "IND",   "L")
	NETLIB_DEVICE_IMPL_NS(analog, D,    "DIODE", "MODEL")
	NETLIB_DEVICE_IMPL_NS(analog, VS,   "VS",    "V")
	NETLIB_DEVICE_IMPL_NS(analog, CS,   "CS",    "I")
} // namespace devices

} // namespace netlist
