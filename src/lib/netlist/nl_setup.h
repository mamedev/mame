// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.h
 *
 */

#ifndef NLSETUP_H_
#define NLSETUP_H_

#include "plib/pstring.h"
#include "plib/putil.h"
#include "plib/pstream.h"
#include "plib/pparser.h"

#include "nl_factory.h"
#include "nl_config.h"
#include "netlist_types.h"

#include <stack>
#include <vector>
#include <memory>

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

#define NET_CONNECT(name, input, output)                                        \
		setup.register_link(# name "." # input, # output);

#define NET_C(term1, ...)                                                       \
		setup.register_link_arr( # term1 ", " # __VA_ARGS__);

#define PARAM(name, val)                                                        \
		setup.register_param(# name, val);

#define HINT(name, val)                                                        \
		setup.register_param(# name ".HINT_" # val, 1);

#define NETDEV_PARAMI(name, param, val)                                         \
		setup.register_param(# name "." # param, val);

#define NETLIST_NAME(name) netlist ## _ ## name

#define NETLIST_EXTERNAL(name)                                                 \
		void NETLIST_NAME(name)(netlist::setup_t &setup);

#define NETLIST_START(name)                                                    \
void NETLIST_NAME(name)(netlist::setup_t &setup)                               \
{
#define NETLIST_END()  }

#define LOCAL_SOURCE(name)                                                     \
		setup.register_source(plib::make_unique_base<netlist::source_t, netlist::source_proc_t>(setup, # name, &NETLIST_NAME(name)));

#define LOCAL_LIB_ENTRY(name)                                                  \
		LOCAL_SOURCE(name)                                                     \
		setup.register_lib_entry(# name, __FILE__);

#define INCLUDE(name)                                                          \
		setup.include(# name);

#define SUBMODEL(model, name)                                                  \
		setup.namespace_push(# name);                                          \
		NETLIST_NAME(model)(setup);                                            \
		setup.namespace_pop();

#define OPTIMIZE_FRONTIER(attach, r_in, r_out)                                  \
		setup.register_frontier(# attach, r_in, r_out);

// -----------------------------------------------------------------------------
// truthtable defines
// -----------------------------------------------------------------------------

#define TRUTHTABLE_START(cname, in, out, def_params) \
	{ \
		netlist::tt_desc desc; \
		desc.name = #cname ; \
		desc.classname = #cname ; \
		desc.ni = in; \
		desc.no = out; \
		desc.def_param = def_params; \
		desc.family = "";

#define TT_HEAD(x) \
		desc.desc.push_back(x);

#define TT_LINE(x) \
		desc.desc.push_back(x);

#define TT_FAMILY(x) \
		desc.family = x;

#define TRUTHTABLE_END() \
		setup.tt_factory_create(desc, __FILE__);       \
	}


namespace netlist
{

	namespace detail {
		class core_terminal_t;
		class net_t;
	}

	namespace devices {
		class nld_base_proxy;
	}

	class core_device_t;
	class param_t;
	class setup_t;
	class netlist_t;
	class logic_family_desc_t;
	class terminal_t;

	// -----------------------------------------------------------------------------
	// truthtable desc
	// -----------------------------------------------------------------------------

	struct tt_desc
	{
		pstring name;
		pstring classname;
		unsigned long ni;
		unsigned long no;
		pstring def_param;
		std::vector<pstring> desc;
		pstring family;
	};

	// -----------------------------------------------------------------------------
	// param_ref_t
	// -----------------------------------------------------------------------------

	struct param_ref_t
	{
		param_ref_t(const pstring &name, core_device_t &device, param_t &param)
		: m_name(name)
		, m_device(device)
		, m_param(param)
		{ }
		pstring m_name;
		core_device_t &m_device;
		param_t &m_param;
	};

	// ----------------------------------------------------------------------------------------
	// A Generic netlist sources implementation
	// ----------------------------------------------------------------------------------------

	class source_t
	{
	public:
		enum type_t
		{
			SOURCE,
			DATA
		};

		using list_t = std::vector<std::unique_ptr<source_t>>;

		source_t(setup_t &setup, const type_t type = SOURCE)
		: m_setup(setup), m_type(type)
		{}

		virtual ~source_t() { }

		virtual bool parse(const pstring &name);
		virtual std::unique_ptr<plib::pistream> stream(const pstring &name) = 0;

		setup_t &setup() { return m_setup; }
		type_t type() const { return m_type; }
	private:
		setup_t &m_setup;
		const type_t m_type;
	};

	// ----------------------------------------------------------------------------------------
	// setup_t
	// ----------------------------------------------------------------------------------------


	class setup_t : plib::nocopyassignmove
	{
	public:

		using link_t = std::pair<pstring, pstring>;

		explicit setup_t(netlist_t &netlist);
		~setup_t();

		netlist_t &netlist() { return m_netlist; }
		const netlist_t &netlist() const { return m_netlist; }

		pstring build_fqn(const pstring &obj_name) const;

		void register_param(const pstring &name, param_t &param);
		pstring get_initial_param_val(const pstring &name, const pstring &def);
		double get_initial_param_val(const pstring &name, const double def);
		int get_initial_param_val(const pstring &name, const int def);

		void register_term(detail::core_terminal_t &obj);

		void register_dev(const pstring &classname, const pstring &name);

		void register_lib_entry(const pstring &name, const pstring &sourcefile);

		void register_model(const pstring &model_in);
		void register_alias(const pstring &alias, const pstring &out);
		void register_dippins_arr(const pstring &terms);

		void register_alias_nofqn(const pstring &alias, const pstring &out);

		void register_link_arr(const pstring &terms);
		void register_link_fqn(const pstring &sin, const pstring &sout);
		void register_link(const pstring &sin, const pstring &sout);

		void register_param(const pstring &param, const pstring &value);
		void register_param(const pstring &param, const double value);

		void register_frontier(const pstring &attach, const double r_IN, const double r_OUT);

		void remove_connections(const pstring &attach);

		bool connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		bool device_exists(const pstring &name) const;

		param_t *find_param(const pstring &param_in, bool required = true) const;

		void start_devices();
		void resolve_inputs();

		/* handle namespace */

		void namespace_push(const pstring &aname);
		void namespace_pop();

		/* parse a source */

		void include(const pstring &netlist_name);

		std::unique_ptr<plib::pistream> get_data_stream(const pstring &name);

		bool parse_stream(plib::putf8_reader &istrm, const pstring &name);

		/* register a source */

		void register_source(std::unique_ptr<source_t> &&src)
		{
			m_sources.push_back(std::move(src));
		}

		void register_define(pstring def, pstring val) { m_defines.push_back(plib::ppreprocessor::define_t(def, val)); }
		void register_define(pstring defstr);

		factory::list_t &factory() { return m_factory; }
		const factory::list_t &factory() const { return m_factory; }

		/* model / family related */

		const pstring model_value_str(detail::model_map_t &map, const pstring &entity);
		double model_value(detail::model_map_t &map, const pstring &entity);

		void model_parse(const pstring &model, detail::model_map_t &map);

		const logic_family_desc_t *family_from_model(const pstring &model);

		void tt_factory_create(tt_desc &desc, const pstring &sourcefile);

		/* helper - also used by nltool */
		const pstring resolve_alias(const pstring &name) const;

		plib::plog_base<netlist_t, NL_DEBUG> &log();
		const plib::plog_base<netlist_t, NL_DEBUG> &log() const;

		std::vector<std::pair<pstring, factory::element_t *>> m_device_factory;

		std::unordered_map<pstring, pstring> m_alias;
		std::unordered_map<pstring, pstring> m_param_values;
		std::unordered_map<pstring, detail::core_terminal_t *> m_terminals;

		/* needed by proxy */
		detail::core_terminal_t *find_terminal(const pstring &outname_in, const detail::terminal_type atype, bool required = true);

	private:

		detail::core_terminal_t *find_terminal(const pstring &outname_in, bool required = true);

		void merge_nets(detail::net_t &thisnet, detail::net_t &othernet);

		void connect_terminals(detail::core_terminal_t &in, detail::core_terminal_t &out);
		void connect_input_output(detail::core_terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, detail::core_terminal_t &inp);
		bool connect_input_input(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		// helpers
		pstring termtype_as_str(detail::core_terminal_t &in) const;

		devices::nld_base_proxy *get_d_a_proxy(detail::core_terminal_t &out);
		devices::nld_base_proxy *get_a_d_proxy(detail::core_terminal_t &inp);

		netlist_t                                   &m_netlist;
		std::unordered_map<pstring, param_ref_t>    m_params;
		std::vector<link_t>                         m_links;
		factory::list_t                             m_factory;
		std::unordered_map<pstring, pstring>        m_models;

		std::stack<pstring>                         m_namespace_stack;
		source_t::list_t                            m_sources;
		std::vector<plib::ppreprocessor::define_t>  m_defines;

		unsigned m_proxy_cnt;
		unsigned m_frontier_cnt;
};

	// ----------------------------------------------------------------------------------------
	// base sources
	// ----------------------------------------------------------------------------------------


	class source_string_t : public source_t
	{
	public:

		source_string_t(setup_t &setup, const pstring &source)
		: source_t(setup), m_str(source)
		{
		}

		virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_file_t : public source_t
	{
	public:

		source_file_t(setup_t &setup, const pstring &filename)
		: source_t(setup), m_filename(filename)
		{
		}

		virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;

	private:
		pstring m_filename;
	};

	class source_mem_t : public source_t
	{
	public:
		source_mem_t(setup_t &setup, const char *mem)
		: source_t(setup), m_str(mem, pstring::UTF8)
		{
		}

		virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_proc_t : public source_t
	{
	public:
		source_proc_t(setup_t &setup, pstring name, void (*setup_func)(setup_t &))
		: source_t(setup),
			m_setup_func(setup_func),
			m_setup_func_name(name)
		{
		}

		virtual bool parse(const pstring &name) override;
		virtual std::unique_ptr<plib::pistream> stream(const pstring &name) override;

	private:
		void (*m_setup_func)(setup_t &);
		pstring m_setup_func_name;
	};

}


#endif /* NLSETUP_H_ */
