// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlbase.c
 *
 */

#include "solver/nld_matrix_solver.h"
#include "solver/nld_solver.h"

#include "plib/putil.h"
#include "plib/palloc.h"

#include "nl_base.h"
#include "devices/nlid_system.h"
#include "devices/nlid_proxy.h"
#include "macro/nlm_base.h"

#include "nl_errstr.h"

#include <cstring>
#include <cmath>
#include <limits>

namespace netlist
{
namespace detail
{

	static plib::mempool *pool()
	{
		static plib::mempool *s_pool = nullptr;
		if (s_pool == nullptr)
			s_pool = new plib::mempool(65536, 16);
		return s_pool;
	}

	void * object_t::operator new (size_t size)
	{
		void *ret = nullptr;
		if ((USE_MEMPOOL))
			ret = pool()->alloc(size);
		else
			ret = ::operator new(size);
		return ret;
	}

	void object_t::operator delete (void * mem)
	{
		if (mem)
		{
			if ((USE_MEMPOOL))
				pool()->free(mem);
			else
				::operator delete(mem);
		}
	}

}

nl_exception::~nl_exception()
{
}


// ----------------------------------------------------------------------------------------
// logic_family_ttl_t
// ----------------------------------------------------------------------------------------

logic_family_desc_t::logic_family_desc_t()
{
}

logic_family_desc_t::~logic_family_desc_t()
{
}


class logic_family_ttl_t : public logic_family_desc_t
{
public:
	logic_family_ttl_t() : logic_family_desc_t()
	{
		m_fixed_V = 5.0;
		m_low_thresh_PCNT = 0.8 / 5.0;
		m_high_thresh_PCNT = 2.0 / 5.0;
		// m_low_V  - these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_low_VO = 0.1;
		m_high_VO = 1.0; // 4.0
		m_R_low = 1.0;
		m_R_high = 130.0;
	}
	virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_base_t &anetlist, const pstring &name, logic_output_t *proxied) const override;
	virtual plib::owned_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_base_t &anetlist, const pstring &name, logic_input_t *proxied) const override;
};

plib::owned_ptr<devices::nld_base_d_to_a_proxy> logic_family_ttl_t::create_d_a_proxy(netlist_base_t &anetlist, const pstring &name, logic_output_t *proxied) const
{
	return plib::owned_ptr<devices::nld_base_d_to_a_proxy>::Create<devices::nld_d_to_a_proxy>(anetlist, name, proxied);
}
plib::owned_ptr<devices::nld_base_a_to_d_proxy> logic_family_ttl_t::create_a_d_proxy(netlist_base_t &anetlist, const pstring &name, logic_input_t *proxied) const
{
	return plib::owned_ptr<devices::nld_base_a_to_d_proxy>::Create<devices::nld_a_to_d_proxy>(anetlist, name, proxied);
}

class logic_family_cd4xxx_t : public logic_family_desc_t
{
public:
	logic_family_cd4xxx_t() : logic_family_desc_t()
	{
		m_fixed_V = 0.0;
		m_low_thresh_PCNT = 1.5 / 5.0;
		m_high_thresh_PCNT = 3.5 / 5.0;
		// m_low_V  - these depend on sinked/sourced current. Values should be suitable for typical applications.
		m_low_VO = 0.05;
		m_high_VO = 0.05; // 4.95
		m_R_low = 10.0;
		m_R_high = 10.0;
	}
	virtual plib::owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_base_t &anetlist, const pstring &name, logic_output_t *proxied) const override;
	virtual plib::owned_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_base_t &anetlist, const pstring &name, logic_input_t *proxied) const override;
};

plib::owned_ptr<devices::nld_base_d_to_a_proxy> logic_family_cd4xxx_t::create_d_a_proxy(netlist_base_t &anetlist, const pstring &name, logic_output_t *proxied) const
{
	return plib::owned_ptr<devices::nld_base_d_to_a_proxy>::Create<devices::nld_d_to_a_proxy>(anetlist, name, proxied);
}
plib::owned_ptr<devices::nld_base_a_to_d_proxy> logic_family_cd4xxx_t::create_a_d_proxy(netlist_base_t &anetlist, const pstring &name, logic_input_t *proxied) const
{
	return plib::owned_ptr<devices::nld_base_a_to_d_proxy>::Create<devices::nld_a_to_d_proxy>(anetlist, name, proxied);
}

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

