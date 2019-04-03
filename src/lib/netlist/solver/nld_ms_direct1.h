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
namespace devices
{
	template <typename FT>
	class matrix_solver_direct1_t: public matrix_solver_direct_t<FT, 1>
	{
	public:

		using float_type = FT;
		using base_type = matrix_solver_direct_t<FT, 1>;

		matrix_solver_direct1_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params)
			: matrix_solver_direct_t<FT, 1>(anetlist, name, params, 1)
			{}

		// ----------------------------------------------------------------------------------------
		// matrix_solver - Direct1
		// ----------------------------------------------------------------------------------------
		unsigned vsolve_non_dynamic(const bool newton_raphson) override
		{
			this->build_LE_A(*this);
			this->build_LE_RHS(*this);
			//NL_VERBOSE_OUT(("{1} {2}\n", new_val, m_RHS[0] / m_A[0][0]);

			std::array<FT, 1> new_V = { this->RHS(0) / this->A(0,0) };

			const FT err = (newton_raphson ? this->delta(new_V) : 0.0);
			this->store(new_V);
			return (err > this->m_params.m_accuracy) ? 2 : 1;
		}

	};



} //namespace devices
} // namespace netlist


#endif /* NLD_MS_DIRECT1_H_ */
