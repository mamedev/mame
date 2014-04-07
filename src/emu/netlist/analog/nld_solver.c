/*
 * nld_solver.c
 *
 */

#include "nld_solver.h"
#include "nld_twoterm.h"
#include "../nl_lists.h"

#if HAS_OPENMP
#include "omp.h"
#endif

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver
// ----------------------------------------------------------------------------------------

#define SOLVER_VERBOSE_OUT(x) do {} while (0)
//#define SOLVER_VERBOSE_OUT(x) printf x

ATTR_COLD void netlist_matrix_solver_t::setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &aowner)
{
	m_owner = &aowner;

	NL_VERBOSE_OUT(("New solver setup\n"));

	for (netlist_net_t * const * pn = nets.first(); pn != NULL; pn = nets.next(pn))
	{
		NL_VERBOSE_OUT(("setting up net\n"));

		m_nets.add(*pn);

		(*pn)->m_solver = this;

		for (netlist_core_terminal_t *p = (*pn)->m_list.first(); p != NULL; p = (*pn)->m_list.next(p))
		{
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
						netlist_terminal_t *pterm = static_cast<netlist_terminal_t *>(p);
						if (pterm->m_otherterm->net().isRailNet())
							(*pn)->m_rails.add(pterm);
						else
							(*pn)->m_terms.add(pterm);
					}
					NL_VERBOSE_OUT(("Added terminal\n"));
					break;
				case netlist_terminal_t::INPUT:
					if (!m_inps.contains(p))
						m_inps.add(p);
					NL_VERBOSE_OUT(("Added input\n"));
					break;
				default:
					owner().netlist().error("unhandled element found\n");
					break;
			}
		}
		NL_VERBOSE_OUT(("added net with %d populated connections (%d railnets)\n", (*pn)->m_terms.count(), (*pn)->m_rails.count()));
	}
}

