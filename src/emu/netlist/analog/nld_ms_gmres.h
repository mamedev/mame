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

#ifndef NLD_MS_GMRES_H_
#define NLD_MS_GMRES_H_

#include <algorithm>

#include "nld_solver.h"
#include "nld_ms_direct.h"

#include "mgmres.hpp"

template <unsigned m_N, unsigned _storage_N>
class netlist_matrix_solver_GMRES_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_GMRES_t(const netlist_solver_parameters_t *params, int size)
		: netlist_matrix_solver_direct_t<m_N, _storage_N>(netlist_matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_gmres(m_N ? m_N : size)
		, m_gs_fail(0)
		, m_gs_total(0)
		{
		}

	virtual ~netlist_matrix_solver_GMRES_t() {}

	virtual void log_stats();

	virtual void vsetup(netlist_analog_net_t::list_t &nets);
	ATTR_HOT virtual int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();

private:
	gmres_t m_gmres;
	int m_gs_fail;
	int m_gs_total;
};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
void netlist_matrix_solver_GMRES_t<m_N, _storage_N>::log_stats()
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

template <unsigned m_N, unsigned _storage_N>
void netlist_matrix_solver_GMRES_t<m_N, _storage_N>::vsetup(netlist_analog_net_t::list_t &nets)
{
	netlist_matrix_solver_direct_t<m_N, _storage_N>::vsetup(nets);
	this->save(NLNAME(m_gs_fail));
	this->save(NLNAME(m_gs_total));
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_GMRES_t<m_N, _storage_N>::vsolve()
{
	this->solve_base(this);
	return this->compute_next_timestep();
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT inline int netlist_matrix_solver_GMRES_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const int iN = this->N();

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + nl_math::sqrt(1-rho))
	 */

	int nz_num = 0;
	int ia[_storage_N + 1];
	int ja[_storage_N * _storage_N];
	double a[_storage_N * _storage_N];
	ATTR_ALIGN nl_double RHS[_storage_N];
	ATTR_ALIGN nl_double new_V[_storage_N];
	ATTR_ALIGN nl_double l_V[_storage_N];

	for (int k = 0; k < iN; k++)
	{
		nl_double gtot_t = 0.0;
		nl_double RHS_t = 0.0;

		const int term_count = this->m_terms[k]->count();
		const int railstart = this->m_terms[k]->m_railstart;
		const nl_double * const RESTRICT gt = this->m_terms[k]->gt();
		const nl_double * const RESTRICT go = this->m_terms[k]->go();
		const nl_double * const RESTRICT Idr = this->m_terms[k]->Idr();
		const nl_double * const *other_cur_analog = this->m_terms[k]->other_curanalog();
		const int * RESTRICT net_other = this->m_terms[k]->net_other();

		ia[k] = nz_num;
		l_V[k] = new_V[k] = this->m_nets[k]->m_cur_Analog;
		for (unsigned i = 0; i < term_count; i++)
		{
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}
		for (int i = this->m_terms[k]->m_railstart; i < term_count; i++)
			RHS_t = RHS_t  + go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;
		// add diagonal element
		a[nz_num] = gtot_t;
		ja[nz_num] = k;
		nz_num++;

		int n=0;

		for (int i = 0; i < railstart; i++)
		{
			int j;
			for (j = 0; j < n; j++)
			{
				if (net_other[i] == ja[nz_num+j])
				{
					a[nz_num+j] -= go[i];
					break;
				}
			}
			if (j>=n)
			{
				a[nz_num+n] = -go[i];
				ja[nz_num+n] = net_other[i];
				n++;
			}
		}
		nz_num += n;
	}
	ia[iN] = nz_num;

	const nl_double accuracy = this->m_params.m_accuracy;

	int gsl = m_gmres.pmgmres_ilu_cr(nz_num, ia, ja, a, new_V, RHS, 1, std::min(iN-1,20), accuracy * (double) (iN), 1e6);

	m_gs_total += gsl;
	this->m_stat_calculations++;

	if (gsl>=19)
	{
		for (int k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog = new_V[k];
		// Fallback to direct solver ...
		this->m_gs_fail++;
		return netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(newton_raphson);
	}

	if (newton_raphson)
	{
		double err = 0;
		for (int k = 0; k < iN; k++)
			err = std::max(nl_math::abs(l_V[k] - new_V[k]), err);

		//printf("here %s\n", this->name().cstr());
		for (int k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog += 1.0 * (new_V[k] - this->m_nets[k]->m_cur_Analog);
		if (err > accuracy)
			return 2;
		else
			return 1;
	}
	else
	{
		for (int k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog = new_V[k];
		return 1;
	}
}


#endif /* NLD_MS_GMRES_H_ */
