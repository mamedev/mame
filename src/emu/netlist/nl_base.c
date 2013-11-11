/*
 * nlbase.c
 *
 */

#include "nl_base.h"
#include "devices/nld_system.h"


// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

netlist_base_t::netlist_base_t()
	: m_mainclock(NULL),
		m_time_ps(netlist_time::zero),
		m_rem(0),
		m_div(NETLIST_DIV)
{
}

netlist_base_t::~netlist_base_t()
{
}

ATTR_COLD void netlist_base_t::set_mainclock_dev(NETLIB_NAME(mainclock) *dev)
{
	m_mainclock = dev;
}

ATTR_COLD void netlist_base_t::reset()
{
		m_time_ps = netlist_time::zero;
		m_rem = 0;
		m_queue.clear();
		if (m_mainclock != NULL)
			m_mainclock->m_Q.net().set_time(netlist_time::zero);
}


void netlist_base_t::set_clock_freq(UINT64 clockfreq)
{
	m_div = netlist_time::from_hz(clockfreq).as_raw();
	m_rem = 0;
	assert_always(m_div == NETLIST_DIV, "netlist: illegal clock!");
	NL_VERBOSE_OUT(("Setting clock %" I64FMT "d and divisor %d\n", clockfreq, m_div));
}

ATTR_HOT ATTR_ALIGN inline void netlist_base_t::update_time(const netlist_time t, INT32 &atime)
{
	if (NETLIST_DIV_BITS == 0)
	{
		const netlist_time delta = t - m_time_ps;
		m_time_ps = t;
		atime -= delta.as_raw();
	} else {
		const netlist_time delta = t - m_time_ps + netlist_time::from_raw(m_rem);
		m_time_ps = t;
		m_rem = delta.as_raw() & NETLIST_MASK;
		atime -= (delta.as_raw() >> NETLIST_DIV_BITS);

		// The folling is suitable for non-power of 2 m_divs ...
		// atime -= divu_64x32_rem(delta.as_raw(), m_div, &m_rem);
	}
}

ATTR_HOT ATTR_ALIGN void netlist_base_t::process_queue(INT32 &atime)
{
	if (m_mainclock == NULL)
	{
		while ( (atime > 0) && (m_queue.is_not_empty()))
		{
			const queue_t::entry_t e = m_queue.pop();
			update_time(e.time(), atime);

			//if (FATAL_ERROR_AFTER_NS)
			//	printf("%s\n", e.object().netdev()->name().cstr());

			e.object().update_devs();

			add_to_stat(m_perf_out_processed, 1);

			if (FATAL_ERROR_AFTER_NS)
				if (time() > NLTIME_FROM_NS(FATAL_ERROR_AFTER_NS))
					fatalerror("Stopped");
		}

		if (atime > 0)
		{
			m_time_ps += netlist_time::from_raw(atime * m_div);
			atime = 0;
		}
	} else {
		netlist_net_t &mcQ = m_mainclock->m_Q.net();
		const netlist_time inc = m_mainclock->m_inc;

		while (atime > 0)
		{
			if (m_queue.is_not_empty())
			{
				while (m_queue.peek().time() > mcQ.time())
				{
					update_time(mcQ.time(), atime);

					NETLIB_NAME(mainclock)::mc_update(mcQ, time() + inc);

				}
				const queue_t::entry_t e = m_queue.pop();

				update_time(e.time(), atime);

				e.object().update_devs();

			} else {
				update_time(mcQ.time(), atime);

				NETLIB_NAME(mainclock)::mc_update(mcQ, time() + inc);
			}
			if (FATAL_ERROR_AFTER_NS)
				if (time() > NLTIME_FROM_NS(FATAL_ERROR_AFTER_NS))
					fatalerror("Stopped");

			add_to_stat(m_perf_out_processed, 1);
		}

		if (atime > 0)
		{
			m_time_ps += netlist_time::from_raw(atime * m_div);
			atime = 0;
		}
	}
}

// ----------------------------------------------------------------------------------------
// Default netlist elements ...
// ----------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------
// net_core_device_t
// ----------------------------------------------------------------------------------------

