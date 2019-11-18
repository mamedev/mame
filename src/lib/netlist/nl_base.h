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

#include "plib/palloc.h" // owned_ptr
#include "plib/pdynlib.h"
#include "plib/pexception.h"
#include "plib/pfmtlog.h"
#include "plib/pfunction.h"
#include "plib/plists.h"
#include "plib/pmempool.h"
#include "plib/ppmf.h"
#include "plib/pstate.h"
#include "plib/pstonum.h"
#include "plib/pstream.h"
#include "plib/ptime.h"

#include "nl_errstr.h"
#include "nltypes.h"

#include <unordered_map>
#include <vector>

//============================================================
//  MACROS / New Syntax
//============================================================

/// Construct a netlist device name
#define NETLIB_NAME(chip) nld_ ## chip

#define NETLIB_OBJECT_DERIVED(name, pclass)                                   \
class NETLIB_NAME(name) : public NETLIB_NAME(pclass)

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
class NETLIB_NAME(name) : public device_t

#define NETLIB_CONSTRUCTOR_DERIVED(cname, pclass)                              \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
	: NETLIB_NAME(pclass)(owner, name)

#define NETLIB_CONSTRUCTOR_DERIVED_EX(cname, pclass, ...)                      \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name, __VA_ARGS__) \
	: NETLIB_NAME(pclass)(owner, name)

/// \brief Used to define the constructor of a netlist device.
///  Use this to define the constructor of a netlist device. Please refer to
///  #NETLIB_OBJECT for an example.

#define NETLIB_CONSTRUCTOR(cname)                                              \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
		: device_t(owner, name)

/// \brief Used to define the destructor of a netlist device.
/// The use of a destructor for netlist device should normally not be necessary.

#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)() noexcept

/// \brief Define an extended constructor and add further parameters to it.
/// The macro allows to add further parameters to a device constructor. This is
/// normally used for sub-devices and system devices only.

#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                      \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name, __VA_ARGS__) \
		: device_t(owner, name)

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
	public: virtual void timestep(const nl_fptype step)  noexcept override

/// \brief Used to implement the body of the time stepping code.
///
/// Used when the implementation is outside the class definition
///
/// Please see \ref NETLIB_IS_TIMESTEP for an example.
///
/// \param cname Name of object as given to \ref NETLIB_OBJECT
///
#define NETLIB_TIMESTEP(cname)                                                 \
	void NETLIB_NAME(cname) :: timestep(nl_fptype step) noexcept

#define NETLIB_FAMILY(family) , m_famsetter(*this, family)

#define NETLIB_DELEGATE(chip, name) nldelegate(&NETLIB_NAME(chip) :: name, this)

#define NETLIB_UPDATE_TERMINALSI() virtual void update_terminals() noexcept override
#define NETLIB_HANDLERI(name) virtual void name() noexcept
#define NETLIB_UPDATEI() virtual void update() noexcept override
#define NETLIB_UPDATE_PARAMI() virtual void update_param() noexcept override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUB_UPTR(ns, chip) unique_pool_ptr< ns :: nld_ ## chip >

#define NETLIB_HANDLER(chip, name) void NETLIB_NAME(chip) :: name() noexcept
#define NETLIB_UPDATE(chip) NETLIB_HANDLER(chip, update)

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param() noexcept

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals() noexcept

//============================================================
// Namespace starts
//============================================================

namespace netlist
{
	/// \brief Delegate type for device notification.
	///
	using nldelegate = plib::pmfp<void>;

	// -----------------------------------------------------------------------------
	// forward definitions
	// -----------------------------------------------------------------------------

	namespace devices
	{
		class NETLIB_NAME(solver);
		class NETLIB_NAME(mainclock);
		class NETLIB_NAME(base_proxy);
		class NETLIB_NAME(base_d_to_a_proxy);
		class NETLIB_NAME(base_a_to_d_proxy);
	} // namespace devices

	namespace solver
	{
		class matrix_solver_t;
	} // namespace solver

	class logic_output_t;
	class logic_input_t;
	class analog_net_t;
	class logic_net_t;
	class setup_t;
	class netlist_t;
	class netlist_state_t;
	class core_device_t;
	class device_t;

	namespace detail
	{
		class net_t;
	} // namespace detail

	//============================================================
	//  Exceptions
	//============================================================

	/// \brief Generic netlist exception.
	///  The exception is used in all events which are considered fatal.

	class nl_exception : public plib::pexception
	{
	public:
		/// \brief Constructor.
		///  Allows a descriptive text to be assed to the exception

		explicit nl_exception(const pstring &text //!< text to be passed
				)
		: plib::pexception(text) { }

		/// \brief Constructor.
		///  Allows to use \ref plib::pfmt logic to be used in exception

		template<typename... Args>
		explicit nl_exception(const pstring &fmt //!< format to be used
			, Args&&... args //!< arguments to be passed
			)
		: plib::pexception(plib::pfmt(fmt)(std::forward<Args>(args)...)) { }
	};

	/// \brief Logic families descriptors are used to create proxy devices.
	///  The logic family describes the analog capabilities of logic devices,
	///  inputs and outputs.

	class logic_family_desc_t
	{
	public:
		logic_family_desc_t();

		COPYASSIGNMOVE(logic_family_desc_t, delete)

		virtual ~logic_family_desc_t() noexcept = default;

