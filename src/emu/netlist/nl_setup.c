// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.c
 *
 */

#include <solver/nld_solver.h>
#include <cstdio>

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
		return m_stack.peek() + "." + obj_name;
}

void setup_t::namespace_push(const pstring &aname)
{
	if (m_stack.empty())
		m_stack.push(netlist().name() + "." + aname);
	else
		m_stack.push(m_stack.peek() + "." + aname);
}

void setup_t::namespace_pop()
{
	m_stack.pop();
}


device_t *setup_t::register_dev(device_t *dev, const pstring &name)
{
	pstring fqn = build_fqn(name);

	dev->init(netlist(), fqn);

	if (!(netlist().m_devices.add(dev, false)==true))
		netlist().error("Error adding %s to device list\n", name.cstr());
	return dev;
}

void setup_t::register_lib_entry(const pstring &name)
{
	if (m_lib.contains(name))
		netlist().warning("Lib entry collection already contains %s. IGNORED", name.cstr());
	else
		m_lib.add(name);
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
			netlist().error("Class %s not found!\n", classname.cstr());
		return register_dev(dev, name);
	}
}

void setup_t::register_model(const pstring &model_in)
{
	int pos = model_in.find(' ');
	if (pos < 0)
		netlist().error("Unable to parse model: %s", model_in.cstr());
	pstring model = model_in.left(pos).trim().ucase();
	pstring def = model_in.substr(pos + 1).trim();
	if (!m_models.add(model, def))
		netlist().error("Model already exists: %s", model_in.cstr());
}

void setup_t::register_alias_nofqn(const pstring &alias, const pstring &out)
{
	if (!m_alias.add(alias, out))
		netlist().error("Error adding alias %s to alias list\n", alias.cstr());
}

void setup_t::register_alias(const pstring &alias, const pstring &out)
{
	pstring alias_fqn = build_fqn(alias);
	pstring out_fqn = build_fqn(out);
	register_alias_nofqn(alias_fqn, out_fqn);
}

void setup_t::register_dippins_arr(const pstring &terms)
{
	pstring_list_t list(terms,", ");
	if (list.size() == 0 || (list.size() % 2) == 1)
		netlist().error("You must pass an equal number of pins to DIPPINS");
	unsigned n = list.size();
	for (unsigned i = 0; i < n / 2; i++)
	{
		register_alias(pstring::sprintf("%d", i+1), list[i * 2]);
		register_alias(pstring::sprintf("%d", n-i), list[i * 2 + 1]);
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
	netlist().error("Unknown object type %d\n", in.type());
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
						netlist().error("Error adding %s %s to terminal list, neither LOGIC nor ANALOG\n", objtype_as_astr(term).cstr(), term.name().cstr());
				}
				else
					term.init_object(dev, dev.name() + "." + name);

				if (!m_terminals.add(term.name(), &term))
					netlist().error("Error adding %s %s to terminal list\n", objtype_as_astr(term).cstr(), term.name().cstr());
				NL_VERBOSE_OUT(("%s %s\n", objtype_as_astr(term).cstr(), name.cstr()));
			}
			break;
		case terminal_t::NET:
			break;
		case terminal_t::PARAM:
			{
				param_t &param = dynamic_cast<param_t &>(obj);
				//printf("name: %s\n", name.cstr());
				if (m_params_temp.contains(name))
				{
					const pstring val = m_params_temp[name];
					switch (param.param_type())
					{
						case param_t::DOUBLE:
						{
							NL_VERBOSE_OUT(("Found parameter ... %s : %s\n", name.cstr(), val.cstr()));
							double vald = 0;
							if (std::sscanf(val.cstr(), "%lf", &vald) != 1)
								netlist().error("Invalid number conversion %s : %s\n", name.cstr(), val.cstr());
							dynamic_cast<param_double_t &>(param).initial(vald);
						}
						break;
						case param_t::INTEGER:
						case param_t::LOGIC:
						{
							NL_VERBOSE_OUT(("Found parameter ... %s : %s\n", name.cstr(), val.cstr()));
							double vald = 0;
							if (std::sscanf(val.cstr(), "%lf", &vald) != 1)
								netlist().error("Invalid number conversion %s : %s\n", name.cstr(), val.cstr());
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
							netlist().error("Parameter is not supported %s : %s\n", name.cstr(), val.cstr());
					}
				}
				if (!m_params.add(param.name(), &param))
					netlist().error("Error adding parameter %s to parameter list\n", name.cstr());
			}
			break;
		case terminal_t::DEVICE:
			netlist().error("Device registration not yet supported - %s\n", name.cstr());
			break;
		case terminal_t::NETLIST:
			netlist().error("Netlist registration not yet supported - %s\n", name.cstr());
			break;
		case terminal_t::QUEUE:
			netlist().error("QUEUE registration not yet supported - %s\n", name.cstr());
			break;
	}
}