netlist_core_device_t::netlist_core_device_t()
: netlist_object_t(DEVICE, ALL)
{
}

ATTR_COLD void netlist_core_device_t::init(netlist_setup_t &setup, const astring &name)
{
    init_object(setup.netlist());
	m_name = name;

#if USE_DELEGATES
#if USE_PMFDELEGATES
	void (netlist_core_device_t::* pFunc)() = &netlist_core_device_t::update;
	static_update = reinterpret_cast<net_update_delegate>((this->*pFunc));
#else
	static_update = net_update_delegate(&netlist_core_device_t::update, "update", this);
	// get the pointer to the member function
#endif
#endif

}

netlist_core_device_t::~netlist_core_device_t()
{
}

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

ATTR_HOT ATTR_ALIGN const netlist_sig_t netlist_core_device_t::INPLOGIC_PASSIVE(netlist_logic_input_t &inp)
{
	if (inp.state() == netlist_input_t::INP_STATE_PASSIVE)
	{
		inp.activate();
		const netlist_sig_t ret = inp.Q();
		inp.inactivate();
		return ret;
	}
	else
		return inp.Q();

}

netlist_device_t::netlist_device_t()
	: netlist_core_device_t(),
		m_terminals(20),
		m_setup(NULL),
		m_variable_input_count(false)
{
}

netlist_device_t::~netlist_device_t()
{
	//printf("~net_device_t\n");
}

ATTR_COLD void netlist_device_t::init(netlist_setup_t &setup, const astring &name)
{
	netlist_core_device_t::init(setup, name);
	m_setup = &setup;
	start();
}


ATTR_COLD void netlist_device_t::register_sub(netlist_core_device_t &dev, const astring &name)
{
	dev.init(*m_setup, name);
}

void netlist_device_t::register_output(netlist_core_device_t &dev, const astring &name, netlist_output_t &port)
{
	m_setup->register_output(*this, dev, name, port);
}

void netlist_device_t::register_output(const astring &name, netlist_output_t &port)
{
	m_setup->register_output(*this,*this,name, port);
}

void netlist_device_t::register_input(netlist_core_device_t &dev, const astring &name, netlist_input_t &inp, netlist_input_t::net_input_state type)
{
	m_setup->register_input(*this, dev, name, inp, type);
}

void netlist_device_t::register_input(const astring &name, netlist_input_t &inp, netlist_input_t::net_input_state type)
{
	register_input(*this, name, inp, type);
}

void netlist_device_t::register_link_internal(netlist_core_device_t &dev, netlist_input_t &in, netlist_output_t &out, netlist_input_t::net_input_state aState)
{
    in.init_input(dev, aState);
    out.init_terminal(dev);
	//if (in.state() != net_input_t::INP_STATE_PASSIVE)
		out.net().register_con(in);
}

void netlist_device_t::register_link_internal(netlist_input_t &in, netlist_output_t &out, netlist_input_t::net_input_state aState)
{
	register_link_internal(*this, in, out, aState);
}

void netlist_device_t::register_param(netlist_core_device_t &dev, const astring &name, netlist_param_t &param, double initialVal)
{
	param.set_netdev(dev);
	param.initial(initialVal);
	m_setup->register_param(name, &param);
}

void netlist_device_t::register_param(const astring &name, netlist_param_t &param, double initialVal)
{
	register_param(*this,name, param, initialVal);
}

// ----------------------------------------------------------------------------------------
// net_net_t
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_net_t::netlist_net_t(const type_t atype, const family_t afamily)
    : netlist_object_t(atype, afamily)
    ,  m_head(NULL)
    , m_num_cons(0)
    , m_time(netlist_time::zero)
    , m_active(0)
    , m_in_queue(2)
    , m_railterminal(NULL)
{
    m_cur.Q = 0;
    m_new.Q = 0;
    m_last.Q = 0;
};

