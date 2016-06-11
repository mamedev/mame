// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 *
 * Sherman-Morrison Solver
 *
 * Computes the updated inverse of A given that the change in A is
 *
 * A <- A + (u x v)   u,v vectors
 *
 * In this specific implementation, u is a unit vector specifying the row which
 * changed. Thus v contains the changed column.
 *
 * Than z = A⁻¹ u ,  w = transpose(A⁻¹) v , lambda = v z
 *
 * A⁻¹ <- 1.0 / (1.0 + lambda) * (z x w)
 *
 * The approach is iterative and applied for each row changed.
 *
 * The performance for a typical circuit like kidniki compared to Gaussian
 * elimination is poor:
 *
 * a) The code needs to be run for each row change.
 * b) The inverse of A typically is fully occupied.
 *
 * It may have advantages for circuits with a high number of elements and only
 * few dynamic/active components.
 *
 */

#ifndef NLD_MS_SM_H_
#define NLD_MS_SM_H_

#include <algorithm>

#include "solver/nld_solver.h"
#include "solver/nld_matrix_solver.h"
#include "solver/vector_base.h"

namespace netlist
{
	namespace devices
	{

//#define nl_ext_double _float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double nl_double

template <unsigned m_N, unsigned storage_N>
class matrix_solver_sm_t: public matrix_solver_t
{
	friend class matrix_solver_t;

public:

	matrix_solver_sm_t(netlist_t &anetlist, const pstring &name,
			const solver_parameters_t *params, const int size);

	virtual ~matrix_solver_sm_t();

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual int vsolve_non_dynamic(const bool newton_raphson) override;
	int solve_non_dynamic(const bool newton_raphson);

	inline unsigned N() const { if (m_N == 0) return m_dim; else return m_N; }

	void LE_invert();

	template <typename T>
	void LE_compute_x(T * RESTRICT x);


	template <typename T1, typename T2>
	inline nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
	template <typename T1, typename T2>
	inline nl_ext_double &W(const T1 &r, const T2 &c) { return m_W[r][c]; }
	template <typename T1, typename T2>
	inline nl_ext_double &Ainv(const T1 &r, const T2 &c) { return m_Ainv[r][c]; }
	template <typename T1>
	inline nl_ext_double &RHS(const T1 &r) { return m_RHS[r]; }


	template <typename T1, typename T2>
	inline nl_ext_double &lA(const T1 &r, const T2 &c) { return m_lA[r][c]; }
	template <typename T1, typename T2>
	inline nl_ext_double &lAinv(const T1 &r, const T2 &c) { return m_lAinv[r][c]; }

