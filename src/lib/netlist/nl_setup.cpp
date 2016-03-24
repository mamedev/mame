// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.c
 *
 */

#include <cstdio>

#include "solver/nld_solver.h"

#include "plib/palloc.h"
#include "nl_base.h"
#include "nl_setup.h"
#include "nl_parser.h"
#include "nl_util.h"
#include "nl_factory.h"
#include "devices/net_lib.h"
#include "devices/nld_system.h"
#include "analog/nld_twoterm.h"

static NETLIST_START(base)
	TTL_INPUT(ttlhigh, 1)
	TTL_INPUT(ttllow, 0)
	NET_REGISTER_DEV(GND, GND)
	NET_REGISTER_DEV(PARAMETER, NETLIST)

	LOCAL_SOURCE(diode_models)
	LOCAL_SOURCE(bjt_models)
	LOCAL_SOURCE(family_models)
	LOCAL_SOURCE(TTL74XX_lib)
	LOCAL_SOURCE(CD4XXX_lib)
	LOCAL_SOURCE(OPAMP_lib)
	LOCAL_SOURCE(otheric_lib)

	INCLUDE(diode_models);
	INCLUDE(bjt_models);
	INCLUDE(family_models);
	INCLUDE(TTL74XX_lib);
	INCLUDE(CD4XXX_lib);
	INCLUDE(OPAMP_lib);
	INCLUDE(otheric_lib);

NETLIST_END()


// ----------------------------------------------------------------------------------------
// setup_t
// ----------------------------------------------------------------------------------------

