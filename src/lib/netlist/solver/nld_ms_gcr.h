// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_gcr.h
 *
 * Gaussian elimination using compressed row format.
 *
 * Fow w==1 we will do the classic Gauss-Seidel approach
 *
 */

#ifndef NLD_MS_GCR_H_
#define NLD_MS_GCR_H_

#include <algorithm>

#include "solver/mat_cr.h"
#include "solver/nld_ms_direct.h"
#include "solver/nld_solver.h"
#include "solver/vector_base.h"

#define NL_USE_SSE 0

NETLIB_NAMESPACE_DEVICES_START()

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_GCR_t: public matrix_solver_direct_t<m_N, _storage_N>
{
public:

	matrix_solver_GCR_t(const solver_parameters_t *params, int size)
		: matrix_solver_direct_t<m_N, _storage_N>(matrix_solver_t::GAUSSIAN_ELIMINATION, params, size)
		{
		}

	virtual ~matrix_solver_GCR_t()
	{
	}

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual int vsolve_non_dynamic(const bool newton_raphson) override;

private:

	pvector_t<int> m_term_cr[_storage_N];
	mat_cr_t<_storage_N> mat;
	nl_double m_A[_storage_N * _storage_N];

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GMRES
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_GCR_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_direct_t<m_N, _storage_N>::vsetup(nets);

	unsigned nz = 0;
	const unsigned iN = this->N();

	/* build the final matrix */

	bool touched[_storage_N][_storage_N] = { { false } };
	for (unsigned k = 0; k < iN; k++)
	{
		for (auto & j : this->m_terms[k]->m_nz)
			touched[k][j] = true;
	}

	unsigned fc = 0;

	unsigned ops = 0;

	const bool static_compile = false;
	for (unsigned k = 0; k < iN; k++)
	{
		ops++; // 1/A(k,k)
		if (static_compile) printf("const double fd%d = 1.0 / A(%d,%d); \n", k, k, k);
		for (unsigned row = k + 1; row < iN; row++)
		{
			if (touched[row][k])
			{
				ops++;
				fc++;
				if (static_compile) printf("  const double f%d = -fd%d * A(%d,%d); \n", fc, k, row, k);
				for (unsigned col = k + 1; col < iN; col++)
					if (touched[k][col])
					{
						if (touched[row][col])
						{
							if (static_compile) printf("    A(%d,%d) += f%d * A(%d,%d); \n", row, col, fc, k, col);
						} else
						{
							if (static_compile) printf("    A(%d,%d) = f%d * A(%d,%d); \n", row, col, fc, k, col);
						}
						touched[row][col] = true;
						ops += 2;
					}
				if (static_compile) printf("    RHS(%d) += f%d * RHS(%d); \n", row, fc, k);
			}
		}
	}


	for (unsigned k=0; k<iN; k++)
	{
		mat.ia[k] = nz;

		for (unsigned j=0; j<iN; j++)
		{
			if (touched[k][j])
			{
				mat.ja[nz] = j;
				if (j == k)
					mat.diag[k] = nz;
				nz++;
			}
		}

		m_term_cr[k].clear();
		/* build pointers into the compressed row format matrix for each terminal */
		for (unsigned j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			for (unsigned i = mat.ia[k]; i<nz; i++)
				if (this->m_terms[k]->net_other()[j] == (int) mat.ja[i])
				{
					m_term_cr[k].push_back(i);
					break;
				}
			nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
		}
	}

	mat.ia[iN] = nz;
	mat.nz_num = nz;

	this->log().verbose("Ops: {1}  Occupancy ratio: {2}\n", ops, (double) nz / double (iN * iN));
}

template <unsigned m_N, unsigned _storage_N>
int matrix_solver_GCR_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const unsigned iN = this->N();

	ATTR_ALIGN nl_double RHS[_storage_N];
	ATTR_ALIGN nl_double new_V[_storage_N];

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

