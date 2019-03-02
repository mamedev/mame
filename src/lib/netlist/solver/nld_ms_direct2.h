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
namespace devices
{

	// ----------------------------------------------------------------------------------------
	// matrix_solver - Direct2
	// ----------------------------------------------------------------------------------------

	template <typename FT>
	class matrix_solver_direct2_t: public matrix_solver_direct_t<FT, 2>
	{
	public:

		using float_type = FT;

		matrix_solver_direct2_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params)
			: matrix_solver_direct_t<double, 2>(anetlist, name, params, 2)
			{}
		unsigned vsolve_non_dynamic(const bool newton_raphson) override
		{
			this->build_LE_A(*this);
			this->build_LE_RHS(*this);

			const float_type a = this->A(0,0);
			const float_type b = this->A(0,1);
			const float_type c = this->A(1,0);
			const float_type d = this->A(1,1);

			const float_type v1 = (a * this->RHS(1) - c * this->RHS(0)) / (a * d - b * c);
			const float_type v0 = (this->RHS(0) - b * v1) / a;
			std::array<float_type, 2> new_V = {v0, v1};

			const float_type err = (newton_raphson ? this->delta(new_V) : 0.0);
			this->store(new_V);
			return (err > this->m_params.m_accuracy) ? 2 : 1;
		}

	};

} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT2_H_ */
