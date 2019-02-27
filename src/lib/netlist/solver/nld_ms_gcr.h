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

#include "plib/mat_cr.h"

#include "nld_ms_direct.h"
#include "nld_solver.h"
#include "plib/pdynlib.h"
#include "plib/pstream.h"
#include "plib/vector_ops.h"

#include <algorithm>

namespace netlist
{
namespace devices
{

template <typename FT, int SIZE>
class matrix_solver_GCR_t: public matrix_solver_t
{
public:

	using mat_type = plib::matrix_compressed_rows_t<FT, SIZE>;
	// FIXME: dirty hack to make this compile
	static constexpr const std::size_t storage_N = 100;

	matrix_solver_GCR_t(netlist_state_t &anetlist, const pstring &name,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_t(anetlist, name, matrix_solver_t::PREFER_IDENTITY_TOP_LEFT, params)
		, m_dim(size)
		, RHS(size)
		, new_V(size)
		, mat(static_cast<typename mat_type::index_type>(size))
		, m_proc()
		{
		}

	constexpr std::size_t N() const { return m_dim; }

	void vsetup(analog_net_t::list_t &nets) override;
	unsigned vsolve_non_dynamic(const bool newton_raphson) override;

	std::pair<pstring, pstring> create_solver_code() override;

private:

	using mat_index_type = typename plib::matrix_compressed_rows_t<FT, SIZE>::index_type;

	void csc_private(plib::putf8_fmt_writer &strm);

	using extsolver = void (*)(double * m_A, double * RHS, double * V);

	pstring static_compile_name();

	const std::size_t m_dim;
	plib::parray<FT, SIZE> RHS;
	plib::parray<FT, SIZE> new_V;

	mat_type mat;

	//extsolver m_proc;
	plib::dynproc<void, double * , double * , double * > m_proc;

};

// ----------------------------------------------------------------------------------------
// matrix_solver - GCR
// ----------------------------------------------------------------------------------------

// FIXME: namespace or static class member
template <typename V>
std::size_t inline get_level(const V &v, std::size_t k)
{
	for (std::size_t i = 0; i < v.size(); i++)
		if (plib::container::contains(v[i], k))
			return i;
	throw plib::pexception("Error in get_level");
}

template <typename FT, int SIZE>
void matrix_solver_GCR_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
{
	setup_base(nets);

	const std::size_t iN = this->N();

	/* build the final matrix */

	std::vector<std::vector<unsigned>> fill(iN);

	std::size_t raw_elements = 0;

	for (std::size_t k = 0; k < iN; k++)
	{
		fill[k].resize(iN, decltype(mat)::FILL_INFINITY);
		for (auto &j : this->m_terms[k]->m_nz)
		{
			fill[k][j] = 0;
			raw_elements++;
		}

	}

	auto gr = mat.gaussian_extend_fill_mat(fill);

	/* FIXME: move this to the cr matrix class and use computed
	 * parallel ordering once it makes sense.
	 */

	std::vector<unsigned> levL(iN, 0);
	std::vector<unsigned> levU(iN, 0);

	// parallel scheme for L x = y
	for (std::size_t k = 0; k < iN; k++)
	{
		unsigned lm=0;
		for (std::size_t j = 0; j<k; j++)
			if (fill[k][j] < decltype(mat)::FILL_INFINITY)
				lm = std::max(lm, levL[j]);
		levL[k] = 1+lm;
	}

	// parallel scheme for U x = y
	for (std::size_t k = iN; k-- > 0; )
	{
		unsigned lm=0;
		for (std::size_t j = iN; --j > k; )
			if (fill[k][j] < decltype(mat)::FILL_INFINITY)
				lm = std::max(lm, levU[j]);
		levU[k] = 1+lm;
	}


	for (std::size_t k = 0; k < iN; k++)
	{
		unsigned fm = 0;
		pstring ml = "";
		for (std::size_t j = 0; j < iN; j++)
		{
			ml += fill[k][j] == 0 ? "X" : fill[k][j] < decltype(mat)::FILL_INFINITY ? "+" : ".";
			if (fill[k][j] < decltype(mat)::FILL_INFINITY)
				if (fill[k][j] > fm)
					fm = fill[k][j];
		}
		this->log().verbose("{1:4} {2} {3:4} {4:4} {5:4} {6:4}", k, ml, levL[k], levU[k], get_level(mat.m_ge_par, k), fm);
	}


	mat.build_from_fill_mat(fill);

	for (mat_index_type k=0; k<iN; k++)
	{
		std::size_t cnt(0);
		/* build pointers into the compressed row format matrix for each terminal */
		for (std::size_t j=0; j< this->m_terms[k]->m_railstart;j++)
		{
			int other = this->m_terms[k]->m_connected_net_idx[j];
			for (auto i = mat.row_idx[k]; i <  mat.row_idx[k+1]; i++)
				if (other == static_cast<int>(mat.col_idx[i]))
				{
					m_mat_ptr[k][j] = &mat.A[i];
					cnt++;
					break;
				}
		}
		nl_assert(cnt == this->m_terms[k]->m_railstart);
		m_mat_ptr[k][this->m_terms[k]->m_railstart] = &mat.A[mat.diag[k]];
	}

	this->log().verbose("maximum fill: {1}", gr.first);
	this->log().verbose("Post elimination occupancy ratio: {2} Ops: {1}", gr.second,
			static_cast<double>(mat.nz_num) / static_cast<double>(iN * iN));
	this->log().verbose(" Pre elimination occupancy ratio: {2}",
			static_cast<double>(raw_elements) / static_cast<double>(iN * iN));

	// FIXME: Move me

	if (state().lib().isLoaded())
	{
		pstring symname = static_compile_name();
		m_proc.load(this->state().lib(), symname);
		if (m_proc.resolved())
			this->log().warning("External static solver {1} found ...", symname);
		else
			this->log().warning("External static solver {1} not found ...", symname);
	}

}

template <typename FT, int SIZE>
void matrix_solver_GCR_t<FT, SIZE>::csc_private(plib::putf8_fmt_writer &strm)
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

