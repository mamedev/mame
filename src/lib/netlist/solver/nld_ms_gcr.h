// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_gcr.h
 *
 * Gaussian elimination using compressed row format.
 *
 */

#ifndef NLD_MS_GCR_H_
#define NLD_MS_GCR_H_

#include <algorithm>

#include "../plib/pdynlib.h"
#include "mat_cr.h"
#include "nld_ms_direct.h"
#include "nld_solver.h"
#include "vector_base.h"
#include "../plib/pstream.h"

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
		, m_proc()
		{
		}

	virtual ~matrix_solver_GCR_t() override
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

	using extsolver = void (*)(double * RESTRICT m_A, double * RESTRICT RHS, double * RESTRICT V);

	pstring static_compile_name();

	const std::size_t m_dim;
	std::vector<unsigned> m_term_cr[storage_N];
	mat_cr_t<storage_N> mat;

	//extsolver m_proc;
	plib::dynproc<void, double * RESTRICT, double * RESTRICT, double * RESTRICT> m_proc;

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GCR
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
#if 0
		m_proc = this->netlist().lib().template getsym<extsolver>(symname);
		if (m_proc != nullptr)
			this->log().verbose("External static solver {1} found ...", symname);
		else
			this->log().warning("External static solver {1} not found ...", symname);
#else
		m_proc.load(this->netlist().lib(), symname);
		if (m_proc.resolved())
			this->log().warning("External static solver {1} found ...", symname);
		else
			this->log().warning("External static solver {1} not found ...", symname);
#endif
	}

}
#if 0
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

	//new_V[iN - 1] = RHS[iN - 1] / mat.A[mat.diag[iN - 1]];
	strm("\tV[{1}] = RHS[{2}] / m_A[{3}];\n", iN - 1, iN - 1, mat.diag[iN - 1]);
	for (std::size_t j = iN - 1; j-- > 0;)
	{
		strm("\tdouble tmp{1} = 0.0;\n", j);
		const std::size_t e = mat.ia[j+1];
		for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
		{
			strm("\ttmp{1} += m_A[{2}] * V[{3}];\n", j, pk, mat.ja[pk]);
		}
		strm("\tV[{1}] = (RHS[{1}] - tmp{1}) / m_A[{4}];\n", j, j, j, mat.diag[j]);
	}
}
#else
template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_GCR_t<m_N, storage_N>::csc_private(plib::putf8_fmt_writer &strm)
{
	const std::size_t iN = N();

	for (std::size_t i = 0; i < mat.nz_num; i++)
		strm("double m_A{1} = m_A[{2}];\n", i, i);

	for (std::size_t i = 0; i < iN - 1; i++)
	{
		const auto &nzbd = this->m_terms[i]->m_nzbd;

		if (nzbd.size() > 0)
		{
			std::size_t pi = mat.diag[i];

			//const nl_double f = 1.0 / m_A[pi++];
			strm("const double f{1} = 1.0 / m_A{2};\n", i, pi);
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
				strm("\tconst double f{1}_{2} = -f{3} * m_A{4};\n", i, j, i, pj);
				pj++;

				// subtract row i from j */
				for (std::size_t pii = pi; pii<piie; )
				{
					while (mat.ja[pj] < mat.ja[pii])
						pj++;
					//m_A[pj++] += m_A[pii++] * f1;
					strm("\tm_A{1} += m_A{2} * f{3}_{4};\n", pj, pii, i, j);
					pj++; pii++;
				}
				//RHS[j] += f1 * RHS[i];
				strm("\tRHS[{1}] += f{2}_{3} * RHS[{4}];\n", j, i, j, i);
			}
		}
	}

	//new_V[iN - 1] = RHS[iN - 1] / mat.A[mat.diag[iN - 1]];
	strm("\tV[{1}] = RHS[{2}] / m_A{3};\n", iN - 1, iN - 1, mat.diag[iN - 1]);
	for (std::size_t j = iN - 1; j-- > 0;)
	{
		strm("\tdouble tmp{1} = 0.0;\n", j);
		const std::size_t e = mat.ia[j+1];
		for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
		{
			strm("\ttmp{1} += m_A{2} * V[{3}];\n", j, pk, mat.ja[pk]);
		}
		strm("\tV[{1}] = (RHS[{1}] - tmp{1}) / m_A{4};\n", j, j, j, mat.diag[j]);
	}
}
#endif

template <std::size_t m_N, std::size_t storage_N>
pstring matrix_solver_GCR_t<m_N, storage_N>::static_compile_name()
{
	plib::postringstream t;
	plib::putf8_fmt_writer w(&t);
	csc_private(w);
	std::hash<pstring> h;

	return plib::pfmt("nl_gcr_{1:x}_{2}")(h( t.str() ))(mat.nz_num);
}

template <std::size_t m_N, std::size_t storage_N>
std::pair<pstring, pstring> matrix_solver_GCR_t<m_N, storage_N>::create_solver_code()
{
	plib::postringstream t;
	plib::putf8_fmt_writer strm(&t);
	pstring name = static_compile_name();

	strm.writeline(plib::pfmt("extern \"C\" void {1}(double * __restrict m_A, double * __restrict RHS, double * __restrict V)\n")(name));
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

#if 0
		for (std::size_t i = 0; i < term_count; i++)
		{
			gtot_t += gt[i];
			RHS_t += Idr[i];
		}

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
			mat.A[tcr[i]] -= go[i];

		for (std::size_t i = 0; i < railstart; i++)
		{
			gtot_t        += gt[i];
			RHS_t         += Idr[i];
		}

		for (std::size_t i = railstart; i < term_count; i++)
		{
			RHS_t += (Idr[i] + go[i] * *other_cur_analog[i]);
			gtot_t += gt[i];
		}

		RHS[k] = RHS_t;
		mat.A[mat.diag[k]] += gtot_t;
	}
#endif
	mat.ia[iN] = static_cast<mattype>(mat.nz_num);

	/* now solve it */

	//if (m_proc != nullptr)
	if (m_proc.resolved())
	{
		//static_solver(m_A, RHS);
		m_proc(&mat.A[0], &RHS[0], &new_V[0]);
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
		/* backward substitution
		 *
		 */

		/* row n-1 */
		new_V[iN - 1] = RHS[iN - 1] / mat.A[mat.diag[iN - 1]];

		for (std::size_t j = iN - 1; j-- > 0;)
		{
			//__builtin_prefetch(&new_V[j-1], 1);
			//if (j>0)__builtin_prefetch(&m_A[mat.diag[j-1]], 0);
			double tmp = 0;
			auto jdiag = mat.diag[j];
			const std::size_t e = mat.ia[j+1];
			for (std::size_t pk = jdiag + 1; pk < e; pk++)
			{
				tmp += mat.A[pk] * new_V[mat.ja[pk]];
			}
			new_V[j] = (RHS[j] - tmp) / mat.A[jdiag];
		}
	}

	this->m_stat_calculations++;

	const nl_double err = (newton_raphson ? delta(new_V) : 0.0);
	store(new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GCR_H_ */
