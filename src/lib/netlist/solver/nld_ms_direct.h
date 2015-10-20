// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include <algorithm>

#include "solver/nld_solver.h"

NETLIB_NAMESPACE_DEVICES_START()

//#define nl_ext_double __float128 // slow, very slow
//#define nl_ext_double long double // slightly slower
#define nl_ext_double double

template <unsigned m_N, unsigned _storage_N>
class matrix_solver_direct_t: public matrix_solver_t
{
public:

	matrix_solver_direct_t(const solver_parameters_t *params, const int size);
	matrix_solver_direct_t(const eSolverType type, const solver_parameters_t *params, const int size);

	virtual ~matrix_solver_direct_t();

	virtual void vsetup(analog_net_t::list_t &nets);
	virtual void reset() { matrix_solver_t::reset(); }

	ATTR_HOT inline unsigned N() const { if (m_N == 0) return m_dim; else return m_N; }

	ATTR_HOT inline int vsolve_non_dynamic(const bool newton_raphson);

protected:
	virtual void add_term(int net_idx, terminal_t *term);

	ATTR_HOT virtual nl_double vsolve();

	ATTR_HOT int solve_non_dynamic(const bool newton_raphson);
	ATTR_HOT void build_LE_A();
	ATTR_HOT void build_LE_RHS(nl_double * RESTRICT rhs);
	ATTR_HOT void LE_solve();
	ATTR_HOT void LE_back_subst(nl_double * RESTRICT x);

	/* Full LU back substitution, not used currently, in for future use */

	ATTR_HOT void LE_back_subst_full(nl_double * RESTRICT x);

	ATTR_HOT nl_double delta(const nl_double * RESTRICT V);
	ATTR_HOT void store(const nl_double * RESTRICT V);

	/* bring the whole system to the current time
	 * Don't schedule a new calculation time. The recalculation has to be
	 * triggered by the caller after the netlist element was changed.
	 */
	ATTR_HOT nl_double compute_next_timestep();

	template <typename T1, typename T2>
	inline nl_ext_double &A(const T1 r, const T2 c) { return m_A[r][c]; }

	ATTR_ALIGN nl_double m_RHS[_storage_N];
	ATTR_ALIGN nl_double m_last_RHS[_storage_N]; // right hand side - contains currents
	ATTR_ALIGN nl_double m_last_V[_storage_N];

	terms_t **m_terms;
	terms_t *m_rails_temp;

private:
	ATTR_ALIGN nl_ext_double m_A[_storage_N][((_storage_N + 7) / 8) * 8];

	const unsigned m_dim;
};

// ----------------------------------------------------------------------------------------
// matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::~matrix_solver_direct_t()
{
	for (unsigned k = 0; k < N(); k++)
	{
		pfree(m_terms[k]);
	}
	pfree_array(m_terms);
	pfree_array(m_rails_temp);
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_direct_t<m_N, _storage_N>::compute_next_timestep()
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

			const nl_double DD_n = (n->m_cur_Analog - m_last_V[k]);
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
		}
		if (new_solver_timestep < m_params.m_min_timestep)
			new_solver_timestep = m_params.m_min_timestep;
	}
	//if (new_solver_timestep > 10.0 * hn)
	//    new_solver_timestep = 10.0 * hn;
	return new_solver_timestep;
}

template <unsigned m_N, unsigned _storage_N>
ATTR_COLD void matrix_solver_direct_t<m_N, _storage_N>::add_term(int k, terminal_t *term)
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
ATTR_COLD void matrix_solver_direct_t<m_N, _storage_N>::vsetup(analog_net_t::list_t &nets)
{
	if (m_dim < nets.size())
		log().fatal("Dimension {1} less than {2}", m_dim, nets.size());

	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k]->clear();
		m_rails_temp[k].clear();
	}

	matrix_solver_t::setup(nets);

	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k]->m_railstart = m_terms[k]->count();
		for (unsigned i = 0; i < m_rails_temp[k].count(); i++)
			this->m_terms[k]->add(m_rails_temp[k].terms()[i], m_rails_temp[k].net_other()[i], false);

		m_rails_temp[k].clear(); // no longer needed
		m_terms[k]->set_pointers();
	}

