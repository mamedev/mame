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

#include "nld_solver.h"
#include "nld_ms_direct.h"

template <int m_N, int _storage_N>
class netlist_matrix_solver_SOR_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_SOR_t(const netlist_solver_parameters_t &params, int size)
		: netlist_matrix_solver_direct_t<m_N, _storage_N>(netlist_matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_lp_fact(0)
		, m_gs_fail(0)
		, m_gs_total(0)
		{
		}

	virtual ~netlist_matrix_solver_SOR_t() {}

	/* ATTR_COLD */ virtual void log_stats();

	ATTR_HOT virtual int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();

private:
	nl_double m_lp_fact;
	int m_gs_fail;
	int m_gs_total;
};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
void netlist_matrix_solver_SOR_t<m_N, _storage_N>::log_stats()
{
	if (this->m_stat_calculations != 0 && this->m_params.m_log_stats)
	{
		this->netlist().log("==============================================");
		this->netlist().log("Solver %s", this->name().cstr());
		this->netlist().log("       ==> %d nets", this->N()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
		this->netlist().log("       has %s elements", this->is_dynamic() ? "dynamic" : "no dynamic");
		this->netlist().log("       has %s elements", this->is_timestep() ? "timestep" : "no timestep");
		this->netlist().log("       %6.3f average newton raphson loops", (double) this->m_stat_newton_raphson / (double) this->m_stat_vsolver_calls);
		this->netlist().log("       %10d invocations (%6d Hz)  %10d gs fails (%6.2f%%) %6.3f average",
				this->m_stat_calculations,
				this->m_stat_calculations * 10 / (int) (this->netlist().time().as_double() * 10.0),
				this->m_gs_fail,
				100.0 * (double) this->m_gs_fail / (double) this->m_stat_calculations,
				(double) this->m_gs_total / (double) this->m_stat_calculations);
	}
}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_SOR_t<m_N, _storage_N>::vsolve()
{
	this->solve_base(this);
	return this->compute_next_timestep();
}

template <int m_N, int _storage_N>
ATTR_HOT inline int netlist_matrix_solver_SOR_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const int iN = this->N();
	bool resched = false;
	int  resched_cnt = 0;

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + nl_math::sqrt(1-rho))
	 */

	const nl_double ws = this->m_params.m_sor; //1.045; //2.0 / (1.0 + /*sin*/(3.14159 * 5.5 / (double) (m_nets.count()+1)));
	//const nl_double ws = 2.0 / (1.0 + sin(3.14159 * 4 / (double) (this->N())));

	ATTR_ALIGN nl_double w[_storage_N];
	ATTR_ALIGN nl_double one_m_w[_storage_N];
	ATTR_ALIGN nl_double RHS[_storage_N];
	ATTR_ALIGN nl_double new_V[_storage_N];

	for (int k = 0; k < iN; k++)
	{
		nl_double gtot_t = 0.0;
		nl_double gabs_t = 0.0;
		nl_double RHS_t = 0.0;

		new_V[k] = this->m_nets[k]->m_cur_Analog;

		{
			const int term_count = this->m_terms[k]->count();
			const nl_double * const RESTRICT gt = this->m_terms[k]->gt();
			const nl_double * const RESTRICT go = this->m_terms[k]->go();
			const nl_double * const RESTRICT Idr = this->m_terms[k]->Idr();
			const nl_double * const *other_cur_analog = this->m_terms[k]->other_curanalog();

			for (int i = 0; i < term_count; i++)
			{
				gtot_t = gtot_t + gt[i];
				RHS_t = RHS_t + Idr[i];
			}

			if (USE_GABS)
				for (int i = 0; i < term_count; i++)
					gabs_t = gabs_t + nl_math::abs(go[i]);

			for (int i = this->m_terms[k]->m_railstart; i < term_count; i++)
				RHS_t = RHS_t  + go[i] * *other_cur_analog[i];
		}

		RHS[k] = RHS_t;

		if (USE_GABS)
		{
			gabs_t *= NL_FCONST(0.95); // avoid rounding issues
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

	do {
		resched = false;

		for (int k = 0; k < iN; k++)
		{
			const int * RESTRICT net_other = this->m_terms[k]->net_other();
			const int railstart = this->m_terms[k]->m_railstart;
			const nl_double * RESTRICT go = this->m_terms[k]->go();

			nl_double Idrive = 0.0;
			for (int i = 0; i < railstart; i++)
				Idrive = Idrive + go[i] * new_V[net_other[i]];

			const nl_double new_val = new_V[k] * one_m_w[k] + (Idrive + RHS[k]) * w[k];

			resched = resched || (nl_math::abs(new_val - new_V[k]) > accuracy);
			new_V[k] = new_val;
		}

		resched_cnt++;
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));

	if (newton_raphson)
	{
		//printf("here %s\n", this->name().cstr());
		for (int k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog += 1.0 * (new_V[k] - this->m_nets[k]->m_cur_Analog);
	}
	else
	{
		for (int k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog = new_V[k];
	}

	this->m_gs_total += resched_cnt;
	this->m_stat_calculations++;

	if (resched)
	{
		// Fallback to direct solver ...
		this->m_gs_fail++;
		return netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(newton_raphson);
	}
	else {
		return resched_cnt;
	}
}


#endif /* NLD_MS_SOR_H_ */
