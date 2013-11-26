/*
 * nlbase.c
 *
 */

#include "nl_base.h"
#include "devices/nld_system.h"
#include "pstring.h"

const netlist_time netlist_time::zero = netlist_time::from_raw(0);

// ----------------------------------------------------------------------------------------
// netlist_object_t
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_object_t::netlist_object_t(const type_t atype, const family_t afamily)
: m_objtype(atype)
, m_family(afamily)
, m_netlist(NULL)
{}

ATTR_COLD netlist_object_t::~netlist_object_t()
{
    //delete m_name;
}

ATTR_COLD void netlist_object_t::init_object(netlist_base_t &nl, const pstring &aname)
{
    m_netlist = &nl;
    m_name = aname;
}

ATTR_COLD const pstring &netlist_object_t::name() const
{
    if (m_name == "")
        fatalerror("object not initialized");
    return m_name;
}

// ----------------------------------------------------------------------------------------
// netlist_owned_object_t
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_owned_object_t::netlist_owned_object_t(const type_t atype,
        const family_t afamily)
: netlist_object_t(atype, afamily)
, m_netdev(NULL)
{
}

ATTR_COLD void netlist_owned_object_t::init_object(netlist_core_device_t &dev,
        const pstring &aname)
{
    netlist_object_t::init_object(dev.netlist(), aname);
    m_netdev = &dev;
}

// ----------------------------------------------------------------------------------------
// netlist_base_t
// ----------------------------------------------------------------------------------------

netlist_base_t::netlist_base_t()
	: 	m_time_ps(netlist_time::zero),
		m_rem(0),
		m_div(NETLIST_DIV),
		m_mainclock(NULL),
		m_solver(NULL)
{
}

netlist_base_t::~netlist_base_t()
{
    pstring::resetmem();
}

ATTR_COLD void netlist_base_t::set_mainclock_dev(NETLIB_NAME(mainclock) *dev)
{
	m_mainclock = dev;
}

ATTR_COLD void netlist_base_t::set_solver_dev(NETLIB_NAME(solver) *dev)
{
    m_solver = dev;
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
			const queue_t::entry_t &e = m_queue.pop();
			update_time(e.time(), atime);

			//if (FATAL_ERROR_AFTER_NS)
			//	NL_VERBOSE_OUT(("%s\n", e.object().netdev()->name().cstr());

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
				const queue_t::entry_t &e = m_queue.pop();

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

ATTR_COLD void netlist_core_device_t::init(netlist_setup_t &setup, const pstring &name)
{
    init_object(setup.netlist(), name);

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
	if (inp.state() == netlist_input_t::STATE_INP_PASSIVE)
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
	//NL_VERBOSE_OUT(("~net_device_t\n");
}

ATTR_COLD void netlist_device_t::init(netlist_setup_t &setup, const pstring &name)
{
	netlist_core_device_t::init(setup, name);
	m_setup = &setup;
	start();
}


ATTR_COLD void netlist_device_t::register_sub(netlist_core_device_t &dev, const pstring &name)
{
	dev.init(*m_setup, this->name() + "." + name);
}

ATTR_COLD void netlist_device_t::register_output(netlist_core_device_t &dev, const pstring &name, netlist_output_t &port)
{
	m_setup->register_object(*this, dev, name, port, netlist_terminal_t::STATE_OUT);
}

ATTR_COLD void netlist_device_t::register_terminal(const pstring &name, netlist_terminal_t &port)
{
    m_setup->register_object(*this,*this,name, port, netlist_terminal_t::STATE_INP_ACTIVE);
}

ATTR_COLD void netlist_device_t::register_output(const pstring &name, netlist_output_t &port)
{
	m_setup->register_object(*this,*this,name, port, netlist_terminal_t::STATE_OUT);
}

ATTR_COLD void netlist_device_t::register_input(netlist_core_device_t &dev, const pstring &name, netlist_input_t &inp, netlist_input_t::state_e type)
{
    m_terminals.add(name);
	m_setup->register_object(*this, dev, name, inp, type);
}

ATTR_COLD void netlist_device_t::register_input(const pstring &name, netlist_input_t &inp, netlist_input_t::state_e type)
{
	register_input(*this, name, inp, type);
}

static void init_term(netlist_core_device_t &dev, netlist_terminal_t &term, netlist_input_t::state_e aState)
{
    if (!term.isInitalized())
    {
        switch (term.type())
        {
            case netlist_terminal_t::OUTPUT:
                dynamic_cast<netlist_output_t &>(term).init_object(dev, "internal output");
                break;
            case netlist_terminal_t::INPUT:
                dynamic_cast<netlist_input_t &>(term).init_object(dev, "internal input", aState);
                break;
            case netlist_terminal_t::TERMINAL:
                dynamic_cast<netlist_terminal_t &>(term).init_object(dev, "internal terminal", aState);
                break;
            default:
                fatalerror("Unknown terminal type");
        }
    }
}

// FIXME: Revise internal links ...
ATTR_COLD void netlist_device_t::register_link_internal(netlist_core_device_t &dev, netlist_input_t &in, netlist_output_t &out, netlist_input_t::state_e aState)
{
    init_term(dev, in, aState);
    init_term(dev, out, aState);
    m_setup->connect(in, out);
}

ATTR_COLD void netlist_device_t::register_link_internal(netlist_input_t &in, netlist_output_t &out, netlist_input_t::state_e aState)
{
	register_link_internal(*this, in, out, aState);
}

template <class C, class T>
ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, C &param, const T initialVal)
{
    param.init_object(dev, sname);
    param.initial(initialVal);
    m_setup->register_object(*this, *this, sname, param, netlist_terminal_t::STATE_NONEX);
}

