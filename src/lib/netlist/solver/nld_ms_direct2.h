// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct1.h
 *
 */

#ifndef NLD_MS_DIRECT2_H_
#define NLD_MS_DIRECT2_H_

#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist
{
namespace solver
{

	// ----------------------------------------------------------------------------------------
	// matrix_solver - Direct2
	// ----------------------------------------------------------------------------------------

	template <typename FT>
	class matrix_solver_direct2_t: public matrix_solver_direct_t<FT, 2>
	{
	public:

		using float_type = FT;

		matrix_solver_direct2_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params)
		: matrix_solver_direct_t<FT, 2>(anetlist, name, nets, params, 2)
		{}
		unsigned vsolve_non_dynamic(const bool newton_raphson) override
		{
			this->clear_square_mat(this->m_A);
			this->fill_matrix(this->m_RHS);

			const float_type a = this->m_A[0][0];
			const float_type b = this->m_A[0][1];
			const float_type c = this->m_A[1][0];
			const float_type d = this->m_A[1][1];

			const float_type v1 = (a * this->m_RHS[1] - c * this->m_RHS[0]) / (a * d - b * c);
			const float_type v0 = (this->m_RHS[0] - b * v1) / a;
			std::array<float_type, 2> new_V = {v0, v1};

			this->m_stat_calculations++;
			const float_type err = (newton_raphson ? this->delta(new_V) : plib::constants<FT>::zero());
			this->store(new_V);
			return (err > static_cast<float_type>(this->m_params.m_accuracy)) ? 2 : 1;
		}

	};

} // namespace solver
} // namespace netlist

#endif /* NLD_MS_DIRECT2_H_ */
