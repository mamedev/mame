// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * mat_cr.h
 *
 * Compressed row format matrices
 *
 */

#ifndef MAT_CR_H_
#define MAT_CR_H_

#include "palloc.h"
#include "parray.h"
#include "pconfig.h"
#include "pomp.h"
#include "pstate.h"
#include "ptypes.h"
#include "putil.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <type_traits>
#include <vector>

namespace plib
{

	// FIXME: causes a crash with GMRES handler
	// template<typename T, int N, typename C = std::size_t>

	template<typename T, int N, typename C = uint16_t>
	struct matrix_compressed_rows_t
	{
		using index_type = C;
		using value_type = T;

		COPYASSIGNMOVE(matrix_compressed_rows_t, default)

		enum constants_e
		{
			FILL_INFINITY = 9999999
		};

		parray<index_type, N> diag;      // diagonal index pointer n
		parray<index_type, (N == 0) ? 0 : (N < 0 ? N - 1 : N + 1)> row_idx;      // row index pointer n + 1
		parray<index_type, N < 0 ? -N * N : N *N> col_idx;       // column index array nz_num, initially (n * n)
		parray<value_type, N < 0 ? -N * N : N *N> A;    // Matrix elements nz_num, initially (n * n)
		//parray<C, N < 0 ? -N * (N-1) / 2 : N * (N+1) / 2 > nzbd;    // Support for gaussian elimination
		parray<std::vector<index_type>, N > nzbd;    // Support for gaussian elimination
		// contains elimination rows below the diagonal
		// FIXME: convert to pvector
		std::vector<std::vector<index_type>> m_ge_par;

		index_type nz_num;

		explicit matrix_compressed_rows_t(const index_type n)
		: diag(n)
		, row_idx(n+1)
		, col_idx(n*n)
		, A(n*n)
		//, nzbd(n * (n+1) / 2)
		, nzbd(n)
		, nz_num(0)
		, m_size(n)
		{
			for (index_type i=0; i<n+1; i++)
				row_idx[i] = 0;
		}

		~matrix_compressed_rows_t() = default;

		constexpr index_type size() const { return static_cast<index_type>((N>0) ? N : m_size); }

		void clear()
		{
			nz_num = 0;
			for (index_type i=0; i < size() + 1; i++)
				row_idx[i] = 0;
		}

		void set_scalar(const T scalar)
		{
			for (index_type i=0, e=nz_num; i<e; i++)
				A[i] = scalar;
		}