namespace netlist
{
setup_t::setup_t(netlist_t *netlist)
	: m_netlist(netlist)
	, m_proxy_cnt(0)
	, m_frontier_cnt(0)
{
	netlist->set_setup(this);
	m_factory = palloc(factory_list_t(*this));
}

void setup_t::init()
{
	initialize_factory(factory());
	NETLIST_NAME(base)(*this);
}


setup_t::~setup_t()
{
	m_links.clear();
	m_alias.clear();
	m_params.clear();
	m_terminals.clear();
	m_params_temp.clear();

	netlist().set_setup(NULL);
	pfree(m_factory);
	m_sources.clear_and_free();

	pstring::resetmem();
}

ATTR_COLD pstring setup_t::build_fqn(const pstring &obj_name) const
{
	if (m_stack.empty())
		return netlist().name() + "." + obj_name;
	else
		return m_stack.top() + "." + obj_name;
}

void setup_t::namespace_push(const pstring &aname)
{
	if (m_stack.empty())
		m_stack.push(netlist().name() + "." + aname);
	else
		m_stack.push(m_stack.top() + "." + aname);
}

void setup_t::namespace_pop()
{
	m_stack.pop();
}


device_t *setup_t::register_dev(device_t *dev, const pstring &name)
{
	pstring fqn = build_fqn(name);

	dev->init(netlist(), fqn);

	for (auto & d : netlist().m_devices)
		if (d->name() == dev->name())
			log().fatal("Error adding {1} to device list. Duplicate name \n", name);

	netlist().m_devices.push_back(dev);
	return dev;
}

void setup_t::register_lib_entry(const pstring &name)
{
	if (m_lib.contains(name))
		log().warning("Lib entry collection already contains {1}. IGNORED", name);
	else
		m_lib.push_back(name);
}

device_t *setup_t::register_dev(const pstring &classname, const pstring &name)
{
	if (m_lib.contains(classname))
	{
		namespace_push(name);
		include(classname);
		namespace_pop();
		return NULL;
	}
	else
	{
		device_t *dev = factory().new_device_by_name(classname);
		//device_t *dev = factory().new_device_by_classname(classname);
		if (dev == NULL)
			log().fatal("Class {1} not found!\n", classname);
		return register_dev(dev, name);
	}
}

void setup_t::register_model(const pstring &model_in)
{
	int pos = model_in.find(" ");
	if (pos < 0)
		log().fatal("Unable to parse model: {1}", model_in);
	pstring model = model_in.left(pos).trim().ucase();
	pstring def = model_in.substr(pos + 1).trim();
	if (!m_models.add(model, def))
		log().fatal("Model already exists: {1}", model_in);
}

void setup_t::register_alias_nofqn(const pstring &alias, const pstring &out)
{
	if (!m_alias.add(alias, out))
		log().fatal("Error adding alias {1} to alias list\n", alias);
}

void setup_t::register_alias(const pstring &alias, const pstring &out)
{
	pstring alias_fqn = build_fqn(alias);
	pstring out_fqn = build_fqn(out);
	register_alias_nofqn(alias_fqn, out_fqn);
}

void setup_t::register_dippins_arr(const pstring &terms)
{
	pstring_vector_t list(terms,", ");
	if (list.size() == 0 || (list.size() % 2) == 1)
		log().fatal("You must pass an equal number of pins to DIPPINS");
	unsigned n = list.size();
	for (unsigned i = 0; i < n / 2; i++)
	{
		register_alias(pfmt("{1}")(i+1), list[i * 2]);
		register_alias(pfmt("{1}")(n-i), list[i * 2 + 1]);
	}
}

pstring setup_t::objtype_as_astr(object_t &in) const
{
	switch (in.type())
	{
		case terminal_t::TERMINAL:
			return "TERMINAL";
		case terminal_t::INPUT:
			return "INPUT";
		case terminal_t::OUTPUT:
			return "OUTPUT";
		case terminal_t::NET:
			return "NET";
		case terminal_t::PARAM:
			return "PARAM";
		case terminal_t::DEVICE:
			return "DEVICE";
		case terminal_t::NETLIST:
			return "NETLIST";
		case terminal_t::QUEUE:
			return "QUEUE";
	}
	// FIXME: noreturn
	log().fatal("Unknown object type {1}\n", (unsigned) in.type());
	return "Error";
}


void setup_t::register_object(device_t &dev, const pstring &name, object_t &obj)
{
	switch (obj.type())
	{
		case terminal_t::TERMINAL:
		case terminal_t::INPUT:
		case terminal_t::OUTPUT:
			{
				core_terminal_t &term = dynamic_cast<core_terminal_t &>(obj);
				if (obj.isType(terminal_t::OUTPUT))
				{
					if (obj.isFamily(terminal_t::LOGIC))
						dynamic_cast<logic_output_t &>(term).init_object(dev, dev.name() + "." + name);
					else if (obj.isFamily(terminal_t::ANALOG))
						dynamic_cast<analog_output_t &>(term).init_object(dev, dev.name() + "." + name);
					else
						log().fatal("Error adding {1} {2} to terminal list, neither LOGIC nor ANALOG\n", objtype_as_astr(term), term.name());
				}
				else
					term.init_object(dev, dev.name() + "." + name);

				if (!m_terminals.add(term.name(), &term))
					log().fatal("Error adding {1} {2} to terminal list\n", objtype_as_astr(term), term.name());
				log().debug("{1} {2}\n", objtype_as_astr(term), name);
			}
			break;
		case terminal_t::NET:
			break;
		case terminal_t::PARAM:
			{
				param_t &param = dynamic_cast<param_t &>(obj);
				if (m_params_temp.contains(name))
				{
					const pstring val = m_params_temp[name];
					switch (param.param_type())
					{
						case param_t::DOUBLE:
						{
							log().debug("Found parameter ... {1} : {1}\n", name, val);
							double vald = 0;
							if (sscanf(val.cstr(), "%lf", &vald) != 1)
								log().fatal("Invalid number conversion {1} : {2}\n", name, val);
							dynamic_cast<param_double_t &>(param).initial(vald);
						}
						break;
						case param_t::INTEGER:
						case param_t::LOGIC:
						{
							log().debug("Found parameter ... {1} : {2}\n", name, val);
							double vald = 0;
							if (sscanf(val.cstr(), "%lf", &vald) != 1)
								log().fatal("Invalid number conversion {1} : {2}\n", name, val);
							dynamic_cast<param_int_t &>(param).initial((int) vald);
						}
						break;
						case param_t::STRING:
						{
							dynamic_cast<param_str_t &>(param).initial(val);
						}
						break;
						case param_t::MODEL:
							//dynamic_cast<param_model_t &>(param).initial(val);
							dynamic_cast<param_model_t &>(param).initial(val);
							break;
						default:
							log().fatal("Parameter is not supported {1} : {2}\n", name, val);
					}
				}
				if (!m_params.add(param.name(), &param))
					log().fatal("Error adding parameter {1} to parameter list\n", name);
			}
			break;
		case terminal_t::DEVICE:
			log().fatal("Device registration not yet supported - {1}\n", name);
			break;
		case terminal_t::NETLIST:
			log().fatal("Netlist registration not yet supported - {1}\n", name);
			break;
		case terminal_t::QUEUE:
			log().fatal("QUEUE registration not yet supported - {1}\n", name);
			break;
	}
}

void setup_t::register_link_arr(const pstring &terms)
{
	pstring_vector_t list(terms,", ");
	if (list.size() < 2)
		log().fatal("You must pass at least 2 terminals to NET_C");
	for (std::size_t i = 1; i < list.size(); i++)
	{
		register_link(list[0], list[i]);
	}
}


void setup_t::register_link_fqn(const pstring &sin, const pstring &sout)
{
	link_t temp = link_t(sin, sout);
	log().debug("link {1} <== {2}\n", sin, sout);
	m_links.push_back(temp);
}

void setup_t::register_link(const pstring &sin, const pstring &sout)
{
	register_link_fqn(build_fqn(sin), build_fqn(sout));
}

void setup_t::remove_connections(const pstring pin)
{
	pstring pinfn = build_fqn(pin);
	bool found = false;

	for (int i = m_links.size() - 1; i >= 0; i--)
	{
		auto &link = m_links[i];
		if ((link.e1 == pinfn) || (link.e2 == pinfn))
		{
			log().verbose("removing connection: {1} <==> {2}\n", link.e1, link.e2);
			m_links.remove_at(i);
			found = true;
		}
	}
	if (!found)
		log().fatal("remove_connections: found no occurrence of {1}\n", pin);
}


void setup_t::register_frontier(const pstring attach, const double r_IN, const double r_OUT)
{
	pstring frontier_name = pfmt("frontier_{1}")(m_frontier_cnt);
	m_frontier_cnt++;
	device_t *front = register_dev("FRONTIER_DEV", frontier_name);
	register_param(frontier_name + ".RIN", r_IN);
	register_param(frontier_name + ".ROUT", r_OUT);
	register_link(frontier_name + ".G", "GND");
	pstring attfn = build_fqn(attach);
	bool found = false;
	for (auto & link  : m_links)
	{
		if (link.e1 == attfn)
		{
			link.e1 = front->name() + ".I";
			found = true;
		}
		else if (link.e2 == attfn)
		{
			link.e2 = front->name() + ".I";
			found = true;
		}
	}
	if (!found)
		log().fatal("Frontier setup: found no occurrence of {1}\n", attach);
	register_link(attach, frontier_name + ".Q");
}


void setup_t::register_param(const pstring &param, const double value)
{
	// FIXME: there should be a better way
	register_param(param, pfmt("{1}").e(value,".9"));
}

void setup_t::register_param(const pstring &param, const pstring &value)
{
	pstring fqn = build_fqn(param);

	int idx = m_params_temp.index_of(fqn);
	if (idx < 0)
	{
		if (!m_params_temp.add(fqn, value))
			log().fatal("Unexpected error adding parameter {1} to parameter list\n", param);
	}
	else
	{
		log().warning("Overwriting {1} old <{2}> new <{3}>\n", fqn, m_params_temp.value_at(idx), value);
		m_params_temp[fqn] = value;
	}
}

const pstring setup_t::resolve_alias(const pstring &name) const
{
	pstring temp = name;
	pstring ret;

	/* FIXME: Detect endless loop */
	do {
		ret = temp;
		int p = m_alias.index_of(ret);
		temp = (p>=0 ? m_alias.value_at(p) : "");
	} while (temp != "");

	log().debug("{1}==>{2}\n", name, ret);
	return ret;
}

core_terminal_t *setup_t::find_terminal(const pstring &terminal_in, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	int ret;

	ret = m_terminals.index_of(tname);
	/* look for default */
	if (ret < 0)
	{
		/* look for ".Q" std output */
		ret = m_terminals.index_of(tname + ".Q");
	}

	core_terminal_t *term = (ret < 0 ? NULL : m_terminals.value_at(ret));

	if (term == NULL && required)
		log().fatal("terminal {1}({2}) not found!\n", terminal_in, tname);
	if (term != NULL)
		log().debug("Found input {1}\n", tname);
	return term;
}

core_terminal_t *setup_t::find_terminal(const pstring &terminal_in, object_t::type_t atype, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	int ret;

	ret = m_terminals.index_of(tname);
	/* look for default */
	if (ret < 0 && atype == object_t::OUTPUT)
	{
		/* look for ".Q" std output */
		ret = m_terminals.index_of(tname + ".Q");
	}
	if (ret < 0 && required)
		log().fatal("terminal {1}({2}) not found!\n", terminal_in, tname);

	core_terminal_t *term = (ret < 0 ? NULL : m_terminals.value_at(ret));

	if (term != NULL && term->type() != atype)
	{
		if (required)
			log().fatal("object {1}({2}) found but wrong type\n", terminal_in, tname);
		else
			term = NULL;
	}
	if (term != NULL)
		log().debug("Found input {1}\n", tname);

	return term;
}

param_t *setup_t::find_param(const pstring &param_in, bool required)
{
	const pstring param_in_fqn = build_fqn(param_in);

	const pstring &outname = resolve_alias(param_in_fqn);
	int ret;

	ret = m_params.index_of(outname);
	if (ret < 0 && required)
		log().fatal("parameter {1}({2}) not found!\n", param_in_fqn, outname);
	if (ret != -1)
		log().debug("Found parameter {1}\n", outname);
	return (ret == -1 ? NULL : m_params.value_at(ret));
}

// FIXME avoid dynamic cast here
devices::nld_base_proxy *setup_t::get_d_a_proxy(core_terminal_t &out)
{
	nl_assert(out.isFamily(terminal_t::LOGIC));

	logic_output_t &out_cast = dynamic_cast<logic_output_t &>(out);
	devices::nld_base_proxy *proxy = out_cast.get_proxy();

	if (proxy == NULL)
	{
		// create a new one ...
		devices::nld_base_d_to_a_proxy *new_proxy = out_cast.logic_family()->create_d_a_proxy(&out_cast);
		pstring x = pfmt("proxy_da_{1}_{2}")(out.name())(m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(new_proxy, x);
		new_proxy->start_dev();

		/* connect all existing terminals to new net */

		for (core_terminal_t *p : out.net().m_core_terms)
		{
			p->clear_net(); // de-link from all nets ...
			if (!connect(new_proxy->proxy_term(), *p))
				log().fatal("Error connecting {1} to {2}\n", new_proxy->proxy_term().name(), (*p).name());
		}
		out.net().m_core_terms.clear(); // clear the list

		out.net().register_con(new_proxy->in());
		out_cast.set_proxy(new_proxy);
		proxy = new_proxy;
	}
	return proxy;
}

void setup_t::connect_input_output(core_terminal_t &in, core_terminal_t &out)
{
	if (out.isFamily(terminal_t::ANALOG) && in.isFamily(terminal_t::LOGIC))
	{
		logic_input_t &incast = dynamic_cast<logic_input_t &>(in);
		devices::nld_a_to_d_proxy *proxy = palloc(devices::nld_a_to_d_proxy(&incast));
		incast.set_proxy(proxy);
		pstring x = pfmt("proxy_ad_{1}_{2}")(in.name())( m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(proxy, x);
		proxy->start_dev();

		proxy->m_Q.net().register_con(in);
		out.net().register_con(proxy->m_I);

	}
	else if (out.isFamily(terminal_t::LOGIC) && in.isFamily(terminal_t::ANALOG))
	{
		devices::nld_base_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->proxy_term(), in);
		//proxy->out().net().register_con(in);
	}
	else
	{
		if (in.has_net())
			out.net().merge_net(&in.net());
		else
			out.net().register_con(in);
	}
}


void setup_t::connect_terminal_input(terminal_t &term, core_terminal_t &inp)
{
	if (inp.isFamily(terminal_t::ANALOG))
	{
		connect_terminals(inp, term);
	}
	else if (inp.isFamily(terminal_t::LOGIC))
	{
		logic_input_t &incast = dynamic_cast<logic_input_t &>(inp);
		log().debug("connect_terminal_input: connecting proxy\n");
		devices::nld_a_to_d_proxy *proxy = palloc(devices::nld_a_to_d_proxy(&incast));
		incast.set_proxy(proxy);
		pstring x = pfmt("proxy_ad_{1}_{2}")(inp.name())(m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(proxy, x);
		proxy->start_dev();

		connect_terminals(term, proxy->m_I);

		if (inp.has_net())
			//fatalerror("logic inputs can only belong to one net!\n");
			proxy->m_Q.net().merge_net(&inp.net());
		else
			proxy->m_Q.net().register_con(inp);
	}
	else
	{
		log().fatal("Netlist: Severe Error");
	}
}

void setup_t::connect_terminal_output(terminal_t &in, core_terminal_t &out)
{
	if (out.isFamily(terminal_t::ANALOG))
	{
		log().debug("connect_terminal_output: {1} {2}\n", in.name(), out.name());
		/* no proxy needed, just merge existing terminal net */
		if (in.has_net())
			out.net().merge_net(&in.net());
		else
			out.net().register_con(in);
	}
	else if (out.isFamily(terminal_t::LOGIC))
	{
		log().debug("connect_terminal_output: connecting proxy\n");
		devices::nld_base_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->proxy_term(), in);
	}
	else
	{
		log().fatal("Netlist: Severe Error");
	}
}

void setup_t::connect_terminals(core_terminal_t &t1, core_terminal_t &t2)
{
	if (t1.has_net() && t2.has_net())
	{
		log().debug("T2 and T1 have net\n");
		t1.net().merge_net(&t2.net());
	}
	else if (t2.has_net())
	{
		log().debug("T2 has net\n");
		t2.net().register_con(t1);
	}
	else if (t1.has_net())
	{
		log().debug("T1 has net\n");
		t1.net().register_con(t2);
	}
	else
	{
		log().debug("adding net ...\n");
		analog_net_t *anet =  palloc(analog_net_t);
		t1.set_net(*anet);
		// FIXME: Nets should have a unique name
		t1.net().init_object(netlist(),"net." + t1.name() );
		t1.net().register_con(t2);
		t1.net().register_con(t1);
	}
}

static core_terminal_t &resolve_proxy(core_terminal_t &term)
{
	if (term.isFamily(core_terminal_t::LOGIC))
	{
		logic_t &out = dynamic_cast<logic_t &>(term);
		if (out.has_proxy())
			return out.get_proxy()->proxy_term();
	}
	return term;
}

bool setup_t::connect_input_input(core_terminal_t &t1, core_terminal_t &t2)
{
	bool ret = false;
	if (t1.has_net())
	{
		if (t1.net().isRailNet())
			ret = connect(t2, t1.net().railterminal());
		if (!ret)
		{
			for (core_terminal_t *t : t1.net().m_core_terms)
			{
				if (t->isType(core_terminal_t::TERMINAL))
					ret = connect(t2, *t);
				if (ret)
					break;
			}
		}
	}
	if (!ret && t2.has_net())
	{
		if (t2.net().isRailNet())
			ret = connect(t1, t2.net().railterminal());
		if (!ret)
		{
			for (core_terminal_t *t : t2.net().m_core_terms)
			{
				if (t->isType(core_terminal_t::TERMINAL))
					ret = connect(t1, *t);
				if (ret)
					break;
			}
		}
	}
	return ret;
}



bool setup_t::connect(core_terminal_t &t1_in, core_terminal_t &t2_in)
{
	log().debug("Connecting {1} to {2}\n", t1_in.name(), t2_in.name());
	core_terminal_t &t1 = resolve_proxy(t1_in);
	core_terminal_t &t2 = resolve_proxy(t2_in);
	bool ret = true;

	if (t1.isType(core_terminal_t::OUTPUT) && t2.isType(core_terminal_t::INPUT))
	{
		if (t2.has_net() && t2.net().isRailNet())
			log().fatal("Input {1} already connected\n", t2.name());
		connect_input_output(t2, t1);
	}
	else if (t1.isType(core_terminal_t::INPUT) && t2.isType(core_terminal_t::OUTPUT))
	{
		if (t1.has_net()  && t1.net().isRailNet())
			log().fatal("Input {1} already connected\n", t1.name());
		connect_input_output(t1, t2);
	}
	else if (t1.isType(core_terminal_t::OUTPUT) && t2.isType(core_terminal_t::TERMINAL))
	{
		connect_terminal_output(dynamic_cast<terminal_t &>(t2), t1);
	}
	else if (t1.isType(core_terminal_t::TERMINAL) && t2.isType(core_terminal_t::OUTPUT))
	{
		connect_terminal_output(dynamic_cast<terminal_t &>(t1), t2);
	}
	else if (t1.isType(core_terminal_t::INPUT) && t2.isType(core_terminal_t::TERMINAL))
	{
		connect_terminal_input(dynamic_cast<terminal_t &>(t2), t1);
	}
	else if (t1.isType(core_terminal_t::TERMINAL) && t2.isType(core_terminal_t::INPUT))
	{
		connect_terminal_input(dynamic_cast<terminal_t &>(t1), t2);
	}
	else if (t1.isType(core_terminal_t::TERMINAL) && t2.isType(core_terminal_t::TERMINAL))
	{
		connect_terminals(dynamic_cast<terminal_t &>(t1), dynamic_cast<terminal_t &>(t2));
	}
	else if (t1.isType(core_terminal_t::INPUT) && t2.isType(core_terminal_t::INPUT))
	{
		ret = connect_input_input(t1, t2);
	}
	else
		ret = false;
		//netlist().error("Connecting {1} to {2} not supported!\n", t1.name(), t2.name());
	return ret;
}

void setup_t::resolve_inputs()
{
	bool has_twoterms = false;

	log().verbose("Resolving inputs ...");

	/* Netlist can directly connect input to input.
	 * We therefore first park connecting inputs and retry
	 * after all other terminals were connected.
	 */
	int tries = 100;
	while (m_links.size() > 0 && tries >  0) // FIXME: convert into constant
	{
		unsigned li = 0;
		while (li < m_links.size())
		{
			const pstring t1s = m_links[li].e1;
			const pstring t2s = m_links[li].e2;
			core_terminal_t *t1 = find_terminal(t1s);
			core_terminal_t *t2 = find_terminal(t2s);

			if (connect(*t1, *t2))
				m_links.remove_at(li);
			else
				li++;
		}
		tries--;
	}
	if (tries == 0)
	{
		for (std::size_t i = 0; i < m_links.size(); i++ )
			log().warning("Error connecting {1} to {2}\n", m_links[i].e1, m_links[i].e2);

		log().fatal("Error connecting -- bailing out\n");
	}

	log().verbose("deleting empty nets ...");

	// delete empty nets ... and save m_list ...

	net_t::list_t todelete;

	for (net_t *net : netlist().m_nets)
	{
		if (net->num_cons() == 0)
			todelete.push_back(net);
		else
			net->rebuild_list();
	}

	for (net_t *net : todelete)
	{
		log().verbose("Deleting net {1} ...", net->name());
		netlist().m_nets.remove(net);
		if (!net->isRailNet())
			pfree(net);
	}

	pstring errstr("");

	log().verbose("looking for terminals not connected ...");
	for (std::size_t i = 0; i < m_terminals.size(); i++)
	{
		core_terminal_t *term = m_terminals.value_at(i);
		if (!term->has_net() && dynamic_cast< devices::NETLIB_NAME(dummy_input) *>(&term->device()) != NULL)
			log().warning("Found dummy terminal {1} without connections", term->name());
		else if (!term->has_net())
			errstr += pfmt("Found terminal {1} without a net\n")(term->name());
		else if (term->net().num_cons() == 0)
			log().warning("Found terminal {1} without connections", term->name());
	}
	if (errstr != "")
		log().fatal("{1}", errstr);


	log().verbose("looking for two terms connected to rail nets ...\n");
	// FIXME: doesn't find internal devices. This needs to be more clever
	for (std::size_t i=0; i < netlist().m_devices.size(); i++)
	{
		devices::NETLIB_NAME(twoterm) *t = dynamic_cast<devices::NETLIB_NAME(twoterm) *>(netlist().m_devices[i]);
		if (t != NULL)
		{
			has_twoterms = true;
			if (t->m_N.net().isRailNet() && t->m_P.net().isRailNet())
				log().warning("Found device {1} connected only to railterminals {2}/{3}\n",
					t->name(), t->m_N.net().name(), t->m_P.net().name());
		}
	}

	log().verbose("initialize solver ...\n");

	if (netlist().solver() == NULL)
	{
		if (has_twoterms)
			log().fatal("No solver found for this net although analog elements are present\n");
	}
	else
		netlist().solver()->post_start();

}

void setup_t::start_devices()
{
	pstring env = nl_util::environment("NL_LOGS");

	if (env != "")
	{
		log().debug("Creating dynamic logs ...\n");
		pstring_vector_t loglist(env, ":");
		for (pstring ll : loglist)
		{
			device_t *nc = factory().new_device_by_name("LOG");
			pstring name = "log_" + ll;
			register_dev(nc, name);
			register_link(name + ".I", ll);
			log().debug("    dynamic link {1}: <{2}>\n",ll, name);
		}
	}

	netlist().start();
}

void setup_t::print_stats() const
{
#if (NL_KEEP_STATISTICS)
	{
		for (std::size_t i = 0; i < netlist().m_started_devices.size(); i++)
		{
			core_device_t *entry = netlist().m_started_devices[i];
			printf("Device %20s : %12d %12d %15ld\n", entry->name(), entry->stat_call_count, entry->stat_update_count, (long int) entry->stat_total_time / (entry->stat_update_count + 1));
		}
		printf("Queue Pushes %15d\n", netlist().queue().m_prof_call);
		printf("Queue Moves  %15d\n", netlist().queue().m_prof_sortmove);
	}
#endif
}

// ----------------------------------------------------------------------------------------
// Model / family
// ----------------------------------------------------------------------------------------

class logic_family_std_proxy_t : public logic_family_desc_t
{
public:
	logic_family_std_proxy_t() { }
	virtual devices::nld_base_d_to_a_proxy *create_d_a_proxy(logic_output_t *proxied) const override
	{
		return palloc(devices::nld_d_to_a_proxy(proxied));
	}
};

logic_family_desc_t *setup_t::family_from_model(const pstring &model)
{
	model_map_t map;
	model_parse(model, map);

	if (setup_t::model_value_str(map, "TYPE") == "TTL")
		return family_TTL;
	if (setup_t::model_value_str(map, "TYPE") == "CD4XXX")
		return family_CD4XXX;

	logic_family_std_proxy_t *ret = palloc(logic_family_std_proxy_t);

	ret->m_low_thresh_V = setup_t::model_value(map, "IVL");
	ret->m_high_thresh_V = setup_t::model_value(map, "IVH");
	ret->m_low_V = setup_t::model_value(map, "OVL");
	ret->m_high_V = setup_t::model_value(map, "OVH");
	ret->m_R_low = setup_t::model_value(map, "ORL");
	ret->m_R_high = setup_t::model_value(map, "ORH");

	return ret;
}

static pstring model_string(model_map_t &map)
{
	pstring ret = map["COREMODEL"] + "(";
	for (unsigned i=0; i<map.size(); i++)
		ret = ret + map.key_at(i) + "=" + map.value_at(i) + " ";

	return ret + ")";
}


void setup_t::model_parse(const pstring &model_in, model_map_t &map)
{
	pstring model = model_in;
	int pos = 0;
	pstring key;

	while (true)
	{
		pos = model.find("(");
		if (pos >= 0) break;

		key = model.ucase();
		if (!m_models.contains(key))
			log().fatal("Model {1} not found\n", model);
		model = m_models[key];
	}
	pstring xmodel = model.left(pos);

	if (xmodel.equals("_"))
		map["COREMODEL"] = key;
	else
	{
		if (m_models.contains(xmodel))
			model_parse(xmodel, map);
		else
			log().fatal("Model doesn't exist {1}\n", xmodel);
	}

	pstring remainder=model.substr(pos+1).trim();
	if (!remainder.endsWith(")"))
		log().fatal("Model error {1}\n", model);
	remainder = remainder.left(remainder.len() - 1);

	pstring_vector_t pairs(remainder," ", true);
	for (pstring &pe : pairs)
	{
		int pose = pe.find("=");
		if (pose < 0)
			log().fatal("Model error on pair {1}\n", model);
		map[pe.left(pose).ucase()] = pe.substr(pose+1);
	}
}

const pstring setup_t::model_value_str(model_map_t &map, const pstring &entity)
{
	pstring ret;

	if (entity != entity.ucase())
		log().fatal("model parameters should be uppercase:{1} {2}\n", entity, model_string(map));
	if (!map.contains(entity))
		log().fatal("Entity {1} not found in model {2}\n", entity, model_string(map));
	else
		ret = map[entity];

	return ret;
}

nl_double setup_t::model_value(model_map_t &map, const pstring &entity)
{
	pstring tmp = model_value_str(map, entity);

	nl_double factor = NL_FCONST(1.0);
	pstring numfac = tmp.right(1);
	switch (numfac.code_at(0))
	{
		case 'M': factor = 1e6; break;
		case 'k': factor = 1e3; break;
		case 'm': factor = 1e-3; break;
		case 'u': factor = 1e-6; break;
		case 'n': factor = 1e-9; break;
		case 'p': factor = 1e-12; break;
		case 'f': factor = 1e-15; break;
		case 'a': factor = 1e-18; break;
		default:
			if (numfac < "0" || numfac > "9")
				fatalerror_e(pfmt("Unknown number factor <{1}> in: {2}")(numfac)(entity));
	}
	if (factor != NL_FCONST(1.0))
		tmp = tmp.left(tmp.len() - 1);
	return tmp.as_double() * factor;
}

// ----------------------------------------------------------------------------------------
// Sources
// ----------------------------------------------------------------------------------------

void setup_t::include(const pstring &netlist_name)
{
	for (source_t *source : m_sources)
	{
		if (source->parse(*this, netlist_name))
			return;
	}
	log().fatal("unable to find {1} in source collection", netlist_name);
}

// ----------------------------------------------------------------------------------------
// base sources
// ----------------------------------------------------------------------------------------

bool source_string_t::parse(setup_t &setup, const pstring &name)
{
	pimemstream istrm(m_str.cstr(), m_str.len());
	pomemstream ostrm;

	pimemstream istrm2(ppreprocessor().process(istrm, ostrm));
	return parser_t(istrm2, setup).parse(name);
}

bool source_mem_t::parse(setup_t &setup, const pstring &name)
{
	pimemstream istrm(m_str.cstr(), m_str.len());
	pomemstream ostrm;

	pimemstream istrm2(ppreprocessor().process(istrm, ostrm));
	return parser_t(istrm2, setup).parse(name);
}

bool source_file_t::parse(setup_t &setup, const pstring &name)
{
	pifilestream istrm(m_filename);
	pomemstream ostrm;

	pimemstream istrm2(ppreprocessor().process(istrm, ostrm));
	return parser_t(istrm2, setup).parse(name);
}

}
