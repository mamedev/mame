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

#define NET_STR(_x) # _x

#define NET_ALIAS(_alias, _name)                                                    \
	netlist.register_alias(# _alias, # _name);
#define NET_NEW(_type)  net_create_device_by_classname(NETLIB_NAME_STR(_type), netlist)

#define NET_REGISTER_DEV(_type, _name)                                              \
		netlist.register_dev(NET_NEW(_type), # _name);
#define NET_REMOVE_DEV(_name)                                                       \
		netlist.remove_dev(# _name);
#define NET_REGISTER_SIGNAL(_type, _name)                                           \
		NET_REGISTER_DEV(_type ## _ ## sig, _name)
#define NET_CONNECT(_name, _input, _output)                                         \
		netlist.register_link(# _name "." # _input, # _output);
#define NET_C(_input, _output)                                                      \
        netlist.register_link(NET_STR(_input) , NET_STR(_output));

#define NETDEV_PARAM(_name, _val)                                                   \
		netlist.register_param(# _name, _val);

#define NETDEV_PARAMI(_name, _param, _val)                                           \
        netlist.register_param(# _name "." # _param, _val);

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

//class NETLIB_NAME(analog_callback);

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

class netlist_setup_t
{
public:

    struct link_t
    {
        link_t(const astring &ae1, const astring &ae2)
        {
            e1 = ae1;
            e2 = ae2;
        }
        astring e1;
        astring e2;
    };

	typedef tagmap_t<netlist_device_t *, 393> tagmap_devices_t;
    typedef tagmap_t<link_t *, 393> tagmap_link_t;
	typedef tagmap_t<const astring *, 393> tagmap_astring_t;
	typedef tagmap_t<netlist_param_t *, 393> tagmap_param_t;
	typedef tagmap_t<netlist_terminal_t *, 393> tagmap_terminal_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	netlist_base_t &netlist() { return m_netlist; }

	netlist_device_t *register_dev(netlist_device_t *dev, const astring &name);
	void remove_dev(const astring &name);

    void register_alias(const astring &alias, const astring &out);
    void register_link(const astring &sin, const astring &sout);
    void register_param(const astring &param, const astring &value);
    void register_param(const astring &param, const double value);

    void register_object(netlist_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, netlist_object_t &obj, netlist_input_t::state_e state);

    netlist_terminal_t &find_terminal(const astring &outname_in);
    netlist_terminal_t &find_terminal(const astring &outname_in, netlist_object_t::type_t atype);

    netlist_param_t &find_param(const astring &param_in);

	void register_callback(const astring &devname, netlist_output_delegate delegate);

	void parse(char *buf);

    void start_devices(void);
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
	tagmap_param_t  m_params;
	tagmap_link_t  m_links;
    tagmap_astring_t m_params_temp;

	int m_proxy_cnt;

	void connect_terminals(netlist_terminal_t &in, netlist_terminal_t &out);
	void connect_input_output(netlist_input_t &in, netlist_output_t &out);
    void connect_terminal_output(netlist_terminal_t &in, netlist_output_t &out);
    void connect_terminal_input(netlist_terminal_t &term, netlist_input_t &inp);

    // helpers
    astring objtype_as_astr(netlist_object_t &in);

	const astring resolve_alias(const astring &name) const;
};

#endif /* NLSETUP_H_ */
