/*
 * nld_bjt.h
 *
 */

#ifndef NLD_BJT_H_
#define NLD_BJT_H_

#include "../nl_base.h"
#include "nld_twoterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_QBJT_SW(_name, _model)                                           \
        NET_REGISTER_DEV(QBJT_switch, _name)                                    \
        NETDEV_PARAMI(_name,  model,   _model)

#define NETDEV_QBJT_EB(_name, _model)                                           \
        NET_REGISTER_DEV(QBJT_EB, _name)                                        \
        NETDEV_PARAMI(_name,  model,   _model)


// ----------------------------------------------------------------------------------------
// nld_Q - Base classes
// ----------------------------------------------------------------------------------------

// Have a common start for transistors

class NETLIB_NAME(Q) : public netlist_device_t
{
public:
    enum q_type {
        BJT_NPN,
        BJT_PNP
    };

    ATTR_COLD NETLIB_NAME(Q)(const family_t afamily)
    : netlist_device_t(afamily)
    , m_qtype(BJT_NPN) { }

    inline q_type qtype() const { return m_qtype; }
    inline bool is_qtype(q_type atype) const { return m_qtype == atype; }
    inline void set_qtype(q_type atype) { m_qtype = atype; }
protected:
    ATTR_COLD virtual void start();
    ATTR_COLD virtual void reset();
    ATTR_HOT ATTR_ALIGN void update();

    netlist_param_model_t m_model;
private:
    q_type m_qtype;
};

class NETLIB_NAME(QBJT) : public NETLIB_NAME(Q)
{
public:

    ATTR_COLD NETLIB_NAME(QBJT)(const family_t afamily)
    : NETLIB_NAME(Q)(afamily) { }

protected:

private:
};




// ----------------------------------------------------------------------------------------
// nld_QBJT_switch
// ----------------------------------------------------------------------------------------


/*
 *         + -              C
 *   B ----VVV----+         |
 *                |         |
 *                Rb        Rc
 *                Rb        Rc
 *                Rb        Rc
 *                |         |
 *                +----+----+
 *                     |
 *                     E
 */

class NETLIB_NAME(QBJT_switch) : public NETLIB_NAME(QBJT)
{
public:
    ATTR_COLD NETLIB_NAME(QBJT_switch)()
    : NETLIB_NAME(QBJT)(BJT_SWITCH), m_gB(NETLIST_GMIN), m_gC(NETLIST_GMIN), m_V(0.0), m_state_on(0) { }

    NETLIB_UPDATEI()
    {
        double vE = INPANALOG(m_EV);
        double vB = INPANALOG(m_BV);
        double m = (is_qtype( BJT_NPN) ? 1 : -1);

        int new_state = ((vB - vE) * m > m_V ) ? 1 : 0;
        if (m_state_on ^ new_state)
        {
            double gb = m_gB;
            double gc = m_gC;
            double v  = m_V * m;
            if (!new_state )
            {
                // not conducting
                gb = NETLIST_GMIN;
                v = 0;
                gc = NETLIST_GMIN;
            }
            m_RB.set(gb, v,   0.0);
            m_RC.set(gc, 0.0, 0.0);
            m_state_on = new_state;
            m_RB.update_dev();
            m_RC.update_dev();
        }

    }

    NETLIB_NAME(R) m_RB;
    NETLIB_NAME(R) m_RC;

    netlist_analog_input_t m_BV;
    netlist_analog_input_t m_EV;

protected:

    ATTR_COLD virtual void start();
    ATTR_HOT void update_param();

    double m_gB; // base conductance / switch on
    double m_gC; // collector conductance / switch on
    double m_V; // internal voltage source
    UINT8 m_state_on;

private:
};

// ----------------------------------------------------------------------------------------
// nld_QBJT_EB
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(QBJT_EB) : public NETLIB_NAME(QBJT)
{
public:
    ATTR_COLD NETLIB_NAME(QBJT_EB)()
    : NETLIB_NAME(QBJT)(BJT_EB),
      m_D_BC(netlist_object_t::ANALOG),
      m_D_BE(netlist_object_t::ANALOG),
      m_I_BC(netlist_object_t::ANALOG),
      m_I_BE(netlist_object_t::ANALOG)
      { }

    NETLIB_UPDATE_TERMINALS()
    {
        m_gD_BE.update_diode(m_D_BE.deltaV());
        m_gD_BC.update_diode(m_D_BC.deltaV());

        m_D_BE.set(m_gD_BE.G(), 0.0, m_gD_BE.Ieq());
        m_D_BC.set(m_gD_BC.G(), 0.0, m_gD_BC.Ieq());

        m_I_BE.set(0.0, 0.0, - m_alpha_r * m_gD_BC.I());
        m_I_BC.set(0.0, 0.0, - m_alpha_f * m_gD_BE.I());
    }

protected:

    ATTR_COLD virtual void start();
    ATTR_COLD virtual void reset();
    ATTR_HOT void update_param();
    ATTR_HOT ATTR_ALIGN void virtual update();

    netlist_generic_diode m_gD_BC;
    netlist_generic_diode m_gD_BE;

    nld_twoterm m_D_BC;
    nld_twoterm m_D_BE;

    nld_twoterm m_I_BC;
    nld_twoterm m_I_BE;

    double m_alpha_f;
    double m_alpha_r;

private:
};

#endif /* NLD_BJT_H_ */
