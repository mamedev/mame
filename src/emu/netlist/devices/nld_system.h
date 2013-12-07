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

NETLIB_DEVICE_WITH_PARAMS(solver,
        typedef netlist_list_t<netlist_core_terminal_t *> terminal_list_t;
        typedef netlist_list_t<netlist_net_t *>      net_list_t;
        typedef netlist_list_t<netlist_core_device_t *>      dev_list_t;

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

        terminal_list_t m_terms;
        terminal_list_t m_inps;
        dev_list_t m_steps;

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



#endif /* NLD_SYSTEM_H_ */
