// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PMATRIX_CR_H_
#define PMATRIX_CR_H_

///
/// \file pmatrix_cr.h
///
/// Compressed row format matrices
///

#include "palloc.h"
#include "parray.h"
#include "pconfig.h"
#include "pmath.h"
#include "pmatrix2d.h"
#include "pomp.h"
#include "ptypes.h"

#include <algorithm>
#include <array>
#include <type_traits>
#include <vector>

namespace plib
{

	template<typename ARENA, typename T, int N, typename C = uint16_t>
	struct pmatrix_cr
	{
		using index_type = C;
		using value_type = T;

		static constexpr const int NSQ = (N < 0 ? -N * N : N * N);
		static constexpr const int Np1 = (N == 0) ? 0 : (N < 0 ? N - 1 : N + 1);

		pmatrix_cr(const pmatrix_cr &) = default;
		pmatrix_cr &operator=(const pmatrix_cr &) = default;
		pmatrix_cr(pmatrix_cr &&) noexcept(std::is_nothrow_move_constructible<parray<value_type, NSQ>>::value) = default;
		pmatrix_cr &operator=(pmatrix_cr &&) noexcept(std::is_nothrow_move_assignable<parray<value_type, NSQ>>::value && std::is_nothrow_move_assignable<pmatrix2d_vrl<ARENA, index_type>>::value) = default;

		enum constants_e
		{
			FILL_INFINITY = 9999999
		};

		// FIXME: these should be private
		// NOLINTNEXTLINE
		parray<index_type, N> diag;      // diagonal index pointer n
		// NOLINTNEXTLINE
		parray<index_type, Np1> row_idx;      // row index pointer n + 1
		// NOLINTNEXTLINE
		parray<index_type, NSQ> col_idx;       // column index array nz_num, initially (n * n)
		// NOLINTNEXTLINE
		parray<value_type, NSQ> A;    // Matrix elements nz_num, initially (n * n)
		// NOLINTNEXTLINE
		std::size_t nz_num;

		explicit pmatrix_cr(ARENA &arena, std::size_t n)
		: diag(n)
		, row_idx(n+1)
		, col_idx(n*n)
		, A(n*n)
		, nz_num(0)
		//, nzbd(n * (n+1) / 2)
		, m_nzbd(arena, n, n)
		, m_size(n)
		{
			for (std::size_t i=0; i<n+1; i++)
			{
				row_idx[i] = 0;
			}
		}

		~pmatrix_cr() = default;

		constexpr std::size_t size() const noexcept { return (N>0) ? narrow_cast<std::size_t>(N) : m_size; }

		void clear() noexcept
		{
			nz_num = 0;
			for (std::size_t i=0; i < size() + 1; i++)
				row_idx[i] = 0;
		}

		void set_scalar(T scalar) noexcept
		{
			for (std::size_t i=0, e=nz_num; i<e; i++)
				A[i] = scalar;
		}

		void set_row_scalar(C r, T val) noexcept
		{
			C ri = row_idx[r];
			while (ri < row_idx[r+1])
				A[ri++] = val;
		}

		void set(C r, C c, T val) noexcept
		{
			C ri = row_idx[r];
			while (ri < row_idx[r+1] && col_idx[ri] < c)
			  ri++;
			// we have the position now;
			if (ri < row_idx[r+1] && col_idx[ri] == c)
				A[ri] = val;
			else
			{
				for (C i = nz_num; i>ri; i--)
				{
					A[i] = A[i-1];
					col_idx[i] = col_idx[i-1];
				}
				A[ri] = val;
				col_idx[ri] = c;
				for (C i = r + 1; i < size() + 1; i++)
					row_idx[i]++;
				nz_num++;
				if (c==r)
					diag[r] = ri;
			}
		}

