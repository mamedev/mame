// license:GPL-2.0+
// copyright-holders:Couriersud
/*!
 *
 * \file nl_base.h
 *
 */

#ifndef NLBASE_H_
#define NLBASE_H_

#include <vector>
#include <unordered_map>
#include <memory>
//#include <cmath>
#include <cstdint>

#include "nl_lists.h"
#include "nl_time.h"
#include "plib/palloc.h"
#include "plib/pdynlib.h"
#include "plib/pstate.h"
#include "plib/pfmtlog.h"

// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------

/*! netlist_sig_t is the type used for logic signals. */
using netlist_sig_t = std::uint_least32_t;

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
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name) \
	: NETLIB_NAME(pclass)(owner, name)

/*! Used to define the constructor of a netlist device.
 *  Use this to define the constructor of a netlist device. Please refer to
 *  #NETLIB_OBJECT for an example.
 */
#define NETLIB_CONSTRUCTOR(cname)                                              \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name) \
		: device_t(owner, name)

	/*! Used to define the destructor of a netlist device.
	*  The use of a destructor for netlist device should normally not be necessary.
	*/
#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)()

	/*! Define an extended constructor and add further parameters to it.
	*  The macro allows to add further parameters to a device constructor. This is
	*  normally used for sub-devices and system devices only.
	*/
#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                      \
	private: detail::family_setter_t m_famsetter;                              \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name, __VA_ARGS__) \
		: device_t(owner, name)

	/*! Add this to a device definition to mark the device as dynamic.
	*  If this is added to device definition the device is treated as an analog
	*  dynamic device, i.e. #NETLIB_UPDATE_TERMINALSI is called on a each step
	*  of the Newton-Raphson step of solving the linear equations.
	*/
#define NETLIB_DYNAMIC()                                                       \
	public: virtual bool is_dynamic() const override { return true; }

	/*! Add this to a device definition to mark the device as a time-stepping device
	*  and add code.
	*  If this is added to device definition the device is treated as an analog
	*  time-stepping device. Currently, only the capacitor device uses this. An other
	*  example would be an inductor device.
	*
	*  Example:
	*
	*   NETLIB_TIMESTEP()
	*       {
	*           // Gpar should support convergence
	*           const nl_double G = m_C.Value() / step +  m_GParallel;
	*           const nl_double I = -G * deltaV();
	*           set(G, 0.0, I);
	*       }
	*
	*/
#define NETLIB_TIMESTEP()                                                      \
	public: virtual bool is_timestep() const override { return true; } \
	public: virtual void step_time(const nl_double step) override

#define NETLIB_UPDATE_AFTER_PARAM_CHANGE()                                     \
	public: virtual bool needs_update_after_param_change() const override { return true; }

#define NETLIB_FAMILY(family) , m_famsetter(*this, family)

#define NETLIB_UPDATE_TERMINALSI() public: virtual void update_terminals() override
#define NETLIB_UPDATEI() protected: virtual void update() noexcept override
#define NETLIB_UPDATE_PARAMI() public: virtual void update_param() override
#define NETLIB_RESETI() protected: virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUBXX(chip) std::unique_ptr< nld_ ## chip >

