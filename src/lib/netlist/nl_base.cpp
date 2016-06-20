// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlbase.c
 *
 */

#include <cstring>

#include "solver/nld_matrix_solver.h"
#include "solver/nld_solver.h"

#include "plib/putil.h"
#include "plib/palloc.h"

#include "nl_base.h"
#include "devices/nlid_system.h"

namespace netlist
{

#if (NL_USE_MEMPOOL)
static plib::mempool p(65536, 8);

void * object_t::operator new (size_t size)
{
	return p.alloc(size);
}

void object_t::operator delete (void * mem)
{
    if (mem)
    	p.free(mem);
}
#else
void * object_t::operator new (size_t size)
{
	return ::operator new(size);
}

void object_t::operator delete (void * mem)
{
    if (mem)
    	::operator delete(mem);
}
#endif

// ----------------------------------------------------------------------------------------
// logic_family_ttl_t
// ----------------------------------------------------------------------------------------

class logic_family_ttl_t : public logic_family_desc_t
{
public:
	logic_family_ttl_t() : logic_family_desc_t()
	{
		m_low_thresh_V = 0.8;
		m_high_thresh_V = 2.0;
		// m_low_V  - these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_low_V = 0.1;
		m_high_V = 4.0;
		m_R_low = 1.0;
		m_R_high = 130.0;
	}
	virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *proxied) const override
	{
		return plib::owned_ptr<devices::nld_base_d_to_a_proxy>::Create<devices::nld_d_to_a_proxy>(anetlist, name, proxied);
	}
};

class logic_family_cd4xxx_t : public logic_family_desc_t
{
public:
	logic_family_cd4xxx_t() : logic_family_desc_t()
	{
		m_low_thresh_V = 0.8;
		m_high_thresh_V = 2.0;
		// m_low_V  - these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_low_V = 0.05;
		m_high_V = 4.95;
		m_R_low = 10.0;
		m_R_high = 10.0;
	}
	virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *proxied) const override
	{
		return plib::owned_ptr<devices::nld_base_d_to_a_proxy>::Create<devices::nld_d_to_a_proxy>(anetlist, name, proxied);
	}
};

const logic_family_desc_t *family_TTL()
{
	static logic_family_ttl_t obj;
	return &obj;
}
const logic_family_desc_t *family_CD4XXX()
{
	static logic_family_cd4xxx_t obj;
	return &obj;
}

// ----------------------------------------------------------------------------------------
// queue_t
// ----------------------------------------------------------------------------------------

queue_t::queue_t(netlist_t &nl)
	: timed_queue<net_t *, netlist_time>(512)
	, object_t("QUEUE")
	, netlist_ref(nl)
	, plib::state_manager_t::callback_t()
	, m_qsize(0)
	, m_times(512)
	, m_names(512)
{
}

void queue_t::register_state(plib::state_manager_t &manager, const pstring &module)
{
	netlist().log().debug("register_state\n");
	manager.save_item(this, m_qsize, module + "." + "qsize");
	manager.save_item(this, &m_times[0], module + "." + "times", m_times.size());
	manager.save_item(this, &(m_names[0].m_buf[0]), module + "." + "names", m_names.size() * sizeof(names_t));
}

void queue_t::on_pre_save()
{
	netlist().log().debug("on_pre_save\n");
	m_qsize = this->size();
	netlist().log().debug("current time {1} qsize {2}\n", netlist().time().as_double(), m_qsize);
	for (int i = 0; i < m_qsize; i++ )
	{
		m_times[i] =  this->listptr()[i].m_exec_time.as_raw();
		pstring p = this->listptr()[i].m_object->name();
		int n = p.len();
		n = std::min(63, n);
		std::strncpy(m_names[i].m_buf, p.cstr(), n);
		m_names[i].m_buf[n] = 0;
	}
}


