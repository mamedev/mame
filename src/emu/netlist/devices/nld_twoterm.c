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

class diode
{
public:
    diode() : m_Is(1e-15), m_VT(0.0258), m_VT_inv(1.0 / m_VT) {}
    diode(const double Is, const double n)
    {
        m_Is = Is;
        m_VT = 0.0258 * n;
        m_VT_inv = 1.0 / m_VT;
    }
    void set(const double Is, const double n)
    {
        m_Is = Is;
        m_VT = 0.0258 * n;
        m_VT_inv = 1.0 / m_VT;
    }
    double I(const double V) const { return m_Is * exp(V * m_VT_inv) - m_Is; }
    double g(const double V) const { return m_Is * m_VT_inv * exp(V * m_VT_inv); }
    double V(const double I) const { return log(1.0 + I / m_Is) * m_VT; }
    double gI(const double I) const { return m_VT_inv * (I + m_Is); }

private:
    double m_Is;
    double m_VT;
    double m_VT_inv;
};

// ----------------------------------------------------------------------------------------
// nld_Q
// ----------------------------------------------------------------------------------------

NETLIB_START(Q)
{
    register_param("model", m_model, "");
}

NETLIB_START(QBJT)
{
    NETLIB_NAME(Q)::start();

    register_terminal("B", m_B);
    register_terminal("C", m_C);
    register_terminal("E", m_E);

}

NETLIB_UPDATE(Q)
{
    netlist().solver()->schedule();
}

template <NETLIB_NAME(Q)::q_type _type>
NETLIB_UPDATE_PARAM(QBJT_switch<_type>)
{
    double IS = m_model.dValue("IS", 1e-15);
    double BF = m_model.dValue("BF", 100);
    double NF = m_model.dValue("NF", 1);
    //double VJE = m_model.dValue("VJE", 0.75);

    double alpha = BF / (1.0 + BF);

    diode d(IS, NF);

    // Assume 5mA Collector current for switch operation

    if (_type == BJT_NPN)
        m_V = d.V(0.005 / alpha);
    else
        m_V = - d.V(0.005 / alpha);

    m_gB = d.gI(0.005 / alpha);
    if (m_gB < NETLIST_GMIN)
        m_gB = NETLIST_GMIN;
    m_gC = BF * m_gB; // very rough estimate
    printf("%f %f \n", m_V, m_gB);
}

template NETLIB_UPDATE_PARAM(QBJT_switch<NETLIB_NAME(Q)::BJT_NPN>);
template NETLIB_UPDATE_PARAM(QBJT_switch<NETLIB_NAME(Q)::BJT_PNP>);
