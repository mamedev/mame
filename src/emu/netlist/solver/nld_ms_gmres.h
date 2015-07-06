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

#include "../solver/mat_cr.h"
#include "../solver/nld_ms_direct.h"
#include "../solver/nld_solver.h"
#include "../solver/vector_base.h"

NETLIB_NAMESPACE_DEVICES_START()

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_GMRES_t: public matrix_solver_direct_t<m_N, _storage_N>
{
public:

	matrix_solver_GMRES_t(const solver_parameters_t *params, int size)
		: matrix_solver_direct_t<m_N, _storage_N>(matrix_solver_t::GAUSS_SEIDEL, params, size)
		, m_use_iLU_preconditioning(true)
		, m_use_more_precise_stop_condition(false)
		, m_accuracy_mult(1.0)
		{
			unsigned mr=this->N(); /* FIXME: maximum iterations locked in here */

			for (unsigned i = 0; i < mr + 1; i++)
				m_ht[i] = new double[mr];

			for (unsigned i = 0; i < this->N(); i++)
				m_v[i] = new double[_storage_N];

		}

	virtual ~matrix_solver_GMRES_t()
	{
		unsigned mr=this->N(); /* FIXME: maximum iterations locked in here */

		for (unsigned i = 0; i < mr + 1; i++)
			delete[] m_ht[i];

		for (unsigned i = 0; i < this->N(); i++)
			delete[] m_v[i];
	}

	virtual void vsetup(analog_net_t::list_t &nets);
	ATTR_HOT virtual int vsolve_non_dynamic(const bool newton_raphson);
protected:
	ATTR_HOT virtual nl_double vsolve();

private:

	int solve_ilu_gmres(double * RESTRICT x, double * RESTRICT rhs, const unsigned restart_max, const unsigned mr, double accuracy);

	plist_t<int> m_term_cr[_storage_N];

	bool m_use_iLU_preconditioning;
	bool m_use_more_precise_stop_condition;

	mat_cr_t<_storage_N> mat;

	double m_A[_storage_N * _storage_N];
	double m_LU[_storage_N * _storage_N];

	double m_c[_storage_N + 1];  /* mr + 1 */
	double m_g[_storage_N + 1];  /* mr + 1 */
	double * RESTRICT m_ht[_storage_N + 1];  /* (mr + 1), mr */
	double m_s[_storage_N];     /* mr + 1 */
	double * RESTRICT m_v[_storage_N + 1];      /*(mr + 1), n */
	//double m_y[_storage_N];       /* mr + 1 */

	double m_accuracy_mult; // FXIME: Save state
};

// ----------------------------------------------------------------------------------------
// matrix_solver - GMRES
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_GMRES_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<m_N, _storage_N>::vsetup(nets);

	unsigned nz = 0;
	const unsigned iN = this->N();

	for (unsigned k=0; k<iN; k++)
	{
		terms_t * RESTRICT row = this->m_terms[k];
		mat.ia[k] = nz;

		for (unsigned j=0; j<row->m_nz.size(); j++)
		{
			mat.ja[nz] = row->m_nz[j];
			if (row->m_nz[j] == k)
				mat.diag[k] = nz;
			nz++;
		}

		/* build pointers into the compressed row format matrix for each terminal */

		for (unsigned j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			for (unsigned i = mat.ia[k]; i<nz; i++)
				if (this->m_terms[k]->net_other()[j] == (int) mat.ja[i])
				{
					m_term_cr[k].add(i);
					break;
				}
			nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
		}
	}

	mat.ia[iN] = nz;
	mat.nz_num = nz;
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_GMRES_t<m_N, _storage_N>::vsolve()
{
	this->solve_base(this);
	return this->compute_next_timestep();
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT inline int matrix_solver_GMRES_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const unsigned iN = this->N();

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + nl_math::sqrt(1-rho))
	 */

	//nz_num = 0;
	ATTR_ALIGN nl_double RHS[_storage_N];
	ATTR_ALIGN nl_double new_V[_storage_N];
	ATTR_ALIGN nl_double l_V[_storage_N];

	for (unsigned i=0, e=mat.nz_num; i<e; i++)
		m_A[i] = 0.0;

	for (unsigned k = 0; k < iN; k++)
	{
		nl_double gtot_t = 0.0;
		nl_double RHS_t = 0.0;

		const unsigned term_count = this->m_terms[k]->count();
		const unsigned railstart = this->m_terms[k]->m_railstart;
		const nl_double * const RESTRICT gt = this->m_terms[k]->gt();
		const nl_double * const RESTRICT go = this->m_terms[k]->go();
		const nl_double * const RESTRICT Idr = this->m_terms[k]->Idr();
		const nl_double * const * RESTRICT other_cur_analog = this->m_terms[k]->other_curanalog();

		l_V[k] = new_V[k] = this->m_nets[k]->m_cur_Analog;
		for (unsigned i = 0; i < term_count; i++)
		{
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}

		for (unsigned i = railstart; i < term_count; i++)
			RHS_t = RHS_t  + go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;

		// add diagonal element
		m_A[mat.diag[k]] = gtot_t;

		for (unsigned i = 0; i < railstart; i++)
		{
			const unsigned pi = m_term_cr[k][i];
			m_A[pi] -= go[i];
		}
	}
	mat.ia[iN] = mat.nz_num;

	const nl_double accuracy = this->m_params.m_accuracy;
