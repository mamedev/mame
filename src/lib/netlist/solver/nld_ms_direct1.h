// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct1.h
 *
 */

#ifndef NLD_MS_DIRECT1_H_
#define NLD_MS_DIRECT1_H_

#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist
{
namespace solver
{
	template <typename FT>
	class matrix_solver_direct1_t: public matrix_solver_direct_t<FT, 1>
	{
	public:

		using float_type = FT;
		using base_type = matrix_solver_direct_t<FT, 1>;

		matrix_solver_direct1_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params)
			: matrix_solver_direct_t<FT, 1>(anetlist, name, nets, params, 1)
			{}

		// ----------------------------------------------------------------------------------------
		// matrix_solver - Direct1
		// ----------------------------------------------------------------------------------------
		unsigned vsolve_non_dynamic(const bool newton_raphson) override
		{
			this->clear_square_mat(this->m_A);
			this->fill_matrix(this->m_RHS);

			std::array<FT, 1> new_V = { this->m_RHS[0] / this->m_A[0][0] };

			const FT err = (newton_raphson ? this->delta(new_V) : 0.0);
			this->store(new_V);
			return (err > this->m_params.m_accuracy) ? 2 : 1;
		}

	};



} // namespace solver
} // namespace netlist


#endif /* NLD_MS_DIRECT1_H_ */
