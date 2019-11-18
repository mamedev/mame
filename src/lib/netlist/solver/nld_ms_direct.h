// license:GPL-2.0+
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

#include <algorithm>

namespace netlist
{
namespace solver
{

	template <typename FT, int SIZE>
	class matrix_solver_direct_t: public matrix_solver_ext_t<FT, SIZE>
	{
		friend class matrix_solver_t;
	public:

		using float_type = FT;

		matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size);

		void reset() override { matrix_solver_t::reset(); }

	private:

		const std::size_t m_pitch;

	protected:
		static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
		static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 0) + 7) / 8) * 8;

		unsigned vsolve_non_dynamic(const bool newton_raphson) override;
		unsigned solve_non_dynamic(const bool newton_raphson);

		void LE_solve();

		template <typename T>
		void LE_back_subst(T & x);

		PALIGNAS_VECTOROPT()
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
				const FT f = plib::reciprocal(m_A[i][i]);
				const auto &nzrd = this->m_terms[i].m_nzrd;
				const auto &nzbd = this->m_terms[i].m_nzbd;

				for (auto &j : nzbd)
				{
					const FT f1 = -f * m_A[j][i];
					for (auto &k : nzrd)
						m_A[j][k] += m_A[i][k] * f1;
					this->m_RHS[j] += this->m_RHS[i] * f1;
				}
			}
		}
		else
		{
			for (std::size_t i = 0; i < kN; i++)
			{
				// Find the row with the largest first value
				std::size_t maxrow = i;
				for (std::size_t j = i + 1; j < kN; j++)
				{
					if (plib::abs(m_A[j][i]) > plib::abs(m_A[maxrow][i]))
					//if (m_A[j][i] * m_A[j][i] > m_A[maxrow][i] * m_A[maxrow][i])
						maxrow = j;
				}

				if (maxrow != i)
				{
					// Swap the maxrow and ith row
					for (std::size_t k = 0; k < kN; k++) {
						std::swap(m_A[i][k], m_A[maxrow][k]);
					}
					std::swap(this->m_RHS[i], this->m_RHS[maxrow]);
				}
				// FIXME: Singular matrix?
				const FT f = plib::reciprocal(m_A[i][i]);

				// Eliminate column i from row j

				for (std::size_t j = i + 1; j < kN; j++)
				{
					const FT f1 = - m_A[j][i] * f;
					if (f1 != plib::constants<FT>::zero())
					{
						const FT * pi = &(m_A[i][i+1]);
						FT * pj = &(m_A[j][i+1]);
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
				FT tmp = 0;
				for (std::size_t k = j+1; k < kN; k++)
					tmp += m_A[j][k] * x[k];
				x[j] = (this->m_RHS[j] - tmp) / m_A[j][j];
			}
		}
		else
		{
			for (std::size_t j = kN; j-- > 0; )
			{
				FT tmp = 0;
				const auto &nzrd = this->m_terms[j].m_nzrd;
				const auto e = nzrd.size(); // - 1; // exclude RHS element
				for ( std::size_t k = 0; k < e; k++)
					tmp += m_A[j][nzrd[k]] * x[nzrd[k]];
				x[j] = (this->m_RHS[j] - tmp) / m_A[j][j];
			}
		}
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_direct_t<FT, SIZE>::solve_non_dynamic(const bool newton_raphson)
	{
		this->LE_solve();
		this->LE_back_subst(this->m_new_V);

		bool err(false);
		if (newton_raphson)
			err = this->check_err();
		this->store();
		return (err) ? 2 : 1;
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{
		// populate matrix
		this->clear_square_mat(m_A);
		this->fill_matrix_and_rhs();

		this->m_stat_calculations++;
		return this->solve_non_dynamic(newton_raphson);
	}

	template <typename FT, int SIZE>
	matrix_solver_direct_t<FT, SIZE>::matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
		const analog_net_t::list_t &nets,
		const solver_parameters_t *params,
		const std::size_t size)
	: matrix_solver_ext_t<FT, SIZE>(anetlist, name, nets, params, size)
	, m_pitch(m_pitch_ABS ? m_pitch_ABS : (((size + 0) + 7) / 8) * 8)
	, m_A(size, m_pitch)
	{
		this->build_mat_ptr(m_A);
	}

} // namespace solver
} // namespace netlist

#endif // NLD_MS_DIRECT_H_