		virtual unique_pool_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_output_t *proxied) const = 0;
		virtual unique_pool_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_input_t *proxied) const = 0;

		// FIXME: remove fixed_V()
		nl_fptype fixed_V() const noexcept{return m_fixed_V; }
		nl_fptype low_thresh_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_low_thresh_PCNT; }
		nl_fptype high_thresh_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_high_thresh_PCNT; }
		nl_fptype low_offset_V() const noexcept{ return m_low_VO; }
		nl_fptype high_offset_V() const noexcept{ return m_high_VO; }
		nl_fptype R_low() const noexcept{ return m_R_low; }
		nl_fptype R_high() const noexcept{ return m_R_high; }

		bool is_above_high_thresh_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return (V - VN) > high_thresh_V(VN, VP); }

		bool is_below_low_thresh_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return (V - VN) < low_thresh_V(VN, VP); }

		nl_fptype m_fixed_V;           //!< For variable voltage families, specify 0. For TTL this would be 5.
		nl_fptype m_low_thresh_PCNT;   //!< low input threshhold offset. If the input voltage is below this value times supply voltage, a "0" input is signalled
		nl_fptype m_high_thresh_PCNT;  //!< high input threshhold offset. If the input voltage is above the value times supply voltage, a "0" input is signalled
		nl_fptype m_low_VO;            //!< low output voltage offset. This voltage is output if the ouput is "0"
		nl_fptype m_high_VO;           //!< high output voltage offset. The supply voltage minus this offset is output if the ouput is "1"
		nl_fptype m_R_low;             //!< low output resistance. Value of series resistor used for low output
		nl_fptype m_R_high;            //!< high output resistance. Value of series resistor used for high output
	};

	/// \brief Base class for devices, terminals, outputs and inputs which support
	///  logic families.
	///  This class is a storage container to store the logic family for a
	///  netlist object. You will not directly use it. Please refer to
	///  \ref NETLIB_FAMILY to learn how to define a logic family for a device.
	///
	/// All terminals inherit the family description from the device
	/// The default is the ttl family, but any device can override the family.
	/// For individual terminals, the family can be overwritten as well.
	///

	class logic_family_t
	{
	public:
		logic_family_t() : m_logic_family(nullptr) {}
		COPYASSIGNMOVE(logic_family_t, delete)

		const logic_family_desc_t *logic_family() const noexcept { return m_logic_family; }
		void set_logic_family(const logic_family_desc_t *fam) noexcept { m_logic_family = fam; }

	protected:
		~logic_family_t() noexcept = default; // prohibit polymorphic destruction
		const logic_family_desc_t *m_logic_family;
	};

	const logic_family_desc_t *family_TTL();        ///< logic family for TTL devices.
	const logic_family_desc_t *family_CD4XXX();     ///< logic family for CD4XXX CMOS devices.

	/// \brief A persistent variable template.
	///  Use the state_var template to define a variable whose value is saved.
	///  Within a device definition use
	///
	///      NETLIB_OBJECT(abc)
	///      {
	///          NETLIB_CONSTRUCTOR(abc)
	///          , m_var(*this, "myvar", 0)
	///          ...
	///          state_var<unsigned> m_var;
	///      }
	template <typename T>
	struct state_var
	{
	public:
		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name,    //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);

		//! Destructor.
		~state_var() noexcept = default;
		//! Copy Constructor.
		constexpr state_var(const state_var &rhs) = default;
		//! Move Constructor.
		constexpr state_var(state_var &&rhs) noexcept = default;
		//! Assignment operator to assign value of a state var.
		C14CONSTEXPR state_var &operator=(const state_var &rhs) = default; // OSX doesn't like noexcept
		//! Assignment move operator to assign value of a state var.
		C14CONSTEXPR state_var &operator=(state_var &&rhs) noexcept = default;
		//! Assignment operator to assign value of type T.
		C14CONSTEXPR state_var &operator=(const T &rhs) noexcept { m_value = rhs; return *this; }
		//! Assignment move operator to assign value of type T.
		C14CONSTEXPR state_var &operator=(T &&rhs) noexcept { std::swap(m_value, rhs); return *this; }
		//! Return non-const value of state variable.
		C14CONSTEXPR operator T & () noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr operator const T & () const noexcept { return m_value; }
		//! Return pointer to state variable.
		C14CONSTEXPR T * ptr() noexcept { return &m_value; }
		//! Return const pointer to state variable.
		constexpr const T * ptr() const noexcept{ return &m_value; }

	private:
		T m_value;
	};

	/// \brief A persistent array template.
	///  Use this state_var template to define an array whose contents are saved.
	///  Please refer to \ref state_var.
	///
	///  \tparam C container class to use.

	template <typename C>
	struct state_container : public C
	{
	public:
		using value_type = typename C::value_type;
		//! Constructor.
		template <typename O>
		state_container(O &owner,           //!< owner must have a netlist() method.
				const pstring &name,        //!< identifier/name for this state variable
				const value_type &value     //!< Initial value after construction
				);
		//! Constructor.
		template <typename O>
		state_container(O &owner,           //!< owner must have a netlist() method.
				const pstring &name,        //!< identifier/name for this state variable
				std::size_t n,              //!< number of elements to allocate
				const value_type &value     //!< Initial value after construction
				);
		//! Copy Constructor.
		state_container(const state_container &rhs) noexcept = default;
		//! Destructor.
		~state_container() noexcept = default;
		//! Move Constructor.
		state_container(state_container &&rhs) noexcept = default;
		state_container &operator=(const state_container &rhs) noexcept = default;
		state_container &operator=(state_container &&rhs) noexcept = default;
	};

	// -----------------------------------------------------------------------------
	// State variables - predefined and c++11 non-optional
	// -----------------------------------------------------------------------------

	/// \brief predefined state variable type for uint8_t
	using state_var_u8 = state_var<std::uint8_t>;
	/// \brief predefined state variable type for int8_t
	using state_var_s8 = state_var<std::int8_t>;

	/// \brief predefined state variable type for uint32_t
	using state_var_u32 = state_var<std::uint32_t>;
	/// \brief predefined state variable type for int32_t
	using state_var_s32 = state_var<std::int32_t>;
	/// \brief predefined state variable type for sig_t
	using state_var_sig = state_var<netlist_sig_t>;

	namespace detail {

		template <typename C, typename T>
		struct property_store_t
		{
			static void add(const C *obj, const T &aname) noexcept
			{
				store().insert({obj, aname});
			}

			static const T &get(const C *obj) noexcept
			{
				try
				{
					auto ret(store().find(obj));
					nl_assert(ret != store().end());
					return ret->second;
				}
				catch (...)
				{
					nl_assert_always(true, "exception in property_store_t.get()");
					return *static_cast<T *>(nullptr);
				}
			}

			static void remove(const C *obj) noexcept
			{
				store().erase(store().find(obj));
			}

			static std::unordered_map<const C *, T> &store() noexcept
			{
				static std::unordered_map<const C *, T> lstore;
				return lstore;
			}

		};

		// -----------------------------------------------------------------------------
		// object_t
		// -----------------------------------------------------------------------------

		/// \brief The base class for netlist devices, terminals and parameters.
		///
		///  This class serves as the base class for all device, terminal and
		///  objects. It provides new and delete operators to support e.g. pooled
		///  memory allocation to enhance locality. Please refer to \ref NL_USE_MEMPOOL as
		///  well.

		class object_t
		{
		public:

			/// \brief Constructor.
			/// Every class derived from the object_t class must have a name.
			///
			/// \param aname string containing name of the object

			explicit object_t(const pstring &aname)
			{
				props::add(this, aname);
			}

			COPYASSIGNMOVE(object_t, delete)
			/// \brief return name of the object
			///
			/// \returns name of the object.

			const pstring &name() const noexcept
			{
				return props::get(this);
			}

		protected:

			using props = property_store_t<object_t, pstring>;

			// only childs should be destructible
			~object_t() noexcept
			{
				props::remove(this);
			}

		private:
		};

		struct netlist_ref
		{
			explicit netlist_ref(netlist_t &nl);

			COPYASSIGNMOVE(netlist_ref, delete)

			netlist_state_t & state() noexcept;
			const netlist_state_t & state() const noexcept;

			netlist_t & exec() noexcept { return m_netlist; }
			const netlist_t & exec() const noexcept { return m_netlist; }

		protected:
			~netlist_ref() noexcept = default; // prohibit polymorphic destruction

		private:
			netlist_t & m_netlist;

		};

		// -----------------------------------------------------------------------------
		// device_object_t
		// -----------------------------------------------------------------------------

		/// \brief Base class for all objects being owned by a device.
		///
		/// Serves as the base class of all objects being owned by a device.
		///

		class device_object_t : public object_t
		{
		public:
			/// \brief Constructor.
			///
			/// \param dev  device owning the object.
			/// \param name string holding the name of the device

			device_object_t(core_device_t &dev, const pstring &name);

			/// \brief returns reference to owning device.
			/// \returns reference to owning device.

			core_device_t &device() noexcept { return m_device; }
			const core_device_t &device() const noexcept { return m_device; }

			/// \brief The netlist owning the owner of this object.
			/// \returns reference to netlist object.

			netlist_state_t &state() noexcept;
			const netlist_state_t &state() const noexcept;

			netlist_t &exec() noexcept;
			const netlist_t &exec() const noexcept;

		private:
			core_device_t & m_device;
		};

		// -----------------------------------------------------------------------------
		// core_terminal_t
		// -----------------------------------------------------------------------------

		/// \brief Base class for all terminals.
		///
		/// All terminals are derived from this class.
		///
		class core_terminal_t : public device_object_t,
								public plib::linkedlist_t<core_terminal_t>::element_t
		{
		public:

			static constexpr const unsigned int INP_HL_SHIFT = 0;
			static constexpr const unsigned int INP_LH_SHIFT = 1;

			enum state_e {
				STATE_INP_PASSIVE = 0,
				STATE_INP_HL      = (1 << INP_HL_SHIFT),
				STATE_INP_LH      = (1 << INP_LH_SHIFT),
				STATE_INP_ACTIVE  = STATE_INP_HL | STATE_INP_LH,
				STATE_OUT = 128,
				STATE_BIDIR = 256
			};

			core_terminal_t(core_device_t &dev, const pstring &aname,
					const state_e state, nldelegate delegate = nldelegate());
			virtual ~core_terminal_t() noexcept = default;

			COPYASSIGNMOVE(core_terminal_t, delete)

			/// \brief The object type.
			/// \returns type of the object

			terminal_type type() const noexcept(false);
			/// \brief Checks if object is of specified type.
			/// \param atype type to check object against.
			/// \returns true if object is of specified type else false.

			bool is_type(const terminal_type atype) const noexcept(false) { return (type() == atype); }

			void set_net(net_t *anet) noexcept { m_net = anet; }
			void clear_net() noexcept { m_net = nullptr; }
			bool has_net() const noexcept { return (m_net != nullptr); }

			const net_t & net() const noexcept { return *m_net;}
			net_t & net() noexcept { return *m_net;}

			bool is_logic() const noexcept;
			bool is_logic_input() const noexcept;
			bool is_logic_output() const noexcept;
			bool is_analog() const noexcept;
			bool is_analog_input() const noexcept;
			bool is_analog_output() const noexcept;

			bool is_state(state_e astate) const noexcept { return (m_state == astate); }
			state_e terminal_state() const noexcept { return m_state; }
			void set_state(state_e astate) noexcept { m_state = astate; }

			void reset() noexcept { set_state(is_type(terminal_type::OUTPUT) ? STATE_OUT : STATE_INP_ACTIVE); }

	#if NL_USE_COPY_INSTEAD_OF_REFERENCE
			void set_copied_input(netlist_sig_t val) noexcept
			{
				m_Q = val;
			}

			state_var_sig m_Q;
	#else
			void set_copied_input(netlist_sig_t val) const noexcept { plib::unused_var(val); }
	#endif

			void set_delegate(const nldelegate &delegate) noexcept { m_delegate = delegate; }
			nldelegate &delegate() noexcept { return m_delegate; }
			const nldelegate &delegate() const noexcept { return m_delegate; }
			void run_delegate() noexcept { m_delegate(); }
		private:
			nldelegate m_delegate;
			net_t * m_net;
			state_var<state_e> m_state;
		};

		// -----------------------------------------------------------------------------
		// net_t
		// -----------------------------------------------------------------------------

		class net_t :
				public object_t,
				public netlist_ref
		{
		public:

			enum class queue_status
			{
				DELAYED_DUE_TO_INACTIVE = 0,
				QUEUED,
				DELIVERED
			};

			net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *railterminal = nullptr);

			COPYASSIGNMOVE(net_t, delete)

			virtual ~net_t() noexcept = default;

			void reset() noexcept;

			void toggle_new_Q() noexcept { m_new_Q = (m_cur_Q ^ 1);   }

			void toggle_and_push_to_queue(netlist_time delay) noexcept
			{
				toggle_new_Q();
				push_to_queue(delay);
			}

			void push_to_queue(netlist_time delay) noexcept;
			bool is_queued() const noexcept { return m_in_queue == queue_status::QUEUED; }

			template <bool KEEP_STATS>
			void update_devs() noexcept;

			netlist_time next_scheduled_time() const noexcept { return m_next_scheduled_time; }
			void set_next_scheduled_time(netlist_time ntime) noexcept { m_next_scheduled_time = ntime; }

			bool isRailNet() const noexcept { return !(m_railterminal == nullptr); }
			core_terminal_t & railterminal() const noexcept { return *m_railterminal; }

			std::size_t num_cons() const noexcept { return m_core_terms.size(); }

			void add_to_active_list(core_terminal_t &term) noexcept;
			void remove_from_active_list(core_terminal_t &term) noexcept;

			// setup stuff

			void add_terminal(core_terminal_t &terminal) noexcept(false);
			void remove_terminal(core_terminal_t &terminal) noexcept(false);

			bool is_logic() const noexcept;
			bool is_analog() const noexcept;

			void rebuild_list();     // rebuild m_list after a load
			void move_connections(net_t &dest_net);

			std::vector<core_terminal_t *> &core_terms() noexcept { return m_core_terms; }
	#if NL_USE_COPY_INSTEAD_OF_REFERENCE
			void update_inputs() noexcept
			{
				for (auto & term : m_core_terms)
					term->m_Q = m_cur_Q;
			}
	#else
			void update_inputs() const noexcept
			{
				// nothing needs to be done
			}
	#endif

		protected:

			// only used for logic nets
			netlist_sig_t Q() const noexcept { return m_cur_Q; }

			// only used for logic nets
			void initial(netlist_sig_t val) noexcept
			{
				m_cur_Q = m_new_Q = val;
				update_inputs();
			}

			// only used for logic nets
			void set_Q_and_push(netlist_sig_t newQ, netlist_time delay) noexcept
			{
				if (newQ != m_new_Q)
				{
					m_new_Q = newQ;
					push_to_queue(delay);
				}
			}

			// only used for logic nets
			void set_Q_time(netlist_sig_t newQ, netlist_time at) noexcept
			{
				if (newQ != m_new_Q)
				{
					m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
					m_next_scheduled_time = at;
					m_cur_Q = m_new_Q = newQ;
					update_inputs();
				}
				else
				{
					m_cur_Q = newQ;
					update_inputs();
				}
			}

			// internal state support
			// FIXME: get rid of this and implement export/import in MAME

			// only used for logic nets
			netlist_sig_t *Q_state_ptr() noexcept { return m_cur_Q.ptr(); }

		private:
			state_var<netlist_sig_t> m_new_Q;
			state_var<netlist_sig_t> m_cur_Q;
			state_var<queue_status>  m_in_queue;    // 0: not in queue, 1: in queue, 2: last was taken
			state_var<netlist_time>  m_next_scheduled_time;

			core_terminal_t * m_railterminal;
			plib::linkedlist_t<core_terminal_t> m_list_active;
			std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

			template <bool KEEP_STATS, typename T>
			void process(T mask, netlist_sig_t sig) noexcept;
		};
	} // namespace detail

	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public detail::core_terminal_t
	{
	public:

		analog_t(core_device_t &dev, const pstring &aname, const state_e state,
			nldelegate delegate = nldelegate());

		const analog_net_t & net() const noexcept;
		analog_net_t & net() noexcept;
	};

	// -----------------------------------------------------------------------------
	// terminal_t
	// -----------------------------------------------------------------------------

	class terminal_t : public analog_t
	{
	public:

		terminal_t(core_device_t &dev, const pstring &aname, terminal_t *otherterm);

		nl_fptype operator ()() const  noexcept;

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
			// FIXME: is this check still needed?
			if (m_go1 != nullptr)
			{
				*m_Idr1 = I;
				*m_go1 = GO;
				*m_gt1 = GT;
			}
		}

		void solve_now();
		void schedule_solve_after(netlist_time after) noexcept;

		void set_ptrs(nl_fptype *gt, nl_fptype *go, nl_fptype *Idr) noexcept(false);

		terminal_t *connected_terminal() const noexcept { return m_connected_terminal; }
	private:

		nl_fptype *m_Idr1; // drive current
		nl_fptype *m_go1;  // conductance for Voltage from other term
		nl_fptype *m_gt1;  // conductance for total conductance

		terminal_t *m_connected_terminal; // FIXME: only used during setup

	};


	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	class logic_t : public detail::core_terminal_t, public logic_family_t
	{
	public:
		logic_t(core_device_t &dev, const pstring &aname,
				const state_e state, nldelegate delegate = nldelegate());

		logic_net_t & net() noexcept;
		const logic_net_t &  net() const noexcept;

	protected:

	private:
	};

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

	class logic_input_t : public logic_t
	{
	public:
		logic_input_t(core_device_t &dev, const pstring &aname,
				nldelegate delegate = nldelegate());

		netlist_sig_t operator()() const noexcept
		{
			return Q();
		}

		void inactivate() noexcept;
		void activate() noexcept;
		void activate_hl() noexcept;
		void activate_lh() noexcept;
	private:
		netlist_sig_t Q() const noexcept;
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
				nldelegate delegate = nldelegate() ///< delegate
		);

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype operator()() const noexcept { return Q_Analog(); }

		/// \brief returns voltage at terminal.
		///  \returns voltage at terminal.
		nl_fptype Q_Analog() const noexcept;
	};

	class logic_net_t : public detail::net_t
	{
	public:

		logic_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal = nullptr);

		using detail::net_t::Q;
		using detail::net_t::initial;
		using detail::net_t::set_Q_and_push;
		using detail::net_t::set_Q_time;
		using detail::net_t::Q_state_ptr;
	};

	class analog_net_t : public detail::net_t
	{
	public:

		using list_t =  std::vector<analog_net_t *>;

		friend class detail::net_t;

		analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal = nullptr);

		nl_fptype Q_Analog() const noexcept { return m_cur_Analog; }
		void set_Q_Analog(const nl_fptype v) noexcept { m_cur_Analog = v; }
		nl_fptype *Q_Analog_state_ptr() noexcept { return m_cur_Analog.ptr(); }

		//FIXME: needed by current solver code
		solver::matrix_solver_t *solver() const noexcept { return m_solver; }
		void set_solver(solver::matrix_solver_t *solver) noexcept { m_solver = solver; }

	private:
		state_var<nl_fptype>     m_cur_Analog;
		solver::matrix_solver_t *m_solver;
	};

	// -----------------------------------------------------------------------------
	// logic_output_t
	// -----------------------------------------------------------------------------

	class logic_output_t : public logic_t
	{
	public:

		logic_output_t(core_device_t &dev, const pstring &aname);

		void initial(netlist_sig_t val) noexcept;

		void push(netlist_sig_t newQ, netlist_time delay) noexcept
		{
			m_my_net.set_Q_and_push(newQ, delay); // take the shortcut
		}

		void set_Q_time(netlist_sig_t newQ, netlist_time at) noexcept
		{
			m_my_net.set_Q_time(newQ, at); // take the shortcut
		}

	private:
		logic_net_t m_my_net;
	};

	class analog_output_t : public analog_t
	{
	public:
		analog_output_t(core_device_t &dev, const pstring &aname);

		void push(nl_fptype val) noexcept { set_Q(val); }
		void initial(nl_fptype val) noexcept;

	private:
		void set_Q(nl_fptype newQ) noexcept;
		analog_net_t m_my_net;
	};

	// -----------------------------------------------------------------------------
	// core_device_t
	// -----------------------------------------------------------------------------

	class core_device_t :
			public detail::object_t,
			public logic_family_t,
			public detail::netlist_ref
	{
	public:
		core_device_t(netlist_state_t &owner, const pstring &name);
		core_device_t(core_device_t &owner, const pstring &name);

		COPYASSIGNMOVE(core_device_t, delete)

		virtual ~core_device_t() noexcept = default;

		void do_inc_active() noexcept
		{
			if (m_hint_deactivate)
			{
				if (++m_active_outputs == 1)
				{
					if (m_stats)
						m_stats->m_stat_inc_active.inc();
					inc_active();
				}
			}
		}

		void do_dec_active() noexcept
		{
			if (m_hint_deactivate)
				if (--m_active_outputs == 0)
				{
					dec_active();
				}
		}

		void set_hint_deactivate(bool v) noexcept { m_hint_deactivate = v; }
		bool get_hint_deactivate() const noexcept { return m_hint_deactivate; }
		// Has to be set in device reset
		void set_active_outputs(int n) noexcept { m_active_outputs = n; }

		void set_default_delegate(detail::core_terminal_t &term);

		// stats
		struct stats_t
		{
			// NL_KEEP_STATISTICS
			plib::pperftime_t<true>  m_stat_total_time;
			plib::pperfcount_t<true> m_stat_call_count;
			plib::pperfcount_t<true> m_stat_inc_active;
		};

		unique_pool_ptr<stats_t> m_stats;

		virtual void update() noexcept { }
		virtual void reset() { }

	protected:

		virtual void inc_active() noexcept {  }
		virtual void dec_active() noexcept {  }

		log_type & log();

	public:
		virtual void timestep(const nl_fptype st) noexcept { plib::unused_var(st); }
		virtual void update_terminals() noexcept { }

		virtual void update_param() noexcept {}
		virtual bool is_dynamic() const noexcept { return false; }
		virtual bool is_timestep() const noexcept { return false; }

	private:
		bool            m_hint_deactivate;
		state_var_s32   m_active_outputs;
	};

	// -----------------------------------------------------------------------------
	// device_t
	// -----------------------------------------------------------------------------

	class device_t : public core_device_t
	{
	public:

		device_t(netlist_state_t &owner, const pstring &name);
		device_t(core_device_t &owner, const pstring &name);

		COPYASSIGNMOVE(device_t, delete)

		~device_t() noexcept override = default;

		template<class C, typename... Args>
		void create_and_register_subdevice(const pstring &name, unique_pool_ptr<C> &dev, Args&&... args);

		void register_subalias(const pstring &name, detail::core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect(const pstring &t1, const pstring &t2);
		void connect(const detail::core_terminal_t &t1, const detail::core_terminal_t &t2);
		void connect_post_start(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
	protected:

		NETLIB_UPDATEI() { }
		NETLIB_UPDATE_TERMINALSI() { }

	private:
	};

	// -----------------------------------------------------------------------------
	// param_t
	// -----------------------------------------------------------------------------

	class param_t : public detail::device_object_t
	{
	public:

		enum param_type_t {
			STRING,
			DOUBLE,
			INTEGER,
			LOGIC,
			POINTER // Special-case which is always initialized at MAME startup time
		};

		param_t(device_t &device, const pstring &name);

		COPYASSIGNMOVE(param_t, delete)

		param_type_t param_type() const noexcept(false);

	protected:
		virtual ~param_t() noexcept = default; // not intended to be destroyed

		void update_param() noexcept
		{
			device().update_param();
		}

		pstring get_initial(const device_t &dev, bool *found) const;

		template<typename C>
		void set(C &p, const C v) noexcept
		{
			if (p != v)
			{
				p = v;
				update_param();
			}
		}

	};

	// -----------------------------------------------------------------------------
	// numeric parameter template
	// -----------------------------------------------------------------------------

	template <typename T>
	class param_num_t final: public param_t
	{
	public:
		param_num_t(device_t &device, const pstring &name, const T val);

		T operator()() const noexcept { return m_param; }
		operator T() const noexcept { return m_param; }

		void setTo(const T &param) noexcept { set(m_param, param); }
	private:
		T m_param;
	};

	template <typename T>
	class param_enum_t final: public param_t
	{
	public:
		param_enum_t(device_t &device, const pstring &name, const T val);

		T operator()() const noexcept { return T(m_param); }
		operator T() const noexcept { return T(m_param); }
		void setTo(const T &param) noexcept { set(m_param, static_cast<int>(param)); }
	private:
		int m_param;
	};

	// FIXME: these should go as well
	using param_logic_t = param_num_t<bool>;
	using param_int_t = param_num_t<int>;
	using param_fp_t = param_num_t<nl_fptype>;

	// -----------------------------------------------------------------------------
	// pointer parameter
	// -----------------------------------------------------------------------------

	class param_ptr_t final: public param_t
	{
	public:
		param_ptr_t(device_t &device, const pstring &name, std::uint8_t* val);
		std::uint8_t * operator()() const noexcept { return m_param; }
		void setTo(std::uint8_t *param) noexcept { set(m_param, param); }
	private:
		std::uint8_t* m_param;
	};

	// -----------------------------------------------------------------------------
	// string parameter
	// -----------------------------------------------------------------------------

	class param_str_t : public param_t
	{
	public:
		param_str_t(device_t &device, const pstring &name, const pstring &val);

		const pstring &operator()() const noexcept { return str(); }
		void setTo(const pstring &param)
		{
			if (m_param != param)
			{
				m_param = param;
				changed();
				update_param();
			}
		}
	protected:
		virtual void changed() noexcept;
		const pstring &str() const noexcept { return m_param; }
	private:
		pstring m_param;
	};

	// -----------------------------------------------------------------------------
	// model parameter
	// -----------------------------------------------------------------------------

	class param_model_t : public param_str_t
	{
	public:

		template <typename T>
		class value_base_t
		{
		public:
			value_base_t(param_model_t &param, const pstring &name)
			: m_value(static_cast<T>(param.value(name)))
			{
			}
			T operator()() const noexcept { return m_value; }
			operator T() const noexcept { return m_value; }
		private:
			const T m_value;
		};

		using value_t = value_base_t<nl_fptype>;

		param_model_t(device_t &device, const pstring &name, const pstring &val)
		: param_str_t(device, name, val) { }

		pstring value_str(const pstring &entity);
		nl_fptype value(const pstring &entity);
		pstring type();
		// hide this
		void setTo(const pstring &param) = delete;
	protected:
		void changed() noexcept override;
	private:
};

	// -----------------------------------------------------------------------------
	// data parameter
	// -----------------------------------------------------------------------------

	class param_data_t : public param_str_t
	{
	public:
		param_data_t(device_t &device, const pstring &name)
		: param_str_t(device, name, "")
		{
		}

		plib::unique_ptr<std::istream> stream();
	protected:
		void changed() noexcept override { }
	};

	// -----------------------------------------------------------------------------
	// rom parameter
	// -----------------------------------------------------------------------------

	template <typename ST, std::size_t AW, std::size_t DW>
	class param_rom_t final: public param_data_t
	{
	public:

		param_rom_t(device_t &device, const pstring &name);

		ST operator[] (std::size_t n) const noexcept { return m_data[n]; }
	protected:
		void changed() noexcept override
		{
			stream()->read(reinterpret_cast<std::istream::char_type *>(&m_data[0]),1<<AW);
		}

	private:
		std::array<ST, 1 << AW> m_data;
	};

	// -----------------------------------------------------------------------------
	// family_setter_t
	// -----------------------------------------------------------------------------

	namespace detail {

		struct family_setter_t
		{
			// NOLINTNEXTLINE(modernize-use-equals-default)
			family_setter_t();
			family_setter_t(core_device_t &dev, const pstring &desc);
			family_setter_t(core_device_t &dev, const logic_family_desc_t &desc);
		};

		template <class T, bool TS>
		using timed_queue = plib::timed_queue_linear<T, TS>;

		// Use timed_queue_heap to use stdc++ heap functions instead of linear processing.
		/// This slows down processing by about 25% on a Kaby Lake.


		//template <class T, bool TS>
		//using timed_queue = timed_queue_heap<T, TS>;

		// -----------------------------------------------------------------------------
		// queue_t
		// -----------------------------------------------------------------------------

		// We don't need a thread-safe queue currently. Parallel processing of
		// solvers will update inputs after parallel processing.

		class queue_t :
				//public timed_queue<pqentry_t<net_t *, netlist_time>, false, NL_KEEP_STATISTICS>,
				public timed_queue<plib::pqentry_t<net_t *, netlist_time>, false>,
				public netlist_ref,
				public plib::state_manager_t::callback_t
		{
		public:
			using base_queue = timed_queue<plib::pqentry_t<net_t *, netlist_time>, false>;
			using entry_t = plib::pqentry_t<net_t *, netlist_time>;
			explicit queue_t(netlist_t &nl);
			~queue_t() noexcept override = default;

			queue_t(const queue_t &) = delete;
			queue_t(queue_t &&) = delete;
			queue_t &operator=(const queue_t &) = delete;
			queue_t &operator=(queue_t &&) = delete;

		protected:

			void register_state(plib::state_manager_t &manager, const pstring &module) override;
			void on_pre_save(plib::state_manager_t &manager) override;
			void on_post_load(plib::state_manager_t &manager) override;

		private:
			std::size_t m_qsize;
			std::vector<netlist_time::internal_type> m_times;
			std::vector<std::size_t> m_net_ids;
		};

	} // namespace detail

	// -----------------------------------------------------------------------------
	// netlist_state__t
	// -----------------------------------------------------------------------------

	class netlist_state_t
	{
	public:

		using nets_collection_type = std::vector<owned_pool_ptr<detail::net_t>>;

		// need to preserve order of device creation ...
		using devices_collection_type = std::vector<std::pair<pstring, owned_pool_ptr<core_device_t>>>;
		netlist_state_t(const pstring &aname,
			plib::unique_ptr<callbacks_t> &&callbacks);

		COPYASSIGNMOVE(netlist_state_t, delete)

		/// \brief Destructor
		///
		/// The destructor is virtual to allow implementation specific devices
		/// to connect to the outside world. For examples see MAME netlist.cpp.
		///
		virtual ~netlist_state_t() noexcept = default;

		friend class netlist_t; // allow access to private members

		template<class C>
		static bool check_class(core_device_t *p) noexcept
		{
			return dynamic_cast<C *>(p) != nullptr;
		}

		core_device_t *get_single_device(const pstring &classname, bool (*cc)(core_device_t *)) const;

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
			for (auto &d : m_devices)
			{
				auto dev = dynamic_cast<C *>(d.second.get());
				if (dev != nullptr)
					tmp.push_back(dev);
			}
			return tmp;
		}

		// logging and name

		const pstring &name() const noexcept { return m_name; }

		log_type & log() noexcept { return m_log; }
		const log_type &log() const noexcept { return m_log; }

		plib::dynlib &lib() const noexcept { return *m_lib; }

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

		detail::net_t *find_net(const pstring &name) const;
		std::size_t find_net_id(const detail::net_t *net) const;

		template <typename T>
		void register_net(owned_pool_ptr<T> &&net) { m_nets.push_back(std::move(net)); }

		/// \brief Get device pointer by name
		///
		///
		/// \param name Name of the device
		///
		/// \return core_device_t pointer if device exists, else nullptr

		core_device_t *find_device(const pstring &name) const
		{
			for (auto & d : m_devices)
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
		void register_device(const pstring &name, owned_pool_ptr<T> &&dev) noexcept(false)
		{
			for (auto & d : m_devices)
				if (d.first == name)
				{
					dev.release();
					log().fatal(MF_DUPLICATE_NAME_DEVICE_LIST(name));
					plib::pthrow<nl_exception>(MF_DUPLICATE_NAME_DEVICE_LIST(name));
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
		void register_device(const pstring &name, unique_pool_ptr<T> &&dev)
		{
			register_device(name, owned_pool_ptr<T>(dev.release(), true, dev.get_deleter()));
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

		// FIXME: make a postload member and include code there
		void rebuild_lists(); // must be called after post_load !

		static void compile_defines(std::vector<std::pair<pstring, pstring>> &defs);

		nets_collection_type & nets() noexcept { return m_nets; }
		const nets_collection_type & nets() const noexcept { return m_nets; }

		devices_collection_type & devices() noexcept { return m_devices; }
		const devices_collection_type & devices() const noexcept { return m_devices; }

		// sole use is to manage lifetime of family objects
		std::unordered_map<pstring, plib::unique_ptr<logic_family_desc_t>> m_family_cache;

		template<typename T, typename... Args>
		unique_pool_ptr<T> make_object(Args&&... args)
		{
			return m_pool.make_unique<T>(std::forward<Args>(args)...);
		}
		// memory pool - still needed in some places
		nlmempool &pool() noexcept { return m_pool; }
		const nlmempool &pool() const noexcept { return m_pool; }

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

	private:

		void reset();
		nlmempool							m_pool; // must be deleted last!

		pstring                             m_name;
		unique_pool_ptr<netlist_t>			m_netlist;
		plib::unique_ptr<plib::dynlib>      m_lib; // external lib needs to be loaded as long as netlist exists
		plib::state_manager_t               m_state;
		plib::unique_ptr<callbacks_t>       m_callbacks;
		log_type                            m_log;
		plib::unique_ptr<setup_t>           m_setup;

		nets_collection_type                m_nets;
		// sole use is to manage lifetime of net objects
		devices_collection_type             m_devices;
		bool m_extended_validation;
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

			NETLIB_RESETI()
			{
				m_Q.net().set_next_scheduled_time(netlist_time::zero());
			}

			NETLIB_UPDATE_PARAMI()
			{
				m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			}

			NETLIB_UPDATEI()
			{
				// only called during start up.
				// mainclock will step forced by main loop
			}

		public:
			logic_output_t m_Q;
			netlist_time m_inc;
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

		explicit netlist_t(netlist_state_t &state);

		COPYASSIGNMOVE(netlist_t, delete)

		virtual ~netlist_t() noexcept = default;

		// run functions

		netlist_time time() const noexcept { return m_time; }

		void process_queue(netlist_time delta) noexcept;
		void abort_current_queue_slice() noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.retime<false>(detail::queue_t::entry_t(m_time, nullptr));
			else
				m_queue.retime<true>(detail::queue_t::entry_t(m_time, nullptr));
		}

		const detail::queue_t &queue() const noexcept { return m_queue; }

		template <typename E>
		void qpush(E && e) noexcept
		{
			if (!NL_USE_QUEUE_STATS || !m_use_stats)
				m_queue.push<false>(std::forward<E>(e)); // NOLINT(performance-move-const-arg)
			else
				m_queue.push<true>(std::forward<E>(e)); // NOLINT(performance-move-const-arg)
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

		bool stats_enabled() const noexcept { return m_use_stats; }
		void enable_stats(bool val) noexcept { m_use_stats = val; }

	private:

		template <bool KEEP_STATS>
		void process_queue_stats(netlist_time delta) noexcept;

		netlist_state_t	&				    m_state;
		devices::NETLIB_NAME(solver) *      m_solver;

		// mostly rw
		PALIGNAS_CACHELINE()
		netlist_time                        m_time;
		devices::NETLIB_NAME(mainclock) *   m_mainclock;

		PALIGNAS_CACHELINE()
		detail::queue_t                     m_queue;
		bool                                m_use_stats;

		// performance
		plib::pperftime_t<true>             m_stat_mainloop;
		plib::pperfcount_t<true>            m_perf_out_processed;
	};

	// -----------------------------------------------------------------------------
	// Support classes for devices
	// -----------------------------------------------------------------------------

	template<class C, int N>
	class object_array_t : public plib::uninitialised_array_t<C, N>
	{
	public:
		struct init
		{
			std::array<const char *, N> p;
		};
		template<typename... Args>
		object_array_t(core_device_t &dev, init names, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace(i, dev, pstring(names.p[i]), std::forward<Args>(args)...);
		}
	};

	// -----------------------------------------------------------------------------
	// inline implementations
	// -----------------------------------------------------------------------------

	inline netlist_state_t & detail::netlist_ref::state() noexcept
	{
		return m_netlist.nlstate();
	}

	inline const netlist_state_t & detail::netlist_ref::state() const noexcept
	{
		return m_netlist.nlstate();
	}

	template<class C, typename... Args>
	void device_t::create_and_register_subdevice(const pstring &name, unique_pool_ptr<C> &dev, Args&&... args)
	{
		dev = state().make_object<C>(*this, name, std::forward<Args>(args)...);
	}

	template <typename T>
	param_num_t<T>::param_num_t(device_t &device, const pstring &name, const T val)
	: param_t(device, name)
	{
		//m_param = device.setup().get_initial_param_val(this->name(),val);
		bool found = false;
		pstring p = this->get_initial(device, &found);
		if (found)
		{
			plib::pfunction<nl_fptype> func;
			func.compile_infix(p, {});
			auto valx = func.evaluate();
			if (std::is_integral<T>::value)
				if (plib::abs(valx - plib::trunc(valx)) > nlconst::magic(1e-6))
					plib::pthrow<nl_exception>(MF_INVALID_NUMBER_CONVERSION_1_2(device.name() + "." + name, p));
			m_param = static_cast<T>(valx);
		}
		else
			m_param = val;

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename T>
	param_enum_t<T>::param_enum_t(device_t &device, const pstring &name, const T val)
	: param_t(device, name), m_param(val)
	{
		bool found = false;
		pstring p = this->get_initial(device, &found);
		if (found)
		{
			T temp(val);
			bool ok = temp.set_from_string(p);
			if (!ok)
			{
				device.state().log().fatal(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
				plib::pthrow<nl_exception>(MF_INVALID_ENUM_CONVERSION_1_2(name, p));
			}
			m_param = temp;
		}

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename ST, std::size_t AW, std::size_t DW>
	param_rom_t<ST, AW, DW>::param_rom_t(device_t &device, const pstring &name)
	: param_data_t(device, name)
	{
		auto f = stream();
		if (f != nullptr)
		{
			f->read(reinterpret_cast<std::istream::char_type *>(&m_data[0]),1<<AW);
			// FIXME: check for failbit if not in validation.
		}
		else
			device.state().log().warning(MW_ROM_NOT_FOUND(str()));
	}

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

	inline void detail::net_t::push_to_queue(netlist_time delay) noexcept
	{
		if ((num_cons() != 0))
		{
			m_next_scheduled_time = exec().time() + delay;

			if (is_queued())
				exec().qremove(this);

			if (!m_list_active.empty())
			{
				m_in_queue = queue_status::QUEUED;
				exec().qpush(queue_t::entry_t(m_next_scheduled_time, this));
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
		if (m_list_active.empty())
		{
			m_list_active.push_front(&term);
			railterminal().device().do_inc_active();
			if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
			{
				if (m_next_scheduled_time > exec().time())
				{
					m_in_queue = queue_status::QUEUED;     // pending
					exec().qpush(detail::queue_t::entry_t(m_next_scheduled_time, this));
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
		else
		{
			term.set_copied_input(m_cur_Q);
			m_list_active.push_front(&term);
		}
	}

	inline void detail::net_t::remove_from_active_list(core_terminal_t &term) noexcept
	{
		m_list_active.remove(&term);
		if (m_list_active.empty())
			railterminal().device().do_dec_active();
	}

	inline const analog_net_t & analog_t::net() const noexcept
	{
		return static_cast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net() noexcept
	{
		return static_cast<analog_net_t &>(core_terminal_t::net());
	}

	inline nl_fptype terminal_t::operator ()() const noexcept { return net().Q_Analog(); }

	inline void terminal_t::set_ptrs(nl_fptype *gt, nl_fptype *go, nl_fptype *Idr) noexcept(false)
	{
		if (!(gt && go && Idr) && (gt || go || Idr))
		{
			state().log().fatal("Inconsistent nullptrs for terminal {}", name());
			plib::pthrow<nl_exception>("Inconsistent nullptrs for terminal {}", name());
		}
		else
		{
			m_gt1 = gt;
			m_go1 = go;
			m_Idr1 = Idr;
		}
	}

	inline logic_net_t & logic_t::net() noexcept
	{
		return static_cast<logic_net_t &>(core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const noexcept
	{
		return static_cast<const logic_net_t &>(core_terminal_t::net());
	}

	inline netlist_sig_t logic_input_t::Q() const noexcept
	{
		nl_assert(terminal_state() != STATE_INP_PASSIVE);
		//if (net().Q() != m_Q)
		//  printf("term: %s, %d %d TS %d\n", this->name().c_str(), net().Q(), m_Q, terminal_state());
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

	inline void analog_output_t::set_Q(nl_fptype newQ) noexcept
	{
		if (newQ != m_my_net.Q_Analog())
		{
			m_my_net.set_Q_Analog(newQ);
			m_my_net.toggle_and_push_to_queue(netlist_time::quantum());
		}
	}

	inline netlist_state_t &detail::device_object_t::state() noexcept
	{
		return m_device.state();
	}

	inline const netlist_state_t &detail::device_object_t::state() const noexcept
	{
		return m_device.state();
	}

	inline netlist_t &detail::device_object_t::exec() noexcept
	{
		return m_device.exec();
	}

	inline const netlist_t &detail::device_object_t::exec() const noexcept
	{
		return m_device.exec();
	}

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring &name, const T &value)
	: m_value(value)
	{
		owner.state().save(owner, m_value, owner.name(), name);
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		const state_container<C>::value_type & value)
	{
		owner.state().save(owner, *static_cast<C *>(this), owner.name(), name);
		for (std::size_t i=0; i < this->size(); i++)
			(*this)[i] = value;
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		std::size_t n, const state_container<C>::value_type & value)
	: C(n, value)
	{
		owner.state().save(owner, *static_cast<C *>(this), owner.name(), name);
	}

	// -----------------------------------------------------------------------------
	// Hot section
	//
	// Any changes below will impact performance.
	// -----------------------------------------------------------------------------

	template <bool KEEP_STATS, typename T>
	inline void detail::net_t::process(const T mask, netlist_sig_t sig) noexcept
	{
		m_cur_Q = sig;

		if (KEEP_STATS)
		{
			for (auto & p : m_list_active)
			{
				p.set_copied_input(sig);
				auto *stats = p.device().m_stats.get();
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
				if ((p.terminal_state() & mask))
					p.run_delegate();
			}
		}
	}

	template <bool KEEP_STATS>
	inline void detail::net_t::update_devs() noexcept
	{
		nl_assert(this->isRailNet());

		m_in_queue = queue_status::DELIVERED; // mark as taken ...
		if (m_new_Q ^ m_cur_Q)
			process<KEEP_STATS>((m_new_Q << core_terminal_t::INP_LH_SHIFT)
				| (m_cur_Q << core_terminal_t::INP_HL_SHIFT), m_new_Q);
	}

	template <bool KEEP_STATS>
	inline void netlist_t::process_queue_stats(const netlist_time delta) noexcept
	{
		netlist_time stop(m_time + delta);

		qpush(detail::queue_t::entry_t(stop, nullptr));

		if (m_mainclock == nullptr)
		{
			detail::queue_t::entry_t e(m_queue.pop());
			m_time = e.exec_time();
			while (e.object() != nullptr)
			{
				e.object()->template update_devs<KEEP_STATS>();
				if (KEEP_STATS)
					m_perf_out_processed.inc();
				e = m_queue.pop();
				m_time = e.exec_time();
			}
		}
		else
		{
			logic_net_t &mc_net(m_mainclock->m_Q.net());
			const netlist_time inc(m_mainclock->m_inc);
			netlist_time mc_time(mc_net.next_scheduled_time());

			do
			{
				while (m_queue.top().exec_time() > mc_time)
				{
					m_time = mc_time;
					mc_net.toggle_new_Q();
					mc_net.update_devs<KEEP_STATS>();
					mc_time += inc;
				}

				detail::queue_t::entry_t e(m_queue.pop());
				m_time = e.exec_time();
				if (e.object() != nullptr)
				{
					e.object()->template update_devs<KEEP_STATS>();
					if (KEEP_STATS)
						m_perf_out_processed.inc();
				}
				else
					break;
			} while (true); //while (e.m_object != nullptr);
			mc_net.set_next_scheduled_time(mc_time);
		}
	}

	inline void netlist_t::process_queue(netlist_time delta) noexcept
	{
		if (!m_use_stats)
			process_queue_stats<false>(delta);
		else
		{
			auto sm_guard(m_stat_mainloop.guard());
			process_queue_stats<true>(delta);
		}
	}

} // namespace netlist

namespace plib
{
	template<typename X>
	struct ptype_traits<netlist::state_var<X>> : ptype_traits<X>
	{
	};
} // namespace plib



#endif // NLBASE_H_
