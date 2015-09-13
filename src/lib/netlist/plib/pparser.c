// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pparser.c
 *
 */

#include <cstdarg>

#include "pparser.h"

// for now, make buggy GCC/Mingw STFU about I64FMT
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

// ----------------------------------------------------------------------------------------
// A simple tokenizer
// ----------------------------------------------------------------------------------------

pstring ptokenizer::currentline_str()
{
	return m_cur_line;
}


void ptokenizer::skipeol()
{
	pstring::code_t c = getc();
	while (c)
	{
		if (c == 10)
		{
			c = getc();
			if (c != 13)
				ungetc();
			return;
		}
		c = getc();
	}
}


pstring::code_t ptokenizer::getc()
{
	if (m_px >= m_cur_line.len())
	{
		if (m_strm.readline(m_cur_line))
		{
			m_cur_line += "\n";
			m_px = 0;
		}
		else
			return 0;
	}
	return m_cur_line.code_at(m_px++);
}

void ptokenizer::ungetc()
{
	m_px--;
}

void ptokenizer::require_token(const token_id_t &token_num)
{
	require_token(get_token(), token_num);
}

void ptokenizer::require_token(const token_t tok, const token_id_t &token_num)
{
	if (!tok.is(token_num))
	{
		error(pfmt("Expected token <{1}> got <{2}>")(m_tokens[token_num.id()])(tok.str()) );
	}
}

pstring ptokenizer::get_string()
{
	token_t tok = get_token();
	if (!tok.is_type(STRING))
	{
		error(pfmt("Expected a string, got <{1}>")(tok.str()) );
	}
	return tok.str();
}

pstring ptokenizer::get_identifier()
{
	token_t tok = get_token();
	if (!tok.is_type(IDENTIFIER))
	{
		error(pfmt("Expected an identifier, got <{1}>")(tok.str()) );
	}
	return tok.str();
}

pstring ptokenizer::get_identifier_or_number()
{
	token_t tok = get_token();
	if (!(tok.is_type(IDENTIFIER) || tok.is_type(NUMBER)))
	{
		error(pfmt("Expected an identifier, got <{1}>")(tok.str()) );
	}
	return tok.str();
}

double ptokenizer::get_number_double()
{
	token_t tok = get_token();
	if (!tok.is_type(NUMBER))
	{
		error(pfmt("Expected a number, got <{1}>")(tok.str()) );
	}
	bool err = false;
	double ret = tok.str().as_double(&err);
	if (err)
		error(pfmt("Expected a number, got <{1}>")(tok.str()) );
	return ret;
}

long ptokenizer::get_number_long()
{
	token_t tok = get_token();
	if (!tok.is_type(NUMBER))
	{
		error(pfmt("Expected a long int, got <{1}>")(tok.str()) );
	}
	bool err = false;
	long ret = tok.str().as_long(&err);
	if (err)
		error(pfmt("Expected a long int, got <{1}>")(tok.str()) );
	return ret;
}

ptokenizer::token_t ptokenizer::get_token()
{
	while (true)
	{
		token_t ret = get_token_internal();
		if (ret.is_type(ENDOFFILE))
			return ret;

		if (ret.is(m_tok_comment_start))
		{
			do {
				ret = get_token_internal();
			} while (ret.is_not(m_tok_comment_end));
		}
		else if (ret.is(m_tok_line_comment))
		{
			skipeol();
		}
		else if (ret.str() == "#")
		{
			skipeol();
		}
		else
		{
			return ret;
		}
	}
}

ptokenizer::token_t ptokenizer::get_token_internal()
{
	/* skip ws */
	pstring::code_t c = getc();
	while (m_whitespace.find(c)>=0)
	{
		c = getc();
		if (eof())
		{
			return token_t(ENDOFFILE);
		}
	}
	if (m_number_chars_start.find(c)>=0)
	{
		/* read number while we receive number or identifier chars
		 * treat it as an identifier when there are identifier chars in it
		 *
		 */
		token_type ret = NUMBER;
		pstring tokstr = "";
		while (true) {
			if (m_identifier_chars.find(c)>=0 && m_number_chars.find(c)<0)
				ret = IDENTIFIER;
			else if (m_number_chars.find(c)<0)
				break;
			tokstr += c;
			c = getc();
		}
		ungetc();
		return token_t(ret, tokstr);
	}
	else if (m_identifier_chars.find(c)>=0)
	{
		/* read identifier till non identifier char */
		pstring tokstr = "";
		while (m_identifier_chars.find(c)>=0) {
			tokstr += c;
			c = getc();
		}
		ungetc();
		token_id_t id = token_id_t(m_tokens.indexof(tokstr));
		if (id.id() >= 0)
			return token_t(id, tokstr);
		else
		{
			return token_t(IDENTIFIER, tokstr);
		}
	}
	else if (c == m_string)
	{
		pstring tokstr = "";
		c = getc();
		while (c != m_string)
		{
			tokstr += c;
			c = getc();
		}
		return token_t(STRING, tokstr);
	}
	else
	{
		/* read identifier till first identifier char or ws */
		pstring tokstr = "";
		while ((m_identifier_chars.find(c)) < 0 && (m_whitespace.find(c) < 0)) {
			tokstr += c;
			/* expensive, check for single char tokens */
			if (tokstr.len() == 1)
			{
				token_id_t id = token_id_t(m_tokens.indexof(tokstr));
				if (id.id() >= 0)
					return token_t(id, tokstr);
			}
			c = getc();
		}
		ungetc();
		token_id_t id = token_id_t(m_tokens.indexof(tokstr));
		if (id.id() >= 0)
			return token_t(id, tokstr);
		else
		{
			return token_t(UNKNOWN, tokstr);
		}
	}

}

