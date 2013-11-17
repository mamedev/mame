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

    m_inc = netlist_time::from_hz(48000);

    register_link_internal(m_feedback, m_Q, netlist_input_t::STATE_INP_ACTIVE);

}

NETLIB_UPDATE_PARAM(solver)
{
    //m_inc = netlist_time::from_hz(m_freq.Value()*2);
}

NETLIB_UPDATE(solver)
{
    //m_Q.setToNoCheck(!m_Q.new_Q(), m_inc  );
    //OUTLOGIC(m_Q, !m_Q.net().new_Q(), m_inc  );

    netlist_net_t **pn = m_nets.first();
    bool resched = false;

    while (pn <= m_nets.last())
    {
        double gtot = 0;
        double iIdr = 0;

        netlist_terminal_t *p = (*pn)->m_head;
        do
        {
            p->netdev().update_terminals();
            gtot += p->m_g;
            iIdr += p->m_Idr;

            p = p->m_update_list_next;
        } while (p != NULL);
        (*pn)->m_new.Analog = iIdr / gtot;
        if (fabs((*pn)->m_new.Analog - (*pn)->m_cur.Analog) > 1e-4)
            resched = true;
        (*pn)->m_cur.Analog = (*pn)->m_new.Analog;

        //printf("New: %f\n", (*pn)->m_new.Analog);
        pn++;
    }
    if (resched)
        schedule();
    else
      m_Q.net().push_to_queue(m_inc); // step circuit

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
