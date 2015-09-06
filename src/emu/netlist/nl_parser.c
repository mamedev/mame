// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#include "nl_parser.h"
#include "nl_factory.h"
#include "devices/nld_truthtable.h"

// for now, make buggy GCC/Mingw STFU about I64FMT
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

namespace netlist
{
// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

ATTR_COLD void parser_t::verror(const pstring &msg, int line_num, const pstring &line)
{
	m_setup.log().fatal("line {1}: error: {2}\n\t\t{3}\n", line_num,
			msg, line);

	//throw error;
}


bool parser_t::parse(const pstring nlname)
{
	set_identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-");
	set_number_chars(".0123456789", "0123456789eE-."); //FIXME: processing of numbers
	char ws[5];
	ws[0] = ' ';
	ws[1] = 9;
	ws[2] = 10;
	ws[3] = 13;
	ws[4] = 0;
	set_whitespace(ws);
	set_comment("/*", "*/", "//");
	m_tok_param_left = register_token("(");
	m_tok_param_right = register_token(")");
	m_tok_comma = register_token(",");

	m_tok_ALIAS = register_token("ALIAS");
	m_tok_DIPPINS = register_token("DIPPINS");
	m_tok_NET_C = register_token("NET_C");
	m_tok_FRONTIER = register_token("OPTIMIZE_FRONTIER");
	m_tok_PARAM = register_token("PARAM");
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
			//error("EOF while searching for <{1}>", nlname);
		}

		if (token.is(m_tok_NETLIST_END))
		{
			require_token(m_tok_param_left);
			if (!in_nl)
				error("Unexpected NETLIST_END");
			else
			{
				in_nl = false;
			}
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_NETLIST_START))
		{
			if (in_nl)
				error("Unexpected NETLIST_START");
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

void parser_t::parse_netlist(ATTR_UNUSED const pstring &nlname)
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
		else if (token.is(m_tok_NET_MODEL))
			net_model();
		else if (token.is(m_tok_SUBMODEL))
			net_submodel();
		else if (token.is(m_tok_INCLUDE))
			net_include();
		else if (token.is(m_tok_LOCAL_SOURCE))
			net_local_source();
		else if (token.is(m_tok_TRUTHTABLE_START))
			net_truthtable_start();
		else if (token.is(m_tok_LOCAL_LIB_ENTRY))
		{
			m_setup.register_lib_entry(get_identifier());
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

void parser_t::net_truthtable_start()
{
	pstring name = get_identifier();
	require_token(m_tok_comma);
	unsigned ni = get_number_long();
	require_token(m_tok_comma);
	unsigned no = get_number_long();
	require_token(m_tok_comma);
	unsigned hs = get_number_long();
	require_token(m_tok_comma);
	pstring def_param = get_string();
	require_token(m_tok_param_right);

	netlist::devices::netlist_base_factory_truthtable_t *ttd = netlist::devices::nl_tt_factory_create(ni, no, hs,
			name, name, "+" + def_param);

	while (true)
	{
		token_t token = get_token();

		if (token.is(m_tok_TT_HEAD))
		{
			require_token(m_tok_param_left);
			ttd->m_desc.add(get_string());
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_TT_LINE))
		{
			require_token(m_tok_param_left);
			ttd->m_desc.add(get_string());
			require_token(m_tok_param_right);
		}
		else if (token.is(m_tok_TT_FAMILY))
		{
			require_token(m_tok_param_left);
			ttd->m_family = m_setup.family_from_model(get_string());
			require_token(m_tok_param_right);
		}
		else
		{
			require_token(token, m_tok_TRUTHTABLE_END);
			require_token(m_tok_param_left);
			require_token(m_tok_param_right);
			m_setup.factory().register_device(ttd);
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
			error(pfmt("expected a comma, found <{1}>")(n.str()) );
	}

}

void parser_t::dippins()
{
	pstring_list_t pins;

	pins.add(get_identifier());
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		pins.add(t1);
		token_t n = get_token();
		if (n.is(m_tok_param_right))
			break;
		if (!n.is(m_tok_comma))
			error(pfmt("expected a comma, found <{1}>")(n.str()) );
	}
	if ((pins.size() % 2) == 1)
		error("You must pass an equal number of pins to DIPPINS");
	unsigned n = pins.size();
	for (unsigned i = 0; i < n / 2; i++)
	{
		m_setup.register_alias(pfmt("{1}")(i+1), pins[i*2]);
		m_setup.register_alias(pfmt("{1}")(n-i), pins[i*2 + 1]);
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

void parser_t::device(const pstring &dev_type)
{
	if (m_setup.is_library_item(dev_type))
	{
		pstring devname = get_identifier();
		m_setup.namespace_push(devname);
		m_setup.include(dev_type);
		m_setup.namespace_pop();
		require_token(m_tok_param_right);
	}
	else
	{
		base_factory_t *f = m_setup.factory().factory_by_name(dev_type);
		device_t *dev;
		pstring_list_t termlist = f->term_param_list();
		pstring_list_t def_params = f->def_params();

		std::size_t cnt;

		pstring devname = get_identifier();

		dev = f->Create();
		m_setup.register_dev(dev, devname);

		m_setup.log().debug("Parser: IC: {1}\n", devname);

		cnt = 0;
		while (cnt < def_params.size())
		{
			pstring paramfq = devname + "." + def_params[cnt];

			m_setup.log().debug("Defparam: {1}\n", paramfq);
			require_token(m_tok_comma);
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
			cnt++;
		}

		token_t tok = get_token();
		cnt = 0;
		while (tok.is(m_tok_comma) && cnt < termlist.size())
		{
			pstring output_name = get_identifier();

			m_setup.register_link(devname + "." + termlist[cnt], output_name);

			cnt++;
			tok = get_token();
		}
		if (cnt != termlist.size())
			m_setup.log().fatal("netlist: input count mismatch for {1} - expected {2} found {3}\n", devname, termlist.size(), cnt);
		require_token(tok, m_tok_param_right);
	}
}


// ----------------------------------------------------------------------------------------
// private
// ----------------------------------------------------------------------------------------


nl_double parser_t::eval_param(const token_t tok)
{
	static const char *macs[6] = {"", "RES_K", "RES_M", "CAP_U", "CAP_N", "CAP_P"};
	static nl_double facs[6] = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
	int i;
	int f=0;
	bool e;
	nl_double ret;
	pstring val;

	//printf("param {1}\n", tok.m_token);
	for (i=1; i<6;i++)
		if (tok.str().equals(macs[i]))
			f = i;
#if 1
	if (f>0)
	{
		require_token(m_tok_param_left);
		ret = get_number_double();
		require_token(m_tok_param_right);
	}
	else
	{
		val = tok.str();
		ret = val.as_double(&e);
		if (e)
			error("Error with parameter ...\n");
	}
	return ret * facs[f];

#else
	if (f>0)
	{
		require_token(m_tok_param_left);
		val = get_identifier();
	}
	else
		val = tok.str();

	ret = val.as_double(&e);

	if (e)
		fatal("Error with parameter ...\n");
	if (f>0)
		require_token(m_tok_param_right);
	return ret * facs[f];
#endif
}
}

#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic pop
#endif
