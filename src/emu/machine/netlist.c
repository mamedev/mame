/***************************************************************************

    netlist.c

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

#include "netlist.h"
#include "net_lib.h"

//============================================================
//  DEBUGGING
//============================================================

#define VERBOSE					(0)
#define KEEP_STATISTICS			(0)

#if (VERBOSE)

#define VERBOSE_OUT(x)		printf x
#else
#define VERBOSE_OUT(x)
#endif

//============================================================
//  MACROS
//============================================================

#if KEEP_STATISTICS
#define add_to_stat(v,x)		do { v += (x); } while (0)
#define inc_stat(v)				add_to_stat(v, 1)
#define begin_timing(v)	 		do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)			do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)		do { } while (0)
#define inc_stat(v)				add_to_stat(v, 1)
#define begin_timing(v)			do { } while (0)
#define end_timing(v)			do { } while (0)
#endif


const netlist_time netlist_time::zero = netlist_time::from_raw(0);

// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

class netlist_parser
{
public:
	netlist_parser(netlist_setup_t &setup)
	: m_setup(setup) {}

	void parse(char *buf)
	{
		m_p = buf;
		while (*m_p)
		{
			char *n;
			skipws();
			if (!*m_p) break;
			n = getname('(');
			VERBOSE_OUT(("Parser: Device: %s\n", n));
			if (strcmp(n,"NET_ALIAS") == 0)
				net_alias();
			else if (strcmp(n,"NETDEV_PARAM") == 0)
				netdev_param();
			else if ((strcmp(n,"NETDEV_TTL_CONST") == 0) || (strcmp(n,"NETDEV_ANALOG_CONST") == 0))
				netdev_const(n);
			else
				netdev_device(n);
		}
	}

	void net_alias()
	{
		char *alias;
		char *out;
		skipws();
		alias = getname(',');
		skipws();
		out = getname(')');
		VERBOSE_OUT(("Parser: Alias: %s %s\n", alias, out));
		m_setup.register_alias(alias, out);
	}

	void netdev_param()
	{
		char *param;
		double val;
		skipws();
		param = getname(',');
		skipws();
		val = eval_param();
		VERBOSE_OUT(("Parser: Param: %s %f\n", param, val));
		m_setup.find_param(param).initial(val);
		check_char(')');
	}

	void netdev_const(const char *dev_name)
	{
		char *devname;
		net_device_t *dev;
		char paramfq[30];
		double val;

		skipws();
		devname = getname(',');
		dev = net_create_device_by_name(dev_name, &m_setup, devname);
		m_setup.register_dev(dev);
		skipws();
		val = eval_param();
		check_char(')');
		strcpy(paramfq, devname);
		strcat(paramfq, ".CONST");
		VERBOSE_OUT(("Parser: Const: %s %f\n", devname, val));
		m_setup.find_param(paramfq).setTo(val);
	}

	void netdev_device(const char *dev_type)
	{
		char *devname;
		net_device_t *dev;
		int cnt;

		skipws();
		devname = getname2(',', ')');
		dev = net_create_device_by_name(dev_type, &m_setup, devname);
		m_setup.register_dev(dev);
		skipws();
		VERBOSE_OUT(("Parser: IC: %s\n", n));
		cnt = 0;
		while (*m_p != ')')
		{
			m_p++;
			skipws();
			char *output_name = getname2(',', ')');
			VERBOSE_OUT(("Parser: ID: %s %s\n", output_name, dev->m_inputs.item(cnt)));
			m_setup.register_link(dev->m_inputs.item(cnt), output_name);
			skipws();
			cnt++;
		}
		if (cnt != dev->m_inputs.count() && !dev->variable_input_count())
			fatalerror("netlist: input count mismatch for %s - expected %d found %d\n", devname, dev->m_inputs.count(), cnt);
		check_char(')');
	}

private:

	char *cdup(const char *s)
	{
		return core_strdup(s);
	}

	void skipeol()
	{
		while (*m_p)
		{
			if (*m_p == 10)
			{
				m_p++;
				if (*m_p && *m_p == 13)
					m_p++;
				return;
			}
			m_p++;
		}
	}

	void skipws()
	{
		while (*m_p)
		{
			switch (*m_p)
			{
			case ' ':
			case 9:
			case 10:
			case 13:
				m_p++;
				break;
			case '/':
				if (*(m_p+1) == '/')
					skipeol();
				break;
			default:
				return;
			}
		}
	}

	char *getname(char sep)
	{
		static char buf[30];
		char *p1 = buf;

		while (*m_p != sep)
			*p1++ = *m_p++;
		*p1 = 0;
		m_p++;
		return cdup(buf);
	}

	char *getname2(char sep1, char sep2)
	{
		static char buf[30];
		char *p1 = buf;

		while ((*m_p != sep1) && (*m_p != sep2))
			*p1++ = *m_p++;
		*p1 = 0;
		return cdup(buf);
	}

	void check_char(char ctocheck)
	{
		skipws();
		if (*m_p == ctocheck)
		{
			m_p++;
			return;
		}
		fatalerror("Parser: expected '%c' found '%c'\n", ctocheck, *m_p);
	}

	double eval_param()
	{
		static const char *macs[6] = {"", "RES_K(", "RES_M(", "CAP_U(", "CAP_N(", "CAP_P("};
		static double facs[6] = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
		int i;
		int f=0;
		char *e;
		double ret;
		char *s = m_p;

		for (i=1; i<6;i++)
			if (strncmp(s, macs[i], strlen(macs[i])) == 0)
				f = i;
		ret = strtod(s+strlen(macs[f]), &e);
		if ((f>0) && (*e != ')'))
			fatalerror("Parser: Error with parameter ...\n");
		if (f>0)
			e++;
		m_p = e;
		return ret * facs[f];
	}

	char * m_p;
	netlist_setup_t &m_setup;

};


// ----------------------------------------------------------------------------------------
// netdev_a_to_d
// ----------------------------------------------------------------------------------------

class netdev_a_to_d_proxy : public net_device_t
{
public:
	netdev_a_to_d_proxy(net_input_t &in_proxied) : net_device_t()
	{
		assert_always(in_proxied.object_type(SIGNAL_MASK) == SIGNAL_DIGITAL, "Digital signal expected");
		m_I.m_high_thresh_V = in_proxied.m_high_thresh_V;
		m_I.m_low_thresh_V = in_proxied.m_low_thresh_V;
	}

	ATTR_HOT void update()
	{
		if (m_I.Q_Analog() > m_I.m_high_thresh_V)
			m_Q.setToPS(1, NLTIME_FROM_NS(1));
		else if (m_I.Q_Analog() < m_I.m_low_thresh_V)
			m_Q.setToPS(0, NLTIME_FROM_NS(1));
	}

	ATTR_COLD void start()
	{
		m_I.init(this);
		m_Q.set_netdev(this);

		m_Q.initial(1);
	}

	analog_input_t m_I;
	ttl_output_t m_Q;
};

// ----------------------------------------------------------------------------------------
// netdev_const
// ----------------------------------------------------------------------------------------

NETLIB_START(netdev_ttl_const)
{
	register_output("Q", m_Q);
	register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(netdev_ttl_const)
{
}

NETLIB_UPDATE_PARAM(netdev_ttl_const)
{
	m_Q.setToPS(m_const.ValueInt(), NLTIME_IMMEDIATE);
}

NETLIB_START(netdev_analog_const)
{
	register_output("Q", m_Q);
	register_param("CONST", m_const, 0.0);
}

NETLIB_UPDATE(netdev_analog_const)
{
}

NETLIB_UPDATE_PARAM(netdev_analog_const)
{
	m_Q.initial(m_const.Value());
}

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------


netlist_base_t::netlist_base_t()
	//m_output_list(ttl_list_t<output_t *>(2048)),
	//:  m_divisor(32), m_gatedelay(100), m_clockfreq(1000000)
	: m_div(1024)
{
}

netlist_base_t::~netlist_base_t()
{
}

void netlist_base_t::set_clock_freq(UINT64 clockfreq)
{
	m_div = netlist_time::from_hz(clockfreq).as_raw();
	VERBOSE_OUT(("Setting clock %lld and divisor %d\n", clockfreq, m_div));
}

ATTR_HOT ATTR_ALIGN void netlist_base_t::process_list(INT32 &atime)
{

	while ( (atime > 0) && (m_queue.is_not_empty()))
	{
		queue_t::entry_t e = m_queue.pop();
 		netlist_time delta = e.time() - m_time_ps + netlist_time::from_raw(m_rem);

		atime -= divu_64x32_rem(delta.as_raw(), m_div, &m_rem);
		m_time_ps = e.time();

		e.object()->update_devs();

		add_to_stat(m_perf_out_processed, 1);
		add_to_stat(m_perf_list_len, m_end);
	}

	if (atime > 0)
	{
		m_time_ps += netlist_time::from_raw(atime * m_div);
		atime = 0;
	}

	if (KEEP_STATISTICS)
		printf("%f\n", (double) m_perf_list_len / (double) m_perf_out_processed);
}

// ----------------------------------------------------------------------------------------
// Default netlist elements ...
// ----------------------------------------------------------------------------------------


static NETLIST_START(base)
	NETDEV_TTL_CONST(ttlhigh, 1)
	NETDEV_TTL_CONST(ttllow, 0)
	NETDEV_ANALOG_CONST(NC, NETLIST_HIGHIMP_V)
NETLIST_END

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

netlist_setup_t::netlist_setup_t(netlist_base_t &netlist)
	: m_netlist(netlist)
{
	NETLIST_NAME(base)(*this);
}

netlist_setup_t::~netlist_setup_t()
{
}


net_device_t *netlist_setup_t::register_dev(net_device_t *dev)
{
	if (!(m_devices.add(dev->name(), dev, false)==TMERR_NONE))
		fatalerror("Error adding %s to device list\n", dev->name());
	dev->start();
	return dev;
}

template <class T>
static void remove_start_with(T &hm, astring &sw)
{
	typename T::entry_t *entry = hm.first();
	while (entry != NULL)
	{
		typename T::entry_t *next = hm.next(entry);
		if (sw.cmpsubstr(entry->tag(), 0, sw.len()) == 0)
		{
			VERBOSE_OUT(("removing %s\n", entry->tag().cstr()));
			hm.remove(entry->object());
		}
		entry = next;
	}
}

void netlist_setup_t::remove_dev(const char *name)
{
	net_device_t *dev = m_devices.find(name);
	astring temp = name;
	if (dev == NULL)
		fatalerror("Device %s does not exist\n", name);

	temp.cat(".");

	remove_start_with<tagmap_input_t>(m_inputs, temp);
	remove_start_with<tagmap_output_t>(m_outputs, temp);
	remove_start_with<tagmap_param_t>(m_params, temp);
	remove_start_with<tagmap_astring_t>(m_links, temp);
	m_devices.remove(name);
}

void netlist_setup_t::register_callback(const char *devname, net_output_delegate delegate)
{
	netdev_callback *dev = (netdev_callback *) m_devices.find(devname);
	if (dev == NULL)
		fatalerror("did not find device %s\n", devname);
	dev->register_callback(delegate);
}

void netlist_setup_t::register_alias(const char *alias, const char *out)
{
	if (!(m_alias.add(alias, new astring(out), false)==TMERR_NONE))
		fatalerror("Error adding alias %s to alias list\n", alias);
}

void netlist_setup_t::register_output(const char *name, net_output_t *out)
{
	VERBOSE_OUT(("out %s\n", name));
	if (!(m_outputs.add(name, out, false)==TMERR_NONE))
		fatalerror("Error adding output %s to output list\n", name);
}

void netlist_setup_t::register_input(const char *name, net_input_t *inp)
{
	VERBOSE_OUT(("input %s\n", name));
	if (!(m_inputs.add(name, inp, false) == TMERR_NONE))
		fatalerror("Error adding input %s to input list\n", name);
}

void netlist_setup_t::register_link(const char *sin, const char *sout)
{
	VERBOSE_OUT(("link %s <== %s\n", sin, sout));
	if (!(m_links.add(sin, new astring(sout), false)==TMERR_NONE))
		fatalerror("Error adding link %s<==%s to link list\n", sin, sout);
}


void netlist_setup_t::register_param(const char *sname, net_param_t *param)
{
	astring temp = param->netdev().name();
	temp.cat(".");
	temp.cat(sname);
	if (!(m_params.add(temp, param, false)==TMERR_NONE))
		fatalerror("Error adding parameter %s to parameter list\n", sname);
}


const char *netlist_setup_t::resolve_alias(const char *name) const
{
	const astring *ret = m_alias.find(name);
	if (ret != NULL)
		return ret->cstr();
	return name;
}

net_output_t *netlist_setup_t::find_output_exact(const char *outname_in)
{
	net_output_t *ret = m_outputs.find(outname_in);

	return ret;
}

net_output_t &netlist_setup_t::find_output(const char *outname_in)
{
	const char *outname = resolve_alias(outname_in);
	net_output_t *ret;

	ret = find_output_exact(outname);
	/* look for default */
	if (ret == NULL)
	{
		/* look for ".Q" std output */
		astring s = outname;
		s.cat(".Q");
		ret = find_output_exact(s);
	}
	if (ret == NULL)
		fatalerror("output %s(%s) not found!\n", outname_in, outname);
	VERBOSE_OUT(("Found input %s\n", outname));
	return *ret;
}