detail::queue_t::queue_t(netlist_t &nl)
	: timed_queue<pqentry_t<net_t *, netlist_time>, false, NL_KEEP_STATISTICS>(512)
	, netlist_ref(nl)
	, plib::state_manager_t::callback_t()
	, m_qsize(0)
	, m_times(512)
	, m_net_ids(512)
{
}

void detail::queue_t::register_state(plib::state_manager_t &manager, const pstring &module)
{
	//netlist().log().debug("register_state\n");
	manager.save_item(this, m_qsize, module + "." + "qsize");
	manager.save_item(this, &m_times[0], module + "." + "times", m_times.size());
	manager.save_item(this, &m_net_ids[0], module + "." + "names", m_net_ids.size());
}

void detail::queue_t::on_pre_save()
{
	netlist().log().debug("on_pre_save\n");
	m_qsize = this->size();
	netlist().log().debug("current time {1} qsize {2}\n", exec().time().as_double(), m_qsize);
	for (std::size_t i = 0; i < m_qsize; i++ )
	{
		m_times[i] =  this->listptr()[i].m_exec_time.as_raw();
		m_net_ids[i] = netlist().find_net_id(this->listptr()[i].m_object);
	}
}


void detail::queue_t::on_post_load()
{
	this->clear();
	netlist().log().debug("current time {1} qsize {2}\n", exec().time().as_double(), m_qsize);
	for (std::size_t i = 0; i < m_qsize; i++ )
	{
		detail::net_t *n = netlist().nets()[m_net_ids[i]].get();
		this->push(queue_t::entry_t(netlist_time::from_raw(m_times[i]),n));
	}
}

// ----------------------------------------------------------------------------------------
// object_t
// ----------------------------------------------------------------------------------------

detail::object_t::object_t(const pstring &aname)
	: m_name(plib::make_unique<pstring>(aname))
{
}

detail::object_t::~object_t()
{
}

const pstring &detail::object_t::name() const
{
	return *m_name;
}

// ----------------------------------------------------------------------------------------
// device_object_t
// ----------------------------------------------------------------------------------------

detail::device_object_t::device_object_t(core_device_t &dev, const pstring &aname)
: object_t(aname)
, m_device(dev)
{
}

detail::terminal_type detail::core_terminal_t::type() const
{
	if (dynamic_cast<const terminal_t *>(this) != nullptr)
		return terminal_type::TERMINAL;
	else if (dynamic_cast<const logic_input_t *>(this) != nullptr)
		return terminal_type::INPUT;
	else if (dynamic_cast<const logic_output_t *>(this) != nullptr)
		return terminal_type::OUTPUT;
	else if (dynamic_cast<const analog_input_t *>(this) != nullptr)
		return terminal_type::INPUT;
	else if (dynamic_cast<const analog_output_t *>(this) != nullptr)
		return terminal_type::OUTPUT;
	else
	{
		netlist().log().fatal(MF_1_UNKNOWN_TYPE_FOR_OBJECT, name());
		return terminal_type::TERMINAL; // please compiler
	}
}

// ----------------------------------------------------------------------------------------
// netlist_t
// ----------------------------------------------------------------------------------------

netlist_t::netlist_t(const pstring &aname, std::unique_ptr<callbacks_t> callbacks)
	: netlist_base_t(aname, std::move(callbacks))
	, m_time(netlist_time::zero())
	, m_mainclock(nullptr)
	, m_queue(*this)
	, m_solver(nullptr)
{
	state().save_item(this, static_cast<plib::state_manager_t::callback_t &>(m_queue), "m_queue");
	state().save_item(this, m_time, "m_time");
}

netlist_t::~netlist_t()
{
}

// ----------------------------------------------------------------------------------------
// netlist_t
// ----------------------------------------------------------------------------------------

netlist_base_t::netlist_base_t(const pstring &aname, std::unique_ptr<callbacks_t> callbacks)
	 :m_params(nullptr)
	, m_name(aname)
	, m_lib(nullptr)
	, m_state()
	, m_callbacks(std::move(callbacks))	// Order is important here
	, m_log(*m_callbacks)
{
	m_setup = plib::make_unique<setup_t>(*this);
	NETLIST_NAME(base)(*m_setup);
}

netlist_base_t::~netlist_base_t()
{
	m_nets.clear();
	m_devices.clear();
}

