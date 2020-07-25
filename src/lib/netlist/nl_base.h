// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLBASE_H_
#define NLBASE_H_

///
/// \file nl_base.h
///

#ifdef NL_PROHIBIT_BASEH_INCLUDE
#error "nl_base.h included. Please correct."
#endif

#include "core/base_objects.h"
#include "core/param.h"
#include "core/state_var.h"
#include "core/logic_family.h"
#include "core/nets.h"

#include "plib/palloc.h" // owned_ptr
#include "plib/pfunction.h"
#include "plib/plists.h"
#include "plib/pmempool.h"
#include "plib/ppmf.h"
#include "plib/pstate.h"
#include "plib/pstream.h"
#include "plib/ptimed_queue.h"
#include "plib/ptypes.h"

#include "nl_errstr.h"
#include "nl_factory.h"
#include "nltypes.h"

#include <initializer_list>
#include <unordered_map>
#include <vector>

//============================================================
//  MACROS / New Syntax
//============================================================

/// \brief Construct a netlist device name
///
#define NETLIB_NAME(chip) nld_ ## chip

/// \brief Start a netlist device class.
///
/// Used to start defining a netlist device class.
/// The simplest device without inputs or outputs would look like this:
///
///      NETLIB_OBJECT(some_object)
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
///  Also refer to #NETLIB_CONSTRUCTOR.
#define NETLIB_OBJECT(name)                                                    \
class NETLIB_NAME(name) : public delegator_t<device_t>

/// \brief Start a derived netlist device class.
///
/// Used to define a derived device class based on plcass.
/// The simplest device without inputs or outputs would look like this:
///
///      NETLIB_OBJECT_DERIVED(some_object, parent_object)
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
///  Also refer to #NETLIB_CONSTRUCTOR.
#define NETLIB_OBJECT_DERIVED(name, pclass)                                   \
class NETLIB_NAME(name) : public delegator_t<NETLIB_NAME(pclass)>



// Only used for analog objects like diodes and resistors

#define NETLIB_BASE_OBJECT(name)                                               \
class NETLIB_NAME(name) : public delegator_t<base_device_t>

#define NETLIB_CONSTRUCTOR_PASS(cname, ...)                                    \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
	: base_type(owner, name, __VA_ARGS__)

/// \brief Used to define the constructor of a netlist device.
///
///  Use this to define the constructor of a netlist device. Please refer to
///  #NETLIB_OBJECT for an example.
#define NETLIB_CONSTRUCTOR(cname)                                              \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name)\
		: base_type(owner, name)

/// \brief Used to define the constructor of a netlist device and define a default model.
///
///
///      NETLIB_CONSTRUCTOR_MODEL(some_object, "TTL")
///      {
///      public:
///          NETLIB_CONSTRUCTOR(some_object) { }
///      };
///
#define NETLIB_CONSTRUCTOR_MODEL(cname, cmodel)                                              \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
		: base_type(owner, name, cmodel)

/// \brief Define an extended constructor and add further parameters to it.
/// The macro allows to add further parameters to a device constructor. This is
/// normally used for sub-devices and system devices only.
#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                      \
	using this_type = NETLIB_NAME(cname);                                      \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name, __VA_ARGS__) \
		: base_type(owner, name)

/// \brief Used to define the destructor of a netlist device.
/// The use of a destructor for netlist device should normally not be necessary.
#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)() noexcept override

/// \brief Add this to a device definition to mark the device as dynamic.
///
///  If NETLIB_IS_DYNAMIC(true) is added to the device definition the device
///  is treated as an analog dynamic device, i.e. \ref NETLIB_UPDATE_TERMINALSI
///  is called on a each step of the Newton-Raphson step
///  of solving the linear equations.
///
///  You may also use e.g. NETLIB_IS_DYNAMIC(m_func() != "") to only make the
///  device a dynamic device if parameter m_func is set.
///
///  \param expr boolean expression
///
#define NETLIB_IS_DYNAMIC(expr)                                                \
	public: virtual bool is_dynamic() const noexcept override { return expr; }

/// \brief Add this to a device definition to mark the device as a time-stepping device.
///
///  You have to implement NETLIB_TIMESTEP in this case as well. Currently, only
///  the capacitor and inductor devices uses this.
///
///  You may also use e.g. NETLIB_IS_TIMESTEP(m_func() != "") to only make the
///  device a dynamic device if parameter m_func is set. This is used by the
///  Voltage Source element.
///
///  Example:
///
///  \code
///  NETLIB_TIMESTEP_IS_TIMESTEP()
///  NETLIB_TIMESTEPI()
///  {
///      // Gpar should support convergence
///      const nl_fptype G = m_C.Value() / step +  m_GParallel;
///      const nl_fptype I = -G/// deltaV();
///      set(G, 0.0, I);
///  }
///  \endcode

#define NETLIB_IS_TIMESTEP(expr)                                               \
	public: virtual bool is_timestep() const  noexcept override { return expr; }

/// \brief Used to implement the time stepping code.
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.

#define NETLIB_TIMESTEPI()                                                     \
	public: virtual void timestep(timestep_type ts_type, nl_fptype step)  noexcept override

