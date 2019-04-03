// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * gmres.h
 *
 */

#ifndef PLIB_GMRES_H_
#define PLIB_GMRES_H_

#include "mat_cr.h"
#include "parray.h"
#include "pconfig.h"
#include "vector_ops.h"

#include <algorithm>
#include <cmath>


namespace plib
{

	template <typename FT, int SIZE>
	struct mat_precondition_ILU
	{
		using mat_type = plib::matrix_compressed_rows_t<FT, SIZE>;

		mat_precondition_ILU(std::size_t size, int ilu_scale = 4
			, std::size_t bw = plib::matrix_compressed_rows_t<FT, SIZE>::FILL_INFINITY)
		: m_mat(static_cast<typename mat_type::index_type>(size))
		, m_LU(static_cast<typename mat_type::index_type>(size))
		, m_use_iLU_preconditioning(ilu_scale >= 0)
		, m_ILU_scale(static_cast<std::size_t>(ilu_scale))
		, m_band_width(bw)
		{
		}

		template <typename M>
		void build(M &fill)
		{
			m_mat.build_from_fill_mat(fill, 0);
			if (m_use_iLU_preconditioning)
			{
				m_LU.gaussian_extend_fill_mat(fill);
				m_LU.build_from_fill_mat(fill, m_ILU_scale, m_band_width); // ILU(2)
				//m_LU.build_from_fill_mat(fill, 9999, 20); // Band matrix width 20
			}
		}


		template<typename R, typename V>
		void calc_rhs(R &rhs, const V &v)
		{
			m_mat.mult_vec(rhs, v);
		}

		void precondition()
		{
			if (m_use_iLU_preconditioning)
			{
				if (m_ILU_scale < 1)
					m_LU.raw_copy_from(m_mat);
				else
					m_LU.reduction_copy_from(m_mat);
				m_LU.incomplete_LU_factorization();
			}
		}

		template<typename V>
		void solve_LU_inplace(V &v)
		{
			if (m_use_iLU_preconditioning)
			{
				m_LU.solveLUx(v);
			}
		}

		PALIGNAS_VECTOROPT()
		mat_type                m_mat;
		PALIGNAS_VECTOROPT()
		mat_type                m_LU;
		bool                    m_use_iLU_preconditioning;
		std::size_t             m_ILU_scale;
		std::size_t             m_band_width;
	};

	template <typename FT, int SIZE>
	struct mat_precondition_diag
	{
		mat_precondition_diag(std::size_t size)
		: m_mat(size)
		, m_diag(size)
		, m_use_iLU_preconditioning(true)
		{
		}

		template <typename M>
		void build(M &fill)
		{
			m_mat.build_from_fill_mat(fill, 0);
		}

		template<typename R, typename V>
		void calc_rhs(R &rhs, const V &v)
		{
			m_mat.mult_vec(rhs, v);
		}

		void precondition()
		{
			if (m_use_iLU_preconditioning)
			{
				for (std::size_t i = 0; i< m_diag.size(); i++)
				{
					m_diag[i] = 1.0 / m_mat.A[m_mat.diag[i]];
				}
			}
		}

		template<typename V>
		void solve_LU_inplace(V &v)
		{
			if (m_use_iLU_preconditioning)
			{
				for (std::size_t i = 0; i< m_diag.size(); i++)
					v[i] = v[i] * m_diag[i];
			}
		}

		plib::matrix_compressed_rows_t<FT, SIZE> m_mat;
		plib::parray<FT, SIZE> m_diag;
		bool m_use_iLU_preconditioning;
	};

	/* FIXME: hardcoding RESTART to 20 becomes an issue on very large
	 * systems.
	 */
	template <typename FT, int SIZE, int RESTART = 20>
	struct gmres_t
	{
	public:

		using float_type = FT;
		// FIXME: dirty hack to make this compile
		static constexpr const std::size_t storage_N = plib::sizeabs<FT, SIZE>::ABS();

		gmres_t(std::size_t size)
			: residual(size)
			, Ax(size)
			, m_size(size)
			, m_use_more_precise_stop_condition(false)
			{
			}