#define NETLIB_UPDATE(chip) void NETLIB_NAME(chip) :: update(void) NL_NOEXCEPT

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_STOP(chip) void NETLIB_NAME(chip) :: stop(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param(void)
#define NETLIB_FUNC_VOID(chip, name, params) void NETLIB_NAME(chip) :: name params

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals(void)

//============================================================
//  Asserts
//============================================================

#if defined(MAME_DEBUG)
#define nl_assert(x)    do { if (1) if (!(x)) throw nl_exception(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#define NL_NOEXCEPT
#else
#define nl_assert(x)    do { if (0) if (!(x)) throw nl_exception(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#define NL_NOEXCEPT		noexcept
#endif
#define nl_assert_always(x, msg)    do { if (!(x)) throw nl_exception(plib::pfmt("Fatal error: {1}\nCaused by assert: {2}:{3}: {4}")(msg)(__FILE__)(__LINE__)(#x)); } while (0)

// -----------------------------------------------------------------------------
// forward definitions
// -----------------------------------------------------------------------------

//============================================================
// Namespace starts
//============================================================

namespace netlist
{
	namespace devices
	{
		class matrix_solver_t;
		class NETLIB_NAME(gnd);
		class NETLIB_NAME(solver);
		class NETLIB_NAME(mainclock);
		class NETLIB_NAME(netlistparams);
		class NETLIB_NAME(base_proxy);
		class NETLIB_NAME(base_d_to_a_proxy);
	}

	namespace detail {
		class object_t;
		class device_object_t;
		struct netlist_ref;
		class core_terminal_t;
		struct family_setter_t;
		class queue_t;
		class net_t;
	}

	//============================================================
	//  Exceptions
	//============================================================

	/*! Generic netlist expection.
	 *  The exception is used in all events which are considered fatal.
	 */
	class nl_exception : public plib::pexception
	{
	public:
		/*! Constructor.
		 *  Allows a descriptive text to be assed to the exception
		 */
		explicit nl_exception(const pstring text //!< text to be passed
				)
		: plib::pexception(text) { }
		/*! Copy constructor. */
		nl_exception(const nl_exception &e) : plib::pexception(e) { }
		virtual ~nl_exception() {}
	};

	class logic_output_t;
	class analog_net_t;
	class logic_net_t;
	class net_t;
	class setup_t;
	class netlist_t;
	class core_device_t;
	class device_t;

	/*! Type of the model map used.
	 *  This is used to hold all #Models in an unordered map
	 */
	using model_map_t = std::unordered_map<pstring, pstring>;

	/*! Logic families descriptors are used create proxy devices.
	 *  The logic family describe the analog capabilities of logic devices,
	 *  inputs and outputs.
	 */
	class logic_family_desc_t
	{
	public:
		logic_family_desc_t() {}
		virtual ~logic_family_desc_t() {}
		virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_t &anetlist, const pstring &name,
				logic_output_t *proxied) const = 0;

		nl_double m_low_thresh_V;   //!< low input threshhold. If the input voltage is below this value, a "0" input is signalled
		nl_double m_high_thresh_V;  //!< high input threshhold. If the input voltage is above this value, a "0" input is signalled
		nl_double m_low_V;          //!< low output voltage. This voltage is output if the ouput is "0"
		nl_double m_high_V;         //!< high output voltage. This voltage is output if the ouput is "1"
		nl_double m_R_low;          //!< low output resistance. Value of series resistor used for low output
		nl_double m_R_high;         //!< high output resistance. Value of series resistor used for high output
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
		~logic_family_t() { }

		const logic_family_desc_t *logic_family() const { return m_logic_family; }
		void set_logic_family(const logic_family_desc_t *fam) { m_logic_family = fam; }

	protected:
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
				const pstring name,     //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);
		//! Copy Constructor.
		state_var(const state_var &rhs) noexcept = default;
		//! Move Constructor.
		state_var(state_var &&rhs) noexcept = default;
		//! Assignment operator to assign value of a state var.
		state_var &operator=(state_var rhs) { std::swap(rhs.m_value, this->m_value); return *this; }
		//! Assignment operator to assign value of type T.
		state_var &operator=(const T rhs) { m_value = rhs; return *this; }
		//! Return value of state variable.
		operator T & () { return m_value; }
		//! Return value of state variable.
		T & operator()() { return m_value; }
		//! Return const value of state variable.
		operator const T & () const { return m_value; }
		//! Return const value of state variable.
		const T & operator()() const { return m_value; }
		T * ptr() { return &m_value; }
		const T * ptr() const { return &m_value; }
	private:
		T m_value;
	};

	/*! A persistent array template.
	 *  Use this state_var template to define an array whose contents are saved.
	 *  Please refer to \ref state_var.
	 */
	template <typename T, std::size_t N>
	struct state_var<T[N]>
	{
	public:
		state_var(device_t &dev, const pstring name, const T & value);
		state_var(const state_var &rhs) noexcept = default;
		state_var(state_var &&rhs) noexcept = default;
		state_var &operator=(const state_var rhs) { m_value = rhs.m_value; return *this; }
		state_var &operator=(const T rhs) { m_value = rhs; return *this; }
		T & operator[](const std::size_t i) { return m_value[i]; }
		const T & operator[](const std::size_t i) const { return m_value[i]; }
	private:
		T m_value[N];
	};

	// -----------------------------------------------------------------------------
	// State variables - predefined and c++11 non-optioanl
	// -----------------------------------------------------------------------------

	/*! predefined state variable type for uint_fast8_t */
	using state_var_u8 = state_var<std::uint_fast8_t>;
	/*! predefined state variable type for int_fast8_t */
	using state_var_s8 = state_var<std::int_fast8_t>;

	/*! predefined state variable type for uint_fast32_t */
	using state_var_u32 = state_var<std::uint_fast32_t>;
	/*! predefined state variable type for int_fast32_t */
	using state_var_s32 = state_var<std::int_fast32_t>;

	// -----------------------------------------------------------------------------
	// object_t
	// -----------------------------------------------------------------------------

	/*! The base class for netlist devices, terminals and parameters.
	 *
	 *  This class serves as the base class for all device, terminal and
	 *  objects. It provides new and delete operators to supported e.g. pooled
	 *  memory allocation to enhance locality. Please refer to \ref USE_MEMPOOL as
	 *  well.
	 */
	class detail::object_t
	{
		P_PREVENT_COPYING(object_t)
	public:

		/*! Constructor.
		 *
		 *  Every class derived from the object_t class must have a name.
		 */
		object_t(const pstring &aname /*!< string containing name of the object */);
		~object_t();

		/*! return name of the object
		 *
		 *  \returns name of the object.
		 */
		const pstring &name() const;

		void * operator new (size_t size, void *ptr) { return ptr; }
		void operator delete (void *ptr, void *) {  }
		void * operator new (size_t size);
		void operator delete (void * mem);

	private:
		pstring m_name;
	};

	struct detail::netlist_ref
	{
		netlist_ref(netlist_t &nl) : m_netlist(nl) { }

		netlist_t & netlist() { return m_netlist; }
		const netlist_t & netlist() const { return m_netlist; }

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
		P_PREVENT_COPYING(device_object_t)
	public:
		/*! Enum specifying the type of object */
		enum type_t {
			TERMINAL = 0, /*!< object is an analog terminal */
			INPUT    = 1, /*!< object is an input */
			OUTPUT   = 2, /*!< object is an output */
			PARAM    = 3, /*!< object is a parameter */
		};

		/*! Constructor.
		 *
		 * \param dev  device owning the object.
		 * \param name string holding the name of the device
		 * \param type type   of this object.
		 */
		device_object_t(core_device_t &dev, const pstring &name, const type_t type);
		/*! returns reference to owning device.
		 * \returns reference to owning device.
		 */
		core_device_t &device() const { return m_device; }

		/*! The object type.
		 * \returns type of the object
		 */
		type_t type() const { return m_type; }
		/*! Checks if object is of specified type.
		 * \param type type to check object against.
		 * \returns true if object is of specified type else false.
		 */
		bool is_type(const type_t type) const { return (m_type == type); }

		/*! The netlist owning the owner of this object.
		 * \returns reference to netlist object.
		 */
		netlist_t &netlist();

	private:
		core_device_t & m_device;
		const type_t    m_type;
	};


	// -----------------------------------------------------------------------------
	// core_terminal_t
	// -----------------------------------------------------------------------------

	class detail::core_terminal_t : public device_object_t, public plib::linkedlist_t<core_terminal_t>::element_t
	{
		P_PREVENT_COPYING(core_terminal_t)
	public:

		using list_t = std::vector<core_terminal_t *>;

		/* needed here ... */

		enum state_e {
			STATE_INP_PASSIVE = 0,
			STATE_INP_ACTIVE = 1,
			STATE_INP_HL = 2,
			STATE_INP_LH = 4,
			STATE_OUT = 128,
			STATE_NONEX = 256
		};

		core_terminal_t(core_device_t &dev, const pstring &aname, const type_t atype);
		virtual ~core_terminal_t() { }

		void set_net(net_t *anet);
		void clear_net();
		bool has_net() const { return (m_net != nullptr); }

		const net_t & net() const { return *m_net;}
		net_t & net() { return *m_net;}

		bool is_logic() const noexcept;
		bool is_analog() const noexcept;

		bool is_state(const state_e astate) const { return (m_state == astate); }
		state_e state() const { return m_state; }
		void set_state(const state_e astate)
		{
			nl_assert(astate != STATE_NONEX);
			m_state = astate;
		}

		void reset();

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

		analog_t(core_device_t &dev, const pstring &aname, const type_t atype)
		: core_terminal_t(dev, aname, atype)
		{
		}

		const analog_net_t & net() const noexcept;
		analog_net_t & net() noexcept;
	};

	// -----------------------------------------------------------------------------
	// terminal_t
	// -----------------------------------------------------------------------------

	class terminal_t : public analog_t
	{
		P_PREVENT_COPYING(terminal_t)
	public:

		terminal_t(core_device_t &dev, const pstring &aname);

		nl_double operator ()() const;

		void set(const nl_double G)
		{
			set_ptr(m_Idr1, 0);
			set_ptr(m_go1, G);
			set_ptr(m_gt1, G);
		}

		void set(const nl_double GO, const nl_double GT)
		{
			set_ptr(m_Idr1, 0);
			set_ptr(m_go1, GO);
			set_ptr(m_gt1, GT);
		}

		void set(const nl_double GO, const nl_double GT, const nl_double I)
		{
			set_ptr(m_Idr1, I);
			set_ptr(m_go1, GO);
			set_ptr(m_gt1, GT);
		}

		void schedule_solve();
		void schedule_after(const netlist_time &after);

		void set_ptrs(nl_double *gt, nl_double *go, nl_double *Idr)
		{
			m_gt1 = gt;
			m_go1 = go;
			m_Idr1 = Idr;
		}

		terminal_t *m_otherterm;

	private:
		void set_ptr(nl_double *ptr, const nl_double val)
		{
			if (ptr != nullptr && *ptr != val)
			{
				*ptr = val;
			}
		}

		state_var<nl_double *> m_Idr1; // drive current
		state_var<nl_double *> m_go1;  // conductance for Voltage from other term
		state_var<nl_double *> m_gt1;  // conductance for total conductance

	};


	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	class logic_t : public detail::core_terminal_t, public logic_family_t
	{
	public:
		logic_t(core_device_t &dev, const pstring &aname, const type_t atype)
			: core_terminal_t(dev, aname, atype), logic_family_t(),
				m_proxy(nullptr)
		{
		}

		bool has_proxy() const { return (m_proxy != nullptr); }
		devices::nld_base_proxy *get_proxy() const  { return m_proxy; }
		void set_proxy(devices::nld_base_proxy *proxy) { m_proxy = proxy; }

		logic_net_t & net() noexcept;
		const logic_net_t &  net() const noexcept;

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
		logic_input_t(core_device_t &dev, const pstring &aname);

		netlist_sig_t Q() const noexcept;

		netlist_sig_t operator()() const NL_NOEXCEPT
		{
			nl_assert(state() != STATE_INP_PASSIVE);
			return Q();
		}

		void inactivate();
		void activate();
		void activate_hl();
		void activate_lh();

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
		nl_double operator()() const { return Q_Analog(); }

		/*! returns voltage at terminal.
		 *  \returns voltage at terminal.
		 */
		nl_double Q_Analog() const noexcept;

	};

	// -----------------------------------------------------------------------------
	// net_t
	// -----------------------------------------------------------------------------

	class detail::net_t :
			public detail::object_t,
			public detail::netlist_ref
	{
		P_PREVENT_COPYING(net_t)
	public:

		net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr = nullptr);
		virtual ~net_t();

		void reset();

		void register_con(core_terminal_t &terminal);

		bool is_logic() const noexcept;
		bool is_analog() const noexcept;

		void toggle_new_Q()             { m_new_Q ^= 1;   }
		void force_queue_execution()    { m_new_Q = (m_cur_Q ^ 1);   }

		void push_to_queue(const netlist_time delay) noexcept;
		void reschedule_in_queue(const netlist_time delay) noexcept;
		bool is_queued() const { return m_in_queue == 1; }

		void update_devs() NL_NOEXCEPT;

		const netlist_time time() const { return m_time; }
		void set_time(const netlist_time ntime) { m_time = ntime; }

		bool isRailNet() const { return !(m_railterminal == nullptr); }
		core_terminal_t & railterminal() const { return *m_railterminal; }

		std::size_t num_cons() const noexcept { return m_core_terms.size(); }

		void inc_active(core_terminal_t &term) NL_NOEXCEPT;
		void dec_active(core_terminal_t &term) NL_NOEXCEPT;

		void rebuild_list();     /* rebuild m_list after a load */
		void move_connections(net_t &dest_net);

		std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

	protected:
		state_var<netlist_sig_t> m_new_Q;
		state_var<netlist_sig_t> m_cur_Q;

		state_var<netlist_time>  m_time;
		state_var_s32            m_active;
		state_var_u8             m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

	private:
		plib::linkedlist_t<core_terminal_t> m_list_active;
		core_terminal_t * m_railterminal;

	public:
		// FIXME: Have to fix the public at some time
		state_var<nl_double>     m_cur_Analog;

	};

	class logic_net_t : public detail::net_t
	{
		P_PREVENT_COPYING(logic_net_t)
	public:

		logic_net_t(netlist_t &nl, const pstring &aname, detail::core_terminal_t *mr = nullptr);
		virtual ~logic_net_t() { }

		netlist_sig_t Q() const { return m_cur_Q; }
		netlist_sig_t new_Q() const     { return m_new_Q; }
		void initial(const netlist_sig_t val) { m_cur_Q = m_new_Q = val; }

		void set_Q(const netlist_sig_t newQ, const netlist_time delay) noexcept
		{
			if (newQ != m_new_Q)
			{
				m_new_Q = newQ;
				push_to_queue(delay);
			}
		}

		void set_Q_time(const netlist_sig_t newQ, const netlist_time at) noexcept
		{
			if (newQ != m_new_Q)
			{
				m_in_queue = 0;
				m_time = at;
			}
			m_cur_Q = m_new_Q = newQ;
		}

		/* internal state support
		 * FIXME: get rid of this and implement export/import in MAME
		 */
		netlist_sig_t &Q_state_ptr() { return m_cur_Q; }

	protected:
	private:

	};

	class analog_net_t : public detail::net_t
	{
		P_PREVENT_COPYING(analog_net_t)
	public:

		using list_t =  std::vector<analog_net_t *>;

		analog_net_t(netlist_t &nl, const pstring &aname, detail::core_terminal_t *mr = nullptr);

		virtual ~analog_net_t() { }

		nl_double Q_Analog() const { return m_cur_Analog; }
		nl_double &Q_Analog_state_ptr() { return m_cur_Analog; }

		//FIXME: needed by current solver code
		devices::matrix_solver_t *solver() const { return m_solver; }
		void set_solver(devices::matrix_solver_t *solver) { m_solver = solver; }

	private:
		devices::matrix_solver_t *m_solver;
	};

	// -----------------------------------------------------------------------------
	// logic_output_t
	// -----------------------------------------------------------------------------

	class logic_output_t : public logic_t
	{
		P_PREVENT_COPYING(logic_output_t)
	public:

		logic_output_t(core_device_t &dev, const pstring &aname);

		void initial(const netlist_sig_t val);

		void push(const netlist_sig_t newQ, const netlist_time delay) noexcept
		{
			m_my_net.set_Q(newQ, delay); // take the shortcut
		}

	private:
		logic_net_t m_my_net;
	};

	class analog_output_t : public analog_t
	{
		P_PREVENT_COPYING(analog_output_t)
	public:
		analog_output_t(core_device_t &dev, const pstring &aname);

		void push(const nl_double val) noexcept { set_Q(val); }
		void initial(const nl_double val);

	private:
		void set_Q(const nl_double newQ) noexcept;
		analog_net_t m_my_net;
	};

	// -----------------------------------------------------------------------------
	// param_t
	// -----------------------------------------------------------------------------

	class param_t : public detail::device_object_t
	{
		P_PREVENT_COPYING(param_t)
	public:

		enum param_type_t {
			MODEL,
			STRING,
			DOUBLE,
			INTEGER,
			LOGIC
		};

		param_t(const param_type_t atype, device_t &device, const pstring &name);
		virtual ~param_t() {}

		param_type_t param_type() const { return m_param_type; }

	private:
		const param_type_t m_param_type;
	};

	template <typename C, param_t::param_type_t T>
	class param_template_t : public param_t
	{
		P_PREVENT_COPYING(param_template_t)
	public:
		param_template_t(device_t &device, const pstring name, const C val);

		const C operator()() const { return Value(); }

		void setTo(const C &param);
		void initial(const C &val) { m_param = val; }

	protected:
		C Value() const { return m_param;   }
		virtual void changed() { }
		C m_param;
	private:
	};

	using param_double_t = param_template_t<nl_double, param_t::DOUBLE>;
	using param_int_t = param_template_t<int, param_t::INTEGER>;
	using param_str_t = param_template_t<pstring, param_t::STRING>;

	using param_logic_t = param_template_t<bool, param_t::LOGIC>;

	class param_model_t : public param_str_t
	{
	public:
		param_model_t(device_t &device, const pstring name, const pstring val)
		: param_str_t(device, name, val) { }

		/* these should be cached! */
		nl_double model_value(const pstring &entity);
		const pstring model_value_str(const pstring &entity);
		const pstring model_type();
	protected:
		void changed() override
		{
			m_map.clear();
		}
	private:
		model_map_t m_map;
	};

	// -----------------------------------------------------------------------------
	// core_device_t
	// -----------------------------------------------------------------------------

	class core_device_t :
			public detail::object_t,
			public logic_family_t,
			public detail::netlist_ref
	{
		P_PREVENT_COPYING(core_device_t)
	public:
		core_device_t(netlist_t &owner, const pstring &name);
		core_device_t(core_device_t &owner, const pstring &name);

		virtual ~core_device_t();

		void update_dev() noexcept
		{
			m_stat_total_time.start();
			do_update();
			m_stat_total_time.stop();
		}

		void do_update() noexcept
		{
			#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF)
				(this->*m_static_update)();
			#elif ((NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV) || (NL_PMF_TYPE == NL_PMF_TYPE_INTERNAL))
				m_static_update(this);
			#else
				update();
			#endif
		}

		void set_delegate_pointer();
		void stop_dev();

		void do_inc_active() noexcept
		{
			if (m_hint_deactivate)
			{
				m_stat_inc_active.inc();
				inc_active();
			}
		}

		void do_dec_active() noexcept
		{
			if (m_hint_deactivate)
				dec_active();
		}
		void do_reset() { reset(); }
		void set_hint_deactivate(bool v) { m_hint_deactivate = v; }

		/* stats */
		nperftime_t  m_stat_total_time;
		nperfcount_t m_stat_call_count;
		nperfcount_t m_stat_inc_active;

	protected:

		virtual void update() noexcept { }
		virtual void inc_active() noexcept {  }
		virtual void dec_active() noexcept {  }
		virtual void stop() { }
		virtual void reset() { }

	public:
		virtual void step_time(ATTR_UNUSED const nl_double st) { }
		virtual void update_terminals() { }

		virtual void update_param() {}
		virtual bool is_dynamic() const { return false; }
		virtual bool is_timestep() const { return false; }
		virtual bool needs_update_after_param_change() const { return false; }

	private:
		bool m_hint_deactivate;
	#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF)
		typedef void (core_device_t::*net_update_delegate)();
	#elif ((NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV) || (NL_PMF_TYPE == NL_PMF_TYPE_INTERNAL))
		using net_update_delegate = MEMBER_ABI void (*)(core_device_t *);
	#endif

	#if (NL_PMF_TYPE > NL_PMF_TYPE_VIRTUAL)
		net_update_delegate m_static_update;
	#endif
	};

	// -----------------------------------------------------------------------------
	// device_t
	// -----------------------------------------------------------------------------

	class device_t : public core_device_t
	{
		P_PREVENT_COPYING(device_t)
	public:

		template <class C>
		device_t(C &owner, const pstring &name)
			: core_device_t(owner, name) { }

		virtual ~device_t();

		setup_t &setup();

#if 1
		template<class C>
		void register_sub(const pstring &name, std::unique_ptr<C> &dev)
		{
			dev.reset(new C(*this, name));
		}
#endif

		void register_subalias(const pstring &name, detail::core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect_late(const pstring &t1, const pstring &t2);
		void connect_late(detail::core_terminal_t &t1, detail::core_terminal_t &t2);
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
		family_setter_t() { }
		family_setter_t(core_device_t &dev, const char *desc);
		family_setter_t(core_device_t &dev, const logic_family_desc_t *desc);
	};

	// -----------------------------------------------------------------------------
	// nld_base_dummy : basis for dummy devices
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(base_dummy)
	{
	public:
		NETLIB_CONSTRUCTOR(base_dummy) { }
	};

	// -----------------------------------------------------------------------------
	// queue_t
	// -----------------------------------------------------------------------------

	class detail::queue_t :
			public timed_queue<net_t *, netlist_time>,
			public detail::object_t,
			public detail::netlist_ref,
			public plib::state_manager_t::callback_t
	{
	public:
		explicit queue_t(netlist_t &nl);

	protected:

		void register_state(plib::state_manager_t &manager, const pstring &module) override;
		void on_pre_save() override;
		void on_post_load() override;

	private:
		struct names_t { char m_buf[64]; };
		std::size_t m_qsize;
		std::vector<netlist_time::internal_type> m_times;
		std::vector<names_t> m_names;
	};

	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------


	class netlist_t : public plib::plog_dispatch_intf
	{
		P_PREVENT_COPYING(netlist_t)
	public:

		explicit netlist_t(const pstring &aname);
		virtual ~netlist_t();

		pstring name() const { return m_name; }

		void start();
		void stop();

		const detail::queue_t &queue() const { return m_queue; }
		detail::queue_t &queue() { return m_queue; }
		const netlist_time time() const { return m_time; }
		devices::NETLIB_NAME(solver) *solver() const { return m_solver; }
		devices::NETLIB_NAME(gnd) *gnd() const { return m_gnd; }
		nl_double gmin() const;

		void push_to_queue(detail::net_t &out, const netlist_time attime) noexcept;
		void remove_from_queue(detail::net_t &out) NL_NOEXCEPT;

		void process_queue(const netlist_time &delta);
		void abort_current_queue_slice() { m_queue.retime(m_time, nullptr); }

		void rebuild_lists(); /* must be called after post_load ! */

		void set_setup(setup_t *asetup) { m_setup = asetup;  }
		setup_t &setup() { return *m_setup; }

		void register_dev(plib::owned_ptr<device_t> dev);

		detail::net_t *find_net(const pstring &name);

		template<class device_class>
		std::vector<device_class *> get_device_list()
		{
			std::vector<device_class *> tmp;
			for (auto &d : m_devices)
			{
				device_class *dev = dynamic_cast<device_class *>(d.get());
				if (dev != nullptr)
					tmp.push_back(dev);
			}
			return tmp;
		}

		template<class device_class>
		device_class *get_single_device(const char *classname)
		{
			device_class *ret = nullptr;
			for (auto &d : m_devices)
			{
				device_class *dev = dynamic_cast<device_class *>(d.get());
				if (dev != nullptr)
				{
					if (ret != nullptr)
						this->log().fatal("more than one {1} device found", classname);
					else
						ret = dev;
				}
			}
			return ret;
		}

		plib::plog_base<NL_DEBUG> &log() { return m_log; }
		const plib::plog_base<NL_DEBUG> &log() const { return m_log; }

		plib::state_manager_t &state() { return m_state; }

		template<typename O, typename C> void save(O &owner, C &state, const pstring &stname)
		{
			this->state().save_item(static_cast<void *>(&owner), state, pstring(owner.name()) + "." + stname);
		}
		template<typename O, typename C> void save(O &owner, C *state, const pstring &stname, const std::size_t count)
		{
			this->state().save_state_ptr(static_cast<void *>(&owner), pstring(owner.name()) + "." + stname, plib::state_manager_t::datatype_f<C>::f(), count, state);
		}

		virtual void reset();

		plib::dynlib &lib() { return *m_lib; }

		void print_stats() const;

		std::vector<plib::owned_ptr<core_device_t>>                           m_devices;
		/* sole use is to manage lifetime of net objects */
		std::vector<plib::owned_ptr<detail::net_t>>                           m_nets;
		/* sole use is to manage lifetime of family objects */
		std::vector<std::pair<pstring, std::unique_ptr<logic_family_desc_t>>> m_family_cache;

	private:
		plib::state_manager_t               m_state;
		/* mostly rw */
		netlist_time                        m_time;
		detail::queue_t                     m_queue;

		/* mostly ro */

		devices::NETLIB_NAME(mainclock) *    m_mainclock;
		devices::NETLIB_NAME(solver) *       m_solver;
		devices::NETLIB_NAME(gnd) *          m_gnd;
		devices::NETLIB_NAME(netlistparams) *m_params;

		pstring                             m_name;
		setup_t *                           m_setup;
		plib::plog_base<NL_DEBUG>           m_log;
		plib::dynlib *                      m_lib; // external lib needs to be loaded as long as netlist exists

		// performance
		nperftime_t     m_stat_mainloop;
		nperfcount_t    m_perf_out_processed;
		nperfcount_t    m_perf_inp_processed;
		nperfcount_t    m_perf_inp_active;
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
			const char *p[N];
		};
		object_array_t(core_device_t &dev, init names)
		{
			for (std::size_t i = 0; i<N; i++)
				this->emplace(i, dev, names.p[i]);
		}
	};

	// -----------------------------------------------------------------------------
	// inline implementations
	// -----------------------------------------------------------------------------

	template <class C, param_t::param_type_t T>
	inline void param_template_t<C, T>::setTo(const C &param)
	{
		if (m_param != param)
		{
			m_param = param;
			changed();
			device().update_param();
			if (device().needs_update_after_param_change())
				device().update_dev();
		}
	}

	inline bool detail::core_terminal_t::is_logic() const noexcept
	{
		return dynamic_cast<const logic_t *>(this) != nullptr;
	}

	inline bool detail::core_terminal_t::is_analog() const noexcept
	{
		return dynamic_cast<const analog_t *>(this) != nullptr;
	}

	inline bool detail::net_t::is_logic() const noexcept
	{
		return dynamic_cast<const logic_net_t *>(this) != nullptr;
	}

	inline bool detail::net_t::is_analog() const noexcept
	{
		return dynamic_cast<const analog_net_t *>(this) != nullptr;
	}

	inline void logic_input_t::inactivate()
	{
		if (!is_state(STATE_INP_PASSIVE))
		{
			set_state(STATE_INP_PASSIVE);
			net().dec_active(*this);
		}
	}

	inline void logic_input_t::activate()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().inc_active(*this);
			set_state(STATE_INP_ACTIVE);
		}
	}

	inline void logic_input_t::activate_hl()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().inc_active(*this);
			set_state(STATE_INP_HL);
		}
	}

	inline void logic_input_t::activate_lh()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().inc_active(*this);
			set_state(STATE_INP_LH);
		}
	}

	inline void detail::net_t::push_to_queue(const netlist_time delay) noexcept
	{
		if (!is_queued() && (num_cons() != 0))
		{
			m_time = netlist().time() + delay;
			m_in_queue = (m_active > 0);     /* queued ? */
			if (m_in_queue)
			{
				netlist().push_to_queue(*this, m_time);
			}
		}
	}

	inline void detail::net_t::reschedule_in_queue(const netlist_time delay) noexcept
	{
		if (is_queued())
			netlist().remove_from_queue(*this);

		m_time = netlist().time() + delay;
		m_in_queue = (m_active > 0);     /* queued ? */
		if (m_in_queue)
		{
			netlist().push_to_queue(*this, m_time);
		}
	}

	inline const analog_net_t & analog_t::net() const noexcept
	{
		return static_cast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net() noexcept
	{
		return static_cast<analog_net_t &>(core_terminal_t::net());
	}

	inline nl_double terminal_t::operator ()() const { return net().Q_Analog(); }

	inline logic_net_t & logic_t::net() noexcept
	{
		return *static_cast<logic_net_t *>(&core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const noexcept
	{
		return static_cast<const logic_net_t &>(core_terminal_t::net());
	}

	inline netlist_sig_t logic_input_t::Q() const noexcept
	{
		return net().Q();
	}

	inline nl_double analog_input_t::Q_Analog() const noexcept
	{
		return net().Q_Analog();
	}

	inline void analog_output_t::set_Q(const nl_double newQ) noexcept
	{
		if (newQ != net().Q_Analog())
		{
			net().m_cur_Analog = newQ;
			net().toggle_new_Q();
			net().push_to_queue(NLTIME_FROM_NS(1));
		}
	}

	inline void netlist_t::push_to_queue(detail::net_t &out, const netlist_time attime) noexcept
	{
		m_queue.push(attime, &out);
	}

	inline void netlist_t::remove_from_queue(detail::net_t &out) NL_NOEXCEPT
	{
		m_queue.remove(&out);
	}

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring name, const T &value)
	: m_value(value)
	{
		owner.netlist().save(owner, m_value, name);
	}

	template <typename T, std::size_t N>
	state_var<T[N]>::state_var(device_t &dev, const pstring name, const T & value)
	{
		dev.netlist().save(dev, m_value, name);
		for (std::size_t i=0; i<N; i++)
			m_value[i] = value;
	}

	inline netlist_t &detail::device_object_t::netlist()
	{
		return m_device.netlist();
	}

}


#endif /* NLBASE_H_ */
