// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file nl_base.h
 *
 */

#ifndef NLBASE_H_
#define NLBASE_H_

#ifdef NL_PROHIBIT_BASEH_INCLUDE
#error "nl_base.h included. Please correct."
#endif

#include <unordered_map>
#include <vector>

#include "plib/palloc.h" // owned_ptr
#include "plib/pdynlib.h"
#include "plib/pfmtlog.h"
#include "plib/pmempool.h"
#include "plib/ppmf.h"
#include "plib/pstate.h"
#include "plib/pstream.h"

#include "nltypes.h"
#include "nl_errstr.h"
#include "nl_lists.h"
#include "plib/ptime.h"

//============================================================
//  MACROS / New Syntax
//============================================================

/*! Construct a netlist device name */
#define NETLIB_NAME(chip) nld_ ## chip

#define NETLIB_OBJECT_DERIVED(name, pclass)                                   \
class NETLIB_NAME(name) : public NETLIB_NAME(pclass)

/*! Start a netlist device class.
 *  Used to start defining a netlist device class.
 *  The simplest device without inputs or outputs would look like this:
 *
 *      NETLIB_OBJECT(base_dummy)
 *      {
 *      public:
 *          NETLIB_CONSTRUCTOR(base_dummy) { }
 *      };
 *
 *  Also refer to #NETLIB_CONSTRUCTOR.
 */
#define NETLIB_OBJECT(name)                                                    \
class NETLIB_NAME(name) : public device_t

#define NETLIB_CONSTRUCTOR_DERIVED(cname, pclass)                              \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
	: NETLIB_NAME(pclass)(owner, name)

/*! Used to define the constructor of a netlist device.
 *  Use this to define the constructor of a netlist device. Please refer to
 *  #NETLIB_OBJECT for an example.
 */
#define NETLIB_CONSTRUCTOR(cname)                                              \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name) \
		: device_t(owner, name)

	/*! Used to define the destructor of a netlist device.
	*  The use of a destructor for netlist device should normally not be necessary.
	*/
#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)() noexcept

	/*! Define an extended constructor and add further parameters to it.
	*  The macro allows to add further parameters to a device constructor. This is
	*  normally used for sub-devices and system devices only.
	*/
#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                      \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring &name, __VA_ARGS__) \
		: device_t(owner, name)

	/*! Add this to a device definition to mark the device as dynamic.
	 *
	 *  If NETLIB_IS_DYNAMIC(true) is added to the device definition the device
	 *  is treated as an analog dynamic device, i.e. #NETLIB_UPDATE_TERMINALSI
	 *  is called on a each step of the Newton-Raphson step
	 *  of solving the linear equations.
	 *
	 *  You may also use e.g. NETLIB_IS_DYNAMIC(m_func() != "") to only make the
	 *  device a dynamic device if parameter m_func is set.
	 */
#define NETLIB_IS_DYNAMIC(expr)                                                \
	public: virtual bool is_dynamic() const override { return expr; }

	/*! Add this to a device definition to mark the device as a time-stepping device.
	 *
	 *  You have to implement NETLIB_TIMESTEP in this case as well. Currently, only
	 *  the capacitor and inductor devices uses this.
	 *
	 *  You may also use e.g. NETLIB_IS_TIMESTEP(m_func() != "") to only make the
	 *  device a dynamic device if parameter m_func is set. This is used by the
	 *  Voltage Source element.
	 *
	 *  Example:
	 *
	 *   NETLIB_TIMESTEP_IS_TIMESTEP()
	 *   NETLIB_TIMESTEPI()
	 *       {
	 *           // Gpar should support convergence
	 *           const nl_double G = m_C.Value() / step +  m_GParallel;
	 *           const nl_double I = -G * deltaV();
	 *           set(G, 0.0, I);
	 *       }
	 *
	 */
#define NETLIB_IS_TIMESTEP(expr)                                               \
	public: virtual bool is_timestep() const override { return expr; }

	/*! Used to implement the time stepping code.
	 *
	 * Please see NETLIB_IS_TIMESTEP for an example.
	 */
#define NETLIB_TIMESTEPI()                                                     \
	public: virtual void timestep(const nl_double step) override

#define NETLIB_FAMILY(family) , m_famsetter(*this, family)

#define NETLIB_DELEGATE(chip, name) nldelegate(&NETLIB_NAME(chip) :: name, this)

#define NETLIB_UPDATE_TERMINALSI() virtual void update_terminals() override
#define NETLIB_HANDLERI(name) virtual void name() NL_NOEXCEPT
#define NETLIB_UPDATEI() virtual void update() NL_NOEXCEPT override
#define NETLIB_UPDATE_PARAMI() virtual void update_param() override
#define NETLIB_RESETI() virtual void reset() override

#define NETLIB_TIMESTEP(chip) void NETLIB_NAME(chip) :: timestep(const nl_double step)

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUBXX(ns, chip) poolptr< ns :: nld_ ## chip >

#define NETLIB_HANDLER(chip, name) void NETLIB_NAME(chip) :: name() NL_NOEXCEPT
#define NETLIB_UPDATE(chip) NETLIB_HANDLER(chip, update)

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param()
#define NETLIB_FUNC_VOID(chip, name, params) void NETLIB_NAME(chip) :: name params

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals()

//============================================================
//  Asserts
//============================================================

#if defined(MAME_DEBUG)
#define nl_assert(x)    do { if (1) if (!(x)) throw nl_exception(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#define NL_NOEXCEPT
#else
#define nl_assert(x)    do { if (0) if (!(x)) { /*throw nl_exception(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); */} } while (0)
#define NL_NOEXCEPT     noexcept
#endif
#define nl_assert_always(x, msg)    do { if (!(x)) throw nl_exception(plib::pfmt("Fatal error: {1}\nCaused by assert: {2}:{3}: {4}")(msg)(__FILE__)(__LINE__)(#x)); } while (0)

//============================================================
// Namespace starts
//============================================================