		void givens_mult( const FT c, const FT s, FT & g0, FT & g1 )
		{
			const FT g0_last(g0);

			g0 = c * g0 - s * g1;
			g1 = s * g0_last + c * g1;
		}

		std::size_t size() const { return (SIZE<=0) ? m_size : static_cast<std::size_t>(SIZE); }

		template <typename OPS, typename VT, typename VRHS>
		std::size_t solve(OPS &ops, VT &x, const VRHS & rhs, const std::size_t itr_max, float_type accuracy)
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

			const    std::size_t n = size();

			ops.precondition();

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
				 */

				vec_set_scalar(n, residual, accuracy);
				ops.calc_rhs(Ax, residual);

				ops.solve_LU_inplace(Ax);

				const float_type rho_to_accuracy = std::sqrt(vec_mult2<FT>(n, Ax)) / accuracy;

				rho_delta = accuracy * rho_to_accuracy;
			}
			else
				rho_delta = accuracy * std::sqrt(static_cast<FT>(n));

			/*
			 * Using
			 *
			 * vec_set(n, x, rhs);
			 * ops.solve_LU_inplace(x);
			 *
			 * to get a starting point for x degrades convergence speed compared
			 * to using the last solution for x.
			 *
			 * LU x = b; solve for x;
			 *
			 */

			while (itr_used < itr_max)
			{
				std::size_t last_k = RESTART;
				float_type rho;

				ops.calc_rhs(Ax, x);

				vec_sub(n, residual, rhs, Ax);

				ops.solve_LU_inplace(residual);

				rho = std::sqrt(vec_mult2<FT>(n, residual));

				if (rho < rho_delta)
					return itr_used + 1;

				/* FIXME: The "+" is necessary to avoid link issues
				 * on some systems / compiler versions. Issue reported by
				 * AJR, no details known yet.
				 */
				vec_set_scalar(RESTART+1, m_g, +constants<FT>::zero());
				m_g[0] = rho;

				//for (std::size_t i = 0; i < mr + 1; i++)
				//  vec_set_scalar(mr, m_ht[i], NL_FCONST(0.0));

				vec_mult_scalar(n, m_v[0], residual, constants<FT>::one() / rho);

				for (std::size_t k = 0; k < RESTART; k++)
				{
					const std::size_t kp1 = k + 1;

					ops.calc_rhs(m_v[kp1], m_v[k]);
					ops.solve_LU_inplace(m_v[kp1]);

					for (std::size_t j = 0; j <= k; j++)
					{
						m_ht[j][k] = vec_mult<FT>(n, m_v[kp1], m_v[j]);
						vec_add_mult_scalar(n, m_v[kp1], m_v[j], -m_ht[j][k]);
					}
					m_ht[kp1][k] = std::sqrt(vec_mult2<FT>(n, m_v[kp1]));

					if (m_ht[kp1][k] != 0.0)
						vec_scale(n, m_v[kp1], constants<FT>::one() / m_ht[kp1][k]);

					for (std::size_t j = 0; j < k; j++)
						givens_mult(m_c[j], m_s[j], m_ht[j][k], m_ht[j+1][k]);

					const float_type mu = 1.0 / std::hypot(m_ht[k][k], m_ht[kp1][k]);

					m_c[k] = m_ht[k][k] * mu;
					m_s[k] = -m_ht[kp1][k] * mu;
					m_ht[k][k] = m_c[k] * m_ht[k][k] - m_s[k] * m_ht[kp1][k];
					m_ht[kp1][k] = 0.0;

					givens_mult(m_c[k], m_s[k], m_g[k], m_g[kp1]);

					rho = std::abs(m_g[kp1]);

					itr_used = itr_used + 1;

					if (rho <= rho_delta)
					{
						last_k = k;
						break;
					}
				}

				if (last_k >= RESTART)
					/* didn't converge within accuracy */
					last_k = RESTART - 1;

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
					vec_add_mult_scalar(n, x, m_v[i], m_y[i]);

				if (rho <= rho_delta)
					break;

			}
			return itr_used;
		}

	private:

		//typedef typename plib::mat_cr_t<FT, SIZE>::index_type mattype;

		plib::parray<float_type, SIZE> residual;
		plib::parray<float_type, SIZE> Ax;

		plib::parray<float_type, RESTART + 1> m_c;              /* mr + 1 */
		plib::parray<float_type, RESTART + 1> m_g;              /* mr + 1 */
		plib::parray<plib::parray<float_type, RESTART>, RESTART + 1> m_ht;  /* (mr + 1), mr */
		plib::parray<float_type, RESTART + 1> m_s;              /* mr + 1 */
		plib::parray<float_type, RESTART + 1> m_y;              /* mr + 1 */

		//plib::parray<float_type, SIZE> m_v[RESTART + 1];  /* mr + 1, n */
		plib::parray<plib::parray<float_type, storage_N>, RESTART + 1> m_v;  /* mr + 1, n */

		std::size_t m_size;

		bool m_use_more_precise_stop_condition;


	};


