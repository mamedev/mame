/*
 * nld_system.c
 *
 */

#include "nld_system.h"

// ----------------------------------------------------------------------------------------
// netdev_const
// ----------------------------------------------------------------------------------------

NETLIB_START(ttl_const)
{
    register_output("Q", m_Q);
    register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(ttl_const)
{
}

NETLIB_UPDATE_PARAM(ttl_const)
{
    OUTLOGIC(m_Q, m_const.ValueInt(), NLTIME_IMMEDIATE);
}

NETLIB_START(analog_const)
{
    register_output("Q", m_Q);
    register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(analog_const)
{
}

NETLIB_UPDATE_PARAM(analog_const)
{
    m_Q.initial(m_const.Value());
}

// ----------------------------------------------------------------------------------------
// analog_callback
// ----------------------------------------------------------------------------------------

NETLIB_UPDATE(analog_callback)
{
    // FIXME: Remove after device cleanup
    if (!m_callback.isnull())
        m_callback(INPANALOG(m_in));
}

// ----------------------------------------------------------------------------------------
// clock
// ----------------------------------------------------------------------------------------

NETLIB_START(clock)
{
    register_output("Q", m_Q);
    //register_input("FB", m_feedback);

    register_param("FREQ", m_freq, 7159000.0 * 5);
    m_inc = netlist_time::from_hz(m_freq.Value()*2);

    register_link_internal(m_feedback, m_Q, netlist_input_t::STATE_INP_ACTIVE);

}

NETLIB_UPDATE_PARAM(clock)
{
    m_inc = netlist_time::from_hz(m_freq.Value()*2);
}

NETLIB_UPDATE(clock)
{
    OUTLOGIC(m_Q, !m_Q.net().new_Q(), m_inc  );
}

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

NETLIB_START(solver)
{
    register_output("Q", m_Q);
    //register_input("FB", m_feedback);

    register_param("FREQ", m_freq, 50000);
    m_inc = netlist_time::from_hz(m_freq.Value());

    register_link_internal(m_feedback, m_Q, netlist_input_t::STATE_INP_ACTIVE);
    m_last_step = netlist_time::zero;

}

NETLIB_UPDATE_PARAM(solver)
{
    m_inc = netlist_time::from_hz(m_freq.Value());
}

NETLIB_FUNC_VOID(solver, post_start, ())
{
    NL_VERBOSE_OUT(("post start solver ...\n"));
    for (netlist_net_t **pn = m_nets.first(); pn != NULL; pn = m_nets.next(pn))
    {
        NL_VERBOSE_OUT(("setting up net\n"));
        for (netlist_terminal_t *p = (*pn)->m_head; p != NULL; p = p->m_update_list_next)
        {
            switch (p->type())
            {
                case netlist_terminal_t::TERMINAL:
                    m_terms.add(p);
                    NL_VERBOSE_OUT(("Added terminal\n"));
                    break;
                case netlist_terminal_t::INPUT:
                    m_inps.add(p);
                    NL_VERBOSE_OUT(("Added input\n"));
                    break;
                default:
                    fatalerror("unhandled element found\n");
                    break;
            }
        }
        if ((*pn)->m_head == NULL)
        {
            NL_VERBOSE_OUT(("Deleting net ...\n"));
            netlist_net_t *to_delete = *pn;
            m_nets.del(to_delete);
            delete to_delete;
            pn--;
        }
    }
}

NETLIB_UPDATE(solver)
{
    //m_Q.setToNoCheck(!m_Q.new_Q(), m_inc  );
    //OUTLOGIC(m_Q, !m_Q.net().new_Q(), m_inc  );

    bool resched = false;
    netlist_time now = netlist().time();
    netlist_time delta = now - m_last_step;

    if (delta >= m_inc)
    {
        NL_VERBOSE_OUT(("Step!\n"));
        /* update all terminals for new time step */
        m_last_step = now;
        for (netlist_terminal_t **p = m_terms.first(); p != NULL; p = m_terms.next(p))
            (*p)->netdev().step_time(delta.as_double());
    }
    for (netlist_net_t **pn = m_nets.first(); pn != NULL; pn = m_nets.next(pn))
    {
        double gtot = 0;
        double iIdr = 0;

        for (netlist_terminal_t *p = (*pn)->m_head; p != NULL; p = p->m_update_list_next)
        {
            if (p->isType(netlist_terminal_t::TERMINAL))
            {
                p->netdev().update_terminals();
                gtot += p->m_g;
                iIdr += p->m_Idr;
            }
        }

        double new_val = iIdr / gtot;
        if (fabs(new_val - (*pn)->m_cur.Analog) > 1e-4)
            resched = true;
        (*pn)->m_cur.Analog = (*pn)->m_new.Analog = new_val;

        NL_VERBOSE_OUT(("Info: %d\n", (*pn)->m_num_cons));
        NL_VERBOSE_OUT(("New: %lld %f %f\n", netlist().time().as_raw(), netlist().time().as_double(), new_val));
    }
    if (resched)
    {
        schedule();
    }
    else
    {
        /* update all inputs connected */
        for (netlist_terminal_t **p = m_inps.first(); p != NULL; p = m_inps.next(p))
            (*p)->netdev().update_dev();
        /* step circuit */
        if (!m_Q.net().is_queued())
        {
            m_Q.net().push_to_queue(m_inc);
        }
    }

        /* only inputs and terminals connected
         * approach:
         *
         * a) Update voltage on this net
         * b) Update devices
         * c) If difference old - new > trigger schedule immediate update
         *    of number of updates < max_update_count
         *    else clear number of updates
         */

}
