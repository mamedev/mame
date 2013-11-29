// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_system.h
 *
 * netlist devices defined in the core
 */

#ifndef NLD_SYSTEM_H_
#define NLD_SYSTEM_H_

#include "../nl_setup.h"
#include "../nl_base.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#define NETDEV_TTL_CONST(_name, _v)                                                 \
		NET_REGISTER_DEV(ttl_const, _name)                                          \
		NETDEV_PARAM(_name.CONST, _v)

#define NETDEV_ANALOG_CONST(_name, _v)                                              \
		NET_REGISTER_DEV(analog_const, _name)                                       \
		NETDEV_PARAM(_name.CONST, _v)

#define NETDEV_MAINCLOCK(_name)                                                     \
        NET_REGISTER_DEV(mainclock, _name)

#define NETDEV_CLOCK(_name)                                                         \
        NET_REGISTER_DEV(clock, _name)

#define NETDEV_SOLVER(_name)                                                        \
        NET_REGISTER_DEV(solver, _name)

// ----------------------------------------------------------------------------------------
// 2 terminal devices
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

// .model 1N914 D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)

// ----------------------------------------------------------------------------------------
// netdev_*_const
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(ttl_const,
	netlist_ttl_output_t m_Q;
	netlist_param_logic_t m_const;
);

NETLIB_DEVICE_WITH_PARAMS(analog_const,
	netlist_analog_output_t m_Q;
	netlist_param_double_t m_const;
);

// ----------------------------------------------------------------------------------------
// mainclock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(mainclock,
public:
    netlist_ttl_output_t m_Q;

	netlist_param_double_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(netlist_net_t &net, const netlist_time curtime);
);

// ----------------------------------------------------------------------------------------
// clock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(clock,
    netlist_ttl_input_t m_feedback;
    netlist_ttl_output_t m_Q;

    netlist_param_double_t m_freq;
    netlist_time m_inc;
);

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(solver,
        typedef netlist_list_t<netlist_terminal_t *> terminal_list_t;
        typedef netlist_list_t<netlist_net_t *>      net_list_t;

        netlist_ttl_input_t m_fb_sync;
        netlist_ttl_output_t m_Q_sync;

        netlist_ttl_input_t m_fb_step;
        netlist_ttl_output_t m_Q_step;

        netlist_param_double_t m_freq;
        netlist_param_double_t m_sync_delay;

        netlist_time m_inc;
        netlist_time m_last_step;
        netlist_time m_nt_sync_delay;

        terminal_list_t m_terms;
        terminal_list_t m_inps;

public:

        ~NETLIB_NAME(solver)();

        net_list_t m_nets;

        ATTR_HOT inline void schedule();

        ATTR_COLD void post_start();
);

inline void NETLIB_NAME(solver)::schedule()
{
    // FIXME: time should be parameter;
    if (!m_Q_sync.net().is_queued())
        m_Q_sync.net().push_to_queue(m_nt_sync_delay);
}

// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(analog_callback) : public netlist_device_t
{
public:
	NETLIB_NAME(analog_callback)()
		: netlist_device_t() { }

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
	}

	ATTR_COLD void register_callback(netlist_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update();


private:
	netlist_analog_input_t m_in;
	netlist_output_delegate m_callback;
};


// ----------------------------------------------------------------------------------------
// nld_twoterm
// ----------------------------------------------------------------------------------------

class nld_twoterm : public netlist_device_t
{
public:
    nld_twoterm()
    : netlist_device_t(), m_g(0.0), m_V(0.0), m_I(0.0)
    {
    }

    netlist_terminal_t m_P;
    netlist_terminal_t m_N;

protected:
    virtual void start()
    {
    }

    ATTR_HOT ATTR_ALIGN virtual void update_terminals()
    {
        m_P.m_g = m_N.m_g = m_g;
        m_N.m_Idr = (m_P.net().Q_Analog() - m_V) * m_g + m_I;
        m_P.m_Idr = (m_N.net().Q_Analog() + m_V) * m_g - m_I;
        //printf("%f %f %f %f\n", m_N.m_Idr, m_P.m_Idr, m_N.net().Q_Analog(), m_P.net().Q_Analog());
    }

    ATTR_HOT ATTR_ALIGN virtual void update()
    {
        /* only called if connected to a rail net ==> notify the solver to recalculate */
        netlist().solver()->schedule();
    }

    double m_g; // conductance
    double m_V; // internal voltage source
    double m_I; // internal current source
private:
};

class nld_R : public nld_twoterm
{
public:
    nld_R()
    : nld_twoterm()
    {
    }

    netlist_param_double_t m_R;

protected:
    void start()
    {
        register_terminal("1", m_P);
        register_terminal("2", m_N);

        register_param("R", m_R, NETLIST_GMIN);
    }

    virtual void update_param()
    {
        m_g = 1.0 / m_R.Value();
    }

private:
};

class nld_C : public nld_twoterm
{
public:
    nld_C()
    : nld_twoterm()
    {
    }

    netlist_param_double_t m_C;

protected:
    void start()
    {
        register_terminal("1", m_P);
        register_terminal("2", m_N);

        register_param("C", m_C, NETLIST_GMIN);
    }

    virtual void update_param()
    {
        // set to some very small step time for now
        step_time(1e-9);
    }

    ATTR_HOT virtual void step_time(const double st)
    {
        m_g = m_P.m_g = m_N.m_g = m_C.Value() / st;
        m_I = -m_g * (m_P.net().Q_Analog()- m_N.net().Q_Analog());
    }
private:
};

class nld_D : public nld_twoterm
{
public:
    nld_D()
    : nld_twoterm()
    {
    }

    netlist_param_multi_t m_model;

    double m_Vt;
    double m_Is;
    double m_n;

    double m_VtInv;
    double m_Vcrit;
    double m_Vd;

protected:
    void start()
    {
        register_terminal("A", m_P);
        register_terminal("K", m_N);
        register_param("model", m_model, "");

        m_Vd = 0.7;
    }

    virtual void update_param()
    {
        m_Is = m_model.dValue("Is", 1e-15);
        m_n = m_model.dValue("N", 1);

        m_Vt = 0.0258 * m_n;

        m_Vcrit = m_Vt * log(m_Vt / m_Is / sqrt(2.0));
        m_VtInv = 1.0 / m_Vt;
        NL_VERBOSE_OUT(("VCutoff: %f\n", m_Vcrit));
    }

    ATTR_HOT ATTR_ALIGN virtual void update_terminals()
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
};

#endif /* NLD_SYSTEM_H_ */
