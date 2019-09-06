// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#include "nl_parser.h"
#include "nl_base.h"
#include "nl_errstr.h"
#include "nl_factory.h"

namespace netlist
{
// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

void parser_t::verror(const pstring &msg, int line_num, const pstring &line)
{
	m_setup.log().fatal("line {1}: error: {2}\n\t\t{3}\n", line_num,
			msg, line);

	//throw error;
}

bool parser_t::parse(const pstring &nlname)
{
	this->identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-")
		.number_chars(".0123456789", "0123456789eE-.") //FIXME: processing of numbers
		//set_whitespace(pstring("").cat(' ').cat(9).cat(10).cat(13));
		.whitespace(pstring("") + ' ' + static_cast<char>(9) + static_cast<char>(10) + static_cast<char>(13))
		.comment("/*", "*/", "//");
	m_tok_param_left = register_token("(");
	m_tok_param_right = register_token(")");
	m_tok_comma = register_token(",");

	m_tok_ALIAS = register_token("ALIAS");
	m_tok_DIPPINS = register_token("DIPPINS");
	m_tok_NET_C = register_token("NET_C");
	m_tok_FRONTIER = register_token("OPTIMIZE_FRONTIER");
	m_tok_PARAM = register_token("PARAM");
	m_tok_HINT = register_token("HINT");
	m_tok_NET_MODEL = register_token("NET_MODEL");
	m_tok_INCLUDE = register_token("INCLUDE");
	m_tok_LOCAL_SOURCE = register_token("LOCAL_SOURCE");
	m_tok_LOCAL_LIB_ENTRY = register_token("LOCAL_LIB_ENTRY");
	m_tok_SUBMODEL = register_token("SUBMODEL");
	m_tok_NETLIST_START = register_token("NETLIST_START");
	m_tok_NETLIST_END = register_token("NETLIST_END");
	m_tok_TRUTHTABLE_START = register_token("TRUTHTABLE_START");
	m_tok_TRUTHTABLE_END = register_token("TRUTHTABLE_END");
	m_tok_TT_HEAD = register_token("TT_HEAD");
	m_tok_TT_LINE = register_token("TT_LINE");
	m_tok_TT_FAMILY = register_token("TT_FAMILY");

	bool in_nl = false;

	while (true)
	{
		token_t token = get_token();
		if (token.is_type(ENDOFFILE))
		{
			return false;
		}

		if (token.is(m_tok_NETLIST_END))
		{
			require_token(m_tok_param_left);
			if (!in_nl)
				error (MF_UNEXPECTED_NETLIST_END());
			else
			{
				in_nl = false;
			}
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_NETLIST_START))
		{
			if (in_nl)
				error (MF_UNEXPECTED_NETLIST_START());
			require_token(m_tok_param_left);
			token_t name = get_token();
			require_token(m_tok_param_right);
			if (name.str() == nlname || nlname == "")
			{
				parse_netlist(name.str());
				return true;
			} else
				in_nl = true;
		}
	}
}

void parser_t::parse_netlist(const pstring &nlname)
{
	while (true)
	{
		token_t token = get_token();

		if (token.is_type(ENDOFFILE))
			return;

		require_token(m_tok_param_left);
		m_setup.log().debug("Parser: Device: {1}\n", token.str());

		if (token.is(m_tok_ALIAS))
			net_alias();
		else if (token.is(m_tok_DIPPINS))
			dippins();
		else if (token.is(m_tok_NET_C))
			net_c();
		else if (token.is(m_tok_FRONTIER))
			frontier();
		else if (token.is(m_tok_PARAM))
			netdev_param();
		else if (token.is(m_tok_HINT))
			netdev_hint();
		else if (token.is(m_tok_NET_MODEL))
			net_model();
		else if (token.is(m_tok_SUBMODEL))
			net_submodel();
		else if (token.is(m_tok_INCLUDE))
			net_include();
		else if (token.is(m_tok_LOCAL_SOURCE))
			net_local_source();
		else if (token.is(m_tok_TRUTHTABLE_START))
			net_truthtable_start(nlname);
		else if (token.is(m_tok_LOCAL_LIB_ENTRY))
		{
			m_setup.register_lib_entry(get_identifier(), "parser: " + nlname);
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_NETLIST_END))
		{
			netdev_netlist_end();
			return;
		}
		else
			device(token.str());
	}
}