#if 1
	int mr = std::min((int) iN-1,(int) sqrt(iN));
	mr = std::min(mr, this->m_params.m_gs_loops);
	int iter = 4;
	int gsl = solve_ilu_gmres(new_V, RHS, iter, mr, accuracy);
	int failed = mr * iter;
#else
	int failed = 6;
	//int gsl = tt_ilu_cr(new_V, RHS, failed, accuracy);
	int gsl = tt_gs_cr(new_V, RHS, failed, accuracy);
#endif
	this->m_iterative_total += gsl;
	this->m_stat_calculations++;

	if (gsl>=failed)
	{
		//for (int k = 0; k < iN; k++)
		//  this->m_nets[k]->m_cur_Analog = new_V[k];
		// Fallback to direct solver ...
		this->m_iterative_fail++;
		return matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(newton_raphson);
	}

	if (newton_raphson)
	{
		double err = 0;
		for (unsigned k = 0; k < iN; k++)
			err = std::max(nl_math::abs(l_V[k] - new_V[k]), err);

		//printf("here %s\n", this->name().cstr());
		for (unsigned k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog += 1.0 * (new_V[k] - this->m_nets[k]->m_cur_Analog);
		if (err > accuracy)
			return 2;
		else
			return 1;
	}
	else
	{
		for (unsigned k = 0; k < iN; k++)
			this->m_nets[k]->m_cur_Analog = new_V[k];
		return 1;
	}
}

static inline void givens_mult( const double c, const double s, double * RESTRICT g0, double * RESTRICT g1 )
{
	const double tg0 = c * *g0 - s * *g1;
	const double tg1 = s * *g0 + c * *g1;

	*g0 = tg0;
	*g1 = tg1;
}