void netlist_base_t::register_dev(plib::owned_ptr<core_device_t> dev)
{
	for (auto & d : m_devices)
		if (d->name() == dev->name())
			log().fatal(MF_1_DUPLICATE_NAME_DEVICE_LIST, d->name());
	m_devices.push_back(std::move(dev));
}

void netlist_base_t::remove_dev(core_device_t *dev)
{
	m_devices.erase(
		std::remove_if(
			m_devices.begin(),
			m_devices.end(),
			[&] (plib::owned_ptr<core_device_t> const& p)
			{
				return p.get() == dev;
			}),
			m_devices.end()
		);
}

void netlist_base_t::delete_empty_nets()
{
	m_nets.erase(
		std::remove_if(m_nets.begin(), m_nets.end(),
			[](plib::owned_ptr<detail::net_t> &x)
			{
				if (x->num_cons() == 0)
				{
					x->netlist().log().verbose("Deleting net {1} ...", x->name());
					return true;
				}
				else
					return false;
			}), m_nets.end());
}

void netlist_base_t::prepare_to_run()
{
	setup().register_dynamic_log_devices();

	/* load the library ... */

	/* make sure the solver and parameters are started first! */

	for (auto & e : setup().m_device_factory)
	{
		if ( setup().factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				|| setup().factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			auto dev = plib::owned_ptr<device_t>(e.second->Create(*this, e.first));
			register_dev(std::move(dev));
		}
	}

	log().debug("Searching for solver and parameters ...\n");

	auto solver = get_single_device<devices::NETLIB_NAME(solver)>("solver");
	m_params = get_single_device<devices::NETLIB_NAME(netlistparams)>("parameter");

	/* create devices */

	log().debug("Creating devices ...\n");
	for (auto & e : setup().m_device_factory)
	{
		if ( !setup().factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				&& !setup().factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			auto dev = plib::owned_ptr<device_t>(e.second->Create(*this, e.first));
			register_dev(std::move(dev));
		}
	}

	bool use_deactivate = (m_params->m_use_deactivate() ? true : false);

	for (auto &d : m_devices)
	{
		if (use_deactivate)
		{
			auto p = setup().m_param_values.find(d->name() + ".HINT_NO_DEACTIVATE");
			if (p != setup().m_param_values.end())
			{
				//FIXME: check for errors ...
				double v = plib::pstonum<double>(p->second);
				if (std::abs(v - std::floor(v)) > 1e-6 )
					log().fatal(MF_1_HND_VAL_NOT_SUPPORTED, p->second);
				d->set_hint_deactivate(v == 0.0);
			}
		}
		else
			d->set_hint_deactivate(false);
	}

	pstring libpath = plib::util::environment("NL_BOOSTLIB", plib::util::buildpath({".", "nlboost.so"}));
	m_lib = plib::make_unique<plib::dynlib>(libpath);

	/* resolve inputs */
	setup().resolve_inputs();

	log().verbose("looking for two terms connected to rail nets ...");
	for (auto & t : get_device_list<analog::NETLIB_NAME(twoterm)>())
	{
		if (t->m_N.net().isRailNet() && t->m_P.net().isRailNet())
		{
			log().warning(MW_3_REMOVE_DEVICE_1_CONNECTED_ONLY_TO_RAILS_2_3,
				t->name(), t->m_N.net().name(), t->m_P.net().name());
			t->m_N.net().remove_terminal(t->m_N);
			t->m_P.net().remove_terminal(t->m_P);
			remove_dev(t);
		}
	}

	log().verbose("initialize solver ...\n");

	if (solver == nullptr)
	{
		for (auto &p : m_nets)
			if (p->is_analog())
				log().fatal(MF_0_NO_SOLVER);
	}
	else
		solver->post_start();

	for (auto &n : m_nets)
		for (auto & term : n->m_core_terms)
		{
			//core_device_t *dev = reinterpret_cast<core_device_t *>(term->m_delegate.object());
			core_device_t *dev = &term->device();
			dev->set_default_delegate(*term);
		}

}

void netlist_t::stop()
{
	log().debug("Printing statistics ...\n");
	print_stats();
	log().debug("Stopping solver device ...\n");
	if (m_solver != nullptr)
		m_solver->stop();
}

detail::net_t *netlist_base_t::find_net(const pstring &name) const
{
	for (auto & net : m_nets)
		if (net->name() == name)
			return net.get();

	return nullptr;
}

