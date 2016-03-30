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
 *       a = R????? * w
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
#include "solver/vector_base.h"

NETLIB_NAMESPACE_DEVICES_START()

//#define nl_ext_double __float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double nl_double

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_w_t: public matrix_solver_t
{
public:

	matrix_solver_w_t(const solver_parameters_t *params, const int size);
	matrix_solver_w_t(const eSolverType type, const solver_parameters_t *params, const int size);

	virtual ~matrix_solver_w_t();

	virtual void vsetup(analog_net_t::list_t &nets) override;
	virtual void reset() override { matrix_solver_t::reset(); }

protected:
	virtual void add_term(int net_idx, terminal_t *term) override;
	virtual int vsolve_non_dynamic(const bool newton_raphson) override;
	int solve_non_dynamic(const bool newton_raphson);

	inline const unsigned N() const { if (m_N == 0) return m_dim; else return m_N; }

	void build_LE_A();
	void build_LE_RHS();
	void LE_invert();

	template <typename T>
	void LE_compute_x(T * RESTRICT x);

	template <typename T>
	T delta(const T * RESTRICT V);

	template <typename T>
	void store(const T * RESTRICT V);

	virtual netlist_time compute_next_timestep() override;

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

	ATTR_ALIGN nl_double m_last_RHS[_storage_N]; // right hand side - contains currents
	ATTR_ALIGN nl_double m_last_V[_storage_N];

	terms_t * m_terms[_storage_N];
	terms_t *m_rails_temp;

private:
	static const std::size_t m_pitch  = (((  _storage_N) + 7) / 8) * 8;
	ATTR_ALIGN nl_ext_double m_A[_storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_Ainv[_storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_W[_storage_N][m_pitch];
	ATTR_ALIGN nl_ext_double m_RHS[_storage_N]; // right hand side - contains currents

	ATTR_ALIGN nl_ext_double m_lA[_storage_N][m_pitch];

	/* temporary */
	ATTR_ALIGN nl_double H[_storage_N][m_pitch] ;
	unsigned rows[_storage_N];
	unsigned cols[_storage_N][m_pitch];
	unsigned colcount[_storage_N];

	unsigned m_cnt;

	//ATTR_ALIGN nl_ext_double m_RHSx[_storage_N];

	const unsigned m_dim;

};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
matrix_solver_w_t<m_N, _storage_N>::~matrix_solver_w_t()
{
	for (unsigned k = 0; k < N(); k++)
	{
		pfree(m_terms[k]);
	}
	pfree_array(m_rails_temp);
#if (NL_USE_DYNAMIC_ALLOCATION)
	pfree_array(m_A);
#endif
}

template <unsigned m_N, unsigned _storage_N>
netlist_time matrix_solver_w_t<m_N, _storage_N>::compute_next_timestep()
{
	nl_double new_solver_timestep = m_params.m_max_timestep;

	if (m_params.m_dynamic)
	{
		/*
		 * FIXME: We should extend the logic to use either all nets or
		 *        only output nets.
		 */
		for (unsigned k = 0, iN=N(); k < iN; k++)
		{
			analog_net_t *n = m_nets[k];

			const nl_double DD_n = (n->Q_Analog() - m_last_V[k]);
			const nl_double hn = current_timestep();

			nl_double DD2 = (DD_n / hn - n->m_DD_n_m_1 / n->m_h_n_m_1) / (hn + n->m_h_n_m_1);
			nl_double new_net_timestep;

			n->m_h_n_m_1 = hn;
			n->m_DD_n_m_1 = DD_n;
			if (nl_math::abs(DD2) > NL_FCONST(1e-30)) // avoid div-by-zero
				new_net_timestep = nl_math::sqrt(m_params.m_lte / nl_math::abs(NL_FCONST(0.5)*DD2));
			else
				new_net_timestep = m_params.m_max_timestep;

			if (new_net_timestep < new_solver_timestep)
				new_solver_timestep = new_net_timestep;

			m_last_V[k] = n->Q_Analog();
		}
		if (new_solver_timestep < m_params.m_min_timestep)
			new_solver_timestep = m_params.m_min_timestep;
	}
	//if (new_solver_timestep > 10.0 * hn)
	//    new_solver_timestep = 10.0 * hn;
	return netlist_time::from_double(new_solver_timestep);
}

template <unsigned m_N, unsigned _storage_N>
ATTR_COLD void matrix_solver_w_t<m_N, _storage_N>::add_term(int k, terminal_t *term)
{
	if (term->m_otherterm->net().isRailNet())
	{
		m_rails_temp[k].add(term, -1, false);
	}
	else
	{
		int ot = get_net_idx(&term->m_otherterm->net());
		if (ot>=0)
		{
			m_terms[k]->add(term, ot, true);
		}
		/* Should this be allowed ? */
		else // if (ot<0)
		{
			m_rails_temp[k].add(term, ot, true);
			log().fatal("found term with missing othernet {1}\n", term->name());
		}
	}
}


template <unsigned m_N, unsigned _storage_N>
ATTR_COLD void matrix_solver_w_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k]->clear();
		m_rails_temp[k].clear();
	}

	matrix_solver_t::setup_base(nets);

	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k]->m_railstart = m_terms[k]->count();
		for (unsigned i = 0; i < m_rails_temp[k].count(); i++)
			this->m_terms[k]->add(m_rails_temp[k].terms()[i], m_rails_temp[k].net_other()[i], false);

		m_rails_temp[k].clear(); // no longer needed
		m_terms[k]->set_pointers();
	}

	/* create a list of non zero elements. */
	for (unsigned k = 0; k < N(); k++)
	{
		terms_t * t = m_terms[k];
		/* pretty brutal */
		int *other = t->net_other();

		t->m_nz.clear();

		for (unsigned i = 0; i < t->m_railstart; i++)
			if (!t->m_nz.contains(other[i]))
				t->m_nz.push_back(other[i]);

		t->m_nz.push_back(k);     // add diagonal

		/* and sort */
		psort_list(t->m_nz);
	}

	/* create a list of non zero elements right of the diagonal
	 * These list anticipate the population of array elements by
	 * Gaussian elimination.
	 */
	for (unsigned k = 0; k < N(); k++)
	{
		terms_t * t = m_terms[k];
		/* pretty brutal */
		int *other = t->net_other();

		if (k==0)
			t->m_nzrd.clear();
		else
		{
			t->m_nzrd = m_terms[k-1]->m_nzrd;
			unsigned j=0;
			while(j < t->m_nzrd.size())
			{
				if (t->m_nzrd[j] < k + 1)
					t->m_nzrd.remove_at(j);
				else
					j++;
			}
		}

		for (unsigned i = 0; i < t->m_railstart; i++)
			if (!t->m_nzrd.contains(other[i]) && other[i] >= (int) (k + 1))
				t->m_nzrd.push_back(other[i]);

		/* and sort */
		psort_list(t->m_nzrd);
	}

	/* create a list of non zero elements below diagonal k
	 * This should reduce cache misses ...
	 */

	bool touched[_storage_N][_storage_N] = { { false } };
	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k]->m_nzbd.clear();
		for (unsigned j = 0; j < m_terms[k]->m_nz.size(); j++)
			touched[k][m_terms[k]->m_nz[j]] = true;
	}

	for (unsigned k = 0; k < N(); k++)
	{
		for (unsigned row = k + 1; row < N(); row++)
		{
			if (touched[row][k])
			{
				if (!m_terms[k]->m_nzbd.contains(row))
					m_terms[k]->m_nzbd.push_back(row);
				for (unsigned col = k; col < N(); col++)
					if (touched[k][col])
						touched[row][col] = true;
			}
		}
	}

	if (0)
		for (unsigned k = 0; k < N(); k++)
		{
			pstring line = pfmt("{1}")(k, "3");
			for (unsigned j = 0; j < m_terms[k]->m_nzrd.size(); j++)
				line += pfmt(" {1}")(m_terms[k]->m_nzrd[j], "3");
			log().verbose("{1}", line);
		}

	/*
	 * save states
	 */
	save(NLNAME(m_last_RHS));
	save(NLNAME(m_last_V));

	for (unsigned k = 0; k < N(); k++)
	{
		pstring num = pfmt("{1}")(k);

		save(RHS(k), "RHS" + num);

		save(m_terms[k]->go(),"GO" + num, m_terms[k]->count());
		save(m_terms[k]->gt(),"GT" + num, m_terms[k]->count());
		save(m_terms[k]->Idr(),"IDR" + num , m_terms[k]->count());
	}

}


