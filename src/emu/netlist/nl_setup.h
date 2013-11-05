// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.h
 *
 *  Created on: 3 Nov 2013
 *      Author: andre
 */

#ifndef NLSETUP_H_
#define NLSETUP_H_

#include "nl_base.h"

//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_ALIAS(_alias, _name)                                                    \
	netlist.register_alias(# _alias, # _name);
#define NET_NEW(_type , _name)  net_create_device_by_classname(NETLIB_NAME_STR(_type), netlist, # _name)

#define NET_REGISTER_DEV(_type, _name)                                              \
		netlist.register_dev(NET_NEW(_type, _name));
#define NET_REMOVE_DEV(_name)                                                       \
		netlist.remove_dev(# _name);
#define NET_REGISTER_SIGNAL(_type, _name)                                           \
		NET_REGISTER_DEV(_type ## _ ## sig, _name)
#define NET_CONNECT(_name, _input, _output)                                         \
		netlist.register_link(# _name "." # _input, # _output);
#define NETDEV_PARAM(_name, _val)                                                   \
		netlist.find_param(# _name).initial(_val);

#define NETLIST_NAME(_name) netlist ## _ ## _name

#define NETLIST_START(_name) \
ATTR_COLD void NETLIST_NAME(_name)(netlist_setup_t &netlist) \
{
#define NETLIST_END  }

#define NETLIST_INCLUDE(_name)                                                      \
		NETLIST_NAME(_name)(netlist);

#define NETLIST_MEMREGION(_name)                                                    \
		netlist.parse((char *)downcast<netlist_t &>(netlist.netlist()).machine().root_device().memregion(_name)->base());

// ----------------------------------------------------------------------------------------
// FIXME: Clean this up
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(netdev_analog_callback);

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

class netlist_setup_t
{
public:

	typedef tagmap_t<net_device_t *, 393> tagmap_devices_t;
	typedef tagmap_t<const astring *, 393> tagmap_astring_t;
	typedef tagmap_t<net_param_t *, 393> tagmap_param_t;
	typedef tagmap_t<net_terminal_t *, 393> tagmap_terminal_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	netlist_base_t &netlist() { return m_netlist; }

	net_device_t *register_dev(net_device_t *dev);
	void remove_dev(const astring &name);

	void register_output(netlist_core_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, net_output_t &out);
	void register_input(net_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, net_input_t &inp, net_input_t::net_input_state type);
	void register_alias(const astring &alias, const astring &out);
	void register_param(const astring &sname, net_param_t *param);

	void register_link(const astring &sin, const astring &sout);

	net_output_t &find_output(const astring &outname_in);
	net_param_t &find_param(const astring &param_in);

	void register_callback(const astring &devname, netlist_output_delegate delegate);

	void parse(char *buf);

	void resolve_inputs(void);
	void step_devices_once(void);

	/* not ideal, but needed for save_state */
	tagmap_terminal_t  m_terminals;

	void print_stats();

protected:

private:

	netlist_base_t &m_netlist;

	tagmap_devices_t m_devices;
	tagmap_astring_t m_alias;
	//tagmap_input_t  m_inputs;
	tagmap_param_t  m_params;
	tagmap_astring_t  m_links;

	net_output_t *find_output_exact(const astring &outname_in);
	const astring &resolve_alias(const astring &name) const;
};

#endif /* NLSETUP_H_ */
