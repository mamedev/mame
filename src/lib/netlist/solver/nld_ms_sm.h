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
 * Than z = A^-1 u ,  w = transpose(A^-1) v , lambda = v z
 *
 * A^-1 <- 1.0 / (1.0 + lambda) * (z x w)
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

#include "nld_solver.h"
#include "nld_matrix_solver.h"
#include "vector_base.h"

namespace netlist
{
	namespace devices
	{
//#define nl_ext_double _float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double nl_double

template <std::size_t m_N, std::size_t storage_N>
class matrix_solver_sm_t: public matrix_solver_t
{
	friend class matrix_solver_t;

public:

	matrix_solver_sm_t(netlist_t &anetlist, const pstring &name,
			const solver_parameters_t *params, const std::size_t size);

	virtual ~matrix_solver_sm_t() override;

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual unsigned vsolve_non_dynamic(const bool newton_raphson) override;
	unsigned solve_non_dynamic(const bool newton_raphson);

	constexpr std::size_t N() const { return (m_N == 0) ? m_dim : m_N; }

	void LE_invert();

	template <typename T>
	void LE_compute_x(T * RESTRICT x);


	template <typename T1, typename T2>
	nl_ext_double &A(const T1 &r, const T2 &c) { return m_A[r][c]; }
	template <typename T1, typename T2>
	nl_ext_double &W(const T1 &r, const T2 &c) { return m_W[r][c]; }
	template <typename T1, typename T2>
	nl_ext_double &Ainv(const T1 &r, const T2 &c) { return m_Ainv[r][c]; }
	template <typename T1>
	nl_ext_double &RHS(const T1 &r) { return m_RHS[r]; }


	template <typename T1, typename T2>
	nl_ext_double &lA(const T1 &r, const T2 &c) { return m_lA[r][c]; }
	template <typename T1, typename T2>
	nl_ext_double &lAinv(const T1 &r, const T2 &c) { return m_lAinv[r][c]; }

	nl_double m_last_RHS[storage_N]; // right hand side - contains currents

private:
	static constexpr std::size_t m_pitch  = (((  storage_N) + 7) / 8) * 8;
	nl_ext_double m_A[storage_N][m_pitch];
	nl_ext_double m_Ainv[storage_N][m_pitch];
	nl_ext_double m_W[storage_N][m_pitch];
	nl_ext_double m_RHS[storage_N]; // right hand side - contains currents

	nl_ext_double m_lA[storage_N][m_pitch];
	nl_ext_double m_lAinv[storage_N][m_pitch];

	//nl_ext_double m_RHSx[storage_N];

	const std::size_t m_dim;
	std::size_t m_cnt;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <std::size_t m_N, std::size_t storage_N>
matrix_solver_sm_t<m_N, storage_N>::~matrix_solver_sm_t()
{
}

template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_sm_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	matrix_solver_t::setup_base(nets);

	netlist().save(*this, m_last_RHS, "m_last_RHS");