void queue_t::on_post_load()
{
	this->clear();
	netlist().log().debug("current time {1} qsize {2}\n", netlist().time().as_double(), m_qsize);
	for (int i = 0; i < m_qsize; i++ )
	{
		net_t *n = netlist().find_net(m_names[i].m_buf);
		//log().debug("Got {1} ==> {2}\n", qtemp[i].m_name, n));
		//log().debug("schedule time {1} ({2})\n", n->time().as_double(),  netlist_time::from_raw(m_times[i]).as_double()));
		this->push(netlist_time::from_raw(m_times[i]), n);
	}
}

// ----------------------------------------------------------------------------------------
// object_t
// ----------------------------------------------------------------------------------------

object_t::object_t(const pstring &aname)
	: m_name(aname)
{
}

object_t::~object_t()
{
}

const pstring &object_t::name() const
{
	return m_name;
}

// ----------------------------------------------------------------------------------------
// device_object_t
// ----------------------------------------------------------------------------------------

device_object_t::device_object_t(core_device_t &dev, const pstring &aname, const type_t atype)
: object_t(aname)
, m_device(dev)
, m_type(atype)
{
}


// ----------------------------------------------------------------------------------------
// netlist_t
// ----------------------------------------------------------------------------------------

netlist_t::netlist_t(const pstring &aname)
	: m_state()
	, m_time(netlist_time::zero())
	, m_queue(*this)
	, m_use_deactivate(0)
	, m_mainclock(nullptr)
	, m_solver(nullptr)
	, m_gnd(nullptr)
	, m_params(nullptr)
	, m_name(aname)
	, m_setup(nullptr)
	, m_log(this)
	, m_lib(nullptr)
{
	state().save_item(this, static_cast<plib::state_manager_t::callback_t &>(m_queue), "m_queue");
	state().save_item(this, m_time, "m_time");
}

netlist_t::~netlist_t()
{

	m_nets.clear();
	m_devices.clear();

	pfree(m_lib);
	pstring::resetmem();
}

nl_double netlist_t::gmin() const
{
	return solver()->gmin();
}

void netlist_t::start()
{
	/* load the library ... */

	pstring libpath = plib::util::environment("NL_BOOSTLIB", plib::util::buildpath({".", "nlboost.so"}));

	m_lib = plib::palloc<plib::dynlib>(libpath);

	/* make sure the solver and parameters are started first! */

	for (auto & e : setup().m_device_factory)
	{
		if ( setup().factory().is_class<devices::NETLIB_NAME(mainclock)>(e.second)
				|| setup().factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				|| setup().factory().is_class<devices::NETLIB_NAME(gnd)>(e.second)
				|| setup().factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			auto dev = plib::owned_ptr<device_t>(e.second->Create(*this, e.first));
			setup().register_dev(std::move(dev));
		}
	}

	log().debug("Searching for mainclock and solver ...\n");

	m_mainclock = get_single_device<devices::NETLIB_NAME(mainclock)>("mainclock");
	m_solver = get_single_device<devices::NETLIB_NAME(solver)>("solver");
	m_gnd = get_single_device<devices::NETLIB_NAME(gnd)>("gnd");
	m_params = get_single_device<devices::NETLIB_NAME(netlistparams)>("parameter");

	m_use_deactivate = (m_params->m_use_deactivate.Value() ? true : false);

	/* create devices */

	for (auto & e : setup().m_device_factory)
	{
		if ( !setup().factory().is_class<devices::NETLIB_NAME(mainclock)>(e.second)
				&& !setup().factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				&& !setup().factory().is_class<devices::NETLIB_NAME(gnd)>(e.second)
				&& !setup().factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			auto dev = plib::owned_ptr<device_t>(e.second->Create(*this, e.first));
			setup().register_dev(std::move(dev));
		}
	}

}

void netlist_t::stop()
{
	/* find the main clock and solver ... */

	log().debug("Stopping all devices ...\n");
	for (auto & dev : m_devices)
		dev->stop_dev();
}

net_t *netlist_t::find_net(const pstring &name)
{
	for (auto & net : m_nets)
		if (net->name() == name)
			return net.get();

	return nullptr;
}

void netlist_t::rebuild_lists()
{
	for (auto & net : m_nets)
		net->rebuild_list();
}


