// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.c
 *
 */

#include "nl_base.h"
#include "nl_setup.h"
#include "nl_parser.h"
#include "nl_util.h"
#include "devices/net_lib.h"
#include "devices/nld_system.h"
#include "analog/nld_solver.h"
#include "analog/nld_twoterm.h"

static NETLIST_START(base)
	TTL_INPUT(ttlhigh, 1)
	TTL_INPUT(ttllow, 0)
	NET_REGISTER_DEV(gnd, GND)

	INCLUDE(diode_models);
	INCLUDE(bjt_models);

NETLIST_END()


// ----------------------------------------------------------------------------------------
// netlist_setup_t
// ----------------------------------------------------------------------------------------

netlist_setup_t::netlist_setup_t(netlist_base_t &netlist)
	: m_netlist(netlist)
	, m_proxy_cnt(0)
{
	netlist.set_setup(this);
}

void netlist_setup_t::init()
{
	m_factory.initialize();
	NETLIST_NAME(base)(*this);
}


netlist_setup_t::~netlist_setup_t()
{
	m_links.clear();
	m_alias.reset();
	m_params.reset();
	m_terminals.reset();
	m_params_temp.reset();

	netlist().set_setup(NULL);

	pstring::resetmem();
}

ATTR_COLD pstring netlist_setup_t::build_fqn(const pstring &obj_name) const
{
	if (m_stack.empty())
		return netlist().name() + "." + obj_name;
	else
		return m_stack.peek() + "." + obj_name;
}

void netlist_setup_t::namespace_push(const pstring &aname)
{
	if (m_stack.empty())
		m_stack.push(netlist().name() + "." + aname);
	else
		m_stack.push(m_stack.peek() + "." + aname);
}

void netlist_setup_t::namespace_pop()
{
	m_stack.pop();
}


netlist_device_t *netlist_setup_t::register_dev(netlist_device_t *dev, const pstring &name)
{
	pstring fqn = build_fqn(name);

	dev->init(netlist(), fqn);

	if (!(netlist().m_devices.add(fqn, dev, false)==TMERR_NONE))
		netlist().error("Error adding %s to device list\n", name.cstr());
	return dev;
}

template <class T>
static void remove_start_with(T &hm, pstring &sw)
{
	typename T::entry_t *entry = hm.first();
	while (entry != NULL)
	{
		typename T::entry_t *next = hm.next(entry);
		pstring x = entry->tag().cstr();
		if (sw.equals(x.substr(0, sw.len())))
		{
			NL_VERBOSE_OUT(("removing %s\n", entry->tag().cstr()));
			hm.remove(entry->object());
		}
		entry = next;
	}
}

void netlist_setup_t::remove_dev(const pstring &name)
{
	netlist_device_t *dev = netlist().m_devices.find(name);
	pstring temp = name + ".";
	if (dev == NULL)
		netlist().error("Device %s does not exist\n", name.cstr());

	//remove_start_with<tagmap_input_t>(m_inputs, temp);
	remove_start_with<tagmap_terminal_t>(m_terminals, temp);
	remove_start_with<tagmap_param_t>(m_params, temp);

	const link_t *p = m_links.first();
	while (p != NULL)
	{
		const link_t *n = m_links.next(p);
		if (temp.equals(p->e1.substr(0,temp.len())) || temp.equals(p->e2.substr(0,temp.len())))
			m_links.remove(*p);
		p = n;
	}
	netlist().m_devices.remove(name);
}

void netlist_setup_t::register_model(const pstring &model)
{
	m_models.add(model);
}

void netlist_setup_t::register_alias_nofqn(const pstring &alias, const pstring &out)
{
	if (!(m_alias.add(alias, out, false)==TMERR_NONE))
		netlist().error("Error adding alias %s to alias list\n", alias.cstr());
}

void netlist_setup_t::register_alias(const pstring &alias, const pstring &out)
{
	pstring alias_fqn = build_fqn(alias);
	pstring out_fqn = build_fqn(out);
	register_alias_nofqn(alias_fqn, out_fqn);
}

