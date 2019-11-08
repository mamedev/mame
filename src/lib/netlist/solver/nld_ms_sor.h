// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_MS_SOR_H_
#define NLD_MS_SOR_H_

///
/// \file nld_ms_sor.h
///
/// Generic successive over relaxation solver.
///
/// Fow w==1 we will do the classic Gauss-Seidel approach.
///

#include "nld_ms_direct.h"
#include "nld_solver.h"

#include <algorithm>

namespace netlist
{
namespace solver
{

	template <typename FT, int SIZE>
	class matrix_solver_SOR_t: public matrix_solver_direct_t<FT, SIZE>
	{
	public:

		using float_type = FT;

		matrix_solver_SOR_t(netlist_state_t &anetlist, const pstring &name,
			analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size)
			: matrix_solver_direct_t<FT, SIZE>(anetlist, name, nets, params, size)
			, m_lp_fact(*this, "m_lp_fact", 0)
			, w(size, plib::constants<FT>::zero())
			, one_m_w(size, plib::constants<FT>::zero())
			{
			}

		unsigned vsolve_non_dynamic(const bool newton_raphson) override;

	private:
		state_var<float_type> m_lp_fact;
		std::vector<float_type> w;
		std::vector<float_type> one_m_w;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver - Gauss - Seidel
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	unsigned matrix_solver_SOR_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{
		const std::size_t iN = this->size();
		bool resched = false;
		unsigned resched_cnt = 0;

		// ideally, we could get an estimate for the spectral radius of
		// Inv(D - L) * U
		//
		// and estimate using
		//
		// omega = 2.0 / (1.0 + std::sqrt(1-rho))
		//

		const auto ws(static_cast<float_type>(this->m_params.m_gs_sor));

		for (std::size_t k = 0; k < iN; k++)
		{
			nl_fptype gtot_t = nlconst::zero();
			nl_fptype gabs_t = nlconst::zero();
			nl_fptype RHS_t = nlconst::zero();

			const std::size_t term_count = this->m_terms[k].count();
			const nl_fptype * const gt = this->m_gtn[k];
			const nl_fptype * const go = this->m_gonn[k];
			const nl_fptype * const Idr = this->m_Idrn[k];
			auto other_cur_analog = this->m_connected_net_Vn[k];

			this->m_new_V[k] = this->m_terms[k].template getV<float_type>();

			for (std::size_t i = 0; i < term_count; i++)
			{
				gtot_t = gtot_t + gt[i];
				RHS_t = RHS_t + Idr[i];
			}

			for (std::size_t i = this->m_terms[k].railstart(); i < term_count; i++)
				RHS_t = RHS_t  - go[i] * *other_cur_analog[i];

			this->m_RHS[k] = static_cast<float_type>(RHS_t);

			if (this->m_params.m_use_gabs)
			{
				for (std::size_t i = 0; i < term_count; i++)
					gabs_t = gabs_t + plib::abs(go[i]);

				gabs_t *= nlconst::magic(0.5); // derived by try and error
				if (gabs_t <= gtot_t)
				{
					w[k] = ws / static_cast<float_type>(gtot_t);
					one_m_w[k] = plib::constants<FT>::one() - ws;
				}
				else
				{
					w[k] = plib::reciprocal(static_cast<float_type>(gtot_t + gabs_t));
					one_m_w[k] = plib::constants<FT>::one() - plib::constants<FT>::one() * static_cast<FT>(gtot_t / (gtot_t + gabs_t));
				}
			}
			else
			{
				w[k] = ws / static_cast<float_type>(gtot_t);
				one_m_w[k] = plib::constants<FT>::one() - ws;
			}
		}

		const auto accuracy(static_cast<float_type>(this->m_params.m_accuracy));

		do {
			resched = false;
			float_type err = 0;
			for (std::size_t k = 0; k < iN; k++)
			{
				const int * net_other = this->m_terms[k].m_connected_net_idx.data();
				const std::size_t railstart = this->m_terms[k].railstart();
				const nl_fptype * go = this->m_gonn[k];

				float_type Idrive = plib::constants<float_type>::zero();
				for (std::size_t i = 0; i < railstart; i++)
					Idrive = Idrive - static_cast<float_type>(go[i]) * this->m_new_V[static_cast<std::size_t>(net_other[i])];

				const float_type new_val = this->m_new_V[k] * one_m_w[k] + (Idrive + this->m_RHS[k]) * w[k];

				err = std::max(plib::abs(new_val - this->m_new_V[k]), err);
				this->m_new_V[k] = new_val;
			}

			if (err > accuracy)
				resched = true;

			resched_cnt++;
		} while (resched && (resched_cnt < this->m_params.m_gs_loops));

		this->m_iterative_total += resched_cnt;
		this->m_stat_calculations++;

		if (resched)
		{
			// Fallback to direct solver ...
			this->m_iterative_fail++;
			return matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic(newton_raphson);
		}

		bool err(false);
		if (newton_raphson)
			err = this->check_err();
		this->store();
		return (err) ? 2 : 1;
	}

} // namespace solver
} // namespace netlist

#endif // NLD_MS_SOR_H_