template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_double_t &param, const double initialVal);
template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_int_t &param, const int initialVal);
template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_logic_t &param, const int initialVal);
template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_str_t &param, const char * initialVal);
template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_str_t &param, const pstring &initialVal);
template ATTR_COLD void netlist_device_t::register_param(netlist_core_device_t &dev, const pstring &sname, netlist_param_multi_t &param, const char * initialVal);


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

ATTR_COLD void netlist_net_t::register_railterminal(netlist_terminal_t &mr)
{
    assert(m_railterminal == NULL);
    m_railterminal = &mr;
}

ATTR_COLD void netlist_net_t::merge_net(netlist_net_t *othernet)
{
    NL_VERBOSE_OUT(("merging nets ...\n"));
    if (othernet == NULL)
        return; // Nothing to do

    if (this->isRailNet() && othernet->isRailNet())
        fatalerror("Trying to merge to rail nets\n");

    if (othernet->isRailNet())
    {
        NL_VERBOSE_OUT(("othernet is railnet\n"));
        othernet->merge_net(this);
    }
    else
    {
        netlist_terminal_t *p = othernet->m_head;
        while (p != NULL)
        {
            netlist_terminal_t *pn = p->m_update_list_next;
            register_con(*p);
            p = pn;
        }

        othernet->m_head = NULL; // FIXME: othernet needs to be free'd from memory
    }
}

ATTR_COLD void netlist_net_t::register_con(netlist_terminal_t &terminal)
{
    terminal.set_net(*this);

    terminal.m_update_list_next = m_head;
    m_head = &terminal;
    m_num_cons++;

    if (terminal.state() != netlist_input_t::STATE_INP_PASSIVE)
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

	assert(this->isRailNet());
	{

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
}

// ----------------------------------------------------------------------------------------
// netlist_terminal_t
// ----------------------------------------------------------------------------------------

ATTR_COLD netlist_terminal_t::netlist_terminal_t(const type_t atype, const family_t afamily)
: netlist_owned_object_t(atype, afamily)
, m_Idr(0.0)
, m_g(NETLIST_GMIN)
, m_update_list_next(NULL)
, m_net(NULL)
, m_state(STATE_NONEX)
{

}

ATTR_COLD netlist_terminal_t::netlist_terminal_t()
: netlist_owned_object_t(TERMINAL, ANALOG)
, m_Idr(0.0)
, m_g(NETLIST_GMIN)
, m_update_list_next(NULL)
, m_net(NULL)
, m_state(STATE_NONEX)
{

}

ATTR_COLD void netlist_terminal_t::init_object(netlist_core_device_t &dev, const pstring &aname, const state_e astate)
{
	set_state(astate);
	netlist_owned_object_t::init_object(dev, aname);
}

ATTR_COLD void netlist_terminal_t::set_net(netlist_net_t &anet)
{
    m_net = &anet;
}

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

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

ATTR_COLD void netlist_output_t::init_object(netlist_core_device_t &dev, const pstring &aname)
{
    netlist_terminal_t::init_object(dev, aname, STATE_OUT);
    net().init_object(dev.netlist(), aname);
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
// netlist_param_t & friends
// ----------------------------------------------------------------------------------------

ATTR_COLD double netlist_param_multi_t::dValue(const pstring &entity, const double defval) const
{
    pstring tmp = this->Value();
    // .model 1N914 D(Is=2.52n Rs=.568 N=1.752 Cjo=4p M=.4 tt=20n Iave=200m Vpk=75 mfg=OnSemi type=silicon)
    int p = tmp.find(entity);
    if (p>=0)
    {
        int pblank = tmp.find(" ", p);
        if (pblank < 0) pblank = tmp.len() + 1;
        tmp = tmp.substr(p, pblank - p);
        int pequal = tmp.find("=", 0);
        if (pequal < 0)
           fatalerror("parameter %s misformat in model %s temp %s\n", entity.cstr(), Value().cstr(), tmp.cstr());
        tmp = tmp.substr(pequal+1);
        double factor = 1.0;
        switch (*(tmp.right(1).cstr()))
        {
            case 'm': factor = 1e-3; break;
            case 'u': factor = 1e-6; break;
            case 'n': factor = 1e-9; break;
            case 'p': factor = 1e-12; break;
            case 'f': factor = 1e-15; break;
            case 'a': factor = 1e-18; break;

        }
        if (factor != 1.0)
            tmp = tmp.left(tmp.len() - 1);
        return atof(tmp.cstr()) * factor;
    }
    else
        return defval;
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