std::size_t netlist_base_t::find_net_id(const detail::net_t *net) const
{
	for (std::size_t i = 0; i < m_nets.size(); i++)
		if (m_nets[i].get() == net)
			return i;
	return std::numeric_limits<std::size_t>::max();
}



void netlist_base_t::rebuild_lists()
{
	for (auto & net : m_nets)
		net->rebuild_list();
}


void netlist_t::reset()
{
	log().debug("Searching for mainclock\n");
	m_mainclock = get_single_device<devices::NETLIB_NAME(mainclock)>("mainclock");

	log().debug("Searching for solver\n");
	m_solver = get_single_device<devices::NETLIB_NAME(solver)>("solver");

	m_time = netlist_time::zero();
	m_queue.clear();
	if (m_mainclock != nullptr)
		m_mainclock->m_Q.net().set_time(netlist_time::zero());
	//if (m_solver != nullptr)
	//  m_solver->do_reset();

	std::unordered_map<core_device_t *, bool> m;

	for (auto &d : m_devices)
	{
		m[d.get()] = d->get_hint_deactivate();
	}


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
	/*
	 * INFO: The order here affects power up of e.g. breakout. However, such
	 * variations are explicitly stated in the breakout manual.
	 */

	const unsigned startup_strategy = 1; //! \note make this a parameter

	switch (startup_strategy)
	{
		case 0:
		{
			std::vector<core_device_t *> d;
			std::vector<nldelegate *> t;
			log().verbose("Using default startup strategy");
			for (auto &n : m_nets)
				for (auto & term : n->m_core_terms)
					if (term->m_delegate.has_object())
					{
						if (!plib::container::contains(t, &term->m_delegate))
						{
							t.push_back(&term->m_delegate);
							term->m_delegate();
						}
						core_device_t *dev = reinterpret_cast<core_device_t *>(term->m_delegate.object());
						if (!plib::container::contains(d, dev))
							d.push_back(dev);
					}
			log().verbose("Call update on devices which need parameter update:");
			for (auto & dev : m_devices)
				if (dev->needs_update_after_param_change())
				{
					if (!plib::container::contains(d, dev.get()))
					{
						d.push_back(dev.get());
						log().verbose("\t ...{1}", dev->name());
						dev->update_dev();
					}
				}
			log().verbose("Devices not yet updated:");
			for (auto &dev : m_devices)
				if (!plib::container::contains(d, dev.get()))
					log().verbose("\t ...{1}", dev->name());
					//x->update_dev();
		}
		break;
		case 1:     // brute force backward
		{
			log().verbose("Using brute force backward startup strategy");
			std::size_t i = m_devices.size();
			while (i>0)
				m_devices[--i]->update_dev();
		}
		break;
		case 2:     // brute force forward
		{
			log().verbose("Using brute force forward startup strategy");
			for (auto &d : m_devices)
				d->update_dev();
		}
		break;
	}

#if 1
	/* the above may screw up m_active and the list */
	for (auto &n : m_nets)
		n->rebuild_list();
#endif
}


void netlist_t::process_queue(const netlist_time &delta) NL_NOEXCEPT
{
	netlist_time stop(m_time + delta);

	m_queue.push(detail::queue_t::entry_t(stop, nullptr));

	m_stat_mainloop.start();

	if (m_mainclock == nullptr)
	{
		detail::queue_t::entry_t e(m_queue.top());
		m_queue.pop();
		m_time = e.m_exec_time;
		while (e.m_object != nullptr)
		{
			e.m_object->update_devs();
			m_perf_out_processed.inc();
			e = m_queue.top();
			m_queue.pop();
			m_time = e.m_exec_time;
		}
	}
	else
	{
		logic_net_t &mc_net(m_mainclock->m_Q.net());
		const netlist_time inc(m_mainclock->m_inc);
		netlist_time mc_time(mc_net.time());

		detail::queue_t::entry_t e;

		do
		{
			while (m_queue.top().m_exec_time > mc_time)
			{
				m_time = mc_time;
				mc_net.toggle_new_Q();
				mc_net.update_devs();
				mc_time += inc;
			}

			e = m_queue.top();
			m_queue.pop();
			m_time = e.m_exec_time;
			if (e.m_object != nullptr)
			{
				e.m_object->update_devs();
				m_perf_out_processed.inc();
			}
		} while (e.m_object != nullptr);
		mc_net.set_time(mc_time);
	}
	m_stat_mainloop.stop();
}

