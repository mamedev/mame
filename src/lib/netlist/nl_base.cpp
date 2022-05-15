// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "solver/nld_matrix_solver.h"
#include "solver/nld_solver.h"

#include "plib/palloc.h"
#include "plib/pdynlib.h"
#include "plib/pfmtlog.h"
#include "plib/pmempool.h"
#include "plib/putil.h"

#include "core/setup.h"
#include "devices/nlid_proxy.h"
#include "nl_base.h"

#include "nl_errstr.h"

#include <limits>

NETLIST_EXTERNAL(base_lib)

namespace netlist
{

	// ----------------------------------------------------------------------------------------
	// callbacks_t
	// ----------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------
	// queue_t
	// ----------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------
	// device_object_t
	// ----------------------------------------------------------------------------------------

	detail::device_object_t::device_object_t(core_device_t *dev, const pstring &aname)
	: object_t(aname)
	, m_device(dev)
	{
	}

	detail::terminal_type detail::core_terminal_t::type() const noexcept(false)
	{
		if (dynamic_cast<const terminal_t *>(this) != nullptr)
			return terminal_type::TERMINAL;
		if (dynamic_cast<const logic_input_t *>(this) != nullptr
			|| dynamic_cast<const analog_input_t *>(this) != nullptr)
			return terminal_type::INPUT;
		if (dynamic_cast<const logic_output_t *>(this) != nullptr
			|| dynamic_cast<const analog_output_t *>(this) != nullptr)
			return terminal_type::OUTPUT;

		state().log().fatal(MF_UNKNOWN_TYPE_FOR_OBJECT(name()));
		throw nl_exception(MF_UNKNOWN_TYPE_FOR_OBJECT(name()));
		//return terminal_type::TERMINAL; // please compiler
	}

	netlist_state_t &detail::device_object_t::state() noexcept
	{
		return m_device->state();
	}

	const netlist_state_t &detail::device_object_t::state() const noexcept
	{
		return m_device->state();
	}

	// ----------------------------------------------------------------------------------------
	// netlist_object_t
	// ----------------------------------------------------------------------------------------

	netlist_state_t & detail::netlist_object_t::state() noexcept
	{
		return m_netlist.nlstate();
	}

	const netlist_state_t & detail::netlist_object_t::state() const noexcept
	{
		return m_netlist.nlstate();
	}

	// ----------------------------------------------------------------------------------------
	// netlist_t
	// ----------------------------------------------------------------------------------------

	netlist_t::netlist_t(netlist_state_t &state, const pstring &aname)
	: m_state(state)
	, m_solver(nullptr)
	, m_time(netlist_time_ext::zero())
	, m_mainclock(nullptr)
	, m_use_stats(false)
	, m_queue(config::max_queue_size::value,
		detail::queue_t::id_delegate(&netlist_state_t :: find_net_id, &state),
		detail::queue_t::obj_delegate(&netlist_state_t :: net_by_id, &state))
	{
		state.save(*this, static_cast<plib::state_manager_t::callback_t &>(m_queue), aname, "m_queue");
		state.save(*this, m_time, aname, "m_time");
	}

	// ----------------------------------------------------------------------------------------
	// netlist_t
	// ----------------------------------------------------------------------------------------

	netlist_state_t::netlist_state_t(const pstring &name,
		plib::plog_delegate logger)
	: m_log(logger)
	, m_dummy_version(1)
	{
		m_setup = plib::make_unique<setup_t, host_arena>(*this);
		// create the run interface
		m_netlist = plib::make_unique<netlist_t>(m_pool, *this, name);

		// Make sure save states are invalidated when a new version is deployed

		m_state.save_item(this, m_dummy_version, pstring("V") + version());

		// Initialize factory
		devices::initialize_factory(m_setup->parser().factory());

		// Add default include file
		const pstring content =
		"#define RES_R(res) (res)            \n"
		"#define RES_K(res) ((res) * 1e3)    \n"
		"#define RES_M(res) ((res) * 1e6)    \n"
		"#define CAP_U(cap) ((cap) * 1e-6)   \n"
		"#define CAP_N(cap) ((cap) * 1e-9)   \n"
		"#define CAP_P(cap) ((cap) * 1e-12)  \n"
		"#define IND_U(ind) ((ind) * 1e-6)   \n"
		"#define IND_N(ind) ((ind) * 1e-9)   \n"
		"#define IND_P(ind) ((ind) * 1e-12)  \n";
		m_setup->parser().add_include<plib::psource_str_t>("netlist/devices/net_lib.h", content);

		// This is for core macro libraries
		m_setup->parser().add_include<plib::psource_str_t>("devices/net_lib.h", content);
#if 1
		NETLIST_NAME(base_lib)(m_setup->parser());
		//m_setup->parser().register_source<source_pattern_t>("../macro/modules/nlmod_{1}.cpp");
		//m_setup->parser().register_source<source_pattern_t>("../macro/nlm_{1}.cpp");
#else
#if 1
		pstring dir = "src/lib/netlist/";
		m_setup->parser().register_source<source_pattern_t>(dir + "/macro/nlm_{1}.cpp", true);
		m_setup->parser().register_source<source_pattern_t>(dir + "/generated/nlm_{1}.cpp", true);
		m_setup->parser().register_source<source_pattern_t>(dir + "/macro/modules/nlmod_{1}.cpp", true);
		m_setup->parser().include("base_lib");
#else
		// FIXME: This is very slow - need optimized parsing scanning
		pstring dir = "src/lib/netlist/macro/";
		//m_setup->parser().register_source<source_pattern_t>("src/lib/netlist/macro/nlm_{}.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_base_lib.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_opamp_lib.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_roms_lib.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_cd4xxx_lib.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_otheric_lib.cpp");
		m_setup->parser().register_source<source_file_t>(dir + "nlm_ttl74xx_lib.cpp");
		m_setup->parser().include("base_lib");
#endif
#endif
	}

