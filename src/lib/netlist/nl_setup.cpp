// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlsetup.c
 *
 */

#include "plib/palloc.h"
#include "analog/nld_twoterm.h"
#include "devices/nlid_proxy.h"
#include "devices/nlid_system.h"
#include "devices/nlid_truthtable.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "nl_parser.h"
#include "nl_setup.h"
#include "plib/putil.h"
#include "solver/nld_solver.h"

#include <cmath>

namespace netlist
{
	// ----------------------------------------------------------------------------------------
	// nl_parse_t
	// ----------------------------------------------------------------------------------------

	nlparse_t::nlparse_t(setup_t &setup, log_type &log)
	: m_factory(log)
	, m_setup(setup)
	, m_log(log)
	, m_frontier_cnt(0)
	{ }

	void nlparse_t::register_alias(const pstring &alias, const pstring &out)
	{
		pstring alias_fqn = build_fqn(alias);
		pstring out_fqn = build_fqn(out);
		register_alias_nofqn(alias_fqn, out_fqn);
	}

	void nlparse_t::register_dippins_arr(const pstring &terms)
	{
		std::vector<pstring> list(plib::psplit(terms,", "));
		if (list.size() == 0 || (list.size() % 2) == 1)
			log().fatal(MF_DIP_PINS_MUST_BE_AN_EQUAL_NUMBER_OF_PINS_1(build_fqn("")));
		std::size_t n = list.size();
		for (std::size_t i = 0; i < n / 2; i++)
		{
			register_alias(plib::pfmt("{1}")(i+1), list[i * 2]);
			register_alias(plib::pfmt("{1}")(n-i), list[i * 2 + 1]);
		}
	}

	void nlparse_t::register_dev(const pstring &classname, const pstring &name)
	{
		auto f = m_factory.factory_by_name(classname);
		if (f == nullptr)
			log().fatal(MF_CLASS_1_NOT_FOUND(classname));
		else
		{
			/* make sure we parse macro library entries */
			f->macro_actions(*this, name);
			pstring key = build_fqn(name);
			if (device_exists(key))
				log().fatal(MF_DEVICE_ALREADY_EXISTS_1(name));
			else
				m_device_factory.insert(m_device_factory.end(), {key, f});
		}
	}

	void nlparse_t::register_link(const pstring &sin, const pstring &sout)
	{
		register_link_fqn(build_fqn(sin), build_fqn(sout));
	}

	void nlparse_t::register_link_arr(const pstring &terms)
	{
		std::vector<pstring> list(plib::psplit(terms,", "));
		if (list.size() < 2)
			log().fatal(MF_NET_C_NEEDS_AT_LEAST_2_TERMINAL());
		for (std::size_t i = 1; i < list.size(); i++)
		{
			register_link(list[0], list[i]);
		}
	}

	void nlparse_t::include(const pstring &netlist_name)
	{
		for (auto &source : m_sources)
		{
			if (source->parse(*this, netlist_name))
				return;
		}
		log().fatal(MF_NOT_FOUND_IN_SOURCE_COLLECTION(netlist_name));
	}


	void nlparse_t::namespace_push(const pstring &aname)
	{
		if (m_namespace_stack.empty())
			//m_namespace_stack.push(netlist().name() + "." + aname);
			m_namespace_stack.push(aname);
		else
			m_namespace_stack.push(m_namespace_stack.top() + "." + aname);
	}

	void nlparse_t::namespace_pop()
	{
		m_namespace_stack.pop();
	}

	void nlparse_t::register_param(const pstring &param, const double value)
	{
		if (std::abs(value - std::floor(value)) > 1e-30 || std::abs(value) > 1e9)
			register_param(param, plib::pfmt("{1:.9}").e(value));
		else
			register_param(param, plib::pfmt("{1}")(static_cast<long>(value)));
	}

	void nlparse_t::register_param(const pstring &param, const pstring &value)
	{
		pstring fqn = build_fqn(param);

		auto idx = m_param_values.find(fqn);
		if (idx == m_param_values.end())
		{
			if (!m_param_values.insert({fqn, value}).second)
				log().fatal(MF_ADDING_PARAMETER_1_TO_PARAMETER_LIST(param));
		}
		else
		{
			log().warning(MW_OVERWRITING_PARAM_1_OLD_2_NEW_3(fqn, idx->second,
					value));
			m_param_values[fqn] = value;
		}
	}

	void nlparse_t::register_lib_entry(const pstring &name, const pstring &sourcefile)
	{
		m_factory.register_device(plib::make_unique<factory::library_element_t>(name, name, "", sourcefile));
	}

