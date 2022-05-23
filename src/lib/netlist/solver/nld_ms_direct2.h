// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_DIRECT2_H_
#define NLD_MS_DIRECT2_H_

///
/// \file nld_ms_direct2.h
///

#include "nld_matrix_solver_ext.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist::solver
{

	// ----------------------------------------------------------------------------------------
	// matrix_solver - Direct2
	// ----------------------------------------------------------------------------------------

	template <typename FT>
	class matrix_solver_direct2_t: public matrix_solver_direct_t<FT, 2>
	{
	public:

		using float_type = FT;

		matrix_solver_direct2_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver::solver_parameters_t *params)
		: matrix_solver_direct_t<FT, 2>(main_solver, name, nets, params, 2)
		{}
		void vsolve_non_dynamic() override
		{
			this->clear_square_mat(this->m_A);
			this->fill_matrix_and_rhs();

			const float_type a = this->m_A[0][0];
			const float_type b = this->m_A[0][1];
			const float_type c = this->m_A[1][0];
			const float_type d = this->m_A[1][1];

			const float_type v1 = (a * this->m_RHS[1] - c * this->m_RHS[0]) / (a * d - b * c);
			const float_type v0 = (this->m_RHS[0] - b * v1) / a;
			this->m_new_V[0] = v0;
			this->m_new_V[1] = v1;
		}

	};

} // namespace netlist::solver

#endif // NLD_MS_DIRECT2_H_