void netlist_t::reset()
{
	m_time = netlist_time::zero();
	m_queue.clear();
	if (m_mainclock != nullptr)
		m_mainclock->m_Q.net().set_time(netlist_time::zero());
	//if (m_solver != nullptr)
	//	m_solver->do_reset();

	// Reset all nets once !
	for (auto & n : m_nets)
		n->reset();

	// Reset all devices once !
	for (auto & dev : m_devices)
		dev->do_reset();

	// Make sure everything depending on parameters is set
	for (auto & dev : m_devices)
		dev->update_param();

	// Step all devices once !
#if 0
	for (std::size_t i = 0; i < m_devices.size(); i++)
	{
		m_devices[i]->update_dev();
	}
#else
	/* FIXME: this makes breakout attract mode working again.
	 * It is however not acceptable that this depends on the startup order.
	 * Best would be, if reset would call update_dev for devices which need it.
	 */
	for (int i = m_devices.size() - 1; i >= 0; i--)
		m_devices[i]->update_dev();
#endif
}


void netlist_t::process_queue(const netlist_time &delta)
{
	netlist_time stop(m_time + delta);

	m_queue.push(stop, nullptr);

	m_stat_mainloop.begin();

	if (m_mainclock == nullptr)
	{
		queue_t::entry_t e(m_queue.pop());
		m_time = e.m_exec_time;
		while (e.m_object != nullptr)
		{
			e.m_object->update_devs();
			m_perf_out_processed.inc();
			e = m_queue.pop();
			m_time = e.m_exec_time;
		}
	}
	else
	{
		logic_net_t &mc_net = m_mainclock->m_Q.net();
		const netlist_time inc = m_mainclock->m_inc;
		netlist_time mc_time(mc_net.time());

		while (1)
		{
			while (m_queue.top().m_exec_time > mc_time)
			{
				m_time = mc_time;
				mc_time += inc;
				mc_net.toggle_new_Q();
				mc_net.update_devs();
			}

			const queue_t::entry_t e(m_queue.pop());
			m_time = e.m_exec_time;
			if (e.m_object == nullptr)
				break;
			e.m_object->update_devs();
			m_perf_out_processed.inc();
		}
		mc_net.set_time(mc_time);
	}
	m_stat_mainloop.end();
}

void netlist_t::print_stats() const
{
	if (nperftime_t::enabled)
	{
		std::vector<size_t> index;
		for (size_t i=0; i<m_devices.size(); i++)
			index.push_back(i);

		std::sort(index.begin(), index.end(),
				[&](size_t i1, size_t i2) { return m_devices[i1]->m_stat_total_time.total() < m_devices[i2]->m_stat_total_time.total(); });

		nperftime_t::type total_time(0);
		uint_least64_t total_count(0);

		for (auto & j : index)
		{
			auto entry = m_devices[j].get();
			log().verbose("Device {1:20} : {2:12} {3:12} {4:15} {5:12}", entry->name(),
					entry->m_stat_call_count(), entry->m_stat_total_time.count(),
					entry->m_stat_total_time.total(), entry->m_stat_inc_active());
			total_time += entry->m_stat_total_time.total();
			total_count += entry->m_stat_total_time.count();
		}

		nperftime_t overhead;
		nperftime_t test;
		overhead.begin();
		for (int j=0; j<100000;j++)
		{
			test.begin();
			test.end();
		}
		overhead.end();

		uint_least64_t total_overhead = (uint_least64_t) overhead()*(uint_least64_t)total_count/(uint_least64_t)200000;

		log().verbose("Queue Pushes   {1:15}", queue().m_prof_call());
		log().verbose("Queue Moves    {1:15}", queue().m_prof_sortmove());

		log().verbose("Total loop     {1:15}", m_stat_mainloop());
		/* Only one serialization should be counted in total time */
		/* But two are contained in m_stat_mainloop */
		log().verbose("Total devices  {1:15}", total_time);
		log().verbose("");
		log().verbose("Take the next lines with a grain of salt. They depend on the measurement implementation.");
		log().verbose("Total overhead {1:15}", total_overhead);
		log().verbose("Overhead per pop  {1:11}", (m_stat_mainloop()-2*total_overhead - (total_time - total_overhead ))/queue().m_prof_call());
		log().verbose("");
		for (auto &entry : m_devices)
		{
			if (entry->m_stat_inc_active() > 3 * entry->m_stat_total_time.count())
				log().verbose("PARAM({}.USE_DEACTIVATE, 0)", entry->name());
		}
	}
}