	void netlist_state_t::set_static_solver_lib(std::unique_ptr<plib::dynlib_base> &&lib)
	{
		m_lib = std::move(lib);
	}


	void netlist_t::stop()
	{
		log().debug("Printing statistics ...\n");
		print_stats();
		log().debug("Stopping solver device ...\n");
		if (m_solver != nullptr)
			m_solver->stop();
	}

	std::size_t netlist_state_t::find_net_id(const detail::net_t *net) const
	{
		for (std::size_t i = 0; i < m_nets.size(); i++)
			if (m_nets[i].get() == net)
				return i;
		return std::numeric_limits<std::size_t>::max();
	}
	detail::net_t *netlist_state_t::net_by_id(std::size_t id) const
	{
		return m_nets[id].get();
	}

	void netlist_state_t::rebuild_lists()
	{
		for (auto & net : m_nets)
			net->rebuild_list();
	}

	void netlist_state_t::compile_defines(std::vector<std::pair<pstring, pstring>> &defs)
	{
	#define ENTRY(x) if (pstring(#x) != PSTRINGIFY(x)) \
			defs.emplace_back(std::pair<pstring, pstring>(#x, PSTRINGIFY(x))); \
		else defs.emplace_back(std::pair<pstring, pstring>(#x, "<NOT DEFINED>"));
	#define ENTRY_EX(x) defs.emplace_back(std::pair<pstring, pstring>(#x, plib::pfmt("{}")(x)));
		ENTRY(NL_VERSION_MAJOR)
		ENTRY(NL_VERSION_MINOR)
		ENTRY(NL_VERSION_PATCHLEVEL)

		ENTRY(PUSE_ACCURATE_STATS)
		ENTRY(PHAS_INT128)
		ENTRY(PUSE_ALIGNED_OPTIMIZATIONS)
		ENTRY(PHAS_OPENMP)
		ENTRY(PUSE_OPENMP)
		ENTRY(PUSE_FLOAT128)
		ENTRY_EX(config::use_mempool::value)
		ENTRY_EX(config::use_queue_stats::value)
		ENTRY(NL_USE_COPY_INSTEAD_OF_REFERENCE)
		ENTRY(NL_USE_FLOAT128)
		ENTRY_EX(config::use_float_matrix::value)
		ENTRY_EX(config::use_long_double_matrix::value)
		ENTRY(NL_DEBUG)
		ENTRY(NVCCBUILD)

		ENTRY(__cplusplus)
		ENTRY(__VERSION__)

		ENTRY(__GNUC__)
		ENTRY(__GNUC_MINOR__)
		ENTRY(__GNUC_PATCHLEVEL__)

		ENTRY(__clang__)
		ENTRY(__clang_major__)
		ENTRY(__clang_minor__)
		ENTRY(__clang_patchlevel__)
		ENTRY(__clang_version__)

		ENTRY(OPENMP )
		ENTRY(_OPENMP )

		ENTRY(__x86_64__ )
		ENTRY(__i386__)
		ENTRY(_WIN32)
		ENTRY(_MSC_VER)
		ENTRY(__APPLE__)
		ENTRY(__unix__)
		ENTRY(__linux__)

		ENTRY_EX(sizeof(base_device_t))
		ENTRY_EX(sizeof(device_t))
		ENTRY_EX(sizeof(logic_t))
		ENTRY_EX(sizeof(logic_input_t))
		ENTRY_EX(sizeof(logic_output_t))
		ENTRY_EX(sizeof(param_model_t))
		ENTRY_EX(sizeof(param_logic_t))
		ENTRY_EX(sizeof(state_var<int>))
		ENTRY_EX(sizeof(pstring))
		ENTRY_EX(sizeof(core_device_t::stats_t))
		ENTRY_EX(sizeof(std::vector<detail::core_terminal_t *>))
		ENTRY_EX(sizeof(plib::plog_level))

		ENTRY_EX(sizeof(nldelegate))
		ENTRY(PPMF_FORCE_TYPE)
		ENTRY(PHAS_PMF_INTERNAL)

	#undef ENTRY
	#undef ENTRY_EX
	}

	pstring netlist_state_t::version()
	{
		return plib::pfmt("{1}.{2}")(NL_VERSION_MAJOR, NL_VERSION_MINOR);
	}

	pstring netlist_state_t::version_patchlevel()
	{
		return plib::pfmt("{1}.{2}.{3}")(NL_VERSION_MAJOR, NL_VERSION_MINOR, NL_VERSION_PATCHLEVEL);
	}

	void netlist_t::reset()
	{
		log().debug("Searching for mainclock\n");
		m_mainclock = m_state.get_single_device<devices::NETLIB_NAME(mainclock)>("mainclock");

		log().debug("Searching for solver\n");
		m_solver = m_state.get_single_device<devices::NETLIB_NAME(solver)>("solver");

		// Don't reset time
		//m_time = netlist_time_ext::zero();
		m_queue.clear();
		if (m_mainclock != nullptr)
			m_mainclock->m_Q.net().set_next_scheduled_time(m_time);
		//if (m_solver != nullptr)
		//  m_solver->reset();

		m_state.reset();
	}

	void netlist_state_t::free_setup_resources()
	{
		m_setup = nullptr;
	}

	void netlist_state_t::reset()
	{
		// Reset all nets once !
		log().verbose("Call reset on all nets:");
		for (auto & n : nets())
			n->reset();

		// Reset all devices once !
		log().verbose("Call reset on all devices:");
		for (auto & dev : m_devices)
			dev.second->reset();

		// Make sure everything depending on parameters is set
		// Currently analog input and logic input also
		// push their outputs to queue.

		std::vector<core_device_t *> devices_called;
		log().verbose("Call update_param on all devices:");
		for (auto & dev : m_devices)
		{
			dev.second->update_param();
			if (!plib::container::contains(devices_called, dev.second.get()))
				devices_called.push_back(dev.second.get());
		}

		// Step all devices once !
		//
		// INFO: The order here affects power up of e.g. breakout. However, such
		// variations are explicitly stated in the breakout manual.

		auto *netlist_params = get_single_device<devices::NETLIB_NAME(netlistparams)>("parameter");

		switch (netlist_params->m_startup_strategy())
		{
			case 0:
			{
				std::vector<const nldelegate *> t;
				log().verbose("Using default startup strategy");
				for (auto &n : m_nets)
				{
					n->update_inputs(); // only used if USE_COPY_INSTEAD_OF_REFERENCE == 1
					for (auto & term : core_terms(*n))
					{
						if (!plib::container::contains(t, &term->delegate()))
						{
							t.push_back(&term->delegate());
							term->run_delegate();
						}
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
						auto *dev = reinterpret_cast<core_device_t *>(term->delegate().object());
						if (!plib::container::contains(devices_called, dev))
							devices_called.push_back(dev);
					}
				}
				log().verbose("Devices not yet updated:");
				for (auto &dev : m_devices)
					if (!plib::container::contains(devices_called, dev.second.get()))
					{
						// FIXME: doesn't seem to be needed, use cases include
						// analog output devices. Check and remove
						log().error("\t Device {1} not yet updated", dev.second->name());
						//dev.second->update();
					}
			}
			break;
		}

		// the above may screw up m_active and the list
		rebuild_lists();

	}

	void netlist_t::print_stats() const
	{
		if (m_use_stats)
		{
			netlist_state_t::stats_info si{m_queue, m_stat_mainloop, m_perf_out_processed};
			m_state.print_stats(si);
		}
		log().verbose("Current pool memory allocated: {1:12} kB", nlstate().pool().cur_alloc() >> 10);
		log().verbose("Maximum pool memory allocated: {1:12} kB", nlstate().pool().max_alloc() >> 10);
	}

	void netlist_state_t::print_stats(stats_info &si) const
	{
		std::vector<size_t> index;
		for (size_t i=0; i < this->m_devices.size(); i++)
			index.push_back(i);

		std::sort(index.begin(), index.end(),
				[&](size_t i1, size_t i2) { return this->m_devices[i1].second->stats()->m_stat_total_time.total() < this->m_devices[i2].second->stats()->m_stat_total_time.total(); });

		plib::pperftime_t<true>::type total_time(0);
		plib::pperftime_t<true>::ctype total_count(0);

		for (auto & j : index)
		{
			auto *entry = this->m_devices[j].second.get();
			auto *stats = entry->stats();
			log().verbose("Device {1:20} : {2:12} {3:12} {4:15} {5:12}", entry->name(),
					stats->m_stat_call_count(), stats->m_stat_total_time.count(),
					stats->m_stat_total_time.total(), stats->m_stat_inc_active());
			total_time += stats->m_stat_total_time.total();
			total_count += stats->m_stat_total_time.count();
		}

		log().verbose("Total calls : {1:12} {2:12} {3:12}", total_count,
			total_time, total_time / gsl::narrow<decltype(total_time)>((total_count > 0) ? total_count : 1));

		log().verbose("Total loop     {1:15}", si.m_stat_mainloop());
		log().verbose("Total time     {1:15}", total_time);

		// FIXME: clang complains about unreachable code without
		const bool clang_workaround_unreachable_code(config::use_queue_stats::value);
		if (clang_workaround_unreachable_code)
		{
			// Only one serialization should be counted in total time
			// But two are contained in m_stat_mainloop
			plib::pperftime_t<true> overhead;
			plib::pperftime_t<true> test;
			{
				auto overhead_guard(overhead.guard());
				for (int j=0; j<100000;j++)
				{
					auto test_guard(test.guard());
				}
			}

			plib::pperftime_t<true>::type total_overhead = overhead()
					* gsl::narrow<plib::pperftime_t<true>::type>(total_count)
					/ gsl::narrow<plib::pperftime_t<true>::type>(200000);

			log().verbose("Queue Pushes   {1:15}", si.m_queue.m_prof_call());
			log().verbose("Queue Moves    {1:15}", si.m_queue.m_prof_sortmove());
			log().verbose("Queue Removes  {1:15}", si.m_queue.m_prof_remove());
			log().verbose("");

			log().verbose("Take the next lines with a grain of salt. They depend on the measurement implementation.");
			log().verbose("Total overhead {1:15}", total_overhead);
			plib::pperftime_t<true>::type overhead_per_pop = (si.m_stat_mainloop()-2*total_overhead - (total_time - total_overhead))
					/ gsl::narrow<plib::pperftime_t<true>::type>(si.m_queue.m_prof_call());
			log().verbose("Overhead per pop  {1:11}", overhead_per_pop );
			log().verbose("");
		}

		auto trigger = total_count * 200 / 1000000; // 200 ppm
		for (const auto &entry : this->m_devices)
		{
			auto *ep = entry.second.get();
			auto *stats = ep->stats();
			// Factor of 3 offers best performace increase
			if (stats->m_stat_inc_active() > 3 * stats->m_stat_total_time.count()
				&& stats->m_stat_inc_active() > trigger)
				log().verbose("HINT({}, NO_DEACTIVATE) // {} {} {}", ep->name(),
					gsl::narrow<nl_fptype>(stats->m_stat_inc_active()) / gsl::narrow<nl_fptype>(stats->m_stat_total_time.count()),
					stats->m_stat_inc_active(), stats->m_stat_total_time.count());
		}
		log().verbose("");
	}

	core_device_t *netlist_state_t::get_single_device(const pstring &classname, bool (*cc)(core_device_t *)) const
	{
		core_device_t *ret = nullptr;
		for (const auto &d : m_devices)
		{
			if (cc(d.second.get()))
			{
				if (ret != nullptr)
				{
					m_log.fatal(MF_MORE_THAN_ONE_1_DEVICE_FOUND(classname));
					throw nl_exception(MF_MORE_THAN_ONE_1_DEVICE_FOUND(classname));
				}
				ret = d.second.get();
			}
		}
		return ret;
	}


	// ----------------------------------------------------------------------------------------
	// core_device_t
	// ----------------------------------------------------------------------------------------

	core_device_t::core_device_t(netlist_state_t &owner, const pstring &name)
		: netlist_object_t(owner.exec(), name)
		, m_hint_deactivate(false)
		, m_active_outputs(*this, "m_active_outputs", 1)
	{
		if (exec().stats_enabled())
			m_stats = owner.make_pool_object<stats_t>();
	}

	core_device_t::core_device_t(core_device_t &owner, const pstring &name)
		: netlist_object_t(owner.state().exec(), owner.name() + "." + name)
		, m_hint_deactivate(false)
		, m_active_outputs(*this, "m_active_outputs", 1)
	{
		//printf("owned device: %s\n", this->name().c_str());
		owner.state().register_device(this->name(), device_arena::owned_ptr<core_device_t>(this, false));
		if (exec().stats_enabled())
			m_stats = owner.state().make_pool_object<stats_t>();
	}

	log_type & core_device_t::log()
	{
		return state().log();
	}

	// ----------------------------------------------------------------------------------------
	// base_device_t
	// ----------------------------------------------------------------------------------------

	base_device_t::base_device_t(netlist_state_t &owner, const pstring &name)
	: core_device_t(owner, name)
	{
	}

	base_device_t::base_device_t(base_device_t &owner, const pstring &name)
	: core_device_t(owner, name)
	{
	}

	void base_device_t::register_subalias(const pstring &name, const detail::core_terminal_t &term)
	{
		pstring alias = this->name() + "." + name;

		// everything already fully qualified
		state().parser().register_alias_nofqn(alias, term.name());
	}

	void base_device_t::register_subalias(const pstring &name, const pstring &aliased)
	{
		pstring alias = this->name() + "." + name;
		pstring aliased_fqn = this->name() + "." + aliased;

		// everything already fully qualified
		state().parser().register_alias_nofqn(alias, aliased_fqn);
	}

	void base_device_t::connect(const detail::core_terminal_t &t1, const detail::core_terminal_t &t2)
	{
		state().parser().register_link_fqn(t1.name(), t2.name());
	}

	void base_device_t::connect(const pstring &t1, const pstring &t2)
	{
		state().parser().register_link_fqn(name() + "." + t1, name() + "." + t2);
	}


	// ----------------------------------------------------------------------------------------
	// device_t
	// ----------------------------------------------------------------------------------------

	device_t::device_t(netlist_state_t &owner, const pstring &name)
	: base_device_t(owner, name)
	, m_model(*this, "MODEL", pstring(config::DEFAULT_LOGIC_FAMILY()))
	{
		set_logic_family(state().setup().family_from_model(m_model()));
		if (logic_family() == nullptr)
			throw nl_exception(MF_NULLPTR_FAMILY(this->name(), m_model()));
	}

	device_t::device_t(netlist_state_t &owner, const pstring &name,
		const pstring &model)
	: base_device_t(owner, name)
	, m_model(*this, "MODEL", model)
	{
		set_logic_family(state().setup().family_from_model(m_model()));
		if (logic_family() == nullptr)
			throw nl_exception(MF_NULLPTR_FAMILY(this->name(), m_model()));
	}

	device_t::device_t(netlist_state_t &owner, const pstring &name,
		const logic_family_desc_t *desc)
	: base_device_t(owner, name)
	, m_model(*this, "MODEL", pstring(""))
	{
		set_logic_family(desc);
		if (logic_family() == nullptr)
			throw nl_exception(MF_NULLPTR_FAMILY(this->name(), "<pointer provided by constructor>"));
	}

	device_t::device_t(device_t &owner, const pstring &name)
	: base_device_t(owner, name)
	, m_model(*this, "MODEL", pstring(""))
	{
		set_logic_family(owner.logic_family());
		if (logic_family() == nullptr)
			throw nl_exception(MF_NULLPTR_FAMILY(this->name(), "<owner logic family>"));
	}

	device_t::device_t(device_t &owner, const pstring &name, const pstring &model)
	: base_device_t(owner, name)
	, m_model(*this, "MODEL", model)
	{
		set_logic_family(state().setup().family_from_model(m_model()));
		if (logic_family() == nullptr)
			throw nl_exception(MF_NULLPTR_FAMILY(this->name(), m_model()));
	}

	// ----------------------------------------------------------------------------------------
	// net_t
	// ----------------------------------------------------------------------------------------

	detail::net_t::net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *railterminal)
		: netlist_object_t(nl.exec(), aname)
		, m_new_Q(*this, "m_new_Q", netlist_sig_t(0))
		, m_cur_Q (*this, "m_cur_Q", netlist_sig_t(0))
		, m_in_queue(*this, "m_in_queue", queue_status::DELIVERED)
		, m_next_scheduled_time(*this, "m_time", netlist_time_ext::zero())
		, m_railterminal(railterminal)
	{
		props::add(this, props::value_type());
	}

	void detail::net_t::rebuild_list()
	{
		// rebuild m_list

		m_list_active.clear();
		for (auto & term : exec().nlstate().core_terms(*this))
			if (term->terminal_state() != logic_t::STATE_INP_PASSIVE)
			{
				m_list_active.push_back(term);
				term->set_copied_input(m_cur_Q);
			}
	}


	void detail::net_t::reset() noexcept
	{
		m_next_scheduled_time = exec().time();
		m_in_queue = queue_status::DELIVERED;

		m_new_Q = 0;
		m_cur_Q = 0;

		auto *p = dynamic_cast<analog_net_t *>(this);

		if (p != nullptr)
			p->set_Q_Analog(nlconst::zero());

		// rebuild m_list and reset terminals to active or analog out state

		m_list_active.clear();
		for (core_terminal_t *ct : exec().nlstate().core_terms(*this))
		{
			ct->reset();
			if (ct->terminal_state() != logic_t::STATE_INP_PASSIVE)
				m_list_active.push_back(ct);
			ct->set_copied_input(m_cur_Q);
		}
	}

	// ----------------------------------------------------------------------------------------
	// logic_net_t
	// ----------------------------------------------------------------------------------------

	logic_net_t::logic_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal)
		: net_t(nl, aname, railterminal)
	{
	}

	// ----------------------------------------------------------------------------------------
	// analog_net_t
	// ----------------------------------------------------------------------------------------

	analog_net_t::analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal)
		: net_t(nl, aname, railterminal)
		, m_cur_Analog(*this, "m_cur_Analog", nlconst::zero())
		, m_solver(nullptr)
	{
	}

	void analog_net_t::reset() noexcept
	{
		net_t::reset();
		m_cur_Analog = nlconst::zero();
	}
	// ----------------------------------------------------------------------------------------
	// core_terminal_t
	// ----------------------------------------------------------------------------------------

	detail::core_terminal_t::core_terminal_t(core_device_t &dev, const pstring &aname,
			const state_e state, nldelegate delegate)
	: device_object_t(&dev, dev.name() + "." + aname)
	#if NL_USE_COPY_INSTEAD_OF_REFERENCE
	, m_Q(*this, "m_Q", 0)
	#endif
	, m_delegate(delegate)
	, m_net(nullptr)
	, m_state(*this, "m_state", state)
	{
	}

	analog_t::analog_t(core_device_t &dev, const pstring &aname, const state_e state,
		nldelegate delegate)
	: core_terminal_t(dev, aname, state, delegate)
	{
	}

	// ----------------------------------------------------------------------------------------
	// terminal_t
	// ----------------------------------------------------------------------------------------

	terminal_t::terminal_t(core_device_t &dev, const pstring &aname,
		terminal_t *otherterm, nldelegate delegate)
	: terminal_t(dev, aname, otherterm, { nullptr, nullptr }, delegate)
	{
	}

	terminal_t::terminal_t(core_device_t &dev, const pstring &aname,
		terminal_t *otherterm, const std::array<terminal_t *, 2> &splitterterms,
		nldelegate delegate)
	: analog_t(dev, aname, STATE_BIDIR, delegate)
	, m_Idr(nullptr)
	, m_go(nullptr)
	, m_gt(nullptr)
	{
		state().setup().register_term(*this, otherterm, splitterterms);
	}

	void terminal_t::set_ptrs(nl_fptype *gt, nl_fptype *go, nl_fptype *Idr) noexcept(false)
	{
		// NOLINTNEXTLINE(readability-implicit-bool-conversion)
		if (!(gt && go && Idr) && (gt || go || Idr))
		{
			throw nl_exception("Inconsistent nullptrs for terminal {}", name());
		}

		m_gt = gt;
		m_go = go;
		m_Idr = Idr;
	}

	// ----------------------------------------------------------------------------------------
	// net_input_t
	// ----------------------------------------------------------------------------------------

	// ----------------------------------------------------------------------------------------
	// net_output_t
	// ----------------------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	logic_t::logic_t(device_t &dev, const pstring &aname, const state_e terminal_state,
			nldelegate delegate)
		: core_terminal_t(dev, aname, terminal_state, delegate)
		, logic_family_t(dev.logic_family())
	{
	}

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

