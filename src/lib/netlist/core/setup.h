// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file setup.h
///

#ifndef NL_CORE_SETUP_H_
#define NL_CORE_SETUP_H_

#include "../nl_config.h"
#include "../nl_factory.h"
#include "../nl_setup.h"
#include "../nltypes.h"

#include "../plib/pstream.h"
#include "../plib/pstring.h"

#include <initializer_list>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>


namespace netlist
{

	// ----------------------------------------------------------------------------------------
	// Collection of models
	// ----------------------------------------------------------------------------------------

	class models_t
	{
	public:
		using raw_map_t = std::unordered_map<pstring, pstring>;
		using map_t = std::unordered_map<pstring, pstring>;
		class model_t
		{
		public:
			model_t(const pstring &model, const map_t &map)
			: m_model(model), m_map(map) { }

			pstring value_str(const pstring &entity) const;

			nl_fptype value(const pstring &entity) const;

			pstring type() const { return value_str("COREMODEL"); }

		private:
			static pstring model_string(const map_t &map);

			const pstring m_model; // only for error messages
			const map_t &m_map;
		};

		models_t(const raw_map_t &models)
		: m_models(models)
		{}

		model_t get_model(const pstring &model);

		std::vector<pstring> known_models() const
		{
			std::vector<pstring> ret;
			for (const auto &e : m_models)
				ret.push_back(e.first);
			return ret;
		}

	private:

		void model_parse(const pstring &model, map_t &map);

		const raw_map_t &m_models;
		std::unordered_map<pstring, map_t> m_cache;
	};

	namespace detail
	{
		// -----------------------------------------------------------------------------
		// abstract_t
		// -----------------------------------------------------------------------------

		struct abstract_t
		{
			using link_t = std::pair<pstring, pstring>;

			abstract_t(log_type &log) : m_factory(log) { }
			std::unordered_map<pstring, pstring>        m_alias;
			std::vector<link_t>                         m_links;
			std::unordered_map<pstring, pstring>        m_param_values;
			models_t::raw_map_t                         m_models;

			// need to preserve order of device creation ...
			std::vector<std::pair<pstring, factory::element_t *>> m_device_factory;
			// lifetime control only - can be cleared before run
			std::vector<std::pair<pstring, pstring>>    m_default_params;
			std::unordered_map<pstring, bool>           m_hints;
			factory::list_t                             m_factory;
		};
	} // namespace detail

	// -----------------------------------------------------------------------------
	// param_ref_t
	// -----------------------------------------------------------------------------

	struct param_ref_t
	{
		param_ref_t() noexcept : m_device(nullptr), m_param(nullptr) {}
		param_ref_t(core_device_t &device, param_t &param) noexcept
		: m_device(&device)
		, m_param(&param)
		{ }

		~param_ref_t() = default;
		PCOPYASSIGNMOVE(param_ref_t, default)

		const core_device_t &device() const noexcept { return *m_device; }
		param_t &param() const noexcept { return *m_param; }

		bool is_valid() const noexcept { return (m_device != nullptr) && (m_param != nullptr); }
	private:
		core_device_t *m_device;
		param_t *m_param;
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
		void register_term(terminal_t &term, terminal_t *other_term, const std::array<terminal_t *, 2> &splitter_terms);

		// called from matrix_solver_t::get_connected_net
		// returns the terminal being part of a two terminal device.
		terminal_t *get_connected_terminal(const terminal_t &term) const noexcept
		{
			auto ret(m_connected_terminals.find(&term));
			return (ret != m_connected_terminals.end()) ? ret->second[0] : nullptr;
		}

		// called from net_splitter
		const std::array<terminal_t *, 4> *get_connected_terminals(const terminal_t &term) const noexcept
		{
			auto ret(m_connected_terminals.find(&term));
			return (ret != m_connected_terminals.end()) ? &ret->second : nullptr;
		}

		// get family -> truth table
		const logic_family_desc_t *family_from_model(const pstring &model);

		param_ref_t find_param(const pstring &param_in) const;
		// needed by nltool
		std::vector<pstring> get_terminals_for_device_name(const pstring &devname) const;