	void nlparse_t::register_frontier(const pstring &attach, const double r_IN, const double r_OUT)
	{
		pstring frontier_name = plib::pfmt("frontier_{1}")(m_frontier_cnt);
		m_frontier_cnt++;
		register_dev("FRONTIER_DEV", frontier_name);
		register_param(frontier_name + ".RIN", r_IN);
		register_param(frontier_name + ".ROUT", r_OUT);
		register_link(frontier_name + ".G", "GND");
		pstring attfn = build_fqn(attach);
		pstring front_fqn = build_fqn(frontier_name);
		bool found = false;
		for (auto & link  : m_links)
		{
			if (link.first == attfn)
			{
				link.first = front_fqn + ".I";
				found = true;
			}
			else if (link.second == attfn)
			{
				link.second = front_fqn + ".I";
				found = true;
			}
		}
		if (!found)
			log().fatal(MF_FOUND_NO_OCCURRENCE_OF_1(attach));
		register_link(attach, frontier_name + ".Q");
	}

	void nlparse_t::tt_factory_create(tt_desc &desc, const pstring &sourcefile)
	{
		auto fac = devices::tt_factory_create(desc, sourcefile);
		m_factory.register_device(std::move(fac));
	}

	pstring nlparse_t::build_fqn(const pstring &obj_name) const
	{
		if (m_namespace_stack.empty())
			//return netlist().name() + "." + obj_name;
			return obj_name;
		else
			return m_namespace_stack.top() + "." + obj_name;
	}

	void nlparse_t::register_alias_nofqn(const pstring &alias, const pstring &out)
	{
		if (!m_alias.insert({alias, out}).second)
			log().fatal(MF_ADDING_ALI1_TO_ALIAS_LIST(alias));
	}

	void nlparse_t::register_link_fqn(const pstring &sin, const pstring &sout)
	{
		link_t temp = link_t(sin, sout);
		log().debug("link {1} <== {2}\n", sin, sout);
		m_links.push_back(temp);
	}

	bool nlparse_t::device_exists(const pstring &name) const
	{
		for (auto &d : m_device_factory)
			if (d.first == name)
				return true;
		return false;
	}

	bool nlparse_t::parse_stream(plib::unique_ptr<plib::pistream> &&istrm, const pstring &name)
	{
		return parser_t(std::move(plib::ppreprocessor(&m_defines).process(std::move(istrm))), *this).parse(name);
	}

	void nlparse_t::add_define(const pstring &defstr)
	{
		auto p = defstr.find("=");
		if (p != pstring::npos)
			add_define(plib::left(defstr, p), defstr.substr(p+1));
		else
			add_define(defstr, "1");
	}

