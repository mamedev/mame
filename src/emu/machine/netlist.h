/***************************************************************************

    netlist.h

    Discrete netlist implementation.

****************************************************************************

    Couriersud reserves the right to license the code under a less restrictive
    license going forward.

    Copyright Nicola Salmoria and the MAME team
    All rights reserved.

    Redistribution and use of this code or any derivative works are permitted
    provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial
    product or activity.

    * Redistributions that are modified from the original source must include the
    complete source code, including the source code for all components used by a
    binary built from the modified sources. However, as a special exception, the
    source code distributed need not include anything that is normally distributed
    (in either source or binary form) with the major components (compiler, kernel,
    and so on) of the operating system on which the executable runs, unless that
    component itself accompanies the executable.

    * Redistributions must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


****************************************************************************/

#ifndef NETLIST_H
#define NETLIST_H

#include "emu.h"
#include "tagmap.h"

//============================================================
//  SETUP
//============================================================

#define USE_DELEGATES			(0)

/*
 * The next options needs -Wno-pmf-conversions to compile and gcc
 * This is intended for non-mame usage.
 *
 */
#define USE_PMFDELEGATES		(0)

// Next if enabled adds 20% performance ... but is not guaranteed to be absolutely timing correct.
#define USE_DEACTIVE_DEVICE		(0)

#define OUTPUT_MAX_CONNECTIONS	(48)

// Use nano-second resolution - Sufficient for now
//#define NETLIST_INTERNAL_RES		(U64(1000000000))
//#define NETLIST_DIV_BITS			(0)
#define NETLIST_INTERNAL_RES		(U64(1000000000000))
#define NETLIST_DIV_BITS			(10)
#define NETLIST_DIV					(U64(1) << NETLIST_DIV_BITS)
#define NETLIST_MASK				(NETLIST_DIV-1)
#define NETLIST_CLOCK              	(NETLIST_INTERNAL_RES / NETLIST_DIV)

#define NLTIME_FROM_NS(_t)  netlist_time::from_nsec(_t)
#define NLTIME_FROM_US(_t)  netlist_time::from_usec(_t)
#define NLTIME_FROM_MS(_t)  netlist_time::from_msec(_t)
#define NLTIME_IMMEDIATE    netlist_time::from_nsec(0)

#define NETLIST_HIGHIMP_V   (1.23456e20)        /* some voltage we should never see */