#if (NL_USE_SSE)
		__m128d mg = _mm_set_pd(0.0, 0.0);
		__m128d mr = _mm_set_pd(0.0, 0.0);
		unsigned i = 0;
		for (; i < term_count - 1; i+=2)
		{
			mg = _mm_add_pd(mg, _mm_loadu_pd(&gt[i]));
			mr = _mm_add_pd(mr, _mm_loadu_pd(&Idr[i]));
		}
		gtot_t = _mm_cvtsd_f64(mg) + _mm_cvtsd_f64(_mm_unpackhi_pd(mg,mg));
		RHS_t = _mm_cvtsd_f64(mr) + _mm_cvtsd_f64(_mm_unpackhi_pd(mr,mr));
		for (; i < term_count; i++)
		{
			gtot_t += gt[i];
			RHS_t += Idr[i];
		}
#else
		for (unsigned i = 0; i < term_count; i++)
		{
			gtot_t += gt[i];
			RHS_t += Idr[i];
		}
#endif
		for (unsigned i = railstart; i < term_count; i++)
			RHS_t += go[i] * *other_cur_analog[i];

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

	/* now solve it */

	for (unsigned i = 0; i < iN - 1; i++)
	{
		const auto &nzbd = this->m_terms[i]->m_nzbd;

		if (nzbd.size() > 0)
		{
			unsigned pi = mat.diag[i];
			const nl_double f = 1.0 / m_A[pi++];
			const unsigned piie = mat.ia[i+1];

			for (auto & j : nzbd)
			{
				// proceed to column i
				//__builtin_prefetch(&m_A[mat.diag[j+1]], 1);
				unsigned pj = mat.ia[j];

				while (mat.ja[pj] < i)
					pj++;

				const nl_double f1 = - m_A[pj++] * f;

				// subtract row i from j */
				for (unsigned pii = pi; pii<piie; )
				{
					while (mat.ja[pj] < mat.ja[pii])
						pj++;
					m_A[pj++] += m_A[pii++] * f1;
				}
				RHS[j] += f1 * RHS[i];
			}
		}
	}

	/* backward substitution
	 *
	 */

	/* row n-1 */
	new_V[iN - 1] = RHS[iN - 1] / m_A[mat.diag[iN - 1]];

	for (int j = iN - 2; j >= 0; j--)
	{
		//__builtin_prefetch(&new_V[j-1], 1);
		//if (j>0)__builtin_prefetch(&m_A[mat.diag[j-1]], 0);
#if (NL_USE_SSE)
		__m128d tmp = _mm_set_pd1(0.0);
		const unsigned e = mat.ia[j+1];
		unsigned pk = mat.diag[j] + 1;
		for (; pk < e - 1; pk+=2)
		{
			//tmp += m_A[pk] * new_V[mat.ja[pk]];
			tmp = _mm_add_pd(tmp, _mm_mul_pd(_mm_set_pd(m_A[pk], m_A[pk+1]),
					_mm_set_pd(new_V[mat.ja[pk]], new_V[mat.ja[pk+1]])));
		}
		double tmpx = _mm_cvtsd_f64(tmp) + _mm_cvtsd_f64(_mm_unpackhi_pd(tmp,tmp));
		for (; pk < e; pk++)
		{
			tmpx += m_A[pk] * new_V[mat.ja[pk]];
		}
		new_V[j] = (RHS[j] - tmpx) / m_A[mat.diag[j]];
#else
		double tmp = 0;
		const unsigned e = mat.ia[j+1];
		for (unsigned pk = mat.diag[j] + 1; pk < e; pk++)
		{
			tmp += m_A[pk] * new_V[mat.ja[pk]];
		}
		new_V[j] = (RHS[j] - tmp) / m_A[mat.diag[j]];
#endif
	}
	this->m_stat_calculations++;

	if (newton_raphson)
	{
		nl_double err = this->delta(new_V);

		this->store(new_V);

		return (err > this->m_params.m_accuracy) ? 2 : 1;
	}
	else
	{
		this->store(new_V);
		return 1;
	}
}

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_GCR_H_ */