// ----------------------------------------------------------------------------------------
// Parameters ...
// ----------------------------------------------------------------------------------------

template <typename C, param_t::param_type_t T>
param_template_t<C, T>::param_template_t(device_t &device, const pstring name, const C val)
: param_t(T, device, device.name() + "." + name)
, m_param(val)
{
	/* pstrings not yet supported, these need special logic */
	if (T != param_t::STRING && T != param_t::MODEL)
		netlist().save(*this, m_param, "m_param");
	device.setup().register_and_set_param(device.name() + "." + name, *this);
}

template class param_template_t<double, param_t::DOUBLE>;
template class param_template_t<int, param_t::INTEGER>;
template class param_template_t<bool, param_t::LOGIC>;
template class param_template_t<pstring, param_t::STRING>;
template class param_template_t<pstring, param_t::MODEL>;

// ----------------------------------------------------------------------------------------
// core_device_t
// ----------------------------------------------------------------------------------------

core_device_t::core_device_t(netlist_t &owner, const pstring &name)
	: object_t(name)
	, logic_family_t()
	, netlist_ref(owner)
	, m_static_update()
{
	if (logic_family() == nullptr)
		set_logic_family(family_TTL());
}

core_device_t::core_device_t(core_device_t &owner, const pstring &name)
	: object_t(owner.name() + "." + name)
	, logic_family_t()
	, netlist_ref(owner.netlist())
	, m_static_update()
{
	set_logic_family(owner.logic_family());
	if (logic_family() == nullptr)
		set_logic_family(family_TTL());
	owner.netlist().m_devices.push_back(plib::owned_ptr<core_device_t>(this, false));
}

core_device_t::~core_device_t()
{
}

void core_device_t::set_delegate_pointer()
{
#if (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF)
	void (core_device_t::* pFunc)() = &core_device_t::update;
	m_static_update = pFunc;
#elif (NL_PMF_TYPE == NL_PMF_TYPE_GNUC_PMF_CONV)
	void (core_device_t::* pFunc)() = &core_device_t::update;
	m_static_update = reinterpret_cast<net_update_delegate>((this->*pFunc));
#elif (NL_PMF_TYPE == NL_PMF_TYPE_INTERNAL)
	m_static_update = plib::mfp::get_mfp<net_update_delegate>(&core_device_t::update, this);
#endif
}

void core_device_t::stop_dev()
{
#if (NL_KEEP_STATISTICS)
#endif
	//stop();
}

netlist_sig_t core_device_t::INPLOGIC_PASSIVE(logic_input_t &inp)
{
	if (inp.state() != logic_t::STATE_INP_PASSIVE)
		return inp.Q();
	else
	{
		inp.activate();
		const netlist_sig_t ret = inp.Q();
		inp.inactivate();
		return ret;
	}
}


// ----------------------------------------------------------------------------------------
// device_t
// ----------------------------------------------------------------------------------------

device_t::~device_t()
{
	//log().debug("~net_device_t\n");
}

setup_t &device_t::setup()
{
	return netlist().setup();
}

void device_t::register_subalias(const pstring &name, core_terminal_t &term)
{
	pstring alias = this->name() + "." + name;

	// everything already fully qualified
	setup().register_alias_nofqn(alias, term.name());
}

void device_t::register_subalias(const pstring &name, const pstring &aliased)
{
	pstring alias = this->name() + "." + name;
	pstring aliased_fqn = this->name() + "." + aliased;

	// everything already fully qualified
	setup().register_alias_nofqn(alias, aliased_fqn);
}