pstring netlist_setup_t::objtype_as_astr(netlist_object_t &in) const
{
	switch (in.type())
	{
		case netlist_terminal_t::TERMINAL:
			return "TERMINAL";
			break;
		case netlist_terminal_t::INPUT:
			return "INPUT";
			break;
		case netlist_terminal_t::OUTPUT:
			return "OUTPUT";
			break;
		case netlist_terminal_t::NET:
			return "NET";
			break;
		case netlist_terminal_t::PARAM:
			return "PARAM";
			break;
		case netlist_terminal_t::DEVICE:
			return "DEVICE";
			break;
		case netlist_terminal_t::NETLIST:
			return "NETLIST";
			break;
	}
	// FIXME: noreturn
	netlist().error("Unknown object type %d\n", in.type());
	return "Error";
}

void netlist_setup_t::register_object(netlist_device_t &dev, const pstring &name, netlist_object_t &obj)
{
	switch (obj.type())
	{
		case netlist_terminal_t::TERMINAL:
		case netlist_terminal_t::INPUT:
		case netlist_terminal_t::OUTPUT:
			{
				netlist_core_terminal_t &term = dynamic_cast<netlist_core_terminal_t &>(obj);
				if (obj.isType(netlist_terminal_t::OUTPUT))
					dynamic_cast<netlist_output_t &>(term).init_object(dev, dev.name() + "." + name);
				else
					term.init_object(dev, dev.name() + "." + name);

				if (!(m_terminals.add(term.name(), &term, false)==TMERR_NONE))
					netlist().error("Error adding %s %s to terminal list\n", objtype_as_astr(term).cstr(), term.name().cstr());
				NL_VERBOSE_OUT(("%s %s\n", objtype_as_astr(term).cstr(), name.cstr()));
			}
			break;
		case netlist_terminal_t::NET:
			break;
		case netlist_terminal_t::PARAM:
			{
				netlist_param_t &param = dynamic_cast<netlist_param_t &>(obj);
				const pstring val = m_params_temp.find(name);
				if (val != "")
				{
					switch (param.param_type())
					{
						case netlist_param_t::DOUBLE:
						{
							NL_VERBOSE_OUT(("Found parameter ... %s : %s\n", name.cstr(), val.cstr()));
							double vald = 0;
							if (sscanf(val.cstr(), "%lf", &vald) != 1)
								netlist().error("Invalid number conversion %s : %s\n", name.cstr(), val.cstr());
							dynamic_cast<netlist_param_double_t &>(param).initial(vald);
						}
						break;
						case netlist_param_t::INTEGER:
						case netlist_param_t::LOGIC:
						{
							NL_VERBOSE_OUT(("Found parameter ... %s : %s\n", name.cstr(), val.cstr()));
							double vald = 0;
							if (sscanf(val.cstr(), "%lf", &vald) != 1)
								netlist().error("Invalid number conversion %s : %s\n", name.cstr(), val.cstr());
							dynamic_cast<netlist_param_int_t &>(param).initial((int) vald);
						}
						break;
						case netlist_param_t::STRING:
						{
							dynamic_cast<netlist_param_str_t &>(param).initial(val);
						}
						break;
						case netlist_param_t::MODEL:
						{
							pstring search = (".model " + val + " ").ucase();
							bool found = false;
							for (int i=0; i < m_models.count(); i++)
							{
								if (m_models[i].ucase().startsWith(search))
								{
									//int pl=m_models[i].find("(");
									//int pr=m_models[i].find(")");
									//dynamic_cast<netlist_param_model_t &>(param).initial(m_models[i].substr(pl+1,pr-pl-1));
									dynamic_cast<netlist_param_model_t &>(param).initial(m_models[i]);
									found = true;
									break;
								}
							}
							if (!found)
								netlist().error("Model %s not found\n", val.cstr());
						}
						break;
						default:
							netlist().error("Parameter is not supported %s : %s\n", name.cstr(), val.cstr());
					}
				}
				if (!(m_params.add(name, &param, false)==TMERR_NONE))
					netlist().error("Error adding parameter %s to parameter list\n", name.cstr());
			}
			break;
		case netlist_terminal_t::DEVICE:
			netlist().error("Device registration not yet supported - %s\n", name.cstr());
			break;
		case netlist_terminal_t::NETLIST:
			netlist().error("Netlist registration not yet supported - %s\n", name.cstr());
			break;
	}
}

