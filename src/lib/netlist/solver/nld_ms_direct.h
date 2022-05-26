// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

///
/// \file nld_ms_direct.h
///

#include "nld_matrix_solver.h"
#include "nld_solver.h"
#include "plib/parray.h"
#include "plib/vector_ops.h"

#include "nld_matrix_solver_ext.h"

#include <algorithm>

namespace netlist::solver
{

	template <typename FT, int SIZE>
	class matrix_solver_direct_t: public matrix_solver_ext_t<FT, SIZE>
	{
	public:

		using float_type = FT;

		matrix_solver_direct_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver::solver_parameters_t *params, std::size_t size);

		void reset() override { matrix_solver_t::reset(); }

	private:

		const std::size_t m_pitch;

	protected:
		static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
		static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 0) + 7) / 8) * 8;

		void vsolve_non_dynamic() override;
		void solve_non_dynamic();

		void LE_solve();

		template <typename T>
		void LE_back_subst(T & x);

		// PALIGNAS_VECTOROPT() `parray` defines alignment already
		plib::parray2D<FT, SIZE, m_pitch_ABS> m_A;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver_direct
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_direct_t<FT, SIZE>::LE_solve()
	{
		const std::size_t kN = this->size();
		if (!this->m_params.m_pivot)
		{
			for (std::size_t i = 0; i < kN; i++)
			{
				// FIXME: Singular matrix?
				const auto &Ai = m_A[i];
				const FT f = plib::reciprocal(Ai[i]);
				const auto &nzrd = this->m_terms[i].m_nzrd;
				const auto &nzbd = this->m_terms[i].m_nzbd;

				for (auto &j : nzbd)
				{
					auto &Aj = m_A[j];
					const FT f1 = -f * Aj[i];
					for (auto &k : nzrd)
						Aj[k] += Ai[k] * f1;
					this->m_RHS[j] += this->m_RHS[i] * f1;
				}
			}
		}
		else
		{
			for (std::size_t i = 0; i < kN; i++)
			{
				// Find the row with the largest first value
				std::size_t max_row = i;
				for (std::size_t j = i + 1; j < kN; j++)
				{
					if (plib::abs(m_A[j][i]) > plib::abs(m_A[max_row][i]))
					//#if (m_A[j][i] * m_A[j][i] > m_A[max_row][i] * m_A[max_row][i])
						max_row = j;
				}

				if (max_row != i)
				{
#if 0
					// Swap the max_row and ith row
					for (std::size_t k = 0; k < kN; k++) {
						std::swap(m_A[i][k], m_A[max_row][k]);
					}
#else
						std::swap(m_A[i], m_A[max_row]);
#endif
					std::swap(this->m_RHS[i], this->m_RHS[max_row]);
				}
				// FIXME: Singular matrix?
				const auto &Ai = m_A[i];
				const FT f = plib::reciprocal(Ai[i]);

				// Eliminate column i from row j

				for (std::size_t j = i + 1; j < kN; j++)
				{
					auto &Aj = m_A[j];
					const FT f1 = - m_A[j][i] * f;
					if (f1 != plib::constants<FT>::zero())
					{
						const FT * pi = &(Ai[i+1]);
						FT * pj = &(Aj[i+1]);
						plib::vec_add_mult_scalar_p(kN-i-1,pj,pi,f1);
						//for (unsigned k = i+1; k < kN; k++)
						//  pj[k] = pj[k] + pi[k] * f1;
						//for (unsigned k = i+1; k < kN; k++)
							//A(j,k) += A(i,k) * f1;
						this->m_RHS[j] += this->m_RHS[i] * f1;
					}
				}
			}
		}
	}

	template <typename FT, int SIZE>
	template <typename T>
	void matrix_solver_direct_t<FT, SIZE>::LE_back_subst(
			T & x)
	{
		const std::size_t kN = this->size();

		// back substitution
		if (this->m_params.m_pivot)
		{
			for (std::size_t j = kN; j-- > 0; )
			{
				FT tmp(0);
				const auto & Aj(m_A[j]);

				for (std::size_t k = j+1; k < kN; k++)
					tmp += Aj[k] * x[k];
				x[j] = (this->m_RHS[j] - tmp) / Aj[j];
			}
		}
		else
		{
			for (std::size_t j = kN; j-- > 0; )
			{
				FT tmp(0);
				const auto &nzrd = this->m_terms[j].m_nzrd;
				const auto & Aj(m_A[j]);
				const auto e = nzrd.size();

				for ( std::size_t k = 0; k < e; k++)
					tmp += Aj[nzrd[k]] * x[nzrd[k]];
				x[j] = (this->m_RHS[j] - tmp) / Aj[j];
			}
		}
	}

	template <typename FT, int SIZE>
	void matrix_solver_direct_t<FT, SIZE>::solve_non_dynamic()
	{
		this->LE_solve();
		this->LE_back_subst(this->m_new_V);
	}

	template <typename FT, int SIZE>
	void matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic()
	{
		// populate matrix
		this->clear_square_mat(m_A);
		this->fill_matrix_and_rhs();

		this->solve_non_dynamic();
	}

	template <typename FT, int SIZE>
	matrix_solver_direct_t<FT, SIZE>::matrix_solver_direct_t(devices::nld_solver &main_solver, const pstring &name,
		const matrix_solver_t::net_list_t &nets,
		const solver::solver_parameters_t *params,
		std::size_t size)
	: matrix_solver_ext_t<FT, SIZE>(main_solver, name, nets, params, size)
	, m_pitch(m_pitch_ABS ? m_pitch_ABS : (((size + 0) + 7) / 8) * 8)
	, m_A(size, m_pitch)
	{
		this->build_mat_ptr(m_A);
	}

} // namespace netlist::solver

#endif // NLD_MS_DIRECT_H_