template <unsigned m_N, unsigned _storage_N>
void matrix_solver_w_t<m_N, _storage_N>::build_LE_A()
{
	const unsigned iN = N();
	for (unsigned k = 0; k < iN; k++)
	{
		for (unsigned i=0; i < iN; i++)
			A(k,i) = 0.0;

		const unsigned terms_count = m_terms[k]->count();
		const unsigned railstart =  m_terms[k]->m_railstart;
		const nl_double * RESTRICT gt = m_terms[k]->gt();

		{
			nl_double akk  = 0.0;
			for (unsigned i = 0; i < terms_count; i++)
				akk += gt[i];

			A(k,k) = akk;
		}

		const nl_double * RESTRICT go = m_terms[k]->go();
		const int * RESTRICT net_other = m_terms[k]->net_other();

		for (unsigned i = 0; i < railstart; i++)
			A(k,net_other[i]) -= go[i];
	}
}

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_w_t<m_N, _storage_N>::build_LE_RHS()
{
	const unsigned iN = N();
	for (unsigned k = 0; k < iN; k++)
	{
		nl_double rhsk_a = 0.0;
		nl_double rhsk_b = 0.0;

		const int terms_count = m_terms[k]->count();
		const nl_double * RESTRICT go = m_terms[k]->go();
		const nl_double * RESTRICT Idr = m_terms[k]->Idr();
		const nl_double * const * RESTRICT other_cur_analog = m_terms[k]->other_curanalog();

		for (int i = 0; i < terms_count; i++)
			rhsk_a = rhsk_a + Idr[i];

		for (int i = m_terms[k]->m_railstart; i < terms_count; i++)
			//rhsk = rhsk + go[i] * terms[i]->m_otherterm->net().as_analog().Q_Analog();
			rhsk_b = rhsk_b + go[i] * *other_cur_analog[i];

		RHS(k) = rhsk_a + rhsk_b;
	}
}

