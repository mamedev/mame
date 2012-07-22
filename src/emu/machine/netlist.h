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
#define USE_DOUBLE				(0)

#define NET_LIST_SIZE			(32)
#define NET_LIST_MASK			(0x1f)

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
		netlist.find_param(# _name)->setTo(_val);

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
#define ATTR_ALIGN __attribute__ ((aligned(64)))
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

#define NETLIB_TIMER_CALLBACK(_chip) ATTR_HOT void _chip :: timer_cb(INT32 timer_id)

#define NETLIB_SIGNAL(_name, _num_input)											\
	class _name : public net_signal_t< _num_input >									\
	{																				\
	public:																			\
		_name (netlist_setup_t *parent, const char *name)							\
		: net_signal_t(parent, name) { }											\
		ATTR_HOT void update();														\
	};																				\

#define NETLIB_DEVICE(_name, _priv)													\
	class _name : public net_dev_t													\
	{																				\
	public:																			\
		_name (netlist_setup_t *parent, const char *name)							\
		: net_dev_t(parent, name) { }												\
		ATTR_HOT void update();														\
		ATTR_COLD void start();														\
	private:																		\
		_priv																		\
	}																				\

#define NETLIB_DEVICE_WITH_PARAMS(_name, _priv)										\
	class _name : public net_dev_t													\
	{																				\
	public:																			\
		_name (netlist_setup_t *parent, const char *name)							\
		: net_dev_t(parent, name) { }												\
		ATTR_HOT void update_param();												\
		ATTR_HOT void update();														\
		ATTR_COLD void start();														\
	private:																		\
		_priv																		\
	}																				\

// MAME specific

#define MCFG_NETLIST_ADD(_tag, _clock, _setup, _subcycles)  						\
    MCFG_DEVICE_ADD(_tag, NETLIST, _clock) 											\
    MCFG_NETLIST_SUBCYCLES(_subcycles)												\
    MCFG_NETLIST_SETUP(_setup) 														\

#define MCFG_NETLIST_REPLACE(_tag, _clock, _setup, _subcycles)  					\
    MCFG_DEVICE_REPLACE(_tag, NETLIST, _clock) 										\
    MCFG_NETLIST_SUBCYCLES(_subcycles)												\
    MCFG_NETLIST_SETUP(_setup) 														\

#define MCFG_NETLIST_SUBCYCLES(_subcycles) 											\
	netlist_mame_device::static_set_subcycles(*device, _subcycles); 				\

#define MCFG_NETLIST_SETUP(_setup) 													\
	netlist_mame_device::static_set_constructor(*device, NETLIST_NAME(_setup));		\


// ----------------------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------------------


enum net_input_type {
	NET_INP_TYPE_PASSIVE,
	NET_INP_TYPE_ACTIVE
};

enum { nst_LOW = 0, nst_HIGH = 1};
//typedef  double net_sig_t;
//enum net_sig_t { nst_LOW = 0, nst_HIGH = 1};

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

class net_output_t;

#if !USE_DOUBLE
#if 0
typedef  UINT8 net_sig_t;
#define INPVAL(_x) (*(_x))
#define GETINPPTR(_x) (_x).Q_ptr()
typedef net_sig_t * RESTRICT net_input_t;
#else
typedef UINT8 net_sig_t;
//#define INPVAL(_x) ((_x)->Q())
#define INPVAL(_x) INP(_x)
#define GETINPPTR(_x) (&(_x))
class net_input_t {
public:
	net_output_t * RESTRICT o;
};
#endif
#else
typedef  double net_sig_t;
#define INPVAL(_x) ((int)*(_x))
#define GETINPPTR(_x) (_x).Q_ptr()
typedef net_sig_t * RESTRICT net_input_t;
#endif



typedef delegate<void (const net_sig_t)> net_output_delegate;

#if USE_DELEGATES
typedef delegate<void ()> net_update_delegate;
typedef delegate<void (const net_output_t *out, UINT8 ns_delay)> net_register_delegate;
#endif

class net_dev_t;
class net_param_t;
class netlist_setup_t;
class netlist_base_t;

// ----------------------------------------------------------------------------------------
// net_list_t
// ----------------------------------------------------------------------------------------

template <class _ListClass, int _NumElements>
struct net_list_t
{
public:
	net_list_t() //(int max_elements = 64)
	{
		//m_list = global_alloc_array_clear(_ListClass, max_elements);
		m_ptr = m_list;
		m_ptr--;
	}
	ATTR_HOT inline void add(const _ListClass elem)
	{
		assert(m_ptr-m_list <= _NumElements - 1);

		*(++m_ptr) = elem;
	}
	ATTR_HOT inline _ListClass *first() { return m_list; }
	ATTR_HOT inline _ListClass *last()  { return m_ptr; }
	ATTR_HOT inline _ListClass item(int i) { return m_list[i]; }
	inline int count() { return m_ptr - m_list + 1; }
	ATTR_HOT inline bool empty() { return (m_ptr < m_list); }
	ATTR_HOT inline void clear() { m_ptr = m_list - 1; }
private:
	_ListClass * m_ptr;
	_ListClass m_list[_NumElements];
};

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

struct net_output_t
{
public:
	net_output_t();

	ATTR_HOT inline void clear() 	{ set_Q(nst_LOW); }
	ATTR_HOT inline void set()   	{ set_Q(nst_HIGH); }
	ATTR_HOT inline void setTo(const net_sig_t val) { set_Q(val); }
	ATTR_HOT inline void setTo(const net_sig_t val, const UINT8 delay_ns) { set_Q(val,delay_ns); }
	ATTR_HOT inline void setToNoCheck(const net_sig_t val)
	{
		m_new_Q = val;
		register_in_list();
	}

	ATTR_HOT inline void setToNoCheck(const net_sig_t val, const UINT8 delay_ns)
	{
		m_new_Q = val;
		register_in_list(delay_ns);
	}

	ATTR_COLD void initial(const net_sig_t val) { m_Q = val; m_new_Q = val; }

	ATTR_HOT inline const net_sig_t Q() const	{ return m_Q; 	}

	inline net_sig_t *Q_ptr()		{ return &m_Q; }
	inline net_sig_t *new_Q_ptr() 	{ return &m_new_Q; }

	ATTR_COLD void register_con(net_dev_t *dev);

	ATTR_HOT void update_devs();

	ATTR_HOT inline void update_out() {
		m_Q = m_new_Q;
		//m_QD = m_new_QD;
	}

	ATTR_HOT inline const net_dev_t *ttl_dev() { return m_ttldev; }

	ATTR_COLD void set_ttl_dev(net_dev_t *dev);

private:

	ATTR_HOT inline void register_in_list(const UINT8 delay_ns);
	ATTR_HOT inline void register_in_list();

	ATTR_HOT inline void set_Q(const net_sig_t newQ)
	{
		if (newQ != m_new_Q)
		{
			m_new_Q = newQ;
			//m_new_QD = newQ ? 1.0 : 0.0;
			register_in_list();
		}
	}

	ATTR_HOT inline void set_Q(const net_sig_t newQ, const UINT8 delay_ns)
	{
		if (newQ != m_new_Q)
		{
			m_new_Q = newQ;
			//m_new_QD = newQ ? 1.0 : 0.0;
			register_in_list(delay_ns);
		}
	}

	net_sig_t m_Q;
	net_sig_t m_new_Q;
	double m_QD;
	double m_new_QD;
	netlist_base_t *m_netlist;
	int m_num_cons;
#if USE_DELEGATES
	net_update_delegate m_cons[32];
#else
	//net_dev_t **m_cons;
	net_dev_t *m_cons[32];
#endif
	net_dev_t *m_ttldev;
};

// ----------------------------------------------------------------------------------------
// net_dev_t
// ----------------------------------------------------------------------------------------

class net_dev_t
{
public:
	net_dev_t(netlist_setup_t *setup, const char *name)
	: m_setup(setup), m_variable_input_count(false), m_name(name)
	{
		assert(name != NULL);
	}

	virtual ~net_dev_t();

	ATTR_HOT virtual void timer_cb(INT32 timer_id) {}

	const char *name() const { return m_name; }
	inline netlist_setup_t *setup() { return m_setup; }

	ATTR_HOT void update_timed();

	ATTR_HOT virtual void update_param() {}

	ATTR_HOT virtual void update() { }

	ATTR_HOT inline net_sig_t INP(net_input_t &v) { return v.o->Q(); }

	ATTR_COLD virtual void start() {}

	net_list_t<const char *, 20> m_inputs;

	ATTR_COLD bool variable_input_count() { return m_variable_input_count; }

	/* stats */
	osd_ticks_t total_time;
	volatile INT32 stat_count;

protected:

	void register_output(const char *name, net_output_t *out);
	void register_input(const char *name, net_input_t *in, net_input_type type = NET_INP_TYPE_ACTIVE);
	void register_param(const char *sname, net_param_t *param, double initialVal = 0.0);

	netlist_setup_t *m_setup;
	bool m_variable_input_count;

private:

	const char *m_name;
};



class net_param_t
{
public:
	net_param_t() { }

	inline void setTo(const double param) { m_param = param; m_ttldev->update_param(); }
	inline void setTo(const int param) { m_param = param; m_ttldev->update_param(); }
	inline void initial(const double val) { m_param = val; }
	inline void initial(const int val) { m_param = val; }

	inline double Value() 		{ return m_param; 	}
	inline int    ValueInt()	{ return m_param; 	}

	ATTR_HOT inline const net_dev_t *ttl_dev() { return m_ttldev; }
	void set_ttl_dev(net_dev_t *dev) { m_ttldev = dev; }

private:

	double m_param;
	net_dev_t *m_ttldev;
};

// ----------------------------------------------------------------------------------------
// net_signal_t
// ----------------------------------------------------------------------------------------

template <int _numdev>
class net_signal_t : public net_dev_t
{
public:
	net_signal_t(netlist_setup_t *parent, const char *name)
	: net_dev_t(parent, name) { }

	//ATTR_HOT virtual void update() = 0;
	ATTR_COLD void start()
	{
		const char *sIN[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };

		register_output("Q", &m_Q);
		for (int i=0; i < _numdev; i++)
			register_input(sIN[i], &m_i[i]);
	}

protected:
	net_input_t m_i[_numdev];
	net_output_t m_Q;
};

// ----------------------------------------------------------------------------------------
// netlist_base_timer_t
// ----------------------------------------------------------------------------------------

class netlist_base_timer_t
{
public:
	netlist_base_timer_t() {}
	virtual void adjust_timer(double delay) = 0;
};

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

class netlist_setup_t
{
public:

	struct net_input_setup_t {
	public:
		net_input_setup_t(net_input_t *inp, net_input_type type = NET_INP_TYPE_ACTIVE)
		: m_inp(inp), m_type(type) {}
		net_input_t *inp() { return m_inp; }
		net_input_type type() { return m_type; }
	private:
		net_input_t *m_inp;
		net_input_type m_type;
	};

	typedef tagmap_t<net_dev_t *, 393> tagmap_devices_t;
	typedef tagmap_t<astring *, 393> tagmap_astring_t;
	typedef tagmap_t<net_output_t *, 393> tagmap_output_t;
	typedef tagmap_t<net_input_setup_t *, 393> tagmap_input_t;
	typedef tagmap_t<net_param_t *, 393> tagmap_param_t;

	netlist_setup_t(netlist_base_t &netlist);
	~netlist_setup_t();

	netlist_base_t &netlist() { return m_netlist; }

	net_dev_t *register_dev(net_dev_t *dev);
	void remove_dev(const char *name);

	void register_output(const char *name, net_output_t *out);
	void register_input(const char *name, net_input_t *inp, net_input_type type = NET_INP_TYPE_ACTIVE);
	void register_alias(const char *alias, const char *out);
	void register_param(const char *sname, net_param_t *param);

	void register_link(const char *sin, const char *sout);

	net_output_t *find_output(const char *outname_in);
	net_param_t *find_param(const char *param_in);

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

	void step_all_devices();
	net_output_t *find_output_exact(const char *outname_in);
	const char *resolve_alias(const char *name) const;
};

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

class netlist_base_t
{
public:

	netlist_base_t(bool sub_cycle_exact);
	virtual ~netlist_base_t();

	virtual netlist_base_timer_t *alloc_timer(net_dev_t *dev, INT32 timer_id) = 0;

	void set_clock_freq(int clockfreq);
	void set_gatedelay(int gatedelay);

	ATTR_HOT inline void register_in_list(net_output_t *out, const UINT8 delay_ns)
	{
		m_output_list[m_sub_cycle_exact ? ((m_current + delay_ns / m_divisor) & m_netlist_mask) : 0].add(out);
	}

	ATTR_HOT inline void register_in_list(net_output_t *out)
	{
		m_output_list[m_sub_cycle_exact ? (m_current & m_netlist_mask) : 0].add(out);
	}

	ATTR_HOT inline void reset_list()
	{
		m_output_list[m_sub_cycle_exact ? m_current : 0].clear();
	}

	ATTR_COLD void reset_lists()
	{
		for (int i = 0; i <= m_netlist_mask; i++)
			m_output_list[i].clear();
	}

	ATTR_HOT void process_list(void);

protected:
	UINT8 m_current;
	UINT8 m_divisor;
	UINT8 m_netlist_mask;
	bool  m_sub_cycle_exact;
	net_list_t<net_output_t *, 512> m_output_list[NET_LIST_SIZE];
	int  m_gatedelay;
	int  m_clockfreq;

private:

};


// ----------------------------------------------------------------------------------------
// dev_callback
// ----------------------------------------------------------------------------------------

class netdev_callback : public net_dev_t
{
public:
	netdev_callback(netlist_setup_t *parent, const char *name)
	: net_dev_t(parent, name)
	{
		register_input("IN", &m_in);
	}

	void register_callback(net_output_delegate callback)
	{
		m_callback = callback;
	}

	ATTR_HOT void update();

private:
	net_input_t m_in;
	net_output_delegate m_callback;
};

// ----------------------------------------------------------------------------------------
// Inline implementations
// ----------------------------------------------------------------------------------------

ATTR_HOT inline void net_output_t::register_in_list(const UINT8 delay_ns)
{
	m_netlist->register_in_list(this, delay_ns);
}

ATTR_HOT inline void net_output_t::register_in_list()
{
	m_netlist->register_in_list(this);
}

// ----------------------------------------------------------------------------------------
// net_dev class factory
// ----------------------------------------------------------------------------------------

class net_dev_t_base_factory
{
public:
    virtual ~net_dev_t_base_factory() {}
	virtual net_dev_t *Create(netlist_setup_t *setup, const char *name) = 0;

	const char *name() { return m_name; }
	const char *classname() { return m_classname; }
protected:
	const char *m_name;								/* device name */
	const char *m_classname;						/* device class name */
};

template <class C>
class net_dev_t_factory : public net_dev_t_base_factory
{
public:
	net_dev_t_factory(const char *name, const char *classname) { m_name = name; m_classname = classname; }
	net_dev_t *Create(netlist_setup_t *setup, const char *name)
	{
		net_dev_t *r = global_alloc_clear(C(setup, name));
		return r;
	}
};

net_dev_t *net_create_device_by_classname(const char *classname, netlist_setup_t *setup, const char *icname);
net_dev_t *net_create_device_by_name(const char *name, netlist_setup_t *setup, const char *icname);

// ----------------------------------------------------------------------------------------
// MAME glue classes
// ----------------------------------------------------------------------------------------

class netlist_timer_t : public netlist_base_timer_t
{
public:
	netlist_timer_t(device_t &parent, net_dev_t *dev, INT32 timer_id)
	: netlist_base_timer_t()
	{
		m_param = timer_id;
		m_timer = parent.machine().scheduler().timer_alloc(timer_expired_delegate(&netlist_timer_t::timer_cb, "netlist_timer_t::timer_cb", this), dev);
	}

	virtual void adjust_timer(double delay)
	{
		m_timer->adjust(attotime::from_double(delay) ,m_param, attotime::never);
	}

	void timer_cb(void *ptr, INT32 param)
	{
		net_dev_t *dev = (net_dev_t *) ptr;
		dev->timer_cb(param);
	}
private:

	INT32	m_param;
	emu_timer *m_timer;
};

class netlist_t : public netlist_base_t
{
public:

	netlist_t(device_t &parent, bool sub_cycle_exact)
	: netlist_base_t(sub_cycle_exact),
	  m_parent(parent)
	{}
	virtual ~netlist_t() { };

	inline running_machine &machine()	{ return m_parent.machine(); }

	virtual netlist_base_timer_t *alloc_timer(net_dev_t *dev, INT32 timer_id)
	{
		netlist_timer_t *ret = new netlist_timer_t(m_parent, dev, timer_id);
		return ret;
	}

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
	class required_output;
	class optional_param;
	class required_param;
	class on_device_start;

	// construction/destruction
	netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_subcycles(device_t &device, int subcycles);
	static void static_set_constructor(device_t &device, void (*setup_func)(netlist_setup_t &));

	netlist_setup_t &setup() { return *m_setup; }
	netlist_t &netlist() { return *m_netlist; }

	//ATTR_HOT inline UINT64 Clocks() { return m_clockcnt; }
	ATTR_HOT inline int SubCycles() const { return m_subcycles; }

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
	net_output_t *m_clock_input;

	netlist_setup_t *m_setup;

private:

    void step_one_clock();
	void save_state();

	int m_clock;
	int m_subcycles;

	void (*m_setup_func)(netlist_setup_t &);

	UINT32 dummy[8];

	int m_icount;
	int m_ss;
	int m_clk;


};

// ======================> netlist_output_finder

class netlist_mame_device::on_device_start
{
public:
	virtual bool OnDeviceStart() = 0;
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
		this->m_target = m_netlist->setup().find_output(m_output);
		return this->report_missing(this->m_target != NULL, "output", false);
	}

};

// required devices are similar but throw an error if they are not found
class netlist_mame_device::required_output : public netlist_mame_device::output_finder<true, net_output_t>
{
public:
	required_output(device_t &base, const char *tag, const char *output) : output_finder<true, net_output_t>(base, tag, output) { }

	virtual bool OnDeviceStart()
	{
		this->m_target = m_netlist->setup().find_output(m_output);
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
		this->m_target = m_netlist->setup().find_param(m_output);
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
		this->m_target = m_netlist->setup().find_param(m_output);
		return this->report_missing(this->m_target != NULL, "output", true);
	}
};

// device type definition
extern const device_type NETLIST;

#endif