	logic_input_t::logic_input_t(device_t &dev, const pstring &aname,
			nldelegate delegate)
			: logic_t(dev, aname, STATE_INP_ACTIVE, delegate)
	{
		state().setup().register_term(*this);
	}


	// ----------------------------------------------------------------------------------------
	// logic_output_t
	// ----------------------------------------------------------------------------------------

	logic_output_t::logic_output_t(device_t &dev, const pstring &aname, bool dummy)
		: logic_t(dev, aname, STATE_OUT, nldelegate())
		, m_my_net(dev.state(), name() + ".net", this)
	{
		plib::unused_var(dummy);
		this->set_net(&m_my_net);
		state().register_net(device_arena::owned_ptr<logic_net_t>(&m_my_net, false));
		state().setup().register_term(*this);
	}

	void logic_output_t::initial(const netlist_sig_t val) noexcept
	{
		if (has_net())
			net().initial(val);
	}

	// -----------------------------------------------------------------------------
	// tristate_output_t
	// -----------------------------------------------------------------------------
	tristate_output_t::tristate_output_t(device_t &dev, const pstring &aname, bool force_logic)
	: logic_output_t(dev, aname)
	, m_last_logic(dev, name() + "." + "m_last_logic", 1) // force change
	, m_tristate(dev, name() + "." + "m_tristate", force_logic ? 0 : 2) // force change
	, m_force_logic(force_logic)
	{}

