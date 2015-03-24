/*
 * nld_ms_direct1.h
 *
 */

#ifndef NLD_MS_GAUSS_SEIDEL_H_
#define NLD_MS_GAUSS_SEIDEL_H_

#include <cmath>

#include "nld_solver.h"
#include "nld_ms_direct.h"

template <int m_N, int _storage_N>
class ATTR_ALIGNED(64) netlist_matrix_solver_gauss_seidel_t: public netlist_matrix_solver_direct_t<m_N, _storage_N>
{
public:

	netlist_matrix_solver_gauss_seidel_t(const netlist_solver_parameters_t &params, int size)
		: netlist_matrix_solver_direct_t<m_N, _storage_N>(netlist_matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_lp_fact(0)
		, m_gs_fail(0)
		, m_gs_total(0)
		{
			const char *p = osd_getenv("NETLIST_STATS");
			if (p != NULL)
				m_log_stats = (bool) atoi(p);
			else
				m_log_stats = false;
		}

	virtual ~netlist_matrix_solver_gauss_seidel_t() {}

	ATTR_COLD virtual void log_stats();

	ATTR_HOT inline int vsolve_non_dynamic();
protected:
	ATTR_HOT virtual nl_double vsolve();

private:
	nl_double m_lp_fact;
	int m_gs_fail;
	int m_gs_total;
	bool m_log_stats;

};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
void netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::log_stats()
{
	if (this->m_stat_calculations != 0 && m_log_stats)
	{
		printf("==============================================\n");
		printf("Solver %s\n", this->name().cstr());
		printf("       ==> %d nets\n", this->N()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
		printf("       has %s elements\n", this->is_dynamic() ? "dynamic" : "no dynamic");
		printf("       has %s elements\n", this->is_timestep() ? "timestep" : "no timestep");
		printf("       %6.3f average newton raphson loops\n", (double) this->m_stat_newton_raphson / (double) this->m_stat_vsolver_calls);
		printf("       %10d invocations (%6d Hz)  %10d gs fails (%6.2f%%) %6.3f average\n",
				this->m_stat_calculations,
				this->m_stat_calculations * 10 / (int) (this->netlist().time().as_double() * 10.0),
				this->m_gs_fail,
				100.0 * (double) this->m_gs_fail / (double) this->m_stat_calculations,
				(double) this->m_gs_total / (double) this->m_stat_calculations);
	}
}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::vsolve()
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
		if (sqo > 1e-90)
			m_lp_fact = std::min(sqrt(sq/sqo), 2.0);
		else
			m_lp_fact = 0.0;
	}


	return this->compute_next_timestep();
}

template <int m_N, int _storage_N>
ATTR_HOT inline int netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::vsolve_non_dynamic()
{
	/* The matrix based code looks a lot nicer but actually is 30% slower than
	 * the optimized code which works directly on the data structures.
	 * Need something like that for gaussian elimination as well.
	 */

#if 0 || USE_MATRIX_GS
	static nl_double ws = 1.0;
	ATTR_ALIGN nl_double new_v[_storage_N] = { 0.0 };
	const int iN = this->N();

	bool resched = false;

	int  resched_cnt = 0;

	this->build_LE();

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
				s = s + fabs(this->m_A[k][i]);
			}

			if (s<rmin)
				rmin = s;
			if (s>rmax)
				rmax = s;
		}
#if 0
		nl_double frobA = sqrt(frob /(iN));
		if (1 &&frobA < 1.0)
			//ws = 2.0 / (1.0 + sqrt(1.0-frobA));
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
			ws = 2.0 / (1.0 + sqrt(1.0-rm));
		else
			ws = 1.0;
		if (ws > 1.02 && rmax > 1.001)
			printf("rmin %f rmax %f ws %f\n", rmin, rmax, ws);
