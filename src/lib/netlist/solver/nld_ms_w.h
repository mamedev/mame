// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 *
 * Woodbury Solver
 *
 * Computes the updated solution of A given that the change in A is
 *
 * A <- A + (U x transpose(V))   U,V matrices
 *
 * The approach is describes in "Numerical Recipes in C", Second edition, Page 75ff
 *
 * Whilst the book proposes to invert the matrix R=(I+transpose(V)*Z) we define
 *
 *       w = transpose(V)*y
 *       a = R^-1 * w
 *
 * and consequently
 *
 *       R * a = w
 *
 * And solve for a using Gaussian elimination. This is a lot faster.
 *
 * One fact omitted in the book is the fact that actually the matrix Z which contains
 * in it's columns the solutions of
 *
 *      A * zk = uk
 *
 * for uk being unit vectors for full rank (max(k) == n) is identical to the
 * inverse of A.
 *
 * The approach performs relatively well for matrices up to n ~ 40 (kidniki using frontiers).
 * Kidniki without frontiers has n==88. Here, the average number of Newton-Raphson
 * loops increase to 20. It looks like that the approach for larger matrices
 * introduces numerical instability.
 */

#ifndef NLD_MS_W_H_
#define NLD_MS_W_H_

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
class matrix_solver_w_t: public matrix_solver_t
{
	friend class matrix_solver_t;
public:

	matrix_solver_w_t(netlist_t &anetlist, const pstring &name, const solver_parameters_t *params, const int size);

	virtual ~matrix_solver_w_t();

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

	/* access to Ainv for fixed columns over row, there store transposed */
	template <typename T1, typename T2>
	inline nl_ext_double &Ainv(const T1 &r, const T2 &c) { return m_Ainv[c][r]; }
	template <typename T1>
	inline nl_ext_double &RHS(const T1 &r) { return m_RHS[r]; }


	template <typename T1, typename T2>
	inline nl_ext_double &lA(const T1 &r, const T2 &c) { return m_lA[r][c]; }

	nl_double m_last_RHS[storage_N]; // right hand side - contains currents

private:
	static const std::size_t m_pitch  = (((  storage_N) + 7) / 8) * 8;
	nl_ext_double m_A[storage_N][m_pitch];
	nl_ext_double m_Ainv[storage_N][m_pitch];
	nl_ext_double m_W[storage_N][m_pitch];
	nl_ext_double m_RHS[storage_N]; // right hand side - contains currents

	nl_ext_double m_lA[storage_N][m_pitch];

	/* temporary */
	nl_double H[storage_N][m_pitch] ;
	unsigned rows[storage_N];
	unsigned cols[storage_N][m_pitch];
	unsigned colcount[storage_N];

	unsigned m_cnt;

	//nl_ext_double m_RHSx[storage_N];

	const unsigned m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned storage_N>
matrix_solver_w_t<m_N, storage_N>::~matrix_solver_w_t()
{
}

template <unsigned m_N, unsigned storage_N>
void matrix_solver_w_t<m_N, storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	matrix_solver_t::setup_base(nets);

	netlist().save(*this, m_last_RHS, "m_last_RHS");

	for (unsigned k = 0; k < N(); k++)
		netlist().save(*this, RHS(k), plib::pfmt("RHS.{1}")(k));
}



template <unsigned m_N, unsigned storage_N>
void matrix_solver_w_t<m_N, storage_N>::LE_invert()
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
			const auto j = pb[jb];
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
		}
	}
}