	// ----------------------------------------------------------------------------------------
	// analog_input_t
	// ----------------------------------------------------------------------------------------

	analog_input_t::analog_input_t(core_device_t &dev, const pstring &aname,
		nldelegate delegate)
	: analog_t(dev, aname, STATE_INP_ACTIVE, delegate)
	{
		state().setup().register_term(*this);
	}

	// ----------------------------------------------------------------------------------------
	// analog_output_t
	// ----------------------------------------------------------------------------------------

	analog_output_t::analog_output_t(core_device_t &dev, const pstring &aname)
		: analog_t(dev, aname, STATE_OUT, nldelegate())
		, m_my_net(dev.state(), name() + ".net", this)
	{
		state().register_net(device_arena::owned_ptr<analog_net_t>(&m_my_net, false));
		this->set_net(&m_my_net);

		//net().m_cur_Analog = NL_FCONST(0.0);
		state().setup().register_term(*this);
	}

	void analog_output_t::initial(nl_fptype val) noexcept
	{
		net().set_Q_Analog(val);
	}

	// ----------------------------------------------------------------------------------------
	// Parameters ...
	// ----------------------------------------------------------------------------------------

	// deviceless, it's the responsibility of the owner to register!
	param_t::param_t(const pstring &name)
		: device_object_t(nullptr, name)
	{
	}