net_param_t &netlist_setup_t::find_param(const char *param_in)
{
	const char *outname = resolve_alias(param_in);
	net_param_t *ret;

	ret = m_params.find(outname);
	if (ret == NULL)
		fatalerror("parameter %s(%s) not found!\n", param_in, outname);
	VERBOSE_OUT(("Found parameter %s\n", outname));
	return *ret;
}

void netlist_setup_t::resolve_inputs(void)
{
	VERBOSE_OUT(("Resolving ...\n"));
	for (tagmap_astring_t::entry_t *entry = m_links.first(); entry != NULL; entry = m_links.next(entry))
	{
		const astring *sout = entry->object();
		astring sin = entry->tag();
		net_input_t *in = m_inputs.find(sin);

		net_output_t  &out = find_output(sout->cstr());
		if (out.object_type(net_output_t::SIGNAL_MASK) == net_output_t::SIGNAL_ANALOG
				&& in->object_type(net_output_t::SIGNAL_MASK) == net_output_t::SIGNAL_DIGITAL)
		{
			// 			fatalerror("connecting analog output %s with %s\n", out.netdev()->name(), in->netdev()->name());
			// 			fatalerror("connecting analog output %s with %s\n", out.netdev()->name(), in->netdev()->name());
			netdev_a_to_d_proxy *proxy = new netdev_a_to_d_proxy(*in);
			proxy->init(this, "abc");
			proxy->start();
			in->set_output(proxy->GETINPPTR(proxy->m_Q));
			//Next check would not work with dynamic activation
			//if (in->state() != net_input_t::INP_STATE_PASSIVE)
				proxy->m_Q.register_con(*in);
			proxy->m_I.set_output(&out);
			//if (proxy->m_I.state() != net_input_t::INP_STATE_PASSIVE)
				out.register_con(proxy->m_I);
		}
		else
		{
			in->set_output(out.netdev()->GETINPPTR(out));

			//Next check would not work with dynamic activation
			//if (in->state() != net_input_t::INP_STATE_PASSIVE)
				out.register_con(*in);
		}
	}
	/* make sure params are set now .. */
	for (tagmap_param_t::entry_t *entry = m_params.first(); entry != NULL; entry = m_params.next(entry))
	{
		entry->object()->netdev().update_param();
	}

	/* make sure all outputs are triggered once */
	for (tagmap_output_t::entry_t *entry = m_outputs.first(); entry != NULL; entry = m_outputs.next(entry))
	{
		net_output_t *out = entry->object();
		out->update_devs_force();
		INT32 time = 1000;
		m_netlist.process_list(time);
	}
}

