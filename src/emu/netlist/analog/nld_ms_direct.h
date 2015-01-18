/*
 * nld_ms_direct.h
 *
 */

#ifndef NLD_MS_DIRECT_H_
#define NLD_MS_DIRECT_H_

#include "nld_solver.h"

template <int m_N, int _storage_N>
class netlist_matrix_solver_direct_t: public netlist_matrix_solver_t
{
public:

	netlist_matrix_solver_direct_t(const netlist_solver_parameters_t &params, int size);
    netlist_matrix_solver_direct_t(const eSolverType type, const netlist_solver_parameters_t &params, int size);

	virtual ~netlist_matrix_solver_direct_t();

	ATTR_COLD virtual void vsetup(netlist_analog_net_t::list_t &nets);
	ATTR_COLD virtual void reset() { netlist_matrix_solver_t::reset(); }

	ATTR_HOT inline const int N() const { if (m_N == 0) return m_dim; else return m_N; }

	ATTR_HOT inline int vsolve_non_dynamic();

protected:
	ATTR_COLD virtual void add_term(int net_idx, netlist_terminal_t *term);

	ATTR_HOT virtual nl_double vsolve();

	ATTR_HOT int solve_non_dynamic();
	ATTR_HOT void build_LE();
	ATTR_HOT void gauss_LE(nl_double (* RESTRICT x));
	ATTR_HOT nl_double delta(const nl_double (* RESTRICT V));
	ATTR_HOT void store(const nl_double (* RESTRICT V), const bool store_RHS);

	/* bring the whole system to the current time
	 * Don't schedule a new calculation time. The recalculation has to be
	 * triggered by the caller after the netlist element was changed.
	 */
	ATTR_HOT nl_double compute_next_timestep();

	nl_double m_A[_storage_N][((_storage_N + 7) / 8) * 8];
	nl_double m_RHS[_storage_N];
	nl_double m_last_RHS[_storage_N]; // right hand side - contains currents
	nl_double m_Vdelta[_storage_N];
	nl_double m_last_V[_storage_N];

	terms_t **m_terms;
	terms_t *m_rails_temp;

private:
	vector_ops_t *m_row_ops[_storage_N + 1];