void netlist_t::print_stats() const
{
	if (nperftime_t<NL_KEEP_STATISTICS>::enabled)
	{
		std::vector<size_t> index;
		for (size_t i=0; i<m_devices.size(); i++)
			index.push_back(i);

		std::sort(index.begin(), index.end(),
				[&](size_t i1, size_t i2) { return m_devices[i1]->m_stat_total_time.total() < m_devices[i2]->m_stat_total_time.total(); });

		nperftime_t<NL_KEEP_STATISTICS>::type total_time(0);
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

		nperftime_t<NL_KEEP_STATISTICS> overhead;
		nperftime_t<NL_KEEP_STATISTICS> test;
		overhead.start();
		for (int j=0; j<100000;j++)
		{
			test.start();
			test.stop();
		}
		overhead.stop();

		nperftime_t<NL_KEEP_STATISTICS>::type total_overhead = overhead()
				* static_cast<nperftime_t<NL_KEEP_STATISTICS>::type>(total_count)
				/ static_cast<nperftime_t<NL_KEEP_STATISTICS>::type>(200000);

		log().verbose("Queue Pushes   {1:15}", m_queue.m_prof_call());
		log().verbose("Queue Moves    {1:15}", m_queue.m_prof_sortmove());

		log().verbose("Total loop     {1:15}", m_stat_mainloop());
		/* Only one serialization should be counted in total time */
		/* But two are contained in m_stat_mainloop */
		log().verbose("Total devices  {1:15}", total_time);
		log().verbose("");
		log().verbose("Take the next lines with a grain of salt. They depend on the measurement implementation.");
		log().verbose("Total overhead {1:15}", total_overhead);
		nperftime_t<NL_KEEP_STATISTICS>::type overhead_per_pop = (m_stat_mainloop()-2*total_overhead - (total_time - total_overhead))
				/ static_cast<nperftime_t<NL_KEEP_STATISTICS>::type>(m_queue.m_prof_call());
		log().verbose("Overhead per pop  {1:11}", overhead_per_pop );
		log().verbose("");
		for (auto &entry : m_devices)
		{
			if (entry->m_stat_inc_active() > 3 * entry->m_stat_total_time.count())
				log().verbose("HINT({}, NO_DEACTIVATE)", entry->name());
		}
	}
}

core_device_t *netlist_base_t::get_single_device(const pstring &classname, bool (*cc)(core_device_t *)) const
{
	core_device_t *ret = nullptr;
	for (auto &d : m_devices)
	{
		if (cc(d.get()))
		{
			if (ret != nullptr)
				this->log().fatal(MF_1_MORE_THAN_ONE_1_DEVICE_FOUND, classname);
			else
				ret = d.get();
		}
	}
	return ret;
}


// ----------------------------------------------------------------------------------------
// core_device_t
// ----------------------------------------------------------------------------------------

core_device_t::core_device_t(netlist_base_t &owner, const pstring &name)
	: object_t(name)
	, logic_family_t()
	, netlist_ref(owner)
	, m_hint_deactivate(false)
{
	if (logic_family() == nullptr)
		set_logic_family(family_TTL());
}

core_device_t::core_device_t(core_device_t &owner, const pstring &name)
	: object_t(owner.name() + "." + name)
	, logic_family_t()
	, netlist_ref(owner.netlist())
	, m_hint_deactivate(false)
{
	set_logic_family(owner.logic_family());
	if (logic_family() == nullptr)
		set_logic_family(family_TTL());
	owner.netlist().register_dev(plib::owned_ptr<core_device_t>(this, false));
}

core_device_t::~core_device_t()
{
}

void core_device_t::set_default_delegate(detail::core_terminal_t &term)
{
	if (!term.m_delegate.is_set())
		term.m_delegate.set(&core_device_t::update, this);
}

log_type & core_device_t::log()
{
	return netlist().log();
}

// ----------------------------------------------------------------------------------------
// device_t
// ----------------------------------------------------------------------------------------

device_t::device_t(netlist_base_t &owner, const pstring &name)
: core_device_t(owner, name)
{
}

device_t::device_t(core_device_t &owner, const pstring &name)
: core_device_t(owner, name)
{
}

device_t::~device_t()
{
	//log().debug("~net_device_t\n");
}

setup_t &device_t::setup()
{
	return netlist().setup();
}

const setup_t &device_t::setup() const
{
	return netlist().setup();
}

void device_t::register_subalias(const pstring &name, detail::core_terminal_t &term)
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