	for (unsigned k = 0; k < N(); k++)
		netlist().save(*this, RHS(k), plib::pfmt("RHS.{1}")(k));
}



template <std::size_t m_N, std::size_t storage_N>
void matrix_solver_sm_t<m_N, storage_N>::LE_invert()
{
	const std::size_t kN = N();

	for (std::size_t i = 0; i < kN; i++)
	{
		for (std::size_t j = 0; j < kN; j++)
		{
			W(i,j) = lA(i,j) = A(i,j);
			Ainv(i,j) = 0.0;
		}
		Ainv(i,i) = 1.0;
	}
	/* down */
	for (std::size_t i = 0; i < kN; i++)
	{
		/* FIXME: Singular matrix? */
		const nl_double f = 1.0 / W(i,i);
		const auto * RESTRICT const p = m_terms[i]->m_nzrd.data();
		const std::size_t e = m_terms[i]->m_nzrd.size();

		/* Eliminate column i from row j */

		const auto * RESTRICT const pb = m_terms[i]->m_nzbd.data();
		const std::size_t eb = m_terms[i]->m_nzbd.size();
		for (std::size_t jb = 0; jb < eb; jb++)
		{
			const unsigned j = pb[jb];
			const nl_double f1 = - W(j,i) * f;
			if (f1 != 0.0)
			{
				for (std::size_t k = 0; k < e; k++)
					W(j,p[k]) += W(i,p[k]) * f1;
				for (std::size_t k = 0; k <= i; k ++)
					Ainv(j,k) += Ainv(i,k) * f1;
			}
		}
	}
	/* up */
	for (std::size_t i = kN; i-- > 0; )
	{
		/* FIXME: Singular matrix? */
		const nl_double f = 1.0 / W(i,i);
		for (std::size_t j = i; j-- > 0; )
		{
			const nl_double f1 = - W(j,i) * f;
			if (f1 != 0.0)
			{
				for (std::size_t k = i; k < kN; k++)
					W(j,k) += W(i,k) * f1;
				for (std::size_t k = 0; k < kN; k++)
					Ainv(j,k) += Ainv(i,k) * f1;
			}
		}
		for (std::size_t k = 0; k < kN; k++)
		{
			Ainv(i,k) *= f;
			lAinv(i,k) = Ainv(i,k);
		}
	}
}

template <std::size_t m_N, std::size_t storage_N>
template <typename T>
void matrix_solver_sm_t<m_N, storage_N>::LE_compute_x(
		T * RESTRICT x)
{
	const std::size_t kN = N();

	for (std::size_t i=0; i<kN; i++)
		x[i] = 0.0;

	for (std::size_t k=0; k<kN; k++)
	{
		const nl_double f = RHS(k);

		for (std::size_t i=0; i<kN; i++)
			x[i] += Ainv(i,k) * f;
	}
}


template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_sm_t<m_N, storage_N>::solve_non_dynamic(const bool newton_raphson)
{
	static constexpr const bool incremental = true;
	const std::size_t iN = N();

	nl_double new_V[storage_N]; // = { 0.0 };

	if ((m_cnt % 50) == 0)
	{
		/* complete calculation */
		this->LE_invert();
	}
	else
	{
		if (!incremental)
		{
			for (std::size_t row = 0; row < iN; row ++)
				for (std::size_t k = 0; k < iN; k++)
					Ainv(row,k) = lAinv(row, k);
		}
		for (std::size_t row = 0; row < iN; row ++)
		{
			nl_double v[m_pitch] = {0};
			std::size_t cols[m_pitch];
			std::size_t colcount = 0;

			auto &nz = m_terms[row]->m_nz;
			for (unsigned & col : nz)
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
				for (std::size_t i = 0; i < iN; i++)
					z[i] = Ainv(i, row); /* u is row'th column */

				for (std::size_t j = 0; j < colcount; j++)
					lamba += v[cols[j]] * z[cols[j]];

				for (std::size_t j=0; j<colcount; j++)
				{
					std::size_t col = cols[j];
					nl_double f = v[col];
					for (std::size_t k = 0; k < iN; k++)
						w[k] += Ainv(col,k) * f; /* Transpose(Ainv) * v */
				}

				lamba = -1.0 / (1.0 + lamba);
				for (std::size_t i=0; i<iN; i++)
				{
					const nl_double f = lamba * z[i];
					if (f != 0.0)
						for (std::size_t k = 0; k < iN; k++)
							Ainv(i,k) += f * w[k];
				}
			}

		}
	}

	m_cnt++;

	this->LE_compute_x(new_V);

	const nl_double err = (newton_raphson ? delta(new_V) : 0.0);
	store(new_V);
	return (err > this->m_params.m_accuracy) ? 2 : 1;
}

template <std::size_t m_N, std::size_t storage_N>
unsigned matrix_solver_sm_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_sm_t>();
	build_LE_RHS<matrix_solver_sm_t>();

	for (std::size_t i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <std::size_t m_N, std::size_t storage_N>
matrix_solver_sm_t<m_N, storage_N>::matrix_solver_sm_t(netlist_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const std::size_t size)
: matrix_solver_t(anetlist, name, NOSORT, params)
, m_dim(size)
, m_cnt(0)
{
	for (std::size_t k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
