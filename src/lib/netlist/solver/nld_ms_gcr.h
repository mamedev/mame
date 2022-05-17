// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_GCR_H_
#define NLD_MS_GCR_H_

///
/// \file  nld_ms_gcr.h
///
/// Gaussian elimination using compressed row format.
///

#include "nld_matrix_solver_ext.h"
#include "nld_solver.h"
#include "plib/pdynlib.h"
#include "plib/pmatrix_cr.h"
#include "plib/pstream.h"
#include "plib/vector_ops.h"

#include <algorithm>

namespace netlist::solver
{

	template <typename FT, int SIZE>
	class matrix_solver_GCR_t: public matrix_solver_ext_t<FT, SIZE>
	{
	public:

		using mat_type = plib::pGEmatrix_cr<plib::pmatrix_cr<FT, SIZE>>;
		using base_type = matrix_solver_ext_t<FT, SIZE>;
		using fptype = typename base_type::fptype;

		matrix_solver_GCR_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver::solver_parameters_t *params, const std::size_t size)
		: matrix_solver_ext_t<FT, SIZE>(main_solver, name, nets, params, size)
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

			this->state().log().verbose("maximum fill: {1}", gr.first);
			this->state().log().verbose("Post elimination occupancy ratio: {2} Ops: {1}", gr.second,
					static_cast<fptype>(mat.nz_num) / static_cast<fptype>(iN * iN));
			this->state().log().verbose(" Pre elimination occupancy ratio: {1}",
					static_cast<fptype>(raw_elements) / static_cast<fptype>(iN * iN));

			// FIXME: Move me
			//

