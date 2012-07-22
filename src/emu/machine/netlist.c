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

//============================================================
//  DEBUGGING
//============================================================

#define VERBOSE					(0)
#define KEEP_STATISTICS			(0)


#if KEEP_STATISTICS && USE_DELEGATES
#error "Statistics only work without delegates!"
#endif

#if (VERBOSE)

#define VERBOSE_OUT(x)		printf x
#else
#define VERBOSE_OUT(x)
#endif

//============================================================
//  MACROS
//============================================================

#if KEEP_STATISTICS
#define add_to_stat(v,x)		do { atomic_add32((v), (x)); } while (0)
#define inc_stat(v)				add_to_stat(v, 1)
#define begin_timing(v)	 		do { (v) -= get_profile_ticks(); } while (0)
#define end_timing(v)			do { (v) += get_profile_ticks(); } while (0)
#else
#define add_to_stat(v,x)		do { } while (0)
#define inc_stat(v)				add_to_stat(v, 1)
#define begin_timing(v)			do { } while (0)
#define end_timing(v)			do { } while (0)
#endif

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
			{
				char *alias;
				char *out;
				m_p++;
				skipws();
				alias = getname(',');
				m_p++;
				skipws();
				out = getname(')');
				m_p++;
				VERBOSE_OUT(("Parser: Alias: %s %s\n", alias, out));
				m_setup.register_alias(alias, out);
			}
			else if (strcmp(n,"NETDEV_PARAM") == 0)
			{
				char *param;
				double val;
				m_p++;
				skipws();
				param = getname(',');
				m_p++;
				skipws();
				val = eval_param();
				m_p++;
				VERBOSE_OUT(("Parser: Param: %s %f\n", param, val));
				m_setup.find_param(param)->setTo(val);
			}
			else if (strcmp(n,"NETDEV_CONST") == 0)
			{
				char *devname;
				net_dev_t *dev;
				char paramfq[30];
				double val;
				m_p++;
				skipws();
				devname = getname(',');
				dev = net_create_device_by_name(n, &m_setup, devname);
				m_setup.register_dev(dev);
				m_p++;
				skipws();
				val = eval_param();
				m_p++;
				strcpy(paramfq, devname);
				strcat(paramfq, ".CONST");
				VERBOSE_OUT(("Parser: Const: %s %f\n", devname, val));
				m_setup.find_param(paramfq)->setTo(val);
			}
			else
			{
				char *devname;
				net_dev_t *dev;
				int cnt;

				m_p++;
				skipws();
				devname = getname2(',', ')');
				dev = net_create_device_by_name(n, &m_setup, devname);
				m_setup.register_dev(dev);
				skipws();
				VERBOSE_OUT(("Parser: IC: %s\n", n));
				cnt = 0;
				while (*m_p != ')')
				{
					m_p++;
					skipws();
					n = getname2(',', ')');
					VERBOSE_OUT(("Parser: ID: %s %s\n", n, dev->m_inputs.item(cnt)));
					m_setup.register_link(dev->m_inputs.item(cnt), n);
					skipws();
					cnt++;
					//return 0;
				}
				if (cnt != dev->m_inputs.count() && !dev->variable_input_count())
					fatalerror("netlist: input count mismatch for %s - expected %d found %d\n", devname, dev->m_inputs.count(), cnt);
				m_p++;
			}
		}
	}

