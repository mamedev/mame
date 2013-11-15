// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.c
 *
 */

#include "nl_base.h"
#include "nl_setup.h"
#include "nl_parser.h"
#include "devices/nld_system.h"

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
    , m_proxy_cnt(0)
{
	NETLIST_NAME(base)(*this);
}

template <class T>
static void tagmap_free_entries(T &tm)
{
	for (typename T::entry_t *entry = tm.first(); entry != NULL; entry = tm.next(entry))
	{
		delete entry->object();
	}
	tm.reset();
}

netlist_setup_t::~netlist_setup_t()
{
	tagmap_free_entries<tagmap_devices_t>(m_devices);
	tagmap_free_entries<tagmap_astring_t>(m_links);
	tagmap_free_entries<tagmap_astring_t>(m_alias);
	m_params.reset();
	m_terminals.reset();
}

netlist_device_t *netlist_setup_t::register_dev(netlist_device_t *dev)
{
	if (!(m_devices.add(dev->name(), dev, false)==TMERR_NONE))
		fatalerror("Error adding %s to device list\n", dev->name().cstr());
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
			NL_VERBOSE_OUT(("removing %s\n", entry->tag().cstr()));
			hm.remove(entry->object());
		}
		entry = next;
	}
}

void netlist_setup_t::remove_dev(const astring &name)
{
	netlist_device_t *dev = m_devices.find(name);
	astring temp = name;
	if (dev == NULL)
		fatalerror("Device %s does not exist\n", name.cstr());

	temp.cat(".");

	//remove_start_with<tagmap_input_t>(m_inputs, temp);
	remove_start_with<tagmap_terminal_t>(m_terminals, temp);
	remove_start_with<tagmap_param_t>(m_params, temp);
	remove_start_with<tagmap_astring_t>(m_links, temp);
	m_devices.remove(name);
}

void netlist_setup_t::register_callback(const astring &devname, netlist_output_delegate delegate)
{
	NETLIB_NAME(analog_callback) *dev = (NETLIB_NAME(analog_callback) *) m_devices.find(devname);
	if (dev == NULL)
		fatalerror("did not find device %s\n", devname.cstr());
	dev->register_callback(delegate);
}

void netlist_setup_t::register_alias(const astring &alias, const astring &out)
{
	if (!(m_alias.add(alias, new astring(out), false)==TMERR_NONE))
		fatalerror("Error adding alias %s to alias list\n", alias.cstr());
}

void netlist_setup_t::register_output(netlist_core_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, netlist_output_t &out)
{
	NL_VERBOSE_OUT(("output %s\n", name.cstr()));
	astring temp = dev.name();
	temp.cat(".");
	temp.cat(name);
	out.init_terminal(upd_dev);
	if (!(m_terminals.add(temp, &out, false)==TMERR_NONE))
		fatalerror("Error adding output %s to terminal list\n", name.cstr());
}

void netlist_setup_t::register_terminal(netlist_core_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, netlist_terminal_t &out)
{
    NL_VERBOSE_OUT(("output %s\n", name.cstr()));
    assert(out.isType(netlist_terminal_t::TERMINAL));
    astring temp = dev.name();
    temp.cat(".");
    temp.cat(name);
    out.init_terminal(upd_dev);
    if (!(m_terminals.add(temp, &out, false)==TMERR_NONE))
        fatalerror("Error adding output %s to terminal list\n", name.cstr());
}

void netlist_setup_t::register_input(netlist_device_t &dev, netlist_core_device_t &upd_dev, const astring &name, netlist_input_t &inp, netlist_input_t::net_input_state type)
{
	NL_VERBOSE_OUT(("input %s\n", name.cstr()));
	astring temp = dev.name();
	temp.cat(".");
	temp.cat(name);
	inp.init_input(upd_dev, type);
	/* add to list of terminals so logic devices are supported in parser */
	dev.m_terminals.add(temp);
	if (!(m_terminals.add(temp, &inp, false) == TMERR_NONE))
		fatalerror("Error adding input %s to terminal list\n", name.cstr());
}

void netlist_setup_t::register_link(const astring &sin, const astring &sout)
{
	const astring *temp = new astring(sout);
	NL_VERBOSE_OUT(("link %s <== %s\n", sin.cstr(), sout.cstr()));
	if (!(m_links.add(sin, temp, false)==TMERR_NONE))
		fatalerror("Error adding link %s<==%s to link list\n", sin.cstr(), sout.cstr());
}


void netlist_setup_t::register_param(const astring &name, netlist_param_t *param)
{
	astring temp = param->netdev().name();
	temp.cat(".");
	temp.cat(name);
	if (!(m_params.add(temp, param, false)==TMERR_NONE))
		fatalerror("Error adding parameter %s to parameter list\n", name.cstr());
}