void setup_t::register_link_arr(const pstring &terms)
{
	pstring_list_t list(terms,", ");
	if (list.size() < 2)
		netlist().error("You must pass at least 2 terminals to NET_C");
	for (std::size_t i = 1; i < list.size(); i++)
	{
		register_link(list[0], list[i]);
	}
}


void setup_t::register_link_fqn(const pstring &sin, const pstring &sout)
{
	link_t temp = link_t(sin, sout);
	NL_VERBOSE_OUT(("link %s <== %s\n", sin.cstr(), sout.cstr()));
	m_links.add(temp);
}

void setup_t::register_link(const pstring &sin, const pstring &sout)
{
	register_link_fqn(build_fqn(sin), build_fqn(sout));
}

void setup_t::remove_connections(const pstring pin)
{
	pstring pinfn = build_fqn(pin);
	bool found = false;
	for (std::size_t i = 0; i < m_links.size(); i++)
	{
		if ((m_links[i].e1 == pinfn) || (m_links[i].e2 == pinfn))
		{
			netlist().log("removing connection: %s <==> %s\n", m_links[i].e1.cstr(), m_links[i].e2.cstr());
			m_links.remove_at(i);
			found = true;
		}
	}
	if (!found)
		netlist().error("remove_connections: found no occurrence of %s\n", pin.cstr());
}


void setup_t::register_frontier(const pstring attach, const double r_IN, const double r_OUT)
{
	static int frontier_cnt = 0;
	pstring frontier_name = pstring::sprintf("frontier_%d", frontier_cnt);
	frontier_cnt++;
	device_t *front = register_dev("FRONTIER_DEV", frontier_name);
	register_param(frontier_name + ".RIN", r_IN);
	register_param(frontier_name + ".ROUT", r_OUT);
	register_link(frontier_name + ".G", "GND");
	pstring attfn = build_fqn(attach);
	bool found = false;
	for (std::size_t i = 0; i < m_links.size(); i++)
	{
		if (m_links[i].e1 == attfn)
		{
			m_links[i].e1 = front->name() + ".I";
			found = true;
		}
		else if (m_links[i].e2 == attfn)
		{
			m_links[i].e2 = front->name() + ".I";
			found = true;
		}
	}
	if (!found)
		netlist().error("Frontier setup: found no occurrence of %s\n", attach.cstr());
	register_link(attach, frontier_name + ".Q");
}


void setup_t::register_param(const pstring &param, const double value)
{
	// FIXME: there should be a better way
	register_param(param, pstring::sprintf("%.9e", value));
}

