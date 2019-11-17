// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_setup.h
///

#ifndef NLSETUP_H_
#define NLSETUP_H_

#include "plib/ppreprocessor.h"
#include "plib/pstream.h"
#include "plib/pstring.h"
#include "plib/putil.h"

#include "nl_config.h"
#include "nl_factory.h"
#include "nltypes.h"

#include <initializer_list>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_STR(x) # x

#define NET_MODEL(model)                                                       \
	setup.register_model(model);

#define ALIAS(alias, name)                                                     \
	setup.register_alias(# alias, # name);

#define DIPPINS(pin1, ...)                                                     \
		setup.register_dip_alias_arr( # pin1 ", " # __VA_ARGS__);

// to be used to reference new library truthtable devices
#define NET_REGISTER_DEV(type, name)                                           \
		setup.register_dev(# type, # name);

// name is first element so that __VA_ARGS__ always has one element
#define NET_REGISTER_DEVEXT(type, ...)                                   \
		setup.register_devx(# type, { PSTRINGIFY_VA(__VA_ARGS__) });

#define NET_CONNECT(name, input, output)                                       \
		setup.register_link(# name "." # input, # output);

#define NET_C(term1, ...)                                                      \
		setup.register_link_arr( # term1 ", " # __VA_ARGS__);

#define PARAM(name, val)                                                       \
		setup.register_param(# name, val);

#define HINT(name, val)                                                        \
		setup.register_param(# name ".HINT_" # val, 1);

#define NETDEV_PARAMI(name, param, val)                                        \
		setup.register_param(# name "." # param, val);

#define NETLIST_NAME(name) netlist ## _ ## name

#define NETLIST_EXTERNAL(name)                                                 \
		void NETLIST_NAME(name)(netlist::nlparse_t &setup);

#define NETLIST_START(name)                                                    \
void NETLIST_NAME(name)(netlist::nlparse_t &setup)                             \
{

#define NETLIST_END()  }

#define LOCAL_SOURCE(name)                                                     \
		setup.register_source(plib::make_unique<netlist::source_proc_t>(# name, &NETLIST_NAME(name)));

#define LOCAL_LIB_ENTRY(name)                                                  \
		LOCAL_SOURCE(name)                                                     \
		setup.register_lib_entry(# name, __FILE__);

#define INCLUDE(name)                                                          \
		setup.include(# name);

#define SUBMODEL(model, name)                                                  \
		setup.namespace_push(# name);                                          \
		setup.include(# model);                                                \
		setup.namespace_pop();

#define OPTIMIZE_FRONTIER(attach, r_in, r_out)                                 \
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
		desc.desc.emplace_back(x);

#define TT_LINE(x) \
		desc.desc.emplace_back(x);

#define TT_FAMILY(x) \
		desc.family = x;

#define TRUTHTABLE_END() \
		setup.truthtable_create(desc, __FILE__);       \
	}

namespace netlist
{

	namespace detail {
		class core_terminal_t;
		class net_t;
	} // namespace detail

	namespace devices {
		class nld_base_proxy;
		class nld_netlistparams;
	} // namespace devices

	class core_device_t;
	class param_t;
	class setup_t;
	class netlist_state_t;
	class netlist_t;
	class logic_family_desc_t;
	class terminal_t;

	// -----------------------------------------------------------------------------
	// truthtable desc
	// -----------------------------------------------------------------------------

	struct tt_desc
	{
		tt_desc() : ni(0), no(0) { }
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
		const pstring &name() const noexcept { return m_name; }
		const core_device_t &device() const noexcept { return m_device; }
		param_t *param() const noexcept { return &m_param; }
	private:
		pstring m_name;
		core_device_t &m_device;
		param_t &m_param;
	};

	// ----------------------------------------------------------------------------------------
	// Specific netlist psource_t implementations
	// ----------------------------------------------------------------------------------------

	class source_netlist_t : public plib::psource_t
	{
	public:

		friend class setup_t;

		source_netlist_t()
		: plib::psource_t()
		{}

		COPYASSIGNMOVE(source_netlist_t, delete)
		~source_netlist_t() noexcept override = default;

		virtual bool parse(nlparse_t &setup, const pstring &name);
	};

	class source_data_t : public plib::psource_t
	{
	public:

		friend class setup_t;

		source_data_t()
		: plib::psource_t()
		{}

		COPYASSIGNMOVE(source_data_t, delete)
		~source_data_t() noexcept override = default;
	};

	// ----------------------------------------------------------------------------------------
	// Collection of models
	// ----------------------------------------------------------------------------------------

	class models_t
	{
	public:
		void register_model(const pstring &model_in);
		// model / family related

		pstring value_str(const pstring &model, const pstring &entity);

		nl_fptype value(const pstring &model, const pstring &entity);

		pstring type(const pstring &model) { return value_str(model, "COREMODEL"); }

	private:
		using model_map_t = std::unordered_map<pstring, pstring>;

		void model_parse(const pstring &model, model_map_t &map);
		pstring model_string(const model_map_t &map) const;

		std::unordered_map<pstring, pstring> m_models;
		std::unordered_map<pstring, model_map_t> m_cache;
	};

	// ----------------------------------------------------------------------------------------
	// nlparse_t
	// ----------------------------------------------------------------------------------------

	class nlparse_t
	{
	public:
		using link_t = std::pair<pstring, pstring>;

		nlparse_t(setup_t &netlist, log_type &log);

		void register_model(const pstring &model_in) { m_models.register_model(model_in); }
		void register_alias(const pstring &alias, const pstring &out);
		void register_dip_alias_arr(const pstring &terms);
		void register_dev(const pstring &classname, const pstring &name);
		void register_dev(const pstring &classname, const pstring &name,
			const std::vector<pstring> &params_and_connections);

		void register_devx(const pstring &classname, std::initializer_list<const char *> params_and_connections);
		void register_dev(const pstring &classname, const pstring &name,
			const char *params_and_connections);

		void register_link(const pstring &sin, const pstring &sout);
		void register_link_arr(const pstring &terms);
		void register_param(const pstring &param, const pstring &value);

		// FIXME: quick hack
		void register_param_x(const pstring &param, const nl_fptype value);

		template <typename T>
		typename std::enable_if<std::is_floating_point<T>::value || std::is_integral<T>::value>::type
		register_param(const pstring &param, T value)
		{
			register_param_x(param, static_cast<nl_fptype>(value));
		}

#if PUSE_FLOAT128
		void register_param(const pstring &param, __float128 value)
		{
			register_param_x(param, static_cast<nl_fptype>(value));
		}
#endif

		void register_lib_entry(const pstring &name, const pstring &sourcefile);
		void register_frontier(const pstring &attach, const nl_fptype r_IN, const nl_fptype r_OUT);

		// register a source
		void register_source(plib::unique_ptr<plib::psource_t> &&src)
		{
			m_sources.add_source(std::move(src));
		}

		void truthtable_create(tt_desc &desc, const pstring &sourcefile);

		// handle namespace

		void namespace_push(const pstring &aname);
		void namespace_pop();

		// include other files

		void include(const pstring &netlist_name);

		pstring build_fqn(const pstring &obj_name) const;
		void register_alias_nofqn(const pstring &alias, const pstring &out);

		// also called from devices for latebinding connected terminals
		void register_link_fqn(const pstring &sin, const pstring &sout);

		// used from netlist.cpp (mame)
		bool device_exists(const pstring &name) const;

		// FIXME: used by source_t - need a different approach at some time
		bool parse_stream(plib::psource_t::stream_ptr &&istrm, const pstring &name);

		void add_include(plib::unique_ptr<plib::psource_t> &&inc)
		{
			m_includes.add_source(std::move(inc));
		}

		void add_define(const pstring &def, const pstring &val)
		{
			m_defines.insert({ def, plib::ppreprocessor::define_t(def, val)});
		}

		void add_define(const pstring &defstr);

		factory::list_t &factory() noexcept { return m_factory; }
		const factory::list_t &factory() const noexcept  { return m_factory; }

		log_type &log() noexcept { return m_log; }
		const log_type &log() const noexcept { return m_log; }

		// FIXME: sources may need access to the netlist parent type
		// since they may be created in a context in which they don't
		// have access to their environment.
		// Example is the MAME memregion source.
		// We thus need a better approach to creating netlists in a context
		// other than static procedures.

		setup_t &setup() noexcept { return m_setup; }
		const setup_t &setup() const noexcept { return m_setup; }

		models_t &models() noexcept { return m_models; }
		const models_t &models() const noexcept { return m_models; }

	protected:
		models_t                                    m_models;
		std::stack<pstring>                         m_namespace_stack;
		std::unordered_map<pstring, pstring>        m_alias;
		std::vector<link_t>                         m_links;
		std::unordered_map<pstring, pstring>        m_param_values;

		plib::psource_collection_t<>                m_sources;

		factory::list_t                             m_factory;

		// need to preserve order of device creation ...
		std::vector<std::pair<pstring, factory::element_t *>> m_device_factory;


	private:
		plib::ppreprocessor::defines_map_type       m_defines;
		plib::psource_collection_t<>                m_includes;

		setup_t  &m_setup;
		log_type &m_log;
		unsigned m_frontier_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// setup_t
	// ----------------------------------------------------------------------------------------

	class setup_t : public nlparse_t
	{
	public:

		explicit setup_t(netlist_state_t &nlstate);
		~setup_t() noexcept = default;

		COPYASSIGNMOVE(setup_t, delete)

		netlist_state_t &nlstate() { return m_nlstate; }
		const netlist_state_t &nlstate() const { return m_nlstate; }

		void register_param_t(const pstring &name, param_t &param);

		pstring get_initial_param_val(const pstring &name, const pstring &def) const;

		void register_term(detail::core_terminal_t &term);

		void remove_connections(const pstring &pin);

		bool connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		param_t *find_param(const pstring &param_in, bool required = true) const;

		// get family
		const logic_family_desc_t *family_from_model(const pstring &model);

		void register_dynamic_log_devices();
		void resolve_inputs();

		plib::psource_t::stream_ptr get_data_stream(const pstring &name);

		factory::list_t &factory() { return m_factory; }
		const factory::list_t &factory() const { return m_factory; }

		// helper - also used by nltool
		pstring resolve_alias(const pstring &name) const;
		pstring de_alias(const pstring &alias) const;

		// needed by nltool
		std::vector<pstring> get_terminals_for_device_name(const pstring &devname) const;

		log_type &log();
		const log_type &log() const;

		// needed by proxy
		detail::core_terminal_t *find_terminal(const pstring &outname_in, const detail::terminal_type atype, bool required = true) const;
		detail::core_terminal_t *find_terminal(const pstring &terminal_in, bool required = true) const;

		// core net handling

		void delete_empty_nets();

		// run preparation

		void prepare_to_run();

	private:

		void merge_nets(detail::net_t &thisnet, detail::net_t &othernet);

		void connect_terminals(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
		void connect_input_output(detail::core_terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, detail::core_terminal_t &inp);
		bool connect_input_input(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		// helpers
		pstring termtype_as_str(detail::core_terminal_t &in) const;

		devices::nld_base_proxy *get_d_a_proxy(detail::core_terminal_t &out);
		devices::nld_base_proxy *get_a_d_proxy(detail::core_terminal_t &inp);
		detail::core_terminal_t &resolve_proxy(detail::core_terminal_t &term);

		std::unordered_map<pstring, detail::core_terminal_t *> m_terminals;

		netlist_state_t                             &m_nlstate;
		devices::nld_netlistparams                  *m_netlist_params;
		std::unordered_map<pstring, param_ref_t>    m_params;
		std::unordered_map<detail::core_terminal_t *,
			devices::nld_base_proxy *>				m_proxies;

		unsigned m_proxy_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// base sources
	// ----------------------------------------------------------------------------------------

	class source_string_t : public source_netlist_t
	{
	public:

		explicit source_string_t(const pstring &source)
		: source_netlist_t(), m_str(source)
		{
		}

	protected:
		stream_ptr stream(const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_file_t : public source_netlist_t
	{
	public:

		explicit source_file_t(const pstring &filename)
		: source_netlist_t(), m_filename(filename)
		{
		}

	protected:
		stream_ptr stream(const pstring &name) override;

	private:
		pstring m_filename;
	};

	class source_mem_t : public source_netlist_t
	{
	public:
		explicit source_mem_t(const char *mem)
		: source_netlist_t(), m_str(mem)
		{
		}

	protected:
		stream_ptr stream(const pstring &name) override;

	private:
		pstring m_str;
	};

	class source_proc_t : public source_netlist_t
	{
	public:
		source_proc_t(const pstring &name, void (*setup_func)(nlparse_t &))
		: source_netlist_t(),
			m_setup_func(setup_func),
			m_setup_func_name(name)
		{
		}

		bool parse(nlparse_t &setup, const pstring &name) override;

	protected:
		stream_ptr stream(const pstring &name) override;

	private:
		void (*m_setup_func)(nlparse_t &);
		pstring m_setup_func_name;
	};

	// -----------------------------------------------------------------------------
	// inline implementations
	// -----------------------------------------------------------------------------

} // namespace netlist


#endif // NLSETUP_H_
