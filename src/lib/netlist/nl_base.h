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
#include "plib/ptimed_queue.h"
#include "plib/ptypes.h"

#include "nl_errstr.h"
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
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
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
	public: virtual void timestep(nl_fptype step)  noexcept override

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

#define NETLIB_DELEGATE(name) nldelegate(&this_type :: name, this)

#define NETLIB_UPDATE_TERMINALSI() virtual void update_terminals() noexcept override
#define NETLIB_HANDLERI(name) void name() noexcept
#define NETLIB_UPDATEI() virtual void update() noexcept override
#define NETLIB_UPDATE_PARAMI() virtual void update_param() noexcept override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUB_UPTR(ns, chip) device_arena::unique_ptr< ns :: nld_ ## chip >

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
	using nldelegate_ts = plib::pmfp<void, nl_fptype>;
	using nldelegate_dyn = plib::pmfp<void>;

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
	class nlparse_t;
	class netlist_t;
	class netlist_state_t;
	class core_device_t;
	class device_t;

	template <class CX>
	class delegator_t : public CX
	{
	protected:
		using base_type = delegator_t<CX>;
		using delegated_type = CX;
		using delegated_type::delegated_type;
	};

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
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, modernize-use-equals-default)
		logic_family_desc_t()
		{
		}


		PCOPYASSIGNMOVE(logic_family_desc_t, delete)

		virtual ~logic_family_desc_t() noexcept = default;

		virtual device_arena::unique_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_state_t &anetlist, const pstring &name,
				const logic_output_t *proxied) const = 0;
		virtual device_arena::unique_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_state_t &anetlist, const pstring &name,
				const logic_input_t *proxied) const = 0;

		nl_fptype low_thresh_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_low_thresh_PCNT; }
		nl_fptype high_thresh_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_high_thresh_PCNT; }
		nl_fptype low_offset_V() const noexcept{ return m_low_VO; }
		nl_fptype high_offset_V() const noexcept{ return m_high_VO; }
		nl_fptype R_low() const noexcept{ return m_R_low; }
		nl_fptype R_high() const noexcept{ return m_R_high; }

		bool is_above_high_thresh_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return V > high_thresh_V(VN, VP); }

		bool is_below_low_thresh_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return V < low_thresh_V(VN, VP); }

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
		logic_family_t(const logic_family_desc_t *d) : m_logic_family(d) {}
		PCOPYASSIGNMOVE(logic_family_t, delete)

		const logic_family_desc_t *logic_family() const noexcept { return m_logic_family; }
		void set_logic_family(const logic_family_desc_t *fam) noexcept { m_logic_family = fam; }

	protected:
		~logic_family_t() noexcept = default; // prohibit polymorphic destruction
	private:
		const logic_family_desc_t *m_logic_family;
	};

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

		using value_type = T;

		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name,    //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);

		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name     //!< identifier/name for this state variable
		);

		PMOVEASSIGN(state_var, delete)

		//! Destructor.
		~state_var() noexcept = default;

		//! Copy Constructor removed.
		constexpr state_var(const state_var &rhs) = delete;
		//! Assignment operator to assign value of a state var.
		constexpr state_var &operator=(const state_var &rhs) noexcept { m_value = rhs.m_value; return *this; } // OSX doesn't like noexcept
		//! Assignment operator to assign value of type T.
		constexpr state_var &operator=(const T &rhs) noexcept { m_value = rhs; return *this; }
		//! Assignment move operator to assign value of type T.
		constexpr state_var &operator=(T &&rhs) noexcept { std::swap(m_value, rhs); return *this; }
		//! Return non-const value of state variable.
		constexpr operator T & () noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr operator const T & () const noexcept { return m_value; }
		//! Return non-const value of state variable.
		constexpr T & var() noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr const T & var() const noexcept { return m_value; }
		//! Return non-const value of state variable.
		constexpr T & operator ()() noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr const T & operator ()() const noexcept { return m_value; }
		//! Access state variable by ->.
		constexpr T * operator->() noexcept { return &m_value; }
		//! Access state variable by const ->.
		constexpr const T * operator->() const noexcept{ return &m_value; }

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
			using value_type = T;
			using key_type = const C *;
			static void add(key_type obj, const value_type &aname) noexcept
			{
				store().insert({obj, aname});
			}

			static value_type *get(key_type obj) noexcept
			{
				try
				{
					auto ret(store().find(obj));
					if (ret == store().end())
						return nullptr;
					return &ret->second;
				}
				catch (...)
				{
					plib::terminate("exception in property_store_t.get()");
					return static_cast<value_type *>(nullptr);
				}
			}

			static void remove(key_type obj) noexcept
			{
				store().erase(store().find(obj));
			}

			static std::unordered_map<key_type, value_type> &store() noexcept
			{
				static std::unordered_map<key_type, value_type> lstore;
				return lstore;
			}

		};

		// -----------------------------------------------------------------------------
		// object_t
		// -----------------------------------------------------------------------------

		/// \brief The base class for netlist devices, terminals and parameters.
		///
		///  This class serves as the base class for all device, terminal and
		///  objects.

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

			PCOPYASSIGNMOVE(object_t, delete)
			/// \brief return name of the object
			///
			/// \returns name of the object.

			const pstring &name() const noexcept
			{
				return *props::get(this);
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

		/// \brief Base class for all objects bejng owned by a netlist
		///
		/// The object provides adds \ref netlist_state_t and \ref netlist_t
		/// accessors.
		///
		class netlist_object_t : public object_t
		{
		public:
			explicit netlist_object_t(netlist_t &nl, const pstring &name)
			: object_t(name)
			, m_netlist(nl)
			{ }

			~netlist_object_t() = default;

			PCOPYASSIGNMOVE(netlist_object_t, delete)

			netlist_state_t & state() noexcept;
			const netlist_state_t & state() const noexcept;

			netlist_t & exec() noexcept { return m_netlist; }
			const netlist_t & exec() const noexcept { return m_netlist; }

			// to ease template design
			template<typename T, typename... Args>
			device_arena::unique_ptr<T> make_pool_object(Args&&... args);

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
		/// The class also supports device-less objects. In this case,
		/// nullptr is passed in as the device object.
		///

		class device_object_t : public object_t
		{
		public:
			/// \brief Constructor.
			///
			/// \param dev  pointer to device owning the object.
			/// \param name string holding the name of the device

			device_object_t(core_device_t *dev, const pstring &name);

			/// \brief returns reference to owning device.
			/// \returns reference to owning device.

			core_device_t &device() noexcept { return *m_device; }
			const core_device_t &device() const noexcept { return *m_device; }

			/// \brief The netlist owning the owner of this object.
			/// \returns reference to netlist object.

			netlist_state_t &state() noexcept;
			const netlist_state_t &state() const noexcept;

			netlist_t &exec() noexcept;
			const netlist_t &exec() const noexcept;

		private:
			core_device_t * m_device;
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
			/// \brief Number of signal bits
			///
			/// Going forward setting this to 8 will allow 8-bit signal
			/// busses to be used in netlist, e.g. for more complex memory
			/// arrangements.
			/// Mimimum value is 2 here to support tristate output on proxies.
			static constexpr const unsigned int INP_BITS = 2;

			static constexpr const unsigned int INP_MASK = (1 << INP_BITS) - 1;
			static constexpr const unsigned int INP_HL_SHIFT = 0;
			static constexpr const unsigned int INP_LH_SHIFT = INP_BITS;

			static constexpr netlist_sig_t OUT_TRISTATE() { return INP_MASK; }

			static_assert(INP_BITS * 2 <= sizeof(netlist_sig_t) * 8, "netlist_sig_t size not sufficient");

			enum state_e {
				STATE_INP_PASSIVE = 0,
				STATE_INP_HL      = (INP_MASK << INP_HL_SHIFT),
				STATE_INP_LH      = (INP_MASK << INP_LH_SHIFT),
				STATE_INP_ACTIVE  = STATE_INP_HL | STATE_INP_LH,
				STATE_OUT         = (1 << (2*INP_BITS)),
				STATE_BIDIR       = (1 << (2*INP_BITS + 1))
			};

			core_terminal_t(core_device_t &dev, const pstring &aname,
					state_e state, nldelegate delegate = nldelegate());
			virtual ~core_terminal_t() noexcept = default;

			PCOPYASSIGNMOVE(core_terminal_t, delete)

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

			net_t & net() const noexcept { return *m_net;}

			bool is_logic() const noexcept;
			bool is_logic_input() const noexcept;
			bool is_logic_output() const noexcept;
			bool is_tristate_output() const noexcept;
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
			void set_copied_input(netlist_sig_t val) const noexcept { plib::unused_var(val); } // NOLINT: static means more message elsewhere
	#endif

			void set_delegate(const nldelegate &delegate) noexcept { m_delegate = delegate; }
			const nldelegate &delegate() const noexcept { return m_delegate; }
			inline void run_delegate() noexcept { return m_delegate(); }
		private:
			nldelegate m_delegate;
			net_t * m_net;
			state_var<state_e> m_state;
		};

		// -----------------------------------------------------------------------------
		// net_t
		// -----------------------------------------------------------------------------

		class net_t : public netlist_object_t
		{
		public:

			enum class queue_status
			{
				DELAYED_DUE_TO_INACTIVE = 0,
				QUEUED,
				DELIVERED
			};

			net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *railterminal = nullptr);

			PCOPYASSIGNMOVE(net_t, delete)

			virtual ~net_t() noexcept = default;

			void reset() noexcept;

			void toggle_new_Q() noexcept { m_new_Q = (m_cur_Q ^ 1);   }

			void toggle_and_push_to_queue(const netlist_time &delay) noexcept
			{
				toggle_new_Q();
				push_to_queue(delay);
			}

			void push_to_queue(const netlist_time &delay) noexcept;
			NVCC_CONSTEXPR bool is_queued() const noexcept { return m_in_queue == queue_status::QUEUED; }

			template <bool KEEP_STATS>
			inline void update_devs() noexcept;

			netlist_time_ext next_scheduled_time() const noexcept { return m_next_scheduled_time; }
			void set_next_scheduled_time(netlist_time_ext ntime) noexcept { m_next_scheduled_time = ntime; }

			NVCC_CONSTEXPR bool is_rail_net() const noexcept { return !(m_railterminal == nullptr); }
			core_terminal_t & railterminal() const noexcept { return *m_railterminal; }

			bool has_connections() const noexcept { return !m_core_terms.empty(); }

			void add_to_active_list(core_terminal_t &term) noexcept;
			void remove_from_active_list(core_terminal_t &term) noexcept;

			// setup stuff

			bool is_logic() const noexcept;
			bool is_analog() const noexcept;

			void rebuild_list();     // rebuild m_list after a load

			std::vector<core_terminal_t *> &core_terms() noexcept { return m_core_terms; }

			void update_inputs() noexcept
			{
#if NL_USE_COPY_INSTEAD_OF_REFERENCE
				for (auto & term : m_core_terms)
					term->m_Q = m_cur_Q;
#endif
				// nothing needs to be done if define not set
			}

		protected:

			// only used for logic nets
			NVCC_CONSTEXPR netlist_sig_t Q() const noexcept { return m_cur_Q; }

			// only used for logic nets
			void initial(netlist_sig_t val) noexcept
			{
				m_cur_Q = m_new_Q = val;
				update_inputs();
			}

			// only used for logic nets
			inline void set_Q_and_push(const netlist_sig_t &newQ, const netlist_time &delay) noexcept
			{
				if (newQ != m_new_Q)
				{
					m_new_Q = newQ;
					push_to_queue(delay);
				}
			}

			// only used for logic nets
			inline void set_Q_time(const netlist_sig_t &newQ, const netlist_time_ext &at) noexcept
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

		private:
			state_var<netlist_sig_t>     m_new_Q;
			state_var<netlist_sig_t>     m_cur_Q;
			state_var<queue_status>      m_in_queue;
			state_var<netlist_time_ext>  m_next_scheduled_time;

			core_terminal_t * m_railterminal;
			plib::linkedlist_t<core_terminal_t> m_list_active;
			std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

			template <bool KEEP_STATS, typename T, typename S>
			void process(T mask, const S &sig) noexcept;
		};
	} // namespace detail

	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public detail::core_terminal_t
	{
	public:

		analog_t(core_device_t &dev, const pstring &aname, state_e state,
			nldelegate delegate = nldelegate());

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
		terminal_t(core_device_t &dev, const pstring &aname, terminal_t *otherterm);

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

		/// @brief Solve the system this terminal is connected to.
		///
		/// \note deprecated - will be removed
		void solve_now() const; // FIXME: remove this

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
				state_e terminal_state, nldelegate delegate = nldelegate());

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
				nldelegate delegate = nldelegate());

