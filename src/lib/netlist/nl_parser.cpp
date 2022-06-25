// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "nl_parser.h"

#include "nl_errstr.h"
#include "nl_factory.h"
#include "nl_setup.h"

namespace netlist
{
	// -------------------------------------------------------------------------
	// A netlist parser
	// -------------------------------------------------------------------------

	void parser_t::verror(const pstring &msg)
	{
		m_setup.log().fatal("{1}", msg);
		throw nl_exception(plib::pfmt("{1}")(msg));
	}

	parser_t::parser_t(nlparse_t &setup)
	: m_setup(setup)
	, m_cur_local(nullptr)
	{
		m_tokenizer
			.identifier_chars(
				"abcdefghijklmnopqrstuvwvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_.-$@")
			.number_chars(".0123456789", "0123456789eE-.") // FIXME: processing
														   // of numbers
			.whitespace(pstring("") + ' ' + static_cast<char>(9)
						+ static_cast<char>(10) + static_cast<char>(13))
			.comment("/*", "*/", "//");
		m_tok_paren_left = m_tokenizer.register_token("(");
		m_tok_paren_right = m_tokenizer.register_token(")");
		m_tok_comma = m_tokenizer.register_token(",");

		m_tok_static = m_tokenizer.register_token("static");
		m_tok_ALIAS = m_tokenizer.register_token("ALIAS");
		m_tok_DIPPINS = m_tokenizer.register_token("DIPPINS");
		m_tok_NET_C = m_tokenizer.register_token("NET_C");
		m_tok_FRONTIER = m_tokenizer.register_token("OPTIMIZE_FRONTIER");
		m_tok_PARAM = m_tokenizer.register_token("PARAM");
		m_tok_DEFPARAM = m_tokenizer.register_token("DEFPARAM");
		m_tok_HINT = m_tokenizer.register_token("HINT");
		m_tok_NET_MODEL = m_tokenizer.register_token("NET_MODEL");
		m_tok_NET_REGISTER_DEV = m_tokenizer.register_token("NET_REGISTER_DEV");
		m_tok_INCLUDE = m_tokenizer.register_token("INCLUDE");
		m_tok_LOCAL_SOURCE = m_tokenizer.register_token("LOCAL_SOURCE");
		m_tok_LOCAL_LIB_ENTRY = m_tokenizer.register_token("LOCAL_LIB_ENTRY");
		m_tok_EXTERNAL_LIB_ENTRY = m_tokenizer.register_token(
			"EXTERNAL_LIB_ENTRY");
		m_tok_SUBMODEL = m_tokenizer.register_token("SUBMODEL");
		m_tok_NETLIST_START = m_tokenizer.register_token("NETLIST_START");
		m_tok_NETLIST_END = m_tokenizer.register_token("NETLIST_END");
		m_tok_NETLIST_EXTERNAL = m_tokenizer.register_token("NETLIST_EXTERNAL");
		m_tok_EXTERNAL_SOURCE = m_tokenizer.register_token("EXTERNAL_SOURCE");
		m_tok_TRUTHTABLE_START = m_tokenizer.register_token("TRUTHTABLE_START");
		m_tok_TRUTHTABLE_END = m_tokenizer.register_token("TRUTHTABLE_END");
		m_tok_TRUTHTABLE_ENTRY = m_tokenizer.register_token("TRUTHTABLE_ENTRY");
		m_tok_TT_HEAD = m_tokenizer.register_token("TT_HEAD");
		m_tok_TT_LINE = m_tokenizer.register_token("TT_LINE");
		m_tok_TT_FAMILY = m_tokenizer.register_token("TT_FAMILY");

		m_tokenizer.register_token("RES_R");
		m_tokenizer.register_token("RES_K");
		m_tokenizer.register_token("RES_M");
		m_tokenizer.register_token("CAP_U");
		m_tokenizer.register_token("CAP_N");
		m_tokenizer.register_token("CAP_P");
	}

	bool parser_t::parse(plib::istream_uptr &&strm, const pstring &nlname)
	{
		token_store_t tokstor;
		parse_tokens(std::move(strm), tokstor);
		return parse(tokstor, nlname);
	}

	void parser_t::parse_tokens(plib::istream_uptr &&strm,
		token_store_t &                              tokstor)
	{
		plib::putf8_reader u8reader(strm.release_stream());
		m_tokenizer.append_to_store(&u8reader, tokstor);
	}

