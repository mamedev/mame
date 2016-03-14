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

#define DIPPINS(_pin1, ...)                                                          \
		setup.register_dippins_arr( #_pin1 ", " # __VA_ARGS__);

/* to be used to reference new library truthtable devices */
#define NET_REGISTER_DEV(_type, _name)                                            \
		setup.register_dev(# _type, # _name);

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

#define NETLIST_EXTERNAL(_name)                                                \
		ATTR_COLD void NETLIST_NAME(_name)(netlist::setup_t &setup);

#define NETLIST_START(_name)                                                   \
ATTR_COLD void NETLIST_NAME(_name)(netlist::setup_t &setup)                    \
{
#define NETLIST_END()  }

#define LOCAL_SOURCE(_name)                                                    \
		setup.register_source(palloc(netlist::source_proc_t(# _name, &NETLIST_NAME(_name))));

#define LOCAL_LIB_ENTRY(_name)                                                 \
		LOCAL_SOURCE(_name)                                                    \
		setup.register_lib_entry(# _name);

#define INCLUDE(_name)                                                         \
		setup.include(# _name);

#define SUBMODEL(_model, _name)                                                \
		setup.namespace_push(# _name);                                         \
		NETLIST_NAME(_model)(setup);                                           \
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
			typedef pvector_t<source_t *> list_t;

			source_t()
			{}

			virtual ~source_t() { }

			virtual bool parse(setup_t &setup, const pstring &name) = 0;
		private:
		};

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

		setup_t(netlist_t *netlist);
		~setup_t();

		void init();

		netlist_t &netlist() { return *m_netlist; }
		const netlist_t &netlist() const { return *m_netlist; }

		pstring build_fqn(const pstring &obj_name) const;

		device_t *register_dev(device_t *dev, const pstring &name);
		device_t *register_dev(const pstring &classname, const pstring &name);
		void remove_dev(const pstring &name);

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

		void register_object(device_t &dev, const pstring &name, object_t &obj);
		bool connect(core_terminal_t &t1, core_terminal_t &t2);

		core_terminal_t *find_terminal(const pstring &outname_in, bool required = true);
		core_terminal_t *find_terminal(const pstring &outname_in, object_t::type_t atype, bool required = true);

		param_t *find_param(const pstring &param_in, bool required = true);

		void start_devices();
		void resolve_inputs();

		/* handle namespace */

		void namespace_push(const pstring &aname);
		void namespace_pop();

		/* parse a source */

		void include(const pstring &netlist_name);

		/* register a source */

		void register_source(source_t *src) { m_sources.push_back(src); }

		factory_list_t &factory() { return *m_factory; }
		const factory_list_t &factory() const { return *m_factory; }

		bool is_library_item(const pstring &name) const { return m_lib.contains(name); }

		void print_stats() const;

		/* model / family related */

		logic_family_desc_t *family_from_model(const pstring &model);
		const pstring model_value_str(model_map_t &map, const pstring &entity);
		nl_double model_value(model_map_t &map, const pstring &entity);

		void model_parse(const pstring &model, model_map_t &map);

		plog_base<NL_DEBUG> &log() { return netlist().log(); }
		const plog_base<NL_DEBUG> &log() const { return netlist().log(); }

	protected:

	private:

		void connect_terminals(core_terminal_t &in, core_terminal_t &out);
		void connect_input_output(core_terminal_t &in, core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, core_terminal_t &inp);
		bool connect_input_input(core_terminal_t &t1, core_terminal_t &t2);

		// helpers
		pstring objtype_as_astr(object_t &in) const;

		const pstring resolve_alias(const pstring &name) const;
		devices::nld_base_proxy *get_d_a_proxy(core_terminal_t &out);

		netlist_t *m_netlist;

		phashmap_t<pstring, pstring> m_alias;
		phashmap_t<pstring, param_t *>  m_params;
		phashmap_t<pstring, pstring> m_params_temp;
		phashmap_t<pstring, core_terminal_t *> m_terminals;

		pvector_t<link_t> m_links;

		factory_list_t *m_factory;

		phashmap_t<pstring, pstring> m_models;

		int m_proxy_cnt;
		int m_frontier_cnt;

		std::stack<pstring> m_stack;
		source_t::list_t m_sources;
		pvector_t<pstring> m_lib;

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