ATTR_HOT void netlist_matrix_solver_t::update_inputs()
{
	for (netlist_core_terminal_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
	{
		if ((*p)->net().m_last_Analog != (*p)->net().m_cur_Analog)
		{
			(*p)->netdev().update_dev();
		}
	}
	for (netlist_core_terminal_t * const *p = m_inps.first(); p != NULL; p = m_inps.next(p))
	{
		(*p)->net().m_last_Analog = (*p)->net().m_cur_Analog;
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

ATTR_HOT void netlist_matrix_solver_t::schedule()
{
	if (!solve())
	{
		// NL_VERBOSE_OUT(("update_inputs\n");
		update_inputs();
	}
	else
	{
		m_owner->netlist().warning("Matrix solver reschedule .. Consider increasing RESCHED_LOOPS");
		if (m_owner != NULL)
			this->m_owner->schedule();
	}
	//solve();
	//    update_inputs();
}

ATTR_COLD void netlist_matrix_solver_t::reset()
{
	m_last_step = netlist_time::zero;
}

ATTR_HOT void netlist_matrix_solver_t::step(const netlist_time delta)
{
	const double dd = delta.as_double();
	for (int k=0; k < m_steps.count(); k++)
		m_steps[k]->step_time(dd);
}

ATTR_HOT bool netlist_matrix_solver_t::solve()
{
	int  resched_cnt = 0;

	netlist_time now = owner().netlist().time();
	netlist_time delta = now - m_last_step;

	if (delta < netlist_time::from_nsec(1)) // always update capacitors
		delta = netlist_time::from_nsec(1);
	{
		NL_VERBOSE_OUT(("Step!\n"));
		/* update all terminals for new time step */
		m_last_step = now;
		step(delta);
	}

	if (is_dynamic())
	{
		int this_resched;
		do
		{
			update_dynamic();
			this_resched = solve_non_dynamic();
			resched_cnt += this_resched;
		} while (this_resched > 1 && resched_cnt < m_params.m_resched_loops);
	}
	else
	{
		resched_cnt = solve_non_dynamic();
		//printf("resched_cnt %d %d\n", resched_cnt, m_resched_loops);
	}
	return (resched_cnt >= m_params.m_resched_loops);
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
ATTR_COLD void netlist_matrix_solver_direct_t<m_N, _storage_N>::setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &owner)
{
	netlist_matrix_solver_t::setup(nets, owner);

	m_term_num = 0;
	m_rail_start = 0;
	for (int k = 0; k < N(); k++)
	{
		netlist_net_t *net = m_nets[k];
		const netlist_net_t::terminal_list_t &terms = net->m_terms;
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
		netlist_net_t *net = m_nets[k];
		const netlist_net_t::terminal_list_t &terms = net->m_terms;
		const netlist_net_t::terminal_list_t &rails = net->m_rails;
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
		m_RHS[t.net_this] += t.term->m_Idr;
		m_A[t.net_this][t.net_this] += t.term->m_gt;
		if (t.net_other >= 0)
		{
			//m_A[t.net_other][t.net_other] += t.term->m_otherterm->m_gt;
			m_A[t.net_this][t.net_other] += -t.term->m_go;
			//m_A[t.net_other][t.net_this] += -t.term->m_otherterm->m_go;
		}
		else
			m_RHS[t.net_this] += t.term->m_go * t.term->m_otherterm->net().Q_Analog();
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

		RHS[t.net_this] += t.term->m_go * t.term->m_otherterm->net().Q_Analog();
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

	for (int i = 0; i < N(); i++) {
#if 0
		/* Find the row with the largest first value */
		maxrow = i;
		for (j=i+1;j<n;j++) {
			if (ABS(a[i][j]) > ABS(a[i][maxrow]))
				maxrow = j;
		}

		/* Swap the maxrow and ith row */
		for (k=i;k<n+1;k++) {
			tmp = a[k][i];
			a[k][i] = a[k][maxrow];
			a[k][maxrow] = tmp;
		}
#endif
		/* Singular matrix? */
		double f = A[i][i];
		//if (fabs(f) < 1e-20) printf("Singular!");
		f = 1.0 / f;

		/* Eliminate column i from row j */
		for (int j = i + 1; j < N(); j++)
		{
			double f1 = A[j][i] * f;

			if (f1 != 0.0)
			{
				for (int k = i; k < N(); k++)
				{
					A[j][k] -= A[i][k] * f1;
				}
				RHS[j] -= RHS[i] * f1;
			}
		}
	}
	/* back substitution */
	for (int j = N() - 1; j >= 0; j--)
	{
		double tmp = 0;
		for (int k = j + 1; k < N(); k++)
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
ATTR_HOT int netlist_matrix_solver_direct_t<m_N, _storage_N>::solve_non_dynamic()
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

ATTR_HOT int netlist_matrix_solver_direct1_t::solve_non_dynamic()
{
#if 1

	double gtot_t = 0.0;
	double RHS_t = 0.0;

	netlist_net_t *net = m_nets[0];
	const netlist_net_t::terminal_list_t &rails = net->m_rails;
	int rail_count = rails.count();

	for (int i = 0; i < rail_count; i++)
	{
		gtot_t += rails[i]->m_gt;
		RHS_t += rails[i]->m_Idr;
		RHS_t += rails[i]->m_go * rails[i]->m_otherterm->net().Q_Analog();
	}

	double iIdr = RHS_t;
	double new_val = iIdr / gtot_t;

#else
	netlist_net_t *net = m_nets[0];
	double m_A[1][1] = { {0.0} };
	double m_RHS[1] = { 0.0 };
	build_LE(m_A, m_RHS);
	//NL_VERBOSE_OUT(("%f %f\n", new_val, m_RHS[0] / m_A[0][0]);

	double new_val =  m_RHS[0] / m_A[0][0];
#endif
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

ATTR_HOT int netlist_matrix_solver_direct2_t::solve_non_dynamic()
{
	double A[2][2] = { { 0.0 } };
	double RHS[2] = { 0.0 };

	build_LE(A, RHS);

	//NL_VERBOSE_OUT(("%f %f\n", new_val, m_RHS[0] / m_A[0][0]);

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
ATTR_HOT int netlist_matrix_solver_gauss_seidel_t<m_N, _storage_N>::solve_non_dynamic()
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

	for (int k = 0; k < N(); k++)
	{
		double gtot_t = 0.0;
		double gabs_t = 0.0;
		double RHS_t = 0.0;

		netlist_net_t *net = m_nets[k];
		const netlist_net_t::terminal_list_t &terms = net->m_terms;
		const netlist_net_t::terminal_list_t &rails = net->m_rails;
		const int term_count = terms.count();
		const int rail_count = rails.count();

		for (int i = 0; i < rail_count; i++)
		{
			gtot_t += rails[i]->m_gt;
			gabs_t += fabs(rails[i]->m_go);
			RHS_t += rails[i]->m_Idr;
			RHS_t += rails[i]->m_go * rails[i]->m_otherterm->net().Q_Analog();
		}

		for (int i = 0; i < term_count; i++)
		{
			gtot_t += terms[i]->m_gt;
			gabs_t += fabs(terms[i]->m_go);
			RHS_t += terms[i]->m_Idr;
		}

		gabs_t *= m_params.m_convergence_factor;
		if (gabs_t > gtot_t)
		{
			// Actually 1.0 / g_tot  * g_tot / (gtot_t + gabs_t)
			w[k] = 1.0 / (gtot_t + gabs_t);
			one_m_w[k] = gabs_t / (gtot_t + gabs_t);
		}
		else
		{
			w[k] = 1.0 / gtot_t;
			one_m_w[k] = 0.0;
		}

		RHS[k] = RHS_t;
	}

	//NL_VERBOSE_OUT(("%f %d\n", w, m_nets.count());
	do {
		resched = false;
		double cerr = 0.0;

		for (int k = 0; k < N(); k++)
		{
			netlist_net_t *net = m_nets[k];
			const netlist_net_t::terminal_list_t &terms = net->m_terms;
			const int term_count = terms.count();

			double iIdr = RHS[k];

			for (int i = 0; i < term_count; i++)
			{
				iIdr += terms[i]->m_go * terms[i]->m_otherterm->net().Q_Analog();
			}

			//double new_val = (net->m_cur_Analog * gabs[k] + iIdr) / (gtot[k]);
			double new_val = net->m_cur_Analog * one_m_w[k] + iIdr * w[k];

			double e = (new_val - net->m_cur_Analog);
			cerr += e * e;

			net->m_cur_Analog = net->m_new_Analog = new_val;
		}
		if (resched || cerr / m_nets.count() > m_params.m_accuracy * m_params.m_accuracy)
		{
			resched = true;
			//last_resched_net = net;
		}
		resched_cnt++;
	} while (resched && (resched_cnt < m_params.m_resched_loops / 3 ));

	if (resched)
		return m_fallback.solve_non_dynamic();

	return resched_cnt;
}

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

typedef netlist_net_t::list_t  *net_groups_t;

ATTR_COLD static bool already_processed(net_groups_t groups, int &cur_group, netlist_net_t *net)
{
	if (net->isRailNet())
		return true;
	for (int i = 0; i <= cur_group; i++)
	{
		if (groups[i].contains(net))
			return true;
	}
	return false;
}

ATTR_COLD static void process_net(net_groups_t groups, int &cur_group, netlist_net_t *net)
{
	if (net->m_list.is_empty())
		return;
	/* add the net */
	SOLVER_VERBOSE_OUT(("add %d - %s\n", cur_group, net->name().cstr()));
	groups[cur_group].add(net);
	for (netlist_core_terminal_t *p = net->m_list.first(); p != NULL; p = net->m_list.next(p))
	{
		SOLVER_VERBOSE_OUT(("terminal %s\n", p->name().cstr()));
		if (p->isType(netlist_terminal_t::TERMINAL))
		{
			SOLVER_VERBOSE_OUT(("isterminal\n"));
			netlist_terminal_t *pt = static_cast<netlist_terminal_t *>(p);
			netlist_net_t *other_net = &pt->m_otherterm->net();
			if (!already_processed(groups, cur_group, other_net))
				process_net(groups, cur_group, other_net);
		}
	}
}


NETLIB_START(solver)
{
	register_output("Q_sync", m_Q_sync);
	register_output("Q_step", m_Q_step);
	//register_input("FB", m_feedback);

	register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(5).as_double());
	m_nt_sync_delay = m_sync_delay.Value();

	register_param("FREQ", m_freq, 48000.0);
	m_inc = netlist_time::from_hz(m_freq.Value());

	register_param("ACCURACY", m_accuracy, 1e-7);
	register_param("CONVERG", m_convergence, 0.3);
	register_param("RESCHED_LOOPS", m_resched_loops, 35);
	register_param("PARALLEL", m_parallel, 0);
	register_param("GMIN", m_gmin, NETLIST_GMIN_DEFAULT);

	// internal staff

	register_input("FB_sync", m_fb_sync);
	register_input("FB_step", m_fb_step);

	connect(m_fb_sync, m_Q_sync);
	connect(m_fb_step, m_Q_step);

	save(NAME(m_last_step));

}

NETLIB_RESET(solver)
{
	m_last_step = netlist_time::zero;
	for (int i = 0; i < m_mat_solvers.count(); i++)
		m_mat_solvers[i]->reset();
}


NETLIB_UPDATE_PARAM(solver)
{
	m_inc = netlist_time::from_hz(m_freq.Value());
}

NETLIB_NAME(solver)::~NETLIB_NAME(solver)()
{
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
	netlist_time now = netlist().time();
	netlist_time delta = now - m_last_step;
	bool do_full = false;
	bool global_resched = false;
	bool this_resched[100];
	int t_cnt = m_mat_solvers.count();

	if (delta < m_inc)
		do_full = true; // we have been called between updates

	m_last_step = now;

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
	for (int i = 0; i < t_cnt; i++)
	{
		if (do_full || (m_mat_solvers[i]->is_timestep()))
			this_resched[i] = m_mat_solvers[i]->solve();
	}
#endif

	for (int i = 0; i < t_cnt; i++)
	{
		if (do_full || m_mat_solvers[i]->is_timestep())
		{
			global_resched = global_resched || this_resched[i];
			if (!this_resched[i])
				m_mat_solvers[i]->update_inputs();
		}
	}

	if (global_resched)
	{
		netlist().warning("Gobal reschedule .. Consider increasing RESCHED_LOOPS");
		schedule();
	}
	else
	{
		/* step circuit */
		if (!m_Q_step.net().is_queued())
			m_Q_step.net().push_to_queue(m_inc);
	}

}

ATTR_COLD void NETLIB_NAME(solver)::post_start()
{
	netlist_net_t::list_t groups[100];
	int cur_group = -1;

	SOLVER_VERBOSE_OUT(("Scanning net groups ...\n"));
	// determine net groups
	for (netlist_net_t * const *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
	{
		NL_VERBOSE_OUT(("proc %s\n", (*pn)->name().cstr()));
		if (!already_processed(groups, cur_group, *pn))
		{
			cur_group++;
			process_net(groups, cur_group, *pn);
		}
	}

	// setup the solvers
	SOLVER_VERBOSE_OUT(("Found %d net groups in %d nets\n", cur_group + 1, netlist().m_nets.count()));
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
#if 0
			case 5:
				//ms = new netlist_matrix_solver_direct_t<5,5>();
				ms = new netlist_matrix_solver_gauss_seidel_t<5,5>();
				break;
			case 6:
				//ms = new netlist_matrix_solver_direct_t<6,6>();
				ms = new netlist_matrix_solver_gauss_seidel_t<6,6>();
				break;
#endif
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

		ms->m_params.m_accuracy = m_accuracy.Value();
		ms->m_params.m_convergence_factor = m_convergence.Value();
		ms->m_params.m_resched_loops = m_resched_loops.Value();
		ms->setup(groups[i], *this);
		m_mat_solvers.add(ms);
		SOLVER_VERBOSE_OUT(("%d ==> %d nets %s\n", i, groups[i].count(), (*groups[i].first())->m_head->name().cstr()));
		SOLVER_VERBOSE_OUT(("       has %s elements\n", ms->is_dynamic() ? "dynamic" : "no dynamic"));
		SOLVER_VERBOSE_OUT(("       has %s elements\n", ms->is_timestep() ? "timestep" : "no timestep"));
		for (int j=0; j<groups[i].count(); j++)
		{
			SOLVER_VERBOSE_OUT(("Net %d: %s\n", j, groups[i][j]->name().cstr()));
			netlist_net_t *n = groups[i][j];
			for (netlist_core_terminal_t *p = n->m_list.first(); p != NULL; p = n->m_list.next(p))
			{
				SOLVER_VERBOSE_OUT(("   %s\n", p->name().cstr()));
			}
		}
	}

}