void device_t::connect_late(core_terminal_t &t1, core_terminal_t &t2)
{
	setup().register_link_fqn(t1.name(), t2.name());
}

void device_t::connect_late(const pstring &t1, const pstring &t2)
{
	setup().register_link_fqn(name() + "." + t1, name() + "." + t2);
}

/* FIXME: this is only used by solver code since matrix solvers are started in
 *        post_start.
 */
void device_t::connect_post_start(core_terminal_t &t1, core_terminal_t &t2)
{
	if (!setup().connect(t1, t2))
		netlist().log().fatal("Error connecting {1} to {2}\n", t1.name(), t2.name());
}


// -----------------------------------------------------------------------------
// family_setter_t
// -----------------------------------------------------------------------------

family_setter_t::family_setter_t(core_device_t &dev, const char *desc)
{
	dev.set_logic_family(dev.netlist().setup().family_from_model(desc));
}

family_setter_t::family_setter_t(core_device_t &dev, const logic_family_desc_t *desc)
{
	dev.set_logic_family(desc);
}

// ----------------------------------------------------------------------------------------
// net_t
// ----------------------------------------------------------------------------------------

// FIXME: move somewhere central

struct do_nothing_deleter{
    template<typename T> void operator()(T*){}
};


net_t::net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr)
	: object_t(aname)
	, netlist_ref(nl)
	, m_new_Q(*this, "m_new_Q", 0)
	, m_cur_Q (*this, "m_cur_Q", 0)
	, m_time(*this, "m_time", netlist_time::zero())
	, m_active(*this, "m_active", 0)
	, m_in_queue(*this, "m_in_queue", 2)
	, m_railterminal(nullptr)
	, m_cur_Analog(*this, "m_cur_Analog", 0.0)
{
	m_railterminal = mr;
	if (mr != nullptr)
		nl.m_nets.push_back(std::shared_ptr<net_t>(this, do_nothing_deleter()));
	else
		nl.m_nets.push_back(std::shared_ptr<net_t>(this));
}

net_t::~net_t()
{
	netlist().state().remove_save_items(this);
}

void net_t::inc_active(core_terminal_t &term)
{
	m_active++;
	m_list_active.push_front(&term);
	nl_assert(m_active <= (int) num_cons());
	if (m_active == 1)
	{
		if (netlist().use_deactivate())
		{
			railterminal().device().do_inc_active();
			//m_cur_Q = m_new_Q;
		}
		if (m_in_queue == 0)
		{
			if (m_time > netlist().time())
			{
				m_in_queue = 1;     /* pending */
				netlist().push_to_queue(*this, m_time);
			}
			else
			{
				m_cur_Q = m_new_Q;
				m_in_queue = 2;
			}
		}
	}
}

void net_t::dec_active(core_terminal_t &term)
{
	--m_active;
	nl_assert(m_active >= 0);
	m_list_active.remove(&term);
	if (m_active == 0 && netlist().use_deactivate())
		railterminal().device().do_dec_active();
}

void net_t::rebuild_list()
{
	/* rebuild m_list */

	unsigned cnt = 0;
	m_list_active.clear();
	for (auto & term : m_core_terms)
		if (term->state() != logic_t::STATE_INP_PASSIVE)
		{
			m_list_active.push_back(term);
			cnt++;
		}
	m_active = cnt;
}

void net_t::update_devs()
{
	//assert(m_num_cons != 0);
	nl_assert(this->isRailNet());

	static const unsigned masks[4] =
	{
		0,
		core_terminal_t::STATE_INP_LH | core_terminal_t::STATE_INP_ACTIVE,
		core_terminal_t::STATE_INP_HL | core_terminal_t::STATE_INP_ACTIVE,
		0
	};

	const unsigned mask = masks[ m_cur_Q  * 2 + m_new_Q ];

	m_in_queue = 2; /* mark as taken ... */
	m_cur_Q = m_new_Q;

	for (auto & p : m_list_active)
	{
		p.device().m_stat_call_count.inc();
		if ((p.state() & mask) != 0)
			p.device().update_dev();
	}
}