void device_t::connect(detail::core_terminal_t &t1, detail::core_terminal_t &t2)
{
	setup().register_link_fqn(t1.name(), t2.name());
}

void device_t::connect(const pstring &t1, const pstring &t2)
{
	setup().register_link_fqn(name() + "." + t1, name() + "." + t2);
}

/* FIXME: this is only used by solver code since matrix solvers are started in
 *        post_start.
 */
void device_t::connect_post_start(detail::core_terminal_t &t1, detail::core_terminal_t &t2)
{
	if (!setup().connect(t1, t2))
		log().fatal(MF_2_ERROR_CONNECTING_1_TO_2, t1.name(), t2.name());
}


// -----------------------------------------------------------------------------
// family_setter_t
// -----------------------------------------------------------------------------

detail::family_setter_t::family_setter_t(core_device_t &dev, const pstring &desc)
{
	dev.set_logic_family(dev.netlist().setup().family_from_model(desc));
}

detail::family_setter_t::family_setter_t(core_device_t &dev, const logic_family_desc_t *desc)
{
	dev.set_logic_family(desc);
}

// ----------------------------------------------------------------------------------------
// net_t
// ----------------------------------------------------------------------------------------

detail::net_t::net_t(netlist_base_t &nl, const pstring &aname, core_terminal_t *mr)
	: object_t(aname)
	, netlist_ref(nl)
	, m_new_Q(*this, "m_new_Q", 0)
	, m_cur_Q (*this, "m_cur_Q", 0)
	, m_in_queue(*this, "m_in_queue", QS_DELIVERED)
	, m_time(*this, "m_time", netlist_time::zero())
	, m_railterminal(mr)
{
}

detail::net_t::~net_t()
{
	netlist().state().remove_save_items(this);
}

void detail::net_t::add_to_active_list(core_terminal_t &term) NL_NOEXCEPT
{
	const bool was_empty = m_list_active.empty();
	m_list_active.push_front(&term);
	if (was_empty)
	{
		railterminal().device().do_inc_active();
		if (m_in_queue == QS_DELAYED_DUE_TO_INACTIVE)
		{
			if (m_time > exec().time())
			{
				m_in_queue = QS_QUEUED;     /* pending */
				exec().queue().push({m_time, this});
			}
			else
			{
				m_in_queue = QS_DELIVERED;
				m_cur_Q = m_new_Q;
			}
		}
	}
}

void detail::net_t::remove_from_active_list(core_terminal_t &term) NL_NOEXCEPT
{
	m_list_active.remove(&term);
	if (m_list_active.empty())
		railterminal().device().do_dec_active();
}

void detail::net_t::rebuild_list()
{
	/* rebuild m_list */

	m_list_active.clear();
	for (auto & term : m_core_terms)
		if (term->state() != logic_t::STATE_INP_PASSIVE)
		{
			m_list_active.push_back(term);
		}
}

void detail::net_t::process(const std::uint_fast8_t mask)
{
	for (auto & p : m_list_active)
	{
		p.device().m_stat_call_count.inc();
		if ((p.state() & mask))
		{
			p.device().m_stat_total_time.start();
			p.m_delegate();
			p.device().m_stat_total_time.stop();
		}
	}
}

void detail::net_t::update_devs() NL_NOEXCEPT
{
	nl_assert(this->isRailNet());

	const auto new_Q(m_new_Q);

	const unsigned mask((new_Q << core_terminal_t::INP_LH_SHIFT)
			| (m_cur_Q << core_terminal_t::INP_HL_SHIFT));

	m_in_queue = QS_DELIVERED; /* mark as taken ... */

	if (mask == core_terminal_t::STATE_INP_HL)
	{
		m_cur_Q = new_Q;
		process(core_terminal_t::STATE_INP_HL | core_terminal_t::STATE_INP_ACTIVE);
	}
	else if (mask == core_terminal_t::STATE_INP_LH)
	{
		m_cur_Q = new_Q;
		process(core_terminal_t::STATE_INP_LH | core_terminal_t::STATE_INP_ACTIVE);
	}
}

void detail::net_t::reset()
{
	m_time = netlist_time::zero();
	m_in_queue = QS_DELIVERED;

	m_new_Q = 0;
	m_cur_Q = 0;

	analog_net_t *p = dynamic_cast<analog_net_t *>(this);

	if (p != nullptr)
		p->m_cur_Analog = 0.0;

	/* rebuild m_list and reset terminals to active or analog out state */

	m_list_active.clear();
	for (core_terminal_t *ct : m_core_terms)
	{
		ct->reset();
		if (ct->state() != logic_t::STATE_INP_PASSIVE)
			m_list_active.push_back(ct);
	}
}

