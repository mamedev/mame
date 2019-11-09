// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLD_MS_GCR_H_
#define NLD_MS_GCR_H_

///
/// \file  nld_ms_gcr.h
///
/// Gaussian elimination using compressed row format.
///

#include "plib/mat_cr.h"

#include "nld_ms_direct.h"
#include "nld_solver.h"
#include "plib/pdynlib.h"
#include "plib/pstream.h"
#include "plib/vector_ops.h"

#include <algorithm>

namespace netlist
{
namespace solver
{

	template <typename FT, int SIZE>
	class matrix_solver_GCR_t: public matrix_solver_ext_t<FT, SIZE>
	{
	public:

		using mat_type = plib::pGEmatrix_cr_t<plib::pmatrix_cr_t<FT, SIZE>>;

		matrix_solver_GCR_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size)
		: matrix_solver_ext_t<FT, SIZE>(anetlist, name, nets, params, size)
		, mat(static_cast<typename mat_type::index_type>(size))
		, m_proc()
		{
			const std::size_t iN = this->size();

			// build the final matrix

			std::vector<std::vector<unsigned>> fill(iN);

			std::size_t raw_elements = 0;

			for (std::size_t k = 0; k < iN; k++)
			{
				fill[k].resize(iN, decltype(mat)::FILL_INFINITY);
				for (auto &j : this->m_terms[k].m_nz)
				{
					fill[k][j] = 0;
					raw_elements++;
				}

			}

			auto gr = mat.gaussian_extend_fill_mat(fill);

			this->log_fill(fill, mat);

			mat.build_from_fill_mat(fill);

			for (mat_index_type k=0; k<iN; k++)
			{
				std::size_t cnt(0);
				// build pointers into the compressed row format matrix for each terminal
				for (std::size_t j=0; j< this->m_terms[k].railstart();j++)
				{
					int other = this->m_terms[k].m_connected_net_idx[j];
					for (auto i = mat.row_idx[k]; i <  mat.row_idx[k+1]; i++)
						if (other == static_cast<int>(mat.col_idx[i]))
						{
							this->m_mat_ptr[k][j] = &mat.A[i];
							cnt++;
							break;
						}
				}
				nl_assert(cnt == this->m_terms[k].railstart());
				this->m_mat_ptr[k][this->m_terms[k].railstart()] = &mat.A[mat.diag[k]];
			}

			this->log().verbose("maximum fill: {1}", gr.first);
			this->log().verbose("Post elimination occupancy ratio: {2} Ops: {1}", gr.second,
					static_cast<nl_fptype>(mat.nz_num) / static_cast<nl_fptype>(iN * iN));
			this->log().verbose(" Pre elimination occupancy ratio: {2}",
					static_cast<nl_fptype>(raw_elements) / static_cast<nl_fptype>(iN * iN));

			// FIXME: Move me

			if (this->state().lib().isLoaded())
			{
				pstring symname = static_compile_name();
				m_proc.load(this->state().lib(), symname);
				if (m_proc.resolved())
				{
					this->log().info("External static solver {1} found ...", symname);
				}
				else
				{
					this->log().warning("External static solver {1} not found ...", symname);
				}
			}
		}

		unsigned vsolve_non_dynamic(const bool newton_raphson) override;

		std::pair<pstring, pstring> create_solver_code() override;

	private:

		using mat_index_type = typename plib::pmatrix_cr_t<FT, SIZE>::index_type;

		void generate_code(plib::putf8_fmt_writer &strm);

		pstring static_compile_name();

		mat_type mat;

		plib::dynproc<void, FT * , FT * , FT * > m_proc;

	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver - GCR
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_GCR_t<FT, SIZE>::generate_code(plib::putf8_fmt_writer &strm)
	{
		const std::size_t iN = this->size();
		pstring fptype(fp_constants<FT>::name());
		pstring fpsuffix(fp_constants<FT>::suffix());

		for (std::size_t i = 0; i < mat.nz_num; i++)
			strm("{1} m_A{2} = m_A[{3}];\n", fptype, i, i);

		for (std::size_t i = 0; i < iN - 1; i++)
		{
			const auto &nzbd = this->m_terms[i].m_nzbd;

			if (nzbd.size() > 0)
			{
				std::size_t pi = mat.diag[i];

				//const FT f = 1.0 / m_A[pi++];
				strm("const {1} f{2} = 1.0{3} / m_A{4};\n", fptype, i, fpsuffix, pi);
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
					strm("\tconst {1} f{2}_{3} = -f{4} * m_A{5};\n", fptype, i, j, i, pj);
					pj++;

					// subtract row i from j
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
			strm("\t{1} tmp{2} = 0.0{3};\n", fptype, j, fpsuffix);
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
		std::stringstream t;
		t.imbue(std::locale::classic());
		plib::putf8_fmt_writer w(&t);
		generate_code(w);
		std::hash<typename std::remove_const<std::remove_reference<decltype(t.str())>::type>::type> h;
		return plib::pfmt("nl_gcr_{1:x}_{2}")(h( t.str() ))(mat.nz_num);
	}

	template <typename FT, int SIZE>
	std::pair<pstring, pstring> matrix_solver_GCR_t<FT, SIZE>::create_solver_code()
	{
		std::stringstream t;
		t.imbue(std::locale::classic());
		plib::putf8_fmt_writer strm(&t);
		pstring name = static_compile_name();
		pstring fptype(fp_constants<FT>::name());

		strm.writeline(plib::pfmt("extern \"C\" void {1}({2} * __restrict m_A, {2} * __restrict RHS, {2} * __restrict V)\n")(name, fptype));
		strm.writeline("{\n");
		generate_code(strm);
		strm.writeline("}\n");
		// some compilers (_WIN32, _WIN64, mac osx) need an explicit cast
		return std::pair<pstring, pstring>(name, pstring(t.str()));
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_GCR_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{
		// populate matrix
		mat.set_scalar(plib::constants<FT>::zero());
		this->fill_matrix_and_rhs();

		// now solve it

		if (m_proc.resolved())
		{
			m_proc(&mat.A[0], &this->m_RHS[0], &this->m_new_V[0]);
		}
		else
		{
			// parallel is slow -- very slow
			// mat.gaussian_elimination_parallel(RHS);
			mat.gaussian_elimination(this->m_RHS);
			// backward substitution
			mat.gaussian_back_substitution(this->m_new_V, this->m_RHS);
		}

		this->m_stat_calculations++;

		bool err(false);
		if (newton_raphson)
			err = this->check_err();
		this->store();
		return (err) ? 2 : 1;
	}

} // namespace solver
} // namespace netlist

#endif // NLD_MS_GCR_H_
