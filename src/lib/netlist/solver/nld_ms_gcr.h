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

#include "plib/pdynlib.h"
#include "solver/mat_cr.h"
#include "solver/nld_ms_direct.h"
#include "solver/nld_solver.h"
#include "solver/vector_base.h"
#include "plib/pstream.h"

#define NL_USE_SSE 0
#if NL_USE_SSE
#include <mmintrin.h>
#endif

namespace netlist
{
	namespace devices
	{
template <std::size_t m_N, std::size_t storage_N>
class matrix_solver_GCR_t: public matrix_solver_t
{
public:

	matrix_solver_GCR_t(netlist_t &anetlist, const pstring &name,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_t(anetlist, name, matrix_solver_t::ASCENDING, params)
		, m_dim(size)
		, mat(size)
		, m_proc(nullptr)
		{
		}

	virtual ~matrix_solver_GCR_t()
	{
	}

	constexpr std::size_t N() const { return (m_N == 0) ? m_dim : m_N; }

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;

	virtual std::pair<pstring, pstring> create_solver_code() override;

private:

	//typedef typename mat_cr_t<storage_N>::type mattype;
	typedef typename mat_cr_t<storage_N>::index_type mattype;

	void csc_private(plib::putf8_fmt_writer &strm);

	using extsolver = void (*)(double * RESTRICT m_A, double * RESTRICT RHS);

	pstring static_compile_name();

	const std::size_t m_dim;
	std::vector<unsigned> m_term_cr[storage_N];
	mat_cr_t<storage_N> mat;

	extsolver m_proc;

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GMRES
// ----------------------------------------------------------------------------------------

template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_GCR_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	setup_base(nets);

	mattype nz = 0;
	const std::size_t iN = this->N();

	/* build the final matrix */

	bool touched[storage_N][storage_N] = { { false } };
	for (std::size_t k = 0; k < iN; k++)
	{
		for (auto &j : this->m_terms[k]->m_nz)
			touched[k][j] = true;
	}

	unsigned fc = 0;

	unsigned ops = 0;

	for (std::size_t k = 0; k < iN; k++)
	{
		ops++; // 1/A(k,k)
		for (std::size_t row = k + 1; row < iN; row++)
		{
			if (touched[row][k])
			{
				ops++;
				fc++;
				for (std::size_t col = k + 1; col < iN; col++)
					if (touched[k][col])
					{
						touched[row][col] = true;
						ops += 2;
					}
			}
		}
	}


	for (mattype k=0; k<iN; k++)
	{
		mat.ia[k] = nz;

		for (mattype j=0; j<iN; j++)
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
		for (std::size_t j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			int other = this->m_terms[k]->connected_net_idx()[j];
			for (auto i = mat.ia[k]; i < nz; i++)
				if (other == static_cast<int>(mat.ja[i]))
				{
					m_term_cr[k].push_back(i);
					break;
				}
		}
		nl_assert(m_term_cr[k].size() == this->m_terms[k]->m_railstart);
	}

	mat.ia[iN] = nz;
	mat.nz_num = nz;

	this->log().verbose("Ops: {1}  Occupancy ratio: {2}\n", ops,
			static_cast<double>(nz) / static_cast<double>(iN * iN));

	// FIXME: Move me

	if (netlist().lib().isLoaded())
	{
		pstring symname = static_compile_name();
		m_proc = this->netlist().lib().template getsym<extsolver>(symname);
		if (m_proc != nullptr)
			this->log().verbose("External static solver {1} found ...", symname);
		else
			this->log().warning("External static solver {1} not found ...", symname);
	}

}

template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_GCR_t<m_N, storage_N>::csc_private(plib::putf8_fmt_writer &strm)
{
	const std::size_t iN = N();
	for (std::size_t i = 0; i < iN - 1; i++)
	{
		const auto &nzbd = this->m_terms[i]->m_nzbd;

		if (nzbd.size() > 0)
		{
			std::size_t pi = mat.diag[i];

			//const nl_double f = 1.0 / m_A[pi++];
			strm("const double f{1} = 1.0 / m_A[{2}];\n", i, pi);
			pi++;
			const std::size_t piie = mat.ia[i+1];

			//for (auto & j : nzbd)
			for (std::size_t j : nzbd)
			{
				// proceed to column i
				std::size_t pj = mat.ia[j];

				while (mat.ja[pj] < i)
					pj++;

				//const nl_double f1 = - m_A[pj++] * f;
				strm("\tconst double f{1}_{2} = -f{3} * m_A[{4}];\n", i, j, i, pj);
				pj++;

				// subtract row i from j */
				for (std::size_t pii = pi; pii<piie; )
				{
					while (mat.ja[pj] < mat.ja[pii])
						pj++;
					//m_A[pj++] += m_A[pii++] * f1;
					strm("\tm_A[{1}] += m_A[{2}] * f{3}_{4};\n", pj, pii, i, j);
					pj++; pii++;
				}
				//RHS[j] += f1 * RHS[i];
				strm("\tRHS[{1}] += f{2}_{3} * RHS[{4}];\n", j, i, j, i);
			}
		}
	}
}


template <std::size_t m_N, std::size_t storage_N>
pstring matrix_solver_GCR_t<m_N, storage_N>::static_compile_name()
{
	plib::postringstream t;
	plib::putf8_fmt_writer w(t);
	csc_private(w);
	std::hash<pstring> h;

	return plib::pfmt("nl_gcr_{1:x}_{2}")(h( t.str() ))(mat.nz_num);
}

template <std::size_t m_N, std::size_t storage_N>
std::pair<pstring, pstring> matrix_solver_GCR_t<m_N, storage_N>::create_solver_code()
{
	plib::postringstream t;
	plib::putf8_fmt_writer strm(t);
	pstring name = static_compile_name();

	strm.writeline(plib::pfmt("extern \"C\" void {1}(double * __restrict m_A, double * __restrict RHS)\n")(name));
	strm.writeline("{\n");
	csc_private(strm);
	strm.writeline("}\n");
	return std::pair<pstring, pstring>(name, t.str());
}


template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_GCR_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	const std::size_t iN = this->N();

