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

#include "nld_solver.h"
#include "nld_ms_direct.h"

template <int m_N, int _storage_N>
class netlist_matrix_solver_SOR_mat_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_SOR_mat_t(const netlist_solver_parameters_t &params, int size)
		: netlist_matrix_solver_direct_t<m_N, _storage_N>(netlist_matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_omega(params.m_sor)
		, m_lp_fact(0)
		, m_gs_fail(0)
		, m_gs_total(0)
		{
			pstring p = nl_util::environment("NETLIST_STATS");
			if (p != "")
				m_log_stats = (bool) p.as_long();
			else
				m_log_stats = false;
		}

	virtual ~netlist_matrix_solver_SOR_mat_t() {}

	/* ATTR_COLD */ virtual void log_stats();

	ATTR_HOT inline int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();

private:
	nl_double m_Vdelta[_storage_N];

	nl_double m_omega;
	nl_double m_lp_fact;
	int m_gs_fail;
	int m_gs_total;
	bool m_log_stats;

};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
void netlist_matrix_solver_SOR_mat_t<m_N, _storage_N>::log_stats()
{
	if (this->m_stat_calculations != 0 && m_log_stats)
	{
		this->netlist().log("==============================================\n");
		this->netlist().log("Solver %s\n", this->name().cstr());
		this->netlist().log("       ==> %d nets\n", this->N()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
		this->netlist().log("       has %s elements\n", this->is_dynamic() ? "dynamic" : "no dynamic");
		this->netlist().log("       has %s elements\n", this->is_timestep() ? "timestep" : "no timestep");
		this->netlist().log("       %6.3f average newton raphson loops\n", (double) this->m_stat_newton_raphson / (double) this->m_stat_vsolver_calls);
		this->netlist().log("       %10d invocations (%6d Hz)  %10d gs fails (%6.2f%%) %6.3f average\n",
				this->m_stat_calculations,
				this->m_stat_calculations * 10 / (int) (this->netlist().time().as_double() * 10.0),
				this->m_gs_fail,
				100.0 * (double) this->m_gs_fail / (double) this->m_stat_calculations,
				(double) this->m_gs_total / (double) this->m_stat_calculations);
	}
}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_SOR_mat_t<m_N, _storage_N>::vsolve()
{
	/*
	 * enable linear prediction on first newton pass
	 */

	if (USE_LINEAR_PREDICTION)
		for (int k = 0; k < this->N(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
			this->m_nets[k]->m_cur_Analog = this->m_nets[k]->m_cur_Analog + this->m_Vdelta[k] * this->current_timestep() * m_lp_fact;
		}
	else
		for (int k = 0; k < this->N(); k++)
		{
			this->m_last_V[k] = this->m_nets[k]->m_cur_Analog;
		}

	this->solve_base(this);

	if (USE_LINEAR_PREDICTION)
	{
		nl_double sq = 0;
		nl_double sqo = 0;
		const nl_double rez_cts = 1.0 / this->current_timestep();
		for (int k = 0; k < this->N(); k++)
		{
			const netlist_analog_net_t *n = this->m_nets[k];
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

template <int m_N, int _storage_N>
ATTR_HOT inline int netlist_matrix_solver_SOR_mat_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	/* The matrix based code looks a lot nicer but actually is 30% slower than
	 * the optimized code which works directly on the data structures.
	 * Need something like that for gaussian elimination as well.
	 */


	ATTR_ALIGN nl_double new_v[_storage_N] = { 0.0 };
	const int iN = this->N();

	bool resched = false;

	int  resched_cnt = 0;

	this->build_LE_A();
	this->build_LE_RHS();

#if 1
#if 0
	static int ws_cnt = 0;
	ws_cnt++;
	if (0 && ws_cnt % 100 == 0)
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
			//if ( akk > lambdaN)
			//  lambdaN = akk;
			if (akk < lambda1)
				lambda1 = akk;
	#endif
		}
		//printf("lambda: %f %f\n", lambda, 2.0 / (1.0 + 2 * sqrt(lambda)) );

		//ws = 2.0 / (2.0 - lambdaN - lambda1);
		m_omega = 2.0 / (2.0 - lambda1);
		//printf("%f\n", ws);
	}
#endif

	for (int k = 0; k < iN; k++)
		new_v[k] = this->m_nets[k]->m_cur_Analog;

#else
	{
		nl_double frob;
		frob = 0;
		nl_double rmin = 1e99, rmax = -1e99;
		for (int k = 0; k < iN; k++)
		{
			new_v[k] = this->m_nets[k]->m_cur_Analog;
			nl_double s=0.0;
			for (int i = 0; i < iN; i++)
			{
				frob += this->m_A[k][i] * this->m_A[k][i];
				s = s + nl_math::abs(this->m_A[k][i]);
			}

			if (s<rmin)
				rmin = s;
			if (s>rmax)
				rmax = s;
		}
#if 0
		nl_double frobA = nl_math::sqrt(frob /(iN));
		if (1 &&frobA < 1.0)
			//ws = 2.0 / (1.0 + nl_math::sqrt(1.0-frobA));
			ws = 2.0 / (2.0 - frobA);
		else
			ws = 1.0;
		ws = 0.9;
#else
		// calculate an estimate for rho.
		// This is based on the Perron???Frobenius theorem for positive matrices.
		// No mathematical proof here. The following estimates the
		// optimal relaxation parameter pretty well. Unfortunately, the
		// overhead is bigger than the gain. Consequently the fast GS below
		// uses a fixed GS. One can however use this here to determine a
		// suitable parameter.
		nl_double rm = (rmax + rmin) * 0.5;
		if (rm < 1.0)
			ws = 2.0 / (1.0 + nl_math::sqrt(1.0-rm));
		else
			ws = 1.0;
		if (ws > 1.02 && rmax > 1.001)
			printf("rmin %f rmax %f ws %f\n", rmin, rmax, ws);
#endif
	}
#endif
	// Frobenius norm for (D-L)^(-1)U
	//nl_double frobU;
	//nl_double frobL;
	//nl_double norm;
	do {
		resched = false;
		nl_double cerr = 0.0;
		//frobU = 0;
		//frobL = 0;
		//norm = 0;

		for (int k = 0; k < iN; k++)
		{
			nl_double Idrive = 0;
			//nl_double norm_t = 0;
			// Reduction loops need -ffast-math
			for (int i = 0; i < iN; i++)
				Idrive += this->m_A[k][i] * new_v[i];

#if 0
			for (int i = 0; i < iN; i++)
			{
				if (i < k) frobL += this->m_A[k][i] * this->m_A[k][i] / this->m_A[k][k] /this-> m_A[k][k];
				if (i > k) frobU += this->m_A[k][i] * this->m_A[k][i] / this->m_A[k][k] / this->m_A[k][k];
				norm_t += nl_math::abs(this->m_A[k][i]);
			}
#endif
			//if (norm_t > norm) norm = norm_t;
#if 0
			const nl_double new_val = (1.0-ws) * new_v[k] + ws * (this->m_RHS[k] - Idrive + this->m_A[k][k] * new_v[k]) / this->m_A[k][k];

			const nl_double e = nl_math::abs(new_val - new_v[k]);
			cerr = (e > cerr ? e : cerr);
			new_v[k] = new_val;
#else
			const nl_double delta = m_omega * (this->m_RHS[k] - Idrive) / this->m_A[k][k];
			const nl_double adelta = nl_math::abs(delta);
			cerr = (adelta > cerr ? adelta : cerr);
			new_v[k] += delta;
#endif
		}

		if (cerr > this->m_params.m_accuracy)
		{
			resched = true;
		}
		resched_cnt++;
		//ATTR_UNUSED nl_double frobUL = nl_math::sqrt((frobU + frobL) / (double) (iN) / (double) (iN));
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));
	//printf("Frobenius %f %f %f %f %f\n", nl_math::sqrt(frobU), nl_math::sqrt(frobL), frobUL, frobA, norm);
	//printf("Omega Estimate1 %f %f\n", 2.0 / (1.0 + nl_math::sqrt(1-frobUL)), 2.0 / (1.0 + nl_math::sqrt(1-frobA)) ); //        printf("Frobenius %f\n", sqrt(frob / (double) (iN * iN) ));
	//printf("Omega Estimate2 %f %f\n", 2.0 / (2.0 - frobUL), 2.0 / (2.0 - frobA) ); //        printf("Frobenius %f\n", sqrt(frob / (double) (iN * iN) ));


	this->store(new_v, false);
	this->m_gs_total += resched_cnt;
	if (resched)
	{
		//this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
		this->m_gs_fail++;

		this->LE_solve();
		this->m_stat_calculations++;
		return netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic(newton_raphson);
	}
	else {
		this->m_stat_calculations++;

		return resched_cnt;
	}

}


#endif /* NLD_MS_GAUSS_SEIDEL_H_ */
