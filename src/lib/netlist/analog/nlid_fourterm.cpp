// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nlid_fourterm.h"

#include "nl_factory.h"

#include "solver/nld_solver.h"

namespace netlist::analog {

	// -------------------------------------------------------------------------
	// nld_VCCS
	// -------------------------------------------------------------------------

	nld_VCCS::nld_VCCS(constructor_param_t data, nl_fptype ri)
		: base_device_t(data)
		, m_G(*this, "G", nlconst::one())
		, m_RI(*this, "RI", ri)
		, m_OP(*this, "OP", &m_IP, {&m_ON, &m_IN},
			  NETLIB_DELEGATE(terminal_handler))
		, m_ON(*this, "ON", &m_IP, {&m_OP, &m_IN},
			  NETLIB_DELEGATE(terminal_handler))
		, m_IP(*this, "IP", &m_IN, {&m_OP, &m_ON},
			  NETLIB_DELEGATE(terminal_handler))
		, m_IN(*this, "IN", &m_IP, {&m_OP, &m_ON},
			  NETLIB_DELEGATE(terminal_handler))
		, m_OP1(*this, "_OP1", &m_IN, NETLIB_DELEGATE(terminal_handler))
		, m_ON1(*this, "_ON1", &m_IN, NETLIB_DELEGATE(terminal_handler))
		, m_factor(nlconst::one())
	{
		connect(m_OP, m_OP1);
		connect(m_ON, m_ON1);
	}

	NETLIB_RESET(VCCS)
	{
		const nl_fptype m_mult = m_G() * m_factor; // 1.0 ==> 1V ==> 1A
		const nl_fptype GI = plib::reciprocal(m_RI());

		m_IP.set_conductivity(GI);
		m_IN.set_conductivity(GI);

		m_OP.set_gt_go(nlconst::zero(), -m_mult);
		m_OP1.set_gt_go(nlconst::zero(), m_mult);

		m_ON.set_gt_go(nlconst::zero(), m_mult);
		m_ON1.set_gt_go(nlconst::zero(), -m_mult);
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

	nld_LVCCS::nld_LVCCS(constructor_param_t data)
		: nld_VCCS(data)
		, m_cur_limit(*this, "CURLIM", nlconst::magic(1000.0))
		, m_vi(nlconst::zero())
	{
	}

	NETLIB_RESET(LVCCS) { NETLIB_NAME(VCCS)::reset(); }

	NETLIB_UPDATE_PARAM(LVCCS) { NETLIB_NAME(VCCS)::update_param(); }

	NETLIB_UPDATE_TERMINALS(LVCCS)
	{
		const nl_fptype m_mult = m_G() * get_factor(); // 1.0 ==> 1V ==> 1A
		const nl_fptype vi = m_IP.net().Q_Analog() - m_IN.net().Q_Analog();
		const auto      c1(nlconst::magic(0.2));

		if (plib::abs(m_mult / m_cur_limit() * vi) > nlconst::half())
			m_vi = m_vi + c1 * plib::tanh((vi - m_vi) / c1);
		else
			m_vi = vi;

		const nl_fptype x = m_mult / m_cur_limit() * m_vi;
		const nl_fptype tanh_of_x = plib::tanh(x);

		const nl_fptype beta = m_mult
							   * (nlconst::one() - tanh_of_x * tanh_of_x);
		const nl_fptype I = m_cur_limit() * tanh_of_x - beta * m_vi;

		m_OP.set_gt_go_I(nlconst::zero(), -beta, I);
		m_OP1.set_gt_go(nlconst::zero(), beta);

		m_ON.set_gt_go_I(nlconst::zero(), beta, -I);
		m_ON1.set_gt_go(nlconst::zero(), -beta);
	}

	// -------------------------------------------------------------------------
	// nld_CCCS
	// -------------------------------------------------------------------------

	nld_CCCS::nld_CCCS(constructor_param_t data)
		: nld_VCCS(data, nlconst::one())
	{
		set_factor(-plib::reciprocal(m_RI()));
	}

	NETLIB_RESET(CCCS) { NETLIB_NAME(VCCS)::reset(); }

	NETLIB_UPDATE_PARAM(CCCS) { NETLIB_NAME(VCCS)::update_param(); }

	// -------------------------------------------------------------------------
	// nld_VCVS
	// -------------------------------------------------------------------------

	nld_VCVS::nld_VCVS(constructor_param_t data)
		: nld_VCCS(data)
		, m_RO(*this, "RO", nlconst::one())
		, m_OP2(*this, "_OP2", &m_ON2, NETLIB_DELEGATE(terminal_handler))
		, m_ON2(*this, "_ON2", &m_OP2, NETLIB_DELEGATE(terminal_handler))
	{
		connect(m_OP2, m_OP1);
		connect(m_ON2, m_ON1);
	}

	NETLIB_RESET(VCVS)
	{
		const auto factor(plib::reciprocal(m_RO()));
		set_factor(factor);

		NETLIB_NAME(VCCS)::reset();

		m_OP2.set_conductivity(factor);
		m_ON2.set_conductivity(factor);
	}

	// -------------------------------------------------------------------------
	// nld_CCVS
	// -------------------------------------------------------------------------

	nld_CCVS::nld_CCVS(constructor_param_t data)
		: nld_VCCS(data, nlconst::one())
		, m_RO(*this, "RO", nlconst::one())
		, m_OP2(*this, "_OP2", &m_ON2, NETLIB_DELEGATE(terminal_handler))
		, m_ON2(*this, "_ON2", &m_OP2, NETLIB_DELEGATE(terminal_handler))
	{
		connect(m_OP2, m_OP1);
		connect(m_ON2, m_ON1);
	}

	NETLIB_RESET(CCVS)
	{
		const auto factor(plib::reciprocal(m_RO()));
		set_factor(factor);

		NETLIB_NAME(VCCS)::reset();

		m_OP2.set_conductivity(factor);
		m_ON2.set_conductivity(factor);
	}

} // namespace netlist::analog

namespace netlist::devices {
	// clang-format off
	NETLIB_DEVICE_IMPL_NS(analog, VCVS,  "VCVS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, VCCS,  "VCCS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, CCCS,  "CCCS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, CCVS,  "CCVS",  "G")
	NETLIB_DEVICE_IMPL_NS(analog, LVCCS, "LVCCS", "")
	// clang-format on
} // namespace netlist::devices
