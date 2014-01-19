/*
 * nld_fourterm.c
 *
 */

#include "nld_fourterm.h"
#include "../nl_setup.h"
#include "nld_solver.h"

// ----------------------------------------------------------------------------------------
// nld_VCCS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCCS)
{
    configure(1.0, NETLIST_GMIN);
}

NETLIB_RESET(VCCS)
{
}

ATTR_COLD void NETLIB_NAME(VCCS)::configure(const double Gfac, const double GI)
{
    register_param("G", m_G, 1.0);

    register_terminal("IP", m_IP);
    register_terminal("IN", m_IN);
    register_terminal("OP", m_OP);
    register_terminal("ON", m_ON);

    m_OP1.init_object(*this, name() + ".OP1");
    m_ON1.init_object(*this, name() + ".ON1");

    const double m_mult = m_G.Value() * Gfac; // 1.0 ==> 1V ==> 1A
    m_IP.set(GI);
    m_IP.m_otherterm = &m_IN; // <= this should be NULL and terminal be filtered out prior to solving...
    m_IN.set(GI);
    m_IN.m_otherterm = &m_IP; // <= this should be NULL and terminal be filtered out prior to solving...

    m_OP.set(m_mult, 0.0);
    m_OP.m_otherterm = &m_IP;
    m_OP1.set(-m_mult, 0.0);
    m_OP1.m_otherterm = &m_IN;

    m_ON.set(-m_mult, 0.0);
    m_ON.m_otherterm = &m_IP;
    m_ON1.set(m_mult, 0.0);
    m_ON1.m_otherterm = &m_IN;

    connect(m_OP, m_OP1);
    connect(m_ON, m_ON1);
}

NETLIB_UPDATE_PARAM(VCCS)
{
}

NETLIB_UPDATE(VCCS)
{
    /* only called if connected to a rail net ==> notify the solver to recalculate */
    /* Big FIXME ... */
    m_IP.net().solve();
    m_IN.net().solve();
    m_OP.net().solve();
    m_ON.net().solve();
}

// ----------------------------------------------------------------------------------------
// nld_VCVS
// ----------------------------------------------------------------------------------------

NETLIB_START(VCVS)
{
    register_param("RO", m_RO, 1.0);

    const double gRO = 1.0 / m_RO.Value();

    configure(gRO, NETLIST_GMIN);

    m_OP2.init_object(*this, "OP2");
    m_ON2.init_object(*this, "ON2");

    m_OP2.set(gRO);
    m_ON2.set(gRO);
    m_OP2.m_otherterm = &m_ON2;
    m_ON2.m_otherterm = &m_OP2;

    connect(m_OP2, m_OP1);
    connect(m_ON2, m_ON1);
}

NETLIB_UPDATE_PARAM(VCVS)
{
}

