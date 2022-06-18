// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nlid_twoterm.h"

#include "nl_factory.h"

#include "solver/nld_solver.h"

namespace netlist::analog
{

	// -------------------------------------------------------------------------
	// nld_twoterm
	// -------------------------------------------------------------------------

	solver::matrix_solver_t *nld_two_terminal::solver() const noexcept
	{
		auto *solv(m_P.solver());
		if (solv != nullptr)
			return solv;
		return m_N.solver();
	}

	void nld_two_terminal::solve_now() const
	{
		auto *solv(solver());
		if (solv != nullptr)
			solv->solve_now();
	}

	NETLIB_HANDLER(two_terminal, terminal_handler)
	{
		// only called if connected to a rail net ==> notify the solver to
		// recalculate
		// printf("%s update\n", this->name().c_str());
		solve_now();
	}

	// -------------------------------------------------------------------------
	// nld_POT
	// -------------------------------------------------------------------------

	NETLIB_RESET(POT)
	{
		nl_fptype v = m_Dial();
		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one())
				/ (plib::exp(nlconst::one()) - nlconst::one());

		m_R1().set_R(std::max(m_R() * v, exec().gmin()));
		m_R2().set_R(std::max(m_R() * (nlconst::one() - v), exec().gmin()));
	}

	NETLIB_UPDATE_PARAM(POT)
	{
		nl_fptype v = m_Dial();
		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one())
				/ (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;

		nl_fptype r1(std::max(m_R() * v, exec().gmin()));
		nl_fptype r2(std::max(m_R() * (nlconst::one() - v), exec().gmin()));

		if (m_R1().solver() == m_R2().solver())
			m_R1().change_state(
				[this, &r1, &r2]()
				{
					m_R1().set_R(r1);
					m_R2().set_R(r2);
				});
		else
		{
			m_R1().change_state([this, &r1]() { m_R1().set_R(r1); });
			m_R2().change_state([this, &r2]() { m_R2().set_R(r2); });
		}
	}

	// -------------------------------------------------------------------------
	// nld_POT2
	// -------------------------------------------------------------------------

	NETLIB_RESET(POT2)
	{
		nl_fptype v = m_Dial();

		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one())
				/ (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;
		m_R1().set_R(std::max(m_R() * v, exec().gmin()));
	}

	NETLIB_UPDATE_PARAM(POT2)
	{
		nl_fptype v = m_Dial();

		if (m_DialIsLog())
			v = (plib::exp(v) - nlconst::one())
				/ (plib::exp(nlconst::one()) - nlconst::one());
		if (m_Reverse())
			v = nlconst::one() - v;

		m_R1().change_state(
			[this, &v]() { m_R1().set_R(std::max(m_R() * v, exec().gmin())); });
	}

	// -------------------------------------------------------------------------
	// nld_L
	// -------------------------------------------------------------------------

	NETLIB_RESET(L)
	{
		m_gmin = exec().gmin();
		m_I = nlconst::zero();
		m_G = m_gmin;
		set_mat(m_G, -m_G, -m_I, //
				-m_G, m_G, m_I);
	}

	NETLIB_UPDATE_PARAM(L) {}

	NETLIB_TIMESTEP(L)
	{
		if (ts_type == time_step_type::FORWARD)
		{
			m_last_I = m_I;
			m_last_G = m_G;
			// Gpar should support convergence
			m_I += m_G * deltaV();
			m_G = step / m_L() + m_gmin;
			set_mat(m_G, -m_G, -m_I, //
					-m_G, m_G, m_I);
		}
		else
		{
			m_I = m_last_I;
			m_G = m_last_G;
		}
	}

	// -------------------------------------------------------------------------
	// nld_D
	// -------------------------------------------------------------------------

	NETLIB_RESET(D)
	{
		nl_fptype Is = m_modacc.m_IS;
		nl_fptype n = m_modacc.m_N;

		m_D.set_param(Is, n, exec().gmin(), nlconst::T0());
		set_G_V_I(m_D.G(), nlconst::zero(), m_D.Ieq());
	}

	NETLIB_UPDATE_PARAM(D)
	{
		nl_fptype Is = m_modacc.m_IS;
		nl_fptype n = m_modacc.m_N;

		m_D.set_param(Is, n, exec().gmin(), nlconst::T0());
	}

	NETLIB_UPDATE_TERMINALS(D)
	{
		m_D.update_diode(deltaV());
		const nl_fptype G(m_D.G());
		const nl_fptype I(m_D.Ieq());
		set_mat(G, -G, -I, //
				-G, G, I);
		// set(m_D.G(), 0.0, m_D.Ieq());
	}

	// -------------------------------------------------------------------------
	// nld_Z
	// -------------------------------------------------------------------------

	NETLIB_RESET(Z)
	{
		nl_fptype IsBV = m_modacc.m_IBV
						 / (plib::exp(m_modacc.m_BV
									  / nlconst::np_VT(m_modacc.m_NBV))
							- nlconst::one());

		m_D.set_param(m_modacc.m_IS, m_modacc.m_N, exec().gmin(),
					  nlconst::T0());
		m_R.set_param(IsBV, m_modacc.m_NBV, exec().gmin(), nlconst::T0());
		set_G_V_I(m_D.G(), nlconst::zero(), m_D.Ieq());
	}

	NETLIB_UPDATE_PARAM(Z)
	{
		nl_fptype IsBV = m_modacc.m_IBV
						 / (plib::exp(m_modacc.m_BV
									  / nlconst::np_VT(m_modacc.m_NBV))
							- nlconst::one());

		m_D.set_param(m_modacc.m_IS, m_modacc.m_N, exec().gmin(),
					  nlconst::T0());
		m_R.set_param(IsBV, m_modacc.m_NBV, exec().gmin(), nlconst::T0());
		set_G_V_I(m_D.G(), nlconst::zero(), m_D.Ieq());
	}

	NETLIB_UPDATE_TERMINALS(Z)
	{
		m_D.update_diode(deltaV());
		m_R.update_diode(-deltaV());
		const nl_fptype G(m_D.G() + m_R.G());
		const nl_fptype I(m_D.Ieq() - m_R.Ieq());
		set_mat(G, -G, -I, //
				-G, G, I);
	}

} // namespace netlist::analog

namespace netlist::devices
{
	// clang-format off
	NETLIB_DEVICE_IMPL_NS(analog, R,    "RES",   "R")
	NETLIB_DEVICE_IMPL_NS(analog, POT,  "POT",   "R")
	NETLIB_DEVICE_IMPL_NS(analog, POT2, "POT2",  "R")
	NETLIB_DEVICE_IMPL_NS(analog, C,    "CAP",   "C")
	NETLIB_DEVICE_IMPL_NS(analog, L,    "IND",   "L")
	NETLIB_DEVICE_IMPL_NS(analog, D,    "DIODE", "MODEL")
	NETLIB_DEVICE_IMPL_NS(analog, Z,    "ZDIODE", "MODEL")
	NETLIB_DEVICE_IMPL_NS(analog, VS,   "VS",    "V")
	NETLIB_DEVICE_IMPL_NS(analog, CS,   "CS",    "I")
	// clang-format on
} // namespace netlist::devices