#if 1

	/* Sort in descending order by number of connected matrix voltages.
	 * The idea is, that for Gauss-Seidel algo the first voltage computed
	 * depends on the greatest number of previous voltages thus taking into
	 * account the maximum amout of information.
	 *
	 * This actually improves performance on popeye slightly. Average
	 * GS computations reduce from 2.509 to 2.370
	 *
	 * Smallest to largest : 2.613
	 * Unsorted            : 2.509
	 * Largest to smallest : 2.370
	 *
	 * Sorting as a general matrix pre-conditioning is mentioned in
	 * literature but I have found no articles about Gauss Seidel.
	 *
	 * For Gaussian Elimination however increasing order is better suited.
	 * FIXME: Even better would be to sort on elements right of the matrix diagonal.
	 *
	 */

	int sort_order = (type() == GAUSS_SEIDEL ? 1 : -1);

	for (unsigned k = 0; k < N() / 2; k++)
		for (unsigned i = 0; i < N() - 1; i++)
		{
			if ((m_terms[i]->m_railstart - m_terms[i+1]->m_railstart) * sort_order < 0)
			{
				std::swap(m_terms[i],m_terms[i+1]);
				m_nets.swap(i, i+1);
			}
		}

	for (unsigned k = 0; k < N(); k++)
	{
		int *other = m_terms[k]->net_other();
		for (unsigned i = 0; i < m_terms[k]->count(); i++)
			if (other[i] != -1)
				other[i] = get_net_idx(&m_terms[k]->terms()[i]->m_otherterm->net());
	}

#endif

	/* create a list of non zero elements right of the diagonal
	 * These list anticipate the population of array elements by
	 * Gaussian elimination.
	 */
	for (unsigned k = 0; k < N(); k++)
	{
		terms_t * t = m_terms[k];
		/* pretty brutal */
		int *other = t->net_other();

		t->m_nz.clear();

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

		for (unsigned j = 0; j < N(); j++)
		{
			for (unsigned i = 0; i < t->m_railstart; i++)
			{
				if (!t->m_nzrd.contains(other[i]) && other[i] >= (int) (k + 1))
					t->m_nzrd.add(other[i]);
				if (!t->m_nz.contains(other[i]))
					t->m_nz.add(other[i]);
			}
		}
		psort_list(t->m_nzrd);

		t->m_nz.add(k);     // add diagonal
		psort_list(t->m_nz);
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
					m_terms[k]->m_nzbd.add(row);
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
	save(NLNAME(m_RHS));
	save(NLNAME(m_last_RHS));
	save(NLNAME(m_last_V));

	for (unsigned k = 0; k < N(); k++)
	{
		pstring num = pfmt("{1}")(k);

		save(m_terms[k]->go(),"GO" + num, m_terms[k]->count());
		save(m_terms[k]->gt(),"GT" + num, m_terms[k]->count());
		save(m_terms[k]->Idr(),"IDR" + num , m_terms[k]->count());
	}

}