//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_ALIAS(_alias, _name)                                                    \
	netlist.register_alias(# _alias, # _name);
#define NET_NEW(_type , _name)  net_create_device_by_classname(# _type, netlist, # _name)

#define NET_REGISTER_DEV(_type, _name)                                              \
		netlist.register_dev(NET_NEW(_type, _name));
#define NET_REMOVE_DEV(_name)                                                       \
		netlist.remove_dev(# _name);
#define NET_REGISTER_SIGNAL(_type, _name)                                           \
		NET_REGISTER_DEV(_type ## _ ## sig, _name)
#define NET_CONNECT(_name, _input, _output)                                         \
		netlist.register_link(# _name "." # _input, # _output);
#define NETDEV_PARAM(_name, _val)                                                   \
		netlist.find_param(# _name).initial(_val);

#define NETLIST_NAME(_name) netlist ## _ ## _name

#define NETLIST_START(_name) \
ATTR_COLD void NETLIST_NAME(_name)(netlist_setup_t &netlist) \
{

#define NETLIST_END  }

#define NETLIST_INCLUDE(_name)                                                      \
		NETLIST_NAME(_name)(netlist);

#define NETLIST_MEMREGION(_name)                                                    \
		netlist.parse((char *)downcast<netlist_t &>(netlist.netlist()).machine().root_device().memregion(_name)->base());

#if defined(__GNUC__) && (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#if !defined(__ppc__) && !defined (__PPC__) && !defined(__ppc64__) && !defined(__PPC64__)
#define ATTR_ALIGN __attribute__ ((aligned(128)))
#else
#define ATTR_ALIGN
#endif
#else
#define ATTR_ALIGN
#endif

//============================================================
//  MACROS / netlist devices
//============================================================

#define NETLIB_UPDATE(_chip) ATTR_HOT ATTR_ALIGN void _chip :: update(void)
#define NETLIB_START(_chip) ATTR_COLD void _chip :: start(void)
//#define NETLIB_CONSTRUCTOR(_chip) ATTR_COLD _chip :: _chip (netlist_setup_t &setup, const char *name)
//			: net_device_t(setup, name)

#define NETLIB_UPDATE_PARAM(_chip) ATTR_HOT ATTR_ALIGN void _chip :: update_param(void)
#define NETLIB_FUNC_VOID(_chip, _name, _params) ATTR_HOT ATTR_ALIGN inline void _chip :: _name _params

#define NETLIB_SIGNAL(_name, _num_input, _check, _invert)                           \
	class _name : public net_signal_t<_num_input, _check, _invert>                  \
	{                                                                               \
	public:                                                                         \
		_name ()										 					        \
		: net_signal_t<_num_input, _check, _invert>() { }							\
	};

#define NETLIB_DEVICE(_name, _priv)                                                 \
	class _name : public net_device_t                                               \
	{                                                                               \
	public:                                                                         \
		_name () 																	\
		: net_device_t()	{ }														\
	protected:                                                                      \
		ATTR_HOT void update();                                                     \
		ATTR_HOT void start();                                                      \
		_priv                                                                       \
	}

#define NETLIB_SUBDEVICE(_name, _priv)                                              \
	class _name : public net_core_device_t                                          \
	{                                                                               \
	public:                                                                         \
		_name ()										 							\
		: net_core_device_t()			 											\
		  { }																		\
	/*protected:*/                                                                  \
		ATTR_HOT void update();                                                     \
		_priv                                                                       \
	}

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)                                     \
	class _name : public net_device_t                                               \
	{                                                                               \
	public:                                                                         \
		_name ()										 							\
		: net_device_t() { }														\
		ATTR_HOT void update_param();                                               \
		ATTR_HOT void update();                                                     \
		ATTR_HOT void start();                                                      \
	/* protected: */                                                                \
		_priv                                                                       \
	}
// MAME specific

#define MCFG_NETLIST_ADD(_tag, _setup )                                             \
	MCFG_DEVICE_ADD(_tag, NETLIST, NETLIST_CLOCK)                                   \
	MCFG_NETLIST_SETUP(_setup)
#define MCFG_NETLIST_REPLACE(_tag, _setup)                                          \
	MCFG_DEVICE_REPLACE(_tag, NETLIST, NETLIST_CLOCK)                               \
	MCFG_NETLIST_SETUP(_setup)
#define MCFG_NETLIST_SETUP(_setup)                                                  \
	netlist_mame_device::static_set_constructor(*device, NETLIST_NAME(_setup));

// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------

class net_core_device_t;

#if USE_DELEGATES
#if USE_PMFDELEGATES
typedef void (*net_update_delegate)(net_core_device_t *);
#else
typedef delegate<void ()> net_update_delegate;
#endif
#endif

typedef UINT8 net_sig_t;


typedef delegate<void (const double)> net_output_delegate;

// ----------------------------------------------------------------------------------------
// Support classes
// ----------------------------------------------------------------------------------------

struct netlist_time
{
public:

	typedef UINT64 INTERNALTYPE;

	static const INTERNALTYPE RESOLUTION = NETLIST_INTERNAL_RES;

	inline netlist_time() : m_time(0) {}

	friend inline const netlist_time operator-(const netlist_time &left, const netlist_time &right);
	friend inline const netlist_time operator+(const netlist_time &left, const netlist_time &right);
	friend inline const netlist_time operator*(const netlist_time &left, const UINT64 factor);
	friend inline bool operator>(const netlist_time &left, const netlist_time &right);
	friend inline bool operator<(const netlist_time &left, const netlist_time &right);
	friend inline bool operator>=(const netlist_time &left, const netlist_time &right);
	friend inline bool operator<=(const netlist_time &left, const netlist_time &right);

	ATTR_HOT inline const netlist_time &operator=(const netlist_time &right) { m_time = right.m_time; return *this; }
	ATTR_HOT inline const netlist_time &operator+=(const netlist_time &right) { m_time += right.m_time; return *this; }

	ATTR_HOT inline const INTERNALTYPE as_raw() const { return m_time; }

	ATTR_HOT static inline const netlist_time from_nsec(const int ns) { return netlist_time((UINT64) ns * (RESOLUTION / U64(1000000000))); }
	ATTR_HOT static inline const netlist_time from_usec(const int us) { return netlist_time((UINT64) us * (RESOLUTION / U64(1000000))); }
	ATTR_HOT static inline const netlist_time from_msec(const int ms) { return netlist_time((UINT64) ms * (RESOLUTION / U64(1000))); }
	ATTR_HOT static inline const netlist_time from_hz(const UINT64 hz) { return netlist_time(RESOLUTION / hz); }
	ATTR_HOT static inline const netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw); }

	static const netlist_time zero;

protected:

	ATTR_HOT inline netlist_time(const INTERNALTYPE val) : m_time(val) {}

	INTERNALTYPE m_time;
};

ATTR_HOT inline const netlist_time operator-(const netlist_time &left, const netlist_time &right)
{
	return netlist_time::from_raw(left.m_time - right.m_time);
}

ATTR_HOT inline const netlist_time operator*(const netlist_time &left, const UINT64 factor)
{
	return netlist_time::from_raw(left.m_time * factor);
}

ATTR_HOT inline const netlist_time operator+(const netlist_time &left, const netlist_time &right)
{
	return netlist_time::from_raw(left.m_time + right.m_time);
}

ATTR_HOT inline bool operator<(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time < right.m_time);
}

ATTR_HOT inline bool operator>(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time > right.m_time);
}

ATTR_HOT inline bool operator<=(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time <= right.m_time);
}