void setup_t::register_param(const pstring &param, const pstring &value)
{
	pstring fqn = build_fqn(param);

	int idx = m_params_temp.index_of(fqn);
	if (idx < 0)
	{
		if (!m_params_temp.add(fqn, value))
			netlist().error("Unexpected error adding parameter %s to parameter list\n", param.cstr());
	}
	else
	{
		netlist().warning("Overwriting %s old <%s> new <%s>\n", fqn.cstr(), m_params_temp.value_at(idx).cstr(), value.cstr());
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

	NL_VERBOSE_OUT(("%s==>%s\n", name.cstr(), ret.cstr()));
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
		netlist().error("terminal %s(%s) not found!\n", terminal_in.cstr(), tname.cstr());
	if (term != NULL)
		NL_VERBOSE_OUT(("Found input %s\n", tname.cstr()));
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
		netlist().error("terminal %s(%s) not found!\n", terminal_in.cstr(), tname.cstr());

	core_terminal_t *term = (ret < 0 ? NULL : m_terminals.value_at(ret));

	if (term != NULL && term->type() != atype)
	{
		if (required)
			netlist().error("object %s(%s) found but wrong type\n", terminal_in.cstr(), tname.cstr());
		else
			term = NULL;
	}
	if (term != NULL)
		NL_VERBOSE_OUT(("Found input %s\n", tname.cstr()));

	return term;
}

param_t *setup_t::find_param(const pstring &param_in, bool required)
{
	const pstring param_in_fqn = build_fqn(param_in);

	const pstring &outname = resolve_alias(param_in_fqn);
	int ret;

	ret = m_params.index_of(outname);
	if (ret < 0 && required)
		netlist().error("parameter %s(%s) not found!\n", param_in_fqn.cstr(), outname.cstr());
	if (ret != -1)
		NL_VERBOSE_OUT(("Found parameter %s\n", outname.cstr()));
	return (ret == -1 ? NULL : m_params.value_at(ret));
}

// FIXME avoid dynamic cast here
devices::nld_base_proxy *setup_t::get_d_a_proxy(core_terminal_t &out)
{
	nl_assert(out.isFamily(terminal_t::LOGIC));

	//printf("proxy for %s\n", out.name().cstr());;
	logic_output_t &out_cast = dynamic_cast<logic_output_t &>(out);
	devices::nld_base_proxy *proxy = out_cast.get_proxy();

	if (proxy == NULL)
	{
		// create a new one ...
		devices::nld_base_d_to_a_proxy *new_proxy = out_cast.logic_family()->create_d_a_proxy(&out_cast);
		pstring x = pstring::sprintf("proxy_da_%s_%d", out.name().cstr(), m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(new_proxy, x);
		new_proxy->start_dev();

#if 1
		/* connect all existing terminals to new net */

		for (std::size_t i = 0; i < out.net().m_core_terms.size(); i++)
		{
			core_terminal_t *p = out.net().m_core_terms[i];
			p->clear_net(); // de-link from all nets ...
			if (!connect(new_proxy->proxy_term(), *p))
				netlist().error("Error connecting %s to %s\n", new_proxy->proxy_term().name().cstr(), (*p).name().cstr());
		}
		out.net().m_core_terms.clear(); // clear the list
#endif
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
		pstring x = pstring::sprintf("proxy_ad_%s_%d", in.name().cstr(), m_proxy_cnt);
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
		NL_VERBOSE_OUT(("connect_terminal_input: connecting proxy\n"));
		devices::nld_a_to_d_proxy *proxy = palloc(devices::nld_a_to_d_proxy(&incast));
		incast.set_proxy(proxy);
		pstring x = pstring::sprintf("proxy_ad_%s_%d", inp.name().cstr(), m_proxy_cnt);
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
		netlist().error("Netlist: Severe Error");
	}
}

void setup_t::connect_terminal_output(terminal_t &in, core_terminal_t &out)
{
	if (out.isFamily(terminal_t::ANALOG))
	{
		NL_VERBOSE_OUT(("connect_terminal_output: %s %s\n", in.name().cstr(), out.name().cstr()));
		/* no proxy needed, just merge existing terminal net */
		if (in.has_net())
			out.net().merge_net(&in.net());
		else
			out.net().register_con(in);
	}
	else if (out.isFamily(terminal_t::LOGIC))
	{
		NL_VERBOSE_OUT(("connect_terminal_output: connecting proxy\n"));
		devices::nld_base_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->proxy_term(), in);
	}
	else
	{
		netlist().error("Netlist: Severe Error");
	}
}

void setup_t::connect_terminals(core_terminal_t &t1, core_terminal_t &t2)
{
	//nl_assert(in.isType(netlist_terminal_t::TERMINAL));
	//nl_assert(out.isType(netlist_terminal_t::TERMINAL));

	if (t1.has_net() && t2.has_net())
	{
		NL_VERBOSE_OUT(("T2 and T1 have net\n"));
		t1.net().merge_net(&t2.net());
	}
	else if (t2.has_net())
	{
		NL_VERBOSE_OUT(("T2 has net\n"));
		t2.net().register_con(t1);
	}
	else if (t1.has_net())
	{
		NL_VERBOSE_OUT(("T1 has net\n"));
		t1.net().register_con(t2);
	}
	else
	{
		NL_VERBOSE_OUT(("adding net ...\n"));
		analog_net_t *anet =  palloc(analog_net_t);
		t1.set_net(*anet);
		//m_netlist.solver()->m_nets.add(anet);
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
			for (std::size_t i=0; i<t1.net().m_core_terms.size(); i++)
			{
				if (t1.net().m_core_terms[i]->isType(core_terminal_t::TERMINAL)
						/*|| t1.net().m_core_terms[i]->isType(netlist_core_terminal_t::OUTPUT)*/)
				{
					ret = connect(t2, *t1.net().m_core_terms[i]);
				}
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
			for (std::size_t i=0; i<t2.net().m_core_terms.size(); i++)
			{
				if (t2.net().m_core_terms[i]->isType(core_terminal_t::TERMINAL)
						/*|| t2.net().m_core_terms[i]->isType(netlist_core_terminal_t::OUTPUT)*/)
				{
					ret = connect(t1, *t2.net().m_core_terms[i]);
				}
				if (ret)
					break;
			}
		}
	}
	return ret;
}



bool setup_t::connect(core_terminal_t &t1_in, core_terminal_t &t2_in)
{
	NL_VERBOSE_OUT(("Connecting %s to %s\n", t1_in.name().cstr(), t2_in.name().cstr()));
	core_terminal_t &t1 = resolve_proxy(t1_in);
	core_terminal_t &t2 = resolve_proxy(t2_in);
	bool ret = true;

	if (t1.isType(core_terminal_t::OUTPUT) && t2.isType(core_terminal_t::INPUT))
	{
		if (t2.has_net() && t2.net().isRailNet())
			netlist().error("Input %s already connected\n", t2.name().cstr());
		connect_input_output(t2, t1);
	}
	else if (t1.isType(core_terminal_t::INPUT) && t2.isType(core_terminal_t::OUTPUT))
	{
		if (t1.has_net()  && t1.net().isRailNet())
			netlist().error("Input %s already connected\n", t1.name().cstr());
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
		//netlist().error("Connecting %s to %s not supported!\n", t1.name().cstr(), t2.name().cstr());
	return ret;
}

void setup_t::resolve_inputs()
{
	bool has_twoterms = false;

	netlist().log("Resolving inputs ...");

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
			netlist().warning("Error connecting %s to %s\n", m_links[i].e1.cstr(), m_links[i].e2.cstr());

		netlist().error("Error connecting -- bailing out\n");
	}

	netlist().log("deleting empty nets ...");

	// delete empty nets ... and save m_list ...

	net_t::list_t todelete;

	for (std::size_t i = 0; i<netlist().m_nets.size(); i++)
	{
		if (netlist().m_nets[i]->num_cons() == 0)
		{
			todelete.add(netlist().m_nets[i]);
		}
		else
		{
			netlist().m_nets[i]->rebuild_list();
		}
	}

	for (std::size_t i=0; i < todelete.size(); i++)
	{
		netlist().log("Deleting net %s ...", todelete[i]->name().cstr());
		netlist().m_nets.remove(todelete[i]);
		if (!todelete[i]->isRailNet())
			pfree(todelete[i]);
	}

	pstring errstr("");

	netlist().log("looking for terminals not connected ...");
	for (std::size_t i = 0; i < m_terminals.size(); i++)
	{
		core_terminal_t *term = m_terminals.value_at(i);
		if (!term->has_net())
			errstr += pstring::sprintf("Found terminal %s without a net\n",
					term->name().cstr());
		else if (term->net().num_cons() == 0)
			netlist().warning("Found terminal %s without connections",
					term->name().cstr());
	}
	if (errstr != "")
		netlist().error("%s", errstr.cstr());


	netlist().log("looking for two terms connected to rail nets ...\n");
	// FIXME: doesn't find internal devices. This needs to be more clever
	for (std::size_t i=0; i < netlist().m_devices.size(); i++)
	{
		devices::NETLIB_NAME(twoterm) *t = dynamic_cast<devices::NETLIB_NAME(twoterm) *>(netlist().m_devices[i]);
		if (t != NULL)
		{
			has_twoterms = true;
			if (t->m_N.net().isRailNet() && t->m_P.net().isRailNet())
#if 0
				netlist().error("Found device %s connected only to railterminals %s/%s\n",
						t->name().cstr(), t->m_N.net().name().cstr(), t->m_P.net().name().cstr());
#else
				netlist().warning("Found device %s connected only to railterminals %s/%s\n",
					t->name().cstr(), t->m_N.net().name().cstr(), t->m_P.net().name().cstr());
#endif
		}
	}

	netlist().log("initialize solver ...\n");

	if (netlist().solver() == NULL)
	{
		if (has_twoterms)
			netlist().error("No solver found for this net although analog elements are present\n");
	}
	else
		netlist().solver()->post_start();

}

void setup_t::start_devices()
{
	pstring env = nl_util::environment("NL_LOGS");

	if (env != "")
	{
		NL_VERBOSE_OUT(("Creating dynamic logs ...\n"));
		pstring_list_t ll(env, ":");
		for (unsigned i=0; i < ll.size(); i++)
		{
			NL_VERBOSE_OUT(("%d: <%s>\n",i, ll[i].cstr()));
			NL_VERBOSE_OUT(("%d: <%s>\n",i, ll[i].cstr()));
			device_t *nc = factory().new_device_by_name("LOG");
			pstring name = "log_" + ll[i];
			register_dev(nc, name);
			register_link(name + ".I", ll[i]);
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
			printf("Device %20s : %12d %12d %15ld\n", entry->name().cstr(), entry->stat_call_count, entry->stat_update_count, (long int) entry->stat_total_time / (entry->stat_update_count + 1));
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
	virtual devices::nld_base_d_to_a_proxy *create_d_a_proxy(logic_output_t *proxied) const
	{
		return palloc(devices::nld_d_to_a_proxy(proxied));
	}
};

logic_family_desc_t *setup_t::family_from_model(const pstring &model)
{
	model_map_t map;
	model_parse(model, map);

	if (setup_t::model_value_str(map, "TYPE") == "TTL")
		return netlist_family_TTL;
	if (setup_t::model_value_str(map, "TYPE") == "CD4XXX")
		return netlist_family_CD4XXX;

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
			netlist().error("Model %s not found\n", model.cstr());
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
			netlist().error("Model doesn't exist %s\n", xmodel.cstr());
	}

	pstring remainder=model.substr(pos+1).trim();
	if (!remainder.endsWith(")"))
		netlist().error("Model error %s\n", model.cstr());
	remainder = remainder.left(remainder.len() - 1);
	pstring_list_t pairs(remainder," ", true);
	for (unsigned i=0; i<pairs.size(); i++)
	{
		int pose = pairs[i].find('=');
		if (pose < 0)
			netlist().error("Model error on pair %s\n", model.cstr());
		map[pairs[i].left(pose).ucase()] = pairs[i].substr(pose+1);
	}
}

const pstring setup_t::model_value_str(model_map_t &map, const pstring &entity)
{
	pstring ret;

	if (entity != entity.ucase())
		netlist().error("model parameters should be uppercase:%s %s\n", entity.cstr(), model_string(map).cstr());
	if (!map.contains(entity))
		netlist().error("Entity %s not found in model %s\n", entity.cstr(), model_string(map).cstr());
	else
		ret = map[entity];

	return ret;
}

nl_double setup_t::model_value(model_map_t &map, const pstring &entity)
{
	pstring tmp = model_value_str(map, entity);

	nl_double factor = NL_FCONST(1.0);
	char numfac = *(tmp.right(1).cstr());
	switch (numfac)
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
			if (numfac < '0' || numfac > '9')
				fatalerror_e("Unknown number factor <%c> in: %s", numfac, entity.cstr());
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
	for (std::size_t i=0; i < m_sources.size(); i++)
	{
		if (m_sources[i]->parse(this, netlist_name))
			return;
	}
	netlist().error("unable to find %s in source collection", netlist_name.cstr());
}

// ----------------------------------------------------------------------------------------
// base sources
// ----------------------------------------------------------------------------------------

bool netlist_source_string_t::parse(setup_t *setup, const pstring name)
{
	parser_t p(*setup);
	return p.parse(m_str, name);
}

bool netlist_source_mem_t::parse(setup_t *setup, const pstring name)
{
	parser_t p(*setup);
	return p.parse(m_str, name);
}

}
