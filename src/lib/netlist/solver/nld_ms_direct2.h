// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_MS_DIRECT2_H_
#define NLD_MS_DIRECT2_H_

///
/// \file nld_ms_direct2.h
///

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
			this->fill_matrix_and_rhs();

			const float_type a = this->m_A[0][0];
			const float_type b = this->m_A[0][1];
			const float_type c = this->m_A[1][0];
			const float_type d = this->m_A[1][1];

			const float_type v1 = (a * this->m_RHS[1] - c * this->m_RHS[0]) / (a * d - b * c);
			const float_type v0 = (this->m_RHS[0] - b * v1) / a;
			this->m_new_V[0] = v0;
			this->m_new_V[1] = v1;

			this->m_stat_calculations++;
			bool err(false);
			if (newton_raphson)
				err = this->check_err();
			this->store();
			return (err) ? 2 : 1;
		}

	};

} // namespace solver
} // namespace netlist

#endif // NLD_MS_DIRECT2_H_
