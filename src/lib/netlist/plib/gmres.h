// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PLIB_GMRES_H_
#define PLIB_GMRES_H_

///
/// \file gmres.h
///

#include "parray.h"
#include "pconfig.h"
#include "pmatrix_cr.h"
#include "vector_ops.h"

#include <algorithm>

namespace plib
{

	template <int k>
	struct do_khelper
	{
		static constexpr bool value = true;
	};

	template <>
	struct do_khelper<-1>
	{
		static constexpr float value = 0.0;
	};

	template <typename FT, int SIZE>
	struct mat_precondition_ILU
	{
		using mat_type = plib::pmatrix_cr<FT, SIZE>;
		using matLU_type = plib::pLUmatrix_cr<mat_type>;

		mat_precondition_ILU(std::size_t size, std::size_t ilu_scale = 4
			, std::size_t bw = plib::pmatrix_cr<FT, SIZE>::FILL_INFINITY)
		: m_mat(narrow_cast<typename mat_type::index_type>(size))
		, m_LU(narrow_cast<typename mat_type::index_type>(size))
		, m_ILU_scale(narrow_cast<std::size_t>(ilu_scale))
		, m_band_width(bw)
		{
		}

		template <typename M>
		void build(M &fill)
		{
			m_mat.build_from_fill_mat(fill, 0);
			m_LU.build(fill, m_ILU_scale);
		}


		template<typename R, typename V>
		void calc_rhs(R &rhs, const V &v)
		{
			m_mat.mult_vec(rhs, v);
		}

		void precondition()
		{
			m_LU.incomplete_LU_factorization(m_mat);
		}

		template<typename V>
		void solve_inplace(V &v)
		{
			m_LU.solveLU(v);
		}

		PALIGNAS_VECTOROPT()
		mat_type                m_mat;
		PALIGNAS_VECTOROPT()
		matLU_type              m_LU;
		std::size_t             m_ILU_scale;
		std::size_t             m_band_width;
	};

	template <typename FT, int SIZE>
	struct mat_precondition_diag
	{
		mat_precondition_diag(std::size_t size, [[maybe_unused]] int dummy = 0)
		: m_mat(size)
		, m_diag(size)
		, nzcol(size)
		{
		}

		template <typename M>
		void build(M &fill)
		{
			m_mat.build_from_fill_mat(fill, 0);
			for (std::size_t i = 0; i< m_diag.size(); i++)
			{
				for (std::size_t j = 0; j< m_diag.size(); j++)
				{
					std::size_t k=m_mat.row_idx[j];
					while (m_mat.col_idx[k] < i && k < m_mat.row_idx[j+1])
						k++;
					if (m_mat.col_idx[k] == i && k < m_mat.row_idx[j+1])
						nzcol[i].push_back(k);
				}
				nzcol[i].push_back(narrow_cast<std::size_t>(-1));
			}
		}

		template<typename R, typename V>
		void calc_rhs(R &rhs, const V &v)
		{
			m_mat.mult_vec(rhs, v);
		}

		void precondition()
		{
			for (std::size_t i = 0; i< m_diag.size(); i++)
			{
				// ILUT: 265%
				FT v(0.0);
#if 0
				// doesn't works, Mame performance drops significantly%
				// 136%
				for (std::size_t j = m_mat.row_idx[i]; j< m_mat.row_idx[i+1]; j++)
					v += m_mat.A[j] * m_mat.A[j];
				m_diag[i] = reciprocal(std::sqrt(v));
#elif 0
				// works halfway, i.e. Mame performance 50%
				// 147% - lowest average solution time with 7.094
				for (std::size_t j = m_mat.row_idx[i]; j< m_mat.row_idx[i+1]; j++)
					v += m_mat.A[j] * m_mat.A[j];
				m_diag[i] = m_mat.A[m_mat.diag[i]] / v;
#elif 0
				// works halfway, i.e. Mame performance 50%
				// sum over column i
				// 344% - lowest average solution time with 3.06
				std::size_t nzcolp = 0;
				const auto &nz = nzcol[i];
				std::size_t j;

				while ((j = nz[nzcolp++])!=narrow_cast<std::size_t>(-1)) // NOLINT(bugprone-infinite-loop)
				{
					v += m_mat.A[j] * m_mat.A[j];
				}
				m_diag[i] = m_mat.A[m_mat.diag[i]] / v;
#elif 0
				// works halfway, i.e. Mame performance 50%
				// 151%
				for (std::size_t j = m_mat.row_idx[i]; j< m_mat.row_idx[i+1]; j++)
					v += plib::abs(m_mat.A[j]);
				m_diag[i] =  reciprocal(v);
#else
				// 124%
				for (std::size_t j = m_mat.row_idx[i]; j< m_mat.row_idx[i+1]; j++)
					v = std::max(v, plib::abs(m_mat.A[j]));
				m_diag[i] = reciprocal(v);
#endif
				//m_diag[i] = reciprocal(m_mat.A[m_mat.diag[i]]);
			}
		}

