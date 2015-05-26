// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nl_parser.c
 *
 */

#include "nl_parser.h"
#include "nl_factory.h"

//#undef NL_VERBOSE_OUT
//#define NL_VERBOSE_OUT(x) printf x

// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

ATTR_COLD void netlist_parser::verror(pstring msg, int line_num, pstring line)
{
	m_setup.netlist().error("line %d: error: %s\n\t\t%s\n", line_num,
			msg.cstr(), line.cstr());

	//throw error;
}


bool netlist_parser::parse(const char *buf, const pstring nlname)
{
	ppreprocessor prepro;

	pstring processed = prepro.process(buf);
	m_buf = processed.cstr();

	reset(m_buf);
	set_identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-");
	set_number_chars("01234567890eE-."); //FIXME: processing of numbers
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
	m_tok_NET_C = register_token("NET_C");
	m_tok_PARAM = register_token("PARAM");
	m_tok_NET_MODEL = register_token("NET_MODEL");
	m_tok_INCLUDE = register_token("INCLUDE");
	m_tok_SUBMODEL = register_token("SUBMODEL");
	m_tok_NETLIST_START = register_token("NETLIST_START");
	m_tok_NETLIST_END = register_token("NETLIST_END");

	bool in_nl = false;

	while (true)
	{
		token_t token = get_token();

		if (token.is_type(ENDOFFILE))
		{
			return false;
			//error("EOF while searching for <%s>", nlname.cstr());
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

void netlist_parser::parse_netlist(ATTR_UNUSED const pstring &nlname)
{
	while (true)
	{
		token_t token = get_token();

		if (token.is_type(ENDOFFILE))
			return;

		require_token(m_tok_param_left);
		NL_VERBOSE_OUT(("Parser: Device: %s\n", token.str().cstr()));

		if (token.is(m_tok_ALIAS))
			net_alias();
		else if (token.is(m_tok_NET_C))
			net_c();
		else if (token.is(m_tok_PARAM))
			netdev_param();
		else if (token.is(m_tok_NET_MODEL))
			net_model();
		else if (token.is(m_tok_SUBMODEL))
			net_submodel();
		else if (token.is(m_tok_INCLUDE))
			net_include();
		else if (token.is(m_tok_NETLIST_END))
		{
			netdev_netlist_end();
			return;
		}
		else
			device(token.str());
	}
}


void netlist_parser::netdev_netlist_start()
{
	// don't do much
	token_t name = get_token();
	require_token(m_tok_param_right);
}

void netlist_parser::netdev_netlist_end()
{
	// don't do much
	require_token(m_tok_param_right);
}

void netlist_parser::net_model()
{
	// don't do much
	pstring model = get_string();
	m_setup.register_model(model);
	require_token(m_tok_param_right);
}

void netlist_parser::net_submodel()
{
	// don't do much
	pstring name = get_identifier();
	require_token(m_tok_comma);
	pstring model = get_identifier();
	require_token(m_tok_param_right);

	m_setup.namespace_push(name);
	netlist_parser subparser(m_setup);
	subparser.parse(m_buf, model);
	m_setup.namespace_pop();
}

void netlist_parser::net_include()
{
	// don't do much
	pstring name = get_identifier();
	require_token(m_tok_param_right);

	netlist_parser subparser(m_setup);
	subparser.parse(m_buf, name);
}

void netlist_parser::net_alias()
{
	pstring alias = get_identifier();

	require_token(m_tok_comma);

	pstring out = get_identifier();

	require_token(m_tok_param_right);

	NL_VERBOSE_OUT(("Parser: Alias: %s %s\n", alias.cstr(), out.cstr()));
	m_setup.register_alias(alias, out);
}

void netlist_parser::net_c()
{
	pstring first = get_identifier();
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		m_setup.register_link(first , t1);
		NL_VERBOSE_OUT(("Parser: Connect: %s %s\n", last.cstr(), t1.cstr()));
		token_t n = get_token();
		if (n.is(m_tok_param_right))
			break;
		if (!n.is(m_tok_comma))
			error("expected a comma, found <%s>", n.str().cstr());
	}

}

void netlist_parser::netdev_param()
{
	pstring param;
	nl_double val;
	param = get_identifier();
	require_token(m_tok_comma);
	val = eval_param(get_token());
	NL_VERBOSE_OUT(("Parser: Param: %s %f\n", param.cstr(), val));
	m_setup.register_param(param, val);
	require_token(m_tok_param_right);
}

void netlist_parser::device(const pstring &dev_type)
{
	pstring devname;
	net_device_t_base_factory *f = m_setup.factory().factory_by_name(dev_type, m_setup);
	netlist_device_t *dev;
	nl_util::pstring_list termlist = f->term_param_list();
	nl_util::pstring_list def_params = f->def_params();

	std::size_t cnt;

	devname = get_identifier();

	dev = f->Create();
	m_setup.register_dev(dev, devname);

	NL_VERBOSE_OUT(("Parser: IC: %s\n", devname.cstr()));

	cnt = 0;
	while (cnt < def_params.size())
	{
		pstring paramfq = devname + "." + def_params[cnt];

		NL_VERBOSE_OUT(("Defparam: %s\n", paramfq.cstr()));
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
		m_setup.netlist().error("netlist: input count mismatch for %s - expected %" SIZETFMT " found %" SIZETFMT "\n", devname.cstr(), termlist.size(), cnt);
	require_token(tok, m_tok_param_right);
}


// ----------------------------------------------------------------------------------------
// private
// ----------------------------------------------------------------------------------------


nl_double netlist_parser::eval_param(const token_t tok)
{
	static const char *macs[6] = {"", "RES_K", "RES_M", "CAP_U", "CAP_N", "CAP_P"};
	static nl_double facs[6] = {1, 1e3, 1e6, 1e-6, 1e-9, 1e-12};
	int i;
	int f=0;
	bool e;
	nl_double ret;
	pstring val;

	//printf("param %s\n", tok.m_token.cstr());
	for (i=1; i<6;i++)
		if (tok.str().equals(macs[i]))
			f = i;
	if (f>0)
	{
		require_token(m_tok_param_left);
		val = get_identifier();
	}
	else
		val = tok.str();

	ret = val.as_double(&e);

	if (e)
		error("Error with parameter ...\n");
	if (f>0)
		require_token(m_tok_param_right);
	return ret * facs[f];
}