void parser_t::net_truthtable_start(const pstring &nlname)
{
	pstring name = get_identifier();
	require_token(m_tok_comma);
	long ni = get_number_long();
	require_token(m_tok_comma);
	long no = get_number_long();
	require_token(m_tok_comma);
	pstring def_param = get_string();
	require_token(m_tok_param_right);

	netlist::tt_desc desc;
	desc.classname = name;
	desc.name = name;
	desc.ni = static_cast<unsigned long>(ni);
	desc.no = static_cast<unsigned long>(no);
	desc.def_param = "+" + def_param;
	desc.family = "";

	while (true)
	{
		token_t token = get_token();

		if (token.is(m_tok_TT_HEAD))
		{
			require_token(m_tok_param_left);
			desc.desc.push_back(get_string());
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_TT_LINE))
		{
			require_token(m_tok_param_left);
			desc.desc.push_back(get_string());
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_TT_FAMILY))
		{
			require_token(m_tok_param_left);
			desc.family = get_string();
			require_token(m_tok_param_right);
		}
		else
		{
			require_token(token, m_tok_TRUTHTABLE_END);
			require_token(m_tok_param_left);
			require_token(m_tok_param_right);
			m_setup.tt_factory_create(desc, nlname);
			return;
		}
	}
}


void parser_t::netdev_netlist_start()
{
	// don't do much
	token_t name = get_token();
	require_token(m_tok_param_right);
}

void parser_t::netdev_netlist_end()
{
	// don't do much
	require_token(m_tok_param_right);
}

void parser_t::net_model()
{
	// don't do much
	pstring model = get_string();
	m_setup.register_model(model);
	require_token(m_tok_param_right);
}

void parser_t::net_submodel()
{
	// don't do much
	pstring model = get_identifier();
	require_token(m_tok_comma);
	pstring name = get_identifier();
	require_token(m_tok_param_right);

	m_setup.namespace_push(name);
	m_setup.include(model);
	m_setup.namespace_pop();
}

void parser_t::frontier()
{
	// don't do much
	pstring attachat = get_identifier();
	require_token(m_tok_comma);
	double r_IN = eval_param(get_token());
	require_token(m_tok_comma);
	double r_OUT = eval_param(get_token());
	require_token(m_tok_param_right);

	m_setup.register_frontier(attachat, r_IN, r_OUT);
}

void parser_t::net_include()
{
	// don't do much
	pstring name = get_identifier();
	require_token(m_tok_param_right);

	m_setup.include(name);
}

void parser_t::net_local_source()
{
	// This directive is only for hardcoded netlists. Ignore it here.
	pstring name = get_identifier();
	require_token(m_tok_param_right);

}

void parser_t::net_alias()
{
	pstring alias = get_identifier_or_number();

	require_token(m_tok_comma);

	pstring out = get_identifier();

	require_token(m_tok_param_right);

	m_setup.log().debug("Parser: Alias: {1} {2}\n", alias, out);
	m_setup.register_alias(alias, out);
}

void parser_t::net_c()
{
	pstring first = get_identifier();
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		m_setup.register_link(first , t1);
		m_setup.log().debug("Parser: Connect: {1} {2}\n", first, t1);
		token_t n = get_token();
		if (n.is(m_tok_param_right))
			break;
		if (!n.is(m_tok_comma))
			error(plib::pfmt("expected a comma, found <{1}>")(n.str()) );
	}

}