namespace netlist
{
	/*! Delegate type for device notification.
	 *
	 */
	using nldelegate = plib::pmfp<void>;

	// -----------------------------------------------------------------------------
	// forward definitions
	// -----------------------------------------------------------------------------

	namespace devices
	{
		class matrix_solver_t;
		class NETLIB_NAME(gnd);
		class NETLIB_NAME(solver);
		class NETLIB_NAME(mainclock);
		class NETLIB_NAME(netlistparams);
		class NETLIB_NAME(base_proxy);
		class NETLIB_NAME(base_d_to_a_proxy);
		class NETLIB_NAME(base_a_to_d_proxy);
	} // namespace devices

	namespace detail {
		class object_t;
		class device_object_t;
		struct netlist_ref;
		class core_terminal_t;
		struct family_setter_t;
		class queue_t;
		class net_t;
	} // namespace detail

	class logic_output_t;
	class logic_input_t;
	class analog_net_t;
	class logic_net_t;
	class setup_t;
	class netlist_t;
	class netlist_state_t;
	class core_device_t;
	class device_t;
	class callbacks_t;

	//============================================================
	//  Exceptions
	//============================================================

	/*! Generic netlist exception.
	 *  The exception is used in all events which are considered fatal.
	 */
	class nl_exception : public plib::pexception
	{
	public:
		/*! Constructor.
		 *  Allows a descriptive text to be assed to the exception
		 */
		explicit nl_exception(const pstring &text //!< text to be passed
				)
		: plib::pexception(text) { }
	};

	/*! Logic families descriptors are used to create proxy devices.
	 *  The logic family describes the analog capabilities of logic devices,
	 *  inputs and outputs.
	 */
	class logic_family_desc_t
	{
	public:
		logic_family_desc_t();

		COPYASSIGNMOVE(logic_family_desc_t, delete)

		virtual ~logic_family_desc_t() noexcept = default;

