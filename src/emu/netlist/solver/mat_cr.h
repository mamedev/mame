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

template<int _storage_N>
struct mat_cr_t
{
	unsigned nz_num;
	unsigned ia[_storage_N + 1];
	unsigned ja[_storage_N * _storage_N];
	unsigned diag[_storage_N];       /* n */

	void mult_vec(const double * RESTRICT A, const double * RESTRICT x, double * RESTRICT res)
	{
		/*
		 * res = A * x
		 */

		unsigned i = 0;
		unsigned k = 0;
		const unsigned oe = nz_num;

		while (k < oe)
		{
			double tmp = 0.0;
			const unsigned e = ia[i+1];
			for (; k < e; k++)
				tmp += A[k] * x[ja[k]];
			res[i++] = tmp;
		}
	}

	void incomplete_LU_factorization(const double * RESTRICT A, double * RESTRICT LU)
	{
		/*
		 * incomplete LU Factorization according to http://de.wikipedia.org/wiki/ILU-Zerlegung
		 *
		 * Result is stored in matrix LU
		 *
		 */

		const unsigned lnz = nz_num;

		for (unsigned k = 0; k < lnz; k++)
			LU[k] = A[k];

		for (unsigned i = 1; ia[i] < lnz; i++) // row i
		{
			const unsigned iai1 = ia[i + 1];
			for (unsigned pk = ia[i]; pk < diag[i]; pk++) // all columns left of diag in row i
			{
				// pk == (i, k)
				const unsigned k = ja[pk];
				const unsigned iak1 = ia[k + 1];
				const double LUpk = LU[pk] = LU[pk] / LU[diag[k]];

				unsigned pt = ia[k];

				for (unsigned pj = pk + 1; pj < iai1; pj++)  // pj = (i, j)
				{
					// we can assume that within a row ja increases continuously */
					const unsigned ej = ja[pj];
					while (ja[pt] < ej && pt < iak1)
						pt++;
					if (pt < iak1 && ja[pt] == ej)
						LU[pj] = LU[pj] - LUpk * LU[pt];
				}
			}
		}
	}

	void solveLUx (const double * RESTRICT LU, double * RESTRICT r)
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
		 * ==> Ux = L?????r = w
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

		unsigned i;

		for (i = 1; ia[i] < nz_num; i++ )
		{
			double tmp = 0.0;
			const unsigned j1 = ia[i];
			const unsigned j2 = diag[i];

			for (unsigned j = j1; j < j2; j++ )
				tmp +=  LU[j] * r[ja[j]];

			r[i] -= tmp;
		}
		// i now is equal to n;
		for (; 0 < i; i-- )
		{
			const unsigned im1 = i - 1;
			double tmp = 0.0;
			const unsigned j1 = diag[im1] + 1;
			const unsigned j2 = ia[im1+1];
			for (int j = j1; j < j2; j++ )
				tmp += LU[j] * r[ja[j]];
			r[im1] = (r[im1] - tmp) / LU[diag[im1]];
		}
	}

};

#endif /* MAT_CR_H_ */