ATTR_HOT inline bool operator>=(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time >= right.m_time);
}

template <class _ListClass, int _NumElements>
struct net_list_t
{
public:
	net_list_t()
	{
		m_list = global_alloc_array(_ListClass, _NumElements);
		m_ptr = m_list;
		m_ptr--;
	}
	~net_list_t()
	{
		global_free(m_list);
	}
	ATTR_HOT inline void add(const _ListClass elem)
	{
		assert(m_ptr-m_list <= _NumElements - 1);

		*(++m_ptr) = elem;
	}
	ATTR_HOT inline _ListClass *first() const { return m_list; }
	ATTR_HOT inline _ListClass *last() const  { return m_ptr; }
	ATTR_HOT inline _ListClass item(int i) const { return m_list[i]; }
	inline int count() const { return m_ptr - m_list + 1; }
	ATTR_HOT inline bool empty() const { return (m_ptr < m_list); }
	ATTR_HOT inline void clear() { m_ptr = m_list - 1; }
private:
	_ListClass * m_ptr;
	//_ListClass m_list[_NumElements];
	_ListClass *m_list;
};

// ----------------------------------------------------------------------------------------
// forward definitions
// ----------------------------------------------------------------------------------------

class net_output_t;
class net_core_device_t;
class net_param_t;
class netlist_setup_t;
class netlist_base_t;

// ----------------------------------------------------------------------------------------
// timed queue
// ----------------------------------------------------------------------------------------

//#define SIZE ((1 << _Size) - 1)

class netlist_timed_queue
{
public:

	static const int SIZE = ((1 << 11) - 1);

	typedef net_output_t element_t;

	struct entry_t
	{
	public:
		inline entry_t() {}
		inline entry_t(const netlist_time atime, element_t &elem) : m_time(atime), m_object(&elem) {}
		ATTR_HOT inline const netlist_time &time() const { return m_time; }
		ATTR_HOT inline element_t & object() const { return *m_object; }
	private:
		netlist_time m_time;
		element_t *m_object;
	};

	netlist_timed_queue()
	{
		clear();
	}

	ATTR_HOT inline bool is_empty() const { return (m_end == 0); }
	ATTR_HOT inline bool is_not_empty() const { return (m_end != 0); }

	ATTR_HOT ATTR_ALIGN void push(const entry_t &e);

	ATTR_HOT inline const entry_t &pop()
	{
		m_end--;
		return item(m_end);
	}

	ATTR_HOT inline const entry_t &peek() const
	{
		return item(m_end-1);
	}

	ATTR_COLD void clear()
	{
		m_end = 0;
	}
	// profiling

	INT32  	m_prof_start;
	INT32	m_prof_end;
	INT32   m_prof_sortmove;
	INT32   m_prof_sort;
private:
	ATTR_HOT inline const entry_t &item(const UINT32 x) const { return m_list[x]; }
	ATTR_HOT inline void set_item(const UINT32 x, const entry_t &aitem) { m_list[x] = aitem; }

	UINT32 m_end;
	entry_t m_list[SIZE + 1];

};

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

	net_terminal_t(const int atype) : net_object_t(atype) {}

	ATTR_COLD void init_terminal(net_core_device_t *dev);
	ATTR_HOT inline net_core_device_t * RESTRICT netdev() const { return m_netdev; }
	ATTR_HOT inline netlist_base_t * RESTRICT netlist() const { return m_netlist; }