	param_t::param_t(core_device_t &device, const pstring &name)
		: device_object_t(&device, device.name() + "." + name)
	{
		device.state().setup().register_param_t(*this);
	}

	// placed here to avoid weak vtable warnings
	param_t::~param_t() noexcept = default;

	param_t::param_type_t param_t::param_type() const noexcept(false)
	{
		if (dynamic_cast<const param_str_t *>(this) != nullptr)
			return STRING;
		if (dynamic_cast<const param_fp_t *>(this) != nullptr)
			return DOUBLE;
		if (dynamic_cast<const param_int_t *>(this) != nullptr)
			return INTEGER;
		if (dynamic_cast<const param_logic_t *>(this) != nullptr)
			return LOGIC;
		if (dynamic_cast<const param_ptr_t *>(this) != nullptr)
			return POINTER;

		state().log().fatal(MF_UNKNOWN_PARAM_TYPE(name()));
		throw nl_exception(MF_UNKNOWN_PARAM_TYPE(name()));
	}



	pstring param_t::get_initial(const core_device_t *dev, bool *found) const
	{
		pstring res = dev->state().setup().get_initial_param_val(this->name(), "");
		*found = (!res.empty());
		return res;
	}

	param_str_t::param_str_t(core_device_t &device, const pstring &name, const pstring &val)
	: param_t(device, name)
	{
		m_param = plib::make_unique<pstring, host_arena>(val);
		*m_param = device.state().setup().get_initial_param_val(this->name(),val);
	}

