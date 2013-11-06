// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlbase.h
 *
 */

#ifndef NLBASE_H_
#define NLBASE_H_

#include "nl_config.h"
#include "nl_lists.h"
#include "nl_time.h"

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

#define NETLIB_UPDATE_PARAM(_chip) ATTR_HOT ATTR_ALIGN void NETLIB_NAME(_chip) :: update_param(void)
#define NETLIB_FUNC_VOID(_chip, _name, _params) ATTR_HOT ATTR_ALIGN inline void NETLIB_NAME(_chip) :: _name _params

#define NETLIB_DEVICE(_name, _priv)                                                 \
	class NETLIB_NAME(_name) : public net_device_t                                  \
	{                                                                               \
	public:                                                                         \
		NETLIB_NAME(_name) ()                                                       \
		: net_device_t()    { }                                                     \
	protected:                                                                      \
		ATTR_HOT void update();                                                     \
		ATTR_HOT void start();                                                      \
		_priv                                                                       \
	}

#define NETLIB_SUBDEVICE(_name, _priv)                                              \
	class NETLIB_NAME(_name) : public netlist_core_device_t                         \
	{                                                                               \
	public:                                                                         \
		NETLIB_NAME(_name) ()                                                       \
		: netlist_core_device_t()                                                   \
			{ }                                                                     \
	/*protected:*/                                                                  \
		ATTR_HOT void update();                                                     \
		_priv                                                                       \
	}

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)                                     \
	class NETLIB_NAME(_name) : public net_device_t                                  \
	{                                                                               \
	public:                                                                         \
		NETLIB_NAME(_name) ()                                                       \
		: net_device_t() { }                                                        \
		ATTR_HOT void update_param();                                               \
		ATTR_HOT void update();                                                     \
		ATTR_HOT void start();                                                      \
	/* protected: */                                                                \
		_priv                                                                       \
	}

// ----------------------------------------------------------------------------------------
// forward definitions
// ----------------------------------------------------------------------------------------

class net_output_t;
class net_param_t;
class netlist_setup_t;
class netlist_base_t;


// ----------------------------------------------------------------------------------------
// net_object_t
// ----------------------------------------------------------------------------------------

class net_object_t
{
public:
	enum type_t {
		INPUT = 0,
		OUTPUT = 1,
		DEVICE = 2,
		PARAM = 3,
		TERMINAL = 4,
		NET_ANALOG = 5,
		NET_DIGITAL = 6,
		TYPE_MASK = 0x0f,
		SIGNAL_DIGITAL = 0x00,
		SIGNAL_ANALOG =  0x10,
		SIGNAL_MASK =    0x10,
	};

	net_object_t(int atype)
		: m_objtype(atype) {}

	virtual ~net_object_t() {}

	ATTR_HOT inline int object_type() const { return m_objtype; }
	ATTR_HOT inline int object_type(const UINT32 mask) const { return m_objtype & mask; }

private:
	int m_objtype;
};

// ----------------------------------------------------------------------------------------
// net_terminal_t
// ----------------------------------------------------------------------------------------

class net_terminal_t : public net_object_t
{
public:

	net_terminal_t(const int atype)
	: net_object_t(atype)
	, m_netdev(NULL)
	, m_netlist(NULL)
	{}

	ATTR_COLD void init_terminal(netlist_core_device_t *dev);
	ATTR_HOT inline netlist_core_device_t * RESTRICT netdev() const { return m_netdev; }
	ATTR_HOT inline netlist_base_t * RESTRICT netlist() const { return m_netlist; }

private:
	netlist_core_device_t * RESTRICT m_netdev;
	netlist_base_t * RESTRICT m_netlist;
};


// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

class net_input_t : public net_terminal_t
{
public:

	enum net_input_state {
		INP_STATE_PASSIVE = 0,
		INP_STATE_ACTIVE = 1,
		INP_STATE_HL = 2,
		INP_STATE_LH = 4,
	};

	ATTR_COLD net_input_t(const int atype)
		: net_terminal_t(atype)
		, m_low_thresh_V(0)
		, m_high_thresh_V(0)
		, m_state(INP_STATE_ACTIVE)
		, m_output(NULL)
	{}

