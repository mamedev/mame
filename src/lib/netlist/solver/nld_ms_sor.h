// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_sor.h
 *
 * Generic successive over relaxation solver.
 *
 * Fow w==1 we will do the classic Gauss-Seidel approach
 *
 */

#ifndef NLD_MS_SOR_H_
#define NLD_MS_SOR_H_

#include <algorithm>

#include "nld_ms_direct.h"
#include "nld_solver.h"

namespace netlist
{
	namespace devices
{

template <typename FT, int SIZE>
class matrix_solver_SOR_t: public matrix_solver_direct_t<FT, SIZE>
{
public:

	using float_type = FT;

	matrix_solver_SOR_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_direct_t<FT, SIZE>(anetlist, name, matrix_solver_t::ASCENDING, params, size)
		, m_lp_fact(*this, "m_lp_fact", 0)
		, w(size, 0.0)
		, one_m_w(size, 0.0)
		, RHS(size, 0.0)
		//, new_V(size, 0.0)
		{
		}

	void vsetup(analog_net_t::list_t &nets) override;
	unsigned vsolve_non_dynamic(const bool newton_raphson) override;

private:
	state_var<float_type> m_lp_fact;
	std::vector<float_type> w;
	std::vector<float_type> one_m_w;
	std::vector<float_type> RHS;
	//std::vector<float_type> new_V;
};

// ----------------------------------------------------------------------------------------
// matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------


template <typename FT, int SIZE>
void matrix_solver_SOR_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<FT, SIZE>::vsetup(nets);
}

template <typename FT, int SIZE>
unsigned matrix_solver_SOR_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
{
	const std::size_t iN = this->size();
	bool resched = false;
	unsigned resched_cnt = 0;

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + std::sqrt(1-rho))
	 */

	const float_type ws = this->m_params.m_gs_sor;

	for (std::size_t k = 0; k < iN; k++)
	{
		float_type gtot_t = 0.0;
		float_type gabs_t = 0.0;
		float_type RHS_t = 0.0;

		const std::size_t term_count = this->m_terms[k]->count();
		const float_type * const gt = this->m_terms[k]->gt();
		const float_type * const go = this->m_terms[k]->go();
		const float_type * const Idr = this->m_terms[k]->Idr();
		auto other_cur_analog = this->m_terms[k]->connected_net_V();

		this->m_new_V[k] = this->m_nets[k]->Q_Analog();

		for (std::size_t i = 0; i < term_count; i++)
		{
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}

		for (std::size_t i = this->m_terms[k]->m_railstart; i < term_count; i++)
			RHS_t = RHS_t  + go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;

		if (this->m_params.m_use_gabs)
		{
			for (std::size_t i = 0; i < term_count; i++)
				gabs_t = gabs_t + std::abs(go[i]);

			gabs_t *= plib::constants<nl_double>::cast(0.5); // derived by try and error
			if (gabs_t <= gtot_t)
			{
				w[k] = ws / gtot_t;
				one_m_w[k] = plib::constants<FT>::one() - ws;
			}
			else
			{
				w[k] = plib::constants<FT>::one() / (gtot_t + gabs_t);
				one_m_w[k] = plib::constants<FT>::one() - plib::constants<FT>::one() * gtot_t / (gtot_t + gabs_t);
			}
		}
		else
		{
			w[k] = ws / gtot_t;
			one_m_w[k] = plib::constants<FT>::one() - ws;
		}
	}

	const float_type accuracy = this->m_params.m_accuracy;

	do {
		resched = false;
		float_type err = 0;
		for (std::size_t k = 0; k < iN; k++)
		{
			const int * net_other = this->m_terms[k]->connected_net_idx();
			const std::size_t railstart = this->m_terms[k]->m_railstart;
			const float_type * go = this->m_terms[k]->go();

			float_type Idrive = 0.0;
			for (std::size_t i = 0; i < railstart; i++)
				Idrive = Idrive + go[i] * this->m_new_V[static_cast<std::size_t>(net_other[i])];

			const float_type new_val = this->m_new_V[k] * one_m_w[k] + (Idrive + RHS[k]) * w[k];

			err = std::max(std::abs(new_val - this->m_new_V[k]), err);
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

	const float_type err = (newton_raphson ? this->delta(this->m_new_V) : 0.0);
	this->store(this->m_new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_SOR_H_ */
