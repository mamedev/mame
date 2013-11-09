/*
 * nld_system.c
 *
 */

#include "nld_system.h"

// ----------------------------------------------------------------------------------------
// netdev_const
// ----------------------------------------------------------------------------------------

NETLIB_START(netdev_ttl_const)
{
    register_output("Q", m_Q);
    register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(netdev_ttl_const)
{
}

NETLIB_UPDATE_PARAM(netdev_ttl_const)
{
    OUTLOGIC(m_Q, m_const.ValueInt(), NLTIME_IMMEDIATE);
}

NETLIB_START(netdev_analog_const)
{
    register_output("Q", m_Q);
    register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(netdev_analog_const)
{
}

NETLIB_UPDATE_PARAM(netdev_analog_const)
{
    m_Q.initial(m_const.Value());
}

NETLIB_UPDATE(netdev_analog_callback)
{
    // FIXME: Remove after device cleanup
    if (!m_callback.isnull())
        m_callback(INPANALOG(m_in));
}

