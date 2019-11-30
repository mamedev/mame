// license:GPL-2.0+
// copyright-holders:Couriersud

#include "nl_parser.h"
#include "nl_base.h"
#include "nl_errstr.h"
#include "nl_factory.h"

namespace netlist
{
// ----------------------------------------------------------------------------------------
// A netlist parser
// ----------------------------------------------------------------------------------------

void parser_t::verror(const pstring &msg)
{
	m_setup.log().fatal("{1}", msg);
	plib::pthrow<nl_exception>(plib::pfmt("{1}")(msg));
}

bool parser_t::parse(const pstring &nlname)
{
	this->identifier_chars("abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-")
		.number_chars(".0123456789", "0123456789eE-.") //FIXME: processing of numbers
		.whitespace(pstring("") + ' ' + static_cast<char>(9) + static_cast<char>(10) + static_cast<char>(13))
		.comment("/*", "*/", "//");
	m_tok_paren_left = register_token("(");
	m_tok_paren_right = register_token(")");
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

	register_token("RES_R");
	register_token("RES_K");
	register_token("RES_M");
	register_token("CAP_U");
	register_token("CAP_N");
	register_token("CAP_P");

	bool in_nl = false;

	while (true)
	{
		token_t token = get_token();
		if (token.is_type(token_type::ENDOFFILE))
		{
			return false;
		}

		if (token.is(m_tok_NETLIST_END))
		{
			require_token(m_tok_paren_left);
			if (!in_nl)
				error (MF_UNEXPECTED_NETLIST_END());
			else
			{
				in_nl = false;
			}
			require_token(m_tok_paren_right);
		}
		else if (token.is(m_tok_NETLIST_START))
		{
			if (in_nl)
				error (MF_UNEXPECTED_NETLIST_START());
			require_token(m_tok_paren_left);
			token_t name = get_token();
			require_token(m_tok_paren_right);
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

		if (token.is_type(token_type::ENDOFFILE))
			error (MF_UNEXPECTED_END_OF_FILE());

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
			require_token(m_tok_paren_left);
			m_setup.register_lib_entry(get_identifier(), "parser: " + nlname);
			require_token(m_tok_paren_right);
		}
		else if (token.is(m_tok_NETLIST_END))
		{
			netdev_netlist_end();
			return;
		}
		else if (!token.is_type(token_type::IDENTIFIER))
			error(MF_EXPECTED_IDENTIFIER_GOT_1(token.str()));
		else
			device(token.str());
	}
}

void parser_t::net_truthtable_start(const pstring &nlname)
{
	require_token(m_tok_paren_left);
	pstring name(get_identifier());
	bool head_found(false);

	require_token(m_tok_comma);
	long ni = get_number_long();
	require_token(m_tok_comma);
	long no = get_number_long();
	require_token(m_tok_comma);
	pstring def_param = get_string();
	require_token(m_tok_paren_right);

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
			require_token(m_tok_paren_left);
			desc.desc.push_back(get_string());
			require_token(m_tok_paren_right);
			head_found = true;
		}
		else if (token.is(m_tok_TT_LINE))
		{
			if (!head_found)
				error(MF_TT_LINE_WITHOUT_HEAD());
			require_token(m_tok_paren_left);
			desc.desc.push_back(get_string());
			require_token(m_tok_paren_right);
		}
		else if (token.is(m_tok_TT_FAMILY))
		{
			require_token(m_tok_paren_left);
			desc.family = get_string();
			require_token(m_tok_paren_right);
		}
		else
		{
			require_token(token, m_tok_TRUTHTABLE_END);
			require_token(m_tok_paren_left);
			require_token(m_tok_paren_right);
			m_setup.truthtable_create(desc, nlname);
			return;
		}
	}
}

void parser_t::netdev_netlist_end()
{
	// don't do much
	require_token(m_tok_paren_left);
	require_token(m_tok_paren_right);
}

void parser_t::net_model()
{
	require_token(m_tok_paren_left);
	pstring model = get_string();
	m_setup.register_model(model);
	require_token(m_tok_paren_right);
}

void parser_t::net_submodel()
{
	require_token(m_tok_paren_left);
	pstring model(get_identifier());
	require_token(m_tok_comma);
	pstring name = get_identifier();
	require_token(m_tok_paren_right);

	m_setup.namespace_push(name);
	m_setup.include(model);
	m_setup.namespace_pop();
}

void parser_t::frontier()
{
	require_token(m_tok_paren_left);
	// don't do much
	pstring attachat = get_identifier();
	require_token(m_tok_comma);
	nl_fptype r_IN = eval_param(get_token());
	require_token(m_tok_comma);
	nl_fptype r_OUT = eval_param(get_token());
	require_token(m_tok_paren_right);

	m_setup.register_frontier(attachat, r_IN, r_OUT);
}

void parser_t::net_include()
{
	require_token(m_tok_paren_left);
	pstring name(get_identifier());
	require_token(m_tok_paren_right);

	m_setup.include(name);
}

