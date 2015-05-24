// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.h
 *
 */

#ifndef NLSETUP_H_
#define NLSETUP_H_

#include "nl_base.h"

//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_STR(_x) # _x

#define NET_MODEL(_model)                                                           \
	setup.register_model(_model);

#define ALIAS(_alias, _name)                                                        \
	setup.register_alias(# _alias, # _name);

#define NET_REGISTER_DEV(_type, _name)                                              \
		setup.register_dev(NETLIB_NAME_STR(_type), # _name);

#define NET_REMOVE_DEV(_name)                                                       \
		setup.remove_dev(# _name);

#define NET_REGISTER_SIGNAL(_type, _name)                                           \
		NET_REGISTER_DEV(_type ## _ ## sig, _name)

#define NET_CONNECT(_name, _input, _output)                                         \
		setup.register_link(# _name "." # _input, # _output);

#define NET_C(_term1, ...)                                                          \
		setup.register_link_arr( #_term1 ", " # __VA_ARGS__);

#define PARAM(_name, _val)                                                          \
		setup.register_param(# _name, _val);

#define NETDEV_PARAMI(_name, _param, _val)                                          \
		setup.register_param(# _name "." # _param, _val);

#define NETLIST_NAME(_name) netlist ## _ ## _name

#define NETLIST_EXTERNAL(_name)                                                     \
ATTR_COLD void NETLIST_NAME(_name)(netlist_setup_t &setup)

#define NETLIST_START(_name)                                                        \
ATTR_COLD void NETLIST_NAME(_name)(netlist_setup_t &setup)                          \
{
#define NETLIST_END()  }

#define INCLUDE(_name)                                                              \
		NETLIST_NAME(_name)(setup);

#define SUBMODEL(_name, _model)                                                     \
		setup.namespace_push(# _name);                                              \
		NETLIST_NAME(_model)(setup);                                                \
		setup.namespace_pop();

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

// Forward definition so we keep nl_factory.h out of the public
class netlist_factory_t;

class netlist_setup_t
{
	NETLIST_PREVENT_COPYING(netlist_setup_t)
public:

	struct link_t
	{
		link_t() { }
		link_t(void *) { }
		// Copy constructor
		link_t(const link_t &from)
		{
			e1 = from.e1;
			e2 = from.e2;
		}

		link_t(const pstring &ae1, const pstring &ae2)
		{
			e1 = ae1;
			e2 = ae2;
		}
		pstring e1;
		pstring e2;

		bool operator==(const link_t &rhs) const { return (e1 == rhs.e1) && (e2 == rhs.e2); }
		link_t &operator=(const link_t &rhs) { e1 = rhs.e1; e2 = rhs.e2; return *this; }

		const pstring &name() const { return e1; }
	};

	typedef pnamedlist_t<link_t> tagmap_nstring_t;
	typedef pnamedlist_t<netlist_param_t *> tagmap_param_t;
	typedef pnamedlist_t<netlist_core_terminal_t *> tagmap_terminal_t;
	typedef plist_t<link_t> tagmap_link_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	void init();

	netlist_base_t &netlist() { return m_netlist; }
	const netlist_base_t &netlist() const { return m_netlist; }

	pstring build_fqn(const pstring &obj_name) const;

	netlist_device_t *register_dev(netlist_device_t *dev, const pstring &name);
	netlist_device_t *register_dev(const pstring &classname, const pstring &name);
	void remove_dev(const pstring &name);

	void register_model(const pstring &model);
	void register_alias(const pstring &alias, const pstring &out);
	void register_alias_nofqn(const pstring &alias, const pstring &out);
	void register_link_arr(const pstring &terms);
	void register_link(const pstring &sin, const pstring &sout);
	void register_param(const pstring &param, const pstring &value);
	void register_param(const pstring &param, const double value);

	void register_object(netlist_device_t &dev, const pstring &name, netlist_object_t &obj);
	bool connect(netlist_core_terminal_t &t1, netlist_core_terminal_t &t2);

	netlist_core_terminal_t *find_terminal(const pstring &outname_in, bool required = true);
	netlist_core_terminal_t *find_terminal(const pstring &outname_in, netlist_object_t::type_t atype, bool required = true);

	netlist_param_t *find_param(const pstring &param_in, bool required = true);

	void parse(const char *buf);

	void start_devices();
	void resolve_inputs();

	/* handle namespace */

	void namespace_push(const pstring &aname);
	void namespace_pop();

	netlist_factory_t &factory() { return *m_factory; }
	const netlist_factory_t &factory() const { return *m_factory; }

	/* not ideal, but needed for save_state */
	tagmap_terminal_t  m_terminals;

	void print_stats() const;

protected:

private:

	netlist_base_t &m_netlist;

	tagmap_nstring_t m_alias;
	tagmap_param_t  m_params;
	tagmap_link_t   m_links;
	tagmap_nstring_t m_params_temp;

	netlist_factory_t *m_factory;

	plist_t<pstring> m_models;

	int m_proxy_cnt;

	pstack_t<pstring> m_stack;


	void connect_terminals(netlist_core_terminal_t &in, netlist_core_terminal_t &out);
	void connect_input_output(netlist_core_terminal_t &in, netlist_core_terminal_t &out);
	void connect_terminal_output(netlist_terminal_t &in, netlist_core_terminal_t &out);
	void connect_terminal_input(netlist_terminal_t &term, netlist_core_terminal_t &inp);
	bool connect_input_input(netlist_core_terminal_t &t1, netlist_core_terminal_t &t2);

	// helpers
	pstring objtype_as_astr(netlist_object_t &in) const;

	const pstring resolve_alias(const pstring &name) const;
	nld_base_proxy *get_d_a_proxy(netlist_core_terminal_t &out);
};

#endif /* NLSETUP_H_ */