		template <typename M>
		void build_from_fill_mat(const M &f, std::size_t max_fill = FILL_INFINITY - 1,
			std::size_t band_width = FILL_INFINITY) noexcept(false)
		{
			C nz = 0;
			if (nz_num != 0)
				throw pexception("build_from_mat only allowed on empty CR matrix");
			for (std::size_t k=0; k < size(); k++)
			{
				row_idx[k] = nz;

				for (std::size_t j=0; j < size(); j++)
					if (f[k][j] <= max_fill && plib::abs(narrow_cast<int>(k)-narrow_cast<int>(j)) <= narrow_cast<int>(band_width))
					{
						col_idx[nz] = narrow_cast<C>(j);
						if (j == k)
							diag[k] = nz;
						nz++;
					}
			}

			row_idx[size()] = nz;
			nz_num = nz;

			// build nzbd

			for (std::size_t k=0; k < size(); k++)
			{
				for (std::size_t j=k + 1; j < size(); j++)
					if (f[j][k] < FILL_INFINITY)
						m_nzbd.set(k, m_nzbd.colcount(k), narrow_cast<C>(j));
				m_nzbd.set(k, m_nzbd.colcount(k), 0); // end of sequence
			}

		}

		template <typename VTV, typename VTR>
		void mult_vec(VTR & res, const VTV & x) const noexcept
		{

			// res = A * x
			// this is a bit faster than the version above
			std::size_t row = 0;
			std::size_t k = 0;
			const std::size_t oe = nz_num;
			while (k < oe)
			{
				T tmp = plib::constants<T>::zero();
				const std::size_t e = row_idx[row+1];
				for (; k < e; k++)
					tmp += A[k] * x[col_idx[k]];
				res[row++] = tmp;
			}
		}

		// throws error if P(source)>P(destination)
		template <typename LUMAT>
		void slim_copy_from(LUMAT & src) noexcept(false)
		{
			for (std::size_t r=0; r<src.size(); r++)
			{
				C dp = row_idx[r];
				for (C sp = src.row_idx[r]; sp < src.row_idx[r+1]; sp++)
				{
					// advance dp to source column and fill 0s if necessary
					while (col_idx[dp] < src.col_idx[sp])
						A[dp++] = 0;
					if (row_idx[r+1] <= dp || col_idx[dp] != src.col_idx[sp])
						throw pexception("slim_copy_from error");
					A[dp++] = src.A[sp];
				}
				// fill remaining elements in row
				while (dp < row_idx[r+1])
					A[dp++] = 0;
			}
		}

		// only copies common elements
		template <typename LUMAT>
		void reduction_copy_from(LUMAT & src) noexcept
		{
			C sp(0);
			for (std::size_t r=0; r<src.size(); r++)
			{
				C dp(row_idx[r]);
				while(sp < src.row_idx[r+1])
				{
					// advance dp to source column and fill 0s if necessary
					if (col_idx[dp] < src.col_idx[sp])
						A[dp++] = 0;
					else if (col_idx[dp] == src.col_idx[sp])
						A[dp++] = src.A[sp++];
					else
						sp++;
				}
				// fill remaining elements in row
				while (dp < row_idx[r+1])
					A[dp++] = 0;
			}
		}

		// no checks at all - may crash
		template <typename LUMAT>
		void raw_copy_from(LUMAT & src) noexcept
		{
			for (std::size_t k = 0; k < nz_num; k++)
				A[k] = src.A[k];
		}

		index_type * nzbd(std::size_t row) { return m_nzbd[row]; }
		std::size_t nzbd_count(std::size_t row) { return m_nzbd.colcount(row) - 1; }
	protected:
		// FIXME: this should be private
		// NOLINTNEXTLINE
		//parray<std::vector<index_type>, N > m_nzbd;    // Support for gaussian elimination
		pmatrix2d_vrl<ARENA, index_type> m_nzbd;    // Support for gaussian elimination
	private:
		//parray<C, N < 0 ? -N * (N-1) / 2 : N * (N+1) / 2 > nzbd;    // Support for gaussian elimination
		std::size_t m_size;
	};

	template<typename B>
	struct pGEmatrix_cr : public B
	{
		using base_type = B;
		using index_type = typename base_type::index_type;

		pGEmatrix_cr(const pGEmatrix_cr &) = default;
		pGEmatrix_cr &operator=(const pGEmatrix_cr &) = default;
		pGEmatrix_cr(pGEmatrix_cr &&) noexcept(std::is_nothrow_move_constructible<base_type>::value) = default;
		pGEmatrix_cr &operator=(pGEmatrix_cr &&) noexcept(std::is_nothrow_move_assignable<base_type>::value) = default;

		template<typename ARENA>
		explicit pGEmatrix_cr(ARENA &arena, std::size_t n)
		: B(arena, n)
		{
		}

