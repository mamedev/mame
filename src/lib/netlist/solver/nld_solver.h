// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_solver.h
 *
 */

#ifndef NLD_SOLVER_H_
#define NLD_SOLVER_H_

#include "netlist/nl_base.h"
#include "nld_matrix_solver.h"
#include "plib/pstream.h"

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

		nl_double gmin() const { return m_params.m_gmin(); }

		void create_solver_code(std::map<pstring, pstring> &mp);

		NETLIB_UPDATEI();
		NETLIB_RESETI();
		// NETLIB_UPDATE_PARAMI();

	private:
		logic_input_t m_fb_step;
		logic_output_t m_Q_step;

		std::vector<plib::unique_ptr<matrix_solver_t>> m_mat_solvers;
		std::vector<matrix_solver_t *> m_mat_solvers_all;
		std::vector<matrix_solver_t *> m_mat_solvers_timestepping;

		solver_parameters_t m_params;

		template <typename FT, int SIZE>
		plib::unique_ptr<matrix_solver_t> create_solver(std::size_t size, const pstring &solvername);
	};

} // namespace devices
} // namespace netlist

#endif /* NLD_SOLVER_H_ */