template <unsigned m_N, unsigned _storage_N>
void matrix_solver_w_t<m_N, _storage_N>::LE_invert()
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
		const unsigned * RESTRICT const p = m_terms[i]->m_nzrd.data();
		const unsigned e = m_terms[i]->m_nzrd.size();

		/* Eliminate column i from row j */

		const unsigned * RESTRICT const pb = m_terms[i]->m_nzbd.data();
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
		}
	}
}

template <unsigned m_N, unsigned _storage_N>
template <typename T>
void matrix_solver_w_t<m_N, _storage_N>::LE_compute_x(
		T * RESTRICT x)
{
	const unsigned kN = N();

	for (int i=0; i<kN; i++)
		x[i] = 0.0;

	for (int k=0; k<kN; k++)
	{
		const nl_double f = RHS(k);

		for (int i=0; i<kN; i++)
			x[i] += Ainv(i,k) * f;
	}
}


template <unsigned m_N, unsigned _storage_N>
template <typename T>
T matrix_solver_w_t<m_N, _storage_N>::delta(
		const T * RESTRICT V)
{
	/* FIXME: Ideally we should also include currents (RHS) here. This would
	 * need a revaluation of the right hand side after voltages have been updated
	 * and thus belong into a different calculation. This applies to all solvers.
	 */

	const unsigned iN = this->N();
	T cerr = 0;
	for (unsigned i = 0; i < iN; i++)
		cerr = std::fmax(cerr, nl_math::abs(V[i] - (T) this->m_nets[i]->m_cur_Analog));
	return cerr;
}

template <unsigned m_N, unsigned _storage_N>
template <typename T>
void matrix_solver_w_t<m_N, _storage_N>::store(
		const T * RESTRICT V)
{
	for (unsigned i = 0, iN=N(); i < iN; i++)
	{
		this->m_nets[i]->m_cur_Analog = V[i];
	}
}

template <unsigned m_N, unsigned _storage_N>
int matrix_solver_w_t<m_N, _storage_N>::solve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	const auto iN = N();

	nl_double new_V[_storage_N]; // = { 0.0 };

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

		for (int row = 0; row < iN; row ++)
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
			nl_double w[_storage_N] = {0};
			for (unsigned i = 0; i < rowcount; i++)
				for (unsigned k = 0; k < iN; k++)
					w[i] += VT(rows[i],k) * new_V[k];

			for (unsigned i = 0; i < rowcount; i++)
				for (unsigned k=0; k< rowcount; k++)
					H[i][k] = 0.0;

			for (unsigned i = 0; i < rowcount; i++)
				H[i][i] += 1.0;
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
			nl_double *t = new nl_double[rowcount];
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
			delete[] t;
		}
	}
	m_cnt++;

	for (unsigned i=0; i<iN; i++)
	{
		nl_double tmp = 0.0;
		for (unsigned j=0; j<iN; j++)
		{
			tmp += A(i,j) * new_V[j];
		}
		if (std::fabs(tmp-RHS(i)) > 1e-6)
			printf("%s failed on row %d: %f RHS: %f\n", this->name().cstr(), i, std::fabs(tmp-RHS(i)), RHS(i));
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

template <unsigned m_N, unsigned _storage_N>
inline int matrix_solver_w_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	this->build_LE_A();
	this->build_LE_RHS();

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_last_RHS[i] = RHS(i);

	this->m_stat_calculations++;
	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned _storage_N>
matrix_solver_w_t<m_N, _storage_N>::matrix_solver_w_t(const solver_parameters_t *params, const int size)
: matrix_solver_t(GAUSSIAN_ELIMINATION, params)
	,m_cnt(0)
	, m_dim(size)
{
	m_rails_temp = palloc_array(terms_t, N());
#if (NL_USE_DYNAMIC_ALLOCATION)
	m_A = palloc_array(nl_ext_double, N() * m_pitch);
#endif
	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k] = palloc(terms_t);
		m_last_RHS[k] = 0.0;
		m_last_V[k] = 0.0;
	}
}

template <unsigned m_N, unsigned _storage_N>
matrix_solver_w_t<m_N, _storage_N>::matrix_solver_w_t(const eSolverType type, const solver_parameters_t *params, const int size)
: matrix_solver_t(type, params)
,m_cnt(0)
, m_dim(size)
{
	m_rails_temp = palloc_array(terms_t, N());
	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k] = palloc(terms_t);
		m_last_RHS[k] = 0.0;
		m_last_V[k] = 0.0;
	}
}

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_MS_DIRECT_H_ */