	ATTR_COLD void init_input(netlist_core_device_t *dev, net_input_state astate = INP_STATE_ACTIVE);

	ATTR_HOT inline net_output_t * RESTRICT output() const { return m_output; }
	ATTR_HOT inline const bool is_state(const net_input_state astate) const { return (m_state == astate); }
	ATTR_HOT inline const net_input_state state() const { return m_state; }

	ATTR_COLD void set_output(net_output_t &aout)   { m_output = &aout; }
	ATTR_HOT inline void inactivate();
	ATTR_HOT inline void activate();
	ATTR_HOT inline void activate_hl();
	ATTR_HOT inline void activate_lh();

	double m_low_thresh_V;
	double m_high_thresh_V;

private:
	net_input_state m_state;
	net_output_t * RESTRICT m_output;
};

// ----------------------------------------------------------------------------------------
// logic_input_t
// ----------------------------------------------------------------------------------------

class logic_input_t : public net_input_t
{
public:
	logic_input_t()
		: net_input_t(INPUT | SIGNAL_DIGITAL)
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
// ttl_input_t
// ----------------------------------------------------------------------------------------

class ttl_input_t : public logic_input_t
{
public:
	ttl_input_t()
		: logic_input_t() { set_thresholds(0.8 , 2.0); }
};

class analog_input_t : public net_input_t
{
public:
	analog_input_t()
		: net_input_t(INPUT | SIGNAL_ANALOG) { }

	ATTR_HOT inline const bool is_highz() const;
	ATTR_HOT inline const double Q_Analog() const;
};

//#define INPVAL(_x) (_x).Q()

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

class NETLIB_NAME(netdev_mainclock);

class net_output_t : public net_terminal_t
{
public:

    typedef struct {
        double        Analog;
        netlist_sig_t Q;
    } hybrid_t;

	net_output_t(int atype);

	friend const netlist_sig_t logic_input_t::Q() const;
	friend const double analog_input_t::Q_Analog() const;
	friend const bool analog_input_t::is_highz() const;
	friend class NETLIB_NAME(netdev_mainclock);

	ATTR_HOT inline const netlist_sig_t last_Q() const  { return m_last.Q;  }
	ATTR_HOT inline const netlist_sig_t new_Q() const   { return m_new.Q;   }

	ATTR_COLD void register_con(net_input_t &inp);

	/* inline not always works out */
	ATTR_HOT /*inline*/ void update_devs();

	ATTR_HOT inline void inc_active();
	ATTR_HOT inline void dec_active();

	ATTR_HOT inline const int active_count() const { return m_active; }
	ATTR_HOT inline const netlist_time time() const { return m_time; }
	ATTR_HOT inline void set_time(const netlist_time ntime) { m_time = ntime; }

	double m_low_V;
	double m_high_V;

protected:

	/* prohibit use in device functions
	 * current (pending) state can be inquired using new_Q()
	 */
	ATTR_HOT inline const netlist_sig_t Q() const
	{
		assert(object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		return m_cur.Q;
	}
	ATTR_HOT inline const double Q_Analog() const
	{
		assert(object_type(SIGNAL_MASK) == SIGNAL_ANALOG);
        return m_cur.Analog;
	}

	ATTR_HOT inline void push_to_queue(const netlist_time &delay);

	hybrid_t m_last;
	hybrid_t m_cur;
	hybrid_t m_new;

	UINT32 m_num_cons;

private:
	ATTR_HOT void update_dev(const net_input_t *inp, const UINT32 mask);

	netlist_time m_time;

	INT32 m_active;

	UINT32 m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

	net_input_t *  RESTRICT m_cons[OUTPUT_MAX_CONNECTIONS];
};


class logic_output_t : public net_output_t
{
public:

	logic_output_t()
		: net_output_t(OUTPUT | SIGNAL_DIGITAL)
	{
		// Default to TTL
		m_low_V = 0.1;  // these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_high_V = 4.8;
	}

	ATTR_COLD void initial(const netlist_sig_t val) { m_cur.Q = val; m_new.Q = val; m_last.Q = !val; }

