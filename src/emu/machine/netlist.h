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

#define USE_DELEGATES			(1)
#define USE_DELEGATES_A			(0)

#define NETLIST_CLOCK 	      	   (U64(1000000000))

#define NLTIME_FROM_NS(_t)	netlist_time::from_ns(_t)
#define NLTIME_FROM_US(_t)	netlist_time::from_us(_t)
#define NLTIME_FROM_MS(_t)	netlist_time::from_ms(_t)
#define NLTIME_IMMEDIATE	netlist_time::from_ns(0)

#define NETLIST_HIGHIMP_V	(1.23456e20)		/* some voltage we should never see */


//============================================================
//  MACROS / inline netlist definitions
//============================================================

#define NET_ALIAS(_alias, _name)													\
	netlist.register_alias(# _alias, # _name);										\

#define NET_NEW(_type , _name)  net_create_device_by_classname(# _type, &netlist, # _name)

#define NET_REGISTER_DEV(_type, _name)												\
		netlist.register_dev(NET_NEW(_type, _name));								\

#define NET_REMOVE_DEV(_name)														\
		netlist.remove_dev(# _name);												\

#define NET_REGISTER_SIGNAL(_type, _name)											\
		NET_REGISTER_DEV(_type ## _ ## sig, _name)									\

#define NET_CONNECT(_name, _input, _output)											\
		netlist.register_link(# _name "." # _input, # _output);						\

#define NETDEV_PARAM(_name, _val)													\
		netlist.find_param(# _name).initial(_val);

#define NETLIST_NAME(_name) netlist ## _ ## _name

#define NETLIST_START(_name) \
ATTR_COLD void NETLIST_NAME(_name)(netlist_setup_t &netlist) \
{ \

#define NETLIST_END  }

#define NETLIST_INCLUDE(_name) 														\
		NETLIST_NAME(_name)(netlist);												\


#define NETLIST_MEMREGION(_name) 													\
		netlist.parse((char *)downcast<netlist_t &>(netlist.netlist()).machine().root_device().memregion(_name)->base());		\

#if defined(__GNUC__) && (__GNUC__ >= 3)
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
#define NETLIB_START(_chip) ATTR_COLD ATTR_ALIGN void _chip :: start(void)
#define NETLIB_UPDATE_PARAM(_chip) ATTR_HOT ATTR_ALIGN void _chip :: update_param(void)
#define NETLIB_FUNC_VOID(_chip, _name) ATTR_HOT ATTR_ALIGN inline void _chip :: _name (void)

#define NETLIB_SIGNAL(_name, _num_input, _check)									\
	class _name : public net_signal_t<_num_input, _check>							\
	{																				\
	public:						 													\
		_name () : net_signal_t<_num_input, _check>() { }							\
	};																				\

#define NETLIB_DEVICE(_name, _priv)													\
	class _name : public net_device_t												\
	{																				\
	public:																			\
		_name () : net_device_t() { }												\
		ATTR_HOT void update();														\
		ATTR_COLD void start();														\
	protected:																		\
		_priv																		\
	}																				\

#define NETLIB_SUBDEVICE(_name, _priv)												\
	class _name : public net_core_device_t											\
	{																				\
	public:																			\
		_name () : net_core_device_t() { }											\
		ATTR_HOT void update();														\
	/*protected:*/																	\
		_priv																		\
	}																				\

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)										\
	class _name : public net_device_t												\
	{																				\
	public:																			\
		_name () : net_device_t() { }												\
		ATTR_HOT void update_param();												\
		ATTR_HOT void update();														\
		ATTR_COLD void start();														\
	/* protected: */																\
		_priv																		\
	}																				\

// MAME specific

#define MCFG_NETLIST_ADD(_tag, _setup )						  						\
    MCFG_DEVICE_ADD(_tag, NETLIST, NETLIST_CLOCK) 									\
    MCFG_NETLIST_SETUP(_setup) 														\

#define MCFG_NETLIST_REPLACE(_tag, _setup)						  					\
    MCFG_DEVICE_REPLACE(_tag, NETLIST, NETLIST_CLOCK)								\
    MCFG_NETLIST_SETUP(_setup) 														\

#define MCFG_NETLIST_SETUP(_setup) 													\
	netlist_mame_device::static_set_constructor(*device, NETLIST_NAME(_setup));		\


// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------


#if USE_DELEGATES || USE_DELEGATES_A
typedef delegate<void ()> net_update_delegate;
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

	static const INTERNALTYPE RESOLUTION = U64(1024000000000);

	inline netlist_time() : m_time(0) {}

	friend netlist_time operator-(const netlist_time &left, const netlist_time &right);
	friend netlist_time operator+(const netlist_time &left, const netlist_time &right);
	friend netlist_time operator*(const netlist_time &left, const UINT64 factor);
	friend bool operator>(const netlist_time &left, const netlist_time &right);
	friend bool operator<(const netlist_time &left, const netlist_time &right);
	friend bool operator>=(const netlist_time &left, const netlist_time &right);
	friend bool operator<=(const netlist_time &left, const netlist_time &right);

	inline netlist_time &operator=(const netlist_time &right) { m_time = right.m_time; return *this; }
	inline netlist_time &operator+=(const netlist_time &right) { m_time += right.m_time; return *this; }

	inline const INTERNALTYPE as_raw() const { return m_time; }

	static inline const netlist_time from_ns(const int ns) { return netlist_time((UINT64) ns * RESOLUTION / U64(1000000000)); }
	static inline const netlist_time from_us(const int us) { return netlist_time((UINT64) us * RESOLUTION / U64(1000000)); }
	static inline const netlist_time from_ms(const int ms) { return netlist_time((UINT64) ms * RESOLUTION / U64(1000)); }
	static inline const netlist_time from_hz(const UINT64 hz) { return netlist_time(RESOLUTION / hz); }
	static inline const netlist_time from_raw(const INTERNALTYPE raw) { return netlist_time(raw); }

	static const netlist_time zero;

protected:

	inline netlist_time(const INTERNALTYPE val) : m_time(val) {}

	INTERNALTYPE m_time;
};

inline netlist_time operator-(const netlist_time &left, const netlist_time &right)
{
	return netlist_time::from_raw(left.m_time - right.m_time);
}

inline netlist_time operator*(const netlist_time &left, const UINT64 factor)
{
	return netlist_time::from_raw(left.m_time * factor);
}

inline netlist_time operator+(const netlist_time &left, const netlist_time &right)
{
	return netlist_time::from_raw(left.m_time + right.m_time);
}

inline bool operator<(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time < right.m_time);
}

inline bool operator>(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time > right.m_time);
}

inline bool operator<=(const netlist_time &left, const netlist_time &right)
{
	return (left.m_time <= right.m_time);
}

inline bool operator>=(const netlist_time &left, const netlist_time &right)
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

#define SIZE ((1 << _Size) - 1)

template <class _QC, int _Size>
class netlist_timed_queue
{
public:
	struct entry_t
	{
	public:
		inline entry_t() {}
		inline entry_t(netlist_time atime, _QC elem) : m_time(atime), m_object(elem) {}
		ATTR_HOT inline const netlist_time &time() const { return m_time; }
		ATTR_HOT inline _QC object() const { return m_object; }
	private:
		netlist_time m_time;
		_QC m_object;
	};


	netlist_timed_queue()
	{
		clear();
	}

	ATTR_HOT inline bool is_empty() { return ((m_start & SIZE) == (m_end & SIZE)); }
	ATTR_HOT inline bool is_not_empty() { return ((m_start & SIZE) != (m_end & SIZE)); }

	ATTR_HOT void push(const entry_t &e);

	ATTR_HOT inline entry_t &pop()
	{
		m_end--;
		return item(m_end);
	}

	ATTR_COLD void clear()
	{
		m_end = m_start = (1 << _Size) >> 1;
	}
private:
	ATTR_HOT inline entry_t &item(UINT32 x) { return m_list[x & SIZE]; }
	ATTR_HOT inline void set_item(UINT32 x, const entry_t &aitem) { m_list[x & SIZE] = aitem; }

	UINT32 m_start;
	UINT32 m_end;
	entry_t m_list[SIZE + 1];
};

template <class _QC, int _Size>
ATTR_HOT ATTR_ALIGN void netlist_timed_queue<_QC, _Size>::push(const entry_t &e)
{
	if (is_empty() || (e.time() <= item(m_end - 1).time()))
	{
		set_item(m_end,  e);
		m_end++;
	}
	else if (e.time() >= item(m_start).time())
	{
		m_start--;
		set_item(m_start, e);
	}
	else
	{
		register UINT32 i = m_end;
		register UINT32 j = i - 1;
		m_end++;
		while ((e.time() > item(j).time()))
		{
			set_item(i, item(j));
			i = j;
			j--;
		}
		set_item(i, e);
	}
}
// ----------------------------------------------------------------------------------------
// forward definitions
// ----------------------------------------------------------------------------------------

class net_output_t;
class net_core_device_t;
class net_param_t;
class netlist_setup_t;
class netlist_base_t;

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

class net_object_t
{
public:
	enum type_t {
		INPUT = 0,
		OUTPUT = 1,
		DEVICE = 2,
		PARAM = 3,
		TYPE_MASK = 0x03,
		SIGNAL_DIGITAL = 0x00,
		SIGNAL_ANALOG =  0x10,
		SIGNAL_MASK = 	 0x10,
	};

	net_object_t(int atype)
		: m_objtype(atype) {}

	ATTR_HOT inline int object_type() const { return m_objtype; }
	ATTR_HOT inline int object_type(int mask) const { return m_objtype & mask; }

private:
	int m_objtype;
};


class net_input_t : public net_object_t
{
public:

	enum net_input_state {
		INP_STATE_PASSIVE = 0,
		INP_STATE_ACTIVE = 1,
		INP_STATE_HL = 2,
		INP_STATE_LH = 4,
	};

	net_input_t(const int atype) : net_object_t(atype), m_state(INP_STATE_ACTIVE) {}
	ATTR_COLD void init(net_core_device_t *dev,int astate = INP_STATE_ACTIVE);

	ATTR_HOT inline net_output_t * RESTRICT output() const { return m_output; }
	ATTR_HOT inline bool is_state(const net_input_state astate) { return (m_state & astate); }
	ATTR_HOT inline UINT32 state() const { return m_state; }

	ATTR_COLD void set_output(net_output_t *aout)	{ m_output = aout; }
	ATTR_HOT inline void inactivate();
	ATTR_HOT inline void activate();
	ATTR_HOT inline void activate_hl();
	ATTR_HOT inline void activate_lh();

	ATTR_HOT inline net_core_device_t * RESTRICT netdev() const { return m_netdev; }

	double m_low_thresh_V;
	double m_high_thresh_V;

#if USE_DELEGATES
	net_update_delegate h;
#endif

private:
	UINT32 m_state;
	net_core_device_t * RESTRICT m_netdev;
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

	ATTR_HOT inline net_sig_t Q() const;
	ATTR_HOT inline net_sig_t last_Q() const;

	ATTR_COLD inline void set_thresholds(double low, double high)
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

	ATTR_HOT inline bool is_highz() const;
	ATTR_HOT inline double Q_Analog() const;
};

//#define INPVAL(_x) (_x).Q()




// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

class net_output_t : public net_object_t
{
public:

	net_output_t(int atype);

	friend net_sig_t logic_input_t::Q() const;

	ATTR_HOT inline const net_sig_t last_Q() const	{ return m_last_Q; 	}
	ATTR_HOT inline const net_sig_t new_Q() const	{ return m_new_Q; 	}

	ATTR_HOT inline const double Q_Analog() const
	{
		switch (object_type(SIGNAL_MASK))
		{
		case SIGNAL_DIGITAL:	return m_Q ? m_high_V : m_low_V;
		case SIGNAL_ANALOG:		return m_Q_analog;
		default: 				assert(true);
		}

		return 0;
	}

	inline net_sig_t *Q_ptr()		{ return &m_Q; }
	inline net_sig_t *new_Q_ptr() 	{ return &m_new_Q; }

	ATTR_COLD void register_con(net_input_t &inp);

	ATTR_HOT void update_devs();
	ATTR_COLD void update_devs_force();

	ATTR_HOT inline const net_core_device_t *netdev() const { return m_netdev; }

	ATTR_HOT inline void inc_active();
	ATTR_HOT inline void dec_active() { m_active--; }
	ATTR_HOT inline int active_count() { return m_active; }

	ATTR_COLD void set_netdev(const net_core_device_t *dev);

protected:

	/* prohibit use in device functions
	 * current (pending) state can be inquired using new_Q()
	 */
	ATTR_HOT inline const net_sig_t Q() const	{ return m_Q; 	}

	ATTR_HOT inline void register_in_listPS(const netlist_time &delay_ps);

	ATTR_HOT inline void set_Q_PS(const net_sig_t newQ, const netlist_time &delay_ps)
	{
		if (newQ != m_new_Q)
		{
			m_new_Q = newQ;
			register_in_listPS(delay_ps);
		}
	}
	ATTR_HOT inline void set_Q_NoCheckPS(const net_sig_t val, const netlist_time &delay_ps)
	{
		m_new_Q = val;
		register_in_listPS(delay_ps);
	}

	ATTR_HOT inline void set_Q_PS_Analog(const double newQ, const netlist_time &delay_ps)
	{
		if (newQ != m_new_Q_analog)
		{
			m_new_Q_analog = newQ;
			register_in_listPS(delay_ps);
		}
	}
	ATTR_HOT inline void set_Q_NoCheckPS_Analog(const double val, const netlist_time &delay_ps)
	{
		m_new_Q_analog = val;
		register_in_listPS(delay_ps);
	}

	netlist_time m_time;
	int m_in_queue;

	net_sig_t m_Q;
	net_sig_t m_new_Q;
	net_sig_t m_last_Q;
	double m_Q_analog;
	double m_new_Q_analog;

	netlist_base_t *m_netlist;

	int m_active;
	int m_num_cons;
	net_input_t *m_cons[48];

	const net_core_device_t *m_netdev;
	double m_low_V;
	double m_high_V;

};

class logic_output_t : public net_output_t
{
public:

	logic_output_t()
		: net_output_t(OUTPUT | SIGNAL_DIGITAL)
	{
		// Default to TTL
		m_low_V = 0.3;
		m_high_V = 3.4;
	}

	ATTR_COLD void initial(const net_sig_t val) { m_Q = val; m_new_Q = val; m_last_Q = !val; }
	ATTR_HOT inline void clear() 	{ set_Q_PS(0, netlist_time::zero); }
	ATTR_HOT inline void set()   	{ set_Q_PS(1, netlist_time::zero); }
	ATTR_HOT inline void setToPS(const UINT8 val, const netlist_time &delay_ps) { set_Q_PS(val, delay_ps); }
	ATTR_HOT inline void setToNoCheckPS(const UINT8 val, const netlist_time &delay_ps) { set_Q_NoCheckPS(val, delay_ps); }
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

	ATTR_COLD void initial(double val) { m_Q_analog = val; m_new_Q_analog = val; }
	ATTR_HOT inline void setToPS(const double val, const netlist_time &delay_ps) { set_Q_PS_Analog(val,delay_ps); }
	ATTR_HOT inline void setToNoCheckPS(const double val, const netlist_time &delay_ps) { set_Q_NoCheckPS_Analog(val,delay_ps); }
};

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

class net_core_device_t : public net_object_t
{
public:

#if USE_DELEGATES
	friend class net_input_t; // for access to update
#endif

	net_core_device_t();

	virtual ~net_core_device_t();

	ATTR_COLD void init_core(netlist_base_t *anetlist, const char *name);

	ATTR_COLD const char *name() const { return m_name; }

	ATTR_HOT inline void update_device()
	{
#if USE_DELEGATES_A
		h();
#else
		update();
#endif
	}

	ATTR_HOT virtual void update_param() {}

	ATTR_HOT inline net_sig_t INPVAL_PASSIVE(logic_input_t &inp)
	{
		net_sig_t ret;
		inp.activate();
		ret = inp.Q();
		inp.inactivate();
		return ret;
	}

	ATTR_HOT inline net_sig_t INPVAL(const logic_input_t &inp)
	{
		assert(inp.state() != net_input_t::INP_STATE_PASSIVE);
		return inp.Q();
	}

	ATTR_HOT inline net_sig_t INPVAL_LAST(const logic_input_t &inp) { return inp.last_Q(); }

	ATTR_HOT inline double INPANALOG(const analog_input_t &inp) { return inp.Q_Analog(); }

	ATTR_HOT inline netlist_base_t *netlist() const { return m_netlist; }

	ATTR_COLD inline net_output_t * GETINPPTR(net_output_t &out) const { return &out; }

	net_list_t<net_core_device_t *, 20> m_subdevs;

	/* stats */
	osd_ticks_t total_time;
	volatile INT32 stat_count;

protected:

	ATTR_HOT virtual void update() { }

	ATTR_COLD void register_subdevice(net_core_device_t &subdev);

	netlist_base_t *m_netlist;

private:

	net_update_delegate h;
	const char *m_name;
};


class net_device_t : public net_core_device_t
{
public:

	net_device_t();

	virtual ~net_device_t();

	ATTR_COLD void init(netlist_setup_t *setup, const char *name);

	ATTR_COLD netlist_setup_t *setup() const { return m_setup; }

	ATTR_COLD virtual void start() {}

	ATTR_COLD bool variable_input_count() { return m_variable_input_count; }

	ATTR_COLD void register_output(const char *name, net_output_t &out);
	ATTR_COLD void register_output(const net_core_device_t &dev, const char *name, net_output_t &out);

	ATTR_COLD void register_input(const char *name, net_input_t &in, int state = net_input_t::INP_STATE_ACTIVE);
	ATTR_COLD void register_input(net_core_device_t &dev, const char *name, net_input_t &in, int state = net_input_t::INP_STATE_ACTIVE);

	ATTR_COLD void register_link_internal(net_input_t &in, net_output_t &out);
	ATTR_COLD void register_link_internal(net_core_device_t &dev, net_input_t &in, net_output_t &out);

	net_list_t<const char *, 20> m_inputs;

protected:

	ATTR_COLD void register_param(const char *sname, net_param_t &param, const double initialVal = 0.0);
	ATTR_COLD void register_param(net_core_device_t &dev, const char *sname, net_param_t &param, const double initialVal = 0.0);

	netlist_setup_t *m_setup;
	bool m_variable_input_count;

private:
};

class net_param_t
{
public:
	net_param_t() { }

	inline void setTo(const double param) { m_param = param; m_netdev->update_param(); }
	inline void setTo(const int param) { m_param = param; m_netdev->update_param(); }
	inline void initial(const double val) { m_param = val; }
	inline void initial(const int val) { m_param = val; }

	ATTR_HOT inline double Value() const		{ return m_param; 	}
	ATTR_HOT inline int    ValueInt() const 	{ return (int) m_param; 	}

	ATTR_HOT inline net_core_device_t &netdev() const { return *m_netdev; }
	void set_netdev(net_core_device_t &dev) { m_netdev = &dev; }

private:

	double m_param;
	net_core_device_t *m_netdev;
};

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev, int _check>
class net_signal_t : public net_device_t
{
public:
	net_signal_t()
	: net_device_t() { }

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

	ATTR_HOT inline void update()
	{
		static netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };
		for (int i=0; i< _numdev; i++)
		{
			m_i[i].activate();
			if (INPVAL(m_i[i]) == _check)
			{
				m_Q.setToPS(!_check, times[_check]);// ? 15000 : 22000);
				for (int j = i + 1; j < _numdev; j++)
					m_i[j].inactivate();
				return;
			}
			m_i[i].inactivate();
		}
		m_Q.setToPS(_check, times[1-_check]);// ? 22000 : 15000);
		for (int i = 0; i < _numdev; i++)
			m_i[i].activate();
	}

protected:
	ttl_input_t m_i[8];
	ttl_output_t m_Q;
};

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

class netlist_setup_t
{
public:

	typedef tagmap_t<net_device_t *, 393> tagmap_devices_t;
	typedef tagmap_t<astring *, 393> tagmap_astring_t;
	typedef tagmap_t<net_output_t *, 393> tagmap_output_t;
	typedef tagmap_t<net_input_t *, 393> tagmap_input_t;
	typedef tagmap_t<net_param_t *, 393> tagmap_param_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	netlist_base_t &netlist() { return m_netlist; }

	net_device_t *register_dev(net_device_t *dev);
	void remove_dev(const char *name);

	void register_output(const char *name, net_output_t *out);
	void register_input(const char *name, net_input_t *inp);
	void register_alias(const char *alias, const char *out);
	void register_param(const char *sname, net_param_t *param);

	void register_link(const char *sin, const char *sout);

	net_output_t &find_output(const char *outname_in);
	net_param_t &find_param(const char *param_in);

	void register_callback(const char *devname, net_output_delegate delegate);

	void parse(char *buf);

	void resolve_inputs(void);

	/* not ideal, but needed for save_state */
	tagmap_output_t  m_outputs;

	void print_stats();

protected:

private:

	netlist_base_t &m_netlist;

	tagmap_devices_t m_devices;
	tagmap_astring_t m_alias;
	tagmap_input_t  m_inputs;
	tagmap_param_t  m_params;
	tagmap_astring_t  m_links;

	net_output_t *find_output_exact(const char *outname_in);
	const char *resolve_alias(const char *name) const;
};

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

class netlist_base_t
{
public:

	struct entry_t
	{
	public:
		entry_t() {}
		entry_t(netlist_time atime, net_output_t *aout) : time(atime), out(aout) {}
		netlist_time time;
		net_output_t *out;
	};
	typedef netlist_timed_queue<net_output_t *, 9> queue_t;

	netlist_base_t();
	virtual ~netlist_base_t();

	void set_clock_freq(UINT64 clockfreq);

	ATTR_HOT inline void register_in_listPS1(net_output_t *out, const netlist_time &attime)
	{
		m_queue.push(queue_t::entry_t(attime, out));
	}

	ATTR_HOT void process_list(INT32 &atime);

	ATTR_HOT inline netlist_time &time() { return m_time_ps; }

protected:
	netlist_time m_time_ps;
	UINT32	 m_rem;
	UINT32 	m_div;

	queue_t m_queue;

	// performance
	int m_perf_out_processed;
	int m_perf_inp_processed;
	int m_perf_inp_active;
	UINT64 m_perf_list_len;


private:

};


// ----------------------------------------------------------------------------------------
// netdev_callback
// ----------------------------------------------------------------------------------------

class netdev_callback : public net_device_t
{
public:
	netdev_callback()
	: net_device_t() {}

	void register_callback(net_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update();
	ATTR_COLD void start()
	{
		register_input("IN", m_in);
	}


private:
	analog_input_t m_in;
	net_output_delegate m_callback;
};

// ----------------------------------------------------------------------------------------
// netdev_*_const
// ----------------------------------------------------------------------------------------

#define NETDEV_TTL_CONST(_name, _v)													\
		NET_REGISTER_DEV(netdev_ttl_const, _name)									\
		NETDEV_PARAM(_name.CONST, _v)												\

#define NETDEV_ANALOG_CONST(_name, _v)												\
		NET_REGISTER_DEV(netdev_analog_const, _name)								\
		NETDEV_PARAM(_name.CONST, _v)												\


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
		m_output->inc_active();
	m_state = INP_STATE_ACTIVE;
}

ATTR_HOT inline void net_input_t::activate_hl()
{
	if (m_state == INP_STATE_PASSIVE)
		m_output->inc_active();
	m_state = INP_STATE_HL;
}

ATTR_HOT inline void net_input_t::activate_lh()
{
	if (m_state == INP_STATE_PASSIVE)
		m_output->inc_active();
	m_state = INP_STATE_LH;
}


ATTR_HOT inline void net_output_t::register_in_listPS(const netlist_time &delay_ps)
{
	m_time = m_netlist->time() + delay_ps;
	m_in_queue = 0;		/* not queued */
	if (m_active > 0)
	{
		m_in_queue = 1;		/* pending */
		m_netlist->register_in_listPS1(this, m_time);
	}
}

ATTR_HOT inline void net_output_t::inc_active()
{
	m_active++;
	if (m_active == 1 && m_in_queue == 0)
	{
		if (m_time >= m_netlist->time())
		{
			m_in_queue = 1;		/* pending */
			m_netlist->register_in_listPS1(this, m_time);
		}
		else
		{
			m_Q = m_last_Q = m_new_Q;
			m_Q_analog = m_new_Q_analog;
			m_in_queue = 2;
		}
	}
}


ATTR_HOT inline net_sig_t logic_input_t::Q() const { return output()->Q(); }
ATTR_HOT inline net_sig_t logic_input_t::last_Q() const { return output()->last_Q(); }
ATTR_HOT inline double analog_input_t::Q_Analog() const { return output()->Q_Analog(); }
ATTR_HOT inline bool analog_input_t::is_highz() const { return output()->Q_Analog() == NETLIST_HIGHIMP_V; }

// ----------------------------------------------------------------------------------------
// net_dev class factory
// ----------------------------------------------------------------------------------------

class net_device_t_base_factory
{
public:
    virtual ~net_device_t_base_factory() {}
	virtual net_device_t *Create(netlist_setup_t *setup, const char *name) = 0;

	const char *name() { return m_name; }
	const char *classname() { return m_classname; }
protected:
	const char *m_name;								/* device name */
	const char *m_classname;						/* device class name */
};

template <class C>
class net_device_t_factory : public net_device_t_base_factory
{
public:
	net_device_t_factory(const char *name, const char *classname) { m_name = name; m_classname = classname; }
	net_device_t *Create(netlist_setup_t *setup, const char *name)
	{
		net_device_t *r = global_alloc_clear(C());
		r->init(setup, name);
		return r;
	}
};

net_device_t *net_create_device_by_classname(const char *classname, netlist_setup_t *setup, const char *icname);
net_device_t *net_create_device_by_name(const char *name, netlist_setup_t *setup, const char *icname);

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

	inline running_machine &machine()	{ return m_parent.machine(); }

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
	template<class _C>
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

    void step_one_clock();
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
class netlist_mame_device::output_finder : public device_t::object_finder_base<_NETClass>,
		netlist_mame_device::on_device_start
{
public:
	// construction/destruction
	output_finder(device_t &base, const char *tag, const char *output)
		: object_finder_base<_NETClass>(base, tag), m_output(output) { }

	// finder
	virtual bool findit()
	{
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

	virtual bool OnDeviceStart()
	{
		this->m_target = &m_netlist->setup().find_output(m_output);
		return this->report_missing(this->m_target != NULL, "output", false);
	}

};

// required devices are similar but throw an error if they are not found
template<class _C>
class netlist_mame_device::required_output : public netlist_mame_device::output_finder<true, _C>
{
public:
	required_output(device_t &base, const char *tag, const char *output) : output_finder<true, _C>(base, tag, output) { }

	virtual bool OnDeviceStart()
	{
		this->m_target = (_C *) &(this->m_netlist->setup().find_output(this->m_output));
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