private:
	net_core_device_t * RESTRICT m_netdev;
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
		, m_state(INP_STATE_ACTIVE)
	{}

	ATTR_COLD void init_input(net_core_device_t *dev, net_input_state astate = INP_STATE_ACTIVE);

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

	ATTR_HOT inline const net_sig_t Q() const;
	ATTR_HOT inline const net_sig_t last_Q() const;

	ATTR_COLD inline void set_thresholds(const double low, const double high)
	{
		m_low_thresh_V = low;
		m_high_thresh_V = high;
	}
};

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

class netdev_mainclock;

class net_output_t : public net_terminal_t
{
public:

	net_output_t(int atype);

	friend const net_sig_t logic_input_t::Q() const;
	friend const double analog_input_t::Q_Analog() const;
	friend const bool analog_input_t::is_highz() const;
	friend class netdev_mainclock;

	ATTR_HOT inline const net_sig_t last_Q() const  { return m_last_Q;  }
	ATTR_HOT inline const net_sig_t new_Q() const   { return m_new_Q;   }

	ATTR_COLD void register_con(net_input_t &inp);

	ATTR_HOT inline void update_devs();

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
	ATTR_HOT inline const net_sig_t Q() const
	{
		assert(object_type(SIGNAL_MASK) == SIGNAL_DIGITAL);
		return m_Q;
	}
	ATTR_HOT inline const double Q_Analog() const
	{
		assert(object_type(SIGNAL_MASK) == SIGNAL_ANALOG);
		return m_Q_analog;
	}

	ATTR_HOT inline void push_to_queue(const netlist_time &delay);

	net_sig_t m_last_Q;
	net_sig_t m_Q;
	net_sig_t m_new_Q;

	double m_Q_analog;
	double m_new_Q_analog;

	UINT32 m_num_cons;
private:
	ATTR_HOT void update_dev(const net_input_t *inp, const UINT32 mask);

	netlist_time m_time;

	INT32 m_active;

	UINT32 m_in_queue;    /* 0: not in queue, 1: in queue, 2: last was taken */

	net_input_t *m_cons[OUTPUT_MAX_CONNECTIONS];
};


class logic_output_t : public net_output_t
{
public:

	logic_output_t()
		: net_output_t(OUTPUT | SIGNAL_DIGITAL)
	{
		// Default to TTL
		m_low_V = 0.1;	// these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_high_V = 4.8;
	}

	ATTR_COLD void initial(const net_sig_t val) { m_Q = val; m_new_Q = val; m_last_Q = !val; }