ATTR_COLD void ptokenizer::error(const pstring &errs)
{
	verror("Error: " + errs, currentline_no(), currentline_str());
	//throw error;
}

// ----------------------------------------------------------------------------------------
// A simple preprocessor
// ----------------------------------------------------------------------------------------

ppreprocessor::ppreprocessor()
: m_ifflag(0), m_level(0), m_lineno(0)
{
	m_expr_sep.add("!");
	m_expr_sep.add("(");
	m_expr_sep.add(")");
	m_expr_sep.add("+");
	m_expr_sep.add("-");
	m_expr_sep.add("*");
	m_expr_sep.add("/");
	m_expr_sep.add("==");
	m_expr_sep.add(" ");
	m_expr_sep.add("\t");

	m_defines.add("__PLIB_PREPROCESSOR__", define_t("__PLIB_PREPROCESSOR__", "1"));
}

void ppreprocessor::error(const pstring &err)
{
	throw pexception("PREPRO ERROR: " + err);
}



double ppreprocessor::expr(const pstring_list_t &sexpr, std::size_t &start, int prio)
{
	double val;
	pstring tok=sexpr[start];
	if (tok == "(")
	{
		start++;
		val = expr(sexpr, start, /*prio*/ 0);
		if (sexpr[start] != ")")
			error("parsing error!");
		start++;
	}
	else if (tok == "!")
	{
		start++;
		val = expr(sexpr, start, 90);
		if (val != 0)
			val = 0;
		else
			val = 1;
	}
	else
	{
		tok=sexpr[start];
		val = tok.as_double();
		start++;
	}
	while (start < sexpr.size())
	{
		tok=sexpr[start];
		if (tok == ")")
		{
			// FIXME: catch error
			return val;
		}
		else if (tok == "+")
		{
			if (prio > 10)
				return val;
			start++;
			val = val + expr(sexpr, start, 10);
		}
		else if (tok == "-")
		{
			if (prio > 10)
				return val;
			start++;
			val = val - expr(sexpr, start, 10);
		}
		else if (tok == "*")
		{
			start++;
			val = val * expr(sexpr, start, 20);
		}
		else if (tok == "/")
		{
			start++;
			val = val / expr(sexpr, start, 20);
		}
		else if (tok == "==")
		{
			if (prio > 5)
				return val;
			start++;
			val = (val == expr(sexpr, start, 5)) ? 1.0 : 0.0;
		}
	}
	return val;
}

ppreprocessor::define_t *ppreprocessor::get_define(const pstring &name)
{
	int idx = m_defines.index_of(name);
	if (idx >= 0)
		return &m_defines.value_at(idx);
	else
		return NULL;
}

pstring ppreprocessor::replace_macros(const pstring &line)
{
	pstring_list_t elems = pstring_list_t::splitexpr(line, m_expr_sep);
	pstringbuffer ret = "";
	for (std::size_t i=0; i<elems.size(); i++)
	{
		define_t *def = get_define(elems[i]);
		if (def != NULL)
			ret.cat(def->m_replace);
		else
			ret.cat(elems[i]);
	}
	return ret;
}

static pstring catremainder(const pstring_list_t &elems, std::size_t start, pstring sep)
{
	pstringbuffer ret = "";
	for (std::size_t i=start; i<elems.size(); i++)
	{
		ret.cat(elems[i]);
		ret.cat(sep);
	}
	return ret;
}

pstring  ppreprocessor::process_line(const pstring &line)
{
	pstring lt = line.replace("\t"," ").trim();
	pstringbuffer ret;
	m_lineno++;
	// FIXME ... revise and extend macro handling
	if (lt.startsWith("#"))
	{
		pstring_list_t lti(lt, " ", true);
		if (lti[0].equals("#if"))
		{
			m_level++;
			std::size_t start = 0;
			lt = replace_macros(lt);
			pstring_list_t t = pstring_list_t::splitexpr(lt.substr(3).replace(" ",""), m_expr_sep);
			int val = expr(t, start, 0);
			if (val == 0)
				m_ifflag |= (1 << m_level);
		}
		else if (lti[0].equals("#ifdef"))
		{
			m_level++;
			if (get_define(lti[1]) == NULL)
				m_ifflag |= (1 << m_level);
		}
		else if (lti[0].equals("#ifndef"))
		{
			m_level++;
			if (get_define(lti[1]) != NULL)
				m_ifflag |= (1 << m_level);
		}
		else if (lti[0].equals("#else"))
		{
			m_ifflag ^= (1 << m_level);
		}
		else if (lti[0].equals("#endif"))
		{
			m_ifflag &= ~(1 << m_level);
			m_level--;
		}
		else if (lti[0].equals("#include"))
		{
			// ignore
		}
		else if (lti[0].equals("#pragma"))
		{
			if (m_ifflag == 0 && lti.size() > 3 && lti[1].equals("NETLIST"))
			{
				if (lti[2].equals("warning"))
					error("NETLIST: " + catremainder(lti, 3, " "));
			}
		}
		else if (lti[0].equals("#define"))
		{
			if (m_ifflag == 0)
			{
				if (lti.size() != 3)
					error("PREPRO: only simple defines allowed: " + line);
				m_defines.add(lti[1], define_t(lti[1], lti[2]));
			}
		}
		else
			error(pfmt("unknown directive on line {1}: {2}")(m_lineno)(line));
	}
	else
	{
		lt = replace_macros(lt);
		if (m_ifflag == 0)
		{
			ret.cat(lt);
			ret.cat("\n");
		}
	}
	return ret;
}


postream & ppreprocessor::process_i(pistream &istrm, postream &ostrm)
{
	pstring line;
	while (istrm.readline(line))
	{
		line = process_line(line);
		ostrm.writeline(line);
	}
	return ostrm;
}


#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic pop
#endif