template <unsigned m_N, unsigned _storage_N>
int matrix_solver_GMRES_t<m_N, _storage_N>::solve_ilu_gmres (double * RESTRICT x, double * RESTRICT rhs, const unsigned restart_max, const unsigned mr, double accuracy)
{
	/*-------------------------------------------------------------------------
	 * The code below was inspired by code published by John Burkardt under
	 * the LPGL here:
	 *
	 * http://people.sc.fsu.edu/~jburkardt/cpp_src/mgmres/mgmres.html
	 *
	 * The code below was completely written from scratch based on the pseudo code
	 * found here:
	 *
	 * http://de.wikipedia.org/wiki/GMRES-Verfahren
	 *
	 * The Algorithm itself is described in
	 *
	 * Yousef Saad,
	 * Iterative Methods for Sparse Linear Systems,
	 * Second Edition,
	 * SIAM, 20003,
	 * ISBN: 0898715342,
	 * LC: QA188.S17.
	 *
	 *------------------------------------------------------------------------*/

	unsigned itr_used = 0;
	double rho_delta = 0.0;

	const unsigned n = this->N();

	if (m_use_iLU_preconditioning)
		mat.incomplete_LU_factorization(m_A, m_LU);

	if (m_use_more_precise_stop_condition)
	{
		/* derive residual for a given delta x
		 *
		 * LU y = A dx
		 *
		 * ==> rho / accuracy = sqrt(y * y)
		 *
		 * This approach will approximate the iterative stop condition
		 * based |xnew - xold| pretty precisely. But it is slow, or expressed
		 * differently: The invest doesn't pay off.
		 * Therefore we use the approach in the else part.
		 */
		double t[_storage_N];
		double Ax[_storage_N];
		vec_set(n, accuracy, t);
		mat.mult_vec(m_A, t, Ax);
		mat.solveLUx(m_LU, Ax);

		const double rho_to_accuracy = std::sqrt(vecmult2(n, Ax)) / accuracy;

		//printf("rho/accuracy = %f\n", rho_to_accuracy);

		rho_delta = accuracy * rho_to_accuracy;
	}
	else
		rho_delta = accuracy * std::sqrt((double) n) * m_accuracy_mult;

	for (unsigned itr = 0; itr < restart_max; itr++)
	{
		unsigned last_k = mr;
		double mu;
		double rho;

		double Ax[_storage_N];
		double residual[_storage_N];

		mat.mult_vec(m_A, x, Ax);

		vec_sub(n, rhs, Ax, residual);

		if (m_use_iLU_preconditioning)
		{
			mat.solveLUx(m_LU, residual);
		}

		rho = std::sqrt(vecmult2(n, residual));

		vec_mult_scalar(n, residual, 1.0 / rho, m_v[0]);

		vec_set(mr+1, 0.0, m_g);
		m_g[0] = rho;

		for (unsigned i = 0; i < mr; i++)
			vec_set(mr + 1, 0.0, m_ht[i]);

		for (unsigned k = 0; k < mr; k++)
		{
			const unsigned k1 = k + 1;

			mat.mult_vec(m_A, m_v[k], m_v[k1]);

			if (m_use_iLU_preconditioning)
				mat.solveLUx(m_LU, m_v[k1]);

			for (unsigned j = 0; j <= k; j++)
			{
				m_ht[j][k] = vecmult(n, m_v[k1], m_v[j]);
				vec_add_mult_scalar(n, m_v[j], -m_ht[j][k], m_v[k1]);
			}
			m_ht[k1][k] = std::sqrt(vecmult2(n, m_v[k1]));

			if (m_ht[k1][k] != 0.0)
				vec_scale(n, m_v[k1], 1.0 / m_ht[k1][k]);

			for (unsigned j = 0; j < k; j++)
				givens_mult(m_c[j], m_s[j], &m_ht[j][k], &m_ht[j+1][k]);

			mu = std::sqrt(std::pow(m_ht[k][k], 2) + std::pow(m_ht[k1][k], 2));

			m_c[k] = m_ht[k][k] / mu;
			m_s[k] = -m_ht[k1][k] / mu;
			m_ht[k][k] = m_c[k] * m_ht[k][k] - m_s[k] * m_ht[k1][k];
			m_ht[k1][k] = 0.0;

			givens_mult(m_c[k], m_s[k], &m_g[k], &m_g[k1]);

			rho = std::abs(m_g[k1]);

			itr_used = itr_used + 1;

			if (rho <= rho_delta)
			{
				last_k = k;
				break;
			}
		}

		if (last_k >= mr)
			/* didn't converge within accuracy */
			last_k = mr - 1;

		double m_y[_storage_N + 1];

		/* Solve the system H * y = g */
		/* x += m_v[j] * m_y[j]       */
		for (int i = last_k; i >= 0; i--)
		{
			double tmp = m_g[i];
			for (unsigned j = i + 1; j <= last_k; j++)
			{
				tmp -= m_ht[i][j] * m_y[j];
			}
			m_y[i] = tmp / m_ht[i][i];
		}

		for (unsigned i = 0; i <= last_k; i++)
			vec_add_mult_scalar(n, m_v[i], m_y[i], x);

#if 1
		if (rho <= rho_delta)
		{
			break;
		}
#else
		/* we try to approximate the x difference between to steps using m_v[last_k] */

		double xdelta = m_y[last_k] * vec_maxabs(n, m_v[last_k]);
		if (xdelta < accuracy)
		{
			if (m_accuracy_mult < 16384.0)
				m_accuracy_mult = m_accuracy_mult * 2.0;
			break;
		}
		else
			m_accuracy_mult = m_accuracy_mult / 2.0;

#endif
	}

	return itr_used;
}



NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_GMRES_H_ */