	ATTR_HOT inline void set_Q(const net_sig_t newQ, const netlist_time &delay)
	{
		if (EXPECTED(newQ != m_new_Q))
		{
			m_new_Q = newQ;
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

	ATTR_COLD void initial(const double val) { m_Q_analog = val; m_new_Q_analog = val; }

	ATTR_HOT inline void set_Q(const double newQ, const netlist_time &delay)
	{
		if (newQ != m_new_Q_analog)
		{
			m_new_Q_analog = newQ;
			push_to_queue(delay);
		}
	}

};

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

class net_core_device_t : public net_object_t
{
public:

	net_core_device_t();

	virtual ~net_core_device_t();

	ATTR_COLD virtual void setup(netlist_setup_t &setup, const char *name);

	ATTR_COLD const char *name() const { return m_name; }

	ATTR_HOT virtual void update_param() {}

	ATTR_HOT const net_sig_t INPLOGIC_PASSIVE(logic_input_t &inp);

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

	ATTR_HOT inline const net_sig_t INPLOGIC(const logic_input_t &inp) const
	{
		assert(inp.state() != net_input_t::INP_STATE_PASSIVE);
		return inp.Q();
	}

	ATTR_HOT inline void OUTLOGIC(logic_output_t &out, const net_sig_t val, const netlist_time &delay)
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

	netlist_base_t *m_netlist;

private:

	const char *m_name;
};


class net_device_t : public net_core_device_t
{
public:

	net_device_t();

	virtual ~net_device_t();

	ATTR_COLD void setup(netlist_setup_t &setup, const char *name);

	ATTR_COLD const netlist_setup_t *setup() const { return m_setup; }

	ATTR_COLD bool variable_input_count() { return m_variable_input_count; }

	ATTR_COLD void register_sub(net_core_device_t &dev, const char *name);

	ATTR_COLD void register_output(const char *name, net_output_t &out);
	ATTR_COLD void register_output(net_core_device_t &dev, const char *name, net_output_t &out);

	ATTR_COLD void register_input(const char *name, net_input_t &in, net_input_t::net_input_state state = net_input_t::INP_STATE_ACTIVE);
	ATTR_COLD void register_input(net_core_device_t &dev, const char *name, net_input_t &in, net_input_t::net_input_state state = net_input_t::INP_STATE_ACTIVE);

	ATTR_COLD void register_link_internal(net_input_t &in, net_output_t &out, net_input_t::net_input_state aState);
	ATTR_COLD void register_link_internal(net_core_device_t &dev, net_input_t &in, net_output_t &out, net_input_t::net_input_state aState);

	net_list_t<const char *, 20> m_inputs;

protected:

	virtual void update() { }
	ATTR_HOT virtual void start() { }

	ATTR_COLD void register_param(const char *sname, net_param_t &param, const double initialVal = 0.0);
	ATTR_COLD void register_param(net_core_device_t &dev, const char *sname, net_param_t &param, const double initialVal = 0.0);

	netlist_setup_t *m_setup;
	bool m_variable_input_count;

private:
};

class net_param_t
{
public:
	net_param_t() { m_param = 0.0; }

	inline void setTo(const double param) { m_param = param; m_netdev->update_param(); }
	inline void setTo(const int param) { m_param = param; m_netdev->update_param(); }
	inline void initial(const double val) { m_param = val; }
	inline void initial(const int val) { m_param = val; }

	ATTR_HOT inline double Value() const        { return m_param;   }
	ATTR_HOT inline int    ValueInt() const     { return (int) m_param;     }

	ATTR_HOT inline net_core_device_t &netdev() const { return *m_netdev; }
	void set_netdev(net_core_device_t &dev) { m_netdev = &dev; }

private:

	double m_param;
	net_core_device_t *m_netdev;
};

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev>
class net_signal_base_t : public net_device_t
{
public:
	net_signal_base_t()
	: net_device_t(), m_active(1) { }

	ATTR_COLD void start()
	{
		const char *sIN[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };

		register_output("Q", m_Q);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i], net_input_t::INP_STATE_ACTIVE);
		}
		m_Q.initial(1);
	}

#if (USE_DEACTIVE_DEVICE)
	ATTR_HOT void inc_active()
	{
		if (m_active == 0)
		{
			update();
		}
		m_active++;
	}

	ATTR_HOT void dec_active()
	{
		m_active--;
		if (m_active == 0)
		{
			for (int i = 0; i< _numdev; i++)
				m_i[i].inactivate();
		}
	}
#endif

public:
	ttl_input_t m_i[_numdev];
	ttl_output_t m_Q;
	INT8 m_active;
};


template <int _numdev, UINT8 _check, UINT8 _invert>
class net_signal_t : public net_device_t
{
public:
	net_signal_t()
	: net_device_t(), m_active(1) { }

	ATTR_COLD void start()
	{
		const char *sIN[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };

		register_output("Q", m_Q);
		for (int i=0; i < _numdev; i++)
		{
			register_input(sIN[i], m_i[i], net_input_t::INP_STATE_ACTIVE);
		}
		m_Q.initial(1);
	}

	#if (USE_DEACTIVE_DEVICE)
		ATTR_HOT void inc_active()
		{
			if (m_active == 0)
			{
				update();
			}
			m_active++;
		}

		ATTR_HOT void dec_active()
		{
			m_active--;
			if (m_active == 0)
			{
				for (int i = 0; i< _numdev; i++)
					m_i[i].inactivate();
			}
		}
	#endif

	virtual void update()
	{
		static const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };
		int pos = -1;

		for (int i = 0; i< _numdev; i++)
		{
			this->m_i[i].activate();
			if (INPLOGIC(this->m_i[i]) == _check)
			{
				OUTLOGIC(this->m_Q, _check ^ (1 ^ _invert), times[_check]);// ? 15000 : 22000);
				pos = i;
				break;
			}
		}
		if (pos >= 0)
		{
			for (int i = 0; i < _numdev; i++)
				if (i != pos)
					this->m_i[i].inactivate();
		} else
			OUTLOGIC(this->m_Q,_check ^ (_invert), times[1-_check]);// ? 22000 : 15000);
	}

