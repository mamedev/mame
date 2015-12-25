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

#ifndef NLD_MS_SOR_MAT_H_
#define NLD_MS_SOR_MAT_H_

#include <algorithm>

#include "solver/nld_ms_direct.h"
#include "solver/nld_solver.h"

NETLIB_NAMESPACE_DEVICES_START()

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_SOR_mat_t: public matrix_solver_direct_t<m_N, _storage_N>
{
public:

	matrix_solver_SOR_mat_t(const solver_parameters_t *params, int size)
		: matrix_solver_direct_t<m_N, _storage_N>(matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_omega(params->m_sor)
		, m_lp_fact(0)
		, m_gs_fail(0)
		, m_gs_total(0)
		{
		}

	virtual ~matrix_solver_SOR_mat_t() {}

	virtual void vsetup(analog_net_t::list_t &nets) override;

	ATTR_HOT inline int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve() override;

private:
	nl_double m_Vdelta[_storage_N];

	nl_double m_omega;
	nl_double m_lp_fact;
	int m_gs_fail;
	int m_gs_total;
};

// ----------------------------------------------------------------------------------------
// matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_SOR_mat_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<m_N, _storage_N>::vsetup(nets);
	this->save(NLNAME(m_omega));
	this->save(NLNAME(m_lp_fact));
	this->save(NLNAME(m_gs_fail));
	this->save(NLNAME(m_gs_total));
	this->save(NLNAME(m_Vdelta));
}


template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_SOR_mat_t<m_N, _storage_N>::vsolve()
{
	/*
	 * enable linear prediction on first newton pass
	 */

	if (USE_LINEAR_PREDICTION)
		for (unsigned k = 0; k < this->N(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
			this->m_nets[k]->m_cur_Analog = this->m_nets[k]->m_cur_Analog + this->m_Vdelta[k] * this->current_timestep() * m_lp_fact;
		}
	else
		for (unsigned k = 0; k < this->N(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
		}

	this->solve_base(this);

	if (USE_LINEAR_PREDICTION)
	{
		nl_double sq = 0;
		nl_double sqo = 0;
		const nl_double rez_cts = 1.0 / this->current_timestep();
		for (unsigned k = 0; k < this->N(); k++)
		{
			const analog_net_t *n = this->m_nets[k];
			const nl_double nv = (n->m_cur_Analog - this->m_last_V[k]) * rez_cts ;
			sq += nv * nv;
			sqo += this->m_Vdelta[k] * this->m_Vdelta[k];
			this->m_Vdelta[k] = nv;
		}

		// FIXME: used to be 1e90, but this would not be compatible with float
		if (sqo > NL_FCONST(1e-20))
			m_lp_fact = std::min(nl_math::sqrt(sq/sqo), (nl_double) 2.0);
		else
			m_lp_fact = NL_FCONST(0.0);
	}


	return this->compute_next_timestep();
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT inline int matrix_solver_SOR_mat_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	/* The matrix based code looks a lot nicer but actually is 30% slower than
	 * the optimized code which works directly on the data structures.
	 * Need something like that for gaussian elimination as well.
	 */


	ATTR_ALIGN nl_double new_v[_storage_N] = { 0.0 };
	const unsigned iN = this->N();

	bool resched = false;

	int  resched_cnt = 0;

	this->build_LE_A();
	this->build_LE_RHS(this->m_RHS);

#if 0
	static int ws_cnt = 0;
	ws_cnt++;
	if (1 && ws_cnt % 200 == 0)
	{
		// update omega
		nl_double lambdaN = 0;
		nl_double lambda1 = 1e9;
		for (int k = 0; k < iN; k++)
		{
	#if 0
			nl_double akk = nl_math::abs(this->m_A[k][k]);
			if ( akk > lambdaN)
				lambdaN = akk;
			if (akk < lambda1)
				lambda1 = akk;
	#else
			nl_double akk = nl_math::abs(this->m_A[k][k]);
			nl_double s = 0.0;
			for (int i=0; i<iN; i++)
				s = s + nl_math::abs(this->m_A[k][i]);
			akk = s / akk - 1.0;
			if ( akk > lambdaN)
				lambdaN = akk;
			if (akk < lambda1)
				lambda1 = akk;
	#endif
		}
		//printf("lambda: %f %f\n", lambda, 2.0 / (1.0 + 2 * sqrt(lambda)) );

		//ws = 2.0 / (2.0 - lambdaN - lambda1);
		m_omega = 2.0 / (2.0 - lambda1);
		//printf("%f %f %f\n", m_omega, lambda1, lambdaN);
	}
#endif

	for (unsigned k = 0; k < iN; k++)
		new_v[k] = this->m_nets[k]->m_cur_Analog;

	do {
		resched = false;
		nl_double cerr = 0.0;

		for (unsigned k = 0; k < iN; k++)
		{
			nl_double Idrive = 0;

			const unsigned *p = this->m_terms[k]->m_nz.data();
			const unsigned e = this->m_terms[k]->m_nz.size();

			for (unsigned i = 0; i < e; i++)
				Idrive = Idrive + this->A(k,p[i]) * new_v[p[i]];

			const nl_double delta = m_omega * (this->m_RHS[k] - Idrive) / this->A(k,k);
			cerr = std::max(cerr, nl_math::abs(delta));
			new_v[k] += delta;
		}

		if (cerr > this->m_params.m_accuracy)
		{
			resched = true;
		}
		resched_cnt++;
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));

	this->m_stat_calculations++;
	this->m_gs_total += resched_cnt;

	if (resched)
	{
		//this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
		this->m_gs_fail++;

		this->LE_solve();
		return matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic(newton_raphson);
	}
	else {
		this->store(new_v);
		return resched_cnt;
	}

}

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_GAUSS_SEIDEL_H_ */