	// ----------------------------------------------------------------------------------------
	// setup_t
	// ----------------------------------------------------------------------------------------


setup_t::setup_t(netlist_state_t &nlstate)
	: nlparse_t(*this, nlstate.log())
	, m_nlstate(nlstate)
	, m_netlist_params(nullptr)
	, m_proxy_cnt(0)
{
}

setup_t::~setup_t() noexcept
{
	// FIXME: can't see a need any longer
	m_links.clear();
	m_params.clear();
	m_terminals.clear();
	m_param_values.clear();
	m_sources.clear();
}

pstring setup_t::termtype_as_str(detail::core_terminal_t &in) const
{
	switch (in.type())
	{
		case detail::terminal_type::TERMINAL:
			return pstring("TERMINAL");
		case detail::terminal_type::INPUT:
			return pstring("INPUT");
		case detail::terminal_type::OUTPUT:
			return pstring("OUTPUT");
	}
	log().fatal(MF_UNKNOWN_OBJECT_TYPE_1(static_cast<unsigned>(in.type())));
	return pstring("Error");
}

pstring setup_t::get_initial_param_val(const pstring &name, const pstring &def) const
{
	auto i = m_param_values.find(name);
	if (i != m_param_values.end())
		return i->second;
	else
		return def;
}

void setup_t::register_term(detail::core_terminal_t &term)
{
	if (!m_terminals.insert({term.name(), &term}).second)
	{
		log().fatal(MF_ADDING_1_2_TO_TERMINAL_LIST(termtype_as_str(term), term.name()));
	}
	log().debug("{1} {2}\n", termtype_as_str(term), term.name());
}

void setup_t::remove_connections(const pstring &pin)
{
	pstring pinfn = build_fqn(pin);
	bool found = false;

	for (auto link = m_links.begin(); link != m_links.end(); )
	{
		if ((link->first == pinfn) || (link->second == pinfn))
		{
			log().verbose("removing connection: {1} <==> {2}\n", link->first, link->second);
			link = m_links.erase(link);
			found = true;
		}
		else
			link++;
	}
	if (!found)
		log().fatal(MF_FOUND_NO_OCCURRENCE_OF_1(pin));
}



void setup_t::register_param_t(const pstring &name, param_t &param)
{
	if (!m_params.insert({param.name(), param_ref_t(param.name(), param.device(), param)}).second)
		log().fatal(MF_ADDING_PARAMETER_1_TO_PARAMETER_LIST(name));
}



const pstring setup_t::resolve_alias(const pstring &name) const
{
	pstring temp = name;
	pstring ret;

	/* FIXME: Detect endless loop */
	do {
		ret = temp;
		auto p = m_alias.find(ret);
		temp = (p != m_alias.end() ? p->second : "");
	} while (temp != "" && temp != ret);

	log().debug("{1}==>{2}\n", name, ret);
	return ret;
}

std::vector<pstring> setup_t::get_terminals_for_device_name(const pstring &devname)
{
	std::vector<pstring> terms;
	for (auto & t : m_terminals)
	{
		if (plib::startsWith(t.second->name(), devname))
		{
			pstring tn(t.second->name().substr(devname.length()+1));
			if (tn.find(".") == pstring::npos)
				terms.push_back(tn);
		}
	}

	for (auto & t : m_alias)
	{
		if (plib::startsWith(t.first, devname))
		{
			pstring tn(t.first.substr(devname.length()+1));
			//printf("\t%s %s %s\n", t.first.c_str(), t.second.c_str(), tn.c_str());
			if (tn.find(".") == pstring::npos)
			{
				terms.push_back(tn);
				pstring resolved = resolve_alias(t.first);
				//printf("\t%s %s %s\n", t.first.c_str(), t.second.c_str(), resolved.c_str());
				if (resolved != t.first)
				{
					auto found = std::find(terms.begin(), terms.end(), resolved.substr(devname.length()+1));
					if (found!=terms.end())
						terms.erase(found);
				}
			}
		}
	}
	return terms;
}

detail::core_terminal_t *setup_t::find_terminal(const pstring &terminal_in, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	auto ret = m_terminals.find(tname);
	/* look for default */
	if (ret == m_terminals.end())
	{
		/* look for ".Q" std output */
		ret = m_terminals.find(tname + ".Q");
	}

	detail::core_terminal_t *term = (ret == m_terminals.end() ? nullptr : ret->second);

	if (term == nullptr && required)
		log().fatal(MF_TERMINAL_1_2_NOT_FOUND(terminal_in, tname));
	if (term != nullptr)
		log().debug("Found input {1}\n", tname);
	return term;
}

detail::core_terminal_t *setup_t::find_terminal(const pstring &terminal_in,
		detail::terminal_type atype, bool required)
{
	const pstring &tname = resolve_alias(terminal_in);
	auto ret = m_terminals.find(tname);
	/* look for default */
	if (ret == m_terminals.end() && atype == detail::terminal_type::OUTPUT)
	{
		/* look for ".Q" std output */
		ret = m_terminals.find(tname + ".Q");
	}
	if (ret == m_terminals.end() && required)
		log().fatal(MF_TERMINAL_1_2_NOT_FOUND(terminal_in, tname));

	detail::core_terminal_t *term = (ret == m_terminals.end() ? nullptr : ret->second);

	if (term != nullptr && term->type() != atype)
	{
		if (required)
			log().fatal(MF_OBJECT_1_2_WRONG_TYPE(terminal_in, tname));
		else
			term = nullptr;
	}
	if (term != nullptr)
		log().debug("Found input {1}\n", tname);

	return term;
}

param_t *setup_t::find_param(const pstring &param_in, bool required) const
{
	const pstring param_in_fqn = build_fqn(param_in);

	const pstring &outname = resolve_alias(param_in_fqn);
	auto ret = m_params.find(outname);
	if (ret == m_params.end() && required)
		log().fatal(MF_PARAMETER_1_2_NOT_FOUND(param_in_fqn, outname));
	if (ret != m_params.end())
		log().debug("Found parameter {1}\n", outname);
	return (ret == m_params.end() ? nullptr : &ret->second.m_param);
}

devices::nld_base_proxy *setup_t::get_d_a_proxy(detail::core_terminal_t &out)
{
	nl_assert(out.is_logic());

	auto &out_cast = static_cast<logic_output_t &>(out);
	devices::nld_base_proxy *proxy = out_cast.get_proxy();

	if (proxy == nullptr)
	{
		// create a new one ...
		pstring x = plib::pfmt("proxy_da_{1}_{2}")(out.name())(m_proxy_cnt);
		auto new_proxy =
				out_cast.logic_family()->create_d_a_proxy(m_nlstate, x, &out_cast);
		m_proxy_cnt++;
		/* connect all existing terminals to new net */

		for (auto & p : out.net().core_terms())
		{
			p->clear_net(); // de-link from all nets ...
			if (!connect(new_proxy->proxy_term(), *p))
				log().fatal(MF_CONNECTING_1_TO_2(
						new_proxy->proxy_term().name(), (*p).name()));
		}
		out.net().core_terms().clear(); // clear the list

		out.net().add_terminal(new_proxy->in());
		out_cast.set_proxy(proxy);

		proxy = new_proxy.get();

		m_nlstate.add_dev(new_proxy->name(), std::move(new_proxy));
	}
	return proxy;
}

devices::nld_base_proxy *setup_t::get_a_d_proxy(detail::core_terminal_t &inp)
{
	nl_assert(inp.is_logic());

	auto &incast = dynamic_cast<logic_input_t &>(inp);
	devices::nld_base_proxy *proxy = incast.get_proxy();

	if (proxy != nullptr)
		return proxy;
	else
	{
		log().debug("connect_terminal_input: connecting proxy\n");
		pstring x = plib::pfmt("proxy_ad_{1}_{2}")(inp.name())(m_proxy_cnt);
		auto new_proxy = incast.logic_family()->create_a_d_proxy(m_nlstate, x, &incast);
		//auto new_proxy = plib::owned_ptr<devices::nld_a_to_d_proxy>::Create(netlist(), x, &incast);
		incast.set_proxy(new_proxy.get());
		m_proxy_cnt++;

		auto ret = new_proxy.get();

		/* connect all existing terminals to new net */

		if (inp.has_net())
		{
			for (auto & p : inp.net().core_terms())
			{
				p->clear_net(); // de-link from all nets ...
				if (!connect(ret->proxy_term(), *p))
					log().fatal(MF_CONNECTING_1_TO_2(
							ret->proxy_term().name(), (*p).name()));
			}
			inp.net().core_terms().clear(); // clear the list
		}
		ret->out().net().add_terminal(inp);
		m_nlstate.add_dev(new_proxy->name(), std::move(new_proxy));
		return ret;
	}
}

void setup_t::merge_nets(detail::net_t &thisnet, detail::net_t &othernet)
{
	log().debug("merging nets ...\n");
	if (&othernet == &thisnet)
	{
		log().warning(MW_CONNECTING_1_TO_ITSELF(thisnet.name()));
		return; // Nothing to do
	}

	if (thisnet.isRailNet() && othernet.isRailNet())
		log().fatal(MF_MERGE_RAIL_NETS_1_AND_2(thisnet.name(), othernet.name()));

	if (othernet.isRailNet())
	{
		log().debug("othernet is railnet\n");
		merge_nets(othernet, thisnet);
	}
	else
	{
		othernet.move_connections(thisnet);
	}
}



void setup_t::connect_input_output(detail::core_terminal_t &in, detail::core_terminal_t &out)
{
	if (out.is_analog() && in.is_logic())
	{
		auto proxy = get_a_d_proxy(in);

		out.net().add_terminal(proxy->proxy_term());
	}
	else if (out.is_logic() && in.is_analog())
	{
		devices::nld_base_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->proxy_term(), in);
		//proxy->out().net().register_con(in);
	}
	else
	{
		if (in.has_net())
			merge_nets(out.net(), in.net());
		else
			out.net().add_terminal(in);
	}
}