public:
	ttl_input_t m_i[_numdev];
	ttl_output_t m_Q;
	INT8 m_active;
};

#if 1
template <UINT8 _check, UINT8 _invert>
class xx_net_signal_t: public net_device_t
{
public:
	xx_net_signal_t()
	: net_device_t(), m_active(1) { }

	ATTR_COLD void start()
	{
		const char *sIN[2] = { "I1", "I2" };

		register_output("Q", m_Q);
		for (int i=0; i < 2; i++)
		{
			register_input(sIN[i], m_i[i], net_input_t::INP_STATE_ACTIVE);
		}
		m_Q.initial(1);
	}

	#if (USE_DEACTIVE_DEVICE)
		ATTR_HOT void inc_active()
		{
			if (m_active == 0)
			{
				update();
			}
			m_active++;
		}

		ATTR_HOT void dec_active()
		{
			m_active--;
			if (m_active == 0)
			{
				m_i[0].inactivate();
				m_i[1].inactivate();
			}
		}
	#endif


	ATTR_HOT ATTR_ALIGN void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };

		UINT8 res = _invert ^ 1 ^_check;
		m_i[0].activate();
		if (INPLOGIC(m_i[0]) ^ _check)
		{
			m_i[1].activate();
			if (INPLOGIC(m_i[1]) ^ _check)
			{
				res = _invert ^ _check;
			}
			else
				m_i[0].inactivate();
		} else {
			m_i[1].activate();
			if (INPLOGIC(m_i[1]) ^ _check)
				m_i[1].inactivate();
		}
		OUTLOGIC(m_Q, res, times[1 - res]);// ? 22000 : 15000);
	}

public:
	ttl_input_t m_i[2];
	ttl_output_t m_Q;
	INT8 m_active;

};

template <UINT8 _check, UINT8 _invert>
class net_signal_t<2, _check, _invert> : public xx_net_signal_t<_check, _invert>
{
public:
	net_signal_t()
	: xx_net_signal_t<_check, _invert>() { }
};

// The following did not improve performance
#if 0
template <UINT8 _check, UINT8 _invert>
class net_signal_t<3, _check, _invert> : public net_signal_base_t<3>
{
public:
	net_signal_t(netlist_setup_t &setup, const char *name)
	: net_signal_base_t<3>(setup, name)
	{
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };
		//const UINT8 res_tab[4] = {1, 1, 1, 0 };
		//const UINT8 ai1[4]     = {0, 1, 0, 0 };
		//const UINT8 ai2[4]     = {1, 0, 1, 0 };

		UINT8 res = _invert ^ 1 ^_check;
		m_i[0].activate();
		if (INPLOGIC(m_i[0]) ^ _check)
		{
			m_i[1].activate();
			if (INPLOGIC(m_i[1]) ^ _check)
			{
				m_i[2].activate();
				if (INPLOGIC(m_i[2]) ^ _check)
				{
					res = _invert ^ _check;
				}
				else
					m_i[1].inactivate();
			}
			else
			{
				if (INPLOGIC(m_i[2]) ^ _check)
					m_i[2].inactivate();
				m_i[0].inactivate();
			}
		} else {
			if (INPLOGIC(m_i[1]) ^ _check)
				m_i[1].inactivate();
		}
		m_Q.setTo(res, times[1 - res]);// ? 22000 : 15000);
	}

};
#endif
#endif

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

class netlist_setup_t
{
public:

	typedef tagmap_t<net_device_t *, 393> tagmap_devices_t;
	typedef tagmap_t<astring *, 393> tagmap_astring_t;
	typedef tagmap_t<net_param_t *, 393> tagmap_param_t;
	typedef tagmap_t<net_terminal_t *, 393> tagmap_terminal_t;

	//typedef tagmap_t<net_output_t *, 393> tagmap_output_t;
	//typedef tagmap_t<net_input_t *, 393> tagmap_input_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	netlist_base_t &netlist() { return m_netlist; }

	net_device_t *register_dev(net_device_t *dev);
	void remove_dev(const char *name);

	void register_output(net_core_device_t &dev, net_core_device_t &upd_dev, const char *name, net_output_t &out);
	void register_input(net_device_t &dev, net_core_device_t &upd_dev, const char *name, net_input_t &inp, net_input_t::net_input_state type);
	void register_alias(const char *alias, const char *out);
	void register_param(const char *sname, net_param_t *param);

	void register_link(const char *sin, const char *sout);

	net_output_t &find_output(const char *outname_in);
	net_param_t &find_param(const char *param_in);