ATTR_COLD void netlist_net_t::register_con(netlist_terminal_t &terminal)
{
    terminal.set_net(*this);
    if (m_head == NULL)
        m_head = &terminal;
    else
    {
        terminal.m_update_list_next = m_head;
        m_head = &terminal;
    }
    m_num_cons++;

    if (terminal.state() != netlist_input_t::INP_STATE_PASSIVE)
        m_active++;
}

ATTR_HOT inline void netlist_net_t::update_dev(const netlist_terminal_t *inp, const UINT32 mask)
{
	if ((inp->state() & mask) != 0)
	{
		netlist_core_device_t &netdev = inp->netdev();
		begin_timing(netdev.total_time);
		inc_stat(netdev.stat_count);
		netdev.update_dev();
		end_timing(netdev().total_time);
	}
}

ATTR_HOT inline void netlist_net_t::update_devs()
{
	assert(m_num_cons != 0);

	const UINT32 masks[4] = { 1, 5, 3, 1 };
	m_cur = m_new;
	m_in_queue = 2; /* mark as taken ... */

	const UINT32 mask = masks[ (m_last.Q  << 1) | m_cur.Q ];

    netlist_terminal_t *p = m_head;
    switch (m_num_cons)
    {
    case 2:
        update_dev(p, mask);
        p = p->m_update_list_next;
    case 1:
        update_dev(p, mask);
        break;
    default:
        do
        {
            update_dev(p, mask);
            p = p->m_update_list_next;
        } while (p != NULL);
        break;
    }
	m_last = m_cur;
}

// ----------------------------------------------------------------------------------------
// netlist_terminal_t
// ----------------------------------------------------------------------------------------

ATTR_COLD void netlist_terminal_t::init_terminal(netlist_core_device_t &dev)
{
	m_netdev = &dev;
	init_object(dev.netlist());
}

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

ATTR_COLD void netlist_input_t::init_input(netlist_core_device_t &dev, net_input_state astate)
{
	init_terminal(dev);
	set_state(astate);
}

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

netlist_output_t::netlist_output_t(const type_t atype, const family_t afamily)
	: netlist_terminal_t(atype, afamily)
	, m_low_V(0.0)
	, m_high_V(0.0)
    , m_my_net(NET, afamily)
{
    //m_net = new net_net_t(NET_DIGITAL);
    this->set_net(m_my_net);
}

ATTR_COLD void netlist_output_t::init_terminal(netlist_core_device_t &dev)
{
    netlist_terminal_t::init_terminal(dev);
    net().init_object(dev.netlist());
    net().register_railterminal(*this);
}

ATTR_COLD void netlist_logic_output_t::initial(const netlist_sig_t val)
{
    net().m_cur.Q = val;
    net().m_new.Q = val;
    net().m_last.Q = !val;
}

ATTR_COLD netlist_logic_output_t::netlist_logic_output_t()
    : netlist_output_t(OUTPUT, LOGIC)
{
    // Default to TTL
    m_low_V = 0.1;  // these depend on sinked/sourced current. Values should be suitable for typical applications.
    m_high_V = 4.8;
}

ATTR_COLD void netlist_logic_output_t::set_levels(const double low, const double high)
{
    m_low_V = low;
    m_high_V = high;
}


// ----------------------------------------------------------------------------------------
// mainclock
// ----------------------------------------------------------------------------------------

ATTR_HOT inline void NETLIB_NAME(mainclock)::mc_update(netlist_net_t &net, const netlist_time curtime)
{
	net.m_new.Q = !net.m_new.Q;
	net.set_time(curtime);
	net.update_devs();
}

ATTR_COLD NETLIB_START(mainclock)
{
	register_output("Q", m_Q);

	register_param("FREQ", m_freq, 7159000.0 * 5);
	m_inc = netlist_time::from_hz(m_freq.Value()*2);

}

ATTR_HOT NETLIB_UPDATE_PARAM(mainclock)
{
	m_inc = netlist_time::from_hz(m_freq.Value()*2);
}

ATTR_HOT NETLIB_UPDATE(mainclock)
{
    netlist_net_t &net = m_Q.net();
	// this is only called during setup ...
	net.m_new.Q = !net.m_new.Q;
	net.set_time(netlist().time() + m_inc);
}
