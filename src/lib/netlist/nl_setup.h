// license:GPL-2.0+
// copyright-holders:Couriersud

///
/// \file nl_setup.h
///

#ifndef NLSETUP_H_
#define NLSETUP_H_

#define NL_AUTO_DEVICES 1

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
		setup.register_dev(# type, { PSTRINGIFY_VA(__VA_ARGS__) });

#define NET_CONNECT(name, input, output)                                       \
		setup.register_link(# name "." # input, # output);

#define NET_C(term1, ...)                                                      \
		setup.register_link_arr( # term1 ", " # __VA_ARGS__);

#define PARAM(name, val)                                                       \
		setup.register_param(NET_STR(name), NET_STR(val));

#define DEFPARAM(name, val)                                                       \
		setup.defparam(NET_STR(name), NET_STR(val));

#define HINT(name, val)                                                        \
		setup.register_param(# name ".HINT_" # val, "1");

#define NETDEV_PARAMI(name, param, val)                                        \
		setup.register_param(# name "." # param, val);

#define NETLIST_NAME(name) netlist ## _ ## name

#define NETLIST_EXTERNAL(name)                                                 \
		void NETLIST_NAME(name)(netlist::nlparse_t &setup);

#define NETLIST_START(name)                                                    \
void NETLIST_NAME(name)(netlist::nlparse_t &setup)                             \
{                                                                              \
	plib::unused_var(setup);

#define NETLIST_END()  }

#define LOCAL_SOURCE(name)                                                     \
		setup.register_source<netlist::source_proc_t>(# name, &NETLIST_NAME(name));

// FIXME: Need to pass in parameter definition
#define LOCAL_LIB_ENTRY_1(name)                                                \
		LOCAL_SOURCE(name)                                                     \
		setup.register_lib_entry(# name, "", __FILE__);

#define LOCAL_LIB_ENTRY_2(name, param_spec)                                    \
		LOCAL_SOURCE(name)                                                     \
		setup.register_lib_entry(# name, param_spec, __FILE__);

//#define LOCAL_LIB_ENTRY(...) PMSVC_VARARG_BUG(PCONCAT, (LOCAL_LIB_ENTRY_, PNARGS(__VA_ARGS__)))(__VA_ARGS__)

#define LOCAL_LIB_ENTRY(...) PCALLVARARG(LOCAL_LIB_ENTRY_, __VA_ARGS__)

#define INCLUDE(name)                                                          \
		setup.include(# name);

#define SUBMODEL(model, name)                                                  \
		setup.namespace_push(# name);                                          \
		setup.include(# model);                                                \
		setup.namespace_pop();

#define OPTIMIZE_FRONTIER(attach, r_in, r_out)                                 \
		setup.register_frontier(# attach, PSTRINGIFY_VA(r_in), PSTRINGIFY_VA(r_out));

// -----------------------------------------------------------------------------
// truthtable defines
// -----------------------------------------------------------------------------

#define TRUTHTABLE_START(cname, in, out, def_params) \
	{ \
		netlist::tt_desc desc; \
		desc.name = #cname ; \
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
		param_ref_t() : m_device(nullptr), m_param(nullptr) {}
		param_ref_t(core_device_t &device, param_t &param)
		: m_device(&device)
		, m_param(&param)
		{ }
		PCOPYASSIGNMOVE(param_ref_t, default)

		const core_device_t &device() const noexcept { return *m_device; }
		param_t &param() const noexcept { return *m_param; }

		bool is_valid() const { return (m_device != nullptr) && (m_param != nullptr); }
	private:
		core_device_t *m_device;
		param_t *m_param;
	};

	// ----------------------------------------------------------------------------------------
	// Specific netlist psource_t implementations
	// ----------------------------------------------------------------------------------------

	class source_netlist_t : public plib::psource_t
	{
	public:

		source_netlist_t() = default;

		PCOPYASSIGNMOVE(source_netlist_t, delete)
		~source_netlist_t() noexcept override = default;

		virtual bool parse(nlparse_t &setup, const pstring &name);
	};

	class source_data_t : public plib::psource_t
	{
	public:

		source_data_t() = default;

		PCOPYASSIGNMOVE(source_data_t, delete)
		~source_data_t() noexcept override = default;
	};

	// ----------------------------------------------------------------------------------------
	// Collection of models
	// ----------------------------------------------------------------------------------------

	class models_t
	{
	public:
		using model_map_t = std::unordered_map<pstring, pstring>;
		class model_t
		{
		public:
			model_t(const pstring &model, const model_map_t &map)
			: m_model(model), m_map(map) { }

			pstring value_str(const pstring &entity) const;

			nl_fptype value(const pstring &entity) const;

			pstring type() const { return value_str("COREMODEL"); }

		private:
			static pstring model_string(const model_map_t &map);

			const pstring m_model; // only for error messages
			const model_map_t &m_map;
		};

		void register_model(const pstring &model_in);

		model_t get_model(const pstring &model);

	private:

		void model_parse(const pstring &model, model_map_t &map);

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

		struct abstract_t
		{
			std::unordered_map<pstring, pstring>        m_alias;
			std::vector<link_t>                         m_links;
			std::unordered_map<pstring, pstring>        m_param_values;

			// need to preserve order of device creation ...
			std::vector<std::pair<pstring, factory::element_t *>> m_device_factory;
			// lifetime control only - can be cleared before run
			std::vector<std::pair<pstring, pstring>>    m_defparams;
		};

		nlparse_t(log_type &log);

		void register_model(const pstring &model_in) { m_models.register_model(model_in); }
		void register_alias(const pstring &alias, const pstring &out);
		void register_alias_nofqn(const pstring &alias, const pstring &out);
		void register_dip_alias_arr(const pstring &terms);

		// last argument only needed by nltool
		void register_dev(const pstring &classname, const pstring &name,
			const std::vector<pstring> &params_and_connections,
			factory::element_t **felem = nullptr);
		void register_dev(const pstring &classname, std::initializer_list<const char *> more_parameters);
		void register_dev(const pstring &classname, const pstring &name)
		{
			register_dev(classname, name, std::vector<pstring>());
		}

		void register_link(const pstring &sin, const pstring &sout);
		void register_link_arr(const pstring &terms);
		// also called from devices for latebinding connected terminals
		void register_link_fqn(const pstring &sin, const pstring &sout);

		void register_param(const pstring &param, const pstring &value);
		void register_param(const pstring &param, nl_fptype value);

		template <typename T>
		typename std::enable_if<plib::is_floating_point<T>::value || plib::is_integral<T>::value>::type
		register_param_val(const pstring &param, T value)
		{
			register_param(param, static_cast<nl_fptype>(value));
		}

		void register_lib_entry(const pstring &name, const pstring &paramdef, const pstring &sourcefile);

		void register_frontier(const pstring &attach, const pstring &r_IN, const pstring &r_OUT);

		// register a source
		template <typename S, typename... Args>
		void register_source(Args&&... args)
		{
			static_assert(std::is_base_of<plib::psource_t, S>::value, "S must inherit from plib::psource_t");

			auto src(plib::make_unique<S>(std::forward<Args>(args)...));
			m_sources.add_source(std::move(src));
		}

		void truthtable_create(tt_desc &desc, const pstring &sourcefile);

		// handle namespace

		void namespace_push(const pstring &aname);
		void namespace_pop();
		pstring namespace_prefix() const;
		pstring build_fqn(const pstring &obj_name) const;

		// include other files

		void include(const pstring &netlist_name);

		// used from netlist.cpp (mame)
		bool device_exists(const pstring &name) const;

		// FIXME: used by source_t - need a different approach at some time
		bool parse_stream(plib::psource_t::stream_ptr &&istrm, const pstring &name);

		template <typename S, typename... Args>
		void add_include(Args&&... args)
		{
			static_assert(std::is_base_of<plib::psource_t, S>::value, "S must inherit from plib::psource_t");

			auto src(plib::make_unique<S>(std::forward<Args>(args)...));
			m_includes.add_source(std::move(src));
		}

		void add_define(const pstring &def, const pstring &val)
		{
			m_defines.insert({ def, plib::ppreprocessor::define_t(def, val)});
		}

		void add_define(const pstring &defstr);

		// DEFPARAM support
		void defparam(const pstring &name, const pstring &def);

		// register a list of logs
		void register_dynamic_log_devices(const std::vector<pstring> &loglist);

		factory::list_t &factory() noexcept { return m_factory; }
		const factory::list_t &factory() const noexcept  { return m_factory; }

		log_type &log() noexcept { return m_log; }
		const log_type &log() const noexcept { return m_log; }

		models_t &models() noexcept { return m_models; }
		const models_t &models() const noexcept { return m_models; }

		plib::psource_t::stream_ptr get_data_stream(const pstring &name);

		abstract_t &result() { return m_abstract; }
		const abstract_t &result() const { return m_abstract; }

	private:
		// FIXME: stale? - remove later
		void remove_connections(const pstring &pin);

		plib::ppreprocessor::defines_map_type       m_defines;
		plib::psource_collection_t<>                m_includes;
		models_t                                    m_models;
		std::stack<pstring>                         m_namespace_stack;
		plib::psource_collection_t<>                m_sources;
		// FIXME: convert to hash and deal with sorting in nltool
		factory::list_t                             m_factory;
		abstract_t                                  m_abstract;

		log_type &m_log;
		unsigned m_frontier_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// setup_t
	// ----------------------------------------------------------------------------------------

	class setup_t
	{
	public:

		explicit setup_t(netlist_state_t &nlstate);
		~setup_t() noexcept = default;

		PCOPYASSIGNMOVE(setup_t, delete)

		// called from param_t creation
		void register_param_t(param_t &param);
		pstring get_initial_param_val(const pstring &name, const pstring &def) const;

		void register_term(detail::core_terminal_t &term);
		void register_term(terminal_t &term, terminal_t &other_term);

		// called from net_splitter
		terminal_t *get_connected_terminal(const terminal_t &term) const noexcept
		{
			auto ret(m_connected_terminals.find(&term));
			return (ret != m_connected_terminals.end()) ? ret->second : nullptr;
		}

		// get family -> truthtable
		const logic_family_desc_t *family_from_model(const pstring &model);

		// FIXME: return param_ref_t
		param_ref_t find_param(const pstring &param_in) const;
		// needed by nltool
		std::vector<pstring> get_terminals_for_device_name(const pstring &devname) const;

		// needed by proxy device to check power terminals
		detail::core_terminal_t *find_terminal(const pstring &terminal_in, detail::terminal_type atype, bool required = true) const;
		detail::core_terminal_t *find_terminal(const pstring &terminal_in, bool required = true) const;
		pstring de_alias(const pstring &alias) const;
		// FIXME: only needed by solver code outside of setup_t
		bool connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		// run preparation

		void prepare_to_run();

		netlist_state_t &nlstate() { return m_nlstate; }
		const netlist_state_t &nlstate() const { return m_nlstate; }

		nlparse_t &parser() { return m_parser; }
		const nlparse_t &parser() const { return m_parser; }

		log_type &log() noexcept;
		const log_type &log() const noexcept;

	private:

		void resolve_inputs();
		pstring resolve_alias(const pstring &name) const;
		void delete_empty_nets();

		void merge_nets(detail::net_t &thisnet, detail::net_t &othernet);

		void connect_terminals(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
		void connect_input_output(detail::core_terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, detail::core_terminal_t &inp);
		bool connect_input_input(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		// helpers
		static pstring termtype_as_str(detail::core_terminal_t &in);

		devices::nld_base_proxy *get_d_a_proxy(const detail::core_terminal_t &out);
		devices::nld_base_proxy *get_a_d_proxy(detail::core_terminal_t &inp);
		detail::core_terminal_t &resolve_proxy(detail::core_terminal_t &term);

		nlparse_t                                   m_parser;
		netlist_state_t                             &m_nlstate;

		// FIXME: currently only used during setup
		devices::nld_netlistparams                  *m_netlist_params;

		// FIXME: can be cleared before run
		std::unordered_map<pstring, detail::core_terminal_t *> m_terminals;
		std::unordered_map<const terminal_t *, terminal_t *> m_connected_terminals;
		std::unordered_map<pstring, param_ref_t>    m_params;
		std::unordered_map<const detail::core_terminal_t *,
			devices::nld_base_proxy *>              m_proxies;
		std::vector<plib::unique_ptr<param_t>> 		m_defparam_lifetime;

		unsigned m_proxy_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// base sources
	// ----------------------------------------------------------------------------------------

	class source_string_t : public source_netlist_t
	{
	public:

		explicit source_string_t(const pstring &source)
		: m_str(source)
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
		: m_filename(filename)
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
		: m_str(mem)
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
		: m_setup_func(setup_func)
		, m_setup_func_name(name)
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