void setup_t::connect_terminal_input(terminal_t &term, detail::core_terminal_t &inp)
{
	if (inp.is_analog())
	{
		connect_terminals(inp, term);
	}
	else if (inp.is_logic())
	{
		log().verbose("connect terminal {1} (in, {2}) to {3}\n", inp.name(),
				inp.is_analog() ? pstring("analog") : inp.is_logic() ? pstring("logic") : pstring("?"), term.name());
		auto proxy = get_a_d_proxy(inp);

		//out.net().register_con(proxy->proxy_term());
		connect_terminals(term, proxy->proxy_term());

	}
	else
	{
		log().fatal(MF_OBJECT_INPUT_TYPE_1(inp.name()));
	}
}

void setup_t::connect_terminal_output(terminal_t &in, detail::core_terminal_t &out)
{
	if (out.is_analog())
	{
		log().debug("connect_terminal_output: {1} {2}\n", in.name(), out.name());
		/* no proxy needed, just merge existing terminal net */
		if (in.has_net())
			merge_nets(out.net(), in.net());
		else
			out.net().add_terminal(in);
	}
	else if (out.is_logic())
	{
		log().debug("connect_terminal_output: connecting proxy\n");
		devices::nld_base_proxy *proxy = get_d_a_proxy(out);

		connect_terminals(proxy->proxy_term(), in);
	}
	else
	{
		log().fatal(MF_OBJECT_OUTPUT_TYPE_1(out.name()));
	}
}

void setup_t::connect_terminals(detail::core_terminal_t &t1, detail::core_terminal_t &t2)
{
	if (t1.has_net() && t2.has_net())
	{
		log().debug("T2 and T1 have net\n");
		merge_nets(t1.net(), t2.net());
	}
	else if (t2.has_net())
	{
		log().debug("T2 has net\n");
		t2.net().add_terminal(t1);
	}
	else if (t1.has_net())
	{
		log().debug("T1 has net\n");
		t1.net().add_terminal(t2);
	}
	else
	{
		log().debug("adding analog net ...\n");
		// FIXME: Nets should have a unique name
		auto anet = pool().make_poolptr<analog_net_t>(m_nlstate,"net." + t1.name());
		auto anetp = anet.get();
		m_nlstate.register_net(std::move(anet));
		t1.set_net(anetp);
		anetp->add_terminal(t2);
		anetp->add_terminal(t1);
	}
}