	ATTR_HOT inline void set_Q(const netlist_sig_t newQ, const netlist_time &delay)
	{
		if (EXPECTED(newQ != m_new.Q))
		{
			m_new.Q = newQ;
			if (m_num_cons)
				push_to_queue(delay);
		}
	}

	ATTR_COLD inline void set_levels(const double low, const double high)
	{
		m_low_V = low;
		m_high_V = high;
	}
};

class ttl_output_t : public logic_output_t
{
public:

	ttl_output_t()
		: logic_output_t()
	{ set_levels(0.3, 3.4); }

};

class analog_output_t : public net_output_t
{
public:

	analog_output_t()
		: net_output_t(OUTPUT | SIGNAL_ANALOG) { }

    ATTR_COLD void initial(const double val) { m_cur.Analog = val; m_new.Analog = val; }

	ATTR_HOT inline void set_Q(const double newQ, const netlist_time &delay)
	{
		if (newQ != m_new.Analog)
		{
            m_new.Analog = newQ;
			push_to_queue(delay);
		}
	}

};

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

class netlist_core_device_t : public net_object_t
{
public:

	netlist_core_device_t();

	virtual ~netlist_core_device_t();

	ATTR_COLD virtual void init(netlist_setup_t &setup, const astring &name);

	ATTR_COLD const astring &name() const { return m_name; }

	ATTR_HOT virtual void update_param() {}

	ATTR_HOT const netlist_sig_t INPLOGIC_PASSIVE(logic_input_t &inp);

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

	ATTR_HOT inline const netlist_sig_t INPLOGIC(const logic_input_t &inp) const
	{
		assert(inp.state() != net_input_t::INP_STATE_PASSIVE);
		return inp.Q();
	}

	ATTR_HOT inline void OUTLOGIC(logic_output_t &out, const netlist_sig_t val, const netlist_time &delay)
	{
		out.set_Q(val, delay);
	}

	ATTR_HOT inline bool INP_HL(const logic_input_t &inp) const
	{
		return ((inp.last_Q() & !inp.Q()) == 1);
	}

	ATTR_HOT inline bool INP_LH(const logic_input_t &inp) const
	{
		return ((!inp.last_Q() & inp.Q()) == 1);
	}

	ATTR_HOT inline const double INPANALOG(const analog_input_t &inp) const { return inp.Q_Analog(); }

	ATTR_HOT inline void OUTANALOG(analog_output_t &out, const double val, const netlist_time &delay)
	{
		out.set_Q(val, delay);
	}

	ATTR_HOT inline netlist_base_t *netlist() const { return m_netlist; }

	ATTR_HOT virtual void inc_active() {  }

	ATTR_HOT virtual void dec_active() { /*printf("DeActivate %s\n", m_name);*/ }

	/* stats */
	osd_ticks_t total_time;
	INT32 stat_count;

#if USE_DELEGATES
	net_update_delegate static_update;
#endif

protected:

	ATTR_HOT virtual void update() { }
	ATTR_HOT virtual void start() { }

	netlist_base_t *  RESTRICT m_netlist;

private:

	astring m_name;
};


class net_device_t : public netlist_core_device_t
{
public:

	net_device_t();

	virtual ~net_device_t();

	ATTR_COLD virtual void init(netlist_setup_t &setup, const astring &name);

	ATTR_COLD const netlist_setup_t *setup() const { return m_setup; }

	ATTR_COLD bool variable_input_count() { return m_variable_input_count; }

	ATTR_COLD void register_sub(netlist_core_device_t &dev, const astring &name);

	ATTR_COLD void register_output(const astring &name, net_output_t &out);
	ATTR_COLD void register_output(netlist_core_device_t &dev, const astring &name, net_output_t &out);

	ATTR_COLD void register_input(const astring &name, net_input_t &in, net_input_t::net_input_state state = net_input_t::INP_STATE_ACTIVE);
	ATTR_COLD void register_input(netlist_core_device_t &dev, const astring &name, net_input_t &in, net_input_t::net_input_state state = net_input_t::INP_STATE_ACTIVE);