#if 0
	/* Example of a Chebyshev iteration solver. This one doesn't work yet,
	 * it needs to be extended for non-symmetric matrix operation and
	 * depends on spectral radius estimates - which we don't have.
	 *
	 * Left here as another example.
	 */

	template <typename FT, int SIZE>
	struct ch_t
	{
	public:

		typedef FT float_type;
		// FIXME: dirty hack to make this compile
		static constexpr const std::size_t storage_N = plib::sizeabs<FT, SIZE>::ABS();

		// Maximum iterations before a restart ...
		static constexpr const std::size_t restart_N = (storage_N > 0 ? 20 : 0);

		ch_t(std::size_t size)
		: residual(size)
		, Ax(size)
		, m_size(size)
		{
		}

		std::size_t size() const { return (SIZE<=0) ? m_size : static_cast<std::size_t>(SIZE); }

		template <typename OPS, typename VT, typename VRHS>
		std::size_t solve(OPS &ops, VT &x0, const VRHS & rhs, const std::size_t iter_max, float_type accuracy)
		{
			/*-------------------------------------------------------------------------
			 *
			 *
			 *------------------------------------------------------------------------*/

			ops.precondition();

			const FT lmax = 20.0;
			const FT lmin = 0.0001;

			const FT d = (lmax+lmin)/2.0;
			const FT c = (lmax-lmin)/2.0;
			FT alpha = 0;
			FT beta = 0;
			std::size_t itr_used = 0;

			plib::parray<FT, SIZE> x(size());
			plib::parray<FT, SIZE> p(size());

			plib::vec_set(size(), x, x0);

			ops.calc_rhs(Ax, x);
			vec_sub(size(), rhs, Ax, residual);

			FT rho_delta = accuracy * std::sqrt(static_cast<FT>(size()));

			rho_delta = 1e-9;

			for (int i = 0; i < iter_max; i++)
			{
				ops.solve_LU_inplace(residual);
				if (i==0)
				{
					vec_set(size(), p, residual);
					alpha = 2.0 / d;
				}
				else
				{
					  beta = alpha * ( c / 2.0)*( c / 2.0);
					  alpha = 1.0 / (d - beta);
					  for (std::size_t k = 0; k < size(); k++)
						  p[k] = residual[k] + beta * p[k];
				}
				plib::vec_add_mult_scalar(size(), p, alpha, x);
				ops.calc_rhs(Ax, x);
				plib::vec_sub(size(), rhs, Ax, residual);
				FT rho = std::sqrt(plib::vec_mult2<FT>(size(), residual));
				if (rho < rho_delta)
					break;
				itr_used++;
			}
			return itr_used;
		}
	private:

		//typedef typename plib::mat_cr_t<FT, SIZE>::index_type mattype;

		plib::parray<float_type, SIZE> residual;
		plib::parray<float_type, SIZE> Ax;

		std::size_t m_size;

	};
#endif

} // namespace plib

#endif /* PLIB_GMRES_H_ */