	param_str_t::param_str_t(netlist_state_t &state, const pstring &name, const pstring &val)
	: param_t(name)
	{
		// deviceless parameter, no registration, owner is responsible
		m_param = plib::make_unique<pstring, host_arena>(val);
		*m_param = state.setup().get_initial_param_val(this->name(),val);
	}

	void param_str_t::changed() noexcept
	{
	}

	param_ptr_t::param_ptr_t(core_device_t &device, const pstring &name, uint8_t * val)
	: param_t(device, name)
	{
		m_param = val;
	}

	void param_model_t::changed() noexcept
	{
	}

	pstring param_model_t::type()
	{
		auto mod = state().setup().models().get_model(str());
		return mod.type();
	}

	pstring param_model_t::value_str(const pstring &entity)
	{
		return state().setup().models().get_model(str()).value_str(entity);
	}

	nl_fptype param_model_t::value(const pstring &entity)
	{
		return state().setup().models().get_model(str()).value(entity);
	}


	plib::istream_uptr param_data_t::stream()
	{
		return device().state().parser().get_data_stream(str());
	}

	bool detail::core_terminal_t::is_logic() const noexcept
	{
		return dynamic_cast<const logic_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_logic_input() const noexcept
	{
		return dynamic_cast<const logic_input_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_logic_output() const noexcept
	{
		return dynamic_cast<const logic_output_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_tristate_output() const noexcept
	{
		return dynamic_cast<const tristate_output_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_analog() const noexcept
	{
		return dynamic_cast<const analog_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_analog_input() const noexcept
	{
		return dynamic_cast<const analog_input_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_analog_output() const noexcept
	{
		return dynamic_cast<const analog_output_t *>(this) != nullptr;
	}


	bool detail::net_t::is_logic() const noexcept
	{
		return dynamic_cast<const logic_net_t *>(this) != nullptr;
	}

	bool detail::net_t::is_analog() const noexcept
	{
		return dynamic_cast<const analog_net_t *>(this) != nullptr;
	}

	// ----------------------------------------------------------------------------------------
	// netlist_state_t
	// ----------------------------------------------------------------------------------------

	nlparse_t &netlist_state_t::parser() { return m_setup->parser(); }
	const nlparse_t &netlist_state_t::parser() const { return m_setup->parser(); }

	void netlist_state_t::remove_device(core_device_t *dev)
	{
		for (auto it = m_devices.begin(); it != m_devices.end(); it++)
			if (it->second.get() == dev)
			{
				m_state.remove_save_items(dev);
				m_devices.erase(it);
				return;
			}
	}

	// ----------------------------------------------------------------------------------------
	// netlist_t
	//
	// Hot section
	//
	// Any changes below will impact performance.
	// -----------------------------------------------------------------------------

	template <bool KEEP_STATS>
	void netlist_t::process_queue_stats(const netlist_time_ext delta) noexcept
	{
		netlist_time_ext stop(m_time + delta);

		qpush(stop, nullptr);

		if (m_mainclock == nullptr)
		{
			m_time = m_queue.top().exec_time();
			detail::net_t *obj(m_queue.top().object());
			m_queue.pop();

			while (obj != nullptr)
			{
				obj->template update_devs<KEEP_STATS>();
				if (KEEP_STATS)
					m_perf_out_processed.inc();
				const detail::queue_t::entry_t *top = &m_queue.top();
				m_time = top->exec_time();
				obj = top->object();
				m_queue.pop();
			}
		}
		else
		{
			logic_net_t &mc_net(m_mainclock->m_Q.net());
			const netlist_time inc(m_mainclock->m_inc);
			netlist_time_ext mc_time(mc_net.next_scheduled_time());

			do
			{
				const detail::queue_t::entry_t *top = &m_queue.top();
				while (top->exec_time() > mc_time)
				{
					m_time = mc_time;
					mc_net.toggle_new_Q();
					mc_net.update_devs<KEEP_STATS>();
					top = &m_queue.top();
					mc_time += inc;
				}

				m_time = top->exec_time();
				detail::net_t *const obj(top->object());
				m_queue.pop();

				if (!!(obj == nullptr))
					break;

				obj->template update_devs<KEEP_STATS>();

				if (KEEP_STATS)
					m_perf_out_processed.inc();
			} while (true);

			mc_net.set_next_scheduled_time(mc_time);
		}
	}

	void netlist_t::process_queue(netlist_time_ext delta) noexcept
	{
		if (!m_use_stats)
			process_queue_stats<false>(delta);
		else
		{
			auto sm_guard(m_stat_mainloop.guard());
			process_queue_stats<true>(delta);
		}
	}


	template struct state_var<std::uint8_t>;
	template struct state_var<std::uint16_t>;
	template struct state_var<std::uint32_t>;
	template struct state_var<std::uint64_t>;
	template struct state_var<std::int8_t>;
	template struct state_var<std::int16_t>;
	template struct state_var<std::int32_t>;
	template struct state_var<std::int64_t>;
	template struct state_var<bool>;

	template class param_num_t<std::uint8_t>;
	template class param_num_t<std::uint16_t>;
	template class param_num_t<std::uint32_t>;
	template class param_num_t<std::uint64_t>;
	template class param_num_t<std::int8_t>;
	template class param_num_t<std::int16_t>;
	template class param_num_t<std::int32_t>;
	template class param_num_t<std::int64_t>;
	template class param_num_t<long double>;
	template class param_num_t<double>;
	template class param_num_t<float>;
	template class param_num_t<bool>;

	template class param_model_t::value_base_t<float>;
	template class param_model_t::value_base_t<double>;
	template class param_model_t::value_base_t<long double>;

	template class object_array_t<logic_input_t, 1>;
	template class object_array_t<logic_input_t, 2>;
	template class object_array_t<logic_input_t, 3>;
	template class object_array_t<logic_input_t, 4>;
	template class object_array_t<logic_input_t, 5>;
	template class object_array_t<logic_input_t, 6>;
	template class object_array_t<logic_input_t, 7>;
	template class object_array_t<logic_input_t, 8>;

	template class object_array_t<logic_output_t, 1>;
	template class object_array_t<logic_output_t, 2>;
	template class object_array_t<logic_output_t, 3>;
	template class object_array_t<logic_output_t, 4>;
	template class object_array_t<logic_output_t, 5>;
	template class object_array_t<logic_output_t, 6>;
	template class object_array_t<logic_output_t, 7>;
	template class object_array_t<logic_output_t, 8>;

} // namespace netlist
