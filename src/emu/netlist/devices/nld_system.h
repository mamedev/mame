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

#define NETDEV_LOGIC_INPUT(_name)                                                   \
		NET_REGISTER_DEV(logic_input, _name)

#define NETDEV_ANALOG_INPUT(_name)                                                  \
		NET_REGISTER_DEV(analog_input, _name)

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
// Special support devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(logic_input,
	netlist_ttl_output_t m_Q;
);

NETLIB_DEVICE(analog_input,
	netlist_analog_output_t m_Q;
);



#endif /* NLD_SYSTEM_H_ */
