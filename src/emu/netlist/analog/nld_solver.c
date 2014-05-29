/*
 * nld_solver.c
 *
 */

#include <algorithm>

#include "nld_solver.h"
#include "nld_twoterm.h"
#include "../nl_lists.h"

#if HAS_OPENMP
#include "omp.h"
#endif

#define USE_PIVOT_SEARCH (0)

#define SOLVER_VERBOSE_OUT(x) do {} while (0)
//#define SOLVER_VERBOSE_OUT(x) printf x

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_matrix_solver_t::netlist_matrix_solver_t()
: m_calculations(0)
{
}

ATTR_COLD netlist_matrix_solver_t::~netlist_matrix_solver_t()
{
    for (int i = 0; i < m_inps.count(); i++)
        delete m_inps[i];
}

ATTR_COLD void netlist_matrix_solver_t::setup(netlist_analog_net_t::list_t &nets)
{
	NL_VERBOSE_OUT(("New solver setup\n"));

	m_nets.clear();

	for (int k = 0; k < nets.count(); k++)
	{
        m_nets.add(nets[k]);
    }

	for (int k = 0; k < nets.count(); k++)
	{
		NL_VERBOSE_OUT(("setting up net\n"));

		netlist_analog_net_t *net = nets[k];

		net->m_solver = this;

        for (int i = 0; i < net->m_core_terms.count(); i++)
		{
		    netlist_core_terminal_t *p = net->m_core_terms[i];
			NL_VERBOSE_OUT(("%s %s %d\n", p->name().cstr(), net->name().cstr(), (int) net->isRailNet()));
			switch (p->type())
			{
				case netlist_terminal_t::TERMINAL:
					switch (p->netdev().family())
					{
						case netlist_device_t::CAPACITOR:
							if (!m_steps.contains(&p->netdev()))
								m_steps.add(&p->netdev());
							break;
						case netlist_device_t::BJT_EB:
						case netlist_device_t::DIODE:
						//case netlist_device_t::VCVS:
						case netlist_device_t::BJT_SWITCH:
							NL_VERBOSE_OUT(("found BJT/Diode\n"));
							if (!m_dynamic.contains(&p->netdev()))
								m_dynamic.add(&p->netdev());
							break;
						default:
							break;
					}
					{
						netlist_terminal_t *pterm = dynamic_cast<netlist_terminal_t *>(p);
						add_term(k, pterm);
					}
					NL_VERBOSE_OUT(("Added terminal\n"));
					break;
				case netlist_terminal_t::INPUT:
				    {
                        netlist_analog_output_t *net_proxy_output = NULL;
                        for (int i = 0; i < m_inps.count(); i++)
                            if (m_inps[i]->m_proxied_net == &p->net().as_analog())
                            {
                                net_proxy_output = m_inps[i];
                                break;
                            }

                        if (net_proxy_output == NULL)
                        {
                            net_proxy_output = new netlist_analog_output_t();
                            net_proxy_output->init_object(*this, this->name() + "." + pstring::sprintf("m%d", m_inps.count()));
                            m_inps.add(net_proxy_output);
                            net_proxy_output->m_proxied_net = &p->net().as_analog();
                        }
                        net_proxy_output->net().register_con(*p);
                        // FIXME: repeated
                        net_proxy_output->net().rebuild_list();
                        NL_VERBOSE_OUT(("Added input\n"));
				    }
                    break;
				default:
					netlist().error("unhandled element found\n");
					break;
			}
		}
		NL_VERBOSE_OUT(("added net with %d populated connections (%d railnets)\n", net->m_terms.count(), (*pn)->m_rails.count()));
	}
}