template <unsigned m_N, unsigned storage_N>
template <typename T>
void matrix_solver_w_t<m_N, storage_N>::LE_compute_x(
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
int matrix_solver_w_t<m_N, storage_N>::solve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	const auto iN = N();

	nl_double new_V[storage_N]; // = { 0.0 };

	if ((m_cnt % 100) == 0)
	{
		/* complete calculation */
		this->LE_invert();
		this->LE_compute_x(new_V);
	}
	else
	{
		/* Solve Ay = b for y */
		this->LE_compute_x(new_V);

		/* determine changed rows */

		unsigned rowcount=0;
		#define VT(r,c) (A(r,c) - lA(r,c))

		for (unsigned row = 0; row < iN; row ++)
		{
			unsigned cc=0;
			auto &nz = m_terms[row]->m_nz;
			for (auto & col : nz)
			{
				if (A(row,col) != lA(row,col))
					cols[rowcount][cc++] = col;
			}
			if (cc > 0)
			{
				colcount[rowcount] = cc;
				rows[rowcount++] = row;
			}
		}
		if (rowcount > 0)
		{
			/* construct w = transform(V) * y
			 * dim: rowcount x iN
			 * */
			nl_double w[storage_N];
			for (unsigned i = 0; i < rowcount; i++)
			{
				const unsigned r = rows[i];
				double tmp = 0.0;
				for (unsigned k = 0; k < iN; k++)
					tmp += VT(r,k) * new_V[k];
				w[i] = tmp;
			}

			for (unsigned i = 0; i < rowcount; i++)
				for (unsigned k=0; k< rowcount; k++)
					H[i][k] = 0.0;

			for (unsigned i = 0; i < rowcount; i++)
				H[i][i] = 1.0;
			/* Construct H = (I + VT*Z) */
			for (unsigned i = 0; i < rowcount; i++)
				for (unsigned k=0; k< colcount[i]; k++)
				{
					const unsigned col = cols[i][k];
					nl_double f = VT(rows[i],col);
					if (f!=0.0)
						for (unsigned j= 0; j < rowcount; j++)
							H[i][j] += f * Ainv(col,rows[j]);
				}

			/* Gaussian elimination of H */
			for (unsigned i = 0; i < rowcount; i++)
			{
				if (H[i][i] == 0.0)
					printf("%s H singular\n", this->name().cstr());
				const nl_double f = 1.0 / H[i][i];
				for (unsigned j = i+1; j < rowcount; j++)
				{
					const nl_double f1 = - f * H[j][i];

					if (f1!=0.0)
					{
						nl_double *pj = &H[j][i+1];
						const nl_double *pi = &H[i][i+1];
						for (unsigned k = 0; k < rowcount-i-1; k++)
							pj[k] += f1 * pi[k];
							//H[j][k] += f1 * H[i][k];
						w[j] += f1 * w[i];
					}
				}
			}
			/* Back substitution */
			//inv(H) w = t     w = H t
			nl_double t[storage_N];  // FIXME: convert to member
			for (int j = rowcount - 1; j >= 0; j--)
			{
				nl_double tmp = 0;
				const nl_double *pj = &H[j][j+1];
				const nl_double *tj = &t[j+1];
				for (unsigned k = 0; k < rowcount-j-1; k++)
					tmp += pj[k] * tj[k];
					//tmp += H[j][k] * t[k];
				t[j] = (w[j] - tmp) / H[j][j];
			}

			/* x = y - Zt */
			for (unsigned i=0; i<iN; i++)
			{
				nl_double tmp = 0.0;
				for (unsigned j=0; j<rowcount;j++)
				{
					const unsigned row = rows[j];
					tmp += Ainv(i,row) * t[j];
				}
				new_V[i] -= tmp;
			}
		}
	}
	m_cnt++;

	if (0)
		for (unsigned i=0; i<iN; i++)
		{
			nl_double tmp = 0.0;
			for (unsigned j=0; j<iN; j++)
			{
				tmp += A(i,j) * new_V[j];
			}
			if (std::abs(tmp-RHS(i)) > 1e-6)
				printf("%s failed on row %d: %f RHS: %f\n", this->name().cstr(), i, std::abs(tmp-RHS(i)), RHS(i));
		}
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
inline int matrix_solver_w_t<m_N, storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	build_LE_A<matrix_solver_w_t>();
	build_LE_RHS<matrix_solver_w_t>();

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned storage_N>
matrix_solver_w_t<m_N, storage_N>::matrix_solver_w_t(netlist_t &anetlist, const pstring &name,
		const solver_parameters_t *params, const int size)
: matrix_solver_t(anetlist, name, NOSORT, params)
	,m_cnt(0)
	, m_dim(size)
{
	for (unsigned k = 0; k < N(); k++)
	{
		m_last_RHS[k] = 0.0;
	}
}

	} //namespace devices
} // namespace netlist

#endif /* NLD_MS_DIRECT_H_ */