void netlist_setup_t::register_link_arr(const pstring &terms)
{
	nl_util::pstring_list list = nl_util::split(terms,", ");
	if (list.count() < 2)
		netlist().error("You must pass at least 2 terminals to NET_C");
	pstring last = list[0];
	for (int i = 1; i < list.count(); i++)
	{
		register_link(last, list[i]);
		last = list[i];
	}
}


void netlist_setup_t::register_link(const pstring &sin, const pstring &sout)
{
	link_t temp = link_t(build_fqn(sin), build_fqn(sout));
	NL_VERBOSE_OUT(("link %s <== %s\n", sin.cstr(), sout.cstr()));
	m_links.add(temp);
	//if (!(m_links.add(sin + "." + sout, temp, false)==TMERR_NONE))
	//  fatalerror("Error adding link %s<==%s to link list\n", sin.cstr(), sout.cstr());
}

void netlist_setup_t::register_param(const pstring &param, const double value)
{
	// FIXME: there should be a better way
	register_param(param, pstring::sprintf("%.9e", value));
}

void netlist_setup_t::register_param(const pstring &param, const pstring &value)
{
	pstring fqn = build_fqn(param);

	if (!(m_params_temp.add(fqn, value, false)==TMERR_NONE))
		netlist().error("Error adding parameter %s to parameter list\n", param.cstr());
}

const pstring netlist_setup_t::resolve_alias(const pstring &name) const
{
	pstring temp = name;
	pstring ret;

	/* FIXME: Detect endless loop */
	do {
		ret = temp;
		temp = m_alias.find(ret);
	} while (temp != "");

	NL_VERBOSE_OUT(("%s==>%s\n", name.cstr(), ret.cstr()));
	return ret;
}

netlist_core_terminal_t *netlist_setup_t::find_terminal(const pstring &terminal_in, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	netlist_core_terminal_t *ret;

	ret = m_terminals.find(tname);
	/* look for default */
	if (ret == NULL)
	{
		/* look for ".Q" std output */
		pstring s = tname + ".Q";
		ret = m_terminals.find(s);
	}
	if (ret == NULL && required)
		netlist().error("terminal %s(%s) not found!\n", terminal_in.cstr(), tname.cstr());
	if (ret != NULL)
		NL_VERBOSE_OUT(("Found input %s\n", tname.cstr()));
	return ret;
}

netlist_core_terminal_t *netlist_setup_t::find_terminal(const pstring &terminal_in, netlist_object_t::type_t atype, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	netlist_core_terminal_t *ret;

	ret = m_terminals.find(tname);
	/* look for default */
	if (ret == NULL && atype == netlist_object_t::OUTPUT)
	{
		/* look for ".Q" std output */
		pstring s = tname + ".Q";
		ret = m_terminals.find(s);
	}
	if (ret == NULL && required)
		netlist().error("terminal %s(%s) not found!\n", terminal_in.cstr(), tname.cstr());
	if (ret != NULL && ret->type() != atype)
	{
		if (required)
			netlist().error("object %s(%s) found but wrong type\n", terminal_in.cstr(), tname.cstr());
		else
			ret = NULL;
	}
	if (ret != NULL)
		NL_VERBOSE_OUT(("Found input %s\n", tname.cstr()));
	return ret;
}

netlist_param_t *netlist_setup_t::find_param(const pstring &param_in, bool required)
{
	const pstring param_in_fqn = build_fqn(param_in);

	const pstring &outname = resolve_alias(param_in_fqn);
	netlist_param_t *ret;

	ret = m_params.find(outname);
	if (ret == NULL && required)
		netlist().error("parameter %s(%s) not found!\n", param_in_fqn.cstr(), outname.cstr());
	if (ret != NULL)
		NL_VERBOSE_OUT(("Found parameter %s\n", outname.cstr()));
	return ret;
}