		// needed by proxy device to check power terminals
		detail::core_terminal_t *find_terminal(const pstring &terminal_in, detail::terminal_type atype, bool required = true) const;
		detail::core_terminal_t *find_terminal(const pstring &terminal_in, bool required = true) const;
		pstring de_alias(const pstring &alias) const;

		// run preparation

		void prepare_to_run();

		models_t &models() noexcept { return m_models; }
		const models_t &models() const noexcept { return m_models; }

		netlist_state_t &nlstate() { return m_nlstate; }
		const netlist_state_t &nlstate() const { return m_nlstate; }

		nlparse_t &parser() { return m_parser; }
		const nlparse_t &parser() const { return m_parser; }

		log_type &log() noexcept;
		const log_type &log() const noexcept;

	private:

		void resolve_inputs();
		pstring resolve_alias(const pstring &name) const;

		void merge_nets(detail::net_t &this_net, detail::net_t &other_net);

		void connect_terminals(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
		void connect_input_output(detail::core_terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_output(terminal_t &in, detail::core_terminal_t &out);
		void connect_terminal_input(terminal_t &term, detail::core_terminal_t &inp);
		bool connect_input_input(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		bool connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2);

		// helpers
		static pstring termtype_as_str(detail::core_terminal_t &in);

		devices::nld_base_proxy *get_d_a_proxy(const detail::core_terminal_t &out);
		devices::nld_base_proxy *get_a_d_proxy(detail::core_terminal_t &inp);
		detail::core_terminal_t &resolve_proxy(detail::core_terminal_t &term);

		// net manipulations

		//void remove_terminal(detail::net_t &net, detail::core_terminal_t &terminal) noexcept(false);
		void move_connections(detail::net_t &net, detail::net_t &dest_net);
		void delete_empty_nets();

		detail::abstract_t                          m_abstract;
		nlparse_t                                   m_parser;
		netlist_state_t                             &m_nlstate;

		models_t                                    m_models;

		// FIXME: currently only used during setup
		devices::nld_netlistparams *                           m_netlist_params;

		// FIXME: can be cleared before run
		std::unordered_map<pstring, detail::core_terminal_t *> m_terminals;
		// FIXME: Limited to 3 additional terminals
		std::unordered_map<const terminal_t *,
			std::array<terminal_t *, 4>>                       m_connected_terminals;
		std::unordered_map<pstring, param_ref_t>               m_params;
		std::unordered_map<const detail::core_terminal_t *,
			devices::nld_base_proxy *>                         m_proxies;
		std::vector<host_arena::unique_ptr<param_t>>           m_defparam_lifetime;

		unsigned m_proxy_cnt;
	};

	// ----------------------------------------------------------------------------------------
	// Specific netlist `psource_t` implementations
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

	class source_string_t : public source_netlist_t
	{
	public:

		explicit source_string_t(const pstring &source)
		: m_str(source)
		{
		}

	protected:
		plib::istream_uptr stream(const pstring &name) override;

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
		plib::istream_uptr stream(const pstring &name) override;

	private:
		pstring m_filename;
	};

	class source_pattern_t : public source_netlist_t
	{
	public:

		explicit source_pattern_t(const pstring &pat, bool force_lowercase)
		: m_pattern(pat)
		, m_force_lowercase(force_lowercase)
		{
		}

	protected:
		plib::istream_uptr stream(const pstring &name) override;

	private:
		pstring m_pattern;
		bool m_force_lowercase;
	};

	class source_mem_t : public source_netlist_t
	{
	public:
		explicit source_mem_t(const char *mem)
		: m_str(mem)
		{
		}

	protected:
		plib::istream_uptr stream(const pstring &name) override;

	private:
		std::string m_str;
	};

	class source_proc_t : public source_netlist_t
	{
	public:
		source_proc_t(const pstring &name, nlsetup_func setup_func)
		: m_setup_func(setup_func)
		, m_setup_func_name(name)
		{
		}

		bool parse(nlparse_t &setup, const pstring &name) override;

	protected:
		plib::istream_uptr stream(const pstring &name) override;

	private:
		nlsetup_func m_setup_func;
		pstring m_setup_func_name;
	};

} // namespace netlist


#endif // NL_CORE_SETUP_H_
