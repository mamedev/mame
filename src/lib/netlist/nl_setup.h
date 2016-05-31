// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.h
 *
 */

#ifndef NLSETUP_H_
#define NLSETUP_H_

#include <memory>
#include <stack>
#include "nl_base.h"
#include "nl_factory.h"

//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_STR(x) # x

#define NET_MODEL(model)                                                           \
	setup.register_model(model);

#define ALIAS(alias, name)                                                        \
	setup.register_alias(# alias, # name);

#define DIPPINS(pin1, ...)                                                          \
		setup.register_dippins_arr( # pin1 ", " # __VA_ARGS__);

/* to be used to reference new library truthtable devices */
#define NET_REGISTER_DEV(type, name)                                            \
		setup.register_dev(# type, # name);

#define NET_CONNECT(name, input, output)                                         \
		setup.register_link(# name "." # input, # output);

#define NET_C(term1, ...)                                                          \
		setup.register_link_arr( # term1 ", " # __VA_ARGS__);

#define PARAM(name, val)                                                          \
		setup.register_param(# name, val);

#define NETDEV_PARAMI(name, param, val)                                          \
		setup.register_param(# name "." # param, val);

#define NETLIST_NAME(name) netlist ## _ ## name

#define NETLIST_EXTERNAL(name)                                                \
		void NETLIST_NAME(name)(netlist::setup_t &setup);

#define NETLIST_START(name)                                                   \
void NETLIST_NAME(name)(netlist::setup_t &setup)                    \
{
#define NETLIST_END()  }

#define LOCAL_SOURCE(name)                                                    \
		setup.register_source(std::make_shared<netlist::source_proc_t>(# name, &NETLIST_NAME(name)));

#define LOCAL_LIB_ENTRY(name)                                                 \
		LOCAL_SOURCE(name)                                                    \
		setup.register_lib_entry(# name);

#define INCLUDE(name)                                                         \
		setup.include(# name);

#define SUBMODEL(model, name)                                                \
		setup.namespace_push(# name);                                         \
		NETLIST_NAME(model)(setup);                                           \
		setup.namespace_pop();

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

namespace netlist
{
	// Forward definition so we keep nl_factory.h out of the public
	class factory_list_t;


	class setup_t
	{
		P_PREVENT_COPYING(setup_t)
	public:

		// ----------------------------------------------------------------------------------------
		// A Generic netlist sources implementation
		// ----------------------------------------------------------------------------------------

		class source_t
		{
		public:
			using list_t = plib::pvector_t<std::shared_ptr<source_t>>;

			source_t()
			{}

			virtual ~source_t() { }

			virtual bool parse(setup_t &setup, const pstring &name) = 0;
		private:
		};

		using link_t = std::pair<pstring, pstring>;

		setup_t(netlist_t &netlist);
		~setup_t();

		netlist_t &netlist() { return m_netlist; }
		const netlist_t &netlist() const { return m_netlist; }

		pstring build_fqn(const pstring &obj_name) const;

		void register_and_set_param(pstring name, param_t &param);

		void register_term(core_terminal_t &obj);

		void register_dev(plib::owned_ptr<device_t> dev);
		void register_dev(const pstring &classname, const pstring &name);

		void register_lib_entry(const pstring &name);

		void register_model(const pstring &model_in);
		void register_alias(const pstring &alias, const pstring &out);
		void register_dippins_arr(const pstring &terms);

		void register_alias_nofqn(const pstring &alias, const pstring &out);

		void register_link_arr(const pstring &terms);
		void register_link_fqn(const pstring &sin, const pstring &sout);
		void register_link(const pstring &sin, const pstring &sout);

		void register_param(const pstring &param, const pstring &value);
		void register_param(const pstring &param, const double value);

		void register_frontier(const pstring attach, const double r_IN, const double r_OUT);

		void remove_connections(const pstring attach);

		bool connect(core_terminal_t &t1, core_terminal_t &t2);

		bool device_exists(const pstring name) const;

		param_t *find_param(const pstring &param_in, bool required = true);

		void start_devices();
		void resolve_inputs();

		/* handle namespace */

		void namespace_push(const pstring &aname);
		void namespace_pop();

		/* parse a source */

		void include(const pstring &netlist_name);

		/* register a source */

		template <class C>
		void register_source(std::shared_ptr<C> src)
		{
			m_sources.push_back(std::static_pointer_cast<source_t>(src));
		}

		factory_list_t &factory() { return m_factory; }
		const factory_list_t &factory() const { return m_factory; }

		bool is_library_item(const pstring &name) const { return m_lib.contains(name); }

		/* model / family related */

		const logic_family_desc_t *family_from_model(const pstring &model);
		const pstring model_value_str(model_map_t &map, const pstring &entity);
		nl_double model_value(model_map_t &map, const pstring &entity);

		void model_parse(const pstring &model, model_map_t &map);

		plib::plog_base<NL_DEBUG> &log() { return netlist().log(); }
		const plib::plog_base<NL_DEBUG> &log() const { return netlist().log(); }

		plib::pvector_t<std::pair<pstring, base_factory_t *>> m_device_factory;

	protected:

	private:

		core_terminal_t *find_terminal(const pstring &outname_in, bool required = true);
		core_terminal_t *find_terminal(const pstring &outname_in, object_t::type_t atype, bool required = true);

		void connect_terminals(core_terminal_t &in, core_terminal_t &out);
		void connect_input_output(core_terminal_t &in, core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, core_terminal_t &inp);
		bool connect_input_input(core_terminal_t &t1, core_terminal_t &t2);

		// helpers
		pstring objtype_as_str(object_t &in) const;

		const pstring resolve_alias(const pstring &name) const;
		devices::nld_base_proxy *get_d_a_proxy(core_terminal_t &out);

		netlist_t &m_netlist;

		plib::hashmap_t<pstring, pstring> m_alias;
		plib::hashmap_t<pstring, param_ref_t>  m_params;
		plib::hashmap_t<pstring, pstring> m_param_values;
		plib::hashmap_t<pstring, core_terminal_t *> m_terminals;

		plib::pvector_t<link_t> m_links;

		factory_list_t m_factory;

		plib::hashmap_t<pstring, pstring> m_models;

		int m_proxy_cnt;
		int m_frontier_cnt;

		std::stack<pstring> m_namespace_stack;
		source_t::list_t m_sources;
		plib::pvector_t<pstring> m_lib;

	};

	// ----------------------------------------------------------------------------------------
	// base sources
	// ----------------------------------------------------------------------------------------


	class source_string_t : public setup_t::source_t
	{
	public:

		source_string_t(const pstring &source)
		: setup_t::source_t(), m_str(source)
		{
		}

		bool parse(setup_t &setup, const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_file_t : public setup_t::source_t
	{
	public:

		source_file_t(const pstring &filename)
		: setup_t::source_t(), m_filename(filename)
		{
		}

		bool parse(setup_t &setup, const pstring &name) override;

	private:
		pstring m_filename;
	};

	class source_mem_t : public setup_t::source_t
	{
	public:
		source_mem_t(const char *mem)
		: setup_t::source_t(), m_str(mem)
		{
		}

		bool parse(setup_t &setup, const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_proc_t : public setup_t::source_t
	{
	public:
		source_proc_t(pstring name, void (*setup_func)(setup_t &))
		: setup_t::source_t(),
			m_setup_func(setup_func),
			m_setup_func_name(name)
		{
		}

		bool parse(setup_t &setup, const pstring &name) override
		{
			if (name == m_setup_func_name)
			{
				m_setup_func(setup);
				return true;
			}
			else
				return false;
		}
	private:
		void (*m_setup_func)(setup_t &);
		pstring m_setup_func_name;
	};

}


#endif /* NLSETUP_H_ */