#endif
	}

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

			for (int i = 0; i < iN; i++)
			{
				//if (i < k) frobL += this->m_A[k][i] * this->m_A[k][i] / this->m_A[k][k] /this-> m_A[k][k];
				//if (i > k) frobU += this->m_A[k][i] * this->m_A[k][i] / this->m_A[k][k] / this->m_A[k][k];
				//norm_t += fabs(this->m_A[k][i]);
			}

			//if (norm_t > norm) norm = norm_t;
			const nl_double new_val = (1.0-ws) * new_v[k] + ws * (this->m_RHS[k] - Idrive + this->m_A[k][k] * new_v[k]) / this->m_A[k][k];

			const nl_double e = fabs(new_val - new_v[k]);
			cerr = (e > cerr ? e : cerr);
			new_v[k] = new_val;
		}

		if (cerr > this->m_params.m_accuracy)
		{
			resched = true;
		}
		resched_cnt++;
		//ATTR_UNUSED nl_double frobUL = sqrt((frobU + frobL) / (double) (iN) / (double) (iN));
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));
	//printf("Frobenius %f %f %f %f %f\n", sqrt(frobU), sqrt(frobL), frobUL, frobA, norm);
	//printf("Omega Estimate1 %f %f\n", 2.0 / (1.0 + sqrt(1-frobUL)), 2.0 / (1.0 + sqrt(1-frobA)) ); //        printf("Frobenius %f\n", sqrt(frob / (double) (iN * iN) ));
	//printf("Omega Estimate2 %f %f\n", 2.0 / (2.0 - frobUL), 2.0 / (2.0 - frobA) ); //        printf("Frobenius %f\n", sqrt(frob / (double) (iN * iN) ));


	this->store(new_v, false);

	this->m_gs_total += resched_cnt;
	if (resched)
	{
		//this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
		this->m_gs_fail++;
		int tmp = netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic();
		this->m_calculations++;
		return tmp;
	}
	else {
		this->m_calculations++;

		return resched_cnt;
	}

#else
	const int iN = this->N();
	bool resched = false;
	int  resched_cnt = 0;

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + sqrt(1-rho))
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
#if VECTALT
			for (int i = 0; i < term_count; i++)
			{
				gtot_t = gtot_t + gt[i];
				RHS_t = RHS_t + Idr[i];
			}
			if (USE_GABS)
				for (int i = 0; i < term_count; i++)
					gabs_t = gabs_t + fabs(go[i]);
#else
			if (USE_GABS)
				this->m_terms[k]->ops()->sum2a(gt, Idr, go, gtot_t, RHS_t, gabs_t);
			else
				this->m_terms[k]->ops()->sum2(gt, Idr, gtot_t, RHS_t);
#endif
			for (int i = this->m_terms[k]->m_railstart; i < term_count; i++)
				RHS_t = RHS_t  + go[i] * *other_cur_analog[i];
		}

		RHS[k] = RHS_t;

		//if (fabs(gabs_t - fabs(gtot_t)) > 1e-20)
		//    printf("%d %e abs: %f tot: %f\n",k, gabs_t / gtot_t -1.0, gabs_t, gtot_t);

		gabs_t *= 0.95; // avoid rounding issues
		if (!USE_GABS || gabs_t <= gtot_t)
		{
			w[k] = ws / gtot_t;
			one_m_w[k] = 1.0 - ws;
		}
		else
		{
			//printf("abs: %f tot: %f\n", gabs_t, gtot_t);
			w[k] = 1.0 / (gtot_t + gabs_t);
			one_m_w[k] = 1.0 - 1.0 * gtot_t / (gtot_t + gabs_t);
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

			//nl_double new_val = (net->m_cur_Analog * gabs[k] + iIdr) / (gtot[k]);
			const nl_double new_val = new_V[k] * one_m_w[k] + (Idrive + RHS[k]) * w[k];

			resched = resched || (std::abs(new_val - new_V[k]) > accuracy);
			new_V[k] = new_val;
		}

		resched_cnt++;
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));

	for (int k = 0; k < iN; k++)
		this->m_nets[k]->m_cur_Analog = new_V[k];

	this->m_gs_total += resched_cnt;
	this->m_stat_calculations++;

	if (resched)
	{
		//this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
		this->m_gs_fail++;
		return netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic();
	}
	else {
		return resched_cnt;
	}
#endif
}


#endif /* NLD_MS_GAUSS_SEIDEL_H_ */
