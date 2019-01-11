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
#include <cmath>

#include "mat_cr.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"
#include "vector_base.h"

namespace netlist
{
	namespace devices
	{
template <std::size_t m_N, std::size_t storage_N>
class matrix_solver_GMRES_t: public matrix_solver_direct_t<m_N, storage_N>
{
public:

	matrix_solver_GMRES_t(netlist_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_direct_t<m_N, storage_N>(anetlist, name, matrix_solver_t::ASCENDING, params, size)
		, m_use_iLU_preconditioning(true)
		, m_use_more_precise_stop_condition(false)
		, m_accuracy_mult(1.0)
		, mat(size)
		{
		}

	virtual ~matrix_solver_GMRES_t() override
	{
	}

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;

private:

	//typedef typename mat_cr_t<storage_N>::type mattype;
	typedef typename mat_cr_t<storage_N>::index_type mattype;

	unsigned solve_ilu_gmres(nl_double (& RESTRICT x)[storage_N], const nl_double (& RESTRICT rhs)[storage_N], const unsigned restart_max, std::size_t mr, nl_double accuracy);

	std::vector<unsigned> m_term_cr[storage_N];

	bool m_use_iLU_preconditioning;
	bool m_use_more_precise_stop_condition;
	nl_double m_accuracy_mult; // FXIME: Save state

	mat_cr_t<storage_N> mat;

	nl_double m_LU[storage_N * storage_N];

	nl_double m_c[storage_N + 1];  /* mr + 1 */
	nl_double m_g[storage_N + 1];  /* mr + 1 */
	nl_double m_ht[storage_N + 1][storage_N];  /* (mr + 1), mr */
	nl_double m_s[storage_N + 1];     /* mr + 1 */
	nl_double m_v[storage_N + 1][storage_N];      /*(mr + 1), n */
	nl_double m_y[storage_N + 1];       /* mr + 1 */

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GMRES
// ----------------------------------------------------------------------------------------

template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_GMRES_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<m_N, storage_N>::vsetup(nets);

	mattype nz = 0;
	const std::size_t iN = this->N();

	for (std::size_t k=0; k<iN; k++)
	{
		terms_for_net_t * RESTRICT row = this->m_terms[k].get();
		mat.ia[k] = nz;

		for (std::size_t j=0; j<row->m_nz.size(); j++)
		{
			mat.ja[nz] = static_cast<mattype>(row->m_nz[j]);
			if (row->m_nz[j] == k)
				mat.diag[k] = nz;
			nz++;
		}

		/* build pointers into the compressed row format matrix for each terminal */

		for (unsigned j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			for (unsigned i = mat.ia[k]; i<nz; i++)
				if (this->m_terms[k]->connected_net_idx()[j] == static_cast<int>(mat.ja[i]))
				{
					m_term_cr[k].push_back(i);
					break;
				}
			nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
		}
	}

	mat.ia[iN] = nz;
	mat.nz_num = nz;
}

template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_GMRES_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const std::size_t iN = this->N();

	/* ideally, we could get an estimate for the spectral radius of
	 * Inv(D - L) * U
	 *
	 * and estimate using
	 *
	 * omega = 2.0 / (1.0 + std::sqrt(1-rho))
	 */

	//nz_num = 0;
	nl_double RHS[storage_N];
	nl_double new_V[storage_N];

	mat.set_scalar(0.0);

	for (std::size_t k = 0; k < iN; k++)
	{
		nl_double gtot_t = 0.0;
		nl_double RHS_t = 0.0;

		const std::size_t term_count = this->m_terms[k]->count();
		const std::size_t railstart = this->m_terms[k]->m_railstart;
		const nl_double * const RESTRICT gt = this->m_terms[k]->gt();
		const nl_double * const RESTRICT go = this->m_terms[k]->go();
		const nl_double * const RESTRICT Idr = this->m_terms[k]->Idr();
		const nl_double * const * RESTRICT other_cur_analog = this->m_terms[k]->connected_net_V();

		for (std::size_t i = 0; i < term_count; i++)
		{
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}

		for (std::size_t i = railstart; i < term_count; i++)
			RHS_t = RHS_t  + go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;

		// add diagonal element
		mat.A[mat.diag[k]] = gtot_t;

		for (std::size_t i = 0; i < railstart; i++)
		{
			const std::size_t pi = m_term_cr[k][i];
			mat.A[pi] -= go[i];
		}

		new_V[k] = this->m_nets[k]->Q_Analog();

	}

	mat.ia[iN] = static_cast<mattype>(mat.nz_num);
	const nl_double accuracy = this->m_params.m_accuracy;

