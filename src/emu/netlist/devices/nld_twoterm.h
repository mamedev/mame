// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_twoterm.h
 *
 * Devices with two terminals ...
 *
 *
 *       (k)
 *  +-----T-----+
 *  |     |     |
 *  |  +--+--+  |
 *  |  |     |  |
 *  |  R     |  |
 *  |  R     |  |
 *  |  R     I  |
 *  |  |     I  |  Device n
 *  |  V+    I  |
 *  |  V     |  |
 *  |  V-    |  |
 *  |  |     |  |
 *  |  +--+--+  |
 *  |     |     |
 *  +-----T-----+
 *       (l)
 *
 *  This is a resistance in series to a voltage source and paralleled by a current source.
 *  This is suitable to model voltage sources, current sources, resistors, capacitors,
 *  inductances and diodes.
 *
 */

#ifndef NLD_TWOTERM_H_
#define NLD_TWOTERM_H_

#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_R(_name, _R)                                                         \
        NET_REGISTER_DEV(R, _name)                                                  \
        NETDEV_PARAMI(_name, R, _R)

#define NETDEV_C(_name, _C)                                                         \
        NET_REGISTER_DEV(C, _name)                                                  \
        NETDEV_PARAMI(_name, C, _C)

/* Generic Diode */
#define NETDEV_D(_name,  _model)                                                    \
        NET_REGISTER_DEV(D, _name)                                                  \
        NETDEV_PARAMI(_name, model, _model)

#define NETDEV_1N914(_name) NETDEV_D(_name, "Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon")

// ----------------------------------------------------------------------------------------
// Implementation
// ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

class nld_twoterm : public netlist_device_t
{
public:
    nld_twoterm()
    : netlist_device_t(), m_g(0.0), m_V(0.0), m_I(0.0) { }

    netlist_terminal_t m_P;
    netlist_terminal_t m_N;

protected:
    virtual void start();

    virtual NETLIB_UPDATE_TERMINALS()
    {
        m_P.m_g = m_N.m_g = m_g;
        m_N.m_Idr = (m_P.net().Q_Analog() - m_V) * m_g + m_I;
        m_P.m_Idr = (m_N.net().Q_Analog() + m_V) * m_g - m_I;
        //printf("%f %f %f %f\n", m_N.m_Idr, m_P.m_Idr, m_N.net().Q_Analog(), m_P.net().Q_Analog());
    }

    ATTR_HOT ATTR_ALIGN void update();

    double m_g; // conductance
    double m_V; // internal voltage source
    double m_I; // internal current source
private:
};

// ----------------------------------------------------------------------------------------
// nld_R
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS_DERIVED(R, twoterm,
        netlist_param_double_t m_R;

    NETLIB_UPDATE_TERMINALS() { NETLIB_NAME(twoterm)::update_terminals(); }

public:
    inline void set_R(double R) { m_g = 1.0 / R; }

);

// ----------------------------------------------------------------------------------------
// nld_C
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS_DERIVED(C, twoterm,

    netlist_param_double_t m_C;

    ATTR_HOT void step_time(const double st)
    {
        m_g = m_P.m_g = m_N.m_g = m_C.Value() / st;
        m_I = -m_g * (m_P.net().Q_Analog()- m_N.net().Q_Analog());
    }

);

// ----------------------------------------------------------------------------------------
// nld_D
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS_DERIVED(D, twoterm,

    netlist_param_multi_t m_model;

    double m_Vt;
    double m_Is;
    double m_n;

    double m_VtInv;
    double m_Vcrit;
    double m_Vd;

    NETLIB_UPDATE_TERMINALS()
    {
        const double nVd = m_P.net().Q_Analog()- m_N.net().Q_Analog();

        //FIXME: Optimize cutoff case
        m_Vd = (nVd > m_Vcrit) ? m_Vd + log((nVd - m_Vd) * m_VtInv + 1.0) * m_Vt : nVd;

        const double eVDVt = exp(m_Vd * m_VtInv);
        const double Id = m_Is * (eVDVt - 1.0);

        m_g = m_Is * m_VtInv * eVDVt;

        m_I = (Id - m_Vd * m_g);
        //printf("Vd: %f %f %f %f\n", m_Vd, m_g, Id, m_I);

        nld_twoterm::update_terminals();
    }

private:
);


#endif /* NLD_TWOTERM_H_ */
