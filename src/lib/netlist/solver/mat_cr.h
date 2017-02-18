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

#include <algorithm>
#include "../plib/pconfig.h"
#include "../plib/palloc.h"

template<std::size_t N, typename C = uint16_t, typename T = double>
struct mat_cr_t
{
	typedef C index_type;
	typedef T value_type;

	C diag[N];      // diagonal index pointer n
	C ia[N+1];      // row index pointer n + 1
	C ja[N*N];       // column index array nz_num, initially (n * n)
	T A[N*N];    // Matrix elements nz_num, initially (n * n)

	std::size_t size;
	std::size_t nz_num;

	mat_cr_t(const std::size_t n)
	: size(n)
	, nz_num(0)
	{
#if 0
#if 0
		ia = plib::palloc_array<C>(n + 1);
		ja = plib::palloc_array<C>(n * n);
		diag = plib::palloc_array<C>(n);
#else
		diag = plib::palloc_array<C>(n + (n + 1) + n * n);
		ia = diag + n;
		ja = ia + (n+1);
		A = plib::palloc_array<T>(n * n);
#endif
#endif
	}

	~mat_cr_t()
	{
#if 0
		plib::pfree_array(diag);
#if 0
		plib::pfree_array(ia);
		plib::pfree_array(ja);
#endif
		plib::pfree_array(A);
#endif
	}

	void set_scalar(const T scalar)
	{
		for (std::size_t i=0, e=nz_num; i<e; i++)
			A[i] = scalar;
	}

	void mult_vec(const T * RESTRICT x, T * RESTRICT res)
	{
		/*
		 * res = A * x
		 */

		std::size_t i = 0;
		std::size_t k = 0;
		const std::size_t oe = nz_num;

		while (k < oe)
		{
			T tmp = 0.0;
			const std::size_t e = ia[i+1];
			for (; k < e; k++)
				tmp += A[k] * x[ja[k]];
			res[i++] = tmp;
		}
	}

	void incomplete_LU_factorization(T * RESTRICT LU)
	{
		/*
		 * incomplete LU Factorization according to http://de.wikipedia.org/wiki/ILU-Zerlegung
		 *
		 * Result is stored in matrix LU
		 *
		 */

		const std::size_t lnz = nz_num;

		for (std::size_t k = 0; k < lnz; k++)
			LU[k] = A[k];

		for (std::size_t i = 1; ia[i] < lnz; i++) // row i
		{
			const std::size_t iai1 = ia[i + 1];
			const std::size_t pke = diag[i];
			for (std::size_t pk = ia[i]; pk < pke; pk++) // all columns left of diag in row i
			{
				// pk == (i, k)
				const std::size_t k = ja[pk];
				const std::size_t iak1 = ia[k + 1];
				const T LUpk = LU[pk] = LU[pk] / LU[diag[k]];

				std::size_t pt = ia[k];

				for (std::size_t pj = pk + 1; pj < iai1; pj++)  // pj = (i, j)
				{
					// we can assume that within a row ja increases continuously */
					const std::size_t ej = ja[pj];
					while (ja[pt] < ej && pt < iak1)
						pt++;
					if (pt < iak1 && ja[pt] == ej)
						LU[pj] = LU[pj] - LUpk * LU[pt];
				}
			}
		}
	}

	void solveLUx (const T * RESTRICT LU, T * RESTRICT r)
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

		for (std::size_t i = 1; ia[i] < nz_num; ++i )
		{
			T tmp = 0.0;
			const std::size_t j1 = ia[i];
			const std::size_t j2 = diag[i];

			for (std::size_t j = j1; j < j2; ++j )
				tmp +=  LU[j] * r[ja[j]];

			r[i] -= tmp;
		}
		// i now is equal to n;
		for (std::size_t i = size; i-- > 0; )
		{
			T tmp = 0.0;
			const std::size_t di = diag[i];
			const std::size_t j2 = ia[i+1];
			for (std::size_t j = di + 1; j < j2; j++ )
				tmp += LU[j] * r[ja[j]];
			r[i] = (r[i] - tmp) / LU[di];
		}
	}
};

#endif /* MAT_CR_H_ */
