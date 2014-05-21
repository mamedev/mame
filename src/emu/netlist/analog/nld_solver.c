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
: m_owner(NULL)
, m_calculations(0)
, m_gs_fail(0)
, m_gs_total(0)
{
}

ATTR_COLD netlist_matrix_solver_t::~netlist_matrix_solver_t()
{
    for (int i = 0; i < m_inps.count(); i++)
        delete m_inps[i];
}

ATTR_COLD void netlist_matrix_solver_t::vsetup(netlist_analog_net_t::list_t &nets, NETLIB_NAME(solver) &aowner)
{
	m_owner = &aowner;

	NL_VERBOSE_OUT(("New solver setup\n"));

	for (netlist_analog_net_t * const * pn = nets.first(); pn != NULL; pn = nets.next(pn))
	{
		NL_VERBOSE_OUT(("setting up net\n"));

		m_nets.add(*pn);

		(*pn)->m_solver = this;

        for (int i = 0; i < (*pn)->m_core_terms.count(); i++)
		{
		    netlist_core_terminal_t *p = (*pn)->m_core_terms[i];
			NL_VERBOSE_OUT(("%s %s %d\n", p->name().cstr(), (*pn)->name().cstr(), (int) (*pn)->isRailNet()));
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
						if (pterm->m_otherterm->net().isRailNet())
							(*pn)->m_rails.add(pterm);
						else
							(*pn)->m_terms.add(pterm);
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
					owner().netlist().error("unhandled element found\n");
					break;
			}
		}
		NL_VERBOSE_OUT(("added net with %d populated connections (%d railnets)\n", (*pn)->m_terms.count(), (*pn)->m_rails.count()));
	}
}

