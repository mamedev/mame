/*
 * nld_solver.c
 *
 */

#include "nld_solver.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// netlist_matrix_solver
// ----------------------------------------------------------------------------------------

#define SOLVER_VERBOSE_OUT(x) do {} while (0)
//#define SOLVER_VERBOSE_OUT(x) printf x

ATTR_COLD void netlist_matrix_solver_t::setup(netlist_net_t::list_t &nets, NETLIB_NAME(solver) &aowner)
{
	m_owner = &aowner;
	for (netlist_net_t::list_t::entry_t *pn = nets.first(); pn != NULL; pn = nets.next(pn))
	{
		NL_VERBOSE_OUT(("setting up net\n"));

		m_nets.add(pn->object());
		pn->object()->m_solver = this;

		for (netlist_core_terminal_t *p = pn->object()->m_head; p != NULL; p = p->m_update_list_next)
		{
			switch (p->type())
			{
				case netlist_terminal_t::TERMINAL:
					switch (p->netdev().family())
					{
						case netlist_device_t::CAPACITOR:
							if (!m_steps.contains(&p->netdev()))
								m_steps.add(&p->netdev());
							break;
						case netlist_device_t::DIODE:
						//case netlist_device_t::VCVS:
						//case netlist_device_t::BJT_SWITCH:
							if (!m_dynamic.contains(&p->netdev()))
								m_dynamic.add(&p->netdev());
							break;
						default:
							break;
					}
					pn->object()->m_terms.add(static_cast<netlist_terminal_t *>(p));
					NL_VERBOSE_OUT(("Added terminal\n"));
					break;
				case netlist_terminal_t::INPUT:
					if (!m_inps.contains(&p->netdev()))
						m_inps.add(&p->netdev());
					NL_VERBOSE_OUT(("Added input\n"));
					break;
				default:
					owner().netlist().xfatalerror("unhandled element found\n");
					break;
			}
		}
	}
}

ATTR_HOT inline void netlist_matrix_solver_t::step(const netlist_time delta)
{
	const double dd = delta.as_double();
	for (dev_list_t::entry_t *p = m_steps.first(); p != NULL; p = m_steps.next(p))
		p->object()->step_time(dd);
}

ATTR_HOT inline void netlist_matrix_solver_t::update_inputs()
{
	for (dev_list_t::entry_t *p = m_inps.first(); p != NULL; p = m_inps.next(p))
		p->object()->update_dev();
}


ATTR_HOT inline bool netlist_matrix_solver_t::solve()
{
	bool resched = false;

	/* update all non-linear devices  */
	for (dev_list_t::entry_t *p = m_dynamic.first(); p != NULL; p = m_dynamic.next(p))
		switch (p->object()->family())
		{
			case netlist_device_t::DIODE:
				static_cast<NETLIB_NAME(D) *>(p->object())->update_terminals();
				break;
			default:
				p->object()->update_terminals();
				break;
		}

	for (netlist_net_t::list_t::entry_t *pn = m_nets.first(); pn != NULL; pn = m_nets.next(pn))
	{
		netlist_net_t *net = pn->object();

		double gtot = 0;
		double gabs = 0;
		double iIdr = 0;
		const netlist_net_t::terminal_list_t &terms = net->m_terms;
#if 1
		switch (terms.count())
		{
			case 1:
				{
					const netlist_terminal_t *pt = terms.first()->object();
					gtot = pt->m_gt;
					gabs = fabs(pt->m_go);
					iIdr = pt->m_Idr + pt->m_go * pt->m_otherterm->net().Q_Analog();
				}
				break;
			case 2:
				{
					const netlist_terminal_t *pt1 = terms[0];
					const netlist_terminal_t *pt2 = terms[1];
					gtot = pt1->m_gt + pt2->m_gt;
					gabs = fabs(pt1->m_go) + fabs(pt2->m_go);
					iIdr = pt1->m_Idr + pt1->m_go * pt1->m_otherterm->net().Q_Analog()
							+ pt2->m_Idr + pt2->m_go * pt2->m_otherterm->net().Q_Analog();
				}
				break;
			case 3:
				{
					const netlist_terminal_t *pt1 = terms[0];
					const netlist_terminal_t *pt2 = terms[1];
					const netlist_terminal_t *pt3 = terms[2];
					gtot = pt1->m_gt + pt2->m_gt + pt3->m_gt;
					gabs = fabs(pt1->m_go) + fabs(pt2->m_go) + fabs(pt3->m_go);
					iIdr = pt1->m_Idr + pt1->m_go * pt1->m_otherterm->net().Q_Analog()
							+ pt2->m_Idr + pt2->m_go * pt2->m_otherterm->net().Q_Analog()
							+ pt3->m_Idr + pt3->m_go * pt3->m_otherterm->net().Q_Analog();
				}
				break;
			default:
				for (netlist_net_t::terminal_list_t::entry_t *e = terms.first(); e != NULL; e = terms.next(e))
				{
					netlist_terminal_t *pt = e->object();
					gtot += pt->m_gt;
					gabs += fabs(pt->m_go);
					iIdr += pt->m_Idr + pt->m_go * pt->m_otherterm->net().Q_Analog();
				}
				break;
		}
#else
		for (netlist_net_t::terminal_list_t::entry_t *e = terms.first(); e != NULL; e = terms.next(e))
		{
			netlist_terminal_t *pt = e->object();
			gtot += pt->m_gt;
			gabs += fabs(pt->m_go);
			iIdr += pt->m_Idr + pt->m_go * pt->m_otherterm->net().Q_Analog();
		}
#endif
		double new_val;
		gabs *= m_convergence_factor;
		if (gabs > gtot)
			new_val = (net->m_cur.Analog * gabs + iIdr) / (gtot + gabs);
		else
			new_val = iIdr / gtot;

		if (fabs(new_val - net->m_cur.Analog) > m_accuracy)
			resched = true;
		net->m_cur.Analog = net->m_new.Analog = new_val;

		NL_VERBOSE_OUT(("Info: %d\n", pn->object()->m_num_cons));
		//NL_VERBOSE_OUT(("New: %lld %f %f\n", netlist().time().as_raw(), netlist().time().as_double(), new_val));
	}
	return resched;
}

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