void net_t::reset()
{
	m_time = netlist_time::zero();
	m_active = 0;
	m_in_queue = 2;

	m_new_Q = 0;
	m_cur_Q = 0;
	m_cur_Analog = 0.0;

	/* rebuild m_list */

	m_list_active.clear();
	for (core_terminal_t *ct : m_core_terms)
		m_list_active.push_back(ct);

	for (core_terminal_t *ct : m_core_terms)
		ct->reset();

	for (core_terminal_t *ct : m_core_terms)
		if (ct->state() != logic_t::STATE_INP_PASSIVE)
			m_active++;
}

void net_t::register_con(core_terminal_t &terminal)
{
	terminal.set_net(this);

	m_core_terms.push_back(&terminal);

	if (terminal.state() != logic_t::STATE_INP_PASSIVE)
		m_active++;
}

void net_t::move_connections(net_t *dest_net)
{
	for (core_terminal_t *ct : m_core_terms)
		dest_net->register_con(*ct);
	m_core_terms.clear();
	m_active = 0;
}

void net_t::merge_net(net_t *othernet)
{
	netlist().log().debug("merging nets ...\n");
	if (othernet == nullptr)
		return; // Nothing to do

	if (othernet == this)
	{
		netlist().log().warning("Connecting {1} to itself. This may be right, though\n", this->name());
		return; // Nothing to do
	}

	if (this->isRailNet() && othernet->isRailNet())
		netlist().log().fatal("Trying to merge two rail nets: {1} and {2}\n", this->name(), othernet->name());

	if (othernet->isRailNet())
	{
		netlist().log().debug("othernet is railnet\n");
		othernet->merge_net(this);
	}
	else
	{
		othernet->move_connections(this);
	}
}


// ----------------------------------------------------------------------------------------
// logic_net_t
// ----------------------------------------------------------------------------------------

logic_net_t::logic_net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr)
	: net_t(nl, aname, mr)
{
}


// ----------------------------------------------------------------------------------------
// analog_net_t
// ----------------------------------------------------------------------------------------

analog_net_t::analog_net_t(netlist_t &nl, const pstring &aname, core_terminal_t *mr)
	: net_t(nl, aname, mr)
	, m_solver(nullptr)
{
}

bool analog_net_t::already_processed(std::vector<list_t> &groups)
{
	if (isRailNet())
		return true;
	for (auto & grp : groups)
	{
		if (plib::container::contains(grp, this))
			return true;
	}
	return false;
}

void analog_net_t::process_net(std::vector<list_t> &groups)
{
	if (num_cons() == 0)
		return;
	/* add the net */
	groups.back().push_back(this);
	for (core_terminal_t *p : m_core_terms)
	{
		if (p->is_type(terminal_t::TERMINAL))
		{
			terminal_t *pt = static_cast<terminal_t *>(p);
			analog_net_t *other_net = &pt->m_otherterm->net();
			if (!other_net->already_processed(groups))
				other_net->process_net(groups);
		}
	}
}


// ----------------------------------------------------------------------------------------
// core_terminal_t
// ----------------------------------------------------------------------------------------

core_terminal_t::core_terminal_t(core_device_t &dev, const pstring &aname, const type_t atype)
: device_object_t(dev, dev.name() + "." + aname, atype)
, plib::linkedlist_t<core_terminal_t>::element_t()
, m_net(nullptr)
, m_state(*this, "m_state", STATE_NONEX)
{
}

void core_terminal_t::reset()
{
	if (is_type(OUTPUT))
		set_state(STATE_OUT);
	else
		set_state(STATE_INP_ACTIVE);
}

void core_terminal_t::set_net(net_t *anet)
{
	m_net = anet;
}

 void core_terminal_t::clear_net()
{
	m_net = nullptr;
}


// ----------------------------------------------------------------------------------------
// terminal_t
// ----------------------------------------------------------------------------------------