template <int m_N, int _storage_N>
ATTR_HOT double netlist_matrix_solver_direct_t<m_N, _storage_N>::compute_next_timestep(const double hn)
{
    double new_solver_timestep = m_params.m_max_timestep;

    if (m_params.m_dynamic)
    {
        /*
         * FIXME: this is a reduced LTE focusing on the nets which drive other nets
         *        The academically correct version using all nets is the one commented out
         *        This causes really bad performance due to rounding errors.
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
            double DD_n = (n->m_cur_Analog - n->m_last_Analog);

            if (fabs(DD_n) < 2.0 * m_params.m_accuracy)
                DD_n = 0.0;
            else
                DD_n = DD_n / hn;

            double h_n_m_1 = n->m_h_n_m_1;
            // limit last timestep in equation.
            //if (h_n_m_1 > 3 * hn)
            //    h_n_m_1 = 3 * hn;

            double DD2 = (DD_n - n->m_DD_n_m_1) / (hn + h_n_m_1);
            double new_net_timestep;

            n->m_DD_n_m_1 = DD_n;
            n->m_h_n_m_1 = hn;
            if (fabs(DD2) > 1e-50) // avoid div-by-zero
                new_net_timestep = sqrt(m_params.m_lte / fabs(0.5*DD2));
            else
            {
                new_net_timestep = m_params.m_max_timestep;

                //if (hn > 0.0 && new_net_timestep > 100.0 * hn)
                //    new_net_timestep = 100.0 * hn;
            }
            //if (N()==2)
            //    printf("%s: k %d ts %e DD2 %e\n", name().cstr(), k, new_net_timestep, DD2);

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

ATTR_HOT void netlist_matrix_solver_t::update_inputs()
{
    // avoid recursive calls. Inputs are updated outside this call
    for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
        if ((*p)->m_proxied_net->m_last_Analog != (*p)->m_proxied_net->m_cur_Analog)
            (*p)->set_Q((*p)->m_proxied_net->m_cur_Analog);
#if 1
    for (int k = 0; k < m_nets.count(); k++)
    {
        netlist_analog_net_t *p= m_nets[k];
        p->m_last_Analog = p->m_cur_Analog;
    }
#else
        for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
        {
            if ((*p)->m_proxied_net->m_last_Analog != (*p)->m_proxied_net->m_cur_Analog)
                (*p)->m_proxied_net->m_last_Analog = (*p)->m_proxied_net->m_cur_Analog;
        }
#endif
}


ATTR_HOT void netlist_matrix_solver_t::update_dynamic()
{
	/* update all non-linear devices  */
	for (netlist_core_device_t * const *p = m_dynamic.first(); p != NULL; p = m_dynamic.next(p))
		switch ((*p)->family())
		{
			case netlist_device_t::DIODE:
				static_cast<NETLIB_NAME(D) *>((*p))->update_terminals();
				break;
			default:
				(*p)->update_terminals();
				break;
		}
}

ATTR_COLD void netlist_matrix_solver_t::start()
{
    register_output("Q_sync", m_Q_sync);
    register_input("FB_sync", m_fb_sync);
    connect(m_fb_sync, m_Q_sync);
}

ATTR_COLD void netlist_matrix_solver_t::reset()
{
	m_last_step = netlist_time::zero;
}

ATTR_COLD void netlist_matrix_solver_t::update()
{
    const double new_timestep = solve();

    if (m_params.m_dynamic && is_timestep() && new_timestep > 0)
        m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(new_timestep));
}

ATTR_COLD void netlist_matrix_solver_t::update_forced()
{
    ATTR_UNUSED const double new_timestep = solve();

    if (m_params.m_dynamic && is_timestep())
        m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(m_params.m_min_timestep));
}

ATTR_HOT void netlist_matrix_solver_t::step(const netlist_time delta)
{
	const double dd = delta.as_double();
	for (int k=0; k < m_steps.count(); k++)
		m_steps[k]->step_time(dd);
}

ATTR_HOT double netlist_matrix_solver_t::solve()
{

	netlist_time now = netlist().time();
	netlist_time delta = now - m_last_step;

	// We are already up to date. Avoid oscillations.
	if (delta < netlist_time::from_nsec(1))
	    return -1.0;

	NL_VERBOSE_OUT(("Step!\n"));
	/* update all terminals for new time step */
	m_last_step = now;
	//printf("usecs: %f\n", delta.as_double()*1000000.0);
	step(delta);

	if (is_dynamic())
	{
		int this_resched;
		int newton_loops = 0;
		do
		{
            update_dynamic();
            // Gauss-Seidel will revert to Gaussian elemination if steps exceeded.
            this_resched = vsolve_non_dynamic();
            newton_loops++;
		} while (this_resched > 1 && newton_loops < m_params.m_nr_loops);

		// reschedule ....
		if (this_resched > 1 && !m_Q_sync.net().is_queued())
		{
            netlist().warning("NEWTON_LOOPS exceeded ... reschedule");
	        m_Q_sync.net().reschedule_in_queue(m_params.m_nt_sync_delay);
	        return 1.0;
		}
	}
	else
	{
		vsolve_non_dynamic();
	}
	const double next_time_step = compute_next_timestep(delta.as_double());
    update_inputs();
	return next_time_step;
}