	bool parser_t::parse(const token_store_t &store, const pstring &nlname)
	{
		set_token_source(&store);

		bool in_nl = false;

		while (true)
		{
			// FIXME: line numbers in cached local netlists are wrong
			//        need to process raw tokens here.
			token_t token = get_token_raw();
			if (token.is_type(token_type::ENDOFFILE))
			{
				return false;
			}

			if (token.is(m_tok_NETLIST_END) || token.is(m_tok_TRUTHTABLE_END))
			{
				if (!in_nl)
					error(MF_PARSER_UNEXPECTED_1(token.str()));
				else
				{
					in_nl = false;
				}
				require_token(m_tok_paren_left);
				require_token(m_tok_paren_right);

				m_cur_local->push_back(token);
				m_cur_local->push_back(token_t(m_tok_paren_left));
				m_cur_local->push_back(token_t(m_tok_paren_right));
			}
			else if (token.is(m_tok_NETLIST_START)
					 || token.is(m_tok_TRUTHTABLE_START))
			{
				if (in_nl)
					error(MF_PARSER_UNEXPECTED_1(token.str()));
				require_token(m_tok_paren_left);
				token_t name = get_token();
				if (token.is(m_tok_NETLIST_START)
					&& (name.str() == nlname || nlname.empty()))
				{
					require_token(m_tok_paren_right);
					parse_netlist();
					return true;
				}
				if (token.is(m_tok_TRUTHTABLE_START) && name.str() == nlname)
				{
					net_truth_table_start(nlname);
					return true;
				}

				// create a new cached local store
				m_local.emplace(name.str(), token_store_t());
				m_cur_local = &m_local[name.str()];
				auto sl = location();
				auto li = plib::pfmt(
					"# {1} \"{2}\"")(sl.line(), sl.file_name());

				m_cur_local->push_back(token_t(token_type::LINEMARKER, li));
				m_cur_local->push_back(token);
				m_cur_local->push_back(token_t(m_tok_paren_left));
				m_cur_local->push_back(name);
				// m_cur_local->push_back(token_t(m_tok_paren_right));
				in_nl = true;
			}
			// FIXME: do we really need this going forward ? there should be no
			// need
			//        for NETLIST_EXTERNAL in netlist files
			else if (token.is(m_tok_NETLIST_EXTERNAL))
			{
				if (in_nl)
					error(MF_UNEXPECTED_NETLIST_EXTERNAL());
				require_token(m_tok_paren_left);
				token_t name = get_token();
				require_token(m_tok_paren_right);
			}
			else if (!in_nl)
			{
				if (!token.is(m_tok_static)
					&& !token.is_type(token_type::SOURCELINE)
					&& !token.is_type(token_type::LINEMARKER))
					error(MF_EXPECTED_NETLIST_START_1(token.str()));
			}
			else
			{
				m_cur_local->push_back(token);
			}
		}
	}