	ATTR_COLD void register_link_internal(net_input_t &in, net_output_t &out, net_input_t::net_input_state aState);
	ATTR_COLD void register_link_internal(netlist_core_device_t &dev, net_input_t &in, net_output_t &out, net_input_t::net_input_state aState);

	netlist_list_t<astring> m_inputs;

protected:

	virtual void update() { }
	ATTR_HOT virtual void start() { }

	ATTR_COLD void register_param(const astring &sname, net_param_t &param, const double initialVal = 0.0);
	ATTR_COLD void register_param(netlist_core_device_t &dev, const astring &sname, net_param_t &param, const double initialVal = 0.0);

	netlist_setup_t *m_setup;
	bool m_variable_input_count;

private:
};

class net_param_t
{
public:
	net_param_t()
	: m_param(0.0)
	, m_netdev(NULL)
	{  }

	inline void setTo(const double param) { m_param = param; m_netdev->update_param(); }
	inline void setTo(const int param) { m_param = param; m_netdev->update_param(); }
	inline void initial(const double val) { m_param = val; }
	inline void initial(const int val) { m_param = val; }

	ATTR_HOT inline double Value() const        { return m_param;   }
	ATTR_HOT inline int    ValueInt() const     { return (int) m_param;     }

	ATTR_HOT inline netlist_core_device_t &netdev() const { return *m_netdev; }
	void set_netdev(netlist_core_device_t &dev) { m_netdev = &dev; }

private:

	double m_param;
	netlist_core_device_t *m_netdev;
};



class netdev_mainclock;

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

class netlist_base_t
{
public:

	typedef netlist_timed_queue1<net_output_t, netlist_time, 512> queue_t;

	netlist_base_t();
	virtual ~netlist_base_t();

	void set_clock_freq(UINT64 clockfreq);

	ATTR_HOT inline void push_to_queue(net_output_t &out, const netlist_time &attime)
	{
		m_queue.push(queue_t::entry_t(attime, out));
	}

	ATTR_HOT void process_queue(INT32 &atime);

	ATTR_HOT inline const netlist_time &time() const { return m_time_ps; }

	ATTR_COLD void set_mainclock_dev(NETLIB_NAME(netdev_mainclock) *dev);

	ATTR_COLD void reset();

	// FIXME: should'nt be public
	queue_t m_queue;

protected:
	// performance
	int m_perf_out_processed;
	int m_perf_inp_processed;
	int m_perf_inp_active;

private:
	NETLIB_NAME(netdev_mainclock) *m_mainclock;
	netlist_time m_time_ps;
	UINT32   m_rem;
	UINT32  m_div;


