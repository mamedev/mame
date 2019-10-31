// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include "nld_matrix_solver.h"
#include "nld_solver.h"
#include "plib/parray.h"
#include "plib/vector_ops.h"

#include <algorithm>
#include <cmath>

namespace netlist
{
namespace solver
{

	template <typename FT, int SIZE>
	class matrix_solver_direct_t: public matrix_solver_t
	{
		friend class matrix_solver_t;
	public:

		using float_type = FT;

		matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
			const analog_net_t::list_t &nets,
			const solver_parameters_t *params, const std::size_t size);

		void reset() override { matrix_solver_t::reset(); }

	private:

		const std::size_t m_dim;
		const std::size_t m_pitch;

	protected:
		static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
		static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 0) + 7) / 8) * 8;

		unsigned vsolve_non_dynamic(const bool newton_raphson) override;
		unsigned solve_non_dynamic(const bool newton_raphson);

		constexpr std::size_t size() const noexcept { return (SIZE > 0) ? static_cast<std::size_t>(SIZE) : m_dim; }

		void LE_solve();

		template <typename T>
		void LE_back_subst(T & x);

		PALIGNAS_VECTOROPT()
		plib::parray<FT, SIZE>  m_new_V;
		PALIGNAS_VECTOROPT()
		plib::parray2D<FT, SIZE, m_pitch_ABS> m_A;
		PALIGNAS_VECTOROPT()
		plib::parray<FT, SIZE> m_RHS;
	};

	// ----------------------------------------------------------------------------------------
	// matrix_solver_direct
	// ----------------------------------------------------------------------------------------

	template <typename FT, int SIZE>
	void matrix_solver_direct_t<FT, SIZE>::LE_solve()
	{
		const std::size_t kN = size();
		if (!m_params.m_pivot)
		{
			for (std::size_t i = 0; i < kN; i++)
			{
				/* FIXME: Singular matrix? */
				const FT f = 1.0 / m_A[i][i];
				const auto &nzrd = m_terms[i].m_nzrd;
				const auto &nzbd = m_terms[i].m_nzbd;

				for (auto &j : nzbd)
				{
					const FT f1 = -f * m_A[j][i];
					for (auto &k : nzrd)
						m_A[j][k] += m_A[i][k] * f1;
					m_RHS[j] += m_RHS[i] * f1;
				}
			}
		}
		else
		{
			for (std::size_t i = 0; i < kN; i++)
			{
				/* Find the row with the largest first value */
				std::size_t maxrow = i;
				for (std::size_t j = i + 1; j < kN; j++)
				{
					//if (std::abs(m_A[j][i]) > std::abs(m_A[maxrow][i]))
					if (m_A[j][i] * m_A[j][i] > m_A[maxrow][i] * m_A[maxrow][i])
						maxrow = j;
				}

				if (maxrow != i)
				{
					/* Swap the maxrow and ith row */
					for (std::size_t k = 0; k < kN + 1; k++) {
						std::swap(m_A[i][k], m_A[maxrow][k]);
					}
					//std::swap(RHS(i), RHS(maxrow));
				}
				/* FIXME: Singular matrix? */
				const FT f = 1.0 / m_A[i][i];

				/* Eliminate column i from row j */

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
						m_RHS[j] += m_RHS[i] * f1;
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
		const std::size_t kN = size();

		/* back substitution */
		if (m_params.m_pivot)
		{
			for (std::size_t j = kN; j-- > 0; )
			{
				FT tmp = 0;
				for (std::size_t k = j+1; k < kN; k++)
					tmp += m_A[j][k] * x[k];
				x[j] = (m_RHS[j] - tmp) / m_A[j][j];
			}
		}
		else
		{
			for (std::size_t j = kN; j-- > 0; )
			{
				FT tmp = 0;
				const auto &nzrd = m_terms[j].m_nzrd;
				const auto e = nzrd.size(); // - 1; /* exclude RHS element */
				for ( std::size_t k = 0; k < e; k++)
					tmp += m_A[j][nzrd[k]] * x[nzrd[k]];
				x[j] = (m_RHS[j] - tmp) / m_A[j][j];
			}
		}
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_direct_t<FT, SIZE>::solve_non_dynamic(const bool newton_raphson)
	{
		this->LE_solve();
		this->LE_back_subst(m_new_V);

		const FT err = (newton_raphson ? delta(m_new_V) : 0.0);
		store(m_new_V);
		return (err > this->m_params.m_accuracy) ? 2 : 1;
	}

	template <typename FT, int SIZE>
	unsigned matrix_solver_direct_t<FT, SIZE>::vsolve_non_dynamic(const bool newton_raphson)
	{
		const std::size_t iN = this->size();

		/* populate matrix */

		this->clear_square_mat(iN, m_A);
		this->fill_matrix(iN, m_RHS);

		this->m_stat_calculations++;
		return this->solve_non_dynamic(newton_raphson);
	}

	template <typename FT, int SIZE>
	matrix_solver_direct_t<FT, SIZE>::matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
		const analog_net_t::list_t &nets,
		const solver_parameters_t *params,
		const std::size_t size)
	: matrix_solver_t(anetlist, name, nets, params)
	, m_dim(size)
	, m_pitch(m_pitch_ABS ? m_pitch_ABS : (((m_dim + 0) + 7) / 8) * 8)
	, m_new_V(size)
	, m_A(size, m_pitch)
	, m_RHS(size)
	{
		this->build_mat_ptr(size, m_A);
	}

} // namespace solver
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