	void register_callback(const char *devname, net_output_delegate delegate);

	void parse(char *buf);

	void resolve_inputs(void);

	/* not ideal, but needed for save_state */
	tagmap_terminal_t  m_terminals;

	void print_stats();

protected:

private:

	netlist_base_t &m_netlist;

	tagmap_devices_t m_devices;
	tagmap_astring_t m_alias;
	//tagmap_input_t  m_inputs;
	tagmap_param_t  m_params;
	tagmap_astring_t  m_links;

	net_output_t *find_output_exact(const char *outname_in);
	const char *resolve_alias(const char *name) const;
};

class netdev_mainclock;

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

class netlist_base_t
{
public:

	typedef netlist_timed_queue queue_t;

	netlist_base_t();
	virtual ~netlist_base_t();

	void set_clock_freq(UINT64 clockfreq);

	ATTR_HOT inline void push_to_queue(net_output_t &out, const netlist_time &attime)
	{
		m_queue.push(queue_t::entry_t(attime, out));
	}

	ATTR_HOT void process_list(INT32 &atime);

	ATTR_HOT inline const netlist_time &time() const { return m_time_ps; }

	ATTR_COLD void set_mainclock_dev(netdev_mainclock *dev) { m_mainclock = dev; }

	// FIXME: should'nt be public
	queue_t m_queue;

protected:
	// performance
	int m_perf_out_processed;
	int m_perf_inp_processed;
	int m_perf_inp_active;

private:
	netdev_mainclock *m_mainclock;
	netlist_time m_time_ps;
	UINT32   m_rem;
	UINT32  m_div;


	ATTR_HOT void update_time(const netlist_time t, INT32 &atime);

};

// ----------------------------------------------------------------------------------------
// netdev_mainclock
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(netdev_mainclock,
	ttl_output_t m_Q;

	net_param_t m_freq;
	netlist_time m_inc;

	ATTR_HOT inline static void mc_update(net_output_t &Q, const netlist_time &curtime);
);

// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

class netdev_analog_callback : public net_device_t
{
public:
	netdev_analog_callback()
		: net_device_t() { }

	ATTR_COLD void start()
	{
		register_input("IN", m_in);
	}

	void register_callback(net_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update();


private:
	analog_input_t m_in;
	net_output_delegate m_callback;
};

// ----------------------------------------------------------------------------------------
// netdev_*_const
// ----------------------------------------------------------------------------------------

#define NETDEV_TTL_CONST(_name, _v)                                                 \
		NET_REGISTER_DEV(netdev_ttl_const, _name)                                   \
		NETDEV_PARAM(_name.CONST, _v)
#define NETDEV_ANALOG_CONST(_name, _v)                                              \
		NET_REGISTER_DEV(netdev_analog_const, _name)                                \
		NETDEV_PARAM(_name.CONST, _v)

NETLIB_DEVICE_WITH_PARAMS(netdev_ttl_const,
	ttl_output_t m_Q;
	net_param_t m_const;
);

NETLIB_DEVICE_WITH_PARAMS(netdev_analog_const,
	analog_output_t m_Q;
	net_param_t m_const;
);

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
		m_last_Q = m_Q;
		netdev()->inc_active();
		m_Q = m_new_Q;
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
			m_Q = m_last_Q = m_new_Q;
			m_Q_analog = m_new_Q_analog;
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




ATTR_HOT inline const net_sig_t logic_input_t::Q() const
{
	return output()->Q();
}

ATTR_HOT inline const net_sig_t logic_input_t::last_Q() const
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
	virtual ~net_device_t_base_factory() {}
	virtual net_device_t *Create() const = 0;

	const char *name() const { return m_name; }
	const char *classname() const { return m_classname; }
protected:
	const char *m_name;                             /* device name */
	const char *m_classname;                        /* device class name */
};

template <class C>
class net_device_t_factory : public net_device_t_base_factory
{
public:
	net_device_t_factory(const char *name, const char *classname) { m_name = name; m_classname = classname; }
	net_device_t *Create() const
	{
		net_device_t *r = global_alloc_clear(C());
		//r->init(setup, name);
		return r;
	}
};

net_device_t *net_create_device_by_classname(const char *classname, netlist_setup_t &setup, const char *icname);
net_device_t *net_create_device_by_name(const char *name, netlist_setup_t &setup, const char *icname);

