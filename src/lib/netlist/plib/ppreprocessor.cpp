// license:GPL-2.0+
// copyright-holders:Couriersud

#include "ppreprocessor.h"
#include "palloc.h"
#include "pstonum.h"
#include "putil.h"

namespace plib {

	// ----------------------------------------------------------------------------------------
	// A simple preprocessor
	// ----------------------------------------------------------------------------------------

	ppreprocessor::ppreprocessor(psource_collection_t<> &sources, defines_map_type *defines)
	: std::istream(new readbuffer(this))
	, m_sources(sources)
	, m_if_flag(0)
	, m_if_level(0)
	, m_pos(0)
	, m_state(PROCESS)
	, m_comment(false)
	{
		m_expr_sep.emplace_back("!");
		m_expr_sep.emplace_back("(");
		m_expr_sep.emplace_back(")");
		m_expr_sep.emplace_back("+");
		m_expr_sep.emplace_back("-");
		m_expr_sep.emplace_back("*");
		m_expr_sep.emplace_back("/");
		m_expr_sep.emplace_back("&&");
		m_expr_sep.emplace_back("||");
		m_expr_sep.emplace_back("==");
		m_expr_sep.emplace_back(",");
		m_expr_sep.emplace_back(";");
		m_expr_sep.emplace_back(".");
		m_expr_sep.emplace_back("##");
		m_expr_sep.emplace_back("#");
		m_expr_sep.emplace_back(" ");
		m_expr_sep.emplace_back("\t");
		m_expr_sep.emplace_back("\"");

		if (defines != nullptr)
			m_defines = *defines;
		m_defines.insert({"__PLIB_PREPROCESSOR__", define_t("__PLIB_PREPROCESSOR__", "1")});
		auto idx = m_defines.find("__PREPROCESSOR_DEBUG__");
		m_debug_out = idx != m_defines.end();

	}

	void ppreprocessor::error(const pstring &err)
	{
		pstring s("");
		pstring trail      ("                 from ");
		pstring trail_first("In file included from ");
		pstring e = plib::pfmt("{1}:{2}:0: error: {3}\n")
				(m_stack.back().m_name, m_stack.back().m_lineno, err);
		m_stack.pop_back();
		while (m_stack.size() > 0)
		{
			if (m_stack.size() == 1)
				trail = trail_first;
			s = trail + plib::pfmt("{1}:{2}:0\n")(m_stack.back().m_name, m_stack.back().m_lineno) + s;
			m_stack.pop_back();
		}
		pthrow<pexception>("\n" + s + e + " " + m_line + "\n");
	}

	template <typename PP, typename L = ppreprocessor::string_list>
	struct simple_iter
	{
		simple_iter(PP *parent, const L &tokens)
		: m_tokens(tokens), m_parent(parent), m_pos(0)
		{}

		/// \brief skip white space in token list
		///
		void skip_ws()
		{
			while (m_pos < m_tokens.size() && (m_tokens[m_pos] == " " || m_tokens[m_pos] == "\t"))
				m_pos++;
		}

		/// \brief return next token skipping white space
		///
		/// \return next token
		///
		pstring next()
		{
			skip_ws();
			if (m_pos >= m_tokens.size())
				error("unexpected end of line");
			return m_tokens[m_pos++];
		}

		/// \brief return next token including white space
		///
		/// \return next token
		///
		pstring next_ws()
		{
			if (m_pos >= m_tokens.size())
				error("unexpected end of line");
			return m_tokens[m_pos++];
		}

		pstring peek_ws()
		{
			if (m_pos >= m_tokens.size())
				error("unexpected end of line");
			return m_tokens[m_pos];
		}

		pstring last()
		{
			if (m_pos == 0)
				error("no last token at beginning of line");
			if (m_pos > m_tokens.size())
				error("unexpected end of line");
			return m_tokens[m_pos-1];
		}

		bool eod()
		{
			return (m_pos >= m_tokens.size());
		}

		void error(pstring err)
		{
			m_parent->error(err);
		}
	private:
		L m_tokens;
		PP *m_parent;
		std::size_t m_pos;
	};