/// \brief Used to implement the body of the time stepping code.
///
/// Used when the implementation is outside the class definition
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.
///
/// \param cname Name of object as given to \ref NETLIB_OBJECT
///
#define NETLIB_TIMESTEP(cname)                                                 \
	void NETLIB_NAME(cname) :: timestep(timestep_type ts_type, nl_fptype step) noexcept

#define NETLIB_DELEGATE(name) nldelegate(&this_type :: name, this)

#define NETLIB_UPDATE_TERMINALSI() virtual void update_terminals() noexcept override
#define NETLIB_HANDLERI(name) void name() noexcept
#define NETLIB_UPDATE_PARAMI() virtual void update_param() noexcept override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUB_UPTR(ns, chip) device_arena::unique_ptr< ns :: nld_ ## chip >

#define NETLIB_HANDLER(chip, name) void NETLIB_NAME(chip) :: name() noexcept

#if 0
#define NETLIB_UPDATEI() virtual void update() noexcept override
#define NETLIB_UPDATE(chip) NETLIB_HANDLER(chip, update)
#endif

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param() noexcept

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals() noexcept

//============================================================
// Namespace starts
//============================================================

namespace netlist
{



	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public detail::core_terminal_t
	{
	public:

		analog_t(core_device_t &dev, const pstring &aname, state_e state,
			nldelegate delegate);

		const analog_net_t & net() const noexcept;
		analog_net_t & net() noexcept;

		solver::matrix_solver_t *solver() const noexcept;
	};

	/// \brief Base clase for terminals.
	///
	/// Each \ref nld_twoterm object consists of two terminals. Terminals
	/// are at the core of analog netlists and provide. \ref net_t objects
	/// connect terminals.
	///
	class terminal_t : public analog_t
	{
	public:

		/// \brief constructor
		///
		/// @param dev core_devict_t object owning the terminal
		/// @param aname name of this terminal
		/// @param otherterm pointer to the sibling terminal
		terminal_t(core_device_t &dev, const pstring &aname, terminal_t *otherterm, nldelegate delegate);

		/// \brief Returns voltage of connected net
		///
		/// @return voltage of net this terminal is connected to
		nl_fptype operator ()() const  noexcept;

		/// @brief sets conductivity value of this terminal
		///
		/// @param G Conductivity
		void set_conductivity(nl_fptype G) const noexcept
		{
			set_go_gt_I(-G, G, nlconst::zero());
		}

		void set_go_gt(nl_fptype GO, nl_fptype GT) const noexcept
		{
			set_go_gt_I(GO, GT, nlconst::zero());
		}

		void set_go_gt_I(nl_fptype GO, nl_fptype GT, nl_fptype I) const noexcept
		{
			// Check for rail nets ...
			if (m_go != nullptr)
			{
				*m_Idr = I;
				*m_go = GO;
				*m_gt = GT;
			}
		}

		void set_ptrs(nl_fptype *gt, nl_fptype *go, nl_fptype *Idr) noexcept(false);

	private:
		nl_fptype *m_Idr; ///< drive current
		nl_fptype *m_go;  ///< conductance for Voltage from other term
		nl_fptype *m_gt;  ///< conductance for total conductance

	};


	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	class logic_t : public detail::core_terminal_t, public logic_family_t
	{
	public:
		logic_t(device_t &dev, const pstring &aname,
				state_e terminal_state, nldelegate delegate);

		logic_net_t & net() noexcept;
		const logic_net_t &  net() const noexcept;
	};

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

	class logic_input_t : public logic_t
	{
	public:
		logic_input_t(device_t &dev, const pstring &aname,
				nldelegate delegate);

		inline netlist_sig_t operator()() const noexcept;

		void inactivate() noexcept;
		void activate() noexcept;
		void activate_hl() noexcept;
		void activate_lh() noexcept;
	};

	// -----------------------------------------------------------------------------
	// analog_input_t
	// -----------------------------------------------------------------------------

	/// \brief terminal providing analog input voltage.
	///
	/// This terminal class provides a voltage measurement. The conductance against
	/// ground is infinite.

	class analog_input_t : public analog_t
	{
	public:
		/// \brief Constructor
		analog_input_t(core_device_t &dev,  ///< owning device
				const pstring &aname,       ///< name of terminal
				nldelegate delegate ///< delegate
		);

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype operator()() const noexcept { return Q_Analog(); }

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype Q_Analog() const noexcept;
	};

	// -----------------------------------------------------------------------------
	// logic_output_t
	// -----------------------------------------------------------------------------

	class logic_output_t : public logic_t
	{
	public:

		/// \brief logic output constructor
		///
		/// The third parameter does nothing. It is provided only for
		/// compatibility with tristate_output_t in templatized device models
		///
		/// \param dev Device owning this output
		/// \param aname The name of this output
		/// \param dummy Dummy parameter to allow construction like tristate output
		///
		logic_output_t(device_t &dev, const pstring &aname, bool dummy = false);

		void initial(netlist_sig_t val) noexcept;

