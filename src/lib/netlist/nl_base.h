// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlbase.h
 *
 *  A mixed signal circuit simulation
 *
 *  D: Device
 *  O: Rail output (output)
 *  I: Infinite impedance input (input)
 *  T: Terminal (finite impedance)
 *
 *  +---+     +---+     +---+     +---+     +---+
 *  |   |     |   |     |   |     |   |     |   |
 *  | D |     | D |     | D |     | D |     | D |
 *  |   |     |   |     |   |     |   |     |   |
 *  +-O-+     +-I-+     +-I-+     +-T-+     +-T-+
 *    |         |         |         |         |
 *  +-+---------+---------+---------+---------+-+
 *  | rail net                                  |
 *  +-------------------------------------------+
 *
 *  A rail net is a net which is driven by exactly one output with an
 *  (idealized) internal resistance of zero.
 *  Ideally, it can deliver infinite current.
 *
 *  A infinite resistance input does not source or sink current.
 *
 *  Terminals source or sink finite (but never zero) current.
 *
 *  The system differentiates between analog and logic input and outputs and
 *  analog terminals. Analog and logic devices can not be connected to the
 *  same net. Instead, proxy devices are inserted automatically:
 *
 *  +---+     +---+
 *  |   |     |   |
 *  | D1|     | D2|
 *  | A |     | L |
 *  +-O-+     +-I-+
 *    |         |
 *  +-+---------+---+
 *  | rail net      |
 *  +---------------+
 *
 *  is converted into
 *              +----------+
 *              |          |
 *  +---+     +-+-+        |   +---+
 *  |   |     | L |  A-L   |   |   |
 *  | D1|     | D | Proxy  |   | D2|
 *  | A |     | A |        |   |   |
 *  +-O-+     +-I-+        |   +-I-+
 *    |         |          |     |
 *  +-+---------+--+     +-+-----+-------+
 *  | rail net (A) |     | rail net (L)  |
 *  +--------------|     +---------------+
 *
 *  This works both analog to logic as well as logic to analog.
 *
 *  The above is an advanced implementation of the existing discrete
 *  subsystem in MAME. Instead of relying on a fixed time-step, analog devices
 *  could either connect to fixed time-step clock or use an internal clock
 *  to update them. This would however introduce macro devices for RC, diodes
 *  and transistors again.
 *
 *  ============================================================================
 *
 *  Instead, the following approach in case of a pure terminal/input network
 *  is taken:
 *
 *  +---+     +---+     +---+     +---+     +---+
 *  |   |     |   |     |   |     |   |     |   |
 *  | D |     | D |     | D |     | D |     | D |
 *  |   |     |   |     |   |     |   |     |   |
 *  +-T-+     +-I-+     +-I-+     +-T-+     +-T-+
 *    |         |         |         |         |
 *   '+'        |         |        '-'       '-'
 *  +-+---------+---------+---------+---------+-+
 *  | Calculated net                            |
 *  +-------------------------------------------+
 *
 *  SPICE uses the following basic two terminal device:
 *
 *       (k)
 *  +-----T-----+
 *  |     |     |
 *  |  +--+--+  |
 *  |  |     |  |
 *  |  R     |  |
 *  |  R     |  |
 *  |  R     I  |
 *  |  |     I  |  Device n
 *  |  V+    I  |
 *  |  V     |  |
 *  |  V-    |  |
 *  |  |     |  |
 *  |  +--+--+  |
 *  |     |     |
 *  +-----T-----+
 *       (l)
 *
 *  This is a resistance in series to a voltage source and paralleled by a
 *  current source. This is suitable to model voltage sources, current sources,
 *  resistors, capacitors, inductances and diodes.
 *
 *  I(n,l) = - I(n,k) = ( V(k) - V - V(l) ) * (1/R(n)) + I(n)
 *
 *  Now, the sum of all currents for a given net must be 0:
 *
 *  Sum(n,I(n,l)) = 0 = sum(n, ( V(k) - V(n) - V(l) ) * (1/R(n)) + I(n) )
 *
 *  With G(n) = 1 / R(n) and sum(n, G(n)) = Gtot and k=k(n)
 *
 *  0 = - V(l) * Gtot + sum(n, (V(k(n)) - V(n)) * G(n) + I(n))
 *
 *  and with l=l(n) and fixed k
 *
 *  0 =  -V(k) * Gtot + sum(n, ( V(l(n) + V(n) ) * G(n) - I(n))
 *
 *  These equations represent a linear Matrix equation (with more math).
 *
 *  In the end the solution of the analog subsystem boils down to
 *
 *  (G - D) * V = I
 *
 *  with G being the conductance matrix, D a diagonal matrix with the total
 *  conductance on the diagonal elements, V the net voltage vector and I the
 *  current vector.
 *
 *  By using solely two terminal devices, we can simplify the whole calculation
 *  significantly. A BJT now is a four terminal device with two terminals being
 *  connected internally.
 *
 *  The system is solved using an iterative approach:
 *
 *  G * V - D * V = I
 *
 *  assuming V=Vn=Vo
 *
 *  Vn = D-1 * (I - G * Vo)
 *
 *  Each terminal thus has three properties:
 *
 *  a) Resistance
 *  b) Voltage source
 *  c) Current source/sink
 *
 *  Going forward, the approach can be extended e.g. to use a linear
 *  equation solver.
 *
 *  The formal representation of the circuit will stay the same, thus scales.
 *
 */