			if (this->state().static_solver_lib().isLoaded())
			{
				pstring symname = static_compile_name();
				m_proc.load(this->state().static_solver_lib(), symname);
				if (m_proc.resolved())
				{
					this->state().log().info("External static solver {1} found ...", symname);
				}
				else
				{
					this->state().log().warning("External static solver {1} not found ...", symname);
				}
			}
		}

		void vsolve_non_dynamic() override;

		std::pair<pstring, pstring> create_solver_code(static_compile_target target) override;

	private:

		using mat_index_type = typename plib::pmatrix_cr<FT, SIZE>::index_type;

		void generate_code(plib::putf8_fmt_writer &strm);

		pstring static_compile_name();

		mat_type mat;
		plib::dynproc<void, FT *, fptype *, fptype *, fptype *, fptype ** > m_proc;

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

		// avoid unused variable warnings
		strm("\tplib::unused_var({1});\n", "cnV");

		for (std::size_t i = 0; i < mat.nz_num; i++)
			strm("\t{1} m_A{2}(0.0);\n", fptype, i, i);

		for (std::size_t k = 0; k < iN; k++)
		{
			auto &net = this->m_terms[k];

			// FIXME: gonn, gtn and Idr - which float types should they have?

			//auto gtot_t = std::accumulate(gt, gt + term_count, plib::constants<FT>::zero());
			//*tcr_r[railstart] = static_cast<FT>(gtot_t); //mat.A[mat.diag[k]] += gtot_t;
			auto pd = this->m_mat_ptr[k][net.railstart()] - &this->mat.A[0];
#if 0
			pstring terms = plib::pfmt("m_A{1} = gt[{2}]")(pd, this->m_gtn.didx(k,0));
			for (std::size_t i=1; i < net.count(); i++)
				terms += plib::pfmt(" + gt[{1}]")(this->m_gtn.didx(k,i));

			strm("\t{1};\n", terms);
#else
			for (std::size_t i=0; i < net.count(); i++)
				strm("\tm_A{1} += gt[{2}];\n", pd, this->m_gtn.didx(k,i));
#endif
			//for (std::size_t i = 0; i < railstart; i++)
			//  *tcr_r[i]       += static_cast<FT>(go[i]);

			for (std::size_t i = 0; i < net.railstart(); i++)
			{
				auto p = this->m_mat_ptr[k][i] - &this->mat.A[0];
				strm("\tm_A{1} += go[{2}];\n", p, this->m_gonn.didx(k,i));
			}

#if 0
			//auto RHS_t(std::accumulate(Idr, Idr + term_count, plib::constants<FT>::zero()));
			terms = plib::pfmt("{1} RHS{2} = Idr[{3}]")(fptype, k, this->m_Idrn.didx(k,0));
			for (std::size_t i=1; i < net.count(); i++)
				terms += plib::pfmt(" + Idr[{1}]")(this->m_Idrn.didx(k,i));
			//for (std::size_t i = railstart; i < term_count; i++)
			//  RHS_t +=  (- go[i]) * *cnV[i];

			for (std::size_t i = net.railstart(); i < net.count(); i++)
				terms += plib::pfmt(" - go[{1}] * *cnV[{2}]")(this->m_gonn.didx(k,i), this->m_connected_net_Vn.didx(k,i));

			strm("\t{1};\n", terms);
#else
			//auto RHS_t(std::accumulate(Idr, Idr + term_count, plib::constants<FT>::zero()));
			strm("\t{1} RHS{2} = Idr[{3}];\n", fptype, k, this->m_Idrn.didx(k,0));
			for (std::size_t i=1; i < net.count(); i++)
				strm("\tRHS{1} += Idr[{2}];\n", k, this->m_Idrn.didx(k,i));
			//for (std::size_t i = railstart; i < term_count; i++)
			//  RHS_t +=  (- go[i]) * *cnV[i];

			for (std::size_t i = net.railstart(); i < net.count(); i++)
				strm("\tRHS{1} -= go[{2}] * *cnV[{3}];\n", k, this->m_gonn.didx(k,i), this->m_connected_net_Vn.didx(k,i));

#endif
		}

		for (std::size_t i = 0; i < iN - 1; i++)
		{
			//const auto &nzbd = this->m_terms[i].m_nzbd;
			const auto *nzbd = mat.nzbd(i);
			const auto nzbd_count = mat.nzbd_count(i);

			if (nzbd_count > 0)
			{
				std::size_t pi = mat.diag[i];

				//const FT f = 1.0 / m_A[pi++];
				strm("\tconst {1} f{2} = 1.0{3} / m_A{4};\n", fptype, i, fpsuffix, pi);
				pi++;
				const std::size_t piie = mat.row_idx[i+1];

				//for (auto & j : nzbd)
				for (std::size_t jj = 0; jj < nzbd_count; jj++)
				{
					std::size_t j = nzbd[jj];
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
					strm("\tRHS{1} += f{2}_{3} * RHS{4};\n", j, i, j, i);
				}
			}
		}

		//new_V[iN - 1] = RHS[iN - 1] / mat.A[mat.diag[iN - 1]];
		strm("\tV[{1}] = RHS{2} / m_A{3};\n", iN - 1, iN - 1, mat.diag[iN - 1]);
		for (std::size_t j = iN - 1; j-- > 0;)
		{
#if 1
			strm("\t{1} tmp{2} = 0.0{3};\n", fptype, j, fpsuffix);
			const std::size_t e = mat.row_idx[j+1];
			for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
			{
				strm("\ttmp{1} += m_A{2} * V[{3}];\n", j, pk, mat.col_idx[pk]);
			}
			strm("\tV[{1}] = (RHS{1} - tmp{1}) / m_A{4};\n", j, j, j, mat.diag[j]);
#else
			pstring tmp;
			const std::size_t e = mat.row_idx[j+1];
			for (std::size_t pk = mat.diag[j] + 1; pk < e; pk++)
			{
				tmp = tmp + plib::pfmt(" + m_A{2} * V[{3}]")(j, pk, mat.col_idx[pk]);
			}
			if (tmp.empty())
			{
				strm("\tV[{1}] = RHS{1} / m_A{2};\n", j, mat.diag[j]);
			}
			else
			{
				//strm("\tconst {1} tmp{2} = {3};\n", fptype, j, tmp.substr(3));
				//strm("\tV[{1}] = (RHS{1} - tmp{1}) / m_A{2};\n", j, mat.diag[j]);
				strm("\tV[{1}] = (RHS{1} - ({2})) / m_A{3};\n", j, tmp.substr(3), mat.diag[j]);
			}
#endif
		}
	}

	template <typename FT, int SIZE>
	pstring matrix_solver_GCR_t<FT, SIZE>::static_compile_name()
	{
		pstring str_floattype(fp_constants<FT>::name());
		pstring str_fptype(fp_constants<fptype>::name());
		std::stringstream t;
		t.imbue(std::locale::classic());
		plib::putf8_fmt_writer w(&t);
		generate_code(w);
		//std::hash<typename std::remove_const<std::remove_reference<decltype(t.str())>::type>::type> h;
		return plib::pfmt("nl_gcr_{1:x}_{2}_{3}_{4}")(plib::hash( t.str().c_str(), t.str().size() ))(mat.nz_num)(str_fptype)(str_floattype);
	}

	template <typename FT, int SIZE>
	std::pair<pstring, pstring> matrix_solver_GCR_t<FT, SIZE>::create_solver_code(static_compile_target target)
	{
		std::stringstream t;
		t.imbue(std::locale::classic());
		plib::putf8_fmt_writer strm(&t);
		pstring name = static_compile_name();
		pstring str_floattype(fp_constants<FT>::name());
		pstring str_fptype(fp_constants<fptype>::name());

		pstring extqual;
		if (target == CXX_EXTERNAL_C)
			extqual = "extern \"C\"";
		else if (target == CXX_STATIC)
			extqual = "static";
		strm.writeline(plib::pfmt("{1} void {2}({3} * __restrict V, "
			"const {4} * __restrict go, const {4} * __restrict gt, "
			"const {4} * __restrict Idr, const {4} * const * __restrict cnV)\n")(extqual, name, str_floattype, str_fptype));
		strm.writeline("{\n");
		generate_code(strm);
		strm.writeline("}\n");
		// some compilers (_WIN32, _WIN64, mac osx) need an explicit cast
		return { name, putf8string(t.str()) };
	}

	template <typename FT, int SIZE>
	void matrix_solver_GCR_t<FT, SIZE>::vsolve_non_dynamic()
	{
		if (m_proc.resolved())
		{
			m_proc(&this->m_new_V[0],
				this->m_gonn.data(), this->m_gtn.data(), this->m_Idrn.data(),
				this->m_connected_net_Vn.data());
		}
		else
		{
			//  clear matrix
			mat.set_scalar(plib::constants<FT>::zero());

			// populate matrix
			this->fill_matrix_and_rhs();

			// now solve it
			// parallel is slow -- very slow
			// mat.gaussian_elimination_parallel(RHS);
			mat.gaussian_elimination(this->m_RHS);
			// backward substitution
			mat.gaussian_back_substitution(this->m_new_V, this->m_RHS);
		}
	}

} // namespace netlist::solver

#endif // NLD_MS_GCR_H_