ATTR_HOT double netlist_matrix_solver_t::compute_next_timestep(const double hn)
{
    double new_solver_timestep = m_params.m_max_timestep;

    if (m_params.m_dynamic)
    {
        for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
        {
            netlist_analog_net_t *n = (*p)->m_proxied_net;
            double DD_n = (n->m_cur_Analog - n->m_last_Analog) / hn;

            double h_n_m_1 = n->m_h_n_m_1;
            // limit last timestep in equation.
            //if (h_n_m_1 > 3 * hn)
            //    h_n_m_1 = 3 * hn;

            double DD2 = (DD_n - n->m_DD_n_m_1) / (hn + h_n_m_1);
            double new_net_timestep;

            n->m_DD_n_m_1 = DD_n;
            n->m_h_n_m_1 = hn;
            if (fabs(DD2) > 1e-200) // avoid div-by-zero
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

ATTR_HOT void netlist_matrix_solver_t::update_inputs()
{
    // avoid recursive calls. Inputs are updated outside this call
    for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
        if ((*p)->m_proxied_net->m_last_Analog != (*p)->m_proxied_net->m_cur_Analog)
            (*p)->set_Q((*p)->m_proxied_net->m_cur_Analog);
    for (netlist_analog_output_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
    {
        if ((*p)->m_proxied_net->m_last_Analog != (*p)->m_proxied_net->m_cur_Analog)
            (*p)->m_proxied_net->m_last_Analog = (*p)->m_proxied_net->m_cur_Analog;
    }
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

    if (m_params.m_dynamic && new_timestep > 0)
        m_Q_sync.net().reschedule_in_queue(netlist_time::from_double(new_timestep));
}

ATTR_COLD void netlist_matrix_solver_t::update_forced()
{
    const double new_timestep = solve();

    if (!m_params.m_dynamic)
        return;

     if (new_timestep > 0)
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

	netlist_time now = owner().netlist().time();
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
            while ((this_resched = vsolve_non_dynamic()) > m_params.m_gs_loops)
                owner().netlist().warning("Dynamic Solve iterations exceeded .. Consider increasing RESCHED_LOOPS");
            newton_loops++;
		} while (this_resched > 1 && newton_loops < m_params.m_nr_loops);

		// reschedule ....
		if (this_resched > 1 && !m_Q_sync.net().is_queued())
		{
            owner().netlist().warning("NEWTON_LOOPS exceeded ... reschedule");
	        m_Q_sync.net().reschedule_in_queue(m_params.m_nt_sync_delay);
	        return 1.0;
		}
	}
	else
	{
		while (vsolve_non_dynamic() > m_params.m_gs_loops)
            owner().netlist().warning("Non-Dynamic Solve iterations exceeded .. Consider increasing RESCHED_LOOPS");
	}
	const double next_time_step = compute_next_timestep(delta.as_double());
    update_inputs();
	return next_time_step;
}

void netlist_matrix_solver_t::log_stats()
{
#if 0
    printf("==============================================\n");
    printf("Solver %s\n", name().cstr());
    printf("       ==> %d nets\n", m_nets.count()); //, (*(*groups[i].first())->m_core_terms.first())->name().cstr());
    printf("       has %s elements\n", is_dynamic() ? "dynamic" : "no dynamic");
    printf("       has %s elements\n", is_timestep() ? "timestep" : "no timestep");
    printf("       %10d invocations (%6d Hz)  %10d gs fails (%6.2f%%) %4.1f average\n",
            m_calculations,
            m_calculations * 10 / (int) (netlist().time().as_double() * 10.0),
            m_gs_fail,
            100.0 * (double) m_gs_fail / (double) m_calculations,
            (double) m_gs_total / (double) m_calculations);
#endif
}

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct base
// ----------------------------------------------------------------------------------------


template <int m_N, int _storage_N>
ATTR_COLD int netlist_matrix_solver_direct_t<m_N, _storage_N>::get_net_idx(netlist_net_t *net)
{
	for (int k = 0; k < N(); k++)
		if (m_nets[k] == net)
			return k;
	return -1;
}

template <int m_N, int _storage_N>
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::vsetup(netlist_analog_net_t::list_t &nets, NETLIB_NAME(solver) &owner)
{
	netlist_matrix_solver_t::vsetup(nets, owner);

	m_term_num = 0;
	m_rail_start = 0;
	for (int k = 0; k < N(); k++)
	{
		netlist_analog_net_t *net = m_nets[k];
		const netlist_terminal_t::list_t &terms = net->m_terms;
		for (int i = 0; i < terms.count(); i++)
		{
			m_terms[m_term_num].net_this = k;
			int ot = get_net_idx(&terms[i]->m_otherterm->net());
			m_terms[m_term_num].net_other = ot;
			m_terms[m_term_num].term = terms[i];
			if (ot>=0)
			{
				m_term_num++;
				SOLVER_VERBOSE_OUT(("Net %d Term %s %f %f\n", k, terms[i]->name().cstr(), terms[i]->m_gt, terms[i]->m_go));
			}
		}
	}
	m_rail_start = m_term_num;
	for (int k = 0; k < N(); k++)
	{
		netlist_analog_net_t *net = m_nets[k];
		const netlist_terminal_t::list_t &terms = net->m_terms;
		const netlist_terminal_t::list_t &rails = net->m_rails;
		for (int i = 0; i < terms.count(); i++)
		{
			m_terms[m_term_num].net_this = k;
			int ot = get_net_idx(&terms[i]->m_otherterm->net());
			m_terms[m_term_num].net_other = ot;
			m_terms[m_term_num].term = terms[i];
			if (ot<0)
			{
				m_term_num++;
				SOLVER_VERBOSE_OUT(("found term with missing othernet %s\n", terms[i]->name().cstr()));
			}
		}
		for (int i = 0; i < rails.count(); i++)
		{
			m_terms[m_term_num].net_this = k;
			m_terms[m_term_num].net_other = -1; //get_net_idx(&rails[i]->m_otherterm->net());
			m_terms[m_term_num].term = rails[i];
			m_term_num++;
			SOLVER_VERBOSE_OUT(("Net %d Rail %s %f %f\n", k, rails[i]->name().cstr(), rails[i]->m_gt, rails[i]->m_go));
		}
	}
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::build_LE(
		double (* RESTRICT A)[_storage_N],
		double (* RESTRICT RHS))
{
#if 0
	for (int i = 0; i < m_term_num; i++)
	{
		terms_t &t = m_terms[i];
		RHS[t.net_this] += t.term->m_Idr;
		A[t.net_this][t.net_this] += t.term->m_gt;
		if (t.net_other >= 0)
		{
			//m_A[t.net_other][t.net_other] += t.term->m_otherterm->m_gt;
			A[t.net_this][t.net_other] += -t.term->m_go;
			//m_A[t.net_other][t.net_this] += -t.term->m_otherterm->m_go;
		}
		else
			RHS[t.net_this] += t.term->m_go * t.term->m_otherterm->net().Q_Analog();
	}
#else
	for (int i = 0; i < m_rail_start; i++)
	{
		terms_t &t = m_terms[i];
		//printf("A %d %d %s %f %f\n",t.net_this, t.net_other, t.term->name().cstr(), t.term->m_gt, t.term->m_go);

		RHS[t.net_this] += t.term->m_Idr;
		A[t.net_this][t.net_this] += t.term->m_gt;
		A[t.net_this][t.net_other] += -t.term->m_go;
	}
	for (int i = m_rail_start; i < m_term_num; i++)
	{
		terms_t &t = m_terms[i];

		RHS[t.net_this] += t.term->m_Idr;
		A[t.net_this][t.net_this] += t.term->m_gt;
		RHS[t.net_this] += t.term->m_go * t.term->m_otherterm->net().as_analog().Q_Analog();
	}
#endif
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::gauss_LE(
		double (* RESTRICT A)[_storage_N],
		double (* RESTRICT RHS),
		double (* RESTRICT x))
{
#if 0
	for (int i = 0; i < N(); i++)
	{
		for (int k = 0; k < N(); k++)
			printf("%f ", A[i][k]);
		printf("| %f = %f \n", x[i], RHS[i]);
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
	            if (fabs(A[j][i]) > fabs(A[maxrow][i]))
	                maxrow = j;
	        }

	        if (maxrow != i)
	        {
	            /* Swap the maxrow and ith row */
	            for (int k = i; k < kN; k++) {
	                std::swap(A[i][k], A[maxrow][k]);
	            }
	            std::swap(RHS[i], RHS[maxrow]);
	        }
	    }

	    /* Singular matrix? */
		double f = A[i][i];
		//if (fabs(f) < 1e-20) printf("Singular!");
		f = 1.0 / f;

		/* Eliminate column i from row j */

		for (int j = i + 1; j < kN; j++)
		{
            //__builtin_prefetch(&A[j+1][i], 1);
            const double f1 = A[j][i] * f;
			if (f1 != 0.0)
			{
	            for (int k = i; k < kN; k++)
					A[j][k] -= A[i][k] * f1;

	            RHS[j] -= RHS[i] * f1;
			}
		}
	}
	/* back substitution */
	for (int j = kN - 1; j >= 0; j--)
	{
        //__builtin_prefetch(&A[j-1][j], 0);
		double tmp = 0;
        for (int k = j + 1; k < kN; k++)
            tmp += A[j][k] * x[k];
		x[j] = (RHS[j] - tmp) / A[j][j];
	}
#if 0
	printf("Solution:\n");
	for (int i = 0; i < N(); i++)
	{
		for (int k = 0; k < N(); k++)
			printf("%f ", A[i][k]);
		printf("| %f = %f \n", x[i], RHS[i]);
	}
	printf("\n");
#endif

}

template <int m_N, int _storage_N>
ATTR_HOT double netlist_matrix_solver_direct_t<m_N, _storage_N>::delta(
		const double (* RESTRICT RHS),
		const double (* RESTRICT V))
{
	double cerr = 0;
	double cerr2 = 0;
	for (int i = 0; i < this->N(); i++)
	{
		double e = (V[i] - this->m_nets[i]->m_cur_Analog);
		double e2 = (RHS[i] - this->m_RHS[i]);
		cerr += e * e;
		cerr2 += e2 * e2;
	}
	return (cerr + cerr2*(100000.0 * 100000.0)) / this->N();
}

template <int m_N, int _storage_N>
ATTR_HOT void netlist_matrix_solver_direct_t<m_N, _storage_N>::store(
		const double (* RESTRICT RHS),
		const double (* RESTRICT V))
{
	for (int i = 0; i < this->N(); i++)
	{
		this->m_nets[i]->m_cur_Analog = this->m_nets[i]->m_new_Analog = V[i];
	}
	if (RHS != NULL)
	{
		for (int i = 0; i < this->N(); i++)
		{
			this->m_RHS[i] = RHS[i];
		}
	}
}

template <int m_N, int _storage_N>
ATTR_HOT int netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic()
{
	double A[_storage_N][_storage_N] = { { 0.0 } };
	double RHS[_storage_N] = { 0.0 };
	double new_v[_storage_N] = { 0.0 };

	this->build_LE(A, RHS);

	this->gauss_LE(A, RHS, new_v);

	if (this->is_dynamic())
	{
		double err = delta(RHS, new_v);

		store(RHS, new_v);

		if (err > this->m_params.m_accuracy * this->m_params.m_accuracy)
		{
			return 2;
		}
		return 1;
	}
	store(NULL, new_v);  // ==> No need to store RHS
	return 1;
}


// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Direct1
// ----------------------------------------------------------------------------------------

ATTR_HOT int netlist_matrix_solver_direct1_t::vsolve_non_dynamic()
{

    netlist_analog_net_t *net = m_nets[0];
	double m_A[1][1] = { {0.0} };
	double m_RHS[1] = { 0.0 };
	build_LE(m_A, m_RHS);
	//NL_VERBOSE_OUT(("%f %f\n", new_val, m_RHS[0] / m_A[0][0]);

	double new_val =  m_RHS[0] / m_A[0][0];

	double e = (new_val - net->m_cur_Analog);
	double cerr = e * e;

	net->m_cur_Analog = net->m_new_Analog = new_val;

	if (is_dynamic() && (cerr  > m_params.m_accuracy * m_params.m_accuracy))
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
	double A[2][2] = { { 0.0 } };
	double RHS[2] = { 0.0 };

	build_LE(A, RHS);

	const double a = A[0][0];
	const double b = A[0][1];
	const double c = A[1][0];
	const double d = A[1][1];

	double new_val[2];
	new_val[1] = a / (a*d - b*c) * (RHS[1] - c / a * RHS[0]);
	new_val[0] = (RHS[0] - b * new_val[1]) / a;

	if (is_dynamic())
	{
		double err = delta(RHS, new_val);
		store(RHS, new_val);
		if (err > m_params.m_accuracy * m_params.m_accuracy)
			return 2;
		else
			return 1;
	}
	store(NULL, new_val);
	return 1;
}

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver - Gauss - Seidel
// ----------------------------------------------------------------------------------------

template <int m_N, int _storage_N>
ATTR_HOT int netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::vsolve_non_dynamic()
{
	bool resched = false;

	int  resched_cnt = 0;
	ATTR_UNUSED netlist_net_t *last_resched_net = NULL;

	/* over-relaxation not really works on these matrices */
	//const double w = 1.0; //2.0 / (1.0 + sin(3.14159 / (m_nets.count()+1)));
	//const double w1 = 1.0 - w;

	double w[_storage_N];
	double one_m_w[_storage_N];
	double RHS[_storage_N];

	for (int k = 0; k < this->N(); k++)
	{
		double gtot_t = 0.0;
		double gabs_t = 0.0;
		double RHS_t = 0.0;

		netlist_analog_net_t *net = this->m_nets[k];
		const netlist_terminal_t::list_t &terms = net->m_terms;
		const netlist_terminal_t::list_t &rails = net->m_rails;
		const int term_count = terms.count();
		const int rail_count = rails.count();

		for (int i = 0; i < rail_count; i++)
		{
			gtot_t += rails[i]->m_gt;
			gabs_t += fabs(rails[i]->m_go);
			RHS_t += rails[i]->m_Idr;
			RHS_t += rails[i]->m_go * rails[i]->m_otherterm->net().as_analog().Q_Analog();
		}

		for (int i = 0; i < term_count; i++)
		{
			gtot_t += terms[i]->m_gt;
			gabs_t += fabs(terms[i]->m_go);
			RHS_t += terms[i]->m_Idr;
		}

		gabs_t *= this->m_params.m_convergence_factor;
		if (gabs_t > gtot_t)
		{
			// Actually 1.0 / g_tot  * g_tot / (gtot_t + gabs_t)
			w[k] = 1.0 / (gtot_t + gabs_t);
			one_m_w[k] = gabs_t / (gtot_t + gabs_t);
		}
		else
		{
			w[k] = 1.0 / gtot_t;
			one_m_w[k] = 1.0 - 1.0;
		}

		RHS[k] = RHS_t;
	}
    for (int k = 0; k < this->N(); k++)
        this->m_nets[k]->m_new_Analog = this->m_nets[k]->m_cur_Analog;

	do {
		resched = false;
		double cerr = 0.0;

		for (int k = 0; k < this->N(); k++)
		{
			netlist_analog_net_t *net = this->m_nets[k];
			const netlist_terminal_t::list_t &terms = net->m_terms;
			const int term_count = terms.count();

			double iIdr = RHS[k];

			for (int i = 0; i < term_count; i++)
			{
                iIdr += terms[i]->m_go * terms[i]->m_otherterm->net().as_analog().m_new_Analog;
			}

			//double new_val = (net->m_cur_Analog * gabs[k] + iIdr) / (gtot[k]);
			const double new_val = net->m_new_Analog * one_m_w[k] + iIdr * w[k];

			const double e = (new_val - net->m_new_Analog);
			cerr += e * e;

			net->m_new_Analog = new_val;
		}
		if (cerr > this->m_params.m_accuracy * this->m_params.m_accuracy)
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
	    int tmp = netlist_matrix_solver_direct_t<m_N, _storage_N>::vsolve_non_dynamic();
        this->m_calculations++;
        return tmp;
	}
    this->m_calculations++;

    for (int k = 0; k < this->N(); k++)
        this->m_nets[k]->m_cur_Analog = this->m_nets[k]->m_new_Analog;

	return resched_cnt;
}

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------



NETLIB_START(solver)
{
	register_output("Q_step", m_Q_step);

    register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(10).as_double());

	register_param("FREQ", m_freq, 48000.0);

	register_param("ACCURACY", m_accuracy, 1e-7);
	register_param("CONVERG", m_convergence, 0.3);
	register_param("GS_LOOPS", m_gs_loops, 7);              // Gauss-Seidel loops
    register_param("NR_LOOPS", m_nr_loops, 25);             // Newton-Raphson loops
	register_param("PARALLEL", m_parallel, 0);
    register_param("GMIN", m_gmin, NETLIST_GMIN_DEFAULT);
    register_param("DYNAMIC_TS", m_dynamic, 0);
	register_param("LTE", m_lte, 1e-3);                     // 1mV diff/timestep
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


ATTR_COLD void NETLIB_NAME(solver)::post_start()
{
	netlist_analog_net_t::list_t groups[100];
	int cur_group = -1;

    m_params.m_accuracy = m_accuracy.Value();
    m_params.m_convergence_factor = m_convergence.Value();
    m_params.m_gs_loops = m_gs_loops.Value();
    m_params.m_nr_loops = m_nr_loops.Value();
    m_params.m_nt_sync_delay = m_sync_delay.Value();
    m_params.m_lte = m_lte.Value();
    m_params.m_min_timestep = m_min_timestep.Value();
    m_params.m_dynamic = (m_dynamic.Value() == 1 ? true : false);
    m_params.m_max_timestep = netlist_time::from_hz(m_freq.Value()).as_double();

    if (m_params.m_dynamic)
    {
        m_params.m_max_timestep *= 100.0;
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
				ms = new netlist_matrix_solver_direct1_t();
				break;
			case 2:
				ms = new netlist_matrix_solver_direct2_t();
				break;
			case 3:
				ms = new netlist_matrix_solver_direct_t<3,3>();
				//ms = new netlist_matrix_solver_gauss_seidel_t<3,3>();
				break;
			case 4:
				ms = new netlist_matrix_solver_direct_t<4,4>();
				//ms = new netlist_matrix_solver_gauss_seidel_t<4,4>();
				break;
			case 5:
				ms = new netlist_matrix_solver_direct_t<5,5>();
				//ms = new netlist_matrix_solver_gauss_seidel_t<5,5>();
				break;
			case 6:
				ms = new netlist_matrix_solver_direct_t<6,6>();
				//ms = new netlist_matrix_solver_gauss_seidel_t<6,6>();
				break;
            case 7:
                //ms = new netlist_matrix_solver_direct_t<6,6>();
                ms = new netlist_matrix_solver_gauss_seidel_t<7,7>();
                break;
            case 8:
                //ms = new netlist_matrix_solver_direct_t<6,6>();
                ms = new netlist_matrix_solver_gauss_seidel_t<8,8>();
                break;
			default:
				if (net_count <= 16)
				{
				    //ms = new netlist_matrix_solver_direct_t<0,16>();
					ms = new netlist_matrix_solver_gauss_seidel_t<0,16>();
				}
				else if (net_count <= 32)
				{
					//ms = new netlist_matrix_solver_direct_t<0,16>();
					ms = new netlist_matrix_solver_gauss_seidel_t<0,32>();
				}
				else if (net_count <= 64)
				{
					//ms = new netlist_matrix_solver_direct_t<0,16>();
					ms = new netlist_matrix_solver_gauss_seidel_t<0,64>();
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

        ms->vsetup(groups[i], *this);

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