		virtual poolptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_output_t *proxied) const = 0;
		virtual poolptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_state_t &anetlist, const pstring &name,
				logic_input_t *proxied) const = 0;

		double fixed_V() const { return m_fixed_V; }
		double low_thresh_V(const double VN, const double VP) const { return VN + (VP - VN) * m_low_thresh_PCNT; }
		double high_thresh_V(const double VN, const double VP) const { return VN + (VP - VN) * m_high_thresh_PCNT; }
		double low_V(const double VN, const double VP) const { plib::unused_var(VP); return VN + m_low_VO; }
		double high_V(const double VN, const double VP) const { plib::unused_var(VN);  return VP - m_high_VO; }
		double R_low() const { return m_R_low; }
		double R_high() const { return m_R_high; }

		double m_fixed_V;           //!< For variable voltage families, specify 0. For TTL this would be 5. */
		double m_low_thresh_PCNT;   //!< low input threshhold offset. If the input voltage is below this value times supply voltage, a "0" input is signalled
		double m_high_thresh_PCNT;  //!< high input threshhold offset. If the input voltage is above the value times supply voltage, a "0" input is signalled
		double m_low_VO;            //!< low output voltage offset. This voltage is output if the ouput is "0"
		double m_high_VO;           //!< high output voltage offset. The supply voltage minus this offset is output if the ouput is "1"
		double m_R_low;             //!< low output resistance. Value of series resistor used for low output
		double m_R_high;            //!< high output resistance. Value of series resistor used for high output
	};

	/*! Base class for devices, terminals, outputs and inputs which support
	 *  logic families.
	 *  This class is a storage container to store the logic family for a
	 *  netlist object. You will not directly use it. Please refer to
	 *  #NETLIB_FAMILY to learn how to define a logic family for a device.
	 *
	 * All terminals inherit the family description from the device
	 * The default is the ttl family, but any device can override the family.
	 * For individual terminals, the family can be overwritten as well.
	 *
	 */
	class logic_family_t
	{
	public:

		logic_family_t() : m_logic_family(nullptr) {}
		COPYASSIGNMOVE(logic_family_t, delete)


		const logic_family_desc_t *logic_family() const { return m_logic_family; }
		void set_logic_family(const logic_family_desc_t *fam) { m_logic_family = fam; }

	protected:
		~logic_family_t() noexcept = default; // prohibit polymorphic destruction
		const logic_family_desc_t *m_logic_family;
	};

	const logic_family_desc_t *family_TTL();        //*!< logic family for TTL devices.
	const logic_family_desc_t *family_CD4XXX();     //*!< logic family for CD4XXX CMOS devices.

	/*! A persistent variable template.
	 *  Use the state_var template to define a variable whose value is saved.
	 *  Within a device definition use
	 *
	 *      NETLIB_OBJECT(abc)
	 *      {
	 *          NETLIB_CONSTRUCTOR(abc)
	 *          , m_var(*this, "myvar", 0)
	 *          ...
	 *          state_var<unsigned> m_var;
	 *      }
	 */

	template <typename T>
	struct state_var
	{
	public:
		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name,     //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);

		//! Destructor.
		~state_var() noexcept = default;
		//! Copy Constructor.
		constexpr state_var(const state_var &rhs) = default;
		//! Move Constructor.
		constexpr state_var(state_var &&rhs) noexcept = default;
		//! Assignment operator to assign value of a state var.
		C14CONSTEXPR state_var &operator=(const state_var &rhs) = default;
		//! Assignment move operator to assign value of a state var.
		C14CONSTEXPR state_var &operator=(state_var &&rhs) noexcept = default;
		//! Assignment operator to assign value of type T.
		C14CONSTEXPR state_var &operator=(const T &rhs) noexcept { m_value = rhs; return *this; }
		//! Assignment move operator to assign value of type T.
		C14CONSTEXPR state_var &operator=(T &&rhs) noexcept { std::swap(m_value, rhs); return *this; }
		//! Return value of state variable.
		C14CONSTEXPR operator T & () noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr operator const T & () const noexcept { return m_value; }
		C14CONSTEXPR T * ptr() noexcept { return &m_value; }
		constexpr const T * ptr() const noexcept{ return &m_value; }

	private:
		T m_value;
	};

	/*! A persistent array template.
	 *  Use this state_var template to define an array whose contents are saved.
	 *  Please refer to \ref state_var.
	 */
	template <typename T, std::size_t N>
	struct state_array
	{
	public:
		//! Constructor.
		template <typename O>
		state_array(O &owner,             //!< owner must have a netlist() method.
				const pstring &name,     //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);
		//! Copy Constructor.
		state_array(const state_array &rhs) NL_NOEXCEPT = default;
		//! Destructor.
		~state_array() noexcept = default;
		//! Move Constructor.
		state_array(state_array &&rhs) NL_NOEXCEPT = default;
		state_array &operator=(const state_array &rhs) NL_NOEXCEPT = default;
		state_array &operator=(state_array &&rhs) NL_NOEXCEPT = default;

		state_array &operator=(const T &rhs) NL_NOEXCEPT { m_value = rhs; return *this; }
		T & operator[](const std::size_t i) NL_NOEXCEPT { return m_value[i]; }
		constexpr const T & operator[](const std::size_t i) const NL_NOEXCEPT { return m_value[i]; }
	private:
		std::array<T, N> m_value;
	};

	// -----------------------------------------------------------------------------
	// State variables - predefined and c++11 non-optional
	// -----------------------------------------------------------------------------

	/*! predefined state variable type for uint8_t */
	using state_var_u8 = state_var<std::uint8_t>;
	/*! predefined state variable type for int8_t */
	using state_var_s8 = state_var<std::int8_t>;

	/*! predefined state variable type for uint32_t */
	using state_var_u32 = state_var<std::uint32_t>;
	/*! predefined state variable type for int32_t */
	using state_var_s32 = state_var<std::int32_t>;
	/*! predefined state variable type for sig_t */
	using state_var_sig = state_var<netlist_sig_t>;

	// -----------------------------------------------------------------------------
	// object_t
	// -----------------------------------------------------------------------------

	/*! The base class for netlist devices, terminals and parameters.
	 *
	 *  This class serves as the base class for all device, terminal and
	 *  objects. It provides new and delete operators to support e.g. pooled
	 *  memory allocation to enhance locality. Please refer to \ref USE_MEMPOOL as
	 *  well.
	 */
	class detail::object_t
	{
	public:

		/*! Constructor.
		 *
		 *  Every class derived from the object_t class must have a name.
		 */
		explicit object_t(const pstring &aname /*!< string containing name of the object */);

		COPYASSIGNMOVE(object_t, delete)
		/*! return name of the object
		 *
		 *  \returns name of the object.
		 */
		pstring name() const;

#if 0
		void * operator new (size_t size, void *ptr) { plib::unused_var(size); return ptr; }
		void operator delete (void *ptr, void *) { plib::unused_var(ptr); }
		void * operator new (size_t size) = delete;
		void operator delete (void * mem) = delete;
#endif
	protected:
		~object_t() noexcept = default; // only childs should be destructible

	private:
		//pstring m_name;
		static std::unordered_map<const object_t *, pstring> &name_hash()
		{
			static std::unordered_map<const object_t *, pstring> lhash;
			return lhash;
		}
	};

	struct detail::netlist_ref
	{
		explicit netlist_ref(netlist_state_t &nl);

		COPYASSIGNMOVE(netlist_ref, delete)

		netlist_state_t & state() noexcept;
		const netlist_state_t & state() const noexcept;

		setup_t & setup() noexcept;
		const setup_t & setup() const noexcept;

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

	/*! Base class for all objects being owned by a device.
	 *
	 * Serves as the base class of all objects being owned by a device.
	 *
	 */
	class detail::device_object_t : public detail::object_t
	{
	public:
		/*! Constructor.
		 *
		 * \param dev  device owning the object.
		 * \param name string holding the name of the device
		 */
		device_object_t(core_device_t &dev, const pstring &name);

		/*! returns reference to owning device.
		 * \returns reference to owning device.
		 */
		core_device_t &device() noexcept { return m_device; }
		const core_device_t &device() const noexcept { return m_device; }

		/*! The netlist owning the owner of this object.
		 * \returns reference to netlist object.
		 */
		netlist_state_t &state() NL_NOEXCEPT;
		const netlist_state_t &state() const NL_NOEXCEPT;

		netlist_t &exec() NL_NOEXCEPT;
		const netlist_t &exec() const NL_NOEXCEPT;

	private:
		core_device_t & m_device;
};

	// -----------------------------------------------------------------------------
	// core_terminal_t
	// -----------------------------------------------------------------------------

	/*! Base class for all terminals.
	 *
	 * All terminals are derived from this class.
	 *
	 */
	class detail::core_terminal_t : public device_object_t,
									public plib::linkedlist_t<core_terminal_t>::element_t
	{
	public:

		using list_t = std::vector<core_terminal_t *>;

		static constexpr const auto INP_HL_SHIFT = 0;
		static constexpr const auto INP_LH_SHIFT = 1;
		static constexpr const auto INP_ACTIVE_SHIFT = 2;

		enum state_e {
			STATE_INP_PASSIVE = 0,
			STATE_INP_HL      = (1 << INP_HL_SHIFT),
			STATE_INP_LH      = (1 << INP_LH_SHIFT),
			STATE_INP_ACTIVE  = (1 << INP_ACTIVE_SHIFT),
			STATE_OUT = 128,
			STATE_BIDIR = 256
		};

		core_terminal_t(core_device_t &dev, const pstring &aname,
				const state_e state, nldelegate delegate = nldelegate());
		virtual ~core_terminal_t() noexcept = default;

		COPYASSIGNMOVE(core_terminal_t, delete)

		/*! The object type.
		 * \returns type of the object
		 */
		terminal_type type() const;
		/*! Checks if object is of specified type.
		 * \param atype type to check object against.
		 * \returns true if object is of specified type else false.
		 */
		bool is_type(const terminal_type atype) const noexcept { return (type() == atype); }

		void set_net(net_t *anet) noexcept { m_net = anet; }
		void clear_net() noexcept { m_net = nullptr; }
		bool has_net() const noexcept { return (m_net != nullptr); }

		const net_t & net() const noexcept { return *m_net;}
		net_t & net() noexcept { return *m_net;}

		bool is_logic() const NL_NOEXCEPT;
		bool is_analog() const NL_NOEXCEPT;

		bool is_state(state_e astate) const noexcept { return (m_state == astate); }
		state_e terminal_state() const noexcept { return m_state; }
		void set_state(state_e astate) noexcept { m_state = astate; }

		void reset() noexcept { set_state(is_type(OUTPUT) ? STATE_OUT : STATE_INP_ACTIVE); }

		nldelegate m_delegate;
#if USE_COPY_INSTEAD_OF_REFERENCE
		void set_copied_input(netlist_sig_t val)
		{
			m_Q = val;
		}

		state_var_sig m_Q;
#else
		void set_copied_input(netlist_sig_t val) const { plib::unused_var(val); }
#endif

	private:
		net_t * m_net;
		state_var<state_e> m_state;
	};

	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public detail::core_terminal_t
	{
	public:

		analog_t(core_device_t &dev, const pstring &aname, const state_e state);

		const analog_net_t & net() const NL_NOEXCEPT;
		analog_net_t & net() NL_NOEXCEPT;
	};

	// -----------------------------------------------------------------------------
	// terminal_t
	// -----------------------------------------------------------------------------

	class terminal_t : public analog_t
	{
	public:

		terminal_t(core_device_t &dev, const pstring &aname, terminal_t *otherterm);

		nl_double operator ()() const  NL_NOEXCEPT;

		void set(const nl_double G) noexcept
		{
			set(G,G, 0.0);
		}

		void set(const nl_double GO, const nl_double GT)  noexcept
		{
			set(GO, GT, 0.0);
		}

		void set(const nl_double GO, const nl_double GT, const nl_double I)  noexcept
		{
			if (m_go1 != nullptr)
			{
				if (*m_Idr1 != I) *m_Idr1 = I;
				if (*m_go1 != GO) *m_go1 = GO;
				if (*m_gt1 != GT) *m_gt1 = GT;
			}
		}

		void solve_now();
		void schedule_solve_after(const netlist_time after);

		void set_ptrs(nl_double *gt, nl_double *go, nl_double *Idr) noexcept;

		terminal_t *otherterm() const noexcept { return m_otherterm; }
	private:

		nl_double *m_Idr1; // drive current
		nl_double *m_go1;  // conductance for Voltage from other term
		nl_double *m_gt1;  // conductance for total conductance

		terminal_t *m_otherterm;

	};


	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	class logic_t : public detail::core_terminal_t, public logic_family_t
	{
	public:
		logic_t(core_device_t &dev, const pstring &aname,
				const state_e state, nldelegate delegate = nldelegate());

		bool has_proxy() const { return (m_proxy != nullptr); }
		devices::nld_base_proxy *get_proxy() const  { return m_proxy; }
		void set_proxy(devices::nld_base_proxy *proxy) { m_proxy = proxy; }

		logic_net_t & net() NL_NOEXCEPT;
		const logic_net_t &  net() const NL_NOEXCEPT;

	protected:

	private:
		devices::nld_base_proxy *m_proxy;
	};

	// -----------------------------------------------------------------------------
	// logic_input_t
	// -----------------------------------------------------------------------------

	class logic_input_t : public logic_t
	{
	public:
		logic_input_t(core_device_t &dev, const pstring &aname,
				nldelegate delegate = nldelegate());

		netlist_sig_t operator()() const NL_NOEXCEPT
		{
			return Q();
		}

		void inactivate() NL_NOEXCEPT;
		void activate() NL_NOEXCEPT;
		void activate_hl() NL_NOEXCEPT;
		void activate_lh() NL_NOEXCEPT;
	private:
		netlist_sig_t Q() const NL_NOEXCEPT;
	};

	// -----------------------------------------------------------------------------
	// analog_input_t
	// -----------------------------------------------------------------------------

	/*! terminal providing analog input voltage.
	 *
	 * This terminal class provides a voltage measurement. The conductance against
	 * ground is infinite.
	 */
	class analog_input_t : public analog_t
	{
	public:
		/*! Constructor */
		analog_input_t(core_device_t &dev, /*!< owning device */
				const pstring &aname       /*!< name of terminal */
		);

		/*! returns voltage at terminal.
		 *  \returns voltage at terminal.
		 */
		nl_double operator()() const NL_NOEXCEPT { return Q_Analog(); }

		/*! returns voltage at terminal.
		 *  \returns voltage at terminal.
		 */
		nl_double Q_Analog() const NL_NOEXCEPT;

	};

	// -----------------------------------------------------------------------------
	// net_t
	// -----------------------------------------------------------------------------

	class detail::net_t :
			public detail::object_t,
			public detail::netlist_ref
	{
	public:

		enum class queue_status
		{
			DELAYED_DUE_TO_INACTIVE = 0,
			QUEUED,
			DELIVERED
		};

		net_t(netlist_state_t &nl, const pstring &aname, core_terminal_t *mr = nullptr);

		COPYASSIGNMOVE(net_t, delete)

		virtual ~net_t() noexcept = default;

		void reset();

		void toggle_new_Q() noexcept { m_new_Q = (m_cur_Q ^ 1);   }

		void toggle_and_push_to_queue(netlist_time delay) NL_NOEXCEPT
		{
			toggle_new_Q();
			push_to_queue(delay);
		}

		void push_to_queue(netlist_time delay) NL_NOEXCEPT;
		bool is_queued() const noexcept { return m_in_queue == queue_status::QUEUED; }

		void update_devs() NL_NOEXCEPT;

		netlist_time next_scheduled_time() const noexcept { return m_next_scheduled_time; }
		void set_next_scheduled_time(netlist_time ntime) noexcept { m_next_scheduled_time = ntime; }

		bool isRailNet() const noexcept { return !(m_railterminal == nullptr); }
		core_terminal_t & railterminal() const noexcept { return *m_railterminal; }

		std::size_t num_cons() const noexcept { return m_core_terms.size(); }

		void add_to_active_list(core_terminal_t &term) NL_NOEXCEPT;
		void remove_from_active_list(core_terminal_t &term) NL_NOEXCEPT;

		/* setup stuff */

		void add_terminal(core_terminal_t &terminal);
		void remove_terminal(core_terminal_t &terminal);

		bool is_logic() const NL_NOEXCEPT;
		bool is_analog() const NL_NOEXCEPT;

		void rebuild_list();     /* rebuild m_list after a load */
		void move_connections(net_t &dest_net);

		std::vector<core_terminal_t *> &core_terms() { return m_core_terms; }
#if USE_COPY_INSTEAD_OF_REFERENCE
		void update_inputs()
		{
			for (auto & term : m_core_terms)
				term->m_Q = m_cur_Q;
		}
#else
		void update_inputs() const
		{
			/* nothing needs to be done */
		}
#endif

	protected:

		/* only used for logic nets */
		netlist_sig_t Q() const noexcept { return m_cur_Q; }

		/* only used for logic nets */
		void initial(const netlist_sig_t val) noexcept
		{
			m_cur_Q = m_new_Q = val;
			update_inputs();
		}

		/* only used for logic nets */
		void set_Q_and_push(const netlist_sig_t newQ, const netlist_time delay) NL_NOEXCEPT
		{
			if (newQ != m_new_Q)
			{
				m_new_Q = newQ;
				push_to_queue(delay);
			}
		}

		/* only used for logic nets */
		void set_Q_time(const netlist_sig_t newQ, const netlist_time at) NL_NOEXCEPT
		{
			if (newQ != m_new_Q)
			{
				m_in_queue = queue_status::DELAYED_DUE_TO_INACTIVE;
				m_next_scheduled_time = at;
			}
			m_cur_Q = m_new_Q = newQ;
			update_inputs();
		}

		/* internal state support
		 * FIXME: get rid of this and implement export/import in MAME
		 */
		/* only used for logic nets */
		netlist_sig_t *Q_state_ptr() { return m_cur_Q.ptr(); }

	private:
		state_var<netlist_sig_t> m_new_Q;
		state_var<netlist_sig_t> m_cur_Q;
		state_var<queue_status>  m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */
		state_var<netlist_time>  m_next_scheduled_time;

		core_terminal_t * m_railterminal;
		plib::linkedlist_t<core_terminal_t> m_list_active;
		std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

		template <typename T>
		void process(const T mask, netlist_sig_t sig);
	};

	class logic_net_t : public detail::net_t
	{
	public:

		logic_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *mr = nullptr);

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

		analog_net_t(netlist_state_t &nl, const pstring &aname, detail::core_terminal_t *mr = nullptr);

		nl_double Q_Analog() const NL_NOEXCEPT { return m_cur_Analog; }
		void set_Q_Analog(const nl_double v) NL_NOEXCEPT { m_cur_Analog = v; }
		nl_double *Q_Analog_state_ptr() NL_NOEXCEPT { return m_cur_Analog.ptr(); }

		//FIXME: needed by current solver code
		devices::matrix_solver_t *solver() const NL_NOEXCEPT { return m_solver; }
		void set_solver(devices::matrix_solver_t *solver) NL_NOEXCEPT { m_solver = solver; }

	private:
		state_var<nl_double>     m_cur_Analog;
		devices::matrix_solver_t *m_solver;
	};

	// -----------------------------------------------------------------------------
	// logic_output_t
	// -----------------------------------------------------------------------------

	class logic_output_t : public logic_t
	{
	public:

		logic_output_t(core_device_t &dev, const pstring &aname);

		void initial(const netlist_sig_t val);

		void push(const netlist_sig_t newQ, const netlist_time delay) NL_NOEXCEPT
		{
			m_my_net.set_Q_and_push(newQ, delay); // take the shortcut
		}

		void set_Q_time(const netlist_sig_t newQ, const netlist_time at) NL_NOEXCEPT
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

		void push(const nl_double val) NL_NOEXCEPT { set_Q(val); }
		void initial(const nl_double val);

	private:
		void set_Q(const nl_double newQ) NL_NOEXCEPT;
		analog_net_t m_my_net;
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

		param_type_t param_type() const;

	protected:
		virtual ~param_t() noexcept = default; /* not intended to be destroyed */

		void update_param();

		pstring get_initial(const device_t &dev, bool *found);

		template<typename C>
		void set(C &p, const C v)
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

		const T operator()() const NL_NOEXCEPT { return m_param; }
		void setTo(const T &param) { set(m_param, param); }
	private:
		T m_param;
	};

	/* FIXME: these should go as well */
	using param_logic_t = param_num_t<bool>;
	using param_int_t = param_num_t<int>;
	using param_double_t = param_num_t<double>;

	// -----------------------------------------------------------------------------
	// pointer parameter
	// -----------------------------------------------------------------------------

	class param_ptr_t final: public param_t
	{
	public:
		param_ptr_t(device_t &device, const pstring &name, std::uint8_t* val);
		std::uint8_t * operator()() const NL_NOEXCEPT { return m_param; }
		void setTo(std::uint8_t *param) { set(m_param, param); }
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

		const pstring &operator()() const NL_NOEXCEPT { return Value(); }
		void setTo(const pstring &param) NL_NOEXCEPT
		{
			if (m_param != param)
			{
				m_param = param;
				changed();
				update_param();
			}
		}
	protected:
		virtual void changed();
		const pstring &Value() const NL_NOEXCEPT { return m_param; }
	private:
		pstring m_param;
	};

	// -----------------------------------------------------------------------------
	// model parameter
	// -----------------------------------------------------------------------------

	class param_model_t : public param_str_t
	{
	public:

		class value_t
		{
		public:
			value_t(param_model_t &param, const pstring &name)
			: m_value(param.model_value(name))
			{
			}
			const double &operator()() const NL_NOEXCEPT { return m_value; }
			operator const double&() const NL_NOEXCEPT { return m_value; }
		private:
			const double m_value;
		};

		friend class value_t;

		param_model_t(device_t &device, const pstring &name, const pstring &val)
		: param_str_t(device, name, val) { }

		const pstring model_value_str(const pstring &entity) /*const*/;
		const pstring model_type() /*const*/;
		/* hide this */
		void setTo(const pstring &param) = delete;
	protected:
		void changed() override;
		nl_double model_value(const pstring &entity) /*const*/;
	private:
		detail::model_map_t m_map;
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

		plib::unique_ptr<plib::pistream> stream();
	protected:
		void changed() override { }
	};

	// -----------------------------------------------------------------------------
	// rom parameter
	// -----------------------------------------------------------------------------

	template <typename ST, std::size_t AW, std::size_t DW>
	class param_rom_t final: public param_data_t
	{
	public:

		param_rom_t(device_t &device, const pstring &name);

		const ST & operator[] (std::size_t n) NL_NOEXCEPT { return m_data[n]; }
	protected:
		void changed() override
		{
			stream()->read(reinterpret_cast<plib::pistream::value_type *>(&m_data[0]),1<<AW);
		}

	private:
		std::array<ST, 1 << AW> m_data;
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

		void do_inc_active() NL_NOEXCEPT
		{
			if (m_hint_deactivate)
			{
				if (++m_active_outputs == 1)
				{
					m_stat_inc_active.inc();
					inc_active();
				}
			}
		}

		void do_dec_active() NL_NOEXCEPT
		{
			if (m_hint_deactivate)
				if (--m_active_outputs == 0)
				{
					dec_active();
				}
		}

		void set_hint_deactivate(bool v) { m_hint_deactivate = v; }
		bool get_hint_deactivate() { return m_hint_deactivate; }
		/* Has to be set in device reset */
		void set_active_outputs(int n) { m_active_outputs = n; }

		void set_default_delegate(detail::core_terminal_t &term);

		/* stats */
		nperftime_t<NL_KEEP_STATISTICS>  m_stat_total_time;
		nperfcount_t<NL_KEEP_STATISTICS> m_stat_call_count;
		nperfcount_t<NL_KEEP_STATISTICS> m_stat_inc_active;

		virtual void update() NL_NOEXCEPT { }
		virtual void reset() { }

	protected:

		virtual void inc_active() NL_NOEXCEPT {  }
		virtual void dec_active() NL_NOEXCEPT {  }

		log_type & log();

	public:
		virtual void timestep(const nl_double st) { plib::unused_var(st); }
		virtual void update_terminals() { }

		virtual void update_param() {}
		virtual bool is_dynamic() const { return false; }
		virtual bool is_timestep() const { return false; }

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

		setup_t &setup();
		const setup_t &setup() const;

		template<class C, typename... Args>
		void register_sub(const pstring &name, poolptr<C> &dev, const Args&... args)
		{
			//dev.reset(plib::palloc<C>(*this, name, args...));
			dev = pool().make_poolptr<C>(*this, name, args...);
		}

		void register_subalias(const pstring &name, detail::core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect(const pstring &t1, const pstring &t2);
		void connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
		void connect_post_start(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
	protected:

		NETLIB_UPDATEI() { }
		NETLIB_UPDATE_TERMINALSI() { }

	private:
	};

	// -----------------------------------------------------------------------------
	// family_setter_t
	// -----------------------------------------------------------------------------

	struct detail::family_setter_t
	{
		/* clang will complain about an unused private field if
		 * a defaulted constructor is used
		 */
		// NOLINTNEXTLINE(modernize-use-equals-default)
		family_setter_t();
		family_setter_t(core_device_t &dev, const pstring &desc);
		family_setter_t(core_device_t &dev, const logic_family_desc_t *desc);
	};

	// -----------------------------------------------------------------------------
	// nld_base_dummy : basis for dummy devices
	// FIXME: this is not the right place to define this
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(base_dummy)
	{
	public:
		NETLIB_CONSTRUCTOR(base_dummy) { }
	};

	// -----------------------------------------------------------------------------
	// queue_t
	// -----------------------------------------------------------------------------

	/* We don't need a thread-safe queue currently. Parallel processing of
	 * solvers will update inputs after parallel processing.
	 */
	class detail::queue_t :
			public timed_queue<pqentry_t<net_t *, netlist_time>, false, NL_KEEP_STATISTICS>,
			public detail::netlist_ref,
			public plib::state_manager_t::callback_t
	{
	public:
		using entry_t = pqentry_t<net_t *, netlist_time>;
		explicit queue_t(netlist_state_t &nl);

	protected:

		void register_state(plib::state_manager_t &manager, const pstring &module) override;
		void on_pre_save(plib::state_manager_t &manager) override;
		void on_post_load(plib::state_manager_t &manager) override;

	private:
		std::size_t m_qsize;
		std::vector<netlist_time::internal_type> m_times;
		std::vector<std::size_t> m_net_ids;
	};

	// -----------------------------------------------------------------------------
	// netlist_state__t
	// -----------------------------------------------------------------------------

	class netlist_state_t
	{
	public:

		using nets_collection_type = std::vector<poolptr<detail::net_t>>;

		/* need to preserve order of device creation ... */
		using devices_collection_type = std::vector<std::pair<pstring, poolptr<core_device_t>>>;
		netlist_state_t(const pstring &aname,
			plib::unique_ptr<callbacks_t> &&callbacks,
			plib::unique_ptr<setup_t> &&setup);

		COPYASSIGNMOVE(netlist_state_t, delete)

		~netlist_state_t() noexcept = default;

		friend class netlist_t; // allow access to private members

		template<class C>
		static bool check_class(core_device_t *p)
		{
			return dynamic_cast<C *>(p) != nullptr;
		}

		template<class C>
		C *get_single_device(const pstring &classname) const
		{
			return dynamic_cast<C *>(get_single_device(classname, check_class<C>));
		}

		/* logging and name */

		pstring name() const { return m_name; }

		log_type & log() { return m_log; }
		const log_type &log() const { return m_log; }

		plib::dynlib &lib() { return *m_lib; }

		/* state handling */
		plib::state_manager_t &run_state_manager() { return m_state; }

		template<typename O, typename C>
		void save(O &owner, C &state, const pstring &module, const pstring &stname)
		{
			this->run_state_manager().save_item(static_cast<void *>(&owner), state, module + pstring(".") + stname);
		}
		template<typename O, typename C>
		void save(O &owner, C *state, const pstring &module, const pstring &stname, const std::size_t count)
		{
			this->run_state_manager().save_state_ptr(static_cast<void *>(&owner), module + pstring(".") + stname, plib::state_manager_t::dtype<C>(), count, state);
		}

		core_device_t *get_single_device(const pstring &classname, bool (*cc)(core_device_t *)) const;

		detail::net_t *find_net(const pstring &name) const;
		std::size_t find_net_id(const detail::net_t *net) const;

		template <typename T>
		void register_net(poolptr<T> &&net) { m_nets.push_back(std::move(net)); }

		template<class device_class>
		inline std::vector<device_class *> get_device_list()
		{
			std::vector<device_class *> tmp;
			for (auto &d : m_devices)
			{
				auto dev = dynamic_cast<device_class *>(d.second.get());
				if (dev != nullptr)
					tmp.push_back(dev);
			}
			return tmp;
		}

		template <typename T>
		void add_dev(const pstring &name, poolptr<T> &&dev)
		{
			for (auto & d : m_devices)
				if (d.first == name)
					log().fatal(MF_1_DUPLICATE_NAME_DEVICE_LIST, d.first);
			//m_devices.push_back(std::move(dev));
			m_devices.insert(m_devices.end(), { name, std::move(dev) });
		}

		/**
		 * @brief Remove device
		 *
		 * Care needs to be applied if this is called to remove devices with
		 * sub-devices which may have registered state.
		 *
		 * @param dev Device to be removed
		 */
		void remove_dev(core_device_t *dev)
		{
			for (auto it = m_devices.begin(); it != m_devices.end(); it++)
				if (it->second.get() == dev)
				{
					m_state.remove_save_items(dev);
					m_devices.erase(it);
					return;
				}
		}

		/* sole use is to manage lifetime of family objects */
		std::vector<std::pair<pstring, plib::unique_ptr<logic_family_desc_t>>> m_family_cache;

		setup_t &setup() NL_NOEXCEPT { return *m_setup; }
		const setup_t &setup() const NL_NOEXCEPT { return *m_setup; }

		nets_collection_type & nets() { return m_nets; }
		devices_collection_type & devices() { return m_devices; }

		// FIXME: make a postload member and include code there
		void rebuild_lists(); /* must be called after post_load ! */

	private:

		void reset();

		pstring                             m_name;
		plib::unique_ptr<plib::dynlib>      m_lib; // external lib needs to be loaded as long as netlist exists
		plib::state_manager_t               m_state;
		plib::unique_ptr<callbacks_t>       m_callbacks;
		log_type                            m_log;
		plib::unique_ptr<setup_t>           m_setup;

		nets_collection_type                m_nets;
		/* sole use is to manage lifetime of net objects */
		devices_collection_type             m_devices;



	};

	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------

	class netlist_t
	{
	public:

		explicit netlist_t(const pstring &aname, plib::unique_ptr<callbacks_t> callbacks);

		COPYASSIGNMOVE(netlist_t, delete)

		~netlist_t() noexcept = default;

		/* run functions */

		netlist_time time() const NL_NOEXCEPT { return m_time; }

		void process_queue(const netlist_time delta) NL_NOEXCEPT;
		void abort_current_queue_slice() NL_NOEXCEPT { m_queue.retime(detail::queue_t::entry_t(m_time, nullptr)); }

		const detail::queue_t &queue() const NL_NOEXCEPT { return m_queue; }
		detail::queue_t &queue() NL_NOEXCEPT { return m_queue; }

		/* Control functions */

		void stop();
		void reset();

		/* state handling */

		plib::state_manager_t &run_state_manager() { return m_state->run_state_manager(); }

		/* only used by nltool to create static c-code */
		devices::NETLIB_NAME(solver) *solver() const NL_NOEXCEPT { return m_solver; }

		/* force late type resolution */
		template <typename X = devices::NETLIB_NAME(solver)>
		nl_double gmin(X *solv = nullptr) const NL_NOEXCEPT
		{
			plib::unused_var(solv);
			return static_cast<X *>(m_solver)->gmin();
		}

		netlist_state_t &nlstate() NL_NOEXCEPT { return *m_state; }
		const netlist_state_t &nlstate() const { return *m_state; }

		log_type & log() { return m_state->log(); }
		const log_type &log() const { return m_state->log(); }

		void print_stats() const;

	private:
		plib::unique_ptr<netlist_state_t>   m_state;
		devices::NETLIB_NAME(solver) *      m_solver;

		/* mostly rw */
		PALIGNAS_CACHELINE()
		netlist_time                        m_time;
		devices::NETLIB_NAME(mainclock) *   m_mainclock;

		PALIGNAS_CACHELINE()
		detail::queue_t                     m_queue;

		// performance
		nperftime_t<NL_KEEP_STATISTICS>     m_stat_mainloop;
		nperfcount_t<NL_KEEP_STATISTICS>    m_perf_out_processed;
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

	inline setup_t & detail::netlist_ref::setup() noexcept
	{
		return m_netlist.nlstate().setup();
	}

	inline const setup_t & detail::netlist_ref::setup() const noexcept
	{
		return m_netlist.nlstate().setup();
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
			bool err = false;
			auto vald = plib::pstonum_ne<T>(p, err);
			if (err)
				device.state().log().fatal(MF_2_INVALID_NUMBER_CONVERSION_1_2, name, p);
			m_param = vald;
		}
		else
			m_param = val;

		device.state().save(*this, m_param, this->name(), "m_param");
	}

	template <typename ST, std::size_t AW, std::size_t DW>
	param_rom_t<ST, AW, DW>::param_rom_t(device_t &device, const pstring &name)
	: param_data_t(device, name)
	{
		auto f = stream();
		if (f != nullptr)
			f->read(reinterpret_cast<plib::pistream::value_type *>(&m_data[0]),1<<AW);
		else
			device.state().log().warning("Rom {1} not found", Value());
	}

	inline void logic_input_t::inactivate() NL_NOEXCEPT
	{
		if (!is_state(STATE_INP_PASSIVE))
		{
			set_state(STATE_INP_PASSIVE);
			net().remove_from_active_list(*this);
		}
	}

	inline void logic_input_t::activate() NL_NOEXCEPT
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_ACTIVE);
		}
	}

	inline void logic_input_t::activate_hl() NL_NOEXCEPT
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_HL);
		}
	}

	inline void logic_input_t::activate_lh() NL_NOEXCEPT
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().add_to_active_list(*this);
			set_state(STATE_INP_LH);
		}
	}

	inline void detail::net_t::push_to_queue(const netlist_time delay) NL_NOEXCEPT
	{
		if ((num_cons() != 0))
		{
			auto &lexec(exec());
			auto &q(lexec.queue());
			auto nst(lexec.time() + delay);

			if (is_queued())
				q.remove(this);
			m_in_queue = (!m_list_active.empty()) ?
				queue_status::QUEUED : queue_status::DELAYED_DUE_TO_INACTIVE;    /* queued ? */
			if (m_in_queue == queue_status::QUEUED)
				q.push(queue_t::entry_t(nst, this));
			else
				update_inputs();
			m_next_scheduled_time = nst;
		}
	}

	inline void detail::net_t::add_to_active_list(core_terminal_t &term) NL_NOEXCEPT
	{
		if (m_list_active.empty())
		{
			m_list_active.push_front(&term);
			railterminal().device().do_inc_active();
			if (m_in_queue == queue_status::DELAYED_DUE_TO_INACTIVE)
			{
				if (m_next_scheduled_time > exec().time())
				{
					m_in_queue = queue_status::QUEUED;     /* pending */
					exec().queue().push({m_next_scheduled_time, this});
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

	inline void detail::net_t::remove_from_active_list(core_terminal_t &term) NL_NOEXCEPT
	{
		m_list_active.remove(&term);
		if (m_list_active.empty())
			railterminal().device().do_dec_active();
	}

	inline const analog_net_t & analog_t::net() const NL_NOEXCEPT
	{
		return static_cast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net() NL_NOEXCEPT
	{
		return static_cast<analog_net_t &>(core_terminal_t::net());
	}

	inline nl_double terminal_t::operator ()() const NL_NOEXCEPT { return net().Q_Analog(); }

	inline void terminal_t::set_ptrs(nl_double *gt, nl_double *go, nl_double *Idr) noexcept
	{
		if (!(gt && go && Idr) && (gt || go || Idr))
			state().log().fatal("Inconsistent nullptrs for terminal {}", name());
		else
		{
			m_gt1 = gt;
			m_go1 = go;
			m_Idr1 = Idr;
		}
	}

	inline logic_net_t & logic_t::net() NL_NOEXCEPT
	{
		return static_cast<logic_net_t &>(core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const NL_NOEXCEPT
	{
		return static_cast<const logic_net_t &>(core_terminal_t::net());
	}

	inline netlist_sig_t logic_input_t::Q() const NL_NOEXCEPT
	{
		nl_assert(terminal_state() != STATE_INP_PASSIVE);
		//if (net().Q() != m_Q)
		//  printf("term: %s, %d %d TS %d\n", this->name().c_str(), net().Q(), m_Q, terminal_state());
#if USE_COPY_INSTEAD_OF_REFERENCE
		return m_Q;
#else
		return net().Q();
#endif
	}

	inline nl_double analog_input_t::Q_Analog() const NL_NOEXCEPT
	{
		return net().Q_Analog();
	}

	inline void analog_output_t::set_Q(const nl_double newQ) NL_NOEXCEPT
	{
		if (newQ != m_my_net.Q_Analog())
		{
			m_my_net.set_Q_Analog(newQ);
			m_my_net.toggle_and_push_to_queue(NLTIME_FROM_NS(1));
		}
	}

	inline netlist_state_t &detail::device_object_t::state() NL_NOEXCEPT
	{
		return m_device.state();
	}

	inline const netlist_state_t &detail::device_object_t::state() const NL_NOEXCEPT
	{
		return m_device.state();
	}

	inline netlist_t &detail::device_object_t::exec() NL_NOEXCEPT
	{
		return m_device.exec();
	}

	inline const netlist_t &detail::device_object_t::exec() const NL_NOEXCEPT
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

	template <typename T, std::size_t N>
	template <typename O>
	state_array<T,N>::state_array(O &owner, const pstring &name, const T & value)
	{
		owner.state().save(owner, m_value, owner.name(), name);
		for (std::size_t i=0; i<N; i++)
			m_value[i] = value;
	}
} // namespace netlist

namespace plib
{
	template<typename X>
	struct ptype_traits<netlist::state_var<X>> : ptype_traits<X>
	{
	};
} // namespace plib



#endif /* NLBASE_H_ */