#ifndef NLBASE_H_
#define NLBASE_H_

#include <vector>
#include <memory>
#include <cmath>
#include <cstdint>

#include "nl_lists.h"
#include "nl_time.h"
#include "plib/pdynlib.h"
#include "plib/pstate.h"
#include "plib/pfmtlog.h"

// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------

using netlist_sig_t = std::uint32_t;

 //============================================================
 //  MACROS / New Syntax
 //============================================================

#define NETLIB_NAME(chip) nld_ ## chip

#define NETLIB_OBJECT_DERIVED(name, pclass)                                   \
class NETLIB_NAME(name) : public NETLIB_NAME(pclass)

#define NETLIB_OBJECT(name)                                                    \
class NETLIB_NAME(name) : public device_t

#define NETLIB_CONSTRUCTOR_DERIVED(cname, pclass)                              \
	private: family_setter_t m_famsetter;                                       \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name) \
	: NETLIB_NAME(pclass)(owner, name)

#define NETLIB_CONSTRUCTOR(cname)                                               \
	private: family_setter_t m_famsetter;                                       \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name) \
		: device_t(owner, name)

#define NETLIB_DESTRUCTOR(name) public: virtual ~NETLIB_NAME(name)()

#define NETLIB_CONSTRUCTOR_EX(cname, ...)                                       \
	private: family_setter_t m_famsetter;                                       \
	public: template <class CLASS> NETLIB_NAME(cname)(CLASS &owner, const pstring name, __VA_ARGS__) \
		: device_t(owner, name)

#define NETLIB_DYNAMIC() 														\
	public: virtual bool is_dynamic() const override { return true; }

#define NETLIB_TIMESTEP() 														\
	public: virtual bool is_timestep() const override { return true; } \
	public: virtual void step_time(const nl_double step) override

#define NETLIB_UPDATE_AFTER_PARAM_CHANGE() 						     			\
	public: virtual bool needs_update_after_param_change() const override { return true; }

#define NETLIB_FAMILY(family) , m_famsetter(*this, family)

#define NETLIB_UPDATE_TERMINALSI() public: virtual void update_terminals() override
#define NETLIB_UPDATEI() protected: virtual void update() NOEXCEPT override
#define NETLIB_UPDATE_PARAMI() public: virtual void update_param() override
#define NETLIB_RESETI() protected: virtual void reset() override

#define NETLIB_SUB(chip) nld_ ## chip
#define NETLIB_SUBXX(chip) std::unique_ptr< nld_ ## chip >

#define NETLIB_UPDATE(chip) void NETLIB_NAME(chip) :: update(void) NOEXCEPT

#define NETLIB_RESET(chip) void NETLIB_NAME(chip) :: reset(void)

#define NETLIB_STOP(chip) void NETLIB_NAME(chip) :: stop(void)

#define NETLIB_UPDATE_PARAM(chip) void NETLIB_NAME(chip) :: update_param(void)
#define NETLIB_FUNC_VOID(chip, name, params) void NETLIB_NAME(chip) :: name params

#define NETLIB_UPDATE_TERMINALS(chip) void NETLIB_NAME(chip) :: update_terminals(void)

//============================================================
//  Asserts
//============================================================