template <unsigned m_N, unsigned _storage_N>
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::build_LE_A()
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
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::build_LE_RHS(nl_double * RESTRICT rhs)
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

		rhs[k] = rhsk_a + rhsk_b;
	}
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::LE_solve()
{
	const unsigned kN = N();

	for (unsigned i = 0; i < kN; i++) {
		// FIXME: use a parameter to enable pivoting? m_pivot
		if (m_params.m_pivot)
		{
			/* Find the row with the largest first value */
			unsigned maxrow = i;
			for (unsigned j = i + 1; j < kN; j++)
			{
				//if (std::abs(m_A[j][i]) > std::abs(m_A[maxrow][i]))
				if (A(j,i) * A(j,i) > A(maxrow,i) * A(maxrow,i))
					maxrow = j;
			}

			if (maxrow != i)
			{
				/* Swap the maxrow and ith row */
				for (unsigned k = 0; k < kN; k++) {
					std::swap(A(i,k), A(maxrow,k));
				}
				std::swap(m_RHS[i], m_RHS[maxrow]);
			}
			/* FIXME: Singular matrix? */
			const nl_double f = 1.0 / A(i,i);

			/* Eliminate column i from row j */

			for (unsigned j = i + 1; j < kN; j++)
			{
				const nl_double f1 = - A(j,i) * f;
				if (f1 != NL_FCONST(0.0))
				{
					for (unsigned k = i+1; k < kN; k++)
						A(j,k) += A(i,k) * f1;
					m_RHS[j] += m_RHS[i] * f1;
				}
			}
		}
		else
		{
			/* FIXME: Singular matrix? */
			const nl_double f = 1.0 / A(i,i);
			const unsigned * RESTRICT const p = m_terms[i]->m_nzrd.data();
			const unsigned e = m_terms[i]->m_nzrd.size();

			/* Eliminate column i from row j */

			const unsigned * RESTRICT const pb = m_terms[i]->m_nzbd.data();
			const unsigned eb = m_terms[i]->m_nzbd.size();
			for (unsigned jb = 0; jb < eb; jb++)
			{
				const unsigned j = pb[jb];
				const nl_double f1 = - A(j,i) * f;
				for (unsigned k = 0; k < e; k++)
				{
					A(j,p[k]) += A(i,p[k]) * f1;
				}
				m_RHS[j] += m_RHS[i] * f1;
			}
		}
	}
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::LE_back_subst(
		nl_double * RESTRICT x)
{
	const unsigned kN = N();

	/* back substitution */
	if (m_params.m_pivot)
	{
		for (int j = kN - 1; j >= 0; j--)
		{
			nl_double tmp = 0;
			for (unsigned k = j+1; k < kN; k++)
				tmp += A(j,k) * x[k];
			x[j] = (m_RHS[j] - tmp) / A(j,j);
		}
	}
	else
	{
		for (int j = kN - 1; j >= 0; j--)
		{
			nl_double tmp = 0;

			const unsigned *p = m_terms[j]->m_nzrd.data();
			const unsigned e = m_terms[j]->m_nzrd.size();

			for (unsigned k = 0; k < e; k++)
			{
				const unsigned pk = p[k];
				tmp += A(j,pk) * x[pk];
			}
			x[j] = (m_RHS[j] - tmp) / A(j,j);
		}
	}
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::LE_back_subst_full(
		nl_double * RESTRICT x)
{
	const unsigned kN = N();

	/* back substitution */

	// int ip;
	// ii=-1

	//for (int i=0; i < kN; i++)
	//  x[i] = m_RHS[i];

	for (int i=0; i < kN; i++)
	{
		//ip=indx[i]; USE_PIVOT_SEARCH
		//sum=b[ip];
		//b[ip]=b[i];
		double sum=m_RHS[i];//x[i];
		for (int j=0; j < i; j++)
			sum -= A(i,j) * x[j];
		x[i]=sum;
	}
	for (int i=kN-1; i >= 0; i--)
	{
		double sum=x[i];
		for (int j = i+1; j < kN; j++)
			sum -= A(i,j)*x[j];
		x[i] = sum / A(i,i);
	}

}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_direct_t<m_N, _storage_N>::delta(
		const nl_double * RESTRICT V)
{
	/* FIXME: Ideally we should also include currents (RHS) here. This would
	 * need a revaluation of the right hand side after voltages have been updated
	 * and thus belong into a different calculation. This applies to all solvers.
	 */

	const unsigned iN = this->N();
	nl_double cerr = 0;
	for (unsigned i = 0; i < iN; i++)
		cerr = std::max(cerr, nl_math::abs(V[i] - this->m_nets[i]->m_cur_Analog));
	return cerr;
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT void matrix_solver_direct_t<m_N, _storage_N>::store(
		const nl_double * RESTRICT V)
{
	for (unsigned i = 0, iN=N(); i < iN; i++)
	{
		this->m_nets[i]->m_cur_Analog = V[i];
	}
}

template <unsigned m_N, unsigned _storage_N>
ATTR_HOT nl_double matrix_solver_direct_t<m_N, _storage_N>::vsolve()
{
	this->solve_base(this);
	return this->compute_next_timestep();
}


template <unsigned m_N, unsigned _storage_N>
ATTR_HOT int matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic(ATTR_UNUSED const bool newton_raphson)
{
	nl_double new_V[_storage_N]; // = { 0.0 };

	this->LE_back_subst(new_V);

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
ATTR_HOT inline int matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic(const bool newton_raphson)
{
	this->build_LE_A();
	this->build_LE_RHS(m_last_RHS);

	for (unsigned i=0, iN=N(); i < iN; i++)
		m_RHS[i] = m_last_RHS[i];

	this->LE_solve();

	return this->solve_non_dynamic(newton_raphson);
}

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::matrix_solver_direct_t(const solver_parameters_t *params, const int size)
: matrix_solver_t(GAUSSIAN_ELIMINATION, params)
, m_dim(size)
{
	m_terms = palloc_array(terms_t *, N());
	m_rails_temp = palloc_array(terms_t, N());

	for (unsigned k = 0; k < N(); k++)
	{
		m_terms[k] = palloc(terms_t);
		m_last_RHS[k] = 0.0;
		m_last_V[k] = 0.0;
	}
}

template <unsigned m_N, unsigned _storage_N>
matrix_solver_direct_t<m_N, _storage_N>::matrix_solver_direct_t(const eSolverType type, const solver_parameters_t *params, const int size)
: matrix_solver_t(type, params)
, m_dim(size)
{
	m_terms = palloc_array(terms_t *, N());
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