const astring &netlist_setup_t::resolve_alias(const astring &name) const
{
	const astring *ret = m_alias.find(name);
	if (ret != NULL)
		return *ret;
	return name;
}

netlist_output_t *netlist_setup_t::find_output_exact(const astring &outname_in)
{
	netlist_terminal_t *term = m_terminals.find(outname_in);
	return dynamic_cast<netlist_output_t *>(term);
}

netlist_output_t &netlist_setup_t::find_output(const astring &outname_in)
{
	const astring &outname = resolve_alias(outname_in);
	netlist_output_t *ret;

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
		fatalerror("output %s(%s) not found!\n", outname_in.cstr(), outname.cstr());
	NL_VERBOSE_OUT(("Found input %s\n", outname.cstr()));
	return *ret;
}

netlist_terminal_t &netlist_setup_t::find_terminal(const astring &terminal_in)
{
	const astring &tname = resolve_alias(terminal_in);
	netlist_terminal_t *ret;

	ret = dynamic_cast<netlist_terminal_t *>(m_terminals.find(tname));
	/* look for default */
	if (ret == NULL)
	{
		/* look for ".Q" std output */
		astring s = tname;
		s.cat(".Q");
		ret = dynamic_cast<netlist_terminal_t *>(m_terminals.find(s));
	}
	if (ret == NULL)
		fatalerror("terminal %s(%s) not found!\n", terminal_in.cstr(), tname.cstr());
	NL_VERBOSE_OUT(("Found input %s\n", tname.cstr()));
	return *ret;
}

netlist_param_t &netlist_setup_t::find_param(const astring &param_in)
{
	const astring &outname = resolve_alias(param_in);
	netlist_param_t *ret;

	ret = m_params.find(outname);
	if (ret == NULL)
		fatalerror("parameter %s(%s) not found!\n", param_in.cstr(), outname.cstr());
	NL_VERBOSE_OUT(("Found parameter %s\n", outname.cstr()));
	return *ret;
}


void netlist_setup_t::connect_input_output(netlist_input_t &in, netlist_output_t &out)
{
    if (out.isFamily(netlist_terminal_t::ANALOG) && in.isFamily(netlist_terminal_t::LOGIC))
    {
        nld_a_to_d_proxy *proxy = new nld_a_to_d_proxy(in);
        astring x = "";
        x.printf("proxy_ad_%d", m_proxy_cnt++);

        proxy->init(*this, x.cstr());
        register_dev(proxy);

        proxy->m_Q.net().register_con(in);
        out.net().register_con(proxy->m_I);

    }
    else if (out.isFamily(netlist_terminal_t::LOGIC) && in.isFamily(netlist_terminal_t::ANALOG))
    {
        //printf("here 1\n");
        nld_d_to_a_proxy *proxy = new nld_d_to_a_proxy(out);
        astring x = "";
        x.printf("proxy_da_%d", m_proxy_cnt++);
        proxy->init(*this, x.cstr());
        register_dev(proxy);

        proxy->m_Q.net().register_con(in);
        out.net().register_con(proxy->m_I);
    }
    else
    {
        out.net().register_con(in);
    }
}

// FIXME: optimize code  ...
void netlist_setup_t::connect_terminal_output(netlist_terminal_t &in, netlist_output_t &out)
{
    if (out.isFamily(netlist_terminal_t::ANALOG))
    {
        /* no proxy needed, just merge existing terminal net */
        if (in.has_net())
            out.net().merge_net(&in.net());
        else
            out.net().register_con(in);

    }
    else if (out.isFamily(netlist_terminal_t::LOGIC))
    {
        //printf("here 1\n");
        nld_d_to_a_proxy *proxy = new nld_d_to_a_proxy(out);
        astring x = "";
        x.printf("proxy_da_%d", m_proxy_cnt++);
        proxy->init(*this, x.cstr());
        register_dev(proxy);

        out.net().register_con(proxy->m_I);

        if (in.has_net())
            proxy->m_Q.net().merge_net(&in.net());
        else
            proxy->m_Q.net().register_con(in);
    }
    else
    {
        fatalerror("Netlist: Severe Error");
    }
}

void netlist_setup_t::connect_terminals(netlist_terminal_t &in, netlist_terminal_t &out)
{
    assert(in.isType(netlist_terminal_t::TERMINAL));
    assert(out.isType(netlist_terminal_t::TERMINAL));

    if (in.has_net() && out.has_net())
    {
        in.net().merge_net(&out.net());
        //in.net().register_con(out);
        //in.net().register_con(in);
    }
    else if (out.has_net())
    {
        out.net().register_con(in);
        //out.net().register_con(out);
    }
    else if (in.has_net())
    {
        in.net().register_con(out);
        //in.net().register_con(in);
    }
    else
    {
        NL_VERBOSE_OUT(("adding net ...\n"));
        in.set_net(*(new netlist_net_t(netlist_object_t::NET, netlist_object_t::ANALOG)));
        in.net().init_object(netlist());
        in.net().register_con(out);
        in.net().register_con(in);
    }
}