	int m_dim;
	nl_double m_lp_fact;
};

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver_direct
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
netlist_matrix_solver_direct_t<m_N, _storage_N>::~netlist_matrix_solver_direct_t()
{
	for (int k=0; k<_storage_N; k++)
	{
		//delete[] m_A[k];
	}
    for (int k = 0; k < N(); k++)
    {
        nl_free(m_terms[k]);
        nl_free(m_row_ops[k]);
    }
    nl_free(m_row_ops[N()]);
    //delete[] m_last_RHS;
	//delete[] m_RHS;
	nl_free_array(m_terms);
	nl_free_array(m_rails_temp);
	//delete[] m_row_ops;

}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_direct_t<m_N, _storage_N>::compute_next_timestep()
{
	nl_double new_solver_timestep = m_params.m_max_timestep;

	if (m_params.m_dynamic)
	{
		/*
		 * FIXME: We should extend the logic to use either all nets or
		 *        only output nets.
		 */
#if 0
		for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
		{
			netlist_analog_net_t *n = (*p)->m_proxied_net;
#else
		for (int k = 0; k < N(); k++)
		{
			netlist_analog_net_t *n = m_nets[k];
#endif
			const nl_double DD_n = (n->m_cur_Analog - m_last_V[k]);
			const nl_double hn = current_timestep();

			nl_double DD2 = (DD_n / hn - n->m_DD_n_m_1 / n->m_h_n_m_1) / (hn + n->m_h_n_m_1);
			nl_double new_net_timestep;

			n->m_h_n_m_1 = hn;
			n->m_DD_n_m_1 = DD_n;
			if (fabs(DD2) > 1e-50) // avoid div-by-zero
				new_net_timestep = sqrt(m_params.m_lte / fabs(0.5*DD2));
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

template <int m_N, int _storage_N>
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::add_term(int k, netlist_terminal_t *term)
{
	if (term->m_otherterm->net().isRailNet())
	{
		m_rails_temp[k].add(term, -1);
	}
	else
	{
		int ot = get_net_idx(&term->m_otherterm->net());
		if (ot>=0)
		{
			m_terms[k]->add(term, ot);
			SOLVER_VERBOSE_OUT(("Net %d Term %s %f %f\n", k, terms[i]->name().cstr(), terms[i]->m_gt, terms[i]->m_go));
		}
		/* Should this be allowed ? */
		else // if (ot<0)
		{
			m_rails_temp[k].add(term, ot);
			netlist().error("found term with missing othernet %s\n", term->name().cstr());
		}
	}
}


template <int m_N, int _storage_N>
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::vsetup(netlist_analog_net_t::list_t &nets)
{
	if (m_dim < nets.count())
		netlist().error("Dimension %d less than %d", m_dim, nets.count());

	for (int k = 0; k < N(); k++)
	{
		m_terms[k]->clear();
		m_rails_temp[k].clear();
	}

	netlist_matrix_solver_t::setup(nets);

	for (int k = 0; k < N(); k++)
	{
		m_terms[k]->m_railstart = m_terms[k]->count();
		for (int i = 0; i < m_rails_temp[k].count(); i++)
			this->m_terms[k]->add(m_rails_temp[k].terms()[i], m_rails_temp[k].net_other()[i]);

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

	for (int k = 0; k < N() / 2; k++)
		for (int i = 0; i < N() - 1; i++)
		{
            if ((m_terms[i]->m_railstart - m_terms[i+1]->m_railstart) * sort_order < 0)
			{
				std::swap(m_terms[i],m_terms[i+1]);
				m_nets.swap(i, i+1);
			}
		}

	for (int k = 0; k < N(); k++)
	{
		int *other = m_terms[k]->net_other();
		for (int i = 0; i < m_terms[k]->count(); i++)
			if (other[i] != -1)
				other[i] = get_net_idx(&m_terms[k]->terms()[i]->m_otherterm->net());
	}

#endif

}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::build_LE()
{
#if 0
	for (int k=0; k < N(); k++)
		for (int i=0; i < N(); i++)
			m_A[k][i] = 0.0;
#endif

	for (int k = 0; k < N(); k++)
	{
		for (int i=0; i < N(); i++)
			m_A[k][i] = 0.0;

		nl_double rhsk = 0.0;
		nl_double akk  = 0.0;
		{
			const int terms_count = m_terms[k]->count();
			const nl_double * RESTRICT gt = m_terms[k]->gt();
			const nl_double * RESTRICT go = m_terms[k]->go();
			const nl_double * RESTRICT Idr = m_terms[k]->Idr();
#if VECTALT

			for (int i = 0; i < terms_count; i++)
			{
				rhsk = rhsk + Idr[i];
				akk = akk + gt[i];
			}
#else
			m_terms[k]->ops()->sum2(Idr, gt, rhsk, akk);
#endif
			nl_double * const * RESTRICT other_cur_analog = m_terms[k]->other_curanalog();
			for (int i = m_terms[k]->m_railstart; i < terms_count; i++)
			{
				//rhsk = rhsk + go[i] * terms[i]->m_otherterm->net().as_analog().Q_Analog();
				rhsk = rhsk + go[i] * *other_cur_analog[i];
			}
		}
#if 0
		/*
		 * Matrix preconditioning with 1.0 / Akk
		 *
		 * will save a number of calculations during elimination
		 *
		 */
		akk = 1.0 / akk;
		m_RHS[k] = rhsk * akk;
		m_A[k][k] += 1.0;
		{
			const int *net_other = m_terms[k]->net_other();
			const nl_double *go = m_terms[k]->go();
			const int railstart =  m_terms[k]->m_railstart;

			for (int i = 0; i < railstart; i++)
			{
				m_A[k][net_other[i]] += -go[i] * akk;
			}
		}
#else
		m_RHS[k] = rhsk;
		m_A[k][k] += akk;
		{
			const int * RESTRICT net_other = m_terms[k]->net_other();
			const nl_double * RESTRICT go = m_terms[k]->go();
			const int railstart =  m_terms[k]->m_railstart;

			for (int i = 0; i < railstart; i++)
			{
				m_A[k][net_other[i]] += -go[i];
			}
		}
#endif
	}
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::gauss_LE(
		nl_double (* RESTRICT x))
{
#if 0
	for (int i = 0; i < N(); i++)
	{
		for (int k = 0; k < N(); k++)
			printf("%f ", m_A[i][k]);
		printf("| %f = %f \n", x[i], m_RHS[i]);
	}
	printf("\n");
#endif

	const int kN = N();

	for (int i = 0; i < kN; i++) {
		// FIXME: use a parameter to enable pivoting?
		if (USE_PIVOT_SEARCH)
		{
			/* Find the row with the largest first value */
			int maxrow = i;
			for (int j = i + 1; j < kN; j++)
			{
				if (fabs(m_A[j][i]) > fabs(m_A[maxrow][i]))
					maxrow = j;
			}

			if (maxrow != i)
			{
				/* Swap the maxrow and ith row */
				for (int k = i; k < kN; k++) {
					std::swap(m_A[i][k], m_A[maxrow][k]);
				}
				std::swap(m_RHS[i], m_RHS[maxrow]);
			}
		}

		/* FIXME: Singular matrix? */
		const nl_double f = 1.0 / m_A[i][i];

		/* Eliminate column i from row j */

		for (int j = i + 1; j < kN; j++)
		{
			const nl_double f1 = - m_A[j][i] * f;
			if (f1 != 0.0)
			{
#if 0 && VECTALT
				for (int k = i + 1; k < kN; k++)
					m_A[j][k] += m_A[i][k] * f1;
#else
				// addmult gives some performance increase here...
				m_row_ops[kN - (i + 1)]->addmult(&m_A[j][i+1], &m_A[i][i+1], f1) ;
#endif
				m_RHS[j] += m_RHS[i] * f1;
			}
		}
	}
	/* back substitution */
	for (int j = kN - 1; j >= 0; j--)
	{
		nl_double tmp = 0;

		for (int k = j + 1; k < kN; k++)
			tmp += m_A[j][k] * x[k];

		x[j] = (m_RHS[j] - tmp) / m_A[j][j];
	}
#if 0
	printf("Solution:\n");
	for (int i = 0; i < N(); i++)
	{
		for (int k = 0; k < N(); k++)
			printf("%f ", m_A[i][k]);
		printf("| %f = %f \n", x[i], m_RHS[i]);
	}
	printf("\n");
#endif

}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_direct_t<m_N, _storage_N>::delta(
		const nl_double (* RESTRICT V))
{
	nl_double cerr = 0;
	nl_double cerr2 = 0;
	for (int i = 0; i < this->N(); i++)
	{
		const nl_double e = (V[i] - this->m_nets[i]->m_cur_Analog);
		const nl_double e2 = (m_RHS[i] - this->m_last_RHS[i]);
		cerr = (fabs(e) > cerr ? fabs(e) : cerr);
		cerr2 = (fabs(e2) > cerr2 ? fabs(e2) : cerr2);
	}
	// FIXME: Review
	return cerr + cerr2*100000.0;
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::store(
		const nl_double (* RESTRICT V), const bool store_RHS)
{
	for (int i = 0; i < this->N(); i++)
	{
		this->m_nets[i]->m_cur_Analog = V[i];
	}
	if (store_RHS)
	{
		for (int i = 0; i < this->N(); i++)
		{
			this->m_last_RHS[i] = m_RHS[i];
		}
	}
}

template <int m_N, int _storage_N>
ATTR_HOT nl_double netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve()
{
	solve_base<netlist_matrix_solver_direct_t>(this);
	return this->compute_next_timestep();
}


template <int m_N, int _storage_N>
ATTR_HOT int netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic()
{
	nl_double new_v[_storage_N] = { 0.0 };

	this->gauss_LE(new_v);

	if (this->is_dynamic())
	{
		nl_double err = delta(new_v);

		store(new_v, true);

		if (err > this->m_params.m_accuracy)
		{
			return 2;
		}
		return 1;
	}
	store(new_v, false);  // ==> No need to store RHS
	return 1;
}

template <int m_N, int _storage_N>
ATTR_HOT inline int netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic()
{
	this->build_LE();

	return this->solve_non_dynamic();
}

template <int m_N, int _storage_N>
netlist_matrix_solver_direct_t<m_N, _storage_N>::netlist_matrix_solver_direct_t(const netlist_solver_parameters_t &params, int size)
: netlist_matrix_solver_t(GAUSSIAN_ELIMINATION, params)
, m_dim(size)
, m_lp_fact(0)
{
	m_terms = nl_alloc_array(terms_t *, N());
	m_rails_temp = nl_alloc_array(terms_t, N());

	for (int k = 0; k < N(); k++)
	{
		m_terms[k] = nl_alloc(terms_t);
		m_row_ops[k] = vector_ops_t::create_ops(k);
	}
	m_row_ops[N()] = vector_ops_t::create_ops(N());
}

template <int m_N, int _storage_N>
netlist_matrix_solver_direct_t<m_N, _storage_N>::netlist_matrix_solver_direct_t(const eSolverType type, const netlist_solver_parameters_t &params, int size)
: netlist_matrix_solver_t(type, params)
, m_dim(size)
, m_lp_fact(0)
{
    m_terms = nl_alloc_array(terms_t *, N());
    m_rails_temp = nl_alloc_array(terms_t, N());

    for (int k = 0; k < N(); k++)
    {
        m_terms[k] = nl_alloc(terms_t);
        m_row_ops[k] = vector_ops_t::create_ops(k);
    }
    m_row_ops[N()] = vector_ops_t::create_ops(N());
}


#endif /* NLD_MS_DIRECT_H_ */
