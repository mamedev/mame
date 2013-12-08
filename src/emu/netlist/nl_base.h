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
 *  A rail net is a net which is driven by exactly one output with an (idealized) internal resistance
 *  of zero. Ideally, it can deliver infinite current.
 *
 *  A infinite resistance input does not source or sink current.
 *
 *  Terminals source or sink finite (but never zero) current.
 *
 *  The system differentiates between analog and logic input and outputs and analog terminals.
 *  Analog and logic devices can not be connected to the same net. Instead, proxy devices
 *  are inserted automatically:
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
 *  subsystem in MAME. Instead of relying on a fixed time-step, analog devices could
 *  either connect to fixed time-step clock or use an internal clock to update them.
 *  This would however introduce macro devices for RC, diodes and transistors again.
 *
 *  ====================================================================================
 *  FIXME: Terminals are not yet implemented.
 *
 *  Instead, the following approach in case of a pure terminal/input network is taken:
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
 *  This is a resistance in series to a voltage source and paralleled by a current source.
 *  This is suitable to model voltage sources, current sources, resistors, capacitors,
 *  inductances and diodes.
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
 *  with G being the conductance matrix, D a diagonal matrix with the total conductance
 *  on the diagonal elements, V the net voltage vector and I the current vector.
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
 *  Going forward, the approach can be extended e.g. to use a linear equation solver
 *
 *  The formal representation of the circuit will stay the same, thus scales.
 *
 */

#ifndef NLBASE_H_
#define NLBASE_H_

#include "nl_config.h"
#include "nl_lists.h"
#include "nl_time.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------

class netlist_core_device_t;

#if USE_DELEGATES
#if USE_PMFDELEGATES
typedef void (*net_update_delegate)(netlist_core_device_t *);
#else
typedef delegate<void ()> net_update_delegate;
#endif
#endif

//============================================================
//  MACROS / netlist devices
//============================================================

#define NETLIB_NAME(_chip) nld_ ## _chip

#define NETLIB_NAME_STR_S(_s) # _s
#define NETLIB_NAME_STR(_chip) NETLIB_NAME_STR_S(nld_ ## _chip)

#define NETLIB_UPDATE(_chip) ATTR_HOT ATTR_ALIGN void NETLIB_NAME(_chip) :: update(void)
#define NETLIB_START(_chip) ATTR_COLD void NETLIB_NAME(_chip) :: start(void)
//#define NETLIB_CONSTRUCTOR(_chip) ATTR_COLD _chip :: _chip (netlist_setup_t &setup, const char *name)
//          : net_device_t(setup, name)

#define NETLIB_UPDATE_PARAM(_chip) ATTR_COLD ATTR_ALIGN void NETLIB_NAME(_chip) :: update_param(void)
#define NETLIB_FUNC_VOID(_chip, _name, _params) ATTR_HOT ATTR_ALIGN void NETLIB_NAME(_chip) :: _name _params

#define NETLIB_UPDATE_TERMINALS() ATTR_HOT ATTR_ALIGN inline void update_terminals(void)

#define NETLIB_DEVICE_BASE(_name, _pclass, _extra, _priv)                           \
    class _name : public _pclass                                                    \
    {                                                                               \
    public:                                                                         \
        _name()                                                                     \
        : _pclass()    { }                                                          \
    protected:                                                                      \
        _extra                                                                      \
        ATTR_HOT void update();                                                     \
        ATTR_HOT void start();                                                      \
        _priv                                                                       \
    }

#define NETLIB_DEVICE_DERIVED(_name, _pclass, _priv)                                \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), NETLIB_NAME(_pclass), , _priv)

#define NETLIB_DEVICE(_name, _priv)                                                 \
		NETLIB_DEVICE_BASE(NETLIB_NAME(_name), netlist_device_t, , _priv)

