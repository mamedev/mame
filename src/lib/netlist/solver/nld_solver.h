// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

///
/// \file nld_solver.h
///

#include "../plib/pstream.h"
#include "netlist/nl_base.h"
#include "nld_matrix_solver.h"

#include <map>
#include <memory>
#include <vector>

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

namespace netlist
{
namespace devices
{
	NETLIB_OBJECT(solver)
	{
		NETLIB_CONSTRUCTOR(solver)
		, m_fb_step(*this, "FB_step")
		, m_Q_step(*this, "Q_step")
		, m_params(*this)
		{
			// internal staff

			connect(m_fb_step, m_Q_step);
		}

		void post_start();
		void stop();

		nl_fptype gmin() const { return m_params.m_gmin(); }

		solver::static_compile_container create_solver_code(solver::static_compile_target target);

		NETLIB_UPDATEI();
		NETLIB_RESETI();
		// NETLIB_UPDATE_PARAMI();

	private:
		logic_input_t m_fb_step;
		logic_output_t m_Q_step;

		// FIXME: these should be created in device space
		std::vector<host_arena::unique_ptr<solver::matrix_solver_t>> m_mat_solvers;
		std::vector<solver::matrix_solver_t *> m_mat_solvers_all;
		std::vector<solver::matrix_solver_t *> m_mat_solvers_timestepping;

		solver::solver_parameters_t m_params;

		template <typename FT, int SIZE>
		host_arena::unique_ptr<solver::matrix_solver_t> create_solver(std::size_t size,
			const pstring &solvername, analog_net_t::list_t &nets);

		template <typename FT>
		host_arena::unique_ptr<solver::matrix_solver_t> create_solvers(
			const pstring &sname, analog_net_t::list_t &nets);
	};

} // namespace devices
} // namespace netlist

#endif // NLD_SOLVER_H_