nld_base_d_to_a_proxy *netlist_setup_t::get_d_a_proxy(netlist_output_t &out)
{
	assert(out.isFamily(netlist_terminal_t::LOGIC));

	//printf("proxy for %s\n", out.name().cstr());;
	netlist_logic_output_t &out_cast = dynamic_cast<netlist_logic_output_t &>(out);
	nld_base_d_to_a_proxy *proxy = out_cast.get_proxy();

	if (proxy == NULL)
	{
		// create a new one ...
		proxy = new nld_d_to_a_proxy(out);
		pstring x = pstring::sprintf("proxy_da_%d", m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(proxy, x);
		proxy->start_dev();

#if 1
		/* connect all existing terminals to new net */

		netlist_core_terminal_t *p = out.net().m_list.first();
		while (p != NULL)
		{
			netlist_core_terminal_t *np = out.net().m_list.next(p);
			p->clear_net(); // de-link from all nets ...
			connect(proxy->out(), *p);
			p = np;
		}
		out.net().m_list.clear(); // clear the list
		out.net().m_num_cons = 0;
#endif
		out.net().register_con(proxy->m_I);
		out_cast.set_proxy(proxy);

	}
	return proxy;
}

void netlist_setup_t::connect_input_output(netlist_input_t &in, netlist_output_t &out)
{
	if (out.isFamily(netlist_terminal_t::ANALOG) && in.isFamily(netlist_terminal_t::LOGIC))
	{
		nld_a_to_d_proxy *proxy = new nld_a_to_d_proxy(in);
		pstring x = pstring::sprintf("proxy_ad_%d", m_proxy_cnt);
		m_proxy_cnt++;

		register_dev(proxy, x);
		proxy->start_dev();

		proxy->m_Q.net().register_con(in);
		out.net().register_con(proxy->m_I);

	}
	else if (out.isFamily(netlist_terminal_t::LOGIC) && in.isFamily(netlist_terminal_t::ANALOG))
	{
		nld_base_d_to_a_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->out(), in);
		//proxy->out().net().register_con(in);
	}
	else
	{
		out.net().register_con(in);
	}
}

void netlist_setup_t::connect_terminal_input(netlist_terminal_t &term, netlist_input_t &inp)
{
	if (inp.isFamily(netlist_terminal_t::ANALOG))
	{
		connect_terminals(inp, term);
	}
	else if (inp.isFamily(netlist_terminal_t::LOGIC))
	{
		NL_VERBOSE_OUT(("connect_terminal_input: connecting proxy\n"));
		nld_a_to_d_proxy *proxy = new nld_a_to_d_proxy(inp);
		pstring x = pstring::sprintf("proxy_da_%d", m_proxy_cnt);
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
		NL_VERBOSE_OUT(("connect_terminal_output: connecting proxy\n"));
		nld_base_d_to_a_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->out(), in);
	}
	else
	{
		netlist().error("Netlist: Severe Error");
	}
}

void netlist_setup_t::connect_terminals(netlist_core_terminal_t &t1, netlist_core_terminal_t &t2)
{
	//assert(in.isType(netlist_terminal_t::TERMINAL));
	//assert(out.isType(netlist_terminal_t::TERMINAL));

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
		netlist_net_t *anet =  new netlist_net_t(netlist_object_t::NET, netlist_object_t::ANALOG);
		t1.set_net(*anet);
		//m_netlist.solver()->m_nets.add(anet);
		// FIXME: Nets should have a unique name
		t1.net().init_object(netlist(),"net." + t1.name() );
		t1.net().register_con(t2);
		t1.net().register_con(t1);
	}
}

static netlist_core_terminal_t &resolve_proxy(netlist_core_terminal_t &term)
{
	if (term.isType(netlist_core_terminal_t::OUTPUT) && term.isFamily(netlist_core_terminal_t::LOGIC))
	{
		netlist_logic_output_t &out = dynamic_cast<netlist_logic_output_t &>(term);
		if (out.has_proxy())
			return out.get_proxy()->out();
	}
	return term;
}

