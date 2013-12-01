/*
 * nld_twoterm.c
 *
 */

#include "nld_twoterm.h"
#include "nld_system.h"

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

NETLIB_START(twoterm)
{
}

NETLIB_UPDATE(twoterm)
{
    /* only called if connected to a rail net ==> notify the solver to recalculate */
    netlist().solver()->schedule();
}

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

NETLIB_START(R)
{
    register_terminal("1", m_P);
    register_terminal("2", m_N);

    register_param("R", m_R, NETLIST_GMIN);
}

NETLIB_UPDATE_PARAM(R)
{
    m_g = 1.0 / m_R.Value();
}

NETLIB_UPDATE(R)
{
    NETLIB_NAME(twoterm)::update();
}

// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

NETLIB_START(C)
{
    register_terminal("1", m_P);
    register_terminal("2", m_N);

    register_param("C", m_C, NETLIST_GMIN);
}

NETLIB_UPDATE_PARAM(C)
{
    // set to some very small step time for now
    step_time(1e-9);
}

NETLIB_UPDATE(C)
{
    NETLIB_NAME(twoterm)::update();
}

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------

NETLIB_START(D)
{
    register_terminal("A", m_P);
    register_terminal("K", m_N);
    register_param("model", m_model, "");

    m_Vd = 0.7;
}

NETLIB_UPDATE_PARAM(D)
{
    m_Is = m_model.dValue("Is", 1e-15);
    m_n = m_model.dValue("N", 1);

    m_Vt = 0.0258 * m_n;

    m_Vcrit = m_Vt * log(m_Vt / m_Is / sqrt(2.0));
    m_VtInv = 1.0 / m_Vt;
    NL_VERBOSE_OUT(("VCutoff: %f\n", m_Vcrit));
}

NETLIB_UPDATE(D)
{
    NETLIB_NAME(twoterm)::update();
}
