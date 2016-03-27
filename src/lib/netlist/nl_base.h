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

#include "nl_lists.h"
#include "nl_time.h"
#include "nl_util.h"
#include "plib/pstate.h"
#include "plib/pfmtlog.h"

// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------

//typedef UINT8 netlist_sig_t;

/*
 *  unsigned int would be a 20% speed increase over UINT8 for pong.
 *  For breakout it causes a slight decrease.
 *
 */
typedef unsigned int netlist_sig_t;

//============================================================
//  MACROS / netlist devices
//============================================================

#define NETLIB_NAMESPACE_DEVICES_START()    namespace netlist { namespace devices {
#define NETLIB_NAMESPACE_DEVICES_END()  }}

#define NETLIB_NAME(_chip) nld_ ## _chip

#define NETLIB_NAME_STR_S(_s) # _s
#define NETLIB_NAME_STR(_chip) NETLIB_NAME_STR_S(nld_ ## _chip)

#define NETLIB_UPDATE(_chip) ATTR_HOT void NETLIB_NAME(_chip) :: update(void)
#define NETLIB_START(_chip) ATTR_COLD void NETLIB_NAME(_chip) :: start(void)

#define NETLIB_RESET(_chip) ATTR_COLD void NETLIB_NAME(_chip) :: reset(void)

#define NETLIB_STOP(_chip) ATTR_COLD void NETLIB_NAME(_chip) :: stop(void)

#define NETLIB_UPDATE_PARAM(_chip) ATTR_HOT void NETLIB_NAME(_chip) :: update_param(void)
#define NETLIB_FUNC_VOID(_chip, _name, _params) ATTR_HOT void NETLIB_NAME(_chip) :: _name _params

#define NETLIB_UPDATE_TERMINALS(_chip) ATTR_HOT void NETLIB_NAME(_chip) :: update_terminals(void)
#define NETLIB_UPDATE_TERMINALSI() ATTR_HOT void update_terminals(void) override
#define NETLIB_UPDATEI() ATTR_HOT void update(void)

#define NETLIB_DEVICE_BASE(_name, _pclass, _extra, _priv)                       \
	class _name : public _pclass                                                \
	{                                                                           \
	public:                                                                     \
		_name()                                                                 \
		: _pclass()    { }                                                      \
	protected:                                                                  \
		_extra                                                                  \
		ATTR_HOT void update() override;                                        \
		ATTR_HOT void start() override;                                         \
		ATTR_HOT void reset() override;                                         \
		_priv                                                                   \
	}

#define NETLIB_DEVICE_DERIVED_PURE(_name, _pclass)                            \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), NETLIB_NAME(_pclass), protected:, private:)

#define NETLIB_DEVICE_DERIVED(_name, _pclass, _priv)                            \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), NETLIB_NAME(_pclass), protected:, _priv)

#define NETLIB_DEVICE(_name, _priv)                                             \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), device_t, protected:, _priv)

#define NETLIB_SUBDEVICE(_name, _priv)                                          \
	class NETLIB_NAME(_name) : public device_t                                  \
	{                                                                           \
	public:                                                                     \
		NETLIB_NAME(_name) ()                                                   \
		: device_t()                                                            \
			{ }                                                                 \
	/*protected:*/                                                              \
		ATTR_HOT void update() override;                                        \
		ATTR_HOT void start() override;                                         \
		ATTR_HOT void reset() override;                                         \
	public:                                                                     \
		_priv                                                                   \
	}

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)                                 \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), device_t,                        \
			ATTR_HOT void update_param() override;                              \
		, _priv)

#define NETLIB_DEVICE_WITH_PARAMS_DERIVED(_name, _pclass, _priv)                \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), NETLIB_NAME(_pclass),            \
			ATTR_HOT void update_param() override;                              \
		, _priv)

#define NETLIB_LOGIC_FAMILY(_fam)                                               \
virtual logic_family_desc_t *default_logic_family() override                    \
{                                                                               \
	return family_ ## _fam;                                                     \
}


//============================================================
//  Asserts
//============================================================

