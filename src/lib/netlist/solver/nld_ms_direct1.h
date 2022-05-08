// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_DIRECT1_H_
#define NLD_MS_DIRECT1_H_

///
/// \file nld_ms_direct1.h
///

#include "nld_matrix_solver_ext.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist::solver
{
	template <typename FT>
	class matrix_solver_direct1_t: public matrix_solver_direct_t<FT, 1>
	{
	public:

		using float_type = FT;
		using base_type = matrix_solver_direct_t<FT, 1>;

		matrix_solver_direct1_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver::solver_parameters_t *params)
			: matrix_solver_direct_t<FT, 1>(main_solver, name, nets, params, 1)
			{}

		// ----------------------------------------------------------------------------------------
		// matrix_solver - Direct1
		// ----------------------------------------------------------------------------------------
		void vsolve_non_dynamic() override
		{
			this->clear_square_mat(this->m_A);
			this->fill_matrix_and_rhs();

			this->m_new_V[0] = this->m_RHS[0] / this->m_A[0][0];
		}
	};


} // namespace netlist::solver


#endif // NLD_MS_DIRECT1_H_