typedef netlist_net_t::list_t  *net_groups_t;

static bool already_processed(net_groups_t groups, int &cur_group, netlist_net_t *net)
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

static void process_net(net_groups_t groups, int &cur_group, netlist_net_t *net)
{
	/* add the net */
	if (net->m_head == NULL)
		return;
	groups[cur_group].add(net);
	for (netlist_core_terminal_t *p = net->m_head; p != NULL; p = p->m_update_list_next)
	{
		if (p->isType(netlist_terminal_t::TERMINAL))
		{
			netlist_terminal_t *pt = static_cast<netlist_terminal_t *>(p);
			netlist_net_t *nnet = &pt->m_otherterm->net();
			if (!already_processed(groups, cur_group, nnet))
				process_net(groups, cur_group, nnet);
		}
	}
}


NETLIB_START(solver)
{
	register_output("Q_sync", m_Q_sync);
	register_output("Q_step", m_Q_step);
	//register_input("FB", m_feedback);

	register_param("SYNC_DELAY", m_sync_delay, NLTIME_FROM_NS(10).as_double());
	m_nt_sync_delay = m_sync_delay.Value();

	register_param("FREQ", m_freq, 48000.0);
	m_inc = netlist_time::from_hz(m_freq.Value());

	register_param("ACCURACY", m_accuracy, 1e-3);
	register_param("CONVERG", m_convergence, 0.3);

	// internal staff

	register_input("FB_sync", m_fb_sync, netlist_input_t::STATE_INP_ACTIVE);
	register_input("FB_step", m_fb_step, netlist_input_t::STATE_INP_ACTIVE);

	setup().connect(m_fb_sync, m_Q_sync);
	setup().connect(m_fb_step, m_Q_step);

	m_last_step = netlist_time::zero;

	save(NAME(m_last_step));

}

NETLIB_UPDATE_PARAM(solver)
{
	m_inc = netlist_time::from_hz(m_freq.Value());
}

NETLIB_NAME(solver)::~NETLIB_NAME(solver)()
{
	netlist_matrix_solver_t::list_t::entry_t *e = m_mat_solvers.first();
	while (e != NULL)
	{
		netlist_matrix_solver_t::list_t::entry_t *en = m_mat_solvers.next(e);
		delete e->object();
		e = en;
	}

}

NETLIB_FUNC_VOID(solver, post_start, ())
{
	netlist_net_t::list_t groups[100];
	int cur_group = -1;

	SOLVER_VERBOSE_OUT(("Scanning net groups ...\n"));
	// determine net groups
	for (netlist_net_t::list_t::entry_t *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
	{
		if (!already_processed(groups, cur_group, pn->object()))
		{
			cur_group++;
			process_net(groups, cur_group, pn->object());
		}
	}

	// setup the solvers
	SOLVER_VERBOSE_OUT(("Found %d net groups in %d nets\n", cur_group + 1, m_nets.count()));
	for (int i = 0; i <= cur_group; i++)
	{
		netlist_matrix_solver_t *ms = new netlist_matrix_solver_t();
		ms->m_accuracy = m_accuracy.Value();
		ms->m_convergence_factor = m_convergence.Value();
		ms->setup(groups[i], *this);
		m_mat_solvers.add(ms);
		SOLVER_VERBOSE_OUT(("%d ==> %d nets %s\n", i, groups[i].count(), groups[i].first()->object()->m_head->name().cstr()));
		SOLVER_VERBOSE_OUT(("  has %s elements\n", ms->is_dynamic() ? "dynamic" : "no dynamic"));
	}

}

NETLIB_UPDATE(solver)
{
	//m_Q.setToNoCheck(!m_Q.new_Q(), m_inc  );
	//OUTLOGIC(m_Q, !m_Q.net().new_Q(), m_inc  );

	bool resched = false;
	int  resched_cnt = 0;
	netlist_time now = netlist().time();
	netlist_time delta = now - m_last_step;

	if (delta >= m_inc)
	{
		NL_VERBOSE_OUT(("Step!\n"));
		/* update all terminals for new time step */
		m_last_step = now;
		for (netlist_matrix_solver_t::list_t::entry_t *e = m_mat_solvers.first(); e != NULL; e = m_mat_solvers.next(e))
		{
			e->object()->step(delta);
		}
	}
	bool global_resched = false;
	for (netlist_matrix_solver_t::list_t::entry_t *e = m_mat_solvers.first(); e != NULL; e = m_mat_solvers.next(e))
	{
		resched_cnt = (e->object()->is_dynamic() ? 0 : 1);
		do {
			resched = e->object()->solve();
			resched_cnt++;
		} while ((resched && (resched_cnt < 5)) || (resched_cnt <= 1));
		global_resched = global_resched || resched;
	}
	//if (global_resched)
	//    printf("rescheduled\n");
	if (global_resched)
	{
		schedule();
	}
	else
	{
		/* update all inputs connected */
		for (netlist_matrix_solver_t::list_t::entry_t *e = m_mat_solvers.first(); e != NULL; e = m_mat_solvers.next(e))
		{
			e->object()->update_inputs();
		}

		/* step circuit */
		if (!m_Q_step.net().is_queued())
			m_Q_step.net().push_to_queue(m_inc);
	}

}