#if defined(MAME_DEBUG)
#define nl_assert(x)               do { if (1) if (!(x)) throw fatalerror_e(pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#else
#define nl_assert(x)               do { if (0) if (!(x)) throw fatalerror_e(pfmt("assert: {1}:{2}: {3}")(__FILE__)(__LINE__)(#x) ); } while (0)
#endif
#define nl_assert_always(x, msg)    do { if (!(x)) throw fatalerror_e(pfmt("Fatal error: {1}\nCaused by assert: {2}:{3}: {4}")(msg)(__FILE__)(__LINE__)(#x)); } while (0)


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

	class fatalerror_e : public pexception
	{
	public:
		fatalerror_e(const pstring &text) : pexception(text) { }
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

	typedef phashmap_t<pstring, pstring> model_map_t;

	// -----------------------------------------------------------------------------
	// logic_family_t
	// -----------------------------------------------------------------------------

	class logic_family_desc_t
	{
	public:
		logic_family_desc_t() : m_is_static(false) {}
		virtual ~logic_family_desc_t() {}
		virtual devices::nld_base_d_to_a_proxy *create_d_a_proxy(logic_output_t *proxied) const = 0;

		nl_double m_low_thresh_V;
		nl_double m_high_thresh_V;
		nl_double m_low_V;
		nl_double m_high_V;
		nl_double m_R_low;
		nl_double m_R_high;

		bool m_is_static;
	};

	class logic_family_t
	{
	public:

		logic_family_t() : m_logic_family(NULL) {}
		~logic_family_t() { }

		ATTR_HOT  logic_family_desc_t *logic_family() const { return m_logic_family; }
		ATTR_COLD void set_logic_family(logic_family_desc_t *fam) { m_logic_family = fam; }

	protected:
		logic_family_desc_t *m_logic_family;
	};

	/* Terminals inherit the family description from the device
	 * The default is the ttl family, but any device can override the family.
	 * For individual terminals, these can be overwritten as well.
	 *
	 * Only devices of type GENERIC should have a family description entry
	 */


	extern logic_family_desc_t *family_TTL;
	extern logic_family_desc_t *family_CD4XXX;


	// -----------------------------------------------------------------------------
	// object_t
	// -----------------------------------------------------------------------------

	class object_t
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
			NETLIST   = 6,
			QUEUE   = 7
		};
		enum family_t {
			// Terminal families
			LOGIC,
			ANALOG,
			// Device families
			GENERIC,    // <== devices usually fall into this category
			TWOTERM,    // Generic twoterm ...
			RESISTOR,   // Resistor
			CAPACITOR,  // Capacitor
			DIODE,      // Diode
			DUMMY,      // DUMMY device without function
			FRONTIER,   // Net frontier
			BJT_EB,     // BJT(Ebers-Moll)
			BJT_SWITCH, // BJT(Switch)
			VCVS,       // Voltage controlled voltage source
			VCCS,       // Voltage controlled current source
			LVCCS,      // Voltage controlled current source (Current limited)
			CCCS,       // Current controlled current source
			VS,         // Voltage Source
			CS,         // Current Source
			GND         // GND device
		};

		ATTR_COLD object_t(const type_t atype, const family_t afamily);

		virtual ~object_t();

		ATTR_COLD void init_object(netlist_t &nl, const pstring &aname);
		ATTR_COLD bool isInitialized() { return (m_netlist != NULL); }

		ATTR_COLD const pstring &name() const;

		PSTATE_INTERFACE_DECL()

		ATTR_HOT  type_t type() const { return m_objtype; }
		ATTR_HOT  family_t family() const { return m_family; }

		ATTR_HOT  bool isType(const type_t atype) const { return (m_objtype == atype); }
		ATTR_HOT  bool isFamily(const family_t afamily) const { return (m_family == afamily); }

		ATTR_HOT  netlist_t & netlist() { return *m_netlist; }
		ATTR_HOT  const netlist_t & netlist() const { return *m_netlist; }

		ATTR_COLD void  do_reset()
		{
			reset();
		}

	protected:

		virtual void reset() = 0;
		// must call parent save_register !
		virtual void save_register() { };

	private:
		pstring m_name;
		const type_t m_objtype;
		const family_t m_family;
		netlist_t * m_netlist;
	};

	// -----------------------------------------------------------------------------
	// device_object_t
	// -----------------------------------------------------------------------------

	class device_object_t : public object_t
	{
		P_PREVENT_COPYING(device_object_t)
	public:
		ATTR_COLD device_object_t(const type_t atype, const family_t afamily);

		ATTR_COLD void init_object(core_device_t &dev, const pstring &aname);

		ATTR_HOT core_device_t &device() const { return *m_device; }
	private:
		core_device_t * m_device;
	};


	// -----------------------------------------------------------------------------
	// core_terminal_t
	// -----------------------------------------------------------------------------

	class core_terminal_t : public device_object_t, public plinkedlist_element_t<core_terminal_t>
	{
		P_PREVENT_COPYING(core_terminal_t)
	public:

		typedef pvector_t<core_terminal_t *> list_t;

		/* needed here ... */

		enum state_e {
			STATE_INP_PASSIVE = 0,
			STATE_INP_ACTIVE = 1,
			STATE_INP_HL = 2,
			STATE_INP_LH = 4,
			STATE_OUT = 128,
			STATE_NONEX = 256
		};


		ATTR_COLD core_terminal_t(const type_t atype, const family_t afamily);

		ATTR_COLD void set_net(net_t &anet);
		ATTR_COLD  void clear_net() { m_net = NULL; }
		ATTR_HOT  bool has_net() const { return (m_net != NULL); }

		ATTR_HOT  const net_t & net() const { return *m_net;}
		ATTR_HOT  net_t & net() { return *m_net;}

		ATTR_HOT  bool is_state(const state_e astate) const { return (m_state == astate); }
		ATTR_HOT  const state_e &state() const { return m_state; }
		ATTR_HOT  void set_state(const state_e astate)
		{
			nl_assert(astate != STATE_NONEX);
			m_state = astate;
		}

	protected:
		virtual void save_register() override
		{
			save(NLNAME(m_state));
			device_object_t::save_register();
		}

	private:
		net_t *  m_net;
		state_e m_state;
	};

	// -----------------------------------------------------------------------------
	// analog_t
	// -----------------------------------------------------------------------------

	class analog_t : public core_terminal_t
	{
	public:


		ATTR_COLD analog_t(const type_t atype)
			: core_terminal_t(atype, ANALOG)
		{
		}

		virtual void reset() override
		{
		}

		ATTR_HOT inline const analog_net_t & net() const;
		ATTR_HOT inline analog_net_t & net();

	protected:

	private:
	};

	// -----------------------------------------------------------------------------
	// terminal_t
	// -----------------------------------------------------------------------------

	class terminal_t : public analog_t
	{
		P_PREVENT_COPYING(terminal_t)
	public:

		typedef pvector_t<terminal_t *> list_t;

		ATTR_COLD terminal_t();

		terminal_t *m_otherterm;

		ATTR_HOT  void set(const nl_double G)
		{
			set_ptr(m_Idr1, 0);
			set_ptr(m_go1, G);
			set_ptr(m_gt1, G);
		}

		ATTR_HOT  void set(const nl_double GO, const nl_double GT)
		{
			set_ptr(m_Idr1, 0);
			set_ptr(m_go1, GO);
			set_ptr(m_gt1, GT);
		}

		ATTR_HOT  void set(const nl_double GO, const nl_double GT, const nl_double I)
		{
			set_ptr(m_Idr1, I);
			set_ptr(m_go1, GO);
			set_ptr(m_gt1, GT);
		}

		ATTR_HOT void schedule_solve();
		ATTR_HOT void schedule_after(const netlist_time &after);

		void set_ptrs(nl_double *gt, nl_double *go, nl_double *Idr)
		{
			m_gt1 = gt;
			m_go1 = go;
			m_Idr1 = Idr;
		}

	protected:
		virtual void save_register() override;

		virtual void reset() override;
	private:
		ATTR_HOT void set_ptr(nl_double *ptr, const nl_double val)
		{
			if (ptr != NULL && *ptr != val)
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


		ATTR_COLD logic_t(const type_t atype)
			: core_terminal_t(atype, LOGIC), logic_family_t(),
				m_proxy(NULL)
		{
		}

		virtual void reset() override
		{
		}

		ATTR_COLD bool has_proxy() const { return (m_proxy != NULL); }
		ATTR_COLD devices::nld_base_proxy *get_proxy() const  { return m_proxy; }
		ATTR_COLD void set_proxy(devices::nld_base_proxy *proxy) { m_proxy = proxy; }

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
		ATTR_COLD logic_input_t()
			: logic_t(INPUT)
		{
			set_state(STATE_INP_ACTIVE);
		}

		ATTR_HOT  netlist_sig_t Q() const;
		ATTR_HOT  netlist_sig_t last_Q() const;

		ATTR_HOT  void inactivate();
		ATTR_HOT  void activate();
		ATTR_HOT  void activate_hl();
		ATTR_HOT  void activate_lh();

	protected:
		virtual void reset() override
		{
			logic_t::reset();
			set_state(STATE_INP_ACTIVE);
		}

	};

	// -----------------------------------------------------------------------------
	// analog_input_t
	// -----------------------------------------------------------------------------

	class analog_input_t : public analog_t
	{
	public:
		ATTR_COLD analog_input_t()
			: analog_t(INPUT)
		{
			set_state(STATE_INP_ACTIVE);
		}

		ATTR_HOT  nl_double Q_Analog() const;

	protected:
		virtual void reset() override
		{
			analog_t::reset();
			set_state(STATE_INP_ACTIVE);
		}
	};

	// -----------------------------------------------------------------------------
	// net_net_t
	// -----------------------------------------------------------------------------

	class net_t : public object_t
	{
		P_PREVENT_COPYING(net_t)
	public:

		typedef pvector_t<net_t *> list_t;

		ATTR_COLD net_t(const family_t afamily);
		virtual ~net_t();

		ATTR_COLD void init_object(netlist_t &nl, const pstring &aname);

		ATTR_COLD void register_con(core_terminal_t &terminal);
		ATTR_COLD void merge_net(net_t *othernet);
		ATTR_COLD void register_railterminal(core_terminal_t &mr);

		ATTR_HOT  logic_net_t & as_logic();
		ATTR_HOT  const logic_net_t &  as_logic() const;

		ATTR_HOT  analog_net_t & as_analog();
		ATTR_HOT  const analog_net_t & as_analog() const;

		ATTR_HOT void update_devs();

		ATTR_HOT  const netlist_time &time() const { return m_time; }
		ATTR_HOT  void set_time(const netlist_time &ntime) { m_time = ntime; }

		ATTR_HOT  bool isRailNet() const { return !(m_railterminal == NULL); }
		ATTR_HOT  core_terminal_t & railterminal() const { return *m_railterminal; }

		ATTR_HOT  void push_to_queue(const netlist_time &delay);
		ATTR_HOT  void reschedule_in_queue(const netlist_time &delay);
		ATTR_HOT bool  is_queued() const { return m_in_queue == 1; }

		ATTR_HOT  int num_cons() const { return m_core_terms.size(); }

		ATTR_HOT void inc_active(core_terminal_t &term);
		ATTR_HOT void dec_active(core_terminal_t &term);

		ATTR_COLD void rebuild_list();     /* rebuild m_list after a load */

		ATTR_COLD void move_connections(net_t *new_net);

		pvector_t<core_terminal_t *> m_core_terms; // save post-start m_list ...

		ATTR_HOT  void set_Q_time(const netlist_sig_t &newQ, const netlist_time &at)
		{
			if (newQ != m_new_Q)
			{
				m_in_queue = 0;
				m_time = at;
			}
			m_cur_Q = m_new_Q = newQ;
		}

	protected:  //FIXME: needed by current solver code

		virtual void save_register() override;
		virtual void reset() override;

		netlist_sig_t m_new_Q;
		netlist_sig_t m_cur_Q;

	private:

		core_terminal_t * m_railterminal;
		plinkedlist_t<core_terminal_t> m_list_active;

		netlist_time m_time;
		INT32        m_active;
		UINT8        m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

	public:
		// We have to have those on one object. Dividing those does lead
		// to a significant performance hit
		// FIXME: Have to fix the public at some time
		nl_double m_cur_Analog;

	};

	class logic_net_t : public net_t
	{
		P_PREVENT_COPYING(logic_net_t)
	public:

		typedef pvector_t<logic_net_t *> list_t;

		ATTR_COLD logic_net_t();
		virtual ~logic_net_t() { };

		ATTR_HOT  netlist_sig_t Q() const
		{
			return m_cur_Q;
		}

		ATTR_HOT  netlist_sig_t new_Q() const
		{
			return m_new_Q;
		}

		ATTR_HOT  void set_Q(const netlist_sig_t &newQ, const netlist_time &delay)
		{
			if (newQ !=  m_new_Q)
			{
				m_new_Q = newQ;
				push_to_queue(delay);
			}
		}

		ATTR_HOT  void toggle_new_Q()
		{
			m_new_Q ^= 1;
		}

		ATTR_COLD void initial(const netlist_sig_t val)
		{
			m_cur_Q = val;
			m_new_Q = val;
		}

		/* internal state support
		 * FIXME: get rid of this and implement export/import in MAME
		 */
		ATTR_COLD  netlist_sig_t &Q_state_ptr()
		{
			return m_cur_Q;
		}

	protected:  //FIXME: needed by current solver code

		virtual void save_register() override;
		virtual void reset() override;


	private:

	public:

	};

	class analog_net_t : public net_t
	{
		P_PREVENT_COPYING(analog_net_t)
	public:

		typedef pvector_t<analog_net_t *> list_t;

		ATTR_COLD analog_net_t();
		virtual ~analog_net_t() { };

		ATTR_HOT const nl_double &Q_Analog() const
		{
			return m_cur_Analog;
		}

		ATTR_COLD  nl_double &Q_Analog_state_ptr()
		{
			return m_cur_Analog;
		}

		ATTR_HOT devices::matrix_solver_t *solver() { return m_solver; }

		ATTR_COLD bool already_processed(pvector_t<list_t> &groups);
		ATTR_COLD void process_net(pvector_t<list_t> &groups);

	protected:

		virtual void save_register() override;
		virtual void reset() override;


	private:

	public:
		nl_double m_DD_n_m_1;
		nl_double m_h_n_m_1;

		//FIXME: needed by current solver code
		devices::matrix_solver_t *m_solver;
	};

	// -----------------------------------------------------------------------------
	// net_output_t
	// -----------------------------------------------------------------------------

	class logic_output_t : public logic_t
	{
		P_PREVENT_COPYING(logic_output_t)
	public:

		ATTR_COLD logic_output_t();

		ATTR_COLD void init_object(core_device_t &dev, const pstring &aname);
		virtual void reset() override
		{
			set_state(STATE_OUT);
		}

		ATTR_COLD void initial(const netlist_sig_t val);

		ATTR_HOT  void set_Q(const netlist_sig_t newQ, const netlist_time &delay)
		{
			net().as_logic().set_Q(newQ, delay);
		}

	private:
		logic_net_t m_my_net;
	};

	class analog_output_t : public analog_t
	{
		P_PREVENT_COPYING(analog_output_t)
	public:

		ATTR_COLD analog_output_t();

		ATTR_COLD void init_object(core_device_t &dev, const pstring &aname);
		virtual void reset() override
		{
			set_state(STATE_OUT);
		}

		ATTR_COLD void initial(const nl_double val);

		ATTR_HOT  void set_Q(const nl_double newQ);

		analog_net_t *m_proxied_net; // only for proxy nets in analog input logic

	private:
		analog_net_t m_my_net;
	};

	// -----------------------------------------------------------------------------
	// net_param_t
	// -----------------------------------------------------------------------------

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

		ATTR_COLD param_t(const param_type_t atype);

		ATTR_HOT  param_type_t param_type() const { return m_param_type; }

	protected:

		virtual void reset() override { }

	private:
		const param_type_t m_param_type;
	};

	template <class C, param_t::param_type_t T>
	class param_template_t : public param_t
	{
		P_PREVENT_COPYING(param_template_t)
	public:
		ATTR_COLD param_template_t()
		: param_t(T)
		, m_param(C(0))
		{
		}

		operator const C() const { return Value(); }

		ATTR_HOT  void setTo(const C &param);
		ATTR_COLD  void initial(const C &val) { m_param = val; }
		ATTR_HOT  C Value() const { return m_param;   }

	protected:
		virtual void save_register() override
		{
			/* pstrings not yet supported, these need special logic */
			if (T != param_t::STRING && T != param_t::MODEL)
				save(NLNAME(m_param));
			param_t::save_register();
		}

		virtual void changed() { }
		C m_param;
	private:
	};

	typedef param_template_t<nl_double, param_t::DOUBLE> param_double_t;
	typedef param_template_t<int, param_t::INTEGER> param_int_t;
	typedef param_template_t<pstring, param_t::STRING> param_str_t;

	class param_logic_t : public param_int_t
	{
		P_PREVENT_COPYING(param_logic_t)
	public:
		ATTR_COLD param_logic_t() : param_int_t() { };
	};

	class param_model_t : public param_template_t<pstring, param_t::MODEL>
	{
		P_PREVENT_COPYING(param_model_t)
	public:
		ATTR_COLD param_model_t() : param_template_t<pstring, param_t::MODEL>() { }

		/* these should be cached! */
		ATTR_COLD nl_double model_value(const pstring &entity);
		ATTR_COLD const pstring model_value_str(const pstring &entity);
		ATTR_COLD const pstring model_type();
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

		typedef pvector_t<core_device_t *> list_t;

		ATTR_COLD core_device_t(const family_t afamily);

		virtual ~core_device_t();

		virtual void init(netlist_t &anetlist, const pstring &name);
		ATTR_HOT virtual void update_param() {}

		ATTR_HOT  void update_dev()
		{
			begin_timing(stat_total_time);
			inc_stat(stat_update_count);

	#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF)
			(this->*m_static_update)();
	#elif ((NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV) || (NL_PMF_TYPE == NL_PMF_TYPE_INTERNAL))
			m_static_update(this);
	#else
			update();
	#endif
			end_timing(stat_total_time);
		}
		ATTR_COLD void start_dev();
		ATTR_COLD void stop_dev();

		ATTR_HOT netlist_sig_t INPLOGIC_PASSIVE(logic_input_t &inp);

		ATTR_HOT  netlist_sig_t INPLOGIC(const logic_input_t &inp) const
		{
			nl_assert(inp.state() != logic_t::STATE_INP_PASSIVE);
			return inp.Q();
		}

		ATTR_HOT  void OUTLOGIC(logic_output_t &out, const netlist_sig_t val, const netlist_time &delay)
		{
			out.set_Q(val, delay);
		}

		ATTR_HOT  nl_double INPANALOG(const analog_input_t &inp) const { return inp.Q_Analog(); }

		ATTR_HOT  nl_double TERMANALOG(const terminal_t &term) const { return term.net().Q_Analog(); }

		ATTR_HOT  void OUTANALOG(analog_output_t &out, const nl_double val)
		{
			out.set_Q(val);
		}

		ATTR_HOT virtual void inc_active() {  }

		ATTR_HOT virtual void dec_active() {  }

		ATTR_HOT virtual void step_time(ATTR_UNUSED const nl_double st) { }
		ATTR_HOT virtual void update_terminals() { }

	#if (NL_KEEP_STATISTICS)
		/* stats */
		osd_ticks_t stat_total_time;
		INT32 stat_update_count;
		INT32 stat_call_count;
	#endif

	protected:

		ATTR_HOT virtual void update() { }
		virtual void start() { }
		virtual void stop() { }                                                  \
		virtual logic_family_desc_t *default_logic_family()
		{
			return family_TTL;
		}

	private:

		#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF)
		typedef void (core_device_t::*net_update_delegate)();
		#elif ((NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV) || (NL_PMF_TYPE == NL_PMF_TYPE_INTERNAL))
		typedef MEMBER_ABI void (*net_update_delegate)(core_device_t *);
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

		ATTR_COLD device_t();
		ATTR_COLD device_t(const family_t afamily);

		virtual ~device_t();

		virtual void init(netlist_t &anetlist, const pstring &name) override;

		ATTR_COLD setup_t &setup();

		ATTR_COLD void register_sub(const pstring &name, device_t &dev);
		ATTR_COLD void register_subalias(const pstring &name, core_terminal_t &term);
		ATTR_COLD void register_subalias(const pstring &name, const pstring &aliased);
		ATTR_COLD void register_terminal(const pstring &name, terminal_t &port);
		ATTR_COLD void register_output(const pstring &name, analog_output_t &out);
		ATTR_COLD void register_output(const pstring &name, logic_output_t &out);
		ATTR_COLD void register_input(const pstring &name, analog_input_t &in);
		ATTR_COLD void register_input(const pstring &name, logic_input_t &in);

		ATTR_COLD void connect_late(const pstring &t1, const pstring &t2);
		ATTR_COLD void connect_late(core_terminal_t &t1, core_terminal_t &t2);
		ATTR_COLD void connect_direct(core_terminal_t &t1, core_terminal_t &t2);

		pvector_t<pstring> m_terminals;

	protected:

		ATTR_HOT virtual void update() override { }
		ATTR_HOT virtual void start() override { }
		ATTR_HOT virtual void update_terminals() override { }

		template <class C, class T>
		ATTR_COLD void register_param(const pstring &sname, C &param, const T initialVal);

	private:
	};


	// -----------------------------------------------------------------------------
	// queue_t
	// -----------------------------------------------------------------------------

	class queue_t : public timed_queue<net_t *, netlist_time>,
							public object_t,
							public pstate_callback_t
	{
	public:
		queue_t(netlist_t &nl);

	protected:

		void reset() override {}

		void register_state(pstate_manager_t &manager, const pstring &module) override;
		void on_pre_save() override;
		void on_post_load() override;

	private:
		struct names_t { char m_buf[64]; };
		int m_qsize;
		parray_t<netlist_time::INTERNALTYPE> m_times;
		parray_t<names_t> m_names;
	};

	// -----------------------------------------------------------------------------
	// netlist_t
	// -----------------------------------------------------------------------------


	class netlist_t : public object_t, public pstate_manager_t, public plog_dispatch_intf
	{
		P_PREVENT_COPYING(netlist_t)
	public:

		netlist_t();
		virtual ~netlist_t();

		ATTR_COLD void start();
		ATTR_COLD void stop();

		ATTR_HOT  const queue_t &queue() const { return m_queue; }
		ATTR_HOT  queue_t &queue() { return m_queue; }
		ATTR_HOT  const netlist_time &time() const { return m_time; }
		ATTR_HOT  devices::NETLIB_NAME(solver) *solver() const { return m_solver; }
		ATTR_HOT  devices::NETLIB_NAME(gnd) *gnd() const { return m_gnd; }
		ATTR_HOT nl_double gmin() const;

		ATTR_HOT void push_to_queue(net_t &out, const netlist_time &attime);
		ATTR_HOT void remove_from_queue(net_t &out);

		ATTR_HOT void process_queue(const netlist_time &delta);
		ATTR_HOT  void abort_current_queue_slice() { m_stop = netlist_time::zero; }

		ATTR_HOT  const bool &use_deactivate() const { return m_use_deactivate; }

		ATTR_COLD void rebuild_lists(); /* must be called after post_load ! */

		ATTR_COLD void set_setup(setup_t *asetup) { m_setup = asetup;  }
		ATTR_COLD setup_t &setup() { return *m_setup; }

		ATTR_COLD net_t *find_net(const pstring &name);

		template<class _device_class>
		ATTR_COLD pvector_t<_device_class *> get_device_list()
		{
			pvector_t<_device_class *> tmp;
			for (std::size_t i = 0; i < m_devices.size(); i++)
			{
				_device_class *dev = dynamic_cast<_device_class *>(m_devices[i]);
				if (dev != NULL)
					tmp.push_back(dev);
			}
			return tmp;
		}

		template<class _device_class>
		ATTR_COLD _device_class *get_first_device()
		{
			for (std::size_t i = 0; i < m_devices.size(); i++)
			{
				_device_class *dev = dynamic_cast<_device_class *>(m_devices[i]);
				if (dev != NULL)
					return dev;
			}
			return NULL;
		}

		template<class _device_class>
		ATTR_COLD _device_class *get_single_device(const char *classname)
		{
			_device_class *ret = NULL;
			for (std::size_t i = 0; i < m_devices.size(); i++)
			{
				_device_class *dev = dynamic_cast<_device_class *>(m_devices[i]);
				if (dev != NULL)
				{
					if (ret != NULL)
						this->log().fatal("more than one {1} device found", classname);
					else
						ret = dev;
				}
			}
			return ret;
		}

		pvector_t<device_t *> m_devices;
		net_t::list_t m_nets;
	#if (NL_KEEP_STATISTICS)
		pnamedlist_t<core_device_t *> m_started_devices;
	#endif

	ATTR_COLD plog_base<NL_DEBUG> &log() { return m_log; }
	ATTR_COLD const plog_base<NL_DEBUG> &log() const { return m_log; }

	protected:

		/* from object */
		virtual void reset() override;
		virtual void save_register() override;

	#if (NL_KEEP_STATISTICS)
		// performance
		int m_perf_out_processed;
		int m_perf_inp_processed;
		int m_perf_inp_active;
	#endif

	private:
		netlist_time                m_stop;     // target time for current queue processing

		netlist_time                m_time;
		bool                        m_use_deactivate;
		queue_t                     m_queue;


		devices::NETLIB_NAME(mainclock) *    m_mainclock;
		devices::NETLIB_NAME(solver) *       m_solver;
		devices::NETLIB_NAME(gnd) *          m_gnd;


		devices::NETLIB_NAME(netlistparams) *m_params;
		setup_t *m_setup;
		plog_base<NL_DEBUG> m_log;
	};

	// -----------------------------------------------------------------------------
	// inline implementations
	// -----------------------------------------------------------------------------

	PSTATE_INTERFACE(object_t, m_netlist, name())

	template <class C, param_t::param_type_t T>
	ATTR_HOT inline void param_template_t<C, T>::setTo(const C &param)
	{
		if (m_param != param)
		{
			m_param = param;
			changed();
			device().update_param();
		}
	}

	ATTR_HOT inline logic_net_t & net_t::as_logic()
	{
		nl_assert(family() == LOGIC);
		return static_cast<logic_net_t &>(*this);
	}

	ATTR_HOT inline const logic_net_t & net_t::as_logic() const
	{
		nl_assert(family() == LOGIC);
		return static_cast<const logic_net_t &>(*this);
	}

	ATTR_HOT inline analog_net_t & net_t::as_analog()
	{
		nl_assert(family() == ANALOG);
		return static_cast<analog_net_t &>(*this);
	}

	ATTR_HOT inline const analog_net_t & net_t::as_analog() const
	{
		nl_assert(family() == ANALOG);
		return static_cast<const analog_net_t &>(*this);
	}


	ATTR_HOT inline void logic_input_t::inactivate()
	{
		if (EXPECTED(!is_state(STATE_INP_PASSIVE)))
		{
			set_state(STATE_INP_PASSIVE);
			net().as_logic().dec_active(*this);
		}
	}

	ATTR_HOT inline void logic_input_t::activate()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().as_logic().inc_active(*this);
			set_state(STATE_INP_ACTIVE);
		}
	}

	ATTR_HOT inline void logic_input_t::activate_hl()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().as_logic().inc_active(*this);
			set_state(STATE_INP_HL);
		}
	}

	ATTR_HOT inline void logic_input_t::activate_lh()
	{
		if (is_state(STATE_INP_PASSIVE))
		{
			net().as_logic().inc_active(*this);
			set_state(STATE_INP_LH);
		}
	}


	ATTR_HOT inline void net_t::push_to_queue(const netlist_time &delay)
	{
		if (!is_queued() && (num_cons() > 0))
		{
			m_time = netlist().time() + delay;
			m_in_queue = (m_active > 0);     /* queued ? */
			if (m_in_queue)
			{
				netlist().push_to_queue(*this, m_time);
			}
		}
	}

	ATTR_HOT inline void net_t::reschedule_in_queue(const netlist_time &delay)
	{
		if (is_queued())
			netlist().remove_from_queue(*this);

		m_time = netlist().time() + delay;
		m_in_queue = (m_active > 0);     /* queued ? */
		if (EXPECTED(m_in_queue))
		{
			netlist().push_to_queue(*this, m_time);
		}
	}

	ATTR_HOT inline const analog_net_t & analog_t::net() const
	{
		return core_terminal_t::net().as_analog();
	}

	ATTR_HOT inline analog_net_t & analog_t::net()
	{
		return core_terminal_t::net().as_analog();
	}


	ATTR_HOT inline netlist_sig_t logic_input_t::Q() const
	{
		return net().as_logic().Q();
	}

	ATTR_HOT inline nl_double analog_input_t::Q_Analog() const
	{
		return net().as_analog().Q_Analog();
	}

	ATTR_HOT inline void analog_output_t::set_Q(const nl_double newQ)
	{
		if (newQ != net().Q_Analog())
		{
			net().m_cur_Analog = newQ;
			net().push_to_queue(NLTIME_FROM_NS(1));
		}
	}

	ATTR_HOT inline void netlist_t::push_to_queue(net_t &out, const netlist_time &attime)
	{
		m_queue.push(queue_t::entry_t(attime, &out));
	}

	ATTR_HOT inline void netlist_t::remove_from_queue(net_t &out)
	{
		m_queue.remove(&out);
	}

}

NETLIST_SAVE_TYPE(netlist::core_terminal_t::state_e, DT_INT);


#endif /* NLBASE_H_ */