void parser_t::dippins()
{
	std::vector<pstring> pins;

	pins.push_back(get_identifier());
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		pins.push_back(t1);
		token_t n = get_token();
		if (n.is(m_tok_param_right))
			break;
		if (!n.is(m_tok_comma))
			error(plib::pfmt("expected a comma, found <{1}>")(n.str()) );
	}
	if ((pins.size() % 2) == 1)
		error(plib::pfmt("You must pass an equal number of pins to DIPPINS, first pin is {}")(pins[0]));
	std::size_t n = pins.size();
	for (std::size_t i = 0; i < n / 2; i++)
	{
		m_setup.register_alias(plib::pfmt("{1}")(i+1), pins[i*2]);
		m_setup.register_alias(plib::pfmt("{1}")(n-i), pins[i*2 + 1]);
	}
}

void parser_t::netdev_param()
{
	pstring param;
	param = get_identifier();
	require_token(m_tok_comma);
	token_t tok = get_token();
	if (tok.is_type(STRING))
	{
		m_setup.log().debug("Parser: Param: {1} {2}\n", param, tok.str());
		m_setup.register_param(param, tok.str());
	}
	else
	{
		nl_double val = eval_param(tok);
		m_setup.log().debug("Parser: Param: {1} {2}\n", param, val);
		m_setup.register_param(param, val);
	}
	require_token(m_tok_param_right);
}

void parser_t::netdev_hint()
{
	pstring dev(get_identifier());
	require_token(m_tok_comma);
	pstring hint(get_identifier());
	m_setup.register_param(dev + ".HINT_" + hint, 1);
	require_token(m_tok_param_right);
}

void parser_t::device(const pstring &dev_type)
{
	factory::element_t *f = m_setup.factory().factory_by_name(dev_type);
	auto paramlist = plib::psplit(f->param_desc(), ",");

	pstring devname = get_identifier();

	m_setup.register_dev(dev_type, devname);
	m_setup.log().debug("Parser: IC: {1}\n", devname);

	for (const pstring &tp : paramlist)
	{
		if (plib::startsWith(tp, "+"))
		{
			require_token(m_tok_comma);
			pstring output_name = get_identifier();
			m_setup.log().debug("Link: {1} {2}\n", tp, output_name);

			m_setup.register_link(devname + "." + tp.substr(1), output_name);
		}
		else if (plib::startsWith(tp, "@"))
		{
			pstring term = tp.substr(1);
			m_setup.log().debug("Link: {1} {2}\n", tp, term);

			//FIXME
			if (term == "VCC")
				m_setup.register_link(devname + "." + term, "V5");
			else
				m_setup.register_link(devname + "." + term, term);
		}
		else
		{
			require_token(m_tok_comma);
			pstring paramfq = devname + "." + tp;

			m_setup.log().debug("Defparam: {1}\n", paramfq);
			token_t tok = get_token();
			if (tok.is_type(STRING))
			{
				m_setup.register_param(paramfq, tok.str());
			}
			else
			{
				nl_double val = eval_param(tok);
				m_setup.register_param(paramfq, val);
			}
		}
	}

	// error(plib::pfmt("Input count mismatch for {1} - expected {2} found {3}")(devname)(termlist.size())(cnt));
	require_token(m_tok_param_right);
}


// ----------------------------------------------------------------------------------------
// private
// ----------------------------------------------------------------------------------------


nl_double parser_t::eval_param(const token_t &tok)
{
	static std::array<pstring, 6> macs = {"", "RES_K", "RES_M", "CAP_U", "CAP_N", "CAP_P"};
	static std::array<nl_double, 6> facs = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
	std::size_t f=0;
	nl_double ret;

	for (std::size_t i=1; i<macs.size();i++)
		if (tok.str() == macs[i])
			f = i;
	if (f>0)
	{
		require_token(m_tok_param_left);
		ret = get_number_double();
		require_token(m_tok_param_right);
	}
	else
	{
		bool err;
		ret = plib::pstonum_ne<nl_double, true>(tok.str(), err);
		if (err)
			error(plib::pfmt("Parameter value <{1}> not double \n")(tok.str()));
	}
	return ret * facs[f];

}

} // namespace netlist
