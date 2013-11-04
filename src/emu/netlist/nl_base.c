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

ATTR_COLD void netlist_base_t::set_mainclock_dev(NETLIB_NAME(netdev_mainclock) *dev)
{
	m_mainclock = dev;
}

ATTR_COLD void netlist_base_t::reset()
{
	  m_time_ps = netlist_time::zero;
	  m_rem = 0;
	  m_queue.clear();
	  if (m_mainclock != NULL)
		  m_mainclock->m_Q.set_time(netlist_time::zero);
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

			if (FATAL_ERROR_AFTER_NS)
				printf("%s\n", e.object().netdev()->name().cstr());

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
		net_output_t &mcQ = m_mainclock->m_Q;
		const netlist_time inc = m_mainclock->m_inc;

		while (atime > 0)
		{
			if (m_queue.is_not_empty())
			{
				while (m_queue.peek().time() > mcQ.time())
				{
					update_time(mcQ.time(), atime);

					NETLIB_NAME(netdev_mainclock)::mc_update(mcQ, time() + inc);

				}
				const queue_t::entry_t e = m_queue.pop();

				update_time(e.time(), atime);

				e.object().update_devs();

			} else {
				update_time(mcQ.time(), atime);

				NETLIB_NAME(netdev_mainclock)::mc_update(mcQ, time() + inc);
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
: net_object_t(DEVICE)
{
}

ATTR_COLD void netlist_core_device_t::init(netlist_setup_t &setup, const astring &name)
{
	m_netlist = &setup.netlist();
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

ATTR_COLD void net_device_t::init(netlist_setup_t &setup, const astring &name)
{
	netlist_core_device_t::init(setup, name);
	m_setup = &setup;
	start();
}


netlist_core_device_t::~netlist_core_device_t()
{
}

// ----------------------------------------------------------------------------------------
// net_device_t
// ----------------------------------------------------------------------------------------

ATTR_HOT ATTR_ALIGN const netlist_sig_t netlist_core_device_t::INPLOGIC_PASSIVE(logic_input_t &inp)
{
	if (inp.state() == net_input_t::INP_STATE_PASSIVE)
	{
		inp.activate();
		const netlist_sig_t ret = inp.Q();
		inp.inactivate();
		return ret;
	}
	else
		return inp.Q();

}

net_device_t::net_device_t()
	: netlist_core_device_t(),
	  m_inputs(20),
	  m_setup(NULL),
	  m_variable_input_count(false)
{
}

net_device_t::~net_device_t()
{
	//printf("~net_device_t\n");
}

ATTR_COLD void net_device_t::register_sub(netlist_core_device_t &dev, const astring &name)
{
	dev.init(*m_setup, name);
}

void net_device_t::register_output(netlist_core_device_t &dev, const astring &name, net_output_t &port)
{
	m_setup->register_output(*this, dev, name, port);
}

void net_device_t::register_output(const astring &name, net_output_t &port)
{
	m_setup->register_output(*this,*this,name, port);
}

void net_device_t::register_input(netlist_core_device_t &dev, const astring &name, net_input_t &inp, net_input_t::net_input_state type)
{
	m_setup->register_input(*this, dev, name, inp, type);
}

void net_device_t::register_input(const astring &name, net_input_t &inp, net_input_t::net_input_state type)
{
	register_input(*this, name, inp, type);
}

void net_device_t::register_link_internal(netlist_core_device_t &dev, net_input_t &in, net_output_t &out, net_input_t::net_input_state aState)
{
	in.set_output(out);
	in.init_input(&dev, aState);
	//if (in.state() != net_input_t::INP_STATE_PASSIVE)
		out.register_con(in);
}

void net_device_t::register_link_internal(net_input_t &in, net_output_t &out, net_input_t::net_input_state aState)
{
	register_link_internal(*this, in, out, aState);
}

void net_device_t::register_param(netlist_core_device_t &dev, const astring &name, net_param_t &param, double initialVal)
{
	param.set_netdev(dev);
	param.initial(initialVal);
	m_setup->register_param(name, &param);
}

void net_device_t::register_param(const astring &name, net_param_t &param, double initialVal)
{
	register_param(*this,name, param, initialVal);
}

// ----------------------------------------------------------------------------------------
// net_terminal_t
// ----------------------------------------------------------------------------------------

ATTR_COLD void net_terminal_t::init_terminal(netlist_core_device_t *dev)
{
	m_netdev = dev;
	m_netlist = dev->netlist();
}

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

ATTR_COLD void net_input_t::init_input(netlist_core_device_t *dev, net_input_state astate)
{
	init_terminal(dev);
	m_state = astate;
}

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

net_output_t::net_output_t(int atype)
	: net_terminal_t(atype)
	, m_low_V(0.0)
	, m_high_V(0.0)
	, m_last_Q(0)
	, m_Q(0)
	, m_new_Q(0)
	, m_Q_analog(0.0)
	, m_new_Q_analog(0.0)
	, m_num_cons(0)
	, m_time(netlist_time::zero)
	, m_active(0)
	, m_in_queue(2)
{
	//m_cons = global_alloc_array(net_input_t *, OUTPUT_MAX_CONNECTIONS);
}

ATTR_HOT inline void net_output_t::update_dev(const net_input_t *inp, const UINT32 mask)
{
	if ((inp->state() & mask) != 0)
	{
		ATTR_UNUSED netlist_core_device_t *netdev = inp->netdev();
		begin_timing(netdev->total_time);
		inc_stat(netdev->stat_count);
		netdev->update_dev();
		end_timing(netdev()->total_time);
	}
}

ATTR_HOT inline void net_output_t::update_devs()
{

	assert(m_num_cons != 0);

	const UINT32 masks[4] = { 1, 5, 3, 1 };
	m_Q = m_new_Q;
	m_Q_analog = m_new_Q_analog;
	m_in_queue = 2; /* mark as taken ... */

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
		break;
	}

	m_last_Q = m_Q;
}

ATTR_COLD void net_output_t::register_con(net_input_t &input)
{
	int i;
	if (m_num_cons >= OUTPUT_MAX_CONNECTIONS)
		fatalerror("Connections exceeded for %s\n", netdev()->name().cstr());

	/* keep similar devices together */
	for (i = 0; i < m_num_cons; i++)
		if (m_cons[i]->netdev() == input.netdev())
			break;

	for (int j = m_num_cons; j > i; j--)
		m_cons[j] = m_cons[j - 1];

	m_cons[i] = &input;
	m_num_cons++;
	if (input.state() != net_input_t::INP_STATE_PASSIVE)
		m_active++;
}

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
	OUTLOGIC(m_Q, m_const.ValueInt(), NLTIME_IMMEDIATE);
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

NETLIB_UPDATE(netdev_analog_callback)
{
	// FIXME: Remove after device cleanup
	if (!m_callback.isnull())
		m_callback(INPANALOG(m_in));
}

// license:GPL-2.0+
// copyright-holders:Couriersud
// ----------------------------------------------------------------------------------------
// netdev_mainclock
// ----------------------------------------------------------------------------------------

ATTR_HOT inline void NETLIB_NAME(netdev_mainclock)::mc_update(net_output_t &Q, const netlist_time curtime)
{
	Q.m_new_Q = !Q.m_new_Q;
	Q.set_time(curtime);
	Q.update_devs();
}

ATTR_COLD NETLIB_START(netdev_mainclock)
{
	register_output("Q", m_Q);

	register_param("FREQ", m_freq, 7159000.0 * 5);
	m_inc = netlist_time::from_hz(m_freq.Value()*2);

}

ATTR_HOT NETLIB_UPDATE_PARAM(netdev_mainclock)
{
	m_inc = netlist_time::from_hz(m_freq.Value()*2);
}

ATTR_HOT NETLIB_UPDATE(netdev_mainclock)
{
	// this is only called during setup ...
	m_Q.m_new_Q = !m_Q.m_new_Q;
	m_Q.set_time(m_netlist->time() + m_inc);
}
