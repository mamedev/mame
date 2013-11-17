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
// netdev_*_const
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(ttl_const,
	netlist_ttl_output_t m_Q;
	netlist_param_t m_const;
);

NETLIB_DEVICE_WITH_PARAMS(analog_const,
	netlist_analog_output_t m_Q;
	netlist_param_t m_const;
);

// ----------------------------------------------------------------------------------------
// mainclock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(mainclock,
public:
    netlist_ttl_output_t m_Q;

	netlist_param_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(netlist_net_t &net, const netlist_time curtime);
);

// ----------------------------------------------------------------------------------------
// clock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(clock,
    netlist_ttl_input_t m_feedback;
    netlist_ttl_output_t m_Q;

    netlist_param_t m_freq;
    netlist_time m_inc;
);

// ----------------------------------------------------------------------------------------
// solver
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(solver,
        netlist_ttl_input_t m_feedback;
        netlist_ttl_output_t m_Q;

        netlist_time m_inc;
        netlist_time m_last_step;
        netlist_list_t<netlist_terminal_t *> m_terms;
        netlist_list_t<netlist_terminal_t *> m_inps;

public:
        netlist_list_t<netlist_net_t *> m_nets;

        ATTR_HOT inline void schedule();

        ATTR_COLD void post_start();
);

inline void NETLIB_NAME(solver)::schedule()
{
    // FIXME: time should be parameter;
    if (!m_Q.net().is_queued()) {
        m_Q.net().push_to_queue(NLTIME_FROM_NS(10));
    }
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

    netlist_param_t m_R;

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
        m_P.m_g = m_g;
        m_N.m_g = m_g;
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

    netlist_param_t m_C;

protected:
    void start()
    {
        register_terminal("1", m_P);
        register_terminal("2", m_N);

        register_param("C", m_C, NETLIST_GMIN);
    }

    virtual void update_param()
    {
        // set to some very big step time for now
        // ==> large resistance
        step_time(1e-9);
    }

    ATTR_HOT virtual void step_time(const double st)
    {
        m_g = m_P.m_g = m_N.m_g = m_C.Value() / st;
        m_I = -m_g * (m_P.net().Q_Analog()- m_N.net().Q_Analog());
    }
private:
};

#endif /* NLD_SYSTEM_H_ */
