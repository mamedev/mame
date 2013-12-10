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

class netlist_matrix_solver_t
{
public:
    typedef netlist_list_t<netlist_matrix_solver_t *> list_t;
    typedef netlist_core_device_t::list_t dev_list_t;
    ATTR_COLD void setup(netlist_net_t::list_t &nets);

    // return true if a reschedule is needed ...
    ATTR_HOT bool solve();
    ATTR_HOT void step(const netlist_time delta);
    ATTR_HOT void update_inputs();

    ATTR_HOT inline bool is_dynamic() { return m_dynamic.count() > 0; }

    double m_accuracy;

private:
    netlist_net_t::list_t m_nets;
    dev_list_t m_dynamic;
    dev_list_t m_inps;
    dev_list_t m_steps;
};

NETLIB_DEVICE_WITH_PARAMS(solver,
        typedef netlist_core_device_t::list_t dev_list_t;

        netlist_ttl_input_t m_fb_sync;
        netlist_ttl_output_t m_Q_sync;

        netlist_ttl_input_t m_fb_step;
        netlist_ttl_output_t m_Q_step;

        netlist_param_double_t m_freq;
        netlist_param_double_t m_sync_delay;
        netlist_param_double_t m_accuracy;

        netlist_time m_inc;
        netlist_time m_last_step;
        netlist_time m_nt_sync_delay;

        netlist_matrix_solver_t::list_t m_mat_solvers;
public:

        ~NETLIB_NAME(solver)();

        netlist_net_t::list_t m_nets;

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



#endif /* NLD_SYSTEM_H_ */