static detail::core_terminal_t &resolve_proxy(detail::core_terminal_t &term)
{
	if (term.is_logic())
	{
		auto &out = dynamic_cast<logic_t &>(term);
		if (out.has_proxy())
			return out.get_proxy()->proxy_term();
	}
	return term;
}

bool setup_t::connect_input_input(detail::core_terminal_t &t1, detail::core_terminal_t &t2)
{
	bool ret = false;
	if (t1.has_net())
	{
		if (t1.net().isRailNet())
			ret = connect(t2, t1.net().railterminal());
		if (!ret)
		{
			for (auto & t : t1.net().core_terms())
			{
				if (t->is_type(detail::terminal_type::TERMINAL))
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
			for (auto & t : t2.net().core_terms())
			{
				if (t->is_type(detail::terminal_type::TERMINAL))
					ret = connect(t1, *t);
				if (ret)
					break;
			}
		}
	}
	return ret;
}

bool setup_t::connect(detail::core_terminal_t &t1_in, detail::core_terminal_t &t2_in)
{
	log().debug("Connecting {1} to {2}\n", t1_in.name(), t2_in.name());
	detail::core_terminal_t &t1 = resolve_proxy(t1_in);
	detail::core_terminal_t &t2 = resolve_proxy(t2_in);
	bool ret = true;

	if (t1.is_type(detail::terminal_type::OUTPUT) && t2.is_type(detail::terminal_type::INPUT))
	{
		if (t2.has_net() && t2.net().isRailNet())
			log().fatal(MF_INPUT_1_ALREADY_CONNECTED(t2.name()));
		connect_input_output(t2, t1);
	}
	else if (t1.is_type(detail::terminal_type::INPUT) && t2.is_type(detail::terminal_type::OUTPUT))
	{
		if (t1.has_net()  && t1.net().isRailNet())
			log().fatal(MF_INPUT_1_ALREADY_CONNECTED(t1.name()));
		connect_input_output(t1, t2);
	}
	else if (t1.is_type(detail::terminal_type::OUTPUT) && t2.is_type(detail::terminal_type::TERMINAL))
	{
		connect_terminal_output(dynamic_cast<terminal_t &>(t2), t1);
	}
	else if (t1.is_type(detail::terminal_type::TERMINAL) && t2.is_type(detail::terminal_type::OUTPUT))
	{
		connect_terminal_output(dynamic_cast<terminal_t &>(t1), t2);
	}
	else if (t1.is_type(detail::terminal_type::INPUT) && t2.is_type(detail::terminal_type::TERMINAL))
	{
		connect_terminal_input(dynamic_cast<terminal_t &>(t2), t1);
	}
	else if (t1.is_type(detail::terminal_type::TERMINAL) && t2.is_type(detail::terminal_type::INPUT))
	{
		connect_terminal_input(dynamic_cast<terminal_t &>(t1), t2);
	}
	else if (t1.is_type(detail::terminal_type::TERMINAL) && t2.is_type(detail::terminal_type::TERMINAL))
	{
		connect_terminals(dynamic_cast<terminal_t &>(t1), dynamic_cast<terminal_t &>(t2));
	}
	else if (t1.is_type(detail::terminal_type::INPUT) && t2.is_type(detail::terminal_type::INPUT))
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
	log().verbose("Resolving inputs ...");

	/* Netlist can directly connect input to input.
	 * We therefore first park connecting inputs and retry
	 * after all other terminals were connected.
	 */
	int tries = NL_MAX_LINK_RESOLVE_LOOPS;
	while (m_links.size() > 0 && tries >  0)
	{

		for (auto li = m_links.begin(); li != m_links.end(); )
		{
			const pstring t1s = li->first;
			const pstring t2s = li->second;
			detail::core_terminal_t *t1 = find_terminal(t1s);
			detail::core_terminal_t *t2 = find_terminal(t2s);

			//printf("%s %s\n", t1s.c_str(), t2s.c_str());
			if (connect(*t1, *t2))
				li = m_links.erase(li);
			else
				li++;
		}
		tries--;
	}
	if (tries == 0)
	{
		for (auto & link : m_links)
			log().warning(MF_CONNECTING_1_TO_2(link.first, link.second));

		log().fatal(MF_LINK_TRIES_EXCEEDED(NL_MAX_LINK_RESOLVE_LOOPS));
	}

	log().verbose("deleting empty nets ...");

	// delete empty nets

	delete_empty_nets();

	pstring errstr("");

	log().verbose("looking for terminals not connected ...");
	for (auto & i : m_terminals)
	{
		detail::core_terminal_t *term = i.second;
		if (!term->has_net() && dynamic_cast< devices::NETLIB_NAME(dummy_input) *>(&term->device()) != nullptr)
			log().info(MI_DUMMY_1_WITHOUT_CONNECTIONS(term->name()));
		else if (!term->has_net())
			errstr += plib::pfmt("Found terminal {1} without a net\n")(term->name());
		else if (term->net().num_cons() == 0)
		{
			if (term->is_logic_input())
				log().warning(MW_LOGIC_INPUT_1_WITHOUT_CONNECTIONS(term->name()));
			else if (term->is_logic_output())
				log().info(MI_LOGIC_OUTPUT_1_WITHOUT_CONNECTIONS(term->name()));
			else if (term->is_analog_output())
				log().info(MI_ANALOG_OUTPUT_1_WITHOUT_CONNECTIONS(term->name()));
			else
				log().warning(MW_TERMINAL_1_WITHOUT_CONNECTIONS(term->name()));
		}
	}
	//FIXME: error string handling
	if (errstr != "")
		log().fatal("{1}", errstr);

}

void setup_t::register_dynamic_log_devices()
{
	pstring env = plib::util::environment("NL_LOGS", "");

	if (env != "")
	{
		log().debug("Creating dynamic logs ...");
		std::vector<pstring> loglist(plib::psplit(env, ":"));
		for (const pstring &ll : loglist)
		{
			pstring name = "log_" + ll;
			auto nc = factory().factory_by_name("LOG")->Create(m_nlstate, name);
			register_link(name + ".I", ll);
			log().debug("    dynamic link {1}: <{2}>\n",ll, name);
			m_nlstate.add_dev(nc->name(), std::move(nc));
		}
	}
}

log_type &setup_t::log()
{
	return m_nlstate.log();
}
const log_type &setup_t::log() const
{
	return m_nlstate.log();
}


// ----------------------------------------------------------------------------------------
// Models
// ----------------------------------------------------------------------------------------

void models_t::register_model(pstring model_in)
{
	auto pos = model_in.find(" ");
	if (pos == pstring::npos)
		throw nl_exception(MF_UNABLE_TO_PARSE_MODEL_1(model_in));
	pstring model = plib::ucase(plib::trim(plib::left(model_in, pos)));
	pstring def = plib::trim(model_in.substr(pos + 1));
	if (!m_models.insert({model, def}).second)
		throw nl_exception(MF_MODEL_ALREADY_EXISTS_1(model_in));
}

void models_t::model_parse(const pstring &model_in, model_map_t &map)
{
	pstring model = model_in;
	std::size_t pos = 0;
	pstring key;

	while (true)
	{
		pos = model.find("(");
		if (pos != pstring::npos) break;

		key = plib::ucase(model);
		auto i = m_models.find(key);
		if (i == m_models.end())
			throw nl_exception(MF_MODEL_NOT_FOUND(model));
		model = i->second;
	}
	pstring xmodel = plib::left(model, pos);

	if (xmodel == "_")
		map["COREMODEL"] = key;
	else
	{
		auto i = m_models.find(xmodel);
		if (i != m_models.end())
			model_parse(xmodel, map);
		else
			throw nl_exception(MF_MODEL_NOT_FOUND(model_in));
	}

	pstring remainder = plib::trim(model.substr(pos + 1));
	if (!plib::endsWith(remainder, ")"))
		throw nl_exception(MF_MODEL_ERROR_1(model));
	// FIMXE: Not optimal
	remainder = plib::left(remainder, remainder.size() - 1);

	std::vector<pstring> pairs(plib::psplit(remainder," ", true));
	for (pstring &pe : pairs)
	{
		auto pose = pe.find("=");
		if (pose == pstring::npos)
			throw nl_exception(MF_MODEL_ERROR_ON_PAIR_1(model));
		map[plib::ucase(plib::left(pe, pose))] = pe.substr(pose + 1);
	}
}

pstring models_t::model_string(model_map_t &map)
{
	pstring ret = map["COREMODEL"] + "(";
	for (auto & i : map)
		ret = ret + i.first + "=" + i.second + " ";

	return ret + ")";
}

pstring models_t::value_str(pstring model, pstring entity)
{
	model_map_t &map = m_cache[model];

	if (map.size() == 0)
		model_parse(model , map);

	pstring ret;

	if (entity != plib::ucase(entity))
		throw nl_exception(MF_MODEL_PARAMETERS_NOT_UPPERCASE_1_2(entity, model_string(map)));
	if (map.find(entity) == map.end())
		throw nl_exception(MF_ENTITY_1_NOT_FOUND_IN_MODEL_2(entity, model_string(map)));
	else
		ret = map[entity];

	return ret;
}

nl_double models_t::value(pstring model, pstring entity)
{
	model_map_t &map = m_cache[model];

	if (map.size() == 0)
		model_parse(model , map);

	pstring tmp = value_str(model, entity);

	nl_double factor = plib::constants<nl_double>::one();
	auto p = std::next(tmp.begin(), static_cast<pstring::difference_type>(tmp.size() - 1));
	switch (*p)
	{
		case 'M': factor = 1e6; break;
		case 'k': factor = 1e3; break;
		case 'K': factor = 1e3; break;
		case 'm': factor = 1e-3; break;
		case 'u': factor = 1e-6; break;
		case 'n': factor = 1e-9; break;
		case 'p': factor = 1e-12; break;
		case 'f': factor = 1e-15; break;
		case 'a': factor = 1e-18; break;
		default:
			if (*p < '0' || *p > '9')
				throw nl_exception(MF_UNKNOWN_NUMBER_FACTOR_IN_1(entity));
	}
	if (factor != plib::constants<nl_double>::one())
		tmp = plib::left(tmp, tmp.size() - 1);
	// FIXME: check for errors
	//printf("%s %s %e %e\n", entity.c_str(), tmp.c_str(), plib::pstonum<nl_double>(tmp), factor);
	return plib::pstonum<nl_double>(tmp) * factor;
}

class logic_family_std_proxy_t : public logic_family_desc_t
{
public:
	logic_family_std_proxy_t() = default;
	pool_owned_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_state_t &anetlist,
			const pstring &name, logic_output_t *proxied) const override;
	pool_owned_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_state_t &anetlist, const pstring &name, logic_input_t *proxied) const override;
};

