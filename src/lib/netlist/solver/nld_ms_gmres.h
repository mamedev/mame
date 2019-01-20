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

#include "../plib/parray.h"
#include "mat_cr.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"
#include "vector_base.h"

namespace netlist
{
	namespace devices
	{

template <typename FT, int SIZE>
class matrix_solver_GMRES_t: public matrix_solver_direct_t<FT, SIZE>
{
public:

	typedef FT float_type;
	// FIXME: dirty hack to make this compile
	static constexpr const std::size_t storage_N = plib::sizeabs<FT, SIZE>::ABS();

	// Maximum iterations before a restart ...
	static constexpr const std::size_t restart_N = (storage_N > 0 ? 20 : 0);

	/* Sort rows in ascending order. This should minimize fill-in and thus
	 * maximize the efficiency of the incomplete LUT.
	 */
	matrix_solver_GMRES_t(netlist_base_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_direct_t<FT, SIZE>(anetlist, name, matrix_solver_t::ASCENDING, params, size)
		, m_use_iLU_preconditioning(true)
		, m_use_more_precise_stop_condition(true)
		, m_ILU_scale(0)
		, m_accuracy_mult(1.0)
		, m_term_cr(size)
		, mat(size)
		, residual(size)
		, Ax(size)
		, m_LU(size)
		//, m_v(size)
		{
		}

	virtual ~matrix_solver_GMRES_t() override
	{
	}

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;

private:

	//typedef typename mat_cr_t<storage_N>::type mattype;
	typedef typename plib::mat_cr_t<FT, SIZE>::index_type mattype;

	template <typename VT, typename VRHS>
	std::size_t solve_ilu_gmres(VT &x, const VRHS & rhs, const std::size_t restart_max, std::size_t mr, float_type accuracy);


	bool m_use_iLU_preconditioning;
	bool m_use_more_precise_stop_condition;
	std::size_t m_ILU_scale;
	float_type m_accuracy_mult; // FXIME: Save state

	plib::parray<std::vector<FT *>, SIZE> m_term_cr;
	plib::mat_cr_t<float_type, SIZE> mat;
	plib::parray<float_type, SIZE> residual;
	plib::parray<float_type, SIZE> Ax;

	plib::mat_cr_t<float_type, SIZE> m_LU;

	float_type m_c[restart_N + 1];  			/* mr + 1 */
	float_type m_g[restart_N + 1];  			/* mr + 1 */
	float_type m_ht[restart_N + 1][restart_N];  /* (mr + 1), mr */
	float_type m_s[restart_N + 1];     			/* mr + 1 */
	float_type m_y[restart_N + 1];       		/* mr + 1 */

	//plib::parray<float_type, SIZE> m_v[restart_N + 1];  /* mr + 1, n */
	float_type m_v[restart_N + 1][storage_N];  /* mr + 1, n */

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GMRES
// ----------------------------------------------------------------------------------------

template <typename FT, int SIZE>
void matrix_solver_GMRES_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<FT, SIZE>::vsetup(nets);

	const std::size_t iN = this->N();

	std::vector<std::vector<unsigned>> fill(iN);

	for (std::size_t k=0; k<iN; k++)
	{
		fill[k].resize(iN, decltype(mat)::FILL_INFINITY);
		terms_for_net_t * RESTRICT row = this->m_terms[k].get();
		for (std::size_t j=0; j<row->m_nz.size(); j++)
		{
			fill[k][static_cast<mattype>(row->m_nz[j])] = 0;
		}
	}

	mat.build_from_fill_mat(fill, 0);
	m_LU.gaussian_extend_fill_mat(fill);
	m_LU.build_from_fill_mat(fill, m_ILU_scale); // ILU(2)
	//m_LU.build_from_fill_mat(fill, 9999, 20); // Band matrix width 20

	/* build pointers into the compressed row format matrix for each terminal */

	for (std::size_t k=0; k<iN; k++)
	{
		for (std::size_t j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			for (std::size_t i = mat.row_idx[k]; i<mat.row_idx[k+1]; i++)
				if (this->m_terms[k]->connected_net_idx()[j] == static_cast<int>(mat.col_idx[i]))
				{
					m_term_cr[k].push_back(&mat.A[i]);
					break;
				}
		}
		nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
		m_term_cr[k].push_back(&mat.A[mat.diag[k]]);
	}
}

template <typename FT, int SIZE>
unsigned matrix_solver_GMRES_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
{
	const std::size_t iN = this->N();

	plib::parray<FT, SIZE> RHS(iN);
	//float_type new_V[storage_N];

	mat.set_scalar(0.0);

	/* populate matrix and V for first estimate */
	for (std::size_t k = 0; k < iN; k++)
	{
		this->m_terms[k]->fill_matrix(m_term_cr[k], RHS[k]);
		this->m_new_V[k] = this->m_nets[k]->Q_Analog();
	}


	//mat.row_idx[iN] = static_cast<mattype>(mat.nz_num);
	const float_type accuracy = this->m_params.m_accuracy;

	const std::size_t mr = (iN > 3 ) ? static_cast<std::size_t>(std::sqrt(iN) * 2.0) : iN;
	std::size_t iter = std::max(1u, this->m_params.m_gs_loops);
	std::size_t gsl = solve_ilu_gmres(this->m_new_V, RHS, iter, mr, accuracy);
	const std::size_t failed = mr * iter;

	this->m_iterative_total += gsl;
	this->m_stat_calculations++;

	if (gsl >= failed)
	{
		this->m_iterative_fail++;
		return matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic(newton_raphson);
	}

	//if (newton_raphson)
	//	printf("%e %e\n", this->delta(this->m_new_V),  this->m_params.m_accuracy);

	const float_type err = (newton_raphson ? this->delta(this->m_new_V) : 0.0);
	this->store(this->m_new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

template <typename T>
inline void givens_mult( const T c, const T s, T & g0, T & g1 )
{
	const T g0_last(g0);

	g0 = c * g0 - s * g1;
	g1 = s * g0_last + c * g1;
}

template <typename FT, int SIZE>
template <typename VT, typename VRHS>
std::size_t matrix_solver_GMRES_t<FT, SIZE>::solve_ilu_gmres (VT &x, const VRHS &rhs, const std::size_t restart_max, std::size_t mr, float_type accuracy)
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

	std::size_t itr_used = 0;
	double rho_delta = 0.0;

	const    std::size_t n = this->N();

	if (mr > restart_N) mr = restart_N;
	if (mr > n) mr = n;

	if (m_use_iLU_preconditioning)
	{
		if (m_ILU_scale < 1)
			m_LU.raw_copy_from(mat);
		else
			m_LU.reduction_copy_from(mat);
		m_LU.incomplete_LU_factorization();
	}

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
		vec_set_scalar(n, residual, accuracy);
		mat.mult_vec(residual, Ax);

		m_LU.solveLUx(Ax);

		const float_type rho_to_accuracy = std::sqrt(vec_mult2<FT>(n, Ax)) / accuracy;

		rho_delta = accuracy * rho_to_accuracy;
	}
	else
		rho_delta = accuracy * std::sqrt(static_cast<FT>(n)) * m_accuracy_mult;

	for (std::size_t itr = 0; itr < restart_max; itr++)
	{
		std::size_t last_k = mr;
		float_type rho;

		mat.mult_vec(x, Ax);

		vec_sub(n, rhs, Ax, residual);

		if (m_use_iLU_preconditioning)
		{
			m_LU.solveLUx(residual);
		}

		rho = std::sqrt(vec_mult2<FT>(n, residual));

		if (rho < rho_delta)
			return itr_used + 1;

		vec_set_scalar(mr+1, m_g, NL_FCONST(0.0));
		m_g[0] = rho;

		for (std::size_t i = 0; i < mr + 1; i++)
			vec_set_scalar(mr, m_ht[i], NL_FCONST(0.0));

		vec_mult_scalar(n, residual, NL_FCONST(1.0) / rho, m_v[0]);

		for (std::size_t k = 0; k < mr; k++)
		{
			const std::size_t k1 = k + 1;

			mat.mult_vec(m_v[k], m_v[k1]);

			if (m_use_iLU_preconditioning)
				m_LU.solveLUx(m_v[k1]);

			for (std::size_t j = 0; j <= k; j++)
			{
				m_ht[j][k] = vec_mult<float_type>(n, m_v[k1], m_v[j]);
				vec_add_mult_scalar(n, m_v[j], -m_ht[j][k], m_v[k1]);
			}
			m_ht[k1][k] = std::sqrt(vec_mult2<FT>(n, m_v[k1]));

			if (m_ht[k1][k] != 0.0)
				vec_scale(n, m_v[k1], NL_FCONST(1.0) / m_ht[k1][k]);

			for (std::size_t j = 0; j < k; j++)
				givens_mult(m_c[j], m_s[j], m_ht[j][k], m_ht[j+1][k]);

			const float_type mu = 1.0 / std::hypot(m_ht[k][k], m_ht[k1][k]);

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
				tmp -= m_ht[i][j] * m_y[j];
			m_y[i] = tmp / m_ht[i][i];
		}

		for (std::size_t i = 0; i <= last_k; i++)
			vec_add_mult_scalar(n, m_v[i], m_y[i], x);

		if (rho <= rho_delta)
			break;

	}
	return itr_used;
}



	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GMRES_H_ */