void netlist_setup_t::connect(netlist_core_terminal_t &t1_in, netlist_core_terminal_t &t2_in)
{
	NL_VERBOSE_OUT(("Connecting %s to %s\n", t1_in.name().cstr(), t2_in.name().cstr()));
	netlist_core_terminal_t &t1 = resolve_proxy(t1_in);
	netlist_core_terminal_t &t2 = resolve_proxy(t2_in);

	if (t1.isType(netlist_core_terminal_t::OUTPUT) && t2.isType(netlist_core_terminal_t::INPUT))
	{
		if (t2.has_net() && t2.net().isRailNet())
			netlist().error("Input %s already connected\n", t2.name().cstr());
		connect_input_output(dynamic_cast<netlist_input_t &>(t2), dynamic_cast<netlist_output_t &>(t1));
	}
	else if (t1.isType(netlist_core_terminal_t::INPUT) && t2.isType(netlist_core_terminal_t::OUTPUT))
	{
		if (t1.has_net()  && t1.net().isRailNet())
			netlist().error("Input %s already connected\n", t1.name().cstr());
		connect_input_output(dynamic_cast<netlist_input_t &>(t1), dynamic_cast<netlist_output_t &>(t2));
	}
	else if (t1.isType(netlist_core_terminal_t::OUTPUT) && t2.isType(netlist_core_terminal_t::TERMINAL))
	{
		connect_terminal_output(dynamic_cast<netlist_terminal_t &>(t2), dynamic_cast<netlist_output_t &>(t1));
	}
	else if (t1.isType(netlist_core_terminal_t::TERMINAL) && t2.isType(netlist_core_terminal_t::OUTPUT))
	{
		connect_terminal_output(dynamic_cast<netlist_terminal_t &>(t1), dynamic_cast<netlist_output_t &>(t2));
	}
	else if (t1.isType(netlist_core_terminal_t::INPUT) && t2.isType(netlist_core_terminal_t::TERMINAL))
	{
		connect_terminal_input(dynamic_cast<netlist_terminal_t &>(t2), dynamic_cast<netlist_input_t &>(t1));
	}
	else if (t1.isType(netlist_core_terminal_t::TERMINAL) && t2.isType(netlist_core_terminal_t::INPUT))
	{
		connect_terminal_input(dynamic_cast<netlist_terminal_t &>(t1), dynamic_cast<netlist_input_t &>(t2));
	}
	else if (t1.isType(netlist_core_terminal_t::TERMINAL) && t2.isType(netlist_core_terminal_t::TERMINAL))
	{
		connect_terminals(dynamic_cast<netlist_terminal_t &>(t1), dynamic_cast<netlist_terminal_t &>(t2));
	}
	else
		netlist().error("Connecting %s to %s not supported!\n", t1.name().cstr(), t2.name().cstr());
}