#define NETLIB_SUBDEVICE(_name, _priv)                                             \
    class NETLIB_NAME(_name) : public netlist_device_t                              \
    {                                                                               \
    public:                                                                         \
        NETLIB_NAME(_name) ()                                                       \
        : netlist_device_t()                                                        \
            { }                                                                     \
    /*protected:*/                                                                  \
        ATTR_HOT void update();                                                     \
        ATTR_HOT void start();                                                      \
    public:                                                                         \
        _priv                                                                       \
    }

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)                                     \
        NETLIB_DEVICE_BASE(NETLIB_NAME(_name), netlist_device_t,                    \
        	ATTR_HOT void update_param();                                           \
	    , _priv)

#define NETLIB_DEVICE_WITH_PARAMS_DERIVED(_name, _pclass, _priv)                    \
        NETLIB_DEVICE_BASE(NETLIB_NAME(_name), NETLIB_NAME(_pclass),                \
            ATTR_HOT void update_param();                                           \
        , _priv)

// ----------------------------------------------------------------------------------------
// forward definitions
// ----------------------------------------------------------------------------------------

class netlist_net_t;
class netlist_output_t;
class netlist_param_t;
class netlist_setup_t;
class netlist_base_t;
class netlist_matrix_solver_t;


// ----------------------------------------------------------------------------------------
// netlist_object_t
// ----------------------------------------------------------------------------------------

class netlist_object_t
{
public:
	enum type_t {
        TERMINAL = 0,
		INPUT    = 1,
		OUTPUT   = 2,
		PARAM    = 3,
		NET      = 4,
        DEVICE   = 5,
	};
    enum family_t {
        // Terminal families
        LOGIC     = 1,
        ANALOG    = 2,
        // Device families
        GENERIC   = 3,   // <== devices usually fall into this category
        RESISTOR  = 4,   // Resistor
        CAPACITOR = 5,   // Capacitor
        DIODE     = 6,   // Diode
        BJT_SWITCH = 7,  // BJT(Switch)
    };

	ATTR_COLD netlist_object_t(const type_t atype, const family_t afamily);

	virtual ~netlist_object_t();

	ATTR_COLD void init_object(netlist_base_t &nl, const pstring &aname);
    ATTR_COLD bool isInitalized() { return (m_netlist != NULL); }

    ATTR_COLD const pstring &name() const;

	ATTR_HOT inline const type_t type() const { return m_objtype; }
    ATTR_HOT inline const family_t family() const { return m_family; }

	ATTR_HOT inline const bool isType(const type_t atype) const { return (m_objtype == atype); }
    ATTR_HOT inline const bool isFamily(const family_t afamily) const { return (m_family == afamily); }

    ATTR_HOT inline netlist_base_t & RESTRICT netlist() { return *m_netlist; }
    ATTR_HOT inline const netlist_base_t & RESTRICT netlist() const { return *m_netlist; }

private:
    pstring m_name;
	const type_t m_objtype;
    const family_t m_family;
    netlist_base_t * RESTRICT m_netlist;
};

// ----------------------------------------------------------------------------------------
// netlist_owned_object_t
// ----------------------------------------------------------------------------------------

class netlist_owned_object_t : public netlist_object_t
{
public:
    ATTR_COLD netlist_owned_object_t(const type_t atype, const family_t afamily);

    ATTR_COLD void init_object(netlist_core_device_t &dev, const pstring &aname);

    ATTR_HOT inline netlist_core_device_t & RESTRICT netdev() const { return *m_netdev; }
private:
    netlist_core_device_t * RESTRICT m_netdev;
};

// ----------------------------------------------------------------------------------------
// netlist_core_terminal_t
// ----------------------------------------------------------------------------------------

class netlist_core_terminal_t : public netlist_owned_object_t
{
public:

    /* needed here ... */

    enum state_e {
        STATE_INP_PASSIVE = 0,
        STATE_INP_ACTIVE = 1,
        STATE_INP_HL = 2,
        STATE_INP_LH = 4,
        STATE_OUT = 128,
        STATE_NONEX = 256
    };