	nl_double RHS[storage_N];
	nl_double new_V[storage_N];

	mat.set_scalar(0.0);

	for (std::size_t k = 0; k < iN; k++)
	{
		terms_for_net_t *t = this->m_terms[k].get();
		nl_double gtot_t = 0.0;
		nl_double RHS_t = 0.0;

		const std::size_t term_count = t->count();
		const std::size_t railstart = t->m_railstart;
		const nl_double * const RESTRICT gt = t->gt();
		const nl_double * const RESTRICT go = t->go();
		const nl_double * const RESTRICT Idr = t->Idr();
		const nl_double * const * RESTRICT other_cur_analog = t->connected_net_V();
		const unsigned * const RESTRICT tcr = m_term_cr[k].data();

#if 1
#if (0 ||NL_USE_SSE)
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
		for (std::size_t i = 0; i < term_count; i++)
		{
			gtot_t += gt[i];
			RHS_t += Idr[i];
		}
#endif
		for (std::size_t i = railstart; i < term_count; i++)
			RHS_t += go[i] * *other_cur_analog[i];

		RHS[k] = RHS_t;

		// add diagonal element
		mat.A[mat.diag[k]] = gtot_t;

		for (std::size_t i = 0; i < railstart; i++)
			mat.A[tcr[i]] -= go[i];
	}
#else
		for (std::size_t i = 0; i < railstart; i++)
		{
			m_A[tcr[i]] -= go[i];
			gtot_t = gtot_t + gt[i];
			RHS_t = RHS_t + Idr[i];
		}

		for (std::size_t i = railstart; i < term_count; i++)
		{
			RHS_t += (Idr[i] + go[i] * *other_cur_analog[i]);
			gtot_t += gt[i];
		}

		RHS[k] = RHS_t;
		m_A[mat.diag[k]] += gtot_t;
	}
#endif
	mat.ia[iN] = static_cast<mattype>(mat.nz_num);

	/* now solve it */

	if (m_proc != nullptr)
	{
		//static_solver(m_A, RHS);
		m_proc(mat.A, RHS);
	}
	else
	{
		for (std::size_t i = 0; i < iN - 1; i++)
		{
			const auto &nzbd = this->m_terms[i]->m_nzbd;

			if (nzbd.size() > 0)
			{
				std::size_t pi = mat.diag[i];
				const nl_double f = 1.0 / mat.A[pi++];
				const std::size_t piie = mat.ia[i+1];

				for (std::size_t j : nzbd) // for (std::size_t j = i + 1; j < iN; j++)
				{
					// proceed to column i
					//__builtin_prefetch(&m_A[mat.diag[j+1]], 1);
					std::size_t pj = mat.ia[j];

					while (mat.ja[pj] < i)
						pj++;

					const nl_double f1 = - mat.A[pj++] * f;

					// subtract row i from j */
					for (std::size_t pii = pi; pii<piie; )
					{
						while (mat.ja[pj] < mat.ja[pii])
							pj++;
						mat.A[pj++] += mat.A[pii++] * f1;
					}
					RHS[j] += f1 * RHS[i];
				}
			}
		}
	}

	/* backward substitution
	 *
	 */

	/* row n-1 */
	new_V[iN - 1] = RHS[iN - 1] / mat.A[mat.diag[iN - 1]];

	for (std::size_t j = iN - 1; j-- > 0;)
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
			tmp = _mm_add_pd(tmp, _mm_mul_pd(_mm_set_pd(mat.A[pk], mat.A[pk+1]),
					_mm_set_pd(new_V[mat.ja[pk]], new_V[mat.ja[pk+1]])));
		}
		double tmpx = _mm_cvtsd_f64(tmp) + _mm_cvtsd_f64(_mm_unpackhi_pd(tmp,tmp));
		for (; pk < e; pk++)
		{
			tmpx += mat.A[pk] * new_V[mat.ja[pk]];
		}
		new_V[j] = (RHS[j] - tmpx) / mat.A[mat.diag[j]];
#else
		double tmp = 0;
		const std::size_t e = mat.ia[j+1];
		for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
		{
			tmp += mat.A[pk] * new_V[mat.ja[pk]];
		}
		new_V[j] = (RHS[j] - tmp) / mat.A[mat.diag[j]];
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

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GCR_H_ */