	#define CHECKTOK2(p_op, p_prio) \
		else if (tok == # p_op)                         \
		{                                               \
			if (!has_val)                               \
				{ sexpr.error("parsing error!"); return 1;} \
			if (prio < (p_prio))                        \
				return val;                             \
			sexpr.next();                                    \
			const auto v2 = prepro_expr(sexpr, (p_prio)); \
			val = (val p_op v2);                        \
		}                                               \

	// Operator precedence see https://en.cppreference.com/w/cpp/language/operator_precedence

	template <typename PP>
	static int prepro_expr(simple_iter<PP> &sexpr, int prio)
	{
		int val(0);
		bool has_val(false);

		pstring tok=sexpr.peek_ws();
		if (tok == "(")
		{
			sexpr.next();
			val = prepro_expr(sexpr, 255);
			if (sexpr.next() != ")")
				sexpr.error("expected ')'");
			has_val = true;
		}
		while (!sexpr.eod())
		{
			tok = sexpr.peek_ws();
			if (tok == ")")
			{
				if (!has_val)
					sexpr.error("Found ')' but have no value computed");
				else
					return val;
			}
			else if (tok == "!")
			{
				if (prio < 3)
				{
					if (!has_val)
						sexpr.error("parsing error!");
					else
						return val;
				}
				sexpr.next();
				val = !prepro_expr(sexpr, 3);
				has_val = true;
			}
			CHECKTOK2(*,  5)
			CHECKTOK2(/,  5) // NOLINT(clang-analyzer-core.DivideZero)
			CHECKTOK2(+,  6)
			CHECKTOK2(-,  6)
			CHECKTOK2(==, 10)
			CHECKTOK2(&&, 14)
			CHECKTOK2(||, 15)
			else
			{
				val = plib::pstonum<decltype(val)>(tok);
				has_val = true;
				sexpr.next();
			}
		}
		if (!has_val)
			sexpr.error("No value computed. Empty expression ?");
		return val;
	}

	ppreprocessor::define_t *ppreprocessor::get_define(const pstring &name)
	{
		auto idx = m_defines.find(name);
		return (idx != m_defines.end()) ? &idx->second : nullptr;
	}

	ppreprocessor::string_list ppreprocessor::tokenize(const pstring &str,
			const string_list &sep, bool remove_ws, bool concat)
	{
		const pstring STR = "\"";
		string_list tmpret;
		string_list tmp(psplit(str, sep));
		std::size_t pi(0);

		while (pi < tmp.size())
		{
			if (tmp[pi] == STR)
			{
				pstring s(STR);
				pi++;
				// FIXME : \"
				while (pi < tmp.size() && tmp[pi] != STR)
				{
					s += tmp[pi];
					pi++;
				}
				s += STR;
				tmpret.push_back(s);
			}
			else
				if (!remove_ws || (tmp[pi] != " " && tmp[pi] != "\t"))
					tmpret.push_back(tmp[pi]);
			pi++;
		}
		if (!concat)
			return tmpret;
		else
		{
			// FIXME: error if concat at beginning or end
			string_list ret;
			pi = 0;
			while (pi<tmpret.size())
			{
				if (tmpret[pi] == "##")
				{
					while (ret.back() == " " || ret.back() == "\t")
						ret.pop_back();
					pstring cc = ret.back();
					ret.pop_back();
					pi++;
					while (pi < tmpret.size() && (tmpret[pi] == " " || tmpret[pi] == "\t"))
						pi++;
					if (pi == tmpret.size())
						error("## found at end of sequence");
					ret.push_back(cc + tmpret[pi]);
				}
				else
					ret.push_back(tmpret[pi]);
				pi++;
			}
			return ret;
		}
	}

	bool ppreprocessor::is_valid_token(const pstring &str)
	{
		if (str.length() == 0)
			return false;
		pstring::value_type c(str.at(0));
		return ((c>='a' && c<='z') || (c>='A' && c<='Z') || c == '_');
	}

	pstring ppreprocessor::replace_macros(const pstring &line)
	{
		//std::vector<pstring> elems(psplit(line, m_expr_sep));
		bool repeat(false);
		pstring tmpret(line);
		do
		{
			repeat = false;
			auto elems(simple_iter<ppreprocessor>(this, tokenize(tmpret, m_expr_sep, false, true)));
			tmpret = "";
			while (!elems.eod())
			{
				auto token(elems.next_ws());
				define_t *def = get_define(token);
				if (def == nullptr)
					tmpret += token;
				else if (!def->m_has_params)
				{
					tmpret += def->m_replace;
					repeat = true;
				}
				else
				{
					token = elems.next();
					if (token != "(")
						error("expected '(' in macro expansion of " + def->m_name);
					string_list rep;
					token = elems.next();
					while (token != ")")
					{
						pstring par("");
						int pcnt(1);
						while (true)
						{
							if (pcnt==1 && token == ",")
							{
								token = elems.next();
								break;
							}
							if (token == "(")
								pcnt++;
							if (token == ")")
								if (--pcnt == 0)
									break;
							par += token;
							token = elems.next();
						}
						rep.push_back(par);
					}
					repeat = true;
					if (def->m_params.size() != rep.size())
						error(pfmt("Expected {1} parameters, got {2}")(def->m_params.size(), rep.size()));
					auto r(simple_iter<ppreprocessor>(this, tokenize(def->m_replace, m_expr_sep, false, false)));
					bool stringify_next = false;
					while (!r.eod())
					{
						token = r.next();
						if (token == "#")
							stringify_next = true;
						else if (token != " " && token != "\t")
						{
							for (std::size_t i=0; i<def->m_params.size(); i++)
								if (def->m_params[i] == token)
								{
									if (stringify_next)
									{
										stringify_next = false;
										token = "\"" + rep[i] + "\"";
									}
									else
										token = rep[i];
									break;
								}
							if (stringify_next)
								error("'#' is not followed by a macro parameter");
							tmpret += token;
							tmpret += " "; // make sure this is not concatenated with next token
						}
						else
							tmpret += token;
					}
				}
			}
		} while (repeat);

		return tmpret;
	}

	static pstring catremainder(const std::vector<pstring> &elems, std::size_t start, const pstring &sep)
	{
		pstring ret("");
		for (std::size_t i = start; i < elems.size(); i++)
		{
			ret += elems[i];
			ret += sep;
		}
		return ret;
	}

	pstring ppreprocessor::process_comments(pstring line)
	{
		bool in_string = false;

		std::size_t e = line.size();
		pstring ret = "";
		for (std::size_t i=0; i < e; )
		{
			pstring c = plib::left(line, 1);
			line = line.substr(1);
			if (!m_comment)
			{
				if (c=="\"")
				{
					in_string = !in_string;
					ret += c;
				}
				else if (in_string && c=="\\")
				{
					i++;
					ret += (c + plib::left(line, 1));
					line = line.substr(1);
				}
				else if (!in_string && c=="/" && plib::left(line,1) == "*")
					m_comment = true;
				else if (!in_string && c=="/" && plib::left(line,1) == "/")
					break;
				else
					ret += c;
			}
			else
				if (c=="*" && plib::left(line,1) == "/")
				{
					i++;
					line = line.substr(1);
					m_comment = false;
				}
			i++;
		}
		return ret;
	}

	std::pair<pstring,bool> ppreprocessor::process_line(pstring line)
	{
		bool line_cont = plib::right(line, 1) == "\\";
		if (line_cont)
			line = plib::left(line, line.size() - 1);

		if (m_state == LINE_CONTINUATION)
			m_line += line;
		else
			m_line = line;

		if (line_cont)
		{
			m_state = LINE_CONTINUATION;
			return {"", false};
		}
		else
			m_state = PROCESS;

		line = process_comments(m_line);

		pstring lt = plib::trim(plib::replace_all(line, "\t", " "));
		// FIXME ... revise and extend macro handling
		if (plib::startsWith(lt, "#"))
		{
			string_list lti(psplit(lt, " ", true));
			if (lti[0] == "#if")
			{
				m_if_level++;
				lt = replace_macros(lt);
				//std::vector<pstring> t(psplit(replace_all(lt.substr(3), " ", ""), m_expr_sep));
				auto t(simple_iter<ppreprocessor>(this, tokenize(lt.substr(3), m_expr_sep, true, true)));
				auto val = static_cast<int>(prepro_expr(t, 255));
				t.skip_ws();
				if (!t.eod())
					error("found unprocessed content at end of line");
				if (val == 0)
					m_if_flag |= (1 << m_if_level);
			}
			else if (lti[0] == "#ifdef")
			{
				m_if_level++;
				if (get_define(lti[1]) == nullptr)
					m_if_flag |= (1 << m_if_level);
			}
			else if (lti[0] == "#ifndef")
			{
				m_if_level++;
				if (get_define(lti[1]) != nullptr)
					m_if_flag |= (1 << m_if_level);
			}
			else if (lti[0] == "#else")
			{
				m_if_flag ^= (1 << m_if_level);
			}
			else if (lti[0] == "#endif")
			{
				m_if_flag &= ~(1 << m_if_level);
				m_if_level--;
			}
			else if (lti[0] == "#include")
			{
				if (m_if_flag == 0)
				{
					pstring arg("");
					for (std::size_t i=1; i<lti.size(); i++)
						arg += (lti[i] + " ");

					arg = plib::trim(arg);

					if (startsWith(arg, "\"") && endsWith(arg, "\""))
					{
						arg = arg.substr(1, arg.length() - 2);
						// first try local context
						auto l(plib::util::buildpath({m_stack.back().m_local_path, arg}));
						auto lstrm(m_sources.get_stream<>(l));
						if (lstrm)
						{
							m_stack.emplace_back(input_context(std::move(lstrm), plib::util::path(l), l));
						}
						else
						{
							auto strm(m_sources.get_stream<>(arg));
							if (strm)
							{
								m_stack.emplace_back(input_context(std::move(strm), plib::util::path(arg), arg));
							}
							else
								error("include not found:" + arg);
						}
					}
					else
						error("include misspelled:" + arg);
					pstring linemarker = pfmt("# {1} \"{2}\" 1\n")(m_stack.back().m_lineno, m_stack.back().m_name);
					push_out(linemarker);
				}
			}
			else if (lti[0] == "#pragma")
			{
				if (m_if_flag == 0 && lti.size() > 3 && lti[1] == "NETLIST")
				{
					if (lti[2] == "warning")
						error("NETLIST: " + catremainder(lti, 3, " "));
				}
			}
			else if (lti[0] == "#define")
			{
				if (m_if_flag == 0)
				{
					if (lti.size() < 2)
						error("define needs at least one argument");
					auto args(simple_iter<ppreprocessor>(this, tokenize(lt.substr(8), m_expr_sep, false, false)));
					pstring n = args.next();
					if (!is_valid_token(n))
						error("define expected identifier");
					if (args.next_ws() == "(")
					{
						define_t def(n);
						def.m_has_params = true;
						auto token(args.next());
						while (true)
						{
							if (token == ")")
								break;
							def.m_params.push_back(token);
							token = args.next();
							if (token != "," && token != ")")
								error(pfmt("expected , or ), found <{1}>")(token));
							if (token == ",")
								token = args.next();
						}
						pstring r;
						while (!args.eod())
							r += args.next_ws();
						def.m_replace = r;
						m_defines.insert({n, def});
					}
					else
					{
						pstring r;
						while (!args.eod())
							r += args.next_ws();
						m_defines.insert({n, define_t(n, r)});
					}
				}
			}
			else
			{
				if (m_if_flag == 0)
					error("unknown directive");
			}
			return { "", false };
		}
		else
		{
			if (m_if_flag == 0)
				return { replace_macros(lt), true };
			else
				return { "", false };
		}
	}

	void ppreprocessor::push_out(const pstring &s)
	{
		m_outbuf += decltype(m_outbuf)(s.c_str());
		if (m_debug_out)
			std::cerr << s;
	}

	void ppreprocessor::process_stack()
	{
		while (m_stack.size() > 0)
		{
			pstring line;
			pstring linemarker = pfmt("# {1} \"{2}\"\n")(m_stack.back().m_lineno, m_stack.back().m_name);
			push_out(linemarker);
			bool last_skipped=false;
			while (m_stack.back().m_reader.readline(line))
			{
				m_stack.back().m_lineno++;
				auto r(process_line(line));
				if (r.second)
				{
					if (last_skipped)
						push_out(pfmt("# {1} \"{2}\"\n")(m_stack.back().m_lineno, m_stack.back().m_name));
					push_out(r.first + "\n");
					last_skipped = false;
				}
				else
					last_skipped = true;
			}
			m_stack.pop_back();
			if (m_stack.size() > 0)
			{
				linemarker = pfmt("# {1} \"{2}\" 2\n")(m_stack.back().m_lineno, m_stack.back().m_name);
				push_out(linemarker);
			}
		}
	}



} // namespace plib
