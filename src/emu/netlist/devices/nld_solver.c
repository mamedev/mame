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
					if (!m_inps.contains(p))
						m_inps.add(p);
					NL_VERBOSE_OUT(("Added input\n"));
					break;
				default:
					owner().netlist().error("unhandled element found\n");
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
	for (netlist_core_terminal_t::list_t::entry_t *p = m_inps.first(); p != NULL; p = m_inps.next(p))
	{
	    if (p->object()->net().m_last.Analog != p->object()->net().m_cur.Analog)
	    {
	        p->object()->netdev().update_dev();
	    }
	}
    for (netlist_core_terminal_t::list_t::entry_t *p = m_inps.first(); p != NULL; p = m_inps.next(p))
    {
        p->object()->net().m_last.Analog = p->object()->net().m_cur.Analog;
    }

}


ATTR_HOT inline bool netlist_matrix_solver_t::solve()
{
	bool resched;
	// FIXME: There may be situations where we *could* need more than one iteration for dynamic elements

    int  resched_cnt = (is_dynamic() ? /* 0 */ 1 : 1);
    ATTR_UNUSED netlist_net_t *last_resched_net = NULL;

    do {
        resched = false;
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
            const netlist_net_t::terminal_list_t &terms = net->m_terms;

            double gtot = 0;
            double gabs = 0;
            double iIdr = 0;
            double new_val;

            for (int i = 0; i < terms.count(); i++)
            {
                gtot += terms[i]->m_gt;
                gabs += fabs(terms[i]->m_go);
                iIdr += terms[i]->m_Idr + terms[i]->m_go * terms[i]->m_otherterm->net().Q_Analog();
            }

            gabs *= m_convergence_factor;
            if (gabs > gtot)
                new_val = (net->m_cur.Analog * gabs + iIdr) / (gtot + gabs);
            else
                new_val = iIdr / gtot;

            if (fabs(new_val - net->m_cur.Analog) > m_accuracy)
            {
                resched = true;
                last_resched_net = net;
            }

            net->m_cur.Analog = net->m_new.Analog = new_val;

            NL_VERBOSE_OUT(("Info: %d\n", pn->object()->m_num_cons));
            //NL_VERBOSE_OUT(("New: %lld %f %f\n", netlist().time().as_raw(), netlist().time().as_double(), new_val));
        }
        resched_cnt++;
    } while ((resched && (resched_cnt < m_resched_loops)) || (resched_cnt <= 1));

    if (!resched)
        update_inputs();
    //if (resched)
        //printf("Resched on net %s first term %s\n", last_resched_net->name().cstr(), last_resched_net->m_terms[0]->name().cstr());

	return resched;
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
    register_param("RESCHED_LOOPS", m_resched_loops, 15);

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

NETLIB_UPDATE(solver)
{
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
	    global_resched = global_resched || e->object()->solve();
	}
	if (global_resched)
	{
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
    for (netlist_net_t::list_t::entry_t *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
    {
        if (!already_processed(groups, cur_group, pn->object()))
        {
            cur_group++;
            process_net(groups, cur_group, pn->object());
        }
    }

    // setup the solvers
    SOLVER_VERBOSE_OUT(("Found %d net groups in %d nets\n", cur_group + 1, netlist().m_nets.count()));
    for (int i = 0; i <= cur_group; i++)
    {
        netlist_matrix_solver_t *ms = new netlist_matrix_solver_t();
        ms->m_accuracy = m_accuracy.Value();
        ms->m_convergence_factor = m_convergence.Value();
        ms->m_resched_loops = m_resched_loops.Value();
        ms->setup(groups[i], *this);
        m_mat_solvers.add(ms);
        SOLVER_VERBOSE_OUT(("%d ==> %d nets %s\n", i, groups[i].count(), groups[i].first()->object()->m_head->name().cstr()));
        SOLVER_VERBOSE_OUT(("       has %s elements\n", ms->is_dynamic() ? "dynamic" : "no dynamic"));
        SOLVER_VERBOSE_OUT(("       has %s elements\n", ms->is_timestep() ? "timestep" : "no timestep"));
    }

}