		template<typename V>
		void solve_inplace(V &v)
		{
			for (std::size_t i = 0; i< m_diag.size(); i++)
				v[i] = v[i] * m_diag[i];
		}

		plib::pmatrix_cr<FT, SIZE> m_mat;
		plib::parray<FT, SIZE> m_diag;
		plib::parray<std::vector<std::size_t>, SIZE > nzcol;
	};

	template <typename FT, int SIZE>
	struct mat_precondition_none
	{
		mat_precondition_none(std::size_t size, [[maybe_unused]] int dummy = 0)
		: m_mat(size)
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
		}

		template<typename V>
		void solve_inplace([[maybe_unused]] V &v)
		{
		}

		plib::pmatrix_cr<FT, SIZE> m_mat;
	};

	// FIXME: hardcoding RESTART to 20 becomes an issue on very large
	// systems.

	template <typename FT, int SIZE, int RESTARTMAX = 16>
	struct gmres_t
	{
	public:

		using float_type = FT;

		//constexpr static int RESTART = RESTARTMAX;
		constexpr static const int RESTART = (SIZE > 0) ? ((SIZE < RESTARTMAX) ? SIZE : RESTARTMAX)
			: ((SIZE < 0) ? ((-SIZE < RESTARTMAX) ? -SIZE : RESTARTMAX) : RESTARTMAX);

		explicit gmres_t(std::size_t size)
			: residual(size)
			, Ax(size)
			, m_ht(RESTART +1, RESTART)
			, m_v(RESTART + 1, size)
			, m_size(size)
			, m_use_more_precise_stop_condition(false)
			{
			}

		std::size_t size() const { return (SIZE<=0) ? m_size : narrow_cast<std::size_t>(SIZE); }

		template <typename OPS, typename VT, typename VRHS>
		std::size_t solve(OPS &ops, VT &x, const VRHS & rhs, const std::size_t itr_max, float_type accuracy)
		{
			// -------------------------------------------------------------------------
			// The code below was inspired by code published by John Burkardt under
			// the LPGL here:
			//
			// http://people.sc.fsu.edu/~jburkardt/cpp_src/mgmres/mgmres.html
			//
			// The code below was completely written from scratch based on the pseudo code
			// found here:
			//
			// http://de.wikipedia.org/wiki/GMRES-Verfahren
			//
			// The Algorithm itself is described in
			//
			// Yousef Saad,
			// Iterative Methods for Sparse Linear Systems,
			// Second Edition,
			// SIAM, 20003,
			// ISBN: 0898715342,
			// LC: QA188.S17.
			//
			//------------------------------------------------------------------------

			std::size_t itr_used = 0;
			float_type rho_delta(plib::constants<float_type>::zero());

			const    std::size_t n = size();

			ops.precondition();

			if (m_use_more_precise_stop_condition)
			{
				// derive residual for a given delta x
				//
				// LU y = A dx
				//
				// ==> rho / accuracy = sqrt(y * y)
				//
				// This approach will approximate the iterative stop condition
				// based |xnew - xold| pretty precisely. But it is slow, or expressed
				// differently: The invest doesn't pay off.
				//

				vec_set_scalar(residual, accuracy);
				ops.calc_rhs(Ax, residual);

				ops.solve_inplace(Ax);

				const float_type rho_to_accuracy = plib::sqrt(vec_mult2<FT>(Ax)) / accuracy;

				rho_delta = accuracy * rho_to_accuracy;
			}
			else
				//rho_delta = accuracy * plib::sqrt(vec_mult2<FT>(n, rhs))
				//      + 1e-4 * std::sqrt(n);
				rho_delta = accuracy * plib::sqrt(narrow_cast<FT>(n));

			//
			// LU x = b; solve for x;
			//
			// Using
			//
			// vec_set(n, x, rhs);
			// ops.solve_inplace(x);
			//
			// to get a starting point for x degrades convergence speed compared
			// to using the last solution for x.

			while (itr_used < itr_max)
			{
				float_type rho;

				ops.calc_rhs(Ax, x);

				vec_sub(residual, rhs, Ax);

				ops.solve_inplace(residual);

				rho = plib::sqrt(vec_mult2<FT>(residual));

				if (rho < rho_delta)
					return itr_used + 1;

				// FIXME: The "+" is necessary to avoid link issues
				// on some systems / compiler versions. Issue reported by
				// AJR, no details known yet.

				vec_set_scalar(m_g, +constants<FT>::zero());
				m_g[0] = rho;

				vec_mult_scalar(m_v[0], residual, plib::reciprocal(rho));

				if (do_k<RESTART-1>(ops, x, itr_used, rho_delta, true))
					// converged
					break;
			}
			return itr_used;
		}

	private:

		static void givens_mult(FT c, FT s, FT & g0, FT & g1 )
		{
			const FT g0_last(g0);

			g0 = c * g0 - s * g1;
			g1 = s * g0_last + c * g1;
		}

		template <int k, typename OPS, typename VT>
		bool do_k(OPS &ops, VT &x, std::size_t &itr_used, FT rho_delta, [[maybe_unused]] bool dummy)
		{
			if (do_k<k-1, OPS>(ops, x, itr_used, rho_delta, do_khelper<k-1>::value))
				return true;

			constexpr const std::size_t kp1 = k + 1;
			//const    std::size_t n = size();

			ops.calc_rhs(m_v[kp1], m_v[k]);
			ops.solve_inplace(m_v[kp1]);

			for (std::size_t j = 0; j <= k; j++)
			{
				m_ht[j][k] = vec_mult<FT>(m_v[kp1], m_v[j]);
				vec_add_mult_scalar(m_v[kp1], m_v[j], -m_ht[j][k]);
			}
			m_ht[kp1][k] = plib::sqrt(vec_mult2<FT>(m_v[kp1]));

			// FIXME: comparison to zero
			if (m_ht[kp1][k] != plib::constants<FT>::zero())
				vec_scale(m_v[kp1], reciprocal(m_ht[kp1][k]));

			for (std::size_t j = 0; j < k; j++)
				givens_mult(m_c[j], m_s[j], m_ht[j][k], m_ht[j+1][k]);

			const float_type mu = reciprocal(plib::hypot(m_ht[k][k], m_ht[kp1][k]));

			m_c[k] = m_ht[k][k] * mu;
			m_s[k] = -m_ht[kp1][k] * mu;
			m_ht[k][k] = m_c[k] * m_ht[k][k] - m_s[k] * m_ht[kp1][k];
			m_ht[kp1][k] = plib::constants<FT>::zero();

			givens_mult(m_c[k], m_s[k], m_g[k], m_g[kp1]);

			const float_type  rho = plib::abs(m_g[kp1]);

			// FIXME ..
			itr_used = itr_used + 1;

			if (rho <= rho_delta || k == RESTART-1)
			{
				// Solve the system H * y = g
				// x += m_v[j] * m_y[j]
				for (std::size_t i = k + 1; i-- > 0;)
				{
					auto tmp = m_g[i];
					const auto htii=plib::reciprocal(m_ht[i][i]);
					for (std::size_t j = i + 1; j <= k; j++)
						tmp -= m_ht[i][j] * m_y[j];
					m_y[i] = tmp * htii;
					vec_add_mult_scalar(x, m_v[i], m_y[i]);
				}

				//for (std::size_t i = 0; i <= k; i++)
				//  vec_add_mult_scalar(n, x, m_v[i], m_y[i]);
				return true;
			}
			return false;
		}

		template <int k, typename OPS, typename VT>
		bool do_k(OPS &ops, VT &x, std::size_t &itr_used, FT rho_delta, float dummy)
		{
			plib::unused_var(ops, x, itr_used, rho_delta, dummy);
			return false;
		}

		plib::parray<float_type, SIZE> residual;
		plib::parray<float_type, SIZE> Ax;

		plib::parray<float_type, RESTART + 1> m_c;              // mr + 1
		plib::parray<float_type, RESTART + 1> m_g;              // mr + 1
		plib::parray2D<float_type, RESTART + 1, RESTART> m_ht;  // (mr + 1), mr
		plib::parray<float_type, RESTART + 1> m_s;              // mr + 1
		plib::parray<float_type, RESTART + 1> m_y;              // mr + 1

		plib::parray2D<float_type, RESTART + 1, SIZE> m_v;  // mr + 1, n

		std::size_t m_size;

		bool m_use_more_precise_stop_condition;
	};


#if 0
	// Example of a Chebyshev iteration solver. This one doesn't work yet,
	// it needs to be extended for non-symmetric matrix operation and
	// depends on spectral radius estimates - which we don't have.
	//
	// Left here as another example.

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

		std::size_t size() const { return (SIZE<=0) ? m_size : narrow_cast<std::size_t>(SIZE); }

		template <typename OPS, typename VT, typename VRHS>
		std::size_t solve(OPS &ops, VT &x0, const VRHS & rhs, const std::size_t iter_max, float_type accuracy)
		{
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

			FT rho_delta = accuracy * std::sqrt(narrow_cast<FT>(size()));

			rho_delta = 1e-9;

			for (int i = 0; i < iter_max; i++)
			{
				ops.solve_inplace(residual);
				if (i==0)
				{
					vec_set(size(), p, residual);
					alpha = 2.0 / d;
				}
				else
				{
					  beta = alpha * ( c / 2.0)*( c / 2.0);
					  alpha = reciprocal(d - beta);
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

#endif // PLIB_GMRES_H_