		~pGEmatrix_cr() = default;

		template <typename M>
		std::pair<std::size_t, std::size_t> gaussian_extend_fill_mat(M &fill)
		{
			std::size_t ops = 0;
			std::size_t fill_max = 0;

			for (std::size_t k = 0; k < fill.size(); k++)
			{
				ops++; // 1/A(k,k)
				for (std::size_t row = k + 1; row < fill.size(); row++)
				{
					if (fill[row][k] < base_type::FILL_INFINITY)
					{
						ops++;
						for (std::size_t col = k + 1; col < fill[row].size(); col++)
							//if (fill[k][col] < FILL_INFINITY)
							{
								auto f = std::min(fill[row][col], 1 + fill[row][k] + fill[k][col]);
								if (f < base_type::FILL_INFINITY)
								{
									if (f > fill_max)
										fill_max = f;
									ops += 2;
								}
								fill[row][col] = f;
							}
					}
				}
			}
			build_parallel_gaussian_execution_scheme(fill);
			return { fill_max, ops };
		}

		template <typename V>
		void gaussian_elimination(V & RHS)
		{
			const std::size_t iN = base_type::size();

			for (std::size_t i = 0; i < iN - 1; i++)
			{
				std::size_t nzbdp = 0;
				std::size_t pi = base_type::diag[i];
				auto f = reciprocal(base_type::A[pi++]);
				const std::size_t piie = base_type::row_idx[i+1];

				const auto *nz = base_type::m_nzbd[i];
				while (auto j = nz[nzbdp++]) // NOLINT(bugprone-infinite-loop)
				{
					// proceed to column i

					std::size_t pj = base_type::row_idx[j];
					std::size_t pje = base_type::row_idx[j+1];

					while (base_type::col_idx[pj] < i)
						pj++;

					const typename base_type::value_type f1 = - base_type::A[pj++] * f;

					// subtract row i from j
					// fill-in available assumed, i.e. matrix was prepared

					for (std::size_t pii = pi; pii<piie && pj < pje; pii++)
					{
						while (base_type::col_idx[pj] < base_type::col_idx[pii])
							pj++;
						if (base_type::col_idx[pj] == base_type::col_idx[pii])
							base_type::A[pj++] += base_type::A[pii] * f1;
					}

					RHS[j] += f1 * RHS[i];
				}
			}
		}

		int get_parallel_level(std::size_t k) const
		{
			for (std::size_t i = 0; i <  m_ge_par.size(); i++)
				if (plib::container::contains( m_ge_par[i], k))
					return narrow_cast<int>(i);
			return -1;
		}

		template <typename V>
		void gaussian_elimination_parallel(V & RHS)
		{
			//printf("omp: %ld %d %d\n", m_ge_par.size(), nz_num, (int)m_ge_par[m_ge_par.size()-2].size());
			for (auto l = 0UL; l < m_ge_par.size(); l++)
				plib::omp::for_static(base_type::nz_num, 0UL, m_ge_par[l].size(), [this, &RHS, &l] (unsigned ll)
				{
					auto &i = m_ge_par[l][ll];
					{
						std::size_t nzbdp = 0;
						std::size_t pi = base_type::diag[i];
						const auto f = reciprocal(base_type::A[pi++]);
						const std::size_t piie = base_type::row_idx[i+1];
						const auto &nz = base_type::nzbd[i];

						while (auto j = nz[nzbdp++])
						{
							// proceed to column i

							std::size_t pj = base_type::row_idx[j];

							while (base_type::col_idx[pj] < i)
								pj++;

							auto f1 = - base_type::A[pj++] * f;

							// subtract row i from j
							// fill-in available assumed, i.e. matrix was prepared
							for (std::size_t pii = pi; pii<piie; pii++)
							{
								while (base_type::col_idx[pj] < base_type::col_idx[pii])
									pj++;
								if (base_type::col_idx[pj] == base_type::col_idx[pii])
									base_type::A[pj++] += base_type::A[pii] * f1;
							}
							RHS[j] += f1 * RHS[i];
						}
					}
				});
		}