void netlist_setup_t::parse(char *buf)
{
	netlist_parser parser(*this);
	parser.parse(buf);
}

void netlist_setup_t::print_stats()
{
	if (KEEP_STATISTICS)
	{
		for (netlist_setup_t::tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
		{
			printf("Device %20s : %12d %15ld\n", entry->object()->name(), entry->object()->stat_count, (long int) entry->object()->total_time / (entry->object()->stat_count + 1));
		}
	}
}


// ----------------------------------------------------------------------------------------
// net_core_device_t
// ----------------------------------------------------------------------------------------

net_core_device_t::net_core_device_t()
: net_object_t(DEVICE)
{
}

net_core_device_t::~net_core_device_t()
{
}


ATTR_COLD void net_core_device_t::init_core(netlist_base_t *anetlist, const char *name)
{
	m_netlist = anetlist;
	m_name = name;
#if USE_DELEGATES_A
	h = net_update_delegate(&net_core_device_t::update, "update", this);
#endif
}


ATTR_COLD void net_core_device_t::register_subdevice(net_core_device_t &subdev)
{
	m_subdevs.add(&subdev);
	subdev.init_core(m_netlist, this->name());
	//subdev.start();
}

// ----------------------------------------------------------------------------------------
// net_dev_t
// ----------------------------------------------------------------------------------------


net_device_t::net_device_t()
: net_core_device_t(), m_variable_input_count(false)
{
}

net_device_t::~net_device_t()
{
}

ATTR_COLD void net_device_t::init(netlist_setup_t *setup, const char *name)
{
	m_setup = setup;
	init_core(&setup->netlist(), name);
}

void net_device_t::register_output(const net_core_device_t &dev, const char *name, net_output_t &port)
{
	astring temp = dev.name();
	temp.cat(".");
	temp.cat(name);
	port.set_netdev(&dev);
	m_setup->register_output(temp, &port);
}

void net_device_t::register_output(const char *name, net_output_t &port)
{
	register_output(*this, name, port);
}

void net_device_t::register_input(net_core_device_t &dev, const char *name, net_input_t &inp, int type)
{
	astring temp = dev.name();
	temp.cat(".");
	temp.cat(name);
	inp.init(&dev, type);
	m_inputs.add(core_strdup(temp.cstr()));
	m_setup->register_input(temp, &inp);
}

void net_device_t::register_input(const char *name, net_input_t &inp, int type)
{
	register_input(*this, name, inp, type);
}

void net_device_t::register_link_internal(net_core_device_t &dev, net_input_t &in, net_output_t &out)
{
	in.set_output(GETINPPTR(out));
	in.init(&dev);
	//if (in.state() != net_input_t::INP_STATE_PASSIVE)
		out.register_con(in);
}

void net_device_t::register_link_internal(net_input_t &in, net_output_t &out)
{
	register_link_internal(*this, in, out);
}

void net_device_t::register_param(net_core_device_t &dev, const char *name, net_param_t &param, double initialVal)
{
	param.set_netdev(dev);
	param.initial(initialVal);
	m_setup->register_param(name, &param);
}

void net_device_t::register_param(const char *name, net_param_t &param, double initialVal)
{
	register_param(*this,name, param, initialVal);
}

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

ATTR_COLD void net_input_t::init(net_core_device_t *dev, int astate)
{
	m_netdev = dev;
	m_state = astate;
#if USE_DELEGATES
	h = net_update_delegate(&net_core_device_t::update, "update", dev);
#endif
}

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

net_output_t::net_output_t(int atype)
	: net_object_t(atype)
{
	m_last_Q = 1;
	m_Q = 0;
	m_new_Q = m_Q;
	m_active = 0;
	m_in_queue = 2;
}

ATTR_COLD void net_output_t::set_netdev(const net_core_device_t *dev)
{
	m_netdev = dev;
	m_netlist = dev->netlist();
}

static inline void update_dev(const net_input_t *inp, const UINT32 mask)
{
	if ((inp->state() & mask) != 0)
	{
		begin_timing(inp->netdev()->total_time);
		inc_stat(inp->netdev()->stat_count);
#if USE_DELEGATES
		inp->h();
#else
		inp->netdev()->update_device();
#endif
		end_timing(inp->netdev()->total_time);
	}
}

ATTR_HOT inline void net_output_t::update_devs()
{

	const UINT32 masks[4] = { 1, 5, 3, 1 };
	m_Q = m_new_Q;
	m_Q_analog = m_new_Q_analog;

	//UINT32 mask = 1 | ((m_last_Q & (m_Q ^ 1)) << 1) | (((m_last_Q ^ 1) & m_Q) << 2);
	const UINT32 mask = masks[ (m_last_Q  << 1) | m_Q ];

	switch (m_num_cons)
	{
	case 2:
		update_dev(m_cons[1], mask);
	case 1:
		update_dev(m_cons[0], mask);
		break;
	default:
		{
			for (int i=0; i < m_num_cons; i++)
				update_dev(m_cons[i], mask);
		}
	case 0:
		break;
	}

	m_in_queue = 2; /* mark as taken ... */
	m_last_Q = m_Q;
}

ATTR_COLD  void net_output_t::update_devs_force()
{
	net_input_t **s = m_cons;
	int i =  m_num_cons;

	m_Q = m_new_Q;
	m_Q_analog = m_new_Q_analog;
	while (i-- > 0)
	{
		if (((*s)->state() & net_input_t::INP_STATE_ACTIVE) != 0)
#if USE_DELEGATES
			(*s)->h();
#else
			(*s)->netdev()->update_device();
#endif
		s++;
	}

	m_last_Q = m_Q;
}

ATTR_COLD void net_output_t::register_con(net_input_t &input)
{
	int i;
	if (m_num_cons >= ARRAY_LENGTH(m_cons))
		fatalerror("Connections exceeded for %s\n", m_netdev->name());

	/* keep similar devices together */
	for (i = 0; i < m_num_cons; i++)
#if USE_DELEGATES
		if (m_cons[i]->h == input.h)
			break;
#else
	if (m_cons[i]->netdev() == input.netdev())
		break;
#endif
	for (int j = m_num_cons; j > i; j--)
		m_cons[j] = m_cons[j - 1];
	m_cons[i] = &input;
	m_num_cons++;
	if (input.state() != net_input_t::INP_STATE_PASSIVE)
		m_active++;
}

NETLIB_UPDATE(netdev_callback)
{
	// FIXME: Remove after device cleanup
	if (!m_callback.isnull())
		m_callback(INPANALOG(m_in));
}

// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------



const device_type NETLIST = &device_creator<netlist_mame_device>;

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST, "netlist", tag, owner, clock),
	  device_execute_interface(mconfig, *this)
{
}