		inline void push(const netlist_sig_t &newQ, const netlist_time &delay) noexcept
		{
			m_my_net.set_Q_and_push(newQ, delay); // take the shortcut
		}

		inline void set_Q_time(const netlist_sig_t &newQ, const netlist_time_ext &at) noexcept
		{
			m_my_net.set_Q_time(newQ, at); // take the shortcut
		}

		/// \brief Dummy implementation for templatized generic devices
		///
		/// This function shall never be called. It is defined here so that
		/// templatized generic device models do not have to do tons of
		/// template magic.
		///
		/// This function terminates if actually called.
		///
		[[noreturn]] static void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off)
		{
			plib::unused_var(v, ts_off_on, ts_on_off);
			plib::terminate("set_tristate on logic_output should never be called!");
		}
	private:
		logic_net_t m_my_net;
	};

	// -----------------------------------------------------------------------------
	// tristate_output_t
	// -----------------------------------------------------------------------------

	/// \brief Tristate output
	///
	/// In a lot of applications tristate enable inputs are just connected to
	/// VCC/GND to permanently enable the outputs. In this case a pure
	/// implementation using analog outputs would not perform well.
	///
	/// For this object during creation it can be decided if a logic output or
	/// a tristate output is used. Generally the owning device uses parameter
	/// FORCE_TRISTATE_LOGIC to determine this.
	///
	/// This is the preferred way to implement tristate outputs.
	///

	class tristate_output_t : public logic_output_t
	{
	public:

		tristate_output_t(device_t &dev, const pstring &aname, bool force_logic)
		: logic_output_t(dev, aname)
		, m_last_logic(dev, name() + "." + "m_last_logic", 1) // force change
		, m_tristate(dev, name() + "." + "m_tristate", force_logic ? 0 : 2) // force change
		, m_force_logic(force_logic)
		{}

		void push(netlist_sig_t newQ, netlist_time delay) noexcept
		{
			if (!m_tristate)
				logic_output_t::push(newQ, delay);
			m_last_logic = newQ;
		}

		void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off) noexcept
		{
			if (!m_force_logic)
				if (v != m_tristate)
				{
					logic_output_t::push((v != 0) ? OUT_TRISTATE() : m_last_logic, v ? ts_off_on : ts_on_off);
					m_tristate = v;
				}
		}

		bool is_force_logic() const noexcept
		{
			return m_force_logic;
		}

	private:
		using logic_output_t::initial;
		using logic_output_t::set_Q_time;
		state_var<netlist_sig_t> m_last_logic;
		state_var<netlist_sig_t> m_tristate;
		bool m_force_logic;
	};


	// -----------------------------------------------------------------------------
	// analog_output_t
	// -----------------------------------------------------------------------------

	class analog_output_t : public analog_t
	{
	public:
		analog_output_t(core_device_t &dev, const pstring &aname);

		void push(nl_fptype val) noexcept;
		void initial(nl_fptype val) noexcept;

	private:
		analog_net_t m_my_net;
	};


	// -----------------------------------------------------------------------------
	// base_device_t
	// -----------------------------------------------------------------------------

	class base_device_t :   public core_device_t
	{
	public:
		base_device_t(netlist_state_t &owner, const pstring &name);
		base_device_t(base_device_t &owner, const pstring &name);

		PCOPYASSIGNMOVE(base_device_t, delete)

		~base_device_t() noexcept override = default;

		template<class O, class C, typename... Args>
		void create_and_register_subdevice(O& owner, const pstring &name, device_arena::unique_ptr<C> &dev, Args&&... args);

		void register_subalias(const pstring &name, const detail::core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect(const pstring &t1, const pstring &t2);
		void connect(const detail::core_terminal_t &t1, const detail::core_terminal_t &t2);
	protected:

		//NETLIB_UPDATE_TERMINALSI() { }

	private:
	};

	// -----------------------------------------------------------------------------
	// device_t
	// -----------------------------------------------------------------------------

	class device_t :    public base_device_t,
						public logic_family_t
	{
	public:
		device_t(netlist_state_t &owner, const pstring &name);
		device_t(netlist_state_t &owner, const pstring &name,
			const pstring &model);
		// only needed by proxies
		device_t(netlist_state_t &owner, const pstring &name,
			const logic_family_desc_t *desc);

		device_t(device_t &owner, const pstring &name);
		// pass in a default model - this may be overwritten by PARAM(DEVICE.MODEL, "XYZ(...)")
		device_t(device_t &owner, const pstring &name,
			const pstring &model);

		PCOPYASSIGNMOVE(device_t, delete)

		~device_t() noexcept override = default;

	protected:

		//NETLIB_UPDATE_TERMINALSI() { }

	private:
		param_model_t m_model;
	};

	namespace detail {
		// Use timed_queue_heap to use stdc++ heap functions instead of linear processing.
		// This slows down processing by about 25% on a Kaby Lake.
		// template <class T, bool TS>
		// using timed_queue = plib::timed_queue_heap<T, TS>;

		template <class T, bool TS>
		using timed_queue = plib::timed_queue_linear<T, TS>;

		// -----------------------------------------------------------------------------
		// queue_t
		// -----------------------------------------------------------------------------

		// We don't need a thread-safe queue currently. Parallel processing of
		// solvers will update inputs after parallel processing.

		template <typename O, bool TS>
		class queue_base :
				public timed_queue<plib::pqentry_t<netlist_time_ext, O *>, false>,
				public plib::state_manager_t::callback_t
		{
		public:
			using entry_t = plib::pqentry_t<netlist_time_ext, O *>;
			using base_queue = timed_queue<entry_t, false>;
			using id_delegate = plib::pmfp<std::size_t, const O *>;
			using obj_delegate = plib::pmfp<O *, std::size_t>;

			explicit queue_base(std::size_t size, id_delegate get_id, obj_delegate get_obj)
			: timed_queue<plib::pqentry_t<netlist_time_ext, O *>, false>(size)
			, m_qsize(0)
			, m_times(size)
			, m_net_ids(size)
			, m_get_id(get_id)
			, m_obj_by_id(get_obj)
			{
			}

			~queue_base() noexcept override = default;

			queue_base(const queue_base &) = delete;
			queue_base(queue_base &&) = delete;
			queue_base &operator=(const queue_base &) = delete;
			queue_base &operator=(queue_base &&) = delete;

		protected:

			void register_state(plib::state_manager_t &manager, const pstring &module) override
			{
				manager.save_item(this, m_qsize, module + "." + "qsize");
				manager.save_item(this, &m_times[0], module + "." + "times", m_times.size());
				manager.save_item(this, &m_net_ids[0], module + "." + "names", m_net_ids.size());
			}
			void on_pre_save(plib::state_manager_t &manager) override
			{
				plib::unused_var(manager);
				m_qsize = this->size();
				for (std::size_t i = 0; i < m_qsize; i++ )
				{
					m_times[i] =  this->listptr()[i].exec_time().as_raw();
					m_net_ids[i] = m_get_id(this->listptr()[i].object());
				}
			}
			void on_post_load(plib::state_manager_t &manager) override
			{
				plib::unused_var(manager);
				this->clear();
				for (std::size_t i = 0; i < m_qsize; i++ )
				{
					O *n = m_obj_by_id(m_net_ids[i]);
					this->template push<false>(entry_t(netlist_time_ext::from_raw(m_times[i]),n));
				}
			}

		private:
			std::size_t m_qsize;
			std::vector<netlist_time_ext::internal_type> m_times;
			std::vector<std::size_t> m_net_ids;
			id_delegate m_get_id;
			obj_delegate m_obj_by_id;
		};

		using queue_t = queue_base<net_t, false>;

	} // namespace detail

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
		netlist_state_t(const pstring &name, host_arena::unique_ptr<callbacks_t> &&callbacks);

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
		inline std::vector<C *> get_device_list() const
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

		plib::dynlib_base &lib() const noexcept { return *m_lib; }

		netlist_t &exec() noexcept { return *m_netlist; }
		const netlist_t &exec() const noexcept { return *m_netlist; }

		// state handling
		plib::state_manager_t &run_state_manager() noexcept { return m_state; }

		template<typename O, typename C>
		void save(O &owner, C &state, const pstring &module, const pstring &stname)
		{
			this->run_state_manager().save_item(static_cast<void *>(&owner), state, module + "." + stname);
		}

		template<typename O, typename C>
		void save(O &owner, C *state, const pstring &module, const pstring &stname, const std::size_t count)
		{
			this->run_state_manager().save_state_ptr(static_cast<void *>(&owner), module + "." + stname, plib::state_manager_t::dtype<C>(), count, state);
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

		void remove_device(core_device_t *dev)
		{
			for (auto it = m_devices.begin(); it != m_devices.end(); it++)
				if (it->second.get() == dev)
				{
					m_state.remove_save_items(dev);
					m_devices.erase(it);
					return;
				}
		}

		setup_t &setup() noexcept { return *m_setup; }
		const setup_t &setup() const noexcept { return *m_setup; }

		nlparse_t &parser();
		const nlparse_t &parser() const;

		// FIXME: make a postload member and include code there
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

		/// \brief set extended validation mode.
		///
		/// The extended validation mode is not intended for running.
		/// The intention is to identify power pins which are not properly
		/// connected. The downside is that this mode creates a netlist which
		/// is different (and not able to run).
		///
		/// Extended validation is supported by nltool validate option.
		///
		/// \param val Boolean value enabling/disabling extended validation mode
		void set_extended_validation(bool val) { m_extended_validation = val; }

		/// \brief State of extended validation mode.
		///
		/// \returns boolean value indicating if extended validation mode is
		/// turned on.
		bool is_extended_validation() const { return m_extended_validation; }

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

	private:

		device_arena                               m_pool; // must be deleted last!

		device_arena::unique_ptr<netlist_t>        m_netlist;
		std::unique_ptr<plib::dynlib_base>         m_lib;
		plib::state_manager_t                      m_state;
		host_arena::unique_ptr<callbacks_t>        m_callbacks;
		log_type                                   m_log;

		// FIXME: should only be available during device construcion
		host_arena::unique_ptr<setup_t>            m_setup;

		nets_collection_type                       m_nets;
		// sole use is to manage lifetime of net objects
		devices_collection_type                    m_devices;
		// sole use is to manage lifetime of family objects
		family_collection_type                     m_family_cache;
		bool                                       m_extended_validation;

		// dummy version
		int                                        m_dummy_version;
	};

	namespace devices
	{
		// -----------------------------------------------------------------------------
		// mainclock
		// -----------------------------------------------------------------------------

		NETLIB_OBJECT(mainclock)
		{
			NETLIB_CONSTRUCTOR(mainclock)
			, m_Q(*this, "Q")
			, m_freq(*this, "FREQ", nlconst::magic(7159000.0 * 5))
			{
				m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			}

			NETLIB_RESETI();

			NETLIB_UPDATE_PARAMI()
			{
				m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			}

		public:
			logic_output_t m_Q; // NOLINT: needed in core
			netlist_time m_inc; // NOLINT: needed in core
		private:
			param_fp_t m_freq;
		};
	} // namespace devices

	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------

	class netlist_t // NOLINT(clang-analyzer-optin.performance.Padding)
	{
	public:

		explicit netlist_t(netlist_state_t &state, const pstring &aname);

		PCOPYASSIGNMOVE(netlist_t, delete)

		virtual ~netlist_t() noexcept = default;

		// run functions

		netlist_time_ext time() const noexcept { return m_time; }

		void process_queue(netlist_time_ext delta) noexcept;
		void abort_current_queue_slice() noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.retime<false>(detail::queue_t::entry_t(m_time, nullptr));
			else
				m_queue.retime<true>(detail::queue_t::entry_t(m_time, nullptr));
		}

		const detail::queue_t &queue() const noexcept { return m_queue; }

		template<typename... Args>
		void qpush(Args&&...args) noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.emplace<false>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
			else
				m_queue.emplace<true>(std::forward<Args>(args)...); // NOLINT(performance-move-const-arg)
		}

		template <class R>
		void qremove(const R &elem) noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.remove<false>(elem);
			else
				m_queue.remove<true>(elem);
		}

		// Control functions

		void stop();
		void reset();

		// only used by nltool to create static c-code
		devices::NETLIB_NAME(solver) *solver() const noexcept { return m_solver; }

		// force late type resolution
		template <typename X = devices::NETLIB_NAME(solver)>
		nl_fptype gmin(X *solv = nullptr) const noexcept
		{
			plib::unused_var(solv);
			return static_cast<X *>(m_solver)->gmin();
		}

		netlist_state_t &nlstate() noexcept { return m_state; }
		const netlist_state_t &nlstate() const noexcept { return m_state; }

		log_type & log() noexcept { return m_state.log(); }
		const log_type &log() const noexcept { return m_state.log(); }

		void print_stats() const;
		bool use_stats() const { return m_use_stats; }

		bool stats_enabled() const noexcept { return m_use_stats; }
		void enable_stats(bool val) noexcept { m_use_stats = val; }

	private:

		template <bool KEEP_STATS>
		void process_queue_stats(netlist_time_ext delta) noexcept;

		netlist_state_t &                   m_state;
		devices::NETLIB_NAME(solver) *      m_solver;

		// mostly rw
		//PALIGNAS(16)
		netlist_time_ext                    m_time;
		devices::NETLIB_NAME(mainclock) *   m_mainclock;

		//PALIGNAS_CACHELINE()
		//PALIGNAS(16)
		detail::queue_t                     m_queue;
		bool                                m_use_stats;
		// performance
		plib::pperftime_t<true>             m_stat_mainloop;
		plib::pperfcount_t<true>            m_perf_out_processed;
	};

	// -----------------------------------------------------------------------------
	// Support classes for devices
	// -----------------------------------------------------------------------------

	template<class C, std::size_t N>
	class object_array_base_t : public plib::static_vector<C, N>
	{
	public:
		template<class D, typename... Args>
		//object_array_base_t(D &dev, const std::initializer_list<const char *> &names, Args&&... args)
		object_array_base_t(D &dev, std::array<const char *, N> &&names, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, pstring(names[i]), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, const pstring &fmt)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i));
		}

		template<class D, typename... Args>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i+offset), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, nldelegate delegate)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace_back(dev, formatted(fmt, i+offset), delegate);
		}

		template<class D>
		object_array_base_t(D &dev, std::size_t offset, std::size_t qmask, const pstring &fmt)
		{
			for (std::size_t i = 0; i<N; i++)
			{
				pstring name(formatted(fmt, i+offset));
				if ((qmask >> i) & 1)
					name += "Q";
				this->emplace(i, dev, name);
			}
		}
	protected:
		object_array_base_t() = default;

		static pstring formatted(const pstring &fmt, std::size_t n)
		{
			if (N != 1)
				return plib::pfmt(fmt)(n);
			return plib::pfmt(fmt)("");
		}
	};


	template<class C, std::size_t N>
	class object_array_t : public object_array_base_t<C, N>
	{
	public:
		using base_type = object_array_base_t<C, N>;
		using base_type::base_type;
	};

	template<std::size_t N>
	class object_array_t<logic_input_t,N> : public object_array_base_t<logic_input_t, N>
	{
	public:
		using base_type = object_array_base_t<logic_input_t, N>;
		using base_type::base_type;

		template<class D, std::size_t ND>
		object_array_t(D &dev, std::size_t offset, std::size_t qmask,
			const pstring &fmt, std::array<nldelegate, ND> &&delegates)
		{
			static_assert(N <= ND, "initializer_list size mismatch");
			std::size_t i = 0;
			for (auto &e : delegates)
			{
				if (i < N)
				{
					pstring name(this->formatted(fmt, i+offset));
					if ((qmask >> i) & 1)
						name += "Q";
					this->emplace_back(dev, name, e);
				}
				i++;
			}
		}

		//using value_type = typename plib::fast_type_for_bits<N>::type;
		using value_type = std::uint32_t;
		value_type operator ()()
		{
			if (N == 1) return e<0>() ;
			if (N == 2) return e<0>() | (e<1>() << 1);
			if (N == 3) return e<0>() | (e<1>() << 1) | (e<2>() << 2);
			if (N == 4) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3);
			if (N == 5) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4);
			if (N == 6) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5);
			if (N == 7) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5) | (e<6>() << 6);
			if (N == 8) return e<0>() | (e<1>() << 1) | (e<2>() << 2) | (e<3>() << 3)
				| (e<4>() << 4) | (e<5>() << 5) | (e<6>() << 6) | (e<7>() << 7);

			value_type r(0);
			for (std::size_t i = 0; i < N; i++)
				r = static_cast<value_type>((*this)[i]() << (N-1)) | (r >> 1);
			return r;
		}

	private:
		template <std::size_t P>
		inline constexpr value_type e() const { return (*this)[P](); }
	};

	template<std::size_t N>
	class object_array_t<logic_output_t,N> : public object_array_base_t<logic_output_t, N>
	{
	public:
		using base_type = object_array_base_t<logic_output_t, N>;
		using base_type::base_type;

		template <typename T>
		inline void push(const T &v, const netlist_time &t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t);
		}

		template<typename T>
		void push(const T &v, const netlist_time * t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t[0]);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t[1]);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t[2]);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t[3]);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t[4]);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t[5]);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t[6]);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t[7]);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t[i]);
		}

		template<typename T, std::size_t NT>
		void push(const T &v, const std::array<netlist_time, NT> &t)
		{
			static_assert(NT >= N, "Not enough timing entries provided");

			push(v, t.data());
		}

		void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off) noexcept
		{
			for (std::size_t i = 0; i < N; i++)
				(*this)[i].set_tristate(v, ts_off_on, ts_on_off);
		}
	};

	template<std::size_t N>
	class object_array_t<tristate_output_t,N> : public object_array_base_t<tristate_output_t, N>
	{
	public:
		using base_type = object_array_base_t<tristate_output_t, N>;
		using base_type::base_type;

		template <typename T>
		inline void push(const T &v, const netlist_time &t)
		{
			if (N >= 1) (*this)[0].push((v >> 0) & 1, t);
			if (N >= 2) (*this)[1].push((v >> 1) & 1, t);
			if (N >= 3) (*this)[2].push((v >> 2) & 1, t);
			if (N >= 4) (*this)[3].push((v >> 3) & 1, t);
			if (N >= 5) (*this)[4].push((v >> 4) & 1, t);
			if (N >= 6) (*this)[5].push((v >> 5) & 1, t);
			if (N >= 7) (*this)[6].push((v >> 6) & 1, t);
			if (N >= 8) (*this)[7].push((v >> 7) & 1, t);
			for (std::size_t i = 8; i < N; i++)
				(*this)[i].push((v >> i) & 1, t);
		}

		void set_tristate(netlist_sig_t v,
			netlist_time ts_off_on, netlist_time ts_on_off) noexcept
		{
			for (std::size_t i = 0; i < N; i++)
				(*this)[i].set_tristate(v, ts_off_on, ts_on_off);
		}
	};

	// -----------------------------------------------------------------------------
	// power pins - not a device, but a helper
	// -----------------------------------------------------------------------------

	/// \brief Power pins class.
	///
	/// Power Pins are passive inputs. Delegate noop will silently ignore any
	/// updates.

	class nld_power_pins
	{
	public:
		using this_type = nld_power_pins;

		explicit nld_power_pins(device_t &owner)
		: m_VCC(owner, owner.logic_family()->vcc_pin(), NETLIB_DELEGATE(noop))
		, m_GND(owner, owner.logic_family()->gnd_pin(), NETLIB_DELEGATE(noop))
		{
		}

		const analog_input_t &VCC() const noexcept
		{
			return m_VCC;
		}
		const analog_input_t &GND() const noexcept
		{
			return m_GND;
		}

	private:
		void noop() { }
		analog_input_t m_VCC;
		analog_input_t m_GND;
	};

	namespace devices
	{
		inline NETLIB_RESET(mainclock)
		{
			m_Q.net().set_next_scheduled_time(exec().time());
		}
	} // namespace devices

	// -----------------------------------------------------------------------------
	// Hot section
	//
	// Any changes below will impact performance.
	// -----------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

	inline void logic_input_t::inactivate() noexcept
	{
		if (!is_state(STATE_INP_PASSIVE))
		{
			set_state(STATE_INP_PASSIVE);
			net().remove_from_active_list(*this);
		}
	}

	inline void logic_input_t::activate() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_ACTIVE);
		}
	}

	inline void logic_input_t::activate_hl() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_HL);
		}
	}

	inline void logic_input_t::activate_lh() noexcept
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_LH);
		}
	}

	inline void detail::net_t::push_to_queue(const netlist_time &delay) noexcept
	{
		if (has_connections())
		{
			if (!!is_queued())
				exec().qremove(this);

			const auto nst(exec().time() + delay);
			m_next_scheduled_time = nst;

			if (!m_list_active.empty())
			{
				m_in_queue = queue_status::QUEUED;
				exec().qpush(nst, this);
			}
			else
			{
				m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
				update_inputs();
			}
		}
	}

	inline void detail::net_t::add_to_active_list(core_terminal_t &term) noexcept
	{
		if (!m_list_active.empty())
		{
			term.set_copied_input(m_cur_Q);
			m_list_active.push_front(&term);
		}
		else
		{
			m_list_active.push_front(&term);
			railterminal().device().do_inc_active();
			if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
			{
				if (m_next_scheduled_time > exec().time())
				{
					m_in_queue = queue_status::QUEUED;     // pending
					exec().qpush(m_next_scheduled_time, this);
				}
				else
				{
					m_in_queue = queue_status::DELIVERED;
					m_cur_Q = m_new_Q;
				}
				update_inputs();
			}
			else
				term.set_copied_input(m_cur_Q);
		}
	}

	inline void detail::net_t::remove_from_active_list(core_terminal_t &term) noexcept
	{
		gsl_Expects(!m_list_active.empty());
		m_list_active.remove(&term);
		if (m_list_active.empty())
			railterminal().device().do_dec_active();
	}

	inline const analog_net_t & analog_t::net() const noexcept
	{
		return plib::downcast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net() noexcept
	{
		return plib::downcast<analog_net_t &>(core_terminal_t::net());
	}

	inline nl_fptype terminal_t::operator ()() const noexcept { return net().Q_Analog(); }

	inline logic_net_t & logic_t::net() noexcept
	{
		return plib::downcast<logic_net_t &>(core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const noexcept
	{
		return plib::downcast<const logic_net_t &>(core_terminal_t::net());
	}

	inline netlist_sig_t logic_input_t::operator()() const noexcept
	{
		nl_assert(terminal_state() != STATE_INP_PASSIVE);
#if NL_USE_COPY_INSTEAD_OF_REFERENCE
		return m_Q;
#else
		return net().Q();
#endif
	}

	inline nl_fptype analog_input_t::Q_Analog() const noexcept
	{
		return net().Q_Analog();
	}

	inline void analog_output_t::push(nl_fptype val) noexcept
	{
		if (val != m_my_net.Q_Analog())
		{
			m_my_net.set_Q_Analog(val);
			m_my_net.toggle_and_push_to_queue(netlist_time::quantum());
		}
	}

	inline netlist_t &detail::device_object_t::exec() noexcept
	{
		return m_device->exec();
	}

	inline const netlist_t &detail::device_object_t::exec() const noexcept
	{
		return m_device->exec();
	}

	template <bool KEEP_STATS, typename T, typename S>
	inline void detail::net_t::process(T mask, const S &sig) noexcept
	{
		m_cur_Q = sig;

		if (KEEP_STATS)
		{
			for (auto & p : m_list_active)
			{
				p.set_copied_input(sig);
				auto *stats(p.device().stats());
				stats->m_stat_call_count.inc();
				if ((p.terminal_state() & mask))
				{
					auto g(stats->m_stat_total_time.guard());
					p.run_delegate();
				}
			}
		}
		else
		{
			for (auto &p : m_list_active)
			{
				p.set_copied_input(sig);
				if ((p.terminal_state() & mask) != 0)
					p.run_delegate();
			}
		}
	}

	template <bool KEEP_STATS>
	inline void detail::net_t::update_devs() noexcept
	{
		nl_assert(this->is_rail_net());

		m_in_queue = queue_status::DELIVERED; // mark as taken ...
		if (m_new_Q ^ m_cur_Q)
		{
			process<KEEP_STATS>((m_new_Q << core_terminal_t::INP_LH_SHIFT)
				| (m_cur_Q << core_terminal_t::INP_HL_SHIFT), m_new_Q);
		}
	}

	template <bool KEEP_STATS>
	inline void netlist_t::process_queue_stats(const netlist_time_ext delta) noexcept
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
				auto *const obj(top->object());
				m_queue.pop();
				if (obj != nullptr)
					obj->template update_devs<KEEP_STATS>();
				else
					break;
				if (KEEP_STATS)
					m_perf_out_processed.inc();
			} while (true);

			mc_net.set_next_scheduled_time(mc_time);
		}
	}

	inline void netlist_t::process_queue(netlist_time_ext delta) noexcept
	{
		if (!m_use_stats)
			process_queue_stats<false>(delta);
		else
		{
			auto sm_guard(m_stat_mainloop.guard());
			process_queue_stats<true>(delta);
		}
	}



	// -----------------------------------------------------------------------------
	// inline implementations - cold
	// -----------------------------------------------------------------------------

	template<typename T, typename... Args>
	inline device_arena::unique_ptr<T> detail::netlist_object_t::make_pool_object(Args&&... args)
	{
		return state().make_pool_object<T>(std::forward<Args>(args)...);
	}

	inline void param_t::update_param() noexcept
	{
		device().update_param();
	}

	template <typename T>
	param_num_t<T>::param_num_t(core_device_t &device, const pstring &name, const T val)
	: param_t(device, name)
	, m_param(val)
	{
		bool found = false;
		pstring p = this->get_initial(&device, &found);
		if (found)
		{
			plib::pfunction<nl_fptype> func;
			func.compile_infix(p, {});
			auto valx = func.evaluate();
			if (plib::is_integral<T>::value)
				if (plib::abs(valx - plib::trunc(valx)) > nlconst::magic(1e-6))
					throw nl_exception(MF_INVALID_NUMBER_CONVERSION_1_2(device.name() + "." + name, p));
			m_param = plib::narrow_cast<T>(valx);
		}

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename T>
	param_enum_t<T>::param_enum_t(core_device_t &device, const pstring &name, const T val)
	: param_t(device, name)
	, m_param(val)
	{
		bool found = false;
		pstring p = this->get_initial(&device, &found);
		if (found)
		{
			T temp(val);
			bool ok = temp.set_from_string(p);
			if (!ok)
			{
				device.state().log().fatal(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
				throw nl_exception(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
			}
			m_param = temp;
		}

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename ST, std::size_t AW, std::size_t DW>
	param_rom_t<ST, AW, DW>::param_rom_t(core_device_t &device, const pstring &name)
	: param_data_t(device, name)
	{
		auto f = stream();
		if (f != nullptr)
		{
			plib::istream_read(*f, m_data.data(), 1<<AW);
			// FIXME: check for failbit if not in validation.
		}
		else
			device.state().log().warning(MW_ROM_NOT_FOUND(str()));
	}

	template<class O, class C, typename... Args>
	void base_device_t::create_and_register_subdevice(O &owner, const pstring &name, device_arena::unique_ptr<C> &dev, Args&&... args)
	{
		dev = state().make_pool_object<C>(owner, name, std::forward<Args>(args)...);
	}

	inline solver::matrix_solver_t *analog_t::solver() const noexcept
	{
		return (this->has_net() ? net().solver() : nullptr);
	}


	extern template struct state_var<std::uint8_t>;
	extern template struct state_var<std::uint16_t>;
	extern template struct state_var<std::uint32_t>;
	extern template struct state_var<std::uint64_t>;
	extern template struct state_var<std::int8_t>;
	extern template struct state_var<std::int16_t>;
	extern template struct state_var<std::int32_t>;
	extern template struct state_var<std::int64_t>;
	extern template struct state_var<bool>;

	extern template class param_num_t<std::uint8_t>;
	extern template class param_num_t<std::uint16_t>;
	extern template class param_num_t<std::uint32_t>;
	extern template class param_num_t<std::uint64_t>;
	extern template class param_num_t<std::int8_t>;
	extern template class param_num_t<std::int16_t>;
	extern template class param_num_t<std::int32_t>;
	extern template class param_num_t<std::int64_t>;
	extern template class param_num_t<float>;
	extern template class param_num_t<double>;
	extern template class param_num_t<long double>;
	extern template class param_num_t<bool>;

	extern template class param_model_t::value_base_t<float>;
	extern template class param_model_t::value_base_t<double>;
	extern template class param_model_t::value_base_t<long double>;

	extern template class object_array_t<logic_input_t, 1>;
	extern template class object_array_t<logic_input_t, 2>;
	extern template class object_array_t<logic_input_t, 3>;
	extern template class object_array_t<logic_input_t, 4>;
	extern template class object_array_t<logic_input_t, 5>;
	extern template class object_array_t<logic_input_t, 6>;
	extern template class object_array_t<logic_input_t, 7>;
	extern template class object_array_t<logic_input_t, 8>;

	extern template class object_array_t<logic_output_t, 1>;
	extern template class object_array_t<logic_output_t, 2>;
	extern template class object_array_t<logic_output_t, 3>;
	extern template class object_array_t<logic_output_t, 4>;
	extern template class object_array_t<logic_output_t, 5>;
	extern template class object_array_t<logic_output_t, 6>;
	extern template class object_array_t<logic_output_t, 7>;
	extern template class object_array_t<logic_output_t, 8>;

} // namespace netlist

namespace plib
{
	template<typename X>
	struct ptype_traits<netlist::state_var<X>> : ptype_traits<X>
	{
	};
} // namespace plib



#endif // NLBASE_H_