	ATTR_HOT void update_time(const netlist_time t, INT32 &atime);

};

// ----------------------------------------------------------------------------------------
// netdev_a_to_d
// ----------------------------------------------------------------------------------------

class netdev_a_to_d_proxy : public net_device_t
{
public:
	netdev_a_to_d_proxy(net_input_t &in_proxied)
			: net_device_t()
	{
		assert(in_proxied.object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		m_I.m_high_thresh_V = in_proxied.m_high_thresh_V;
		m_I.m_low_thresh_V = in_proxied.m_low_thresh_V;
	}

	virtual ~netdev_a_to_d_proxy() {}

	analog_input_t m_I;
	ttl_output_t m_Q;

protected:
	void start()
	{
		m_I.init_input(this);

		m_Q.init_terminal(this);
		m_Q.initial(1);
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		if (m_I.Q_Analog() > m_I.m_high_thresh_V)
			OUTLOGIC(m_Q, 1, NLTIME_FROM_NS(1));
		else if (m_I.Q_Analog() < m_I.m_low_thresh_V)
			OUTLOGIC(m_Q, 0, NLTIME_FROM_NS(1));
	}

};

// ----------------------------------------------------------------------------------------
// netdev_d_to_a
// ----------------------------------------------------------------------------------------

class netdev_d_to_a_proxy : public net_device_t
{
public:
	netdev_d_to_a_proxy(net_output_t &out_proxied)
			: net_device_t()
	{
		assert(out_proxied.object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		m_low_V = out_proxied.m_low_V;
		m_high_V = out_proxied.m_high_V;
	}

	virtual ~netdev_d_to_a_proxy() {}

	ttl_input_t m_I;
	analog_output_t m_Q;

protected:
	void start()
	{
		m_I.init_input(this);
		m_Q.init_terminal(this);
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

ATTR_HOT inline void net_input_t::inactivate()
{
	if (m_state != INP_STATE_PASSIVE)
	{
		m_state = INP_STATE_PASSIVE;
		m_output->dec_active();
	}
}

ATTR_HOT inline void net_input_t::activate()
{
	if (m_state == INP_STATE_PASSIVE)
	{
		m_output->inc_active();
		m_state = INP_STATE_ACTIVE;
	}
}

ATTR_HOT inline void net_input_t::activate_hl()
{
	if (m_state == INP_STATE_PASSIVE)
	{
		m_output->inc_active();
		m_state = INP_STATE_HL;
	}
}

ATTR_HOT inline void net_input_t::activate_lh()
{
	if (m_state == INP_STATE_PASSIVE)
	{
		m_output->inc_active();
		m_state = INP_STATE_LH;
	}
}


ATTR_HOT inline void net_output_t::push_to_queue(const netlist_time &delay)
{
	// if (m_in_queue == 1) return; FIXME: check this at some time
	m_time = netlist()->time() + delay;
	m_in_queue = (m_active > 0) ? 1 : 0;     /* queued ? */
	if (m_in_queue)
	{
		//m_in_queue = 1;     /* pending */
		netlist()->push_to_queue(*this, m_time);
	}
}

ATTR_HOT inline void net_output_t::inc_active()
{
	m_active++;

#if USE_DEACTIVE_DEVICE
	if (m_active == 1 && m_in_queue > 0)
	{
		m_last = m_cur;
		netdev()->inc_active();
		m_cur = m_new;
	}
#endif

	if (EXPECTED(m_active == 1 && m_in_queue == 0))
	{
		if (EXPECTED(m_time > netlist()->time()))
		{
			m_in_queue = 1;     /* pending */
			netlist()->push_to_queue(*this, m_time);
		}
		else
		{
			m_cur = m_last = m_new;
			m_in_queue = 2;
		}
	}
}

ATTR_HOT inline void net_output_t::dec_active()
{
	m_active--;
#if (USE_DEACTIVE_DEVICE)
	if (m_active == 0)
		netdev()->dec_active();
#endif
}

ATTR_HOT inline const netlist_sig_t logic_input_t::Q() const
{
	return output()->Q();
}

ATTR_HOT inline const netlist_sig_t logic_input_t::last_Q() const
{
	return output()->last_Q();
}

ATTR_HOT inline const double analog_input_t::Q_Analog() const
{
	return output()->Q_Analog();
}

ATTR_HOT inline const bool analog_input_t::is_highz() const
{
	return (output()->Q_Analog() == NETLIST_HIGHIMP_V);
}

// ----------------------------------------------------------------------------------------
// net_dev class factory
// ----------------------------------------------------------------------------------------

class net_device_t_base_factory
{
public:
	net_device_t_base_factory(const astring &name, const astring &classname)
	: m_name(name), m_classname(classname)
	{}

	virtual ~net_device_t_base_factory() {}

	virtual net_device_t *Create() const = 0;

	const astring &name() const { return m_name; }
	const astring &classname() const { return m_classname; }
protected:
	astring m_name;                             /* device name */
	astring m_classname;                        /* device class name */
};

template <class C>
class net_device_t_factory : public net_device_t_base_factory
{
public:
	net_device_t_factory(const astring &name, const astring &classname)
	: net_device_t_base_factory(name, classname) { }

	net_device_t *Create() const
	{
		net_device_t *r = new C();
		//r->init(setup, name);
		return r;
	}
};

net_device_t *net_create_device_by_classname(const astring &classname, netlist_setup_t &setup, const astring &icname);
net_device_t *net_create_device_by_name(const astring &name, netlist_setup_t &setup, const astring &icname);


#endif /* NLBASE_H_ */
