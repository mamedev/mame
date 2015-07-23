// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pparser.c
 *
 */

#include <cstdio>
#include <cstdarg>

#include "pparser.h"

// for now, make buggy GCC/Mingw STFU about I64FMT
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

//#undef NL_VERBOSE_OUT
//#define NL_VERBOSE_OUT(x) printf x

// ----------------------------------------------------------------------------------------
// A simple tokenizer
// ----------------------------------------------------------------------------------------

pstring ptokenizer::currentline_str()
{
	char buf[300];
	int bufp = 0;
	const char *p = m_line_ptr;
	while (*p && *p != 10)
		buf[bufp++] = *p++;
	buf[bufp] = 0;
	return pstring(buf);
}


void ptokenizer::skipeol()
{
	char c = getc();
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


unsigned char ptokenizer::getc()
{
	if (*m_px == 10)
	{
		m_line++;
		m_line_ptr = m_px + 1;
	}
	if (*m_px)
		return *(m_px++);
	else
		return *m_px;
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
		error("Error: expected token <%s> got <%s>\n", m_tokens[token_num.id()].cstr(), tok.str().cstr());
	}
}

pstring ptokenizer::get_string()
{
	token_t tok = get_token();
	if (!tok.is_type(STRING))
	{
		error("Error: expected a string, got <%s>\n", tok.str().cstr());
	}
	return tok.str();
}

pstring ptokenizer::get_identifier()
{
	token_t tok = get_token();
	if (!tok.is_type(IDENTIFIER))
	{
		error("Error: expected an identifier, got <%s>\n", tok.str().cstr());
	}
	return tok.str();
}

pstring ptokenizer::get_identifier_or_number()
{
	token_t tok = get_token();
	if (!(tok.is_type(IDENTIFIER) || tok.is_type(NUMBER)))
	{
		error("Error: expected an identifier, got <%s>\n", tok.str().cstr());
	}
	return tok.str();
}

double ptokenizer::get_number_double()
{
	token_t tok = get_token();
	if (!tok.is_type(NUMBER))
	{
		error("Error: expected a number, got <%s>\n", tok.str().cstr());
	}
	bool err = false;
	double ret = tok.str().as_double(&err);
	if (err)
		error("Error: expected a number, got <%s>\n", tok.str().cstr());
	return ret;
}

long ptokenizer::get_number_long()
{
	token_t tok = get_token();
	if (!tok.is_type(NUMBER))
	{
		error("Error: expected a long int, got <%s>\n", tok.str().cstr());
	}
	bool err = false;
	long ret = tok.str().as_long(&err);
	if (err)
		error("Error: expected a long int, got <%s>\n", tok.str().cstr());
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
			return ret;
	}
}

ptokenizer::token_t ptokenizer::get_token_internal()
{
	/* skip ws */
	char c = getc();
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

ATTR_COLD void ptokenizer::error(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	pstring errmsg1 = pstring(format).vprintf(ap);
	va_end(ap);

	verror(errmsg1, currentline_no(), currentline_str());

	//throw error;
}

// ----------------------------------------------------------------------------------------
// A simple preprocessor
// ----------------------------------------------------------------------------------------

ppreprocessor::ppreprocessor()
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

	m_defines.add(define_t("__PLIB_PREPROCESSOR__", "1"));
}

void ppreprocessor::error(const pstring &err)
{
	fprintf(stderr, "PREPRO ERROR: %s\n", err.cstr());
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
	for (std::size_t i = 0; i<m_defines.size(); i++)
	{
		if (m_defines[i].m_name == name)
			return &m_defines[i];
	}
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
	return pstring(ret.cstr());
}

static pstring catremainder(const pstring_list_t &elems, std::size_t start, pstring sep)
{
	pstringbuffer ret = "";
	for (std::size_t i=start; i<elems.size(); i++)
	{
		ret.cat(elems[i]);
		ret.cat(sep);
	}
	return pstring(ret.cstr());
}

pstring ppreprocessor::process(const pstring &contents)
{
	pstringbuffer ret = "";
	pstring_list_t lines(contents,"\n", false);
	UINT32 ifflag = 0; // 31 if levels
	int level = 0;

	std::size_t i=0;
	while (i<lines.size())
	{
		pstring line = lines[i];
		pstring lt = line.replace("\t"," ").trim();
		// FIXME ... revise and extend macro handling
		if (lt.startsWith("#"))
		{
			pstring_list_t lti(lt, " ", true);
			if (lti[0].equals("#if"))
			{
				level++;
				std::size_t start = 0;
				lt = replace_macros(lt);
				pstring_list_t t = pstring_list_t::splitexpr(lt.substr(3).replace(" ",""), m_expr_sep);
				int val = expr(t, start, 0);
				if (val == 0)
					ifflag |= (1 << level);
			}
			else if (lti[0].equals("#ifdef"))
			{
				level++;
				if (get_define(lti[1]) == NULL)
					ifflag |= (1 << level);
			}
			else if (lti[0].equals("#ifndef"))
			{
				level++;
				if (get_define(lti[1]) != NULL)
					ifflag |= (1 << level);
			}
			else if (lti[0].equals("#else"))
			{
				ifflag ^= (1 << level);
			}
			else if (lti[0].equals("#endif"))
			{
				ifflag &= ~(1 << level);
				level--;
			}
			else if (lti[0].equals("#include"))
			{
				// ignore
			}
			else if (lti[0].equals("#pragma"))
			{
				if (ifflag == 0 && lti.size() > 3 && lti[1].equals("NETLIST"))
				{
					if (lti[2].equals("warning"))
						error("NETLIST: " + catremainder(lti, 3, " "));
				}
			}
			else if (lti[0].equals("#define"))
			{
				if (ifflag == 0)
				{
					if (lti.size() != 3)
						error(pstring::sprintf("PREPRO: only simple defines allowed: %s", line.cstr()));
					m_defines.add(define_t(lti[1], lti[2]));
				}
			}
			else
				error(pstring::sprintf("unknown directive on line %" SIZETFMT ": %s\n", SIZET_PRINTF(i), line.cstr()));
		}
		else
		{
			//if (ifflag == 0 && level > 0)
			//  fprintf(stderr, "conditional: %s\n", line.cstr());
			lt = replace_macros(lt);
			if (ifflag == 0)
			{
				ret.cat(lt);
				ret.cat("\n");
			}
		}
		i++;
	}
	return pstring(ret.cstr());
}


#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic pop
#endif