	ATTR_COLD netlist_core_terminal_t(const type_t atype, const family_t afamily);

	ATTR_COLD void init_object(netlist_core_device_t &dev, const pstring &aname, const state_e astate);

    ATTR_COLD void set_net(netlist_net_t &anet);
    ATTR_COLD inline bool has_net() { return (m_net != NULL); }
    ATTR_HOT inline const netlist_net_t & RESTRICT net() const { return *m_net;}
    ATTR_HOT inline netlist_net_t & RESTRICT net() { return *m_net;}

    ATTR_HOT inline const bool is_state(const state_e astate) const { return (m_state == astate); }
    ATTR_HOT inline const state_e state() const { return m_state; }
    ATTR_HOT inline void set_state(const state_e astate)
    {
        assert(astate != STATE_NONEX);
        m_state = astate;
    }

    netlist_core_terminal_t *m_update_list_next;

private:
    netlist_net_t * RESTRICT m_net;
    state_e m_state;
};

class netlist_terminal_t : public netlist_core_terminal_t
{
public:
    ATTR_COLD netlist_terminal_t();

    double m_Idr; // drive current
    double m_g; // conductance

    netlist_terminal_t *m_otherterm;
};


// ----------------------------------------------------------------------------------------
// netlist_input_t
// ----------------------------------------------------------------------------------------

class netlist_input_t : public netlist_core_terminal_t
{
public:


	ATTR_COLD netlist_input_t(const type_t atype, const family_t afamily)
		: netlist_core_terminal_t(atype, afamily)
        , m_low_thresh_V(0)
        , m_high_thresh_V(0)
	{
	    set_state(STATE_INP_ACTIVE);
	}

	ATTR_HOT inline void inactivate();
	ATTR_HOT inline void activate();
	ATTR_HOT inline void activate_hl();
	ATTR_HOT inline void activate_lh();

	double m_low_thresh_V;
	double m_high_thresh_V;

private:
};

// ----------------------------------------------------------------------------------------
// netlist_logic_input_t
// ----------------------------------------------------------------------------------------

class netlist_logic_input_t : public netlist_input_t
{
public:
	netlist_logic_input_t()
		: netlist_input_t(INPUT, LOGIC)
	{
		// default to TTL
		m_low_thresh_V = 0.8;
		m_high_thresh_V = 2.0;
	}

	ATTR_HOT inline const netlist_sig_t Q() const;
	ATTR_HOT inline const netlist_sig_t last_Q() const;

	ATTR_COLD inline void set_thresholds(const double low, const double high)
	{
		m_low_thresh_V = low;
		m_high_thresh_V = high;
	}
};

// ----------------------------------------------------------------------------------------
// netlist_ttl_input_t
// ----------------------------------------------------------------------------------------

class netlist_ttl_input_t : public netlist_logic_input_t
{
public:
	netlist_ttl_input_t()
		: netlist_logic_input_t() { set_thresholds(0.8 , 2.0); }
};

// ----------------------------------------------------------------------------------------
// netlist_analog_input_t
// ----------------------------------------------------------------------------------------

class netlist_analog_input_t : public netlist_input_t
{
public:
	netlist_analog_input_t()
		: netlist_input_t(INPUT, ANALOG) { }

	ATTR_HOT inline const bool is_highz() const;
	ATTR_HOT inline const double Q_Analog() const;
};

//#define INPVAL(_x) (_x).Q()

// ----------------------------------------------------------------------------------------
// net_net_t
// ----------------------------------------------------------------------------------------

class netlist_net_t : public netlist_object_t
{
public:

    typedef netlist_list_t<netlist_net_t *> list_t;

    friend class NETLIB_NAME(mainclock);
    friend class NETLIB_NAME(solver);
    friend class netlist_matrix_solver_t;
    friend class netlist_output_t;
    friend class netlist_input_t;
    friend class netlist_logic_output_t;
    friend class netlist_analog_output_t;
    friend class netlist_logic_input_t;
    friend class netlist_analog_input_t;