void parser_t::net_local_source()
{
	// This directive is only for hardcoded netlists. Ignore it here.
	require_token(m_tok_paren_left);
	pstring name(get_identifier());
	require_token(m_tok_paren_right);

}

void parser_t::net_alias()
{
	require_token(m_tok_paren_left);
	pstring alias = get_identifier_or_number();

	require_token(m_tok_comma);

	pstring out = get_identifier();

	require_token(m_tok_paren_right);

	m_setup.log().debug("Parser: Alias: {1} {2}\n", alias, out);
	m_setup.register_alias(alias, out);
}

void parser_t::net_c()
{
	require_token(m_tok_paren_left);
	pstring first = get_identifier();
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		m_setup.register_link(first , t1);
		m_setup.log().debug("Parser: Connect: {1} {2}\n", first, t1);
		token_t n = get_token();
		if (n.is(m_tok_paren_right))
			break;
		if (!n.is(m_tok_comma))
			error(MF_EXPECTED_COMMA_OR_RP_1(n.str()));
	}

}

void parser_t::dippins()
{
	std::vector<pstring> pins;

	require_token(m_tok_paren_left);
	pins.push_back(get_identifier());
	require_token(m_tok_comma);

	while (true)
	{
		pstring t1 = get_identifier();
		pins.push_back(t1);
		token_t n = get_token();
		if (n.is(m_tok_paren_right))
			break;
		if (!n.is(m_tok_comma))
			error(MF_EXPECTED_COMMA_OR_RP_1(n.str()));
	}
	if ((pins.size() % 2) == 1)
		error(MF_DIPPINS_EQUAL_NUMBER_1(pins[0]));
	std::size_t n = pins.size();
	for (std::size_t i = 0; i < n / 2; i++)
	{
		m_setup.register_alias(plib::pfmt("{1}")(i+1), pins[i*2]);
		m_setup.register_alias(plib::pfmt("{1}")(n-i), pins[i*2 + 1]);
	}
}

void parser_t::netdev_param()
{
	require_token(m_tok_paren_left);
	pstring param(get_identifier());
	require_token(m_tok_comma);
	token_t tok = get_token();
	if (tok.is_type(token_type::STRING))
	{
		m_setup.log().debug("Parser: Param: {1} {2}\n", param, tok.str());
		m_setup.register_param(param, tok.str());
	}
	else
	{
		nl_fptype val = eval_param(tok);
		m_setup.log().debug("Parser: Param: {1} {2}\n", param, val);
		m_setup.register_param(param, val);
	}
	require_token(m_tok_paren_right);
}

void parser_t::netdev_hint()
{
	require_token(m_tok_paren_left);
	pstring dev(get_identifier());
	require_token(m_tok_comma);
	pstring hint(get_identifier());
	m_setup.register_param(dev + ".HINT_" + hint, 1);
	require_token(m_tok_paren_right);
}

void parser_t::device(const pstring &dev_type)
{
	std::vector<pstring> params;

	require_token(m_tok_paren_left);
	pstring devname(get_identifier());

	m_setup.log().debug("Parser: IC: {1}\n", devname);

	auto tok(get_token());

	while (tok.is(m_tok_comma))
	{
		tok = get_token();
		if (tok.is_type(token_type::IDENTIFIER) || tok.is_type(token_type::STRING))
			params.push_back(tok.str());
		else
		{
			// FIXME: Do we really need this?
			nl_fptype value = eval_param(tok);
			if (plib::abs(value - plib::floor(value)) > nlconst::magic(1e-30)
				|| plib::abs(value) > nlconst::magic(1e9))
				params.push_back(plib::pfmt("{1:.9}").e(value));
			else
				params.push_back(plib::pfmt("{1}")(static_cast<long>(value)));
		}
		tok = get_token();
	}

	require_token(tok, m_tok_paren_right);
	m_setup.register_dev(dev_type, devname, params);
}


// ----------------------------------------------------------------------------------------
// private
// ----------------------------------------------------------------------------------------

nl_fptype parser_t::eval_param(const token_t &tok)
{
	static std::array<pstring, 7> macs = {"", "RES_R", "RES_K", "RES_M", "CAP_U", "CAP_N", "CAP_P"};
	static std::array<nl_fptype, 7> facs = {
		nlconst::magic(1.0),
		nlconst::magic(1.0),
		nlconst::magic(1e3),
		nlconst::magic(1e6),
		nlconst::magic(1e-6),
		nlconst::magic(1e-9),
		nlconst::magic(1e-12)
	};
	std::size_t f=0;
	nl_fptype ret(0);

	for (std::size_t i=1; i<macs.size();i++)
		if (tok.str() == macs[i])
			f = i;
	if (f>0)
	{
		require_token(m_tok_paren_left);
		ret = static_cast<nl_fptype>(get_number_double());
		require_token(m_tok_paren_right);
	}
	else
	{
		bool err(false);
		ret = plib::pstonum_ne<nl_fptype>(tok.str(), err);
		if (err)
			error(MF_PARAM_NOT_FP_1(tok.str()));
	}
	return ret * facs[f];

}

} // namespace netlist