void detail::net_t::add_terminal(detail::core_terminal_t &terminal)
{
	for (auto &t : m_core_terms)
		if (t == &terminal)
			netlist().log().fatal(MF_2_NET_1_DUPLICATE_TERMINAL_2, name(),
					t->name());

	terminal.set_net(this);

	m_core_terms.push_back(&terminal);
}

void detail::net_t::remove_terminal(detail::core_terminal_t &terminal)
{
	if (plib::container::contains(m_core_terms, &terminal))
	{
		terminal.set_net(nullptr);
		plib::container::remove(m_core_terms, &terminal);
	}
	else
		netlist().log().fatal(MF_2_REMOVE_TERMINAL_1_FROM_NET_2, terminal.name(),
				this->name());
}

void detail::net_t::move_connections(detail::net_t &dest_net)
{
	for (auto &ct : m_core_terms)
		dest_net.add_terminal(*ct);
	m_core_terms.clear();
}

// ----------------------------------------------------------------------------------------
// logic_net_t
// ----------------------------------------------------------------------------------------

logic_net_t::logic_net_t(netlist_base_t &nl, const pstring &aname, detail::core_terminal_t *mr)
	: net_t(nl, aname, mr)
{
}

logic_net_t::~logic_net_t()
{
}

// ----------------------------------------------------------------------------------------
// analog_net_t
// ----------------------------------------------------------------------------------------

analog_net_t::analog_net_t(netlist_base_t &nl, const pstring &aname, detail::core_terminal_t *mr)
	: net_t(nl, aname, mr)
	, m_cur_Analog(*this, "m_cur_Analog", 0.0)
	, m_solver(nullptr)
{
}

analog_net_t::~analog_net_t()
{
}

// ----------------------------------------------------------------------------------------
// core_terminal_t
// ----------------------------------------------------------------------------------------

detail::core_terminal_t::core_terminal_t(core_device_t &dev, const pstring &aname,
		const state_e state, nldelegate delegate)
: device_object_t(dev, dev.name() + "." + aname)
, plib::linkedlist_t<core_terminal_t>::element_t()
, m_delegate(delegate)
, m_net(nullptr)
, m_state(*this, "m_state", state)
{
}

detail::core_terminal_t::~core_terminal_t()
{
}

analog_t::analog_t(core_device_t &dev, const pstring &aname, const state_e state)
: core_terminal_t(dev, aname, state)
{
}

analog_t::~analog_t()
{
}

logic_t::logic_t(core_device_t &dev, const pstring &aname, const state_e state,
		nldelegate delegate)
	: core_terminal_t(dev, aname, state, delegate)
	, logic_family_t()
	, m_proxy(nullptr)
{
}

logic_t::~logic_t()
{
}

// ----------------------------------------------------------------------------------------
// terminal_t
// ----------------------------------------------------------------------------------------

terminal_t::terminal_t(core_device_t &dev, const pstring &aname)
: analog_t(dev, aname, STATE_BIDIR)
, m_otherterm(nullptr)
, m_Idr1(nullptr)
, m_go1(nullptr)
, m_gt1(nullptr)
{
	netlist().setup().register_term(*this);
}

terminal_t::~terminal_t()
{
}

void terminal_t::solve_now()
{
	// Nets may belong to railnets which do not have a solver attached
	if (this->has_net())
		if (net().solver() != nullptr)
			net().solver()->update_forced();
}

