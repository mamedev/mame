// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file netlist_state.h
///

#ifndef NL_CORE_NETLIST_STATE_H_
#define NL_CORE_NETLIST_STATE_H_

#include "queue.h"

#include "../nltypes.h"

#include "../plib/plists.h"
#include "../plib/pstate.h"
#include "../plib/pstring.h"

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace netlist
{
	// -----------------------------------------------------------------------------
	// netlist_state__t
	// -----------------------------------------------------------------------------

	class netlist_state_t
	{
	public:

		using nets_collection_type = std::vector<device_arena::owned_ptr<detail::net_t>>;
		using family_collection_type = std::unordered_map<pstring, host_arena::unique_ptr<logic_family_desc_t>>;

		// need to preserve order of device creation ...
		using devices_collection_type = std::vector<std::pair<pstring, device_arena::owned_ptr<core_device_t>>>;

		netlist_state_t(const pstring &name, plib::plog_delegate logger);

		PCOPYASSIGNMOVE(netlist_state_t, delete)

		/// \brief Destructor
		///
		/// The destructor is virtual to allow implementation specific devices
		/// to connect to the outside world. For examples see MAME netlist.cpp.
		///
		virtual ~netlist_state_t() noexcept = default;

		template<class C>
		static bool check_class(core_device_t *p) noexcept
		{
			return dynamic_cast<C *>(p) != nullptr;
		}

		core_device_t *get_single_device(const pstring &classname, bool (*cc)(core_device_t *)) const noexcept(false);

		/// \brief Get single device filtered by class and name
		///
		/// \tparam C Device class for which devices will be returned
		/// \param  name Name of the device
		///
		/// \return pointers to device

		template<class C>
		C *get_single_device(const pstring &name) const
		{
			return dynamic_cast<C *>(get_single_device(name, check_class<C>));
		}

		/// \brief Get vector of devices
		///
		/// \tparam C Device class for which devices will be returned
		///
		/// \return vector with pointers to devices

		template<class C>
		std::vector<C *> get_device_list() const
		{
			std::vector<C *> tmp;
			for (const auto &d : m_devices)
			{
				auto * const dev = dynamic_cast<C *>(d.second.get());
				if (dev != nullptr)
					tmp.push_back(dev);
			}
			return tmp;
		}

		// logging

		log_type & log() noexcept { return m_log; }
		const log_type &log() const noexcept { return m_log; }

		plib::dynlib_base &static_solver_lib() const noexcept { return *m_lib; }

		/// \brief provide library with static solver implementations.
		///
		/// By default no static solvers are provided since these are
		/// determined by the specific use case. You can pass such a collection
		/// of symbols with this method.
		///
		void set_static_solver_lib(std::unique_ptr<plib::dynlib_base> &&lib);

		netlist_t &exec() noexcept { return *m_netlist; }
		const netlist_t &exec() const noexcept { return *m_netlist; }

		// state handling
		plib::state_manager_t &run_state_manager() noexcept { return m_state; }

		template<typename O, typename C>
		void save(O &owner, C &state, const pstring &module, const pstring &stname)
		{
			this->run_state_manager().save_item(plib::void_ptr_cast(&owner), state, module + "." + stname);
		}

		template<typename O, typename C>
		void save(O &owner, C *state, const pstring &module, const pstring &stname, const std::size_t count)
		{
			this->run_state_manager().save_state_ptr(plib::void_ptr_cast(&owner), module + "." + stname, plib::state_manager_t::dtype<C>(), count, state);
		}

		// FIXME: only used by queue_t save state
		std::size_t find_net_id(const detail::net_t *net) const;
		detail::net_t *net_by_id(std::size_t id) const;

		template <typename T>
		void register_net(device_arena::owned_ptr<T> &&net) { m_nets.push_back(std::move(net)); }

		/// \brief Get device pointer by name
		///
		///
		/// \param name Name of the device
		///
		/// \return core_device_t pointer if device exists, else nullptr

		core_device_t *find_device(const pstring &name) const
		{
			for (const auto & d : m_devices)
				if (d.first == name)
					return d.second.get();
			return nullptr;
		}

		/// \brief Register device using owned_ptr
		///
		/// Used to register owned devices. These are devices declared as objects
		/// in another devices.
		///
		/// \param name Name of the device
		/// \param dev Device to be registered

		template <typename T>
		void register_device(const pstring &name, device_arena::owned_ptr<T> &&dev) noexcept(false)
		{
			for (auto & d : m_devices)
				if (d.first == name)
				{
					dev.release();
					log().fatal(MF_DUPLICATE_NAME_DEVICE_LIST(name));
					throw nl_exception(MF_DUPLICATE_NAME_DEVICE_LIST(name));
				}
			//m_devices.push_back(std::move(dev));
			m_devices.insert(m_devices.end(), { name, std::move(dev) });
		}

		/// \brief Register device using unique_ptr
		///
		/// Used to register devices.
		///
		/// \param name Name of the device
		/// \param dev Device to be registered

		template <typename T>
		void register_device(const pstring &name, device_arena::unique_ptr<T> &&dev)
		{
			register_device(name, device_arena::owned_ptr<T>(dev.release(), true, dev.get_deleter()));
		}

		/// \brief Remove device
		///
		/// Care needs to be applied if this is called to remove devices with
		/// sub-devices which may have registered state.
		///
		/// \param dev Device to be removed

		void remove_device(core_device_t *dev);

		setup_t &setup() noexcept { return *m_setup; }
		const setup_t &setup() const noexcept { return *m_setup; }

		nlparse_t &parser();
		const nlparse_t &parser() const;

		// FIXME: make a post load member and include code there
		void rebuild_lists(); // must be called after post_load !

		static void compile_defines(std::vector<std::pair<pstring, pstring>> &defs);
		static pstring version();
		static pstring version_patchlevel();

		nets_collection_type & nets() noexcept { return m_nets; }
		const nets_collection_type & nets() const noexcept { return m_nets; }

		devices_collection_type & devices() noexcept { return m_devices; }
		const devices_collection_type & devices() const noexcept { return m_devices; }

		family_collection_type &family_cache() { return m_family_cache; }

		template<typename T, typename... Args>
		device_arena::unique_ptr<T> make_pool_object(Args&&... args)
		{
			return plib::make_unique<T>(m_pool, std::forward<Args>(args)...);
		}
		// memory pool - still needed in some places
		device_arena &pool() noexcept { return m_pool; }
		const device_arena &pool() const noexcept { return m_pool; }

		struct stats_info
		{
			const detail::queue_t               &m_queue;// performance
			const plib::pperftime_t<true>       &m_stat_mainloop;
			const plib::pperfcount_t<true>      &m_perf_out_processed;
		};

		/// \brief print statistics gathered during run
		///
		void print_stats(stats_info &si) const;

		/// \brief call reset on all netlist components
		///
		void reset();

		/// \brief prior to running free no longer needed resources
		///
		void free_setup_resources();

		std::vector<detail::core_terminal_t *> &core_terms(const detail::net_t &net) noexcept
		{
			return m_core_terms[&net];
		}

	private:

		device_arena                               m_pool; // must be deleted last!

		device_arena::unique_ptr<netlist_t>        m_netlist;
		std::unique_ptr<plib::dynlib_base>         m_lib;
		plib::state_manager_t                      m_state;
		log_type                                   m_log;

		// FIXME: should only be available during device construction
		host_arena::unique_ptr<setup_t>            m_setup;

		nets_collection_type                       m_nets;
		// sole use is to manage lifetime of net objects
		devices_collection_type                    m_devices;
		// sole use is to manage lifetime of family objects
		family_collection_type                     m_family_cache;
		// all terms for a net
		std::unordered_map<const detail::net_t *, std::vector<detail::core_terminal_t *>> m_core_terms;
		// dummy version
		int                                        m_dummy_version;
	};

} // namespace netlist


#endif // NL_CORE_NETLIST_STATE_H_