		template <typename V1, typename V2>
		void gaussian_back_substitution(V1 &V, const V2 &RHS)
		{
			const std::size_t iN = base_type::size();
			// row n-1
			V[iN - 1] = RHS[iN - 1] / base_type::A[base_type::diag[iN - 1]];

			for (std::size_t j = iN - 1; j-- > 0;)
			{
				typename base_type::value_type tmp = 0;
				const auto jdiag = base_type::diag[j];
				const std::size_t e = base_type::row_idx[j+1];
				for (std::size_t pk = jdiag + 1; pk < e; pk++)
					tmp += base_type::A[pk] * V[base_type::col_idx[pk]];
				V[j] = (RHS[j] - tmp) / base_type::A[jdiag];
			}
		}

		template <typename V1>
		void gaussian_back_substitution(V1 &V)
		{
			const std::size_t iN = base_type::size();
			// row n-1
			V[iN - 1] = V[iN - 1] / base_type::A[base_type::diag[iN - 1]];

			for (std::size_t j = iN - 1; j-- > 0;)
			{
				typename base_type::value_type tmp = 0;
				const auto jdiag = base_type::diag[j];
				const std::size_t e = base_type::row_idx[j+1];
				for (std::size_t pk = jdiag + 1; pk < e; pk++)
					tmp += base_type::A[pk] * V[base_type::col_idx[pk]];
				V[j] = (V[j] - tmp) / base_type::A[jdiag];
			}
		}

	private:
		template <typename M>
		void build_parallel_gaussian_execution_scheme(const M &fill)
		{
			// calculate parallel scheme for gaussian elimination
			std::vector<std::vector<std::size_t>> rt(base_type::size());
			for (std::size_t k = 0; k < base_type::size(); k++)
			{
				for (std::size_t j = k+1; j < base_type::size(); j++)
				{
					if (fill[j][k] < base_type::FILL_INFINITY)
					{
						rt[k].push_back(j);
					}
				}
			}

			std::vector<std::size_t> levGE(base_type::size(), 0);
			std::size_t cl = 0;

			for (std::size_t k = 0; k < base_type::size(); k++ )
			{
				if (levGE[k] >= cl)
				{
					std::vector<std::size_t> t = rt[k];
					for (std::size_t j = k+1; j < base_type::size(); j++ )
					{
						bool overlap = false;
						// is there overlap
						if (plib::container::contains(t, j))
							overlap = true;
						for (auto &x : rt[j])
							if (plib::container::contains(t, x))
							{
								overlap = true;
								break;
							}
						if (overlap)
							levGE[j] = cl + 1;
						else
						{
							t.push_back(j);
							for (auto &x : rt[j])
								t.push_back(x);
						}
					}
					cl++;
				}
			}

			m_ge_par.clear();
			m_ge_par.resize(cl+1);
			for (std::size_t k = 0; k < base_type::size(); k++)
				m_ge_par[levGE[k]].push_back(k);
			//for (std::size_t k = 0; k < m_ge_par.size(); k++)
			//  printf("%d %d\n", (int) k, (int) m_ge_par[k].size());
		}
		std::vector<std::vector<std::size_t>> m_ge_par; // parallel execution support for Gauss
	};

	template<typename B>
	struct pLUmatrix_cr : public B
	{
		using base_type = B;
		using index_type = typename base_type::index_type;

		pLUmatrix_cr(const pLUmatrix_cr &) = default;
		pLUmatrix_cr &operator=(const pLUmatrix_cr &) = default;
		pLUmatrix_cr(pLUmatrix_cr &&) noexcept(std::is_nothrow_move_constructible<base_type>::value) = default;
		pLUmatrix_cr &operator=(pLUmatrix_cr &&) noexcept(std::is_nothrow_move_assignable<base_type>::value) = default;

		template<typename ARENA>
		explicit pLUmatrix_cr(ARENA &arena, std::size_t n)
		: B(arena, n)
		, ilu_rows(n+1)
		, m_ILUp(0)
		{
		}

		~pLUmatrix_cr() = default;