	ATTR_ALIGN nl_double m_last_RHS[storage_N]; // right hand side - contains currents

private:
	static const std::size_t m_pitch  = (((  storage_N) + 7) / 8) * 8;
	ATTR_ALIGN nl_ext_double m_A[storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_Ainv[storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_W[storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_RHS[storage_N]; // right hand side - contains currents

	ATTR_ALIGN nl_ext_double m_lA[storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_lAinv[storage_N][m_pitch];

	//ATTR_ALIGN nl_ext_double m_RHSx[storage_N];

	const unsigned m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned storage_N>
matrix_solver_sm_t<m_N, storage_N>::~matrix_solver_sm_t()
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	pfree_array(m_A);
#endif
}

template <unsigned m_N, unsigned storage_N>
void matrix_solver_sm_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	matrix_solver_t::setup_base(nets);

	save(m_last_RHS, "m_last_RHS");

	for (unsigned k = 0; k < N(); k++)
		save(RHS(k), plib::pfmt("RHS.{1}")(k));
}



template <unsigned m_N, unsigned storage_N>
void matrix_solver_sm_t<m_N, storage_N>::LE_invert()
{
	const unsigned kN = N();

	for (unsigned i = 0; i < kN; i++)
	{
		for (unsigned j = 0; j < kN; j++)
		{
			W(i,j) = lA(i,j) = A(i,j);
			Ainv(i,j) = 0.0;
		}
		Ainv(i,i) = 1.0;
	}
	/* down */
	for (unsigned i = 0; i < kN; i++)
	{
		/* FIXME: Singular matrix? */
		const nl_double f = 1.0 / W(i,i);
		const auto * RESTRICT const p = m_terms[i]->m_nzrd.data();
		const unsigned e = m_terms[i]->m_nzrd.size();

		/* Eliminate column i from row j */

		const auto * RESTRICT const pb = m_terms[i]->m_nzbd.data();
		const unsigned eb = m_terms[i]->m_nzbd.size();
		for (unsigned jb = 0; jb < eb; jb++)
		{
			const unsigned j = pb[jb];
			const nl_double f1 = - W(j,i) * f;
			if (f1 != 0.0)
			{
				for (unsigned k = 0; k < e; k++)
					W(j,p[k]) += W(i,p[k]) * f1;
				for (unsigned k = 0; k <= i; k ++)
					Ainv(j,k) += Ainv(i,k) * f1;
			}
		}
	}
	/* up */
	for (int i = kN - 1; i >= 0; i--)
	{
		/* FIXME: Singular matrix? */
		const nl_double f = 1.0 / W(i,i);
		for (int j = i - 1; j>=0; j--)
		{
			const nl_double f1 = - W(j,i) * f;
			if (f1 != 0.0)
			{
				for (unsigned k = i; k < kN; k++)
					W(j,k) += W(i,k) * f1;
				for (unsigned k = 0; k < kN; k++)
					Ainv(j,k) += Ainv(i,k) * f1;
			}
		}
		for (unsigned k = 0; k < kN; k++)
		{
			Ainv(i,k) *= f;
			lAinv(i,k) = Ainv(i,k);
		}
	}
}

template <unsigned m_N, unsigned storage_N>
template <typename T>
void matrix_solver_sm_t<m_N, storage_N>::LE_compute_x(
		T * RESTRICT x)
{
	const unsigned kN = N();

	for (unsigned i=0; i<kN; i++)
		x[i] = 0.0;

	for (unsigned k=0; k<kN; k++)
	{
		const nl_double f = RHS(k);

		for (unsigned i=0; i<kN; i++)
			x[i] += Ainv(i,k) * f;
	}
}


template <unsigned m_N, unsigned storage_N>
int matrix_solver_sm_t<m_N, storage_N>::solve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	static const bool incremental = true;
	static unsigned cnt = 0;
	const auto iN = N();

	nl_double new_V[storage_N]; // = { 0.0 };

	if (0 || ((cnt % 200) == 0))
	{
		/* complete calculation */
		this->LE_invert();
	}
	else
	{
		if (!incremental)
		{
			for (unsigned row = 0; row < iN; row ++)
				for (unsigned k = 0; k < iN; k++)
					Ainv(row,k) = lAinv(row, k);
		}
		for (unsigned row = 0; row < iN; row ++)
		{
			nl_double v[m_pitch] = {0};
			unsigned cols[m_pitch];
			unsigned colcount = 0;

			auto &nz = m_terms[row]->m_nz;
			for (auto & col : nz)
			{
				v[col] = A(row,col) - lA(row,col);
				if (incremental)
					lA(row,col) = A(row,col);
				if (v[col] != 0.0)
					cols[colcount++] = col;
			}

			if (colcount > 0)
			{
				nl_double lamba = 0.0;
				nl_double w[m_pitch] = {0};
				nl_double z[m_pitch];
				/* compute w and lamba */
				for (unsigned i = 0; i < iN; i++)
					z[i] = Ainv(i, row); /* u is row'th column */

				for (unsigned j = 0; j < colcount; j++)
					lamba += v[cols[j]] * z[cols[j]];

				for (unsigned j=0; j<colcount; j++)
				{
					auto col = cols[j];
					auto f = v[col];
					for (unsigned k = 0; k < iN; k++)
						w[k] += Ainv(col,k) * f; /* Transpose(Ainv) * v */
				}

				lamba = -1.0 / (1.0 + lamba);
				for (unsigned i=0; i<iN; i++)
				{
					const nl_double f = lamba * z[i];
					if (f != 0.0)
						for (unsigned k = 0; k < iN; k++)
							Ainv(i,k) += f * w[k];
				}
			}

		}
	}

	cnt++;

	this->LE_compute_x(new_V);

	if (newton_raphson)
	{
		nl_double err = delta(new_V);

		store(new_V);

		return (err > this->m_params.m_accuracy) ? 2 : 1;
	}
	else
	{
		store(new_V);
		return 1;
	}
}

template <unsigned m_N, unsigned storage_N>
inline int matrix_solver_sm_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_sm_t>();
	build_LE_RHS<matrix_solver_sm_t>();

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned storage_N>
matrix_solver_sm_t<m_N, storage_N>::matrix_solver_sm_t(netlist_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const int size)
: matrix_solver_t(anetlist, name, NOSORT, params)
, m_dim(size)
{
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A = palloc_array(nl_ext_double, N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