void netlist_setup_t::resolve_inputs(void)
{
    NL_VERBOSE_OUT(("Searching for clocks ...\n"));
    /* find the main clock ... */
    for (tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
    {
        netlist_device_t *dev = entry->object();
        if (dynamic_cast<NETLIB_NAME(mainclock)*>(dev) != NULL)
        {
            m_netlist.set_mainclock_dev(dynamic_cast<NETLIB_NAME(mainclock)*>(dev));
        }
    }

    NL_VERBOSE_OUT(("Resolving ...\n"));
    for (tagmap_astring_t::entry_t *entry = m_links.first(); entry != NULL; entry = m_links.next(entry))
    {
        const astring t1s = *entry->object();
        const astring t2s = entry->tag();
        netlist_terminal_t &t1 = find_terminal(t1s);
        netlist_terminal_t &t2 = find_terminal(t2s);

        // FIXME: amend device design so that warnings can be turned into errors
        //        Only variable inputs have this issue
        if (t1.isType(netlist_terminal_t::OUTPUT) && t2.isType(netlist_terminal_t::INPUT))
        {
            if (t2.has_net())
                mame_printf_warning("Input %s already connected\n", t2s.cstr());
            connect_input_output(dynamic_cast<netlist_input_t &>(t2), dynamic_cast<netlist_output_t &>(t1));
        }
        else if (t1.isType(netlist_terminal_t::INPUT) && t2.isType(netlist_terminal_t::OUTPUT))
        {
            if (t1.has_net())
                mame_printf_warning("Input %s already connected\n", t1s.cstr());
            connect_input_output(dynamic_cast<netlist_input_t &>(t1), dynamic_cast<netlist_output_t &>(t2));
        }
        else if (t1.isType(netlist_terminal_t::OUTPUT) && t2.isType(netlist_terminal_t::TERMINAL))
        {
            connect_terminal_output(dynamic_cast<netlist_terminal_t &>(t2), dynamic_cast<netlist_output_t &>(t1));
        }
        else if (t1.isType(netlist_terminal_t::TERMINAL) && t2.isType(netlist_terminal_t::OUTPUT))
        {
            connect_terminal_output(dynamic_cast<netlist_terminal_t &>(t1), dynamic_cast<netlist_output_t &>(t2));
        }
        else if (t1.isType(netlist_terminal_t::TERMINAL) && t2.isType(netlist_terminal_t::TERMINAL))
        {
            connect_terminals(dynamic_cast<netlist_terminal_t &>(t1), dynamic_cast<netlist_terminal_t &>(t2));
        }
        else
            fatalerror("Connecting %s to %s not supported!\n", t1s.cstr(), t2s.cstr());
    }

    /* print all outputs */
    for (tagmap_terminal_t::entry_t *entry = m_terminals.first(); entry != NULL; entry = m_terminals.next(entry))
    {
        ATTR_UNUSED netlist_output_t *out = dynamic_cast<netlist_output_t *>(entry->object());
        //if (out != NULL)
            //VERBOSE_OUT(("%s %d\n", out->netdev()->name(), *out->Q_ptr()));
    }


}

void netlist_setup_t::step_devices_once(void)
{
	/* make sure params are set now .. */
	for (tagmap_param_t::entry_t *entry = m_params.first(); entry != NULL; entry = m_params.next(entry))
	{
		entry->object()->netdev().update_param();
	}

	for (tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
	{
		netlist_device_t *dev = entry->object();
		dev->update_dev();
	}
}


void netlist_setup_t::parse(char *buf)
{
	netlist_parser parser(*this);
	parser.parse(buf);
}

void netlist_setup_t::print_stats()
{
	if (NL_KEEP_STATISTICS)
	{
		for (netlist_setup_t::tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
		{
			//entry->object()->s
			printf("Device %20s : %12d %15ld\n", entry->object()->name().cstr(), entry->object()->stat_count, (long int) entry->object()->total_time / (entry->object()->stat_count + 1));
		}
		printf("Queue Start %15d\n", m_netlist.m_queue.m_prof_start);
		printf("Queue End   %15d\n", m_netlist.m_queue.m_prof_end);
		printf("Queue Sort  %15d\n", m_netlist.m_queue.m_prof_sort);
		printf("Queue Move  %15d\n", m_netlist.m_queue.m_prof_sortmove);
	}
}