		void set(C r, C c, T val)
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
		std::pair<std::size_t, std::size_t> gaussian_extend_fill_mat(M &fill)
		{
			std::size_t ops = 0;
			std::size_t fill_max = 0;

			for (std::size_t k = 0; k < fill.size(); k++)
			{
				ops++; // 1/A(k,k)
				for (std::size_t row = k + 1; row < fill.size(); row++)
				{
					if (fill[row][k] < FILL_INFINITY)
					{
						ops++;
						for (std::size_t col = k + 1; col < fill[row].size(); col++)
							//if (fill[k][col] < FILL_INFINITY)
							{
								auto f = std::min(fill[row][col], 1 + fill[row][k] + fill[k][col]);
								if (f < FILL_INFINITY)
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

		template <typename M>
		void build_from_fill_mat(const M &f, std::size_t max_fill = FILL_INFINITY - 1,
			std::size_t band_width = FILL_INFINITY)
		{
			C nz = 0;
			if (nz_num != 0)
				throw pexception("build_from_mat only allowed on empty CR matrix");
			for (std::size_t k=0; k < size(); k++)
			{
				row_idx[k] = nz;

				for (std::size_t j=0; j < size(); j++)
					if (f[k][j] <= max_fill && std::abs(static_cast<int>(k)-static_cast<int>(j)) <= static_cast<int>(band_width))
					{
						col_idx[nz] = static_cast<C>(j);
						if (j == k)
							diag[k] = nz;
						nz++;
					}
			}

			row_idx[size()] = nz;
			nz_num = nz;
			/* build nzbd */

			for (std::size_t k=0; k < size(); k++)
			{
				for (std::size_t j=k + 1; j < size(); j++)
					if (f[j][k] < FILL_INFINITY)
						nzbd[k].push_back(static_cast<C>(j));
				nzbd[k].push_back(0); // end of sequence
			}
		}

		template <typename V>
		void gaussian_elimination(V & RHS)
		{
			const std::size_t iN = size();

			for (std::size_t i = 0; i < iN - 1; i++)
			{
				std::size_t nzbdp = 0;
				std::size_t pi = diag[i];
				const value_type f = 1.0 / A[pi++];
				const std::size_t piie = row_idx[i+1];
				const auto &nz = nzbd[i];

				while (auto j = nz[nzbdp++])
				{
					// proceed to column i

					std::size_t pj = row_idx[j];

					while (col_idx[pj] < i)
						pj++;

					const value_type f1 = - A[pj++] * f;

					// subtract row i from j
					// fill-in available assumed, i.e. matrix was prepared

					for (std::size_t pii = pi; pii<piie; pii++)
					{
						while (col_idx[pj] < col_idx[pii])
							pj++;
						if (col_idx[pj] == col_idx[pii])
							A[pj++] += A[pii] * f1;
					}

					RHS[j] += f1 * RHS[i];
				}
			}
		}

		template <typename V>
		void gaussian_elimination_parallel(V & RHS)
		{
			// FIXME: move into solver creation ...
			plib::omp::set_num_threads(4);
			for (auto l = 0ul; l < m_ge_par.size(); l++)
			plib::omp::for_static(0ul, m_ge_par[l].size(), [this, &RHS, &l] (unsigned ll)
			{
				auto &i = m_ge_par[l][ll];
				{
					std::size_t nzbdp = 0;
					std::size_t pi = diag[i];
					const value_type f = 1.0 / A[pi++];
					const std::size_t piie = row_idx[i+1];

					while (auto j = nzbd[i][nzbdp++])
					{
						// proceed to column i

						std::size_t pj = row_idx[j];

						while (col_idx[pj] < i)
							pj++;

						const value_type f1 = - A[pj++] * f;

						// subtract row i from j
						// fill-in available assumed, i.e. matrix was prepared
						for (std::size_t pii = pi; pii<piie; pii++)
						{
							while (col_idx[pj] < col_idx[pii])
								pj++;
							if (col_idx[pj] == col_idx[pii])
								A[pj++] += A[pii] * f1;
						}
						RHS[j] += f1 * RHS[i];
					}
				}
			});
		}

		template <typename V1, typename V2>
		void gaussian_back_substitution(V1 &V, const V2 &RHS)
		{
			const std::size_t iN = size();
			/* row n-1 */
			V[iN - 1] = RHS[iN - 1] / A[diag[iN - 1]];

			for (std::size_t j = iN - 1; j-- > 0;)
			{
				value_type tmp = 0;
				const auto jdiag = diag[j];
				const std::size_t e = row_idx[j+1];
				for (std::size_t pk = jdiag + 1; pk < e; pk++)
					tmp += A[pk] * V[col_idx[pk]];
				V[j] = (RHS[j] - tmp) / A[jdiag];
			}
		}

		template <typename V1>
		void gaussian_back_substitution(V1 &V)
		{
			const std::size_t iN = size();
			/* row n-1 */
			V[iN - 1] = V[iN - 1] / A[diag[iN - 1]];

			for (std::size_t j = iN - 1; j-- > 0;)
			{
				value_type tmp = 0;
				const auto jdiag = diag[j];
				const std::size_t e = row_idx[j+1];
				for (std::size_t pk = jdiag + 1; pk < e; pk++)
					tmp += A[pk] * V[col_idx[pk]];
				V[j] = (V[j] - tmp) / A[jdiag];
			}
		}


		template <typename VTV, typename VTR>
		void mult_vec(VTR & res, const VTV & x)
		{
			/*
			 * res = A * x
			 */

			std::size_t row = 0;
			std::size_t k = 0;
			const std::size_t oe = nz_num;

			while (k < oe)
			{
				T tmp = 0.0;
				const std::size_t e = row_idx[row+1];
				for (; k < e; k++)
					tmp += A[k] * x[col_idx[k]];
				res[row++] = tmp;
			}
		}

		/* throws error if P(source)>P(destination) */
		template <typename LUMAT>
		void slim_copy_from(LUMAT & src)
		{
			for (std::size_t r=0; r<src.size(); r++)
			{
				C dp = row_idx[r];
				for (C sp = src.row_idx[r]; sp < src.row_idx[r+1]; sp++)
				{
					/* advance dp to source column and fill 0s if necessary */
					while (col_idx[dp] < src.col_idx[sp])
						A[dp++] = 0;
					if (row_idx[r+1] <= dp || col_idx[dp] != src.col_idx[sp])
						throw plib::pexception("slim_copy_from error");
					A[dp++] = src.A[sp];
				}
				/* fill remaining elements in row */
				while (dp < row_idx[r+1])
					A[dp++] = 0;
			}
		}

		/* only copies common elements */
		template <typename LUMAT>
		void reduction_copy_from(LUMAT & src)
		{
			C sp = 0;
			for (std::size_t r=0; r<src.size(); r++)
			{
				C dp = row_idx[r];
				while(sp < src.row_idx[r+1])
				{
					/* advance dp to source column and fill 0s if necessary */
					if (col_idx[dp] < src.col_idx[sp])
						A[dp++] = 0;
					else if (col_idx[dp] == src.col_idx[sp])
						A[dp++] = src.A[sp++];
					else
						sp++;
				}
				/* fill remaining elements in row */
				while (dp < row_idx[r+1])
					A[dp++] = 0;
			}
		}

		/* checks at all - may crash */
		template <typename LUMAT>
		void raw_copy_from(LUMAT & src)
		{
			for (std::size_t k = 0; k < nz_num; k++)
				A[k] = src.A[k];
		}

		void incomplete_LU_factorization()
		{
			/*
			 * incomplete LU Factorization according to http://de.wikipedia.org/wiki/ILU-Zerlegung
			 *
			 * Result is stored in matrix LU
			 *
			 * For i = 1,...,N-1
			 *   For k = 0, ... , i - 1
			 *     If a[i,k] != 0
			 *       a[i,k] = a[i,k] / a[k,k]
			 *       For j = k + 1, ... , N - 1
			 *         If a[i,j] != 0
			 *           a[i,j] = a[i,j] - a[i,k] * a[k,j]
			 *         j=j+1
			 *      k=k+1
			 *    i=i+1
			 *
			 */

			for (std::size_t i = 1; i < size(); i++) // row i
			{
				const std::size_t p_i_end = row_idx[i + 1];
				// loop over all columns k left of diag in row i
				for (std::size_t i_k = row_idx[i]; i_k < diag[i]; i_k++)
				{
					const std::size_t k = col_idx[i_k];
					const std::size_t p_k_end = row_idx[k + 1];
					const T LUp_i_k = A[i_k] = A[i_k] / A[diag[k]];

					std::size_t k_j = diag[k] + 1;
					std::size_t i_j = i_k + 1;

					while (i_j < p_i_end && k_j < p_k_end )  // pj = (i, j)
					{
						// we can assume that within a row ja increases continuously */
						const std::size_t c_i_j = col_idx[i_j]; // row i, column j
						const std::size_t c_k_j = col_idx[k_j]; // row i, column j
						if (c_k_j < c_i_j)
							k_j++;
						else if (c_k_j == c_i_j)
							A[i_j++] -= LUp_i_k * A[k_j++];
						else
							i_j++;
					}
				}
			}
		}

		template <typename R>
		void solveLUx (R &r)
		{
			/*
			 * Solve a linear equation Ax = r
			 * where
			 *      A = L*U
			 *
			 *      L unit lower triangular
			 *      U upper triangular
			 *
			 * ==> LUx = r
			 *
			 * ==> Ux = L⁻¹ r = w
			 *
			 * ==> r = Lw
			 *
			 * This can be solved for w using backwards elimination in L.
			 *
			 * Now Ux = w
			 *
			 * This can be solved for x using backwards elimination in U.
			 *
			 */
			for (std::size_t i = 1; i < size(); ++i )
			{
				T tmp = 0.0;
				const std::size_t j1 = row_idx[i];
				const std::size_t j2 = diag[i];

				for (std::size_t j = j1; j < j2; ++j )
					tmp +=  A[j] * r[col_idx[j]];
				r[i] -= tmp;
			}
			// i now is equal to n;
			for (std::size_t i = size(); i-- > 0; )
			{
				T tmp = 0.0;
				const std::size_t di = diag[i];
				const std::size_t j2 = row_idx[i+1];
				for (std::size_t j = di + 1; j < j2; j++ )
					tmp += A[j] * r[col_idx[j]];
				r[i] = (r[i] - tmp) / A[di];
			}
		}
	private:
		template <typename M>
		void build_parallel_gaussian_execution_scheme(const M &fill)
		{
			// calculate parallel scheme for gaussian elimination
			std::vector<std::vector<index_type>> rt(size());
			for (index_type k = 0; k < size(); k++)
			{
				for (index_type j = k+1; j < size(); j++)
				{
					if (fill[j][k] < FILL_INFINITY)
					{
						rt[k].push_back(j);
					}
				}
			}

			std::vector<index_type> levGE(size(), 0);
			index_type cl = 0;

			for (index_type k = 0; k < size(); k++ )
			{
				if (levGE[k] >= cl)
				{
					std::vector<index_type> t = rt[k];
					for (index_type j = k+1; j < size(); j++ )
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
			for (index_type k = 0; k < size(); k++)
				m_ge_par[levGE[k]].push_back(k);
		}

		index_type m_size;
	};

} // namespace plib

#endif /* MAT_CR_H_ */