#if defined(MAME_DEBUG)
#define nl_assert(x)               do { if (1) if (!(x)) throw fatalerror_e(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#else
#define nl_assert(x)               do { if (0) if (!(x)) throw fatalerror_e(plib::pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#endif
#define nl_assert_always(x, msg)    do { if (!(x)) throw fatalerror_e(plib::pfmt("Fatal error: {1}\nCaused by assert: {2}:{3}: {4}")(msg)(__FILE__)(__LINE__)(#x)); } while (0)


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
		class nld_base_d_to_a_proxy;
	}

	//============================================================
	//  Exceptions
	//============================================================

	class fatalerror_e : public plib::pexception
	{
	public:
		fatalerror_e(const pstring &text) : plib::pexception(text) { }
		virtual ~fatalerror_e() throw() {}
	};

	class logic_output_t;
	class analog_net_t;
	class logic_net_t;
	class net_t;
	class setup_t;
	class netlist_t;
	class core_device_t;
	class param_model_t;

	// -----------------------------------------------------------------------------
	// model_map_t
	// -----------------------------------------------------------------------------

	using model_map_t = plib::hashmap_t<pstring, pstring>;

	// -----------------------------------------------------------------------------
	// logic_family_t
	// -----------------------------------------------------------------------------

	class logic_family_desc_t
	{
	public:
		logic_family_desc_t() {}
		virtual ~logic_family_desc_t() {}
		virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_t &anetlist, const pstring &name,
				logic_output_t *proxied) const = 0;

		nl_double m_low_thresh_V;
		nl_double m_high_thresh_V;
		nl_double m_low_V;
		nl_double m_high_V;
		nl_double m_R_low;
		nl_double m_R_high;
	};

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

	/* Terminals inherit the family description from the device
	 * The default is the ttl family, but any device can override the family.
	 * For individual terminals, these can be overwritten as well.
	 *
	 * Only devices of type GENERIC should have a family description entry
	 */


	const logic_family_desc_t *family_TTL();
	const logic_family_desc_t *family_CD4XXX();


	// -----------------------------------------------------------------------------
	// object_t
	// -----------------------------------------------------------------------------

	class object_t : public plib::pstate_interface_t<object_t>
	{
		P_PREVENT_COPYING(object_t)
	public:
		enum type_t {
			TERMINAL = 0,
			INPUT    = 1,
			OUTPUT   = 2,
			PARAM    = 3,
			NET      = 4,
			DEVICE   = 5,
			QUEUE    = 6
		};

		object_t(netlist_t &nl, const pstring &aname, const type_t atype);
		~object_t();

		const pstring &name() const;

		plib::pstate_manager_t &state_manager();

		type_t type() const { return m_objtype; }
		bool is_type(const type_t atype) const { return (m_objtype == atype); }

		netlist_t & netlist() { return m_netlist; }
		const netlist_t & netlist() const { return m_netlist; }

	private:
		netlist_t & m_netlist;
		pstring m_name;
		const type_t m_objtype;

	public:
	    void * operator new (size_t size, void *ptr) { return ptr; }
	    void operator delete (void *ptr, void *) {  }
	    void * operator new (size_t size);
	    void operator delete (void * mem);
	};

	// -----------------------------------------------------------------------------
	// device_object_t
	// -----------------------------------------------------------------------------

	class device_object_t : public object_t
	{
		P_PREVENT_COPYING(device_object_t)
	public:
		device_object_t(core_device_t &dev, const pstring &aname, const type_t atype);
		core_device_t &device() const { return m_device; }
	private:
		core_device_t & m_device;
	};


	// -----------------------------------------------------------------------------
	// core_terminal_t
	// -----------------------------------------------------------------------------

	class core_terminal_t : public device_object_t, public plib::linkedlist_t<core_terminal_t>::element_t
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

		bool is_logic() const;
		bool is_analog() const;

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
		state_e m_state;
	};

	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public core_terminal_t
	{
	public:

		analog_t(core_device_t &dev, const pstring &aname, const type_t atype)
		: core_terminal_t(dev, aname, atype)
		{
		}

		const analog_net_t & net() const;
		analog_net_t & net();
	};

	// -----------------------------------------------------------------------------
	// terminal_t
	// -----------------------------------------------------------------------------

	class terminal_t : public analog_t
	{
		P_PREVENT_COPYING(terminal_t)
	public:

		terminal_t(core_device_t &dev, const pstring &aname);

		terminal_t *m_otherterm;

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

	private:
		void set_ptr(nl_double *ptr, const nl_double val)
		{
			if (ptr != nullptr && *ptr != val)
			{
				*ptr = val;
			}
		}

		nl_double *m_Idr1; // drive current
		nl_double *m_go1;  // conductance for Voltage from other term
		nl_double *m_gt1;  // conductance for total conductance

	};


	// -----------------------------------------------------------------------------
	// logic_t
	// -----------------------------------------------------------------------------

	class logic_t : public core_terminal_t, public logic_family_t
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

		logic_net_t & net();
		const logic_net_t &  net() const;

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

		netlist_sig_t Q() const;

		void inactivate();
		void activate();
		void activate_hl();
		void activate_lh();

	};

	// -----------------------------------------------------------------------------
	// analog_input_t
	// -----------------------------------------------------------------------------

	class analog_input_t : public analog_t
	{
	public:
		analog_input_t(core_device_t &dev, const pstring &aname);

		nl_double Q_Analog() const;

	};

	// -----------------------------------------------------------------------------
	// net_t
	// -----------------------------------------------------------------------------

	class net_t : public object_t
	{
		P_PREVENT_COPYING(net_t)
	public:

		net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr = nullptr);
		virtual ~net_t();

		void reset();

		void register_con(core_terminal_t &terminal);
		void merge_net(net_t *othernet);

		bool is_logic() const;
		bool is_analog() const;

		void toggle_new_Q()				{ m_new_Q ^= 1;   }

		void push_to_queue(const netlist_time delay) NOEXCEPT;
		void reschedule_in_queue(const netlist_time delay) NOEXCEPT;
		bool is_queued() const { return m_in_queue == 1; }

		void update_devs();

		const netlist_time time() const { return m_time; }
		void set_time(const netlist_time ntime) { m_time = ntime; }

		bool isRailNet() const { return !(m_railterminal == nullptr); }
		core_terminal_t & railterminal() const { return *m_railterminal; }

		std::size_t num_cons() const { return m_core_terms.size(); }

		void inc_active(core_terminal_t &term);
		void dec_active(core_terminal_t &term);

		void rebuild_list();     /* rebuild m_list after a load */

		void move_connections(net_t *new_net);

		std::vector<core_terminal_t *> m_core_terms; // save post-start m_list ...

	protected:
		netlist_sig_t m_new_Q;
		netlist_sig_t m_cur_Q;

		netlist_time  m_time;
		INT32         m_active;
		UINT8    	  m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

	private:
		plib::linkedlist_t<core_terminal_t> m_list_active;
		core_terminal_t * m_railterminal;

	public:
		// FIXME: Have to fix the public at some time
		nl_double m_cur_Analog;

	};

	class logic_net_t : public net_t
	{
		P_PREVENT_COPYING(logic_net_t)
	public:

		logic_net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr = nullptr);
		virtual ~logic_net_t() { };

		netlist_sig_t Q() const { return m_cur_Q; }
		netlist_sig_t new_Q() const 	{ return m_new_Q; }
		void initial(const netlist_sig_t val) { m_cur_Q = m_new_Q = val; }

		void set_Q(const netlist_sig_t newQ, const netlist_time delay) NOEXCEPT
		{
			if (newQ != m_new_Q)
			{
				m_new_Q = newQ;
				push_to_queue(delay);
			}
		}

		void set_Q_time(const netlist_sig_t newQ, const netlist_time at)
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

	class analog_net_t : public net_t
	{
		P_PREVENT_COPYING(analog_net_t)
	public:

		using list_t =  std::vector<analog_net_t *>;

		analog_net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr = nullptr);

		virtual ~analog_net_t() { };

		nl_double Q_Analog() const { return m_cur_Analog; }
		nl_double &Q_Analog_state_ptr() { return m_cur_Analog; }

		//FIXME: needed by current solver code
		devices::matrix_solver_t *solver() { return m_solver; }
		void set_solver(devices::matrix_solver_t *solver) { m_solver = solver; }

		bool already_processed(std::vector<list_t> &groups);
		void process_net(std::vector<list_t> &groups);

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

		void set_Q(const netlist_sig_t newQ, const netlist_time delay) NOEXCEPT
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

		void initial(const nl_double val);
		void set_Q(const nl_double newQ);

	private:
		analog_net_t m_my_net;
	};

	// -----------------------------------------------------------------------------
	// param_t
	// -----------------------------------------------------------------------------

	class device_t;

	class param_t : public device_object_t
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

		operator const C() const { return Value(); }

		void setTo(const C &param);
		void initial(const C &val) { m_param = val; }
		C Value() const { return m_param;   }

	protected:
		virtual void changed() { }
		C m_param;
	private:
	};

	using param_double_t = param_template_t<nl_double, param_t::DOUBLE>;
	using param_int_t = param_template_t<int, param_t::INTEGER>;
	using param_str_t = param_template_t<pstring, param_t::STRING>;

	using param_logic_t = param_template_t<int, param_t::INTEGER>;

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

	class core_device_t : public object_t, public logic_family_t
	{
		P_PREVENT_COPYING(core_device_t)
	public:
		core_device_t(netlist_t &owner, const pstring &name);
		core_device_t(core_device_t &owner, const pstring &name);

		virtual ~core_device_t();

		void update_dev()
		{
			begin_timing(stat_total_time);
			inc_stat(stat_update_count);
			do_update();
			end_timing(stat_total_time);
		}

		void do_update() NOEXCEPT
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

		void do_inc_active() { inc_active();  }
		void do_dec_active() { dec_active(); }
		void do_reset() { reset(); }

		netlist_sig_t INPLOGIC_PASSIVE(logic_input_t &inp);
		netlist_sig_t INPLOGIC(const logic_input_t &inp) const
		{
			nl_assert(inp.state() != logic_t::STATE_INP_PASSIVE);
			return inp.Q();
		}

		void OUTLOGIC(logic_output_t &out, const netlist_sig_t val, const netlist_time delay) NOEXCEPT;
		nl_double INPANALOG(const analog_input_t &inp) const { return inp.Q_Analog(); }
		nl_double TERMANALOG(const terminal_t &term) const { return term.net().Q_Analog(); }
		void OUTANALOG(analog_output_t &out, const nl_double val) { out.set_Q(val); }

	#if (NL_KEEP_STATISTICS)
		/* stats */
		plib::ticks_t stat_total_time;
		INT32 stat_update_count;
		INT32 stat_call_count;
	#endif

	protected:

		virtual void update() NOEXCEPT { }
		virtual void inc_active() {  }
		virtual void dec_active() {  }
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
	// param_ref_t
	// -----------------------------------------------------------------------------

	struct param_ref_t
	{
		param_ref_t(const pstring name, core_device_t &device, param_t &param)
		: m_name(name)
		, m_device(device)
		, m_param(param)
		{ }
		pstring m_name;
		core_device_t &m_device;
		param_t &m_param;
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

		void register_subalias(const pstring &name, core_terminal_t &term);
		void register_subalias(const pstring &name, const pstring &aliased);

		void connect_late(const pstring &t1, const pstring &t2);
		void connect_late(core_terminal_t &t1, core_terminal_t &t2);
		void connect_post_start(core_terminal_t &t1, core_terminal_t &t2);

		std::vector<pstring> m_terminals;

	protected:

		NETLIB_UPDATEI() { }
		NETLIB_UPDATE_TERMINALSI() { }

	private:
		void register_p(const pstring &name, object_t &obj);
		void register_sub_p(device_t &dev);
	};

	// -----------------------------------------------------------------------------
	// family_setter_t
	// -----------------------------------------------------------------------------

	struct family_setter_t
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

	class queue_t : public timed_queue<net_t *, netlist_time>,
							public object_t,
							public plib::pstate_manager_t::callback_t
	{
	public:
		queue_t(netlist_t &nl);

	protected:

		void register_state(plib::pstate_manager_t &manager, const pstring &module) override;
		void on_pre_save() override;
		void on_post_load() override;

	private:
		struct names_t { char m_buf[64]; };
		int m_qsize;
		std::vector<netlist_time::INTERNALTYPE> m_times;
		std::vector<names_t> m_names;
	};

	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------


	class netlist_t : public plib::pstate_manager_t, public plib::plog_dispatch_intf //, public device_owner_t
	{
		P_PREVENT_COPYING(netlist_t)
	public:

		netlist_t(const pstring &aname);
		virtual ~netlist_t();

		pstring name() const { return m_name; }

		void start();
		void stop();

		 const queue_t &queue() const { return m_queue; }
		 queue_t &queue() { return m_queue; }
		 const netlist_time time() const { return m_time; }
		 devices::NETLIB_NAME(solver) *solver() const { return m_solver; }
		 devices::NETLIB_NAME(gnd) *gnd() const { return m_gnd; }
		nl_double gmin() const;

		void push_to_queue(net_t &out, const netlist_time attime) NOEXCEPT;
		void remove_from_queue(net_t &out);

		void process_queue(const netlist_time &delta);
		void abort_current_queue_slice() { m_queue.retime(m_time, nullptr); }

		bool use_deactivate() const { return m_use_deactivate; }

		void rebuild_lists(); /* must be called after post_load ! */

		void set_setup(setup_t *asetup) { m_setup = asetup;  }
		setup_t &setup() { return *m_setup; }

		net_t *find_net(const pstring &name);

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

		virtual void reset();

		plib::dynlib &lib() { return *m_lib; }

		void print_stats() const;

		std::vector<plib::owned_ptr<core_device_t>> m_devices;

		/* sole use is to manage lifetime of net objects */
		std::vector<std::shared_ptr<net_t>> m_nets;

		/* sole use is to manage lifetime of family objects */
		std::vector<std::pair<pstring, std::unique_ptr<logic_family_desc_t>>> m_family_cache;

protected:

	#if (NL_KEEP_STATISTICS)
		// performance
		int m_perf_out_processed;
		int m_perf_inp_processed;
		int m_perf_inp_active;
	#endif

	private:
		/* mostly rw */
		netlist_time                m_time;
		queue_t                     m_queue;
		/* mostly rw */
		bool                        m_use_deactivate;

		devices::NETLIB_NAME(mainclock) *    m_mainclock;
		devices::NETLIB_NAME(solver) *       m_solver;
		devices::NETLIB_NAME(gnd) *          m_gnd;
		devices::NETLIB_NAME(netlistparams) *m_params;

		pstring m_name;
		setup_t *m_setup;
		plib::plog_base<NL_DEBUG> m_log;
		plib::dynlib *m_lib;                 // external lib needs to be loaded as long as netlist exists
	};

	template <typename T>
	struct state_var
	{
	public:
		state_var(device_t &d, const pstring name, T value)
		: m_value(value)
		{
			d.save(m_value, name);
		}

		state_var(const state_var &rhs) NOEXCEPT = default;
		state_var(state_var &&rhs) NOEXCEPT = default;
		state_var &operator=(const state_var rhs) { m_value = rhs.m_value; return *this; }
		state_var &operator=(const T rhs) { m_value = rhs; return *this; }


		operator T &() { return m_value; }
		operator const T &() const { return m_value; }
	private:
		T m_value;
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

	inline plib::pstate_manager_t &object_t::state_manager()
	{
		return m_netlist;
	}

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

	inline bool core_terminal_t::is_logic() const
	{
		return dynamic_cast<const logic_t *>(this) != nullptr;
	}

	inline bool core_terminal_t::is_analog() const
	{
		return dynamic_cast<const analog_t *>(this) != nullptr;
	}

	inline bool net_t::is_logic() const
	{
		return dynamic_cast<const logic_net_t *>(this) != nullptr;
	}

	inline bool net_t::is_analog() const
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

	inline void net_t::push_to_queue(const netlist_time delay) NOEXCEPT
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

	inline void net_t::reschedule_in_queue(const netlist_time delay) NOEXCEPT
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

	inline const analog_net_t & analog_t::net() const
	{
		return static_cast<const analog_net_t &>(core_terminal_t::net());
	}

	inline analog_net_t & analog_t::net()
	{
		return static_cast<analog_net_t &>(core_terminal_t::net());
	}

	inline logic_net_t & logic_t::net()
	{
		return *static_cast<logic_net_t *>(&core_terminal_t::net());
	}

	inline const logic_net_t & logic_t::net() const
	{
		return static_cast<const logic_net_t &>(core_terminal_t::net());
	}

	inline netlist_sig_t logic_input_t::Q() const
	{
		return net().Q();
	}

	inline nl_double analog_input_t::Q_Analog() const
	{
		return net().Q_Analog();
	}

	inline void analog_output_t::set_Q(const nl_double newQ)
	{
		if (newQ != net().Q_Analog())
		{
			net().m_cur_Analog = newQ;
			net().toggle_new_Q();
			net().push_to_queue(NLTIME_FROM_NS(1));
		}
	}

	inline void netlist_t::push_to_queue(net_t &out, const netlist_time attime) NOEXCEPT
	{
		m_queue.push(attime, &out);
	}

	inline void netlist_t::remove_from_queue(net_t &out)
	{
		m_queue.remove(&out);
	}

	inline void core_device_t::OUTLOGIC(logic_output_t &out, const netlist_sig_t val, const netlist_time delay) NOEXCEPT
	{
		out.set_Q(val, delay);
	}


}


#endif /* NLBASE_H_ */