			//const FT f = 1.0 / m_A[pi++];
			strm("const double f{1} = 1.0 / m_A{2};\n", i, pi);
			pi++;
			const std::size_t piie = mat.row_idx[i+1];

			//for (auto & j : nzbd)
			for (std::size_t j : nzbd)
			{
				// proceed to column i
				std::size_t pj = mat.row_idx[j];

				while (mat.col_idx[pj] < i)
					pj++;

				//const FT f1 = - m_A[pj++] * f;
				strm("\tconst double f{1}_{2} = -f{3} * m_A{4};\n", i, j, i, pj);
				pj++;

				// subtract row i from j */
				for (std::size_t pii = pi; pii<piie; )
				{
					while (mat.col_idx[pj] < mat.col_idx[pii])
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
		const std::size_t e = mat.row_idx[j+1];
		for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
		{
			strm("\ttmp{1} += m_A{2} * V[{3}];\n", j, pk, mat.col_idx[pk]);
		}
		strm("\tV[{1}] = (RHS[{1}] - tmp{1}) / m_A{4};\n", j, j, j, mat.diag[j]);
	}
}

template <typename FT, int SIZE>
pstring matrix_solver_GCR_t<FT, SIZE>::static_compile_name()
{
	plib::postringstream t;
	plib::putf8_fmt_writer w(&t);
	csc_private(w);
	std::hash<pstring> h;

	return plib::pfmt("nl_gcr_{1:x}_{2}")(h( t.str() ))(mat.nz_num);
}

template <typename FT, int SIZE>
std::pair<pstring, pstring> matrix_solver_GCR_t<FT, SIZE>::create_solver_code()
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


template <typename FT, int SIZE>
unsigned matrix_solver_GCR_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
{
	const std::size_t iN = this->N();

	mat.set_scalar(0.0);

	/* populate matrix */

	this->fill_matrix(iN, m_mat_ptr, RHS);

	/* now solve it */

	//if (m_proc != nullptr)
	if (m_proc.resolved())
	{
		//static_solver(m_A, RHS);
		m_proc(&mat.A[0], &RHS[0], &new_V[0]);
	}
	else
	{
		// mat.gaussian_elimination_parallel(RHS);
		mat.gaussian_elimination(RHS);
		/* backward substitution */
		mat.gaussian_back_substitution(new_V, RHS);
	}

	this->m_stat_calculations++;

	const FT err = (newton_raphson ? delta(new_V) : 0.0);
	store(new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_GCR_H_ */