	const std::size_t mr = (iN > 3 ) ? static_cast<std::size_t>(std::sqrt(iN) * 2.0) : iN;
	unsigned iter = std::max(1u, this->m_params.m_gs_loops);
	unsigned gsl = solve_ilu_gmres(new_V, RHS, iter, mr, accuracy);
	const std::size_t failed = mr * iter;

	this->m_iterative_total += gsl;
	this->m_stat_calculations++;

	if (gsl >= failed)
	{
		this->m_iterative_fail++;
		return matrix_solver_direct_t<m_N, storage_N>::vsolve_non_dynamic(newton_raphson);
	}

	const nl_double err = (newton_raphson ? this->delta(new_V) : 0.0);
	this->store(new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

template <typename T>
inline static void givens_mult( const T c, const T s, T & g0, T & g1 )
{
	const T tg0 = c * g0 - s * g1;
	const T tg1 = s * g0 + c * g1;

	g0 = tg0;
	g1 = tg1;
}

template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_GMRES_t<m_N, storage_N>::solve_ilu_gmres (nl_double (& RESTRICT x)[storage_N], const nl_double (& RESTRICT rhs)[storage_N], const unsigned restart_max, std::size_t mr, nl_double accuracy)
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

	const    std::size_t n = this->N();

	if (mr > n) mr = n;

	if (m_use_iLU_preconditioning)
		mat.incomplete_LU_factorization(m_LU);

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
		nl_double t[storage_N];
		nl_double Ax[storage_N];
		vec_set(n, accuracy, t);
		mat.mult_vec(t, Ax);

		mat.solveLUx(m_LU, Ax);

		const nl_double rho_to_accuracy = std::sqrt(vec_mult2(n, Ax)) / accuracy;

		rho_delta = accuracy * rho_to_accuracy;
	}
	else
		rho_delta = accuracy * std::sqrt(n) * m_accuracy_mult;

	for (unsigned itr = 0; itr < restart_max; itr++)
	{
		std::size_t last_k = mr;
		nl_double rho;

		nl_double Ax[storage_N];
		nl_double residual[storage_N];

		mat.mult_vec(x, Ax);

		vec_sub(n, rhs, Ax, residual);

		if (m_use_iLU_preconditioning)
		{
			mat.solveLUx(m_LU, residual);
		}

		rho = std::sqrt(vec_mult2(n, residual));

		if (rho < rho_delta)
			return itr_used + 1;

		vec_set(mr+1, NL_FCONST(0.0), m_g);
		m_g[0] = rho;

		for (std::size_t i = 0; i < mr; i++)
			vec_set(mr + 1, NL_FCONST(0.0), m_ht[i]);

		vec_mult_scalar(n, residual, NL_FCONST(1.0) / rho, m_v[0]);

		for (std::size_t k = 0; k < mr; k++)
		{
			const std::size_t k1 = k + 1;

			mat.mult_vec(m_v[k], m_v[k1]);

			if (m_use_iLU_preconditioning)
				mat.solveLUx(m_LU, m_v[k1]);

			for (std::size_t j = 0; j <= k; j++)
			{
				m_ht[j][k] = vec_mult(n, m_v[k1], m_v[j]);
				vec_add_mult_scalar(n, m_v[j], -m_ht[j][k], m_v[k1]);
			}
			m_ht[k1][k] = std::sqrt(vec_mult2(n, m_v[k1]));

			if (m_ht[k1][k] != 0.0)
				vec_scale(n, m_v[k1], NL_FCONST(1.0) / m_ht[k1][k]);

			for (std::size_t j = 0; j < k; j++)
				givens_mult(m_c[j], m_s[j], m_ht[j][k], m_ht[j+1][k]);

			const nl_double mu = 1.0 / std::hypot(m_ht[k][k], m_ht[k1][k]);

			m_c[k] = m_ht[k][k] * mu;
			m_s[k] = -m_ht[k1][k] * mu;
			m_ht[k][k] = m_c[k] * m_ht[k][k] - m_s[k] * m_ht[k1][k];
			m_ht[k1][k] = 0.0;

			givens_mult(m_c[k], m_s[k], m_g[k], m_g[k1]);

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

		/* Solve the system H * y = g */
		/* x += m_v[j] * m_y[j]       */
		for (std::size_t i = last_k + 1; i-- > 0;)
		{
			double tmp = m_g[i];
			for (std::size_t j = i + 1; j <= last_k; j++)
			{
				tmp -= m_ht[i][j] * m_y[j];
			}
			m_y[i] = tmp / m_ht[i][i];
		}

		for (std::size_t i = 0; i <= last_k; i++)
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



	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GMRES_H_ */