void netlist_mame_device::static_set_constructor(device_t &device, void (*setup_func)(netlist_setup_t &))
{
	netlist_mame_device &netlist = downcast<netlist_mame_device &>(device);
	netlist.m_setup_func = setup_func;
}

void netlist_mame_device::device_config_complete()
{
}

void netlist_mame_device::device_start()
{

	//double dt = clocks_to_attotime(1).as_double();
	m_netlist = global_alloc_clear(netlist_t(*this));
    m_netlist->set_clock_freq(this->clock());

	m_setup = global_alloc_clear(netlist_setup_t(*m_netlist));

	m_setup_func(*m_setup);

	bool allok = true;
	for (on_device_start **ods = m_device_start_list.first(); ods <= m_device_start_list.last(); ods++)
		allok &= (*ods)->OnDeviceStart();

	if (!allok)
		fatalerror("required elements not found\n");

    m_setup->resolve_inputs();

	//save_item(NAME(m_clockcnt));
	save_state();
	/* TODO: we have to save the round robin queue as well */

	// set our instruction counter
	m_icountptr = &m_icount;
}

void netlist_mame_device::device_reset()
{
}

void netlist_mame_device::device_stop()
{
	m_setup->print_stats();
}

void netlist_mame_device::device_post_load()
{

}

void netlist_mame_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{

}

void netlist_mame_device::save_state()
{
	for (netlist_setup_t::tagmap_output_t::entry_t *entry = m_setup->m_outputs.first(); entry != NULL; entry = m_setup->m_outputs.next(entry))
	{
		save_item(*entry->object()->Q_ptr(), entry->tag().cstr(), 0);
		save_item(*entry->object()->new_Q_ptr(), entry->tag().cstr(), 1);
	}
}

UINT64 netlist_mame_device::execute_clocks_to_cycles(UINT64 clocks) const
{
	return clocks;
}

UINT64 netlist_mame_device::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles;
}

ATTR_HOT void netlist_mame_device::execute_run()
{
	//bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);

	// debugging
	//m_ppc = m_pc;	// copy PC to previous PC
	//if (check_debugger)
	//	debugger_instruction_hook(this, 0); //m_pc);

	m_netlist->process_list(m_icount);

}



