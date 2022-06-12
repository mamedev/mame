// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLD_MS_SOR_MAT_H_
#define NLD_MS_SOR_MAT_H_

// Names
// spell-checker: words Seidel,
//

///
/// \file nld_ms_sor.h
///
/// Generic successive over relaxation solver.
///
/// For w==1 we will do the classic Gauss-Seidel approach
///

#include "nld_matrix_solver_ext.h"
#include "nld_ms_direct.h"

#include <algorithm>

namespace netlist::solver
{

	template <typename FT, int SIZE>
	class matrix_solver_SOR_mat_t: public matrix_solver_direct_t<FT, SIZE>
	{
	public:

		using float_type = FT;

		matrix_solver_SOR_mat_t(devices::nld_solver &main_solver, const pstring &name,
			const matrix_solver_t::net_list_t &nets,
			const solver_parameters_t *params, std::size_t size)
			: matrix_solver_direct_t<FT, SIZE>(main_solver, name, nets, params, size)
			, m_omega(*this, "m_omega", static_cast<float_type>(params->m_gs_sor))
			{
			}

		void upstream_solve_non_dynamic() override;

	private:
		state_var<float_type> m_omega;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver - Gauss - Seidel
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_SOR_mat_t<FT, SIZE>::upstream_solve_non_dynamic()
	{
		// The matrix based code looks a lot nicer but actually is 30% slower than
		// the optimized code which works directly on the data structures.
		// Need something like that for gaussian elimination as well.

		const std::size_t iN = this->size();

		this->clear_square_mat(this->m_A);
		this->fill_matrix_and_rhs();

		bool resched = false;

		unsigned resched_cnt = 0;

	#if 0
		static int ws_cnt = 0;
		ws_cnt++;
		if (1 && ws_cnt % 200 == 0)
		{
			// update omega
			float_type lambdaN = 0;
			float_type lambda1 = 1e9;
			for (int k = 0; k < iN; k++)
			{
		#if 0
				float_type akk = plib::abs(this->m_A[k][k]);
				if ( akk > lambdaN)
					lambdaN = akk;
				if (akk < lambda1)
					lambda1 = akk;
		#else
				float_type akk = plib::abs(this->m_A[k][k]);
				float_type s = 0.0;
				for (int i=0; i<iN; i++)
					s = s + plib::abs(this->m_A[k][i]);
				akk = s / akk - 1.0;
				if ( akk > lambdaN)
					lambdaN = akk;
				if (akk < lambda1)
					lambda1 = akk;
		#endif
			}

			//ws = 2.0 / (2.0 - lambdaN - lambda1);
			m_omega = 2.0 / (2.0 - lambda1);
		}
	#endif

		for (std::size_t k = 0; k < iN; k++)
			this->m_new_V[k] = static_cast<float_type>(this->m_terms[k].getV());

		do {
			resched = false;
			FT cerr = plib::constants<FT>::zero();

			for (std::size_t k = 0; k < iN; k++)
			{
				float_type Idrive = 0;

				const auto *p = this->m_terms[k].m_nz.data();
				const std::size_t e = this->m_terms[k].m_nz.size();

				for (std::size_t i = 0; i < e; i++)
					Idrive = Idrive + this->m_A[k][p[i]] * this->m_new_V[p[i]];

				FT w = m_omega / this->m_A[k][k];
				if (this->m_params.m_use_gabs)
				{
					FT gabs_t = plib::constants<FT>::zero();
					for (std::size_t i = 0; i < e; i++)
						if (p[i] != k)
							gabs_t = gabs_t + plib::abs(this->m_A[k][p[i]]);

					gabs_t *= plib::constants<FT>::one(); // derived by try and error
					if (gabs_t > this->m_A[k][k])
					{
						w = plib::constants<FT>::one() / (this->m_A[k][k] + gabs_t);
					}
				}

				const float_type delta = w * (this->m_RHS[k] - Idrive) ;
				cerr = std::max(cerr, plib::abs(delta));
				this->m_new_V[k] += delta;
			}

			if (cerr > static_cast<float_type>(this->m_params.m_accuracy))
			{
				resched = true;
			}
			resched_cnt++;
		} while (resched && (resched_cnt < this->m_params.m_gs_loops));

		this->m_iterative_total += resched_cnt;

		if (resched)
		{
			this->m_iterative_fail++;
			matrix_solver_direct_t<FT, SIZE>::solve_non_dynamic();
		}
	}

} // namespace netlist::solver

#endif // NLD_MS_SOR_MAT_