		template <typename M>
		void build(M &fill, std::size_t ilup)
		{
			std::size_t p(0);
			// build ilu_rows
			for (decltype(fill.size()) i=1; i < fill.size(); i++)
			{
				bool found(false);
				for (decltype(fill.size()) k = 0; k < i; k++)
				{
					// if (fill[i][k] < base::FILL_INFINITY)
					if (fill[i][k] <= ilup)
					{
						// assume A[k][k]!=0
						for (decltype(fill.size()) j=k+1; j < fill.size(); j++)
						{
							auto f = std::min(fill[i][j], 1 + fill[i][k] + fill[k][j]);
							if (f <= ilup)
								fill[i][j] = f;
						}
						found = true;
					}
				}
				if (found)
					ilu_rows[p++] = narrow_cast<index_type>(i);
			}
			ilu_rows[p] = 0; // end of array
			this->build_from_fill_mat(fill, ilup); //, m_band_width); // ILU(2)
			m_ILUp = ilup;
		}

		/// \brief incomplete LU Factorization.
		///
		/// We are following http://de.wikipedia.org/wiki/ILU-Zerlegung here.
		///
		/// The result is stored in matrix LU
		///
		/// For i = 1,...,N-1
		///   For k = 0, ... , i - 1
		///     If a[i,k] != 0
		///       a[i,k] = a[i,k] / a[k,k]
		///       For j = k + 1, ... , N - 1
		///         If a[i,j] != 0
		///           a[i,j] = a[i,j] - a[i,k] * a[k,j]
		///         j=j+1
		///      k=k+1
		///    i=i+1
		///
		void incomplete_LU_factorization(const base_type &mat)
		{
			if (m_ILUp < 1)
				this->raw_copy_from(mat);
			else
				this->reduction_copy_from(mat);

			std::size_t p(0);
			while (auto i = ilu_rows[p++]) // NOLINT(bugprone-infinite-loop)
			{
				const auto p_i_end = base_type::row_idx[i + 1];
				// loop over all columns k left of diag in row i
				//if (row_idx[i] < diag[i])
				//  printf("occ %d\n", (int)i);
				for (auto i_k = base_type::row_idx[i]; i_k < base_type::diag[i]; i_k++)
				{
					const index_type k(base_type::col_idx[i_k]);
					const index_type p_k_end(base_type::row_idx[k + 1]);
					const typename base_type::value_type LUp_i_k = base_type::A[i_k] = base_type::A[i_k] / base_type::A[base_type::diag[k]];

					std::size_t k_j(base_type::diag[k] + 1);
					std::size_t i_j(i_k + 1);

					while (i_j < p_i_end && k_j < p_k_end )  // pj = (i, j)
					{
						// we can assume that within a row ja increases continuously
						const index_type c_i_j(base_type::col_idx[i_j]); // row i, column j
						const index_type c_k_j(base_type::col_idx[k_j]); // row k, column j

						if (c_k_j == c_i_j)
							base_type::A[i_j] -= LUp_i_k * base_type::A[k_j];
						k_j += (c_k_j <= c_i_j ? 1 : 0);
						i_j += (c_k_j >= c_i_j ? 1 : 0);

					}
				}
			}
		}


		/// \brief Solve a linear equation
		///
		/// Solve a linear equation Ax = r
		///
		/// where
		///     A = L*U
		///
		///     L unit lower triangular
		///     U upper triangular
		///
		/// ==> LUx = r
		///
		/// ==> Ux = L⁻¹ r = w
		///
		/// ==> r = Lw
		///
		/// This can be solved for w using backwards elimination in L.
		///
		/// Now Ux = w
		///
		/// This can be solved for x using backwards elimination in U.
		///
		template <typename R>
		void solveLU (R &r)
		{
			for (std::size_t i = 1; i < base_type::size(); ++i )
			{
				typename base_type::value_type tmp(0);
				const index_type j1(base_type::row_idx[i]);
				const index_type j2(base_type::diag[i]);

				for (auto j = j1; j < j2; ++j )
					tmp +=  base_type::A[j] * r[base_type::col_idx[j]];
				r[i] -= tmp;
			}
			// i now is equal to n;
			for (std::size_t i = base_type::size(); i-- > 0; )
			{
				typename base_type::value_type tmp(0);
				const index_type di(base_type::diag[i]);
				const index_type j2(base_type::row_idx[i+1]);
				for (std::size_t j = di + 1; j < j2; j++ )
					tmp += base_type::A[j] * r[base_type::col_idx[j]];
				r[i] = (r[i] - tmp) / base_type::A[di];
			}
		}
	private:
		parray<index_type, base_type::Np1> ilu_rows;
		std::size_t m_ILUp;
	};

} // namespace plib

#endif // PMATRIX_CR_H_
