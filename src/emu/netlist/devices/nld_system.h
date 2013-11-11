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
	netlist_ttl_output_t m_Q;

	netlist_param_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(netlist_net_t &net, const netlist_time curtime);
);

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