    // FIXME: union does not work
    typedef struct
    {
        netlist_sig_t Q;
        double        Analog;
    } hybrid_t;

    ATTR_COLD netlist_net_t(const type_t atype, const family_t afamily);

    ATTR_COLD void register_con(netlist_core_terminal_t &terminal);
    ATTR_COLD void merge_net(netlist_net_t *othernet);
    ATTR_COLD void register_railterminal(netlist_output_t &mr);

    /* inline not always works out */
    ATTR_HOT inline void update_devs();

    ATTR_HOT inline const netlist_time time() const { return m_time; }
    ATTR_HOT inline void set_time(const netlist_time ntime) { m_time = ntime; }

    ATTR_HOT inline bool isRailNet() { return !(m_railterminal == NULL); }
    ATTR_HOT inline const netlist_core_terminal_t & RESTRICT  railterminal() const { return *m_railterminal; }
    ATTR_HOT inline const netlist_core_terminal_t & RESTRICT railterminal() { return *m_railterminal; }

    /* Everything below is used by the logic subsystem */

    ATTR_HOT inline void inc_active();
    ATTR_HOT inline void dec_active();

    ATTR_HOT inline const int active_count() const { return m_active; }

    ATTR_HOT inline const netlist_sig_t last_Q() const  { return m_last.Q;  }
    ATTR_HOT inline const netlist_sig_t new_Q() const   { return m_new.Q;   }

    ATTR_HOT inline const double Q_Analog() const
    {
        //assert(object_type(SIGNAL_MASK) == SIGNAL_ANALOG);
        assert(family() == ANALOG);
        return m_cur.Analog;
    }

    ATTR_HOT inline void push_to_queue(const netlist_time &delay);
    ATTR_HOT bool is_queued() { return m_in_queue == 1; }

    // m_terms is only used by analog subsystem
    typedef netlist_list_t<netlist_terminal_t *> terminal_list_t;
    terminal_list_t m_terms;
    netlist_matrix_solver_t *m_solver;

    netlist_core_terminal_t *m_head;

protected:

    /* prohibit use in device functions
     * current (pending) state can be inquired using new_Q()
     */
    ATTR_HOT inline const netlist_sig_t Q() const
    {
        //assert(object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		assert(family() == LOGIC);
        return m_cur.Q;
    }

    hybrid_t m_last;
    hybrid_t m_cur;
    hybrid_t m_new;

    UINT32 m_num_cons;

private:
    ATTR_HOT void update_dev(const netlist_core_terminal_t *inp, const UINT32 mask);

    netlist_time m_time;
    INT32        m_active;
    UINT32       m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

    netlist_core_terminal_t * RESTRICT m_railterminal;
};


// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(mainclock);
class NETLIB_NAME(solver);

class netlist_output_t : public netlist_core_terminal_t
{
public:

	ATTR_COLD netlist_output_t(const type_t atype, const family_t afamily);

    ATTR_COLD void init_object(netlist_core_device_t &dev, const pstring &aname);

	double m_low_V;
	double m_high_V;

protected:

private:
	netlist_net_t m_my_net;
};


class netlist_logic_output_t : public netlist_output_t
{
public:

	ATTR_COLD netlist_logic_output_t();

	ATTR_COLD void initial(const netlist_sig_t val);
    ATTR_COLD void set_levels(const double low, const double high);

	ATTR_HOT inline void set_Q(const netlist_sig_t newQ, const netlist_time &delay)
	{
	    netlist_net_t &anet = net();

		if (EXPECTED(newQ != anet.m_new.Q))
		{
		    anet.m_new.Q = newQ;
			if (anet.m_num_cons)
			    anet.push_to_queue(delay);
		}
	}
};

class netlist_ttl_output_t : public netlist_logic_output_t
{
public:

	netlist_ttl_output_t()
		: netlist_logic_output_t()
	{
	    set_levels(0.3, 3.4);
	}

};

class netlist_analog_output_t : public netlist_output_t
{
public:

	ATTR_COLD netlist_analog_output_t()
		: netlist_output_t(OUTPUT, ANALOG)
    {
	    net().m_cur.Analog = 0.0;
	    net().m_new.Analog = 99.0;
    }

    ATTR_COLD void initial(const double val)
    {
        net().m_cur.Analog = val;
        net().m_new.Analog = 99.0;
    }

	ATTR_HOT inline void set_Q(const double newQ, const netlist_time &delay)
	{
		if (newQ != net().m_new.Analog)
		{
		    net().m_new.Analog = newQ;
		    net().push_to_queue(delay);
		}
	}

};

// ----------------------------------------------------------------------------------------
// net_param_t
// ----------------------------------------------------------------------------------------

class netlist_param_t : public netlist_owned_object_t
{
public:

    enum param_type_t {
        STRING,
        DOUBLE,
        INTEGER,
        LOGIC
    };

    netlist_param_t(const param_type_t atype)
    : netlist_owned_object_t(PARAM, ANALOG)
    , m_param_type(atype)
    {  }


    ATTR_HOT inline const param_type_t param_type() const { return m_param_type; }

private:
    const param_type_t m_param_type;
};

class netlist_param_double_t : public netlist_param_t
{
public:
    netlist_param_double_t()
    : netlist_param_t(DOUBLE)
    , m_param(0.0)
    {  }

    ATTR_HOT inline void setTo(const double param);
    ATTR_COLD inline void initial(const double val) { m_param = val; }
    ATTR_HOT inline const double Value() const        { return m_param;   }

private:
    double m_param;
};

class netlist_param_int_t : public netlist_param_t
{
public:
    netlist_param_int_t()
    : netlist_param_t(INTEGER)
    , m_param(0)
    {  }

    ATTR_HOT inline void setTo(const int param);
    ATTR_COLD inline void initial(const int val) { m_param = val; }

    ATTR_HOT inline const int Value() const     { return m_param;     }

private:
    int m_param;
};

class netlist_param_logic_t : public netlist_param_int_t
{
public:
    netlist_param_logic_t()
    : netlist_param_int_t()
    {  }
};

class netlist_param_str_t : public netlist_param_t
{
public:
    netlist_param_str_t()
    : netlist_param_t(STRING)
    , m_param("")
    {  }

    ATTR_HOT inline void setTo(const pstring &param);
    ATTR_COLD inline void initial(const pstring &val) { m_param = val; }

    ATTR_HOT inline const pstring &Value() const     { return m_param;     }

private:
   pstring m_param;
};

class netlist_param_multi_t : public netlist_param_str_t
{
public:
    netlist_param_multi_t()
    : netlist_param_str_t()
    {  }

    /* these should be cached! */
    ATTR_COLD double dValue(const pstring &entity, const double defval = 0.0) const;

private:
};

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

class netlist_core_device_t : public netlist_object_t
{
public:

    typedef netlist_list_t<netlist_core_device_t *> list_t;

    ATTR_COLD netlist_core_device_t();
    ATTR_COLD netlist_core_device_t(const family_t afamily);

    ATTR_COLD virtual ~netlist_core_device_t();

	ATTR_COLD virtual void init(netlist_setup_t &setup, const pstring &name);


	ATTR_HOT virtual void update_param() {}

	ATTR_HOT inline void update_dev()
	{
#if USE_DELEGATES
#if USE_PMFDELEGATES
		static_update(this);
#else
		static_update();
#endif
#else
		update();
#endif
	}

    ATTR_HOT const netlist_sig_t INPLOGIC_PASSIVE(netlist_logic_input_t &inp);

	ATTR_HOT inline const netlist_sig_t INPLOGIC(const netlist_logic_input_t &inp) const
	{
		assert(inp.state() != netlist_input_t::STATE_INP_PASSIVE);
		return inp.Q();
	}

	ATTR_HOT inline void OUTLOGIC(netlist_logic_output_t &out, const netlist_sig_t val, const netlist_time &delay)
	{
		out.set_Q(val, delay);
	}

