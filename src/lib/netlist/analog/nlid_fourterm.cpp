// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nlid_fourterm.h"

#include "nl_factory.h"

#include "solver/nld_solver.h"

namespace netlist::analog
{

	// -------------------------------------------------------------------------
	// nld_VCCS
	// -------------------------------------------------------------------------

	NETLIB_RESET(VCCS)
	{
		const nl_fptype m_mult = m_G() * m_gfac; // 1.0 ==> 1V ==> 1A
		const nl_fptype GI = plib::reciprocal(m_RI());

		m_IP.set_conductivity(GI);
		m_IN.set_conductivity(GI);

		m_OP.set_go_gt(-m_mult, nlconst::zero());
		m_OP1.set_go_gt(m_mult, nlconst::zero());

		m_ON.set_go_gt(m_mult, nlconst::zero());
		m_ON1.set_go_gt(-m_mult, nlconst::zero());
	}

	NETLIB_HANDLER(VCCS, terminal_handler)
	{
		solver::matrix_solver_t *solv = nullptr;
		// only called if connected to a rail net ==> notify the solver to
		// recalculate
		// NOLINTNEXTLINE(bugprone-branch-clone)
		if ((solv = m_IP.solver()) != nullptr)
			solv->solve_now();
		else if ((solv = m_IN.solver()) != nullptr)
			solv->solve_now();
		else if ((solv = m_OP.solver()) != nullptr)
			solv->solve_now();
		else if ((solv = m_ON.solver()) != nullptr)
			solv->solve_now();
	}

	// -------------------------------------------------------------------------
	// nld_LVCCS
	// -------------------------------------------------------------------------

	NETLIB_RESET(LVCCS) { NETLIB_NAME(VCCS)::reset(); }

	NETLIB_UPDATE_PARAM(LVCCS) { NETLIB_NAME(VCCS)::update_param(); }

	NETLIB_UPDATE_TERMINALS(LVCCS)
	{
		const nl_fptype m_mult = m_G() * get_gfac(); // 1.0 ==> 1V ==> 1A
		const nl_fptype vi = m_IP.net().Q_Analog() - m_IN.net().Q_Analog();
		const auto      c1(nlconst::magic(0.2));

		if (plib::abs(m_mult / m_cur_limit() * vi) > nlconst::half())
			m_vi = m_vi + c1 * plib::tanh((vi - m_vi) / c1);
		else
			m_vi = vi;

		const nl_fptype x = m_mult / m_cur_limit() * m_vi;
		const nl_fptype tanhx = plib::tanh(x);

		const nl_fptype beta = m_mult * (nlconst::one() - tanhx * tanhx);
		const nl_fptype I = m_cur_limit() * tanhx - beta * m_vi;

		m_OP.set_go_gt_I(-beta, nlconst::zero(), I);
		m_OP1.set_go_gt(beta, nlconst::zero());

		m_ON.set_go_gt_I(beta, nlconst::zero(), -I);
		m_ON1.set_go_gt(-beta, nlconst::zero());
	}

	// -------------------------------------------------------------------------
	// nld_CCCS
	// -------------------------------------------------------------------------

	NETLIB_RESET(CCCS) { NETLIB_NAME(VCCS)::reset(); }

	NETLIB_UPDATE_PARAM(CCCS) { NETLIB_NAME(VCCS)::update_param(); }

	// -------------------------------------------------------------------------
	// nld_VCVS
	// -------------------------------------------------------------------------

	NETLIB_RESET(VCVS)
	{
		const auto gfac(plib::reciprocal(m_RO()));
		set_gfac(gfac);

		NETLIB_NAME(VCCS)::reset();

		m_OP2.set_conductivity(gfac);
		m_ON2.set_conductivity(gfac);
	}

	// -------------------------------------------------------------------------
	// nld_CCVS
	// -------------------------------------------------------------------------

	NETLIB_RESET(CCVS)
	{
		const auto gfac(plib::reciprocal(m_RO()));
		set_gfac(gfac);

		NETLIB_NAME(VCCS)::reset();

		m_OP2.set_conductivity(gfac);
		m_ON2.set_conductivity(gfac);
	}

} // namespace netlist::analog

namespace netlist::devices
{
	// clang-format off
	NETLIB_DEVICE_IMPL_NS(analog, VCVS,  "VCVS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, VCCS,  "VCCS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, CCCS,  "CCCS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, CCVS,  "CCVS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, LVCCS, "LVCCS", "")
	// clang-format on
} // namespace netlist::devices