template <int m_N, int _storage_N>
void netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::log_stats()
{
#if 0
    printf("==============================================\n");
    printf("Solver %s\n", this->name().cstr());
    printf("       ==> %d nets\n", this->N()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
    printf("       has %s elements\n", this->is_dynamic() ? "dynamic" : "no dynamic");
    printf("       has %s elements\n", this->is_timestep() ? "timestep" : "no timestep");
    printf("       %10d invocations (%6d Hz)  %10d gs fails (%6.2f%%) %4.1f average\n",
            this->m_calculations,
            this->m_calculations * 10 / (int) (this->netlist().time().as_double() * 10.0),
            this->m_gs_fail,
            100.0 * (double) this->m_gs_fail / (double) this->m_calculations,
            (double) this->m_gs_total / (double) this->m_calculations);
#endif
}

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct base
// ----------------------------------------------------------------------------------------

ATTR_COLD int netlist_matrix_solver_t::get_net_idx(netlist_net_t *net)
{
	for (int k = 0; k < m_nets.count(); k++)
		if (m_nets[k] == net)
			return k;
	return -1;
}

template <int m_N, int _storage_N>
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::add_term(int k, netlist_terminal_t *term)
{
    if (term->m_otherterm->net().isRailNet())
    {
        //m_nets[k].m_rails.add(pterm);
        m_rails[k].add(term, -1);
    }
    else
    {
        int ot = get_net_idx(&term->m_otherterm->net());
        if (ot>=0)
        {
            m_terms[k].add(term, ot);
            SOLVER_VERBOSE_OUT(("Net %d Term %s %f %f\n", k, terms[i]->name().cstr(), terms[i]->m_gt, terms[i]->m_go));
        }
        /* Should this be allowed ? */
        else // if (ot<0)
        {
           m_rails[k].add(term, ot);
           netlist().error("found term with missing othernet %s\n", term->name().cstr());
        }
    }
}


template <int m_N, int _storage_N>
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::vsetup(netlist_analog_net_t::list_t &nets)
{
    m_dim = nets.count();

    for (int k = 0; k < N(); k++)
    {
        m_terms[k].clear();
        m_rails[k].clear();
    }

    netlist_matrix_solver_t::setup(nets);

#if 0
	for (int k = 0; k < N(); k++)
	{
		const netlist_terminal_t::list_t &terms = m_nets[k].m_terms;
		for (int i = 0; i < terms.count(); i++)
		{
			int ot = get_net_idx(&terms[i]->m_otherterm->net());
			if (ot>=0)
			{
			    m_terms[k].add(terms_t(terms[i], ot));
				SOLVER_VERBOSE_OUT(("Net %d Term %s %f %f\n", k, terms[i]->name().cstr(), terms[i]->m_gt, terms[i]->m_go));
			}
		}
	}

    /* Should this be allowed ? */
	for (int k = 0; k < N(); k++)
	{
		const netlist_terminal_t::list_t &terms = m_nets[k].m_terms;
		for (int i = 0; i < terms.count(); i++)
		{
            int ot = get_net_idx(&terms[i]->m_otherterm->net());
			if (ot<0)
			{
                m_rails[k].add(terms_t(terms[i], ot));
				netlist().warning("found term with missing othernet %s\n", terms[i]->name().cstr());
			}
		}
    }


	for (int k = 0; k < N(); k++)
	{
		const netlist_terminal_t::list_t &rails = m_nets[k].m_rails;
		for (int i = 0; i < rails.count(); i++)
		{
            m_rails[k].add(terms_t(rails[i], -1));
			SOLVER_VERBOSE_OUT(("Net %d Rail %s %f %f\n", k, rails[i]->name().cstr(), rails[i]->m_gt, rails[i]->m_go));
		}
	}
#endif
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::build_LE()
{
    for (int k=0; k < _storage_N; k++)
        for (int i=0; i < _storage_N; i++)
            m_A[k][i] = 0.0;

    for (int k = 0; k < N(); k++)
	{
        double rhsk = 0.0;
        double akk  = 0.0;
        const int terms_count = m_terms[k].count();
        const netlist_terminal_t * const *terms = m_terms[k].terms();
        const int *net_other = m_terms[k].net_other();

        for (int i = 0; i < terms_count; i++)
		{
            //printf("A %d %d %s %f %f\n",t.net_this, t.net_other, t.term->name().cstr(), t.term->m_gt, t.term->m_go);

            rhsk = rhsk + terms[i]->m_Idr;
            akk = akk + terms[i]->m_gt;
            m_A[k][net_other[i]] += -terms[i]->m_go;
		}

        const int rails_count = m_rails[k].count();
        const netlist_terminal_t * const *rails = m_rails[k].terms();

        for (int i = 0; i < rails_count; i++)
        {
            rhsk = rhsk + rails[i]->m_Idr + rails[i]->m_go * rails[i]->m_otherterm->net().as_analog().Q_Analog();
            akk = akk + rails[i]->m_gt;
        }
        m_RHS[k] = rhsk;
        m_A[k][k] += akk;
    }
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::gauss_LE(
		double (* RESTRICT x))
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

	    /* Singular matrix? */
		double f = m_A[i][i];
		//if (fabs(f) < 1e-20) printf("Singular!");
		f = 1.0 / f;

		/* Eliminate column i from row j */

		for (int j = i + 1; j < kN; j++)
		{
            //__builtin_prefetch(&A[j+1][i], 1);
            const double f1 = m_A[j][i] * f;
			if (f1 != 0.0)
			{
                for (int k = i; k < kN; k++)
                    m_A[j][k] -= m_A[i][k] * f1;
	            m_RHS[j] -= m_RHS[i] * f1;
			}
		}
	}
	/* back substitution */
	for (int j = kN - 1; j >= 0; j--)
	{
        //__builtin_prefetch(&A[j-1][j], 0);
		double tmp = 0;
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
ATTR_HOT double netlist_matrix_solver_direct_t<m_N, _storage_N>::delta(
		const double (* RESTRICT V))
{
	double cerr = 0;
	double cerr2 = 0;
	for (int i = 0; i < this->N(); i++)
	{
		const double e = (V[i] - this->m_nets[i]->m_cur_Analog);
		const double e2 = (m_RHS[i] - this->m_last_RHS[i]);
		cerr = (fabs(e) > cerr ? fabs(e) : cerr);
        cerr2 = (fabs(e2) > cerr2 ? fabs(e2) : cerr2);
	}
	// FIXME: Review
	return cerr + cerr2*100000.0;
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::store(
		const double (* RESTRICT V), const bool store_RHS)
{
	for (int i = 0; i < this->N(); i++)
	{
		this->m_nets[i]->m_cur_Analog = this->m_nets[i]->m_new_Analog = V[i];
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
ATTR_HOT int netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic()
{
    double new_v[_storage_N] = { 0.0 };

    this->gauss_LE(new_v);

    if (this->is_dynamic())
    {
        double err = delta(new_v);

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
ATTR_HOT int netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic()
{
	this->build_LE();

	return this->solve_non_dynamic();
}


// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct1
// ----------------------------------------------------------------------------------------

ATTR_HOT int netlist_matrix_solver_direct1_t::vsolve_non_dynamic()
{

    netlist_analog_net_t *net = m_nets[0];
	this->build_LE();
	//NL_VERBOSE_OUT(("%f %f\n", new_val, m_RHS[0] / m_A[0][0]);

	double new_val =  m_RHS[0] / m_A[0][0];

	double e = (new_val - net->m_cur_Analog);
	double cerr = fabs(e);

	net->m_cur_Analog = net->m_new_Analog = new_val;

	if (is_dynamic() && (cerr  > m_params.m_accuracy))
	{
		return 2;
	}
	else
		return 1;

}



// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct2
// ----------------------------------------------------------------------------------------

ATTR_HOT int netlist_matrix_solver_direct2_t::vsolve_non_dynamic()
{

	build_LE();

	const double a = m_A[0][0];
	const double b = m_A[0][1];
	const double c = m_A[1][0];
	const double d = m_A[1][1];

	double new_val[2];
	new_val[1] = (a * m_RHS[1] - c * m_RHS[0]) / (a * d - b * c);
	new_val[0] = (m_RHS[0] - b * new_val[1]) / a;

	if (is_dynamic())
	{
		double err = delta(new_val);
		store(new_val, true);
		if (err > m_params.m_accuracy )
			return 2;
		else
			return 1;
	}
	store(new_val, false);
	return 1;
}

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
ATTR_HOT int netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::vsolve_non_dynamic()
{
    /* The matrix based code looks a lot nicer but actually is 30% slower than
     * the optimized code which works directly on the data structures.
     * Need something like that for gaussian elimination as well.
     */

#if 0
    double new_v[_storage_N] = { 0.0 };
    const int iN = this->N();

    bool resched = false;

    int  resched_cnt = 0;

    this->build_LE();

    for (int k = 0; k < iN; k++)
    {
        new_v[k] = this->m_nets[k]->m_cur_Analog;
    }
    do {
        resched = false;
        double cerr = 0.0;

        for (int k = 0; k < iN; k++)
        {
            double Idrive = 0;

            // loop auto-vectorized
            for (int i = 0; i < iN; i++)
                Idrive -= this->m_A[k][i] * new_v[i];

            const double new_val = (this->m_RHS[k] + Idrive + this->m_A[k][k] * new_v[k]) / this->m_A[k][k];

            const double e = fabs(new_val - new_v[k]);
            cerr = (e > cerr ? e : cerr);
            new_v[k] = new_val;
        }

        if (cerr > this->m_params.m_accuracy)
        {
            resched = true;
        }
        resched_cnt++;
    } while (resched && (resched_cnt < this->m_params.m_gs_loops));

    this->m_gs_total += resched_cnt;
    if (resched)
    {
        //this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
        this->m_gs_fail++;
        int tmp = netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic();
        this->m_calculations++;
        return tmp;
    }
    else {
        this->m_calculations++;

        this->store(new_v, false);

        return resched_cnt;
    }

#else
    const int iN = this->N();
	bool resched = false;

	int  resched_cnt = 0;
	ATTR_UNUSED netlist_net_t *last_resched_net = NULL;

	/* over-relaxation not really works on these matrices */
	//const double w = 1.0; //2.0 / (1.0 + sin(3.14159 / (m_nets.count()+1)));
	//const double w1 = 1.0 - w;

	double w[_storage_N];
	double one_m_w[_storage_N];
	double RHS[_storage_N];
	double new_V[_storage_N];

    for (int k = 0; k < iN; k++)
    {
        new_V[k] = this->m_nets[k]->m_new_Analog = this->m_nets[k]->m_cur_Analog;
    }

	for (int k = 0; k < iN; k++)
	{
		double gtot_t = 0.0;
		double gabs_t = 0.0;
		double RHS_t = 0.0;

		{
	        const netlist_terminal_t * const * rails = this->m_rails[k].terms();
	        //const int * othernet = this->m_rails[k].m_othernet;
	        const int rail_count = this->m_rails[k].count();

	        for (int i = 0; i < rail_count; i++)
	        {
	            const netlist_terminal_t *rail = rails[i];
	            gtot_t += rail->m_gt;
	            gabs_t += fabs(rail->m_go);
	            RHS_t += rail->m_Idr;
	            // this may point to a rail net ...
	            RHS_t += rail->m_go * rail->m_otherterm->net().as_analog().Q_Analog();
	        }
		}
		{
            const netlist_terminal_t * const * terms = this->m_terms[k].terms();
            const int term_count = this->m_terms[k].count();

            for (int i = 0; i < term_count; i++)
            {
                const netlist_terminal_t *term = terms[i];
                gtot_t += term->m_gt;
                gabs_t += fabs(term->m_go);
                RHS_t += term->m_Idr;
            }
		}
		gabs_t *= 1.0;
		if (gabs_t > gtot_t)
		{
			w[k] = 1.0 / (gtot_t + gabs_t);
            one_m_w[k] = 1.0 - 1.0 * gtot_t / (gtot_t + gabs_t);
		}
		else
		{
	        const double ws = 1.0;
			w[k] = ws / gtot_t;
			one_m_w[k] = 1.0 - ws;
		}

		RHS[k] = RHS_t;
	}

	do {
		resched = false;
		double cerr = 0.0;

		for (int k = 0; k < iN; k++)
		{
			//netlist_analog_net_t & RESTRICT net = *this->m_nets[k];
	        const netlist_terminal_t * const * terms = this->m_terms[k].terms();
            const int * net_other = this->m_terms[k].net_other();
			const int term_count = this->m_terms[k].count();

			double Idrive = 0;

            for (int i = 0; i < term_count; i++)
                Idrive += terms[i]->m_go * new_V[net_other[i]];

            //double new_val = (net->m_cur_Analog * gabs[k] + iIdr) / (gtot[k]);
			const double new_val = new_V[k] * one_m_w[k] + (Idrive + RHS[k]) * w[k];

			const double e = fabs(new_val - new_V[k]);
			cerr = (e > cerr ? e : cerr);

			new_V[k] = new_val;
		}
		if (cerr > this->m_params.m_accuracy)
		{
			resched = true;
		}
		resched_cnt++;
	} while (resched && (resched_cnt < this->m_params.m_gs_loops));

    for (int k = 0; k < iN; k++)
    {
        this->m_nets[k]->m_new_Analog = this->m_nets[k]->m_cur_Analog = new_V[k];
    }


	this->m_gs_total += resched_cnt;
	if (resched)
	{
	    //this->netlist().warning("Falling back to direct solver .. Consider increasing RESCHED_LOOPS");
	    this->m_gs_fail++;
	    int tmp = netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic();
	    this->m_calculations++;
        return tmp;
	}
	else {
	    this->m_calculations++;

	    //for (int k = 0; k < this->N(); k++)
	    //    this->m_nets[k]->m_cur_Analog = this->m_nets[k]->m_new_Analog;

	    return resched_cnt;
	}
#endif
}

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------



NETLIB_START(solver)
{
	register_output("Q_step", m_Q_step);

    register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(10).as_double());

	register_param("FREQ", m_freq, 48000.0);

	register_param("ACCURACY", m_accuracy, 1e-4);
	register_param("GS_LOOPS", m_gs_loops, 5);              // Gauss-Seidel loops
    register_param("NR_LOOPS", m_nr_loops, 25);             // Newton-Raphson loops
	register_param("PARALLEL", m_parallel, 0);
    register_param("GMIN", m_gmin, NETLIST_GMIN_DEFAULT);
    register_param("DYNAMIC_TS", m_dynamic, 0);
    register_param("LTE", m_lte, 1e-2);                     // 100mV diff/timestep
	register_param("MIN_TIMESTEP", m_min_timestep, 2e-9);   // double timestep resolution

	// internal staff

	register_input("FB_step", m_fb_step);
	connect(m_fb_step, m_Q_step);

}

NETLIB_RESET(solver)
{
	for (int i = 0; i < m_mat_solvers.count(); i++)
		m_mat_solvers[i]->reset();
}


NETLIB_UPDATE_PARAM(solver)
{
	//m_inc = netlist_time::from_hz(m_freq.Value());
}

NETLIB_NAME(solver)::~NETLIB_NAME(solver)()
{
    for (int i = 0; i < m_mat_solvers.count(); i++)
        m_mat_solvers[i]->log_stats();

	netlist_matrix_solver_t * const *e = m_mat_solvers.first();
	while (e != NULL)
	{
		netlist_matrix_solver_t * const *en = m_mat_solvers.next(e);
		delete *e;
		e = en;
	}

}

NETLIB_UPDATE(solver)
{
	if (m_params.m_dynamic)
	    return;

    const int t_cnt = m_mat_solvers.count();

#if HAS_OPENMP && USE_OPENMP
	if (m_parallel.Value())
	{
		omp_set_num_threads(4);
		omp_set_dynamic(0);
		#pragma omp parallel
		{
			#pragma omp for nowait
			for (int i = 0; i <  t_cnt; i++)
			{
				this_resched[i] = m_mat_solvers[i]->solve();
			}
		}
	}
	else
		for (int i = 0; i < t_cnt; i++)
		{
			if (do_full || (m_mat_solvers[i]->is_timestep()))
				this_resched[i] = m_mat_solvers[i]->solve();
		}
#else
	// FIXME: parameter
    for (int i = 0; i < t_cnt; i++)
    {
        if (m_mat_solvers[i]->is_timestep())
            {
            // Ignore return value
                ATTR_UNUSED const double ts = m_mat_solvers[i]->solve();
            }
    }
#endif

    /* step circuit */
    if (!m_Q_step.net().is_queued())
    {
        m_Q_step.net().push_to_queue(netlist_time::from_double(m_params.m_max_timestep));
    }
}

template <int m_N, int _storage_N>
netlist_matrix_solver_t * NETLIB_NAME(solver)::create_solver(const int gs_threshold, const bool use_specific)
{
    if (use_specific && m_N == 1)
        return new netlist_matrix_solver_direct1_t();
    else if (use_specific && m_N == 2)
        return new netlist_matrix_solver_direct2_t();
    else
    {
        if (_storage_N >= gs_threshold)
            return new netlist_matrix_solver_gauss_seidel_t<m_N,_storage_N>();
        else
            return new netlist_matrix_solver_direct_t<m_N, _storage_N>();
    }
}

ATTR_COLD void NETLIB_NAME(solver)::post_start()
{
	netlist_analog_net_t::list_t groups[100];
	int cur_group = -1;
	// FIXME: Turn into parameters ...
	const int gs_threshold = 5;
	const bool use_specific = true;

    m_params.m_accuracy = m_accuracy.Value();
    m_params.m_gs_loops = m_gs_loops.Value();
    m_params.m_nr_loops = m_nr_loops.Value();
    m_params.m_nt_sync_delay = m_sync_delay.Value();
    m_params.m_lte = m_lte.Value();
    m_params.m_min_timestep = m_min_timestep.Value();
    m_params.m_dynamic = (m_dynamic.Value() == 1 ? true : false);
    m_params.m_max_timestep = netlist_time::from_hz(m_freq.Value()).as_double();

    if (m_params.m_dynamic)
    {
        m_params.m_max_timestep *= 1000.0;
    }
    else
    {
        m_params.m_min_timestep = m_params.m_max_timestep;
    }

	netlist().log("Scanning net groups ...");
	// determine net groups
	for (netlist_net_t * const *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
	{
	    SOLVER_VERBOSE_OUT(("processing %s\n", (*pn)->name().cstr()));
	    if (!(*pn)->isRailNet())
	    {
	        SOLVER_VERBOSE_OUT(("   ==> not a rail net\n"));
	        netlist_analog_net_t *n = &(*pn)->as_analog();
	        if (!n->already_processed(groups, cur_group))
	        {
	            cur_group++;
	            n->process_net(groups, cur_group);
	        }
	    }
	}

	// setup the solvers
	netlist().log("Found %d net groups in %d nets\n", cur_group + 1, netlist().m_nets.count());
	for (int i = 0; i <= cur_group; i++)
	{
		netlist_matrix_solver_t *ms;
		int net_count = groups[i].count();

		switch (net_count)
		{
			case 1:
				ms = create_solver<1,1>(gs_threshold, use_specific);
				break;
			case 2:
                ms = create_solver<2,2>(gs_threshold, use_specific);
				break;
			case 3:
                ms = create_solver<3,3>(gs_threshold, use_specific);
				break;
			case 4:
                ms = create_solver<4,4>(gs_threshold, use_specific);
				break;
			case 5:
                ms = create_solver<5,5>(gs_threshold, use_specific);
				break;
			case 6:
                ms = create_solver<6,6>(gs_threshold, use_specific);
				break;
            case 7:
                ms = create_solver<7,7>(gs_threshold, use_specific);
                break;
            case 8:
                ms = create_solver<8,8>(gs_threshold, use_specific);
                break;
			default:
				if (net_count <= 16)
				{
	                ms = create_solver<0,16>(gs_threshold, use_specific);
				}
				else if (net_count <= 32)
				{
	                ms = create_solver<0,32>(gs_threshold, use_specific);
				}
				else if (net_count <= 64)
				{
	                ms = create_solver<0,64>(gs_threshold, use_specific);
				}
				else
				{
					netlist().error("Encountered netgroup with > 64 nets");
					ms = NULL; /* tease compilers */
				}

				break;
		}

		ms->m_params = m_params;

		register_sub(*ms, pstring::sprintf("Solver %d",m_mat_solvers.count()));

        ms->vsetup(groups[i]);

        m_mat_solvers.add(ms);

        netlist().log("Solver %s", ms->name().cstr());
        netlist().log("       # %d ==> %d nets", i, groups[i].count()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
		netlist().log("       has %s elements", ms->is_dynamic() ? "dynamic" : "no dynamic");
		netlist().log("       has %s elements", ms->is_timestep() ? "timestep" : "no timestep");
		for (int j=0; j<groups[i].count(); j++)
		{
		    netlist().log("Net %d: %s", j, groups[i][j]->name().cstr());
			netlist_net_t *n = groups[i][j];
			for (int k = 0; k < n->m_core_terms.count(); k++)
			{
			    const netlist_core_terminal_t *p = n->m_core_terms[k];
			    netlist().log("   %s", p->name().cstr());
			}
		}
	}
}