// ----------------------------------------------------------------------------------------
// MAME glue classes
// ----------------------------------------------------------------------------------------


class netlist_t : public netlist_base_t
{
public:

	netlist_t(device_t &parent)
	: netlist_base_t(),
		m_parent(parent)
	{}
	virtual ~netlist_t() { };

	inline running_machine &machine()   { return m_parent.machine(); }

	device_t &parent() { return m_parent; }

private:
	device_t &m_parent;
};

// ======================> netlist_mame_device

class netlist_mame_device : public device_t,
						public device_execute_interface
{
public:

	template<bool _Required, class _NETClass>
	class output_finder;
	class optional_output;
	template<class C>
	class required_output;
	class optional_param;
	class required_param;
	class on_device_start;

	// construction/destruction
	netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~netlist_mame_device() {}

	static void static_set_constructor(device_t &device, void (*setup_func)(netlist_setup_t &));

	netlist_setup_t &setup() { return *m_setup; }
	netlist_t &netlist() { return *m_netlist; }

	net_list_t<on_device_start *, 393> m_device_start_list;

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;

	ATTR_HOT virtual void execute_run();

	netlist_t *m_netlist;

	netlist_setup_t *m_setup;

private:

	void save_state();

	void (*m_setup_func)(netlist_setup_t &);

	int m_icount;


};

// ======================> netlist_output_finder

class netlist_mame_device::on_device_start
{
public:
	virtual bool OnDeviceStart() = 0;
	virtual ~on_device_start() {}
};

// device finder template
template<bool _Required, class _NETClass>
class netlist_mame_device::output_finder : public object_finder_base<_NETClass>,
		netlist_mame_device::on_device_start
{
public:
	// construction/destruction
	output_finder(device_t &base, const char *tag, const char *output)
		: object_finder_base<_NETClass>(base, tag), m_output(output) { }

	// finder
	virtual bool findit(bool isvalidation = false)
	{
		if (isvalidation) return true;
		device_t *device = this->m_base.subdevice(this->m_tag);
		m_netlist = dynamic_cast<netlist_mame_device *>(device);
		if (device != NULL && m_netlist == NULL)
		{
			void mame_printf_warning(const char *format, ...) ATTR_PRINTF(1,2);
			mame_printf_warning("Device '%s' found but is not netlist\n", this->m_tag);
		}
		m_netlist->m_device_start_list.add(this);
		return this->report_missing(m_netlist != NULL, "device", _Required);
	}

protected:
	netlist_mame_device *m_netlist;
	const char *m_output;
};

// optional device finder
class netlist_mame_device::optional_output : public netlist_mame_device::output_finder<false, net_output_t>
{
public:
	optional_output(device_t &base, const char *tag, const char *output) : output_finder<false, net_output_t>(base, tag, output) { }

	virtual ~optional_output() {};

	virtual bool OnDeviceStart()
	{
		this->m_target = &m_netlist->setup().find_output(m_output);
		return this->report_missing(this->m_target != NULL, "output", false);
	}

};

// required devices are similar but throw an error if they are not found
template<class C>
class netlist_mame_device::required_output : public netlist_mame_device::output_finder<true, C>
{
public:
	required_output(device_t &base, const char *tag, const char *output) : output_finder<true, C>(base, tag, output) { }

	virtual ~required_output() {};

	virtual bool OnDeviceStart()
	{
		this->m_target = (C *) &(this->m_netlist->setup().find_output(this->m_output));
		return this->report_missing(this->m_target != NULL, "output", true);
	}

};

// optional device finder
class netlist_mame_device::optional_param : public netlist_mame_device::output_finder<false, net_param_t>
{
public:
	optional_param(device_t &base, const char *tag, const char *output) : output_finder<false, net_param_t>(base, tag, output) { }

	virtual bool OnDeviceStart()
	{
		this->m_target = &m_netlist->setup().find_param(m_output);
		return this->report_missing(this->m_target != NULL, "parameter", false);
	}

};

// required devices are similar but throw an error if they are not found
class netlist_mame_device::required_param : public netlist_mame_device::output_finder<true, net_param_t>
{
public:
	required_param(device_t &base, const char *tag, const char *output) : output_finder<true, net_param_t>(base, tag, output) { }

	virtual bool OnDeviceStart()
	{
		this->m_target = &m_netlist->setup().find_param(m_output);
		return this->report_missing(this->m_target != NULL, "output", true);
	}
};


// device type definition
extern const device_type NETLIST;

#endif
