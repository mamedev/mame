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
		NET_REGISTER_DEV(netdev_ttl_const, _name)                                   \
		NETDEV_PARAM(_name.CONST, _v)

#define NETDEV_ANALOG_CONST(_name, _v)                                              \
		NET_REGISTER_DEV(netdev_analog_const, _name)                                \
		NETDEV_PARAM(_name.CONST, _v)

// ----------------------------------------------------------------------------------------
// netdev_*_const
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(netdev_ttl_const,
	ttl_output_t m_Q;
	net_param_t m_const;
);

NETLIB_DEVICE_WITH_PARAMS(netdev_analog_const,
	analog_output_t m_Q;
	net_param_t m_const;
);

// ----------------------------------------------------------------------------------------
// netdev_mainclock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(netdev_mainclock,
	ttl_output_t m_Q;

	net_param_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(net_net_t &net, const netlist_time curtime);
);

// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(netdev_analog_callback) : public net_device_t
{
public:
	NETLIB_NAME(netdev_analog_callback)()
		: net_device_t() { }

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
	analog_input_t m_in;
	netlist_output_delegate m_callback;
};

#endif /* NLD_SYSTEM_H_ */