	void parser_t::parse_netlist()
	{
		while (true)
		{
			token_t token = get_token();

			if (token.is_type(token_type::ENDOFFILE))
				error(MF_UNEXPECTED_END_OF_FILE());

			m_setup.log().debug("Parser: Device: {1}\n", token.str());

			if (token.is(m_tok_ALIAS))
				net_alias();
			else if (token.is(m_tok_DIPPINS))
				dip_pins();
			else if (token.is(m_tok_NET_C))
				net_c();
			else if (token.is(m_tok_FRONTIER))
				frontier();
			else if (token.is(m_tok_PARAM))
				netdev_param();
			else if (token.is(m_tok_DEFPARAM))
				netdev_default_param();
			else if (token.is(m_tok_HINT))
				netdev_hint();
			else if (token.is(m_tok_NET_MODEL))
				net_model();
			else if (token.is(m_tok_SUBMODEL))
				net_sub_model();
			else if (token.is(m_tok_INCLUDE))
				net_include();
			else if (token.is(m_tok_LOCAL_SOURCE))
				net_local_source();
			else if (token.is(m_tok_EXTERNAL_SOURCE))
				net_external_source();
			else if (token.is(m_tok_LOCAL_LIB_ENTRY))
				net_lib_entry(true);
			else if (token.is(m_tok_EXTERNAL_LIB_ENTRY))
				net_lib_entry(false);
			else if (token.is(m_tok_TRUTHTABLE_ENTRY))
			{
				require_token(m_tok_paren_left);
				pstring name(get_identifier());
				register_local_as_source(name);
				m_setup.include(name);
				require_token(m_tok_paren_right);
			}
			else if (token.is(m_tok_NET_REGISTER_DEV))
			{
				net_register_dev();
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

	void parser_t::net_lib_entry(bool is_local)
	{
		require_token(m_tok_paren_left);
		pstring name(get_identifier());

		if (is_local)
			register_local_as_source(name);
		else if (m_local.find(name) != m_local.end())
			error(MF_EXTERNAL_SOURCE_IS_LOCAL_1(name));

		// FIXME: Need to pass in parameter definition FIXME: get line number
		// right
		m_setup.register_lib_entry(name, "", location());
		require_token(m_tok_paren_right);
	}

	void parser_t::net_truth_table_start(const pstring &nlname)
	{
		bool head_found(false);

		// parse remaining parameters
		require_token(m_tok_comma);
		long ni = get_number_long();
		require_token(m_tok_comma);
		long no = get_number_long();
		require_token(m_tok_comma);
		pstring def_param = get_string();
		require_token(m_tok_paren_right);

		netlist::tt_desc desc;
		desc.name = nlname;
		desc.ni = gsl::narrow<unsigned long>(ni);
		desc.no = gsl::narrow<unsigned long>(no);
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
				// FIXME: proper location
				m_setup.truth_table_create(desc, def_param, location());
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

	void parser_t::net_sub_model()
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
		pstring attach_at = get_identifier();
		require_token(m_tok_comma);
		auto tok = get_token();
		auto r_IN = stringify_expression(tok);
		require_token(tok, m_tok_comma);
		tok = get_token();
		auto r_OUT = stringify_expression(tok);
		require_token(tok, m_tok_paren_right);

		m_setup.register_frontier(attach_at, r_IN, r_OUT);
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

		register_local_as_source(name);
	}

	void parser_t::register_local_as_source(const pstring &name)
	{
		auto p = m_local.find(name);
		if (p != m_local.end())
			m_setup.register_source<source_token_t>(name, p->second);
		else
			error(MF_LOCAL_SOURCE_NOT_FOUND_1(name));
	}

	void parser_t::net_external_source()
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
			m_setup.register_link(first, t1);
			m_setup.log().debug("Parser: Connect: {1} {2}\n", first, t1);
			token_t n = get_token();
			if (n.is(m_tok_paren_right))
				break;
			if (!n.is(m_tok_comma))
				error(MF_EXPECTED_COMMA_OR_RP_1(n.str()));
		}
	}

	void parser_t::dip_pins()
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
			m_setup.register_alias(plib::pfmt("{1}")(i + 1), pins[i * 2]);
			m_setup.register_alias(plib::pfmt("{1}")(n - i), pins[i * 2 + 1]);
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
			require_token(m_tok_paren_right);
		}
		else
		{
			auto val = stringify_expression(tok);
			m_setup.log().debug("Parser: Param: {1} {2}\n", param, val);
			m_setup.register_param(param, val);
			require_token(tok, m_tok_paren_right);
		}
	}

	void parser_t::netdev_default_param()
	{
		require_token(m_tok_paren_left);
		pstring param(get_identifier());
		require_token(m_tok_comma);
		token_t tok = get_token();
		if (tok.is_type(token_type::STRING))
		{
			m_setup.log().debug("Parser: DefParam: {1} {2}\n", param,
				tok.str());
			m_setup.register_default_param(param, tok.str());
			require_token(m_tok_paren_right);
		}
		else
		{
			auto val = stringify_expression(tok);
			m_setup.log().debug("Parser: Param: {1} {2}\n", param, val);
			m_setup.register_default_param(param, val);
			require_token(tok, m_tok_paren_right);
		}
	}

	void parser_t::netdev_hint()
	{
		require_token(m_tok_paren_left);
		pstring id(get_identifier());
		require_token(m_tok_comma);
		pstring hint(get_identifier());
		m_setup.register_hint(id, ".HINT_" + hint);
		require_token(m_tok_paren_right);
	}

	void parser_t::net_register_dev()
	{
		require_token(m_tok_paren_left);
		pstring type(get_identifier());
		require_token(m_tok_comma);
		pstring name(get_identifier());
		require_token(m_tok_paren_right);

		m_setup.register_dev(type, name);
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
			if (tok.is_type(token_type::STRING))
			{
				params.push_back(tok.str());
				tok = get_token();
			}
			else
			{
				auto value = stringify_expression(tok);
				params.push_back(value);
			}
		}

		require_token(tok, m_tok_paren_right);
		m_setup.register_dev(dev_type, devname, params);
	}

	// -------------------------------------------------------------------------
	// private
	// -------------------------------------------------------------------------

	pstring parser_t::stringify_expression(token_t &tok)
	{
		int     pc(0);
		pstring ret;
		while (!tok.is(m_tok_comma))
		{
			if (tok.is(m_tok_paren_left))
				pc++;
			else if (tok.is(m_tok_paren_right))
			{
				if (pc <= 0)
					break;
				pc--;
			}
			ret += tok.str();
			tok = get_token();
		}
		return ret;
	}

	// -------------------------------------------------------------------------
	// source_token_t
	// -------------------------------------------------------------------------

	bool source_token_t::parse(nlparse_t &setup, const pstring &name)
	{
		if (name == m_name)
		{
			auto ret = setup.parse_tokens(m_store, name);
			return ret;
		}

		return false;
	}

	plib::istream_uptr source_token_t::stream(
		[[maybe_unused]] const pstring &name)
	{
		return plib::istream_uptr();
	}

} // namespace netlist