	ATTR_HOT inline bool INP_HL(const netlist_logic_input_t &inp) const
	{
		return ((inp.last_Q() & !inp.Q()) == 1);
	}

	ATTR_HOT inline bool INP_LH(const netlist_logic_input_t &inp) const
	{
		return ((!inp.last_Q() & inp.Q()) == 1);
	}

	ATTR_HOT inline const double INPANALOG(const netlist_analog_input_t &inp) const { return inp.Q_Analog(); }

	ATTR_HOT inline const double TERMANALOG(const netlist_terminal_t &term) const { return term.net().Q_Analog(); }

	ATTR_HOT inline void OUTANALOG(netlist_analog_output_t &out, const double val, const netlist_time &delay)
	{
		out.set_Q(val, delay);
	}

	ATTR_HOT virtual void inc_active() {  }

	ATTR_HOT virtual void dec_active() { /*printf("DeActivate %s\n", m_name);*/ }

	ATTR_HOT virtual void step_time(const double st) { }
    ATTR_HOT virtual void update_terminals() { }

#if (NL_KEEP_STATISTICS)
    /* stats */
	osd_ticks_t total_time;
	INT32 stat_count;
#endif

#if USE_DELEGATES
	net_update_delegate static_update;
#endif

protected:

	ATTR_HOT virtual void update() { }
	ATTR_HOT virtual void start() { }

private:
};


class netlist_device_t : public netlist_core_device_t
{
public:

	ATTR_COLD netlist_device_t();
    ATTR_COLD netlist_device_t(const family_t afamily);

	ATTR_COLD virtual ~netlist_device_t();

	ATTR_COLD virtual void init(netlist_setup_t &setup, const pstring &name);

	ATTR_COLD const netlist_setup_t *setup() const { return m_setup; }

	ATTR_COLD bool variable_input_count() { return m_variable_input_count; }

	ATTR_COLD void register_sub(netlist_device_t &dev, const pstring &name);
    ATTR_COLD void register_subalias(const pstring &name, netlist_core_terminal_t &term);

    ATTR_COLD void register_terminal(const pstring &name, netlist_terminal_t &port);

	ATTR_COLD void register_output(const pstring &name, netlist_output_t &out);

	ATTR_COLD void register_input(const pstring &name, netlist_input_t &in, netlist_input_t::state_e state = netlist_input_t::STATE_INP_ACTIVE);

	ATTR_COLD void register_link_internal(netlist_input_t &in, netlist_output_t &out, netlist_input_t::state_e aState);
	ATTR_COLD void register_link_internal(netlist_core_device_t &dev, netlist_input_t &in, netlist_output_t &out, netlist_input_t::state_e aState);

    ATTR_HOT virtual void update_terminals() { }

    /* driving logic outputs don't count in here */
	netlist_list_t<pstring, 20> m_terminals;

protected:

	virtual void update() { }
	virtual void start() { }

    template <class C, class T>
	ATTR_COLD void register_param(const pstring &sname, C &param, const T initialVal)
	{
	    register_param(*this, sname, param, initialVal);
	}

	template <class C, class T>
    ATTR_COLD void register_param(netlist_core_device_t &dev, const pstring &sname, C &param, const T initialVal);

	netlist_setup_t *m_setup;
	bool m_variable_input_count;

private:
};


// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

class netlist_base_t
{
public:

	typedef netlist_timed_queue<netlist_net_t, netlist_time, 512> queue_t;

	netlist_base_t();
	virtual ~netlist_base_t();

	ATTR_COLD void set_clock_freq(UINT64 clockfreq);

    ATTR_HOT inline queue_t &queue() { return m_queue; }

	ATTR_HOT inline void push_to_queue(netlist_net_t &out, const netlist_time &attime)
	{
		m_queue.push(queue_t::entry_t(attime, out));
	}

	ATTR_HOT NETLIB_NAME(solver) *solver() { return m_solver; }

