// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include <algorithm>

#include "nld_solver.h"
#include "nld_matrix_solver.h"
#include "vector_base.h"

/* Disabling dynamic allocation gives a ~10% boost in performance
 * This flag has been added to support continuous storage for arrays
 * going forward in case we implement cuda solvers in the future.
 */
#define NL_USE_DYNAMIC_ALLOCATION (1)

namespace netlist
{
	namespace devices
	{
//#define nl_ext_double _float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double nl_double


template <std::size_t m_N, std::size_t storage_N>
class matrix_solver_direct_t: public matrix_solver_t
{
	friend class matrix_solver_t;
public:

	matrix_solver_direct_t(netlist_t &anetlist, const pstring &name, const solver_parameters_t *params, const std::size_t size);
	matrix_solver_direct_t(netlist_t &anetlist, const pstring &name, const eSortType sort, const solver_parameters_t *params, const std::size_t size);

	virtual ~matrix_solver_direct_t() override;

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;
	unsigned solve_non_dynamic(const bool newton_raphson);

	constexpr std::size_t N() const { return (m_N == 0) ? m_dim : m_N; }

	void LE_solve();

	template <typename T>
	void LE_back_subst(T * RESTRICT x);

#if (NL_USE_DYNAMIC_ALLOCATION)
	template <typename T1, typename T2>
	nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r * m_pitch + c]; }
	template <typename T1>
	nl_ext_double &RHS(const T1 &r) { return m_A[r * m_pitch + N()]; }
#else
	template <typename T1, typename T2>
	nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
	template <typename T1>
	nl_ext_double &RHS(const T1 &r) { return m_A[r][N()]; }
#endif
	nl_double m_last_RHS[storage_N]; // right hand side - contains currents

private:
	//static const std::size_t m_pitch = (((storage_N + 1) + 0) / 1) * 1;
	static constexpr std::size_t m_pitch = (((storage_N + 1) + 7) / 8) * 8;
	//static const std::size_t m_pitch = (((storage_N + 1) + 15) / 16) * 16;
	//static const std::size_t m_pitch = (((storage_N + 1) + 31) / 32) * 32;
#if (NL_USE_DYNAMIC_ALLOCATION)
	//nl_ext_double * RESTRICT m_A;
	std::vector<nl_ext_double> m_A;
#else
	nl_ext_double m_A[storage_N][m_pitch];
#endif
	//nl_ext_double m_RHSx[storage_N];

	const std::size_t m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <std::size_t m_N, std::size_t storage_N>
matrix_solver_direct_t<m_N, storage_N>::~matrix_solver_direct_t()
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	//plib::pfree_array(m_A);
#endif
}

template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_direct_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_t::setup_base(nets);

	/* add RHS element */
	for (std::size_t k = 0; k < N(); k++)
	{
		terms_for_net_t * t = m_terms[k].get();

		if (!plib::container::contains(t->m_nzrd, static_cast<unsigned>(N())))
			t->m_nzrd.push_back(static_cast<unsigned>(N()));
	}

	netlist().save(*this, m_last_RHS, "m_last_RHS");

	for (std::size_t k = 0; k < N(); k++)
		netlist().save(*this, RHS(k), plib::pfmt("RHS.{1}")(k));
}


template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_direct_t<m_N, storage_N>::LE_solve()
{
	const std::size_t kN = N();
	if (!m_params.m_pivot)
	{
		for (std::size_t i = 0; i < kN; i++)
		{

			/* FIXME: Singular matrix? */
			nl_double *Ai = &A(i, 0);
			const nl_double f = 1.0 / A(i,i);
			const auto &nzrd = m_terms[i]->m_nzrd;
			const auto &nzbd = m_terms[i]->m_nzbd;

			for (std::size_t j : nzbd)
			{
				nl_double *Aj = &A(j, 0);
				const nl_double f1 = -f * Aj[i];
				for (std::size_t k : nzrd)
					Aj[k] += Ai[k] * f1;
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
			const nl_double f = 1.0 / A(i,i);

			/* Eliminate column i from row j */

			for (std::size_t j = i + 1; j < kN; j++)
			{
				const nl_double f1 = - A(j,i) * f;
				if (f1 != NL_FCONST(0.0))
				{
					const nl_double * RESTRICT pi = &A(i,i+1);
					nl_double * RESTRICT pj = &A(j,i+1);
#if 1
					vec_add_mult_scalar_p(kN-i,pi,f1,pj);
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

template <std::size_t m_N, std::size_t storage_N>
template <typename T>
void matrix_solver_direct_t<m_N, storage_N>::LE_back_subst(
		T * RESTRICT x)
{
	const std::size_t kN = N();

	/* back substitution */
	if (m_params.m_pivot)
	{
		for (std::size_t j = kN; j-- > 0; )
		{
			T tmp = 0;
			for (std::size_t k = j+1; k < kN; k++)
				tmp += A(j,k) * x[k];
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
	else
	{
		for (std::size_t j = kN; j-- > 0; )
		{
			T tmp = 0;

			const auto *p = m_terms[j]->m_nzrd.data();
			const auto e = m_terms[j]->m_nzrd.size() - 1; /* exclude RHS element */
			T * Aj = &A(j,0);
			for (std::size_t k = 0; k < e; k++)
			{
				const auto pk = p[k];
				tmp += Aj[pk] * x[pk];
			}
			x[j] = (RHS(j) - tmp) / A(j,j);
		}
	}
}


template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_direct_t<m_N, storage_N>::solve_non_dynamic(const bool newton_raphson)
{
	nl_double new_V[storage_N]; // = { 0.0 };

	this->LE_solve();
	this->LE_back_subst(new_V);

	const nl_double err = (newton_raphson ? delta(new_V) : 0.0);
	store(new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

template <std::size_t m_N, std::size_t storage_N>
inline unsigned matrix_solver_direct_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_direct_t>();
	build_LE_RHS<matrix_solver_direct_t>();

	for (std::size_t i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <std::size_t m_N, std::size_t storage_N>
matrix_solver_direct_t<m_N, storage_N>::matrix_solver_direct_t(netlist_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const std::size_t size)
: matrix_solver_t(anetlist, name, ASCENDING, params)
, m_dim(size)
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A.resize(N() * m_pitch);
	//m_A = plib::palloc_array<nl_ext_double>(N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
}

template <std::size_t m_N, std::size_t storage_N>
matrix_solver_direct_t<m_N, storage_N>::matrix_solver_direct_t(netlist_t &anetlist, const pstring &name,
		const eSortType sort, const solver_parameters_t *params, const std::size_t size)
: matrix_solver_t(anetlist, name, sort, params)
, m_dim(size)
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A.resize(N() * m_pitch);
	//m_A = plib::palloc_array<nl_ext_double>(N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