private:
	void skipeol()
	{
		while (*m_p)
		{
			if (*m_p == 10)
			{
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
		return core_strdup(buf);
	}

	char *getname2(char sep1, char sep2)
	{
		static char buf[30];
		char *p1 = buf;

		while ((*m_p != sep1) && (*m_p != sep2))
			*p1++ = *m_p++;
		*p1 = 0;
		return core_strdup(buf);
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
			exit(0);
		if (f>0)
			e++;
		m_p = e;
		return ret * facs[f];
	}

	char * m_p;
	netlist_setup_t &m_setup;

};


// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------


netlist_base_t::netlist_base_t(bool sub_cycle_exact)
	//m_output_list(ttl_list_t<output_t *>(2048)),
	:  m_current(0), m_divisor(32), m_sub_cycle_exact(sub_cycle_exact), m_gatedelay(100), m_clockfreq(1000000)
{
	m_netlist_mask = NET_LIST_MASK;
	reset_lists();
}

netlist_base_t::~netlist_base_t()
{
}

void netlist_base_t::set_clock_freq(int clockfreq)
{
	m_clockfreq = clockfreq;
	m_divisor = 1000000000L * 100L / (m_clockfreq) / m_gatedelay;
	VERBOSE_OUT(("Divisor %d\n", m_divisor));
}

void netlist_base_t::set_gatedelay(int gatedelay)
{
	m_gatedelay = gatedelay;
	m_divisor = 1000000000L * 100L / (m_clockfreq) / m_gatedelay;
	VERBOSE_OUT(("Divisor %d\n", m_divisor));
}

ATTR_HOT inline void netlist_base_t::process_list(void)
{
	net_list_t<net_output_t *, 512> &list =  m_output_list[m_sub_cycle_exact ? m_current : 0];
#if 0
	net_output_t * RESTRICT * RESTRICT first;
	net_output_t * RESTRICT * RESTRICT last;

	first = list.first();
	last = list.last();
	while (first <= last)
	{
		net_output_t * RESTRICT * RESTRICT out = first;
		while (out <= last)
		{
			(*out)->update_out();
			(*out)->update_devs();
			out++;
		}
		first = last + 1;
		last = list.last();
	}
#else
	net_output_t * RESTRICT * RESTRICT out;

	out = list.first();
	while (out <= list.last())
	{
		(*out)->update_out();
		(*out)->update_devs();
		out++;
	}
#endif
	reset_list();

	if (m_sub_cycle_exact)
		m_current = (m_current + 1) & m_netlist_mask;
}

// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

netlist_setup_t::netlist_setup_t(netlist_base_t &netlist)
	//m_output_list(ttl_list_t<output_t *>(2048)),
	: m_netlist(netlist)
{
}

netlist_setup_t::~netlist_setup_t()
{
}


net_dev_t *netlist_setup_t::register_dev(net_dev_t *dev)
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
	net_dev_t *dev = m_devices.find(name);
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

void netlist_setup_t::register_input(const char *name, net_input_t *inp, net_input_type type)
{
	net_input_setup_t *setup_inp = new net_input_setup_t(inp, type);
	VERBOSE_OUT(("input %s\n", name));
	if (!(m_inputs.add(name, setup_inp, false) == TMERR_NONE))
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
	astring temp = param->ttl_dev()->name();
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

net_output_t *netlist_setup_t::find_output(const char *outname_in)
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
		fatalerror("output %s(%s) not found!", outname_in, outname);
	VERBOSE_OUT(("Found input %s\n", outname));
	return ret;
}

net_param_t *netlist_setup_t::find_param(const char *param_in)
{
	const char *outname = resolve_alias(param_in);
	net_param_t *ret;

	ret = m_params.find(outname);
	if (ret == NULL)
		fatalerror("parameter %s(%s) not found!", param_in, outname);
	VERBOSE_OUT(("Found parameter %s\n", outname));
	return ret;
}

void netlist_setup_t::resolve_inputs(void)
{
	VERBOSE_OUT(("Resolving ...\n"));
	for (tagmap_astring_t::entry_t *entry = m_links.first(); entry != NULL; entry = m_links.next(entry))
	{
		const astring *sout = entry->object();
		astring sin = entry->tag();
		net_input_setup_t *in = m_inputs.find(sin);
		int p = sin.find(".");
		const char *devname = sin.substr(0, p);
		net_dev_t *dev = m_devices.find(devname);
		net_output_t *out = find_output(sout->cstr() );

		(*in->inp()).o = GETINPPTR(*out);
		//in->inp()->v = out->Q_ptr();
		if (in->type() == NET_INP_TYPE_ACTIVE)
			out->register_con(dev);
	}

	step_all_devices();
	//reset_list();
    //process_list();
}

void netlist_setup_t::step_all_devices()
{
    m_netlist.reset_list();

	for (tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
	{
		net_dev_t &dev = *entry->object();
		dev.update_param();
		dev.update();

		for (tagmap_output_t::entry_t *entry = m_outputs.first(); entry != NULL; entry = m_outputs.next(entry))
		{
			net_output_t &out = *entry->object();
			out.update_out();
		}
		m_netlist.reset_lists();
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
// net_dev_t
// ----------------------------------------------------------------------------------------

net_dev_t::~net_dev_t()
{
}

ATTR_HOT void net_dev_t::update_timed()
{
	inc_stat(&stat_count);
	begin_timing(total_time);
	update();
	end_timing(total_time);
}

void net_dev_t::register_output(const char *name, net_output_t *port)
{
	astring temp = this->name();
	temp.cat(".");
	temp.cat(name);
	port->set_ttl_dev(this);
	m_setup->register_output(temp, port);
}

void net_dev_t::register_input(const char *name, net_input_t *inp, net_input_type type)
{
	astring temp = this->name();
	temp.cat(".");
	temp.cat(name);
	m_inputs.add(core_strdup(temp.cstr()));
	m_setup->register_input(temp, inp, type);
}

void net_dev_t::register_param(const char *name, net_param_t *param, double initialVal)
{
	param->set_ttl_dev(this);
	param->initial(initialVal);
	m_setup->register_param(name, param);
}

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

net_output_t::net_output_t()
{
#if USE_DELEGATES
	//m_cons = global_alloc_array_clear(net_update_delegate, 32);
#else
	// m_cons = global_alloc_array_clear(net_dev_t *, 32);
#endif
	//m_Q = parent.alloc_sig();
	//m_new_Q = parent.alloc_sig();
	m_num_cons = 0;
	m_Q = 0;
	m_new_Q = m_Q;
}

ATTR_COLD void net_output_t::set_ttl_dev(net_dev_t *dev)
{
	m_ttldev = dev;
	m_netlist = &dev->setup()->netlist();
}


ATTR_COLD void net_output_t::register_con(net_dev_t *dev)
{
	assert(m_num_cons<32);
#if USE_DELEGATES
	net_update_delegate aDelegate = net_update_delegate(&net_dev_t::update, "update", dev);
	for (int i=0; i < m_num_cons; i++)
		if (m_cons[i] == aDelegate)
			return;
	m_cons[m_num_cons++] = aDelegate;
#else
	for (int i=0; i < m_num_cons; i++)
		if (m_cons[i] == dev)
			return;
	m_cons[m_num_cons++] = dev;
#endif
}

ATTR_HOT inline void net_output_t::update_devs()
{
#if USE_DELEGATES

	net_update_delegate *s = m_cons;
	net_update_delegate *e = s + m_num_cons;

	while (s<e)
	{
		(*s++)();
	}
#else
	net_dev_t **s = m_cons;
	net_dev_t **e = s + m_num_cons;

#if KEEP_STATISTICS
	while (s<e)
		(*s++)->update_timed();
#else
	while (s<e)
		(*s++)->update();
#endif
#endif
}

NETLIB_UPDATE(netdev_callback)
{
	// FIXME: Remove after device cleanup
	if (!m_callback.isnull())
		m_callback(INPVAL(m_in));
}

// ----------------------------------------------------------------------------------------
// netlist_mame_device
// ----------------------------------------------------------------------------------------



const device_type NETLIST = &device_creator<netlist_mame_device>;

netlist_mame_device::netlist_mame_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NETLIST, "netlist", tag, owner, clock),
	  device_execute_interface(mconfig, *this)
{
	m_clock = clock;
}

void netlist_mame_device::static_set_subcycles(device_t &device, int subcycles)
{
	netlist_mame_device &netlist = downcast<netlist_mame_device &>(device);

	assert((subcycles & 1) == 0);
	assert( subcycles > 0);

	netlist.m_subcycles = subcycles;
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
	m_netlist = global_alloc_clear(netlist_t(*this, m_subcycles > 2));
    m_netlist->set_clock_freq(m_clock *  m_subcycles);

	m_setup = global_alloc_clear(netlist_setup_t(*m_netlist));

	m_setup_func(*m_setup);

	bool allok = true;
	for (on_device_start **ods = m_device_start_list.first(); ods <= m_device_start_list.last(); ods++)
		allok &= (*ods)->OnDeviceStart();

	if (!allok)
		fatalerror("required elements not found\n");

    m_setup->resolve_inputs();

    //m_clockcnt = 0;

	m_clock_input = m_setup->find_output("clk");

	//save_item(NAME(m_clockcnt));
	save_state();
	/* TODO: we have to save the round robin queue as well */

	// set our instruction counter
	m_icountptr = &m_icount;
	m_ss = SubCycles() / 2;
	m_clk = 0;
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
	return clocks * SubCycles();
}

UINT64 netlist_mame_device::execute_cycles_to_clocks(UINT64 cycles) const
{
	return cycles / SubCycles();
}

ATTR_HOT void netlist_mame_device::execute_run()
{
	//bool check_debugger = ((device_t::machine().debug_flags & DEBUG_FLAG_ENABLED) != 0);
	osd_ticks_t a =  -osd_ticks();
	UINT8 ssdiv2 = (SubCycles() / 2);
	//int p = m_icount;
	do
	{
		// debugging
		//m_ppc = m_pc;	// copy PC to previous PC
		//if (check_debugger)
		//	debugger_instruction_hook(this, 0); //m_pc);
		if (--m_ss == 0)
		{
			m_ss = ssdiv2;
			m_clk = !m_clk;
			m_clock_input->setTo(m_clk);
		}
		m_netlist->process_list();
		m_icount--;
	} while (m_icount > 0);
	a+=osd_ticks();
	//printf("%ld %d %ld\n", (long) a, p, (long) (a * 1000 / p));
}



