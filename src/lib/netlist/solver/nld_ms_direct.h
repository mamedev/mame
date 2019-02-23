// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include <algorithm>
#include <cmath>
#include <netlist/plib/mat_cr.h>
#include <netlist/plib/vector_ops.h>

#include "nld_matrix_solver.h"
#include "nld_solver.h"

namespace netlist
{
namespace devices
{

template <typename FT, int SIZE>
class matrix_solver_direct_t: public matrix_solver_t
{
	friend class matrix_solver_t;
public:

	using float_type = FT;

	matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size);
	matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name, const eSortType sort, const solver_parameters_t *params, const std::size_t size);

	void vsetup(analog_net_t::list_t &nets) override;
	void reset() override { matrix_solver_t::reset(); }

protected:
	unsigned vsolve_non_dynamic(const bool newton_raphson) override;
	unsigned solve_non_dynamic(const bool newton_raphson);

	constexpr std::size_t size() const { return (SIZE > 0) ? static_cast<std::size_t>(SIZE) : m_dim; }

	void LE_solve();

	template <typename T>
	void LE_back_subst(T & x);

	FT &A(std::size_t r, std::size_t c) { return m_A[r * m_pitch + c]; }
	FT &RHS(std::size_t r) { return m_A[r * m_pitch + size()]; }
	plib::parray<FT, SIZE>  m_new_V;

private:
	static constexpr const std::size_t SIZEABS = plib::parray<FT, SIZE>::SIZEABS();
	static constexpr const std::size_t m_pitch_ABS = (((SIZEABS + 1) + 7) / 8) * 8;

	const std::size_t m_dim;
	const std::size_t m_pitch;
	plib::parray<FT, SIZE * int(m_pitch_ABS)> m_A;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <typename FT, int SIZE>
void matrix_solver_direct_t<FT, SIZE>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_t::setup_base(nets);

	/* add RHS element */
	for (std::size_t k = 0; k < size(); k++)
	{
		terms_for_net_t * t = m_terms[k].get();

		if (!plib::container::contains(t->m_nzrd, static_cast<unsigned>(size())))
			t->m_nzrd.push_back(static_cast<unsigned>(size()));
	}

	// FIXME: This shouldn't be necessary ...
	for (std::size_t k = 0; k < size(); k++)
		state().save(*this, RHS(k), this->name(), plib::pfmt("RHS.{1}")(k));
}


template <typename FT, int SIZE>
void matrix_solver_direct_t<FT, SIZE>::LE_solve()
{
	const std::size_t kN = size();
	if (!m_params.m_pivot)
	{
		for (std::size_t i = 0; i < kN; i++)
		{
			/* FIXME: Singular matrix? */
			const FT f = 1.0 / A(i,i);
			const auto &nzrd = m_terms[i]->m_nzrd;
			const auto &nzbd = m_terms[i]->m_nzbd;

			for (std::size_t j : nzbd)
			{
				const FT f1 = -f * A(j, i);
				for (std::size_t k : nzrd)
					A(j, k) += A(i, k) * f1;
				//RHS(j) += RHS(i) * f1;
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
				if (A(j,i) * A(j,i) > A(maxrow,i) * A(maxrow,i))
					maxrow = j;
			}

			if (maxrow != i)
			{
				/* Swap the maxrow and ith row */
				for (std::size_t k = 0; k < kN + 1; k++) {
					std::swap(A(i,k), A(maxrow,k));
				}
				//std::swap(RHS(i), RHS(maxrow));
			}
			/* FIXME: Singular matrix? */
			const FT f = 1.0 / A(i,i);

			/* Eliminate column i from row j */

			for (std::size_t j = i + 1; j < kN; j++)
			{
				const FT f1 = - A(j,i) * f;
				if (f1 != plib::constants<FT>::zero())
				{
					const FT * pi = &A(i,i+1);
					FT * pj = &A(j,i+1);
#if 1
					plib::vec_add_mult_scalar_p(kN-i,pi,f1,pj);
#else
					vec_add_mult_scalar_p(kN-i-1,pj,f1,pi);
					//for (unsigned k = i+1; k < kN; k++)
					//  pj[k] = pj[k] + pi[k] * f1;
					//for (unsigned k = i+1; k < kN; k++)
						//A(j,k) += A(i,k) * f1;
					RHS(j) += RHS(i) * f1;
#endif
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
				tmp += A(j,k) * x[k];
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
	else
	{
		for (std::size_t j = kN; j-- > 0; )
		{
			FT tmp = 0;
			const auto &nzrd = m_terms[j]->m_nzrd;
			const auto e = nzrd.size() - 1; /* exclude RHS element */
			for ( std::size_t k = 0; k < e; k++)
				tmp += A(j, nzrd[k]) * x[nzrd[k]];
			x[j] = (RHS(j) - tmp) / A(j,j);
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
	this->build_LE_A(*this);
	this->build_LE_RHS(*this);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <typename FT, int SIZE>
matrix_solver_direct_t<FT, SIZE>::matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const std::size_t size)
: matrix_solver_t(anetlist, name, ASCENDING, params)
, m_new_V(size)
, m_dim(size)
, m_pitch(m_pitch_ABS ? m_pitch_ABS : (((m_dim + 1) + 7) / 8) * 8)
, m_A(size * m_pitch)
{
}

template <typename FT, int SIZE>
matrix_solver_direct_t<FT, SIZE>::matrix_solver_direct_t(netlist_state_t &anetlist, const pstring &name,
		const eSortType sort, const solver_parameters_t *params, const std::size_t size)
: matrix_solver_t(anetlist, name, sort, params)
, m_new_V(size)
, m_dim(size)
, m_pitch(m_pitch_ABS ? m_pitch_ABS : (((m_dim + 1) + 7) / 8) * 8)
, m_A(size * m_pitch)
{
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