void netlist_setup_t::resolve_inputs()
{
	bool has_twoterms = false;

	netlist().log("Resolving inputs ...");

	for (const link_t *entry = m_links.first(); entry != NULL; entry = m_links.next(entry))
	{
		const pstring t1s = entry->e1;
		const pstring t2s = entry->e2;
		netlist_core_terminal_t *t1 = find_terminal(t1s);
		netlist_core_terminal_t *t2 = find_terminal(t2s);

		connect(*t1, *t2);
	}

	//netlist().log("printing outputs ...");
	/* print all outputs */
	for (tagmap_terminal_t::entry_t *entry = m_terminals.first(); entry != NULL; entry = m_terminals.next(entry))
	{
		ATTR_UNUSED netlist_output_t *out = dynamic_cast<netlist_output_t *>(entry->object());
		//if (out != NULL)
			//VERBOSE_OUT(("%s %d\n", out->netdev()->name(), *out->Q_ptr()));
	}

	netlist().log("deleting empty nets ...");

	// delete empty nets ... and save m_list ...

	netlist_net_t::list_t todelete;

	for (netlist_net_t *const *pn = netlist().m_nets.first(); pn != NULL; pn = netlist().m_nets.next(pn))
	{
		if ((*pn)->m_list.is_empty())
		{
			todelete.add(*pn);
		}
		else
		{
			for (netlist_core_terminal_t *p = (*pn)->m_list.first(); p != NULL; p = (*pn)->m_list.next(p))
				(*pn)->m_registered.add(p);
		}
	}

	for (int i=0; i < todelete.count(); i++)
	{
		netlist().log("Deleting net %s ...", todelete[i]->name().cstr());
		netlist().m_nets.remove(todelete[i]);
		if (!todelete[i]->isRailNet())
			delete todelete[i];
	}

	pstring errstr("");

	netlist().log("looking for terminals not connected ...");
	for (tagmap_terminal_t::entry_t *entry = m_terminals.first(); entry != NULL; entry = m_terminals.next(entry))
	{
		if (!entry->object()->has_net())
			errstr += pstring::sprintf("Found terminal %s without a net\n",
					entry->object()->name().cstr());
		else if (entry->object()->net().num_cons() == 0)
			netlist().warning("Found terminal %s without connections",
					entry->object()->name().cstr());
	}
	if (errstr != "")
		netlist().error("%s", errstr.cstr());


	netlist().log("looking for two terms connected to rail nets ...\n");
	for (tagmap_devices_t::entry_t *entry = netlist().m_devices.first(); entry != NULL; entry = netlist().m_devices.next(entry))
	{
		NETLIB_NAME(twoterm) *t = dynamic_cast<NETLIB_NAME(twoterm) *>(entry->object());
		if (t != NULL)
		{
			has_twoterms = true;
			if (t->m_N.net().isRailNet() && t->m_P.net().isRailNet())
				netlist().error("Found device %s connected only to railterminals %s/%s\n",
						t->name().cstr(), t->m_N.net().name().cstr(), t->m_P.net().name().cstr());
		}
	}

	netlist().log("initialize solver ...\n");

	if (m_netlist.solver() == NULL)
	{
		if (has_twoterms)
			netlist().error("No solver found for this net although analog elements are present\n");
	}
	else
		m_netlist.solver()->post_start();

}

void netlist_setup_t::start_devices()
{
	if (getenv("NL_LOGS"))
	{
		NL_VERBOSE_OUT(("Creating dynamic logs ...\n"));
		nl_util::pstring_list ll = nl_util::split(getenv("NL_LOGS"), ":");
		for (int i=0; i < ll.count(); i++)
		{
			NL_VERBOSE_OUT(("%d: <%s>\n",i, ll[i].cstr()));
			printf("%d: <%s>\n",i, ll[i].cstr());
			netlist_device_t *nc = factory().new_device_by_classname("nld_log", *this);
			pstring name = "log_" + ll[i];
			register_dev(nc, name);
			register_link(name + ".I", ll[i]);
		}
	}


	NL_VERBOSE_OUT(("Searching for mainclock and solver ...\n"));

	netlist().start();
}

void netlist_setup_t::parse(const char *buf)
{
	netlist_parser parser(*this);
	parser.parse(buf);
}

void netlist_setup_t::print_stats() const
{
#if (NL_KEEP_STATISTICS)
	{
		for (netlist_setup_t::tagmap_devices_t::entry_t *entry = m_devices.first(); entry != NULL; entry = m_devices.next(entry))
		{
			//entry->object()->s
			printf("Device %20s : %12d %15ld\n", entry->object()->name().cstr(), entry->object()->stat_count, (long int) entry->object()->total_time / (entry->object()->stat_count + 1));
		}
		printf("Queue Start %15d\n", m_netlist.queue().m_prof_start);
		printf("Queue End   %15d\n", m_netlist.queue().m_prof_end);
		printf("Queue Sort  %15d\n", m_netlist.queue().m_prof_sort);
		printf("Queue Move  %15d\n", m_netlist.queue().m_prof_sortmove);
	}
#endif
}