void terminal_t::schedule_solve_after(const netlist_time &after)
{
	// Nets may belong to railnets which do not have a solver attached
	if (this->has_net())
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
	: logic_t(dev, aname, STATE_OUT)
	, m_my_net(dev.netlist(), name() + ".net", this)
{
	this->set_net(&m_my_net);
	netlist().register_net(plib::owned_ptr<logic_net_t>(&m_my_net, false));
	set_logic_family(dev.logic_family());
	netlist().setup().register_term(*this);
}

logic_output_t::~logic_output_t()
{
}

void logic_output_t::initial(const netlist_sig_t val)
{
	if (has_net())
		net().initial(val);
}

// ----------------------------------------------------------------------------------------
// analog_input_t
// ----------------------------------------------------------------------------------------

analog_input_t::analog_input_t(core_device_t &dev, const pstring &aname)
: analog_t(dev, aname, STATE_INP_ACTIVE)
{
	netlist().setup().register_term(*this);
}

analog_input_t::~analog_input_t()
{
}

// ----------------------------------------------------------------------------------------
// analog_output_t
// ----------------------------------------------------------------------------------------

analog_output_t::analog_output_t(core_device_t &dev, const pstring &aname)
	: analog_t(dev, aname, STATE_OUT)
	, m_my_net(dev.netlist(), name() + ".net", this)
{
	netlist().register_net(plib::owned_ptr<analog_net_t>(&m_my_net, false));
	this->set_net(&m_my_net);

	//net().m_cur_Analog = NL_FCONST(0.0);
	netlist().setup().register_term(*this);
}

analog_output_t::~analog_output_t()
{
}

void analog_output_t::initial(const nl_double val)
{
	net().set_Q_Analog(val);
}

// -----------------------------------------------------------------------------
// logic_input_t
// -----------------------------------------------------------------------------

logic_input_t::logic_input_t(core_device_t &dev, const pstring &aname,
		nldelegate delegate)
		: logic_t(dev, aname, STATE_INP_ACTIVE, delegate)
{
	set_logic_family(dev.logic_family());
	netlist().setup().register_term(*this);
}

logic_input_t::~logic_input_t()
{
}

// ----------------------------------------------------------------------------------------
// Parameters ...
// ----------------------------------------------------------------------------------------

param_t::param_t(device_t &device, const pstring &name)
	: device_object_t(device, device.name() + "." + name)
{
	device.setup().register_param(this->name(), *this);
}

param_t::~param_t()
{
}

param_t::param_type_t param_t::param_type() const
{
	if (dynamic_cast<const param_str_t *>(this) != nullptr)
		return STRING;
	else if (dynamic_cast<const param_double_t *>(this) != nullptr)
		return DOUBLE;
	else if (dynamic_cast<const param_int_t *>(this) != nullptr)
		return INTEGER;
	else if (dynamic_cast<const param_logic_t *>(this) != nullptr)
		return LOGIC;
	else if (dynamic_cast<const param_ptr_t *>(this) != nullptr)
		return POINTER;
	else
	{
		netlist().log().fatal(MF_1_UNKNOWN_PARAM_TYPE, name());
		return POINTER; /* Please compiler */
	}
}


void param_t::update_param()
{
	device().update_param();
	if (device().needs_update_after_param_change())
		device().update_dev();
}

pstring param_t::get_initial(const device_t &dev, bool *found)
{
	pstring res = dev.setup().get_initial_param_val(this->name(), "");
	*found = (res != "");
	return res;
}


const pstring param_model_t::model_type()
{
	if (m_map.size() == 0)
		netlist().setup().model_parse(this->Value(), m_map);
	return m_map["COREMODEL"];
}

param_str_t::param_str_t(device_t &device, const pstring &name, const pstring &val)
: param_t(device, name)
{
	m_param = device.setup().get_initial_param_val(this->name(),val);
}

param_str_t::~param_str_t()
{
}

void param_str_t::changed()
{
}

param_ptr_t::param_ptr_t(device_t &device, const pstring &name, uint8_t * val)
: param_t(device, name)
{
	m_param = val; //device.setup().get_initial_param_val(this->name(),val);
	//netlist().save(*this, m_param, "m_param");
}

void param_model_t::changed()
{
	netlist().log().fatal(MF_1_MODEL_1_CAN_NOT_BE_CHANGED_AT_RUNTIME, name());
	m_map.clear();
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


std::unique_ptr<plib::pistream> param_data_t::stream()
{
	return device().netlist().setup().get_data_stream(Value());
}

	bool detail::core_terminal_t::is_logic() const NL_NOEXCEPT
	{
		return dynamic_cast<const logic_t *>(this) != nullptr;
	}

	bool detail::core_terminal_t::is_analog() const NL_NOEXCEPT
	{
		return dynamic_cast<const analog_t *>(this) != nullptr;
	}

	bool detail::net_t::is_logic() const NL_NOEXCEPT
	{
		return dynamic_cast<const logic_net_t *>(this) != nullptr;
	}

	bool detail::net_t::is_analog() const NL_NOEXCEPT
	{
		return dynamic_cast<const analog_net_t *>(this) != nullptr;
	}


} // namespace netlist