pool_owned_ptr<devices::nld_base_d_to_a_proxy> logic_family_std_proxy_t::create_d_a_proxy(netlist_state_t &anetlist,
		const pstring &name, logic_output_t *proxied) const
{
	return pool().make_poolptr<devices::nld_d_to_a_proxy>(anetlist, name, proxied);
}
pool_owned_ptr<devices::nld_base_a_to_d_proxy> logic_family_std_proxy_t::create_a_d_proxy(netlist_state_t &anetlist, const pstring &name, logic_input_t *proxied) const
{
	return pool().make_poolptr<devices::nld_a_to_d_proxy>(anetlist, name, proxied);
}


const logic_family_desc_t *setup_t::family_from_model(const pstring &model)
{

	if (m_models.value_str(model, "TYPE") == "TTL")
		return family_TTL();
	if (m_models.value_str(model, "TYPE") == "CD4XXX")
		return family_CD4XXX();

	for (auto & e : m_nlstate.m_family_cache)
		if (e.first == model)
			return e.second.get();

	auto ret = plib::make_unique<logic_family_std_proxy_t>();

	ret->m_fixed_V = m_models.value(model, "FV");
	ret->m_low_thresh_PCNT = m_models.value(model, "IVL");
	ret->m_high_thresh_PCNT = m_models.value(model, "IVH");
	ret->m_low_VO = m_models.value(model, "OVL");
	ret->m_high_VO = m_models. value(model, "OVH");
	ret->m_R_low = m_models.value(model, "ORL");
	ret->m_R_high = m_models.value(model, "ORH");

	auto retp = ret.get();

	m_nlstate.m_family_cache.emplace_back(model, std::move(ret));

	return retp;
}