terminal_t::terminal_t(core_device_t &dev, const pstring &aname)
: analog_t(dev, aname, TERMINAL)
, m_otherterm(nullptr)
, m_Idr1(*this, "m_Idr1", nullptr)
, m_go1(*this, "m_go1", nullptr)
, m_gt1(*this, "m_gt1", nullptr)
{
	netlist().setup().register_term(*this);
}


void terminal_t::schedule_solve()
{
	// FIXME: Remove this after we found a way to remove *ALL* twoterms connected to railnets only.
	if (net().solver() != nullptr)
		net().solver()->update_forced();
}

void terminal_t::schedule_after(const netlist_time &after)
{
	// FIXME: Remove this after we found a way to remove *ALL* twoterms connected to railnets only.
	if (net().solver() != nullptr)
		net().solver()->update_after(after);
}

// ----------------------------------------------------------------------------------------
// net_input_t
// ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
// net_output_t
// ----------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
// logic_output_t
// ----------------------------------------------------------------------------------------

logic_output_t::logic_output_t(core_device_t &dev, const pstring &aname)
	: logic_t(dev, aname, OUTPUT)
	, m_my_net(dev.netlist(), name() + ".net", this)
{
	set_state(STATE_OUT);
	this->set_net(&m_my_net);
	set_logic_family(dev.logic_family());
	netlist().setup().register_term(*this);
}

void logic_output_t::initial(const netlist_sig_t val)
{
	net().initial(val);
}

// ----------------------------------------------------------------------------------------
// analog_input_t
// ----------------------------------------------------------------------------------------

analog_input_t::analog_input_t(core_device_t &dev, const pstring &aname)
: analog_t(dev, aname, INPUT)
{
	set_state(STATE_INP_ACTIVE);
	netlist().setup().register_term(*this);
}

// ----------------------------------------------------------------------------------------
// analog_output_t
// ----------------------------------------------------------------------------------------

analog_output_t::analog_output_t(core_device_t &dev, const pstring &aname)
	: analog_t(dev, aname, OUTPUT)
	, m_my_net(dev.netlist(), name() + ".net", this)
{
	this->set_net(&m_my_net);
	set_state(STATE_OUT);

	net().m_cur_Analog = NL_FCONST(0.0);
	netlist().setup().register_term(*this);
}

void analog_output_t::initial(const nl_double val)
{
	net().m_cur_Analog = val;
}

// -----------------------------------------------------------------------------
// logic_input_t
// -----------------------------------------------------------------------------

logic_input_t::logic_input_t(core_device_t &dev, const pstring &aname)
		: logic_t(dev, aname, INPUT)
{
	set_state(STATE_INP_ACTIVE);
	set_logic_family(dev.logic_family());
	netlist().setup().register_term(*this);
}

// ----------------------------------------------------------------------------------------
// param_t & friends
// ----------------------------------------------------------------------------------------

param_t::param_t(const param_type_t atype, device_t &device, const pstring &name)
	: device_object_t(device, name, PARAM)
	, m_param_type(atype)
{
}

const pstring param_model_t::model_type()
{
	if (m_map.size() == 0)
		netlist().setup().model_parse(this->Value(), m_map);
	return m_map["COREMODEL"];
}


const pstring param_model_t::model_value_str(const pstring &entity)
{
	if (m_map.size() == 0)
		netlist().setup().model_parse(this->Value(), m_map);
	return netlist().setup().model_value_str(m_map, entity);
}

nl_double param_model_t::model_value(const pstring &entity)
{
	if (m_map.size() == 0)
		netlist().setup().model_parse(this->Value(), m_map);
	return netlist().setup().model_value(m_map, entity);
}


} // namespace

namespace netlist
{
	namespace devices
	{

	// ----------------------------------------------------------------------------------------
	// mainclock
	// ----------------------------------------------------------------------------------------

	void NETLIB_NAME(mainclock)::mc_update(logic_net_t &net)
	{
		net.toggle_new_Q();
		net.update_devs();
	}


	} //namespace devices
} // namespace netlist