#if 0
		template <class D>
		logic_input_t(D &dev, const pstring &aname);
#endif
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
	};

	class analog_net_t : public detail::net_t
	{
	public:

		using list_t =  plib::aligned_vector<analog_net_t *>;

		analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *railterminal = nullptr);

		nl_fptype Q_Analog() const noexcept { return m_cur_Analog; }
		void set_Q_Analog(nl_fptype v) noexcept { m_cur_Analog = v; }
		// used by solver code ...
		nl_fptype *Q_Analog_state_ptr() noexcept { return &m_cur_Analog(); }

		//FIXME: needed by current solver code
		solver::matrix_solver_t *solver() const noexcept { return m_solver; }
		void set_solver(solver::matrix_solver_t *solver) noexcept { m_solver = solver; }

		friend constexpr bool operator==(const analog_net_t &lhs, const analog_net_t &rhs) noexcept
		{
			return &lhs == &rhs;
		}

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

	/// @brief Base class for all device parameters
	///
	/// All device parameters classes derive from this object.
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

		//deviceless, it's the responsibility of the owner to register!
		param_t(const pstring &name);

		param_t(core_device_t &device, const pstring &name);

		PCOPYASSIGNMOVE(param_t, delete)
		virtual ~param_t() noexcept;

		param_type_t param_type() const noexcept(false);

		virtual pstring valstr() const = 0;

	protected:

		void update_param() noexcept;

		pstring get_initial(const core_device_t *dev, bool *found) const;

		template<typename C>
		void set_and_update_param(C &p, const C v) noexcept
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
		using value_type = T;

		param_num_t(core_device_t &device, const pstring &name, T val) noexcept(false);

		T operator()() const noexcept { return m_param; }
		operator T() const noexcept { return m_param; }

		void set(const T &param) noexcept { set_and_update_param(m_param, param); }

		pstring valstr() const override
		{
			return plib::pfmt("{}").e(gsl::narrow<nl_fptype>(m_param));
		}

	private:
		T m_param;
	};

	template <typename T>
	class param_enum_t final: public param_t
	{
	public:
		using value_type = T;

		param_enum_t(core_device_t &device, const pstring &name, T val) noexcept(false);

		T operator()() const noexcept { return T(m_param); }
		operator T() const noexcept { return T(m_param); }
		void set(const T &param) noexcept { set_and_update_param(m_param, static_cast<int>(param)); }

		pstring valstr() const override
		{
			// returns the numerical value
			return plib::pfmt("{}")(m_param);
		}
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

	// FIXME: not a core component -> legacy
	class param_ptr_t final: public param_t
	{
	public:
		param_ptr_t(core_device_t &device, const pstring &name, std::uint8_t* val);
		std::uint8_t * operator()() const noexcept { return m_param; }
		void set(std::uint8_t *param) noexcept { set_and_update_param(m_param, param); }

		pstring valstr() const override
		{
			// returns something which errors
			return pstring("PTRERROR");
		}

	private:
		std::uint8_t* m_param;
	};

	// -----------------------------------------------------------------------------
	// string parameter
	// -----------------------------------------------------------------------------

	class param_str_t : public param_t
	{
	public:
		param_str_t(core_device_t &device, const pstring &name, const pstring &val);
		param_str_t(netlist_state_t &state, const pstring &name, const pstring &val);

		pstring operator()() const noexcept { return str(); }
		void set(const pstring &param)
		{
			if (*m_param != param)
			{
				*m_param = param;
				changed();
				update_param();
			}
		}
		pstring valstr() const override
		{
			return *m_param;
		}
	protected:
		virtual void changed() noexcept;
		pstring str() const noexcept { pstring ret = *m_param; return ret;}
	private:
		host_arena::unique_ptr<pstring> m_param;
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
			template <typename P, typename Y=T, typename DUMMY = std::enable_if_t<plib::is_arithmetic<Y>::value>>
			value_base_t(P &param, const pstring &name)
			: m_value(gsl::narrow<T>(param.value(name)))
			{
			}
			template <typename P, typename Y=T, std::enable_if_t<!plib::is_arithmetic<Y>::value, int> = 0>
			value_base_t(P &param, const pstring &name)
			: m_value(static_cast<T>(param.value_str(name)))
			{
			}
			T operator()() const noexcept { return m_value; }
			operator T() const noexcept { return m_value; }
		private:
			const T m_value;
		};

		using value_t = value_base_t<nl_fptype>;
		using value_str_t = value_base_t<pstring>;

		param_model_t(core_device_t &device, const pstring &name, const pstring &val)
		: param_str_t(device, name, val)
		{
		}

		pstring value_str(const pstring &entity);
		nl_fptype value(const pstring &entity);
		pstring type();
		// hide this
		void set(const pstring &param) = delete;
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
		param_data_t(core_device_t &device, const pstring &name)
		: param_str_t(device, name, "")
		{
		}

		std::unique_ptr<std::istream> stream();
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

		param_rom_t(core_device_t &device, const pstring &name);

		const ST & operator[] (std::size_t n) const noexcept { return m_data[n]; }
	protected:
		void changed() noexcept override
		{
			stream()->read(reinterpret_cast<std::istream::char_type *>(&m_data[0]),1<<AW);
		}

	private:
		std::array<ST, 1 << AW> m_data;
	};


	// -----------------------------------------------------------------------------
	// core_device_t
	// -----------------------------------------------------------------------------

	class core_device_t : public detail::netlist_object_t
	{
	public:
		core_device_t(netlist_state_t &owner, const pstring &name);
		core_device_t(core_device_t &owner, const pstring &name);

		PCOPYASSIGNMOVE(core_device_t, delete)

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

		stats_t * stats() const noexcept { return m_stats.get(); }

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
		device_arena::unique_ptr<stats_t> m_stats;
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

		NETLIB_UPDATEI() { }
		NETLIB_UPDATE_TERMINALSI() { }

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

		//nldelegate default_delegate() { return nldelegate(&device_t::update, this); }
		nldelegate default_delegate() { return { &device_t::update, this }; }
	protected:

		NETLIB_UPDATEI() { }
		NETLIB_UPDATE_TERMINALSI() { }

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

		class queue_t :
				//public timed_queue<pqentry_t<net_t *, netlist_time>, false, NL_KEEP_STATISTICS>,
				public timed_queue<plib::pqentry_t<netlist_time_ext, net_t *>, false>,
				public netlist_object_t,
				public plib::state_manager_t::callback_t
		{
		public:
			using entry_t = plib::pqentry_t<netlist_time_ext, net_t *>;
			using base_queue = timed_queue<entry_t, false>;
			explicit queue_t(netlist_t &nl, const pstring &name);
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
			std::vector<netlist_time_ext::internal_type> m_times;
			std::vector<std::size_t> m_net_ids;
		};

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

		void reset();

	private:

		device_arena                               m_pool; // must be deleted last!

		device_arena::unique_ptr<netlist_t>        m_netlist;
		host_arena::unique_ptr<plib::dynlib_base>  m_lib;
		plib::state_manager_t               m_state;
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

			NETLIB_RESETI()
			{
				m_Q.net().set_next_scheduled_time(netlist_time_ext::zero());
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

		bool stats_enabled() const noexcept { return m_use_stats; }
		void enable_stats(bool val) noexcept { m_use_stats = val; }

	private:

		template <bool KEEP_STATS>
		void process_queue_stats(netlist_time_ext delta) noexcept;

		netlist_state_t &                   m_state;
		devices::NETLIB_NAME(solver) *      m_solver;

		// mostly rw
		PALIGNAS_CACHELINE()
		netlist_time_ext                    m_time;
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

	template<class C, std::size_t N>
	class object_array_base_t : public plib::uninitialised_array_t<C, N>
	{
	public:
		template<class D, typename... Args>
		object_array_base_t(D &dev, const std::initializer_list<const char *> &names, Args&&... args)
		{
			passert_always_msg(names.size() == N, "initializer_list size mismatch");
			std::size_t i = 0;
			for (const auto &n : names)
				this->emplace(i++, dev, pstring(n), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, const pstring &fmt)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace(i, dev, formatted(fmt, i));
		}

		template<class D, typename... Args>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, Args&&... args)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace(i, dev, formatted(fmt, i+offset), std::forward<Args>(args)...);
		}

		template<class D>
		object_array_base_t(D &dev, std::size_t offset, const pstring &fmt, nldelegate delegate)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace(i, dev, formatted(fmt, i+offset), delegate);
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
			passert_always_msg(delegates.size() >= N, "initializer_list size mismatch");
			std::size_t i = 0;
			for (auto &e : delegates)
			{
				if (i < N)
				{
					pstring name(this->formatted(fmt, i+offset));
					if ((qmask >> i) & 1)
						name += "Q";
					this->emplace(i, dev, name, e);
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

		explicit nld_power_pins(device_t &owner, const pstring &sVCC = sPowerVCC,
			const pstring &sGND = sPowerGND)
		: m_VCC(owner, sVCC, NETLIB_DELEGATE(noop))
		, m_GND(owner, sGND, NETLIB_DELEGATE(noop))
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

	// -----------------------------------------------------------------------------
	// Hot section
	//
	// Any changes below will impact performance.
	// -----------------------------------------------------------------------------

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

#if 0
	template <class D>
	logic_input_t::logic_input_t(D &dev, const pstring &aname)
			: logic_input_t(dev, aname, STATE_INP_ACTIVE, nldelegate(&D :: update, &dev))
	{
	}
#endif
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

	inline netlist_state_t & detail::netlist_object_t::state() noexcept
	{
		return m_netlist.nlstate();
	}

	inline const netlist_state_t & detail::netlist_object_t::state() const noexcept
	{
		return m_netlist.nlstate();
	}

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
			f->read(reinterpret_cast<std::istream::char_type *>(&m_data[0]),1<<AW);
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

	inline netlist_state_t &detail::device_object_t::state() noexcept
	{
		return m_device->state();
	}

	inline const netlist_state_t &detail::device_object_t::state() const noexcept
	{
		return m_device->state();
	}

	inline solver::matrix_solver_t *analog_t::solver() const noexcept
	{
		return (this->has_net() ? net().solver() : nullptr);
	}

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring &name, const T &value)
	: m_value(value)
	{
		owner.state().save(owner, m_value, owner.name(), name);
	}

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring &name)
	{
		owner.state().save(owner, m_value, owner.name(), name);
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		const state_container<C>::value_type & value)
	{
		owner.state().save(owner, static_cast<C &>(*this), owner.name(), name);
		for (std::size_t i=0; i < this->size(); i++)
			(*this)[i] = value;
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		std::size_t n, const state_container<C>::value_type & value)
	: C(n, value)
	{
		owner.state().save(owner, static_cast<C &>(*this), owner.name(), name);
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
