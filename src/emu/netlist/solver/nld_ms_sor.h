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

#include "solver/nld_ms_direct.h"
#include "solver/nld_solver.h"

NETLIB_NAMESPACE_DEVICES_START()

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_SOR_t: public matrix_solver_direct_t<m_N, _storage_N>
{
public:

	matrix_solver_SOR_t(const solver_parameters_t *params, int size)
		: matrix_solver_direct_t<m_N, _storage_N>(matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_lp_fact(0)
		{
		}

	virtual ~matrix_solver_SOR_t() {}

	virtual void vsetup(analog_net_t::list_t &nets);
	ATTR_HOT virtual int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();

private:
	nl_double m_lp_fact;
};

// ----------------------------------------------------------------------------------------
// matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------


template <unsigned m_N, unsigned _storage_N>
void matrix_solver_SOR_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<m_N, _storage_N>::vsetup(nets);
	this->save(NLNAME(m_lp_fact));
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_SOR_t<m_N, _storage_N>::vsolve()
{
	this->solve_base(this);
	return this->compute_next_timestep();
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT inline int matrix_solver_SOR_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const unsigned iN = this->N();
	bool resched = false;
	int  resched_cnt = 0;

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + nl_math::sqrt(1-rho))
	 */

	const nl_double ws = this->m_params.m_sor;

	ATTR_ALIGN nl_double w[_storage_N];
	ATTR_ALIGN nl_double one_m_w[_storage_N];
	ATTR_ALIGN nl_double RHS[_storage_N];
	ATTR_ALIGN nl_double new_V[_storage_N];

	for (unsigned k = 0; k < iN; k++)
	{
		nl_double gtot_t = 0.0;
		nl_double gabs_t = 0.0;
		nl_double RHS_t = 0.0;

		const unsigned term_count = this->m_terms[k]->count();
		const nl_double * const RESTRICT gt = this->m_terms[k]->gt();
		const nl_double * const RESTRICT go = this->m_terms[k]->go();
		const nl_double * const RESTRICT Idr = this->m_terms[k]->Idr();
		const nl_double * const *other_cur_analog = this->m_terms[k]->other_curanalog();

		new_V[k] = this->m_nets[k]->m_cur_Analog;

		for (unsigned i = 0; i < term_count; i++)
		{
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}

		for (unsigned i = this->m_terms[k]->m_railstart; i < term_count; i++)
			RHS_t = RHS_t  + go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;

		if (USE_GABS)
		{
			for (unsigned i = 0; i < term_count; i++)
				gabs_t = gabs_t + nl_math::abs(go[i]);

			gabs_t *= NL_FCONST(0.5); // derived by try and error
			if (gabs_t <= gtot_t)
			{
				w[k] = ws / gtot_t;
				one_m_w[k] = NL_FCONST(1.0) - ws;
			}
			else
			{
				w[k] = NL_FCONST(1.0) / (gtot_t + gabs_t);
				one_m_w[k] = NL_FCONST(1.0) - NL_FCONST(1.0) * gtot_t / (gtot_t + gabs_t);
			}
		}
		else
		{
			w[k] = ws / gtot_t;
			one_m_w[k] = NL_FCONST(1.0) - ws;
		}
	}

	const nl_double accuracy = this->m_params.m_accuracy;

	/* uncommenting the line below will force dynamic updates every X iterations
	 * althought the system has not converged yet. This is a proof of concept,
	 * 91glub
	 *
	 */
	const bool interleaved_dynamic_updates = false;
	//const bool interleaved_dynamic_updates = newton_raphson;

	do {
		resched = false;
		nl_double err = 0;
		for (unsigned k = 0; k < iN; k++)
		{
			const int * RESTRICT net_other = this->m_terms[k]->net_other();
			const unsigned railstart = this->m_terms[k]->m_railstart;
			const nl_double * RESTRICT go = this->m_terms[k]->go();

			nl_double Idrive = 0.0;
			for (unsigned i = 0; i < railstart; i++)
				Idrive = Idrive + go[i] * new_V[net_other[i]];

			const nl_double new_val = new_V[k] * one_m_w[k] + (Idrive + RHS[k]) * w[k];

			err = std::max(nl_math::abs(new_val - new_V[k]), err);
			new_V[k] = new_val;
		}

		if (err > accuracy)
			resched = true;

		resched_cnt++;
	//} while (resched && (resched_cnt < this->m_params.m_gs_loops));
	} while (resched && ((!interleaved_dynamic_updates && resched_cnt < this->m_params.m_gs_loops) || (interleaved_dynamic_updates && resched_cnt < 5 )));

	this->m_iterative_total += resched_cnt;
	this->m_stat_calculations++;

	if (resched && !interleaved_dynamic_updates)
	{
		// Fallback to direct solver ...
		this->m_iterative_fail++;
		return matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(newton_raphson);
	}

	if (interleaved_dynamic_updates)
	{
		for (unsigned k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog += 1.0 * (new_V[k] - this->m_nets[k]->m_cur_Analog);
	}
	else
	{
		for (unsigned k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog = new_V[k];
	}

	return resched_cnt;
}

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_SOR_H_ */