// ----------------------------------------------------------------------------------------
// Sources
// ----------------------------------------------------------------------------------------

plib::unique_ptr<plib::pistream> setup_t::get_data_stream(const pstring &name)
{
	for (auto &source : m_sources)
	{
		if (source->type() == source_t::DATA)
		{
			auto strm = source->stream(name);
			if (strm)
				return strm;
		}
	}
	log().warning(MW_DATA_1_NOT_FOUND(name));
	return plib::unique_ptr<plib::pistream>(nullptr);
}


// ----------------------------------------------------------------------------------------
// Device handling
// ----------------------------------------------------------------------------------------

void setup_t::delete_empty_nets()
{
	m_nlstate.nets().erase(
		std::remove_if(m_nlstate.nets().begin(), m_nlstate.nets().end(),
			[](pool_owned_ptr<detail::net_t> &x)
			{
				if (x->num_cons() == 0)
				{
					x->state().log().verbose("Deleting net {1} ...", x->name());
					x->state().run_state_manager().remove_save_items(x.get());
					return true;
				}
				else
					return false;
			}), m_nlstate.nets().end());
}

// ----------------------------------------------------------------------------------------
// Run preparation
// ----------------------------------------------------------------------------------------

void setup_t::prepare_to_run()
{
	register_dynamic_log_devices();

	/* make sure the solver and parameters are started first! */

	for (auto & e : m_device_factory)
	{
		if ( factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				|| factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			m_nlstate.add_dev(e.first, pool_owned_ptr<device_t>(e.second->Create(m_nlstate, e.first)));
		}
	}

	log().debug("Searching for solver and parameters ...\n");

	auto solver = m_nlstate.get_single_device<devices::NETLIB_NAME(solver)>("solver");
	m_netlist_params = m_nlstate.get_single_device<devices::NETLIB_NAME(netlistparams)>("parameter");

	/* set default model parameters */

	m_models.register_model(plib::pfmt("NMOS_DEFAULT _(CAPMOD={1})")(m_netlist_params->m_mos_capmodel()));
	m_models.register_model(plib::pfmt("PMOS_DEFAULT _(CAPMOD={1})")(m_netlist_params->m_mos_capmodel()));

	/* create devices */

	log().debug("Creating devices ...\n");
	for (auto & e : m_device_factory)
	{
		if ( !factory().is_class<devices::NETLIB_NAME(solver)>(e.second)
				&& !factory().is_class<devices::NETLIB_NAME(netlistparams)>(e.second))
		{
			auto dev = pool_owned_ptr<device_t>(e.second->Create(m_nlstate, e.first));
			m_nlstate.add_dev(dev->name(), std::move(dev));
		}
	}

	log().debug("Looking for unknown parameters ...\n");
	for (auto &p : m_param_values)
	{
		auto f = m_params.find(p.first);
		if (f == m_params.end())
		{
			if (plib::endsWith(p.first, pstring(".HINT_NO_DEACTIVATE")))
			{
				// FIXME: get device name, check for device
			}
			else
				log().warning("Unknown parameter: {}", p.first);
		}
	}

	bool use_deactivate = m_netlist_params->m_use_deactivate() ? true : false;

	for (auto &d : m_nlstate.devices())
	{
		if (use_deactivate)
		{
			auto p = m_param_values.find(d.second->name() + ".HINT_NO_DEACTIVATE");
			if (p != m_param_values.end())
			{
				//FIXME: check for errors ...
				auto v = plib::pstonum<double>(p->second);
				if (std::abs(v - std::floor(v)) > 1e-6 )
					log().fatal(MF_HND_VAL_NOT_SUPPORTED(p->second));
				d.second->set_hint_deactivate(v == 0.0);
			}
		}
		else
			d.second->set_hint_deactivate(false);
	}

	/* resolve inputs */
	resolve_inputs();

	log().verbose("looking for two terms connected to rail nets ...");
	for (auto & t : m_nlstate.get_device_list<analog::NETLIB_NAME(twoterm)>())
	{
		if (t->m_N.net().isRailNet() && t->m_P.net().isRailNet())
		{
			log().info(MI_REMOVE_DEVICE_1_CONNECTED_ONLY_TO_RAILS_2_3(
				t->name(), t->m_N.net().name(), t->m_P.net().name()));
			t->m_N.net().remove_terminal(t->m_N);
			t->m_P.net().remove_terminal(t->m_P);
			m_nlstate.remove_dev(t);
		}
	}

	log().verbose("initialize solver ...\n");

	if (solver == nullptr)
	{
		for (auto &p : m_nlstate.nets())
			if (p->is_analog())
				log().fatal(MF_NO_SOLVER());
	}
	else
		solver->post_start();

	for (auto &n : m_nlstate.nets())
		for (auto & term : n->core_terms())
		{
			//core_device_t *dev = reinterpret_cast<core_device_t *>(term->m_delegate.object());
			core_device_t *dev = &term->device();
			dev->set_default_delegate(*term);
		}

}