	ATTR_HOT void process_queue(INT32 &atime);

	ATTR_HOT inline const netlist_time &time() const { return m_time_ps; }

	ATTR_COLD void set_mainclock_dev(NETLIB_NAME(mainclock) *dev);
    ATTR_COLD void set_solver_dev(NETLIB_NAME(solver) *dev);

	ATTR_COLD void reset();

protected:
#if (NL_KEEP_STATISTICS)
	// performance
	int m_perf_out_processed;
	int m_perf_inp_processed;
	int m_perf_inp_active;
#endif

private:
    netlist_time                m_time_ps;
    queue_t                     m_queue;
	UINT32                      m_rem;
	UINT32                      m_div;

	NETLIB_NAME(mainclock) *    m_mainclock;
    NETLIB_NAME(solver) *       m_solver;

	ATTR_HOT void update_time(const netlist_time t, INT32 &atime);

};

// ----------------------------------------------------------------------------------------
// netdev_a_to_d
// ----------------------------------------------------------------------------------------

class nld_a_to_d_proxy : public netlist_device_t
{
public:
	nld_a_to_d_proxy(netlist_input_t &in_proxied)
			: netlist_device_t()
	{
		//assert(in_proxied.object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		assert(in_proxied.family() == LOGIC);
		m_I.m_high_thresh_V = in_proxied.m_high_thresh_V;
		m_I.m_low_thresh_V = in_proxied.m_low_thresh_V;
	}

	virtual ~nld_a_to_d_proxy() {}

	netlist_analog_input_t m_I;
	netlist_ttl_output_t m_Q;

protected:
	void start()
	{
		m_I.init_object(*this, "I", netlist_terminal_t::STATE_INP_ACTIVE);

		m_Q.init_object(*this, "Q");
		m_Q.initial(1);
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		if (m_I.Q_Analog() > m_I.m_high_thresh_V)
			OUTLOGIC(m_Q, 1, NLTIME_FROM_NS(1));
		else if (m_I.Q_Analog() < m_I.m_low_thresh_V)
			OUTLOGIC(m_Q, 0, NLTIME_FROM_NS(1));
		else
		    OUTLOGIC(m_Q, m_Q.net().last_Q(), NLTIME_FROM_NS(1));
	}

};

// ----------------------------------------------------------------------------------------
// netdev_d_to_a
// ----------------------------------------------------------------------------------------

class nld_d_to_a_proxy : public netlist_device_t
{
public:
	nld_d_to_a_proxy(netlist_output_t &out_proxied)
			: netlist_device_t()
	{
		//assert(out_proxied.object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		assert(out_proxied.family() == LOGIC);
		m_low_V = out_proxied.m_low_V;
		m_high_V = out_proxied.m_high_V;
	}

	virtual ~nld_d_to_a_proxy() {}

	netlist_ttl_input_t m_I;
	netlist_analog_output_t m_Q;

protected:
	void start()
	{
		m_I.init_object(*this, "I", netlist_terminal_t::STATE_INP_ACTIVE);
		m_Q.init_object(*this, "Q");
		m_Q.initial(0);
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		OUTANALOG(m_Q, INPLOGIC(m_I) ? m_high_V : m_low_V, NLTIME_FROM_NS(1));
	}

private:
	double m_low_V;
	double m_high_V;
};

// ----------------------------------------------------------------------------------------
// Inline implementations
// ----------------------------------------------------------------------------------------

ATTR_HOT inline void netlist_param_str_t::setTo(const pstring &param)
{
    m_param = param;
    netdev().update_param();
}

ATTR_HOT inline void netlist_param_int_t::setTo(const int param)
{
    m_param = param;
    netdev().update_param();
}

ATTR_HOT inline void netlist_param_double_t::setTo(const double param)
{
    m_param = param;
    netdev().update_param();
}


ATTR_HOT inline void netlist_input_t::inactivate()
{
	if (!is_state(STATE_INP_PASSIVE))
	{
		set_state(STATE_INP_PASSIVE);
		net().dec_active();
	}
}

ATTR_HOT inline void netlist_input_t::activate()
{
	if (is_state(STATE_INP_PASSIVE))
	{
		net().inc_active();
		set_state(STATE_INP_ACTIVE);
	}
}

ATTR_HOT inline void netlist_input_t::activate_hl()
{
	if (is_state(STATE_INP_PASSIVE))
	{
		net().inc_active();
		set_state(STATE_INP_HL);
	}
}

ATTR_HOT inline void netlist_input_t::activate_lh()
{
	if (is_state(STATE_INP_PASSIVE))
	{
		net().inc_active();
		set_state(STATE_INP_LH);
	}
}


ATTR_HOT inline void netlist_net_t::push_to_queue(const netlist_time &delay)
{
	// if (m_in_queue == 1) return; FIXME: check this at some time
	m_time = netlist().time() + delay;
	m_in_queue = (m_active > 0) ? 1 : 0;     /* queued ? */
	if (m_in_queue)
	{
		//m_in_queue = 1;     /* pending */
		netlist().push_to_queue(*this, m_time);
	}
}

ATTR_HOT inline void netlist_net_t::inc_active()
{
	m_active++;

	if (USE_DEACTIVE_DEVICE)
        if (m_active == 1 && m_in_queue > 0)
        {
            m_last = m_cur;
            railterminal().netdev().inc_active();
            m_cur = m_new;
        }

	if (EXPECTED(m_active == 1 && m_in_queue == 0))
	{
		if (EXPECTED(m_time > netlist().time()))
		{
			m_in_queue = 1;     /* pending */
			netlist().push_to_queue(*this, m_time);
		}
		else
		{
			m_cur = m_last = m_new;
			m_in_queue = 2;
		}
	}
}

ATTR_HOT inline void netlist_net_t::dec_active()
{
	m_active--;
	if (USE_DEACTIVE_DEVICE)
	    if (m_active == 0)
	        railterminal().netdev().dec_active();

}

ATTR_HOT inline const netlist_sig_t netlist_logic_input_t::Q() const
{
	return net().Q();
}

ATTR_HOT inline const netlist_sig_t netlist_logic_input_t::last_Q() const
{
	return net().last_Q();
}

ATTR_HOT inline const double netlist_analog_input_t::Q_Analog() const
{
	return net().Q_Analog();
}

ATTR_HOT inline const bool netlist_analog_input_t::is_highz() const
{
	return (net().Q_Analog() == NETLIST_HIGHIMP_V);
}

// ----------------------------------------------------------------------------------------
// net_dev class factory
// ----------------------------------------------------------------------------------------

class net_device_t_base_factory
{
public:
	net_device_t_base_factory(const pstring &name, const pstring &classname)
	: m_name(name), m_classname(classname)
	{}

	virtual ~net_device_t_base_factory() {}

	virtual netlist_device_t *Create() const = 0;

	const pstring &name() const { return m_name; }
	const pstring &classname() const { return m_classname; }
protected:
	pstring m_name;                             /* device name */
	pstring m_classname;                        /* device class name */
};

template <class C>
class net_device_t_factory : public net_device_t_base_factory
{
public:
	net_device_t_factory(const pstring &name, const pstring &classname)
	: net_device_t_base_factory(name, classname) { }

	netlist_device_t *Create() const
	{
		netlist_device_t *r = new C();
		//r->init(setup, name);
		return r;
	}
};

class netlist_factory
{
public:

    netlist_factory();
    ~netlist_factory();

    void initialize();

    netlist_device_t *new_device_by_classname(const pstring &classname, netlist_setup_t &setup) const;
    netlist_device_t *new_device_by_name(const pstring &name, netlist_setup_t &setup) const;

private:
    typedef netlist_list_t<net_device_t_base_factory *> list_t;
    list_t m_list;

};



#endif /* NLBASE_H_ */