// ----------------------------------------------------------------------------------------
// base sources
// ----------------------------------------------------------------------------------------

bool source_t::parse(nlparse_t &setup, const pstring &name)
{
	if (m_type != SOURCE)
		return false;
	else
	{
		auto strm(stream(name));
		if (strm)
			return setup.parse_stream(std::move(strm), name);
		else
			return false;
	}
}

plib::unique_ptr<plib::pistream> source_string_t::stream(const pstring &name)
{
	plib::unused_var(name);
	return plib::make_unique<plib::pimemstream>(m_str.c_str(), std::strlen(m_str.c_str()));
}

plib::unique_ptr<plib::pistream> source_mem_t::stream(const pstring &name)
{
	plib::unused_var(name);
	return plib::make_unique<plib::pimemstream>(m_str.c_str(), std::strlen(m_str.c_str()));
}

plib::unique_ptr<plib::pistream> source_file_t::stream(const pstring &name)
{
	plib::unused_var(name);
	return plib::make_unique<plib::pifilestream>(m_filename);
}

bool source_proc_t::parse(nlparse_t &setup, const pstring &name)
{
	if (name == m_setup_func_name)
	{
		m_setup_func(setup);
		return true;
	}
	else
		return false;
}

plib::unique_ptr<plib::pistream> source_proc_t::stream(const pstring &name)
{
	plib::unused_var(name);
	plib::unique_ptr<plib::pistream> p(nullptr);
	return p;
}

} // namespace netlist

