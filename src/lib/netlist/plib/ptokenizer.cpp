// license:GPL-2.0+
// copyright-holders:Couriersud

#include "ptokenizer.h"
#include "palloc.h"
#include "pstonum.h"
#include "putil.h"

namespace plib {

	// ----------------------------------------------------------------------------------------
	// A simple tokenizer
	// ----------------------------------------------------------------------------------------

	pstring ptokenizer::currentline_str()
	{
		return m_cur_line;
	}

	void ptokenizer::skipeol()
	{
		pstring::value_type c = getc();
		while (c)
		{
			if (c == 10)
			{
				c = getc();
				if (c != 13)
					ungetc(c);
				return;
			}
			c = getc();
		}
	}

	pstring::value_type ptokenizer::getc()
	{
		if (m_unget != 0)
		{
			pstring::value_type c = m_unget;
			m_unget = 0;
			return c;
		}
		if (m_px == m_cur_line.end())
		{
			++m_source_location.back();
			if (m_strm.readline(m_cur_line))
				m_px = m_cur_line.begin();
			else
				return 0;
			return '\n';
		}
		pstring::value_type c = *(m_px++);
		return c;
	}

	void ptokenizer::ungetc(pstring::value_type c)
	{
		m_unget = c;
	}

	void ptokenizer::require_token(const token_id_t &token_num)
	{
		require_token(get_token(), token_num);
	}

	void ptokenizer::require_token(const token_t &tok, const token_id_t &token_num)
	{
		if (!tok.is(token_num))
		{
			pstring val("");
			for (auto &i : m_tokens)
				if (i.second.id() == token_num.id())
					val = i.first;
			error(pfmt("Expected token <{1}> got <{2}>")(val)(tok.str()) );
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
			error(pfmt("Expected an identifier or number, got <{1}>")(tok.str()) );
		}
		return tok.str();
	}

	// FIXME: combine into template
	double ptokenizer::get_number_double()
	{
		token_t tok = get_token();
		if (!tok.is_type(NUMBER))
		{
			error(pfmt("Expected a number, got <{1}>")(tok.str()) );
		}
		bool err(false);
		auto ret = plib::pstonum_ne<double>(tok.str(), err);
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
		bool err(false);
		auto ret = plib::pstonum_ne<long>(tok.str(), err);
		if (err)
			error(pfmt("Expected a long int, got <{1}>")(tok.str()) );
		return ret;
	}

	ptokenizer::token_t ptokenizer::get_token()
	{
		token_t ret = get_token_internal();
		while (true)
		{
			if (ret.is_type(ENDOFFILE))
				return ret;
			else if (m_support_line_markers && ret.is_type(LINEMARKER))
			{
				bool benter(false);
				bool bexit(false);
				pstring file;
				unsigned lineno;

				ret = get_token_internal();
				if (!ret.is_type(NUMBER))
					error(pfmt("Expected line number after line marker but got <{1}>")(ret.str()) );
				lineno = pstonum<unsigned>(ret.str());
				ret = get_token_internal();
				if (!ret.is_type(STRING))
					error(pfmt("Expected file name after line marker but got <{1}>")(ret.str()) );
				file = ret.str();
				ret = get_token_internal();
				while (ret.is_type(NUMBER))
				{
					if (ret.str() == "1")
						benter = true;
					if (ret.str() == "2")
						bexit = false;
					// FIXME: process flags; actually only 1 (file enter) and 2 (after file exit)
					ret = get_token_internal();
				}
				if (bexit) // pop the last location
					m_source_location.pop_back();
				if (!benter) // new location!
					m_source_location.pop_back();
				m_source_location.emplace_back(plib::source_location(file, lineno));
			}
			else if (ret.is(m_tok_comment_start))
			{
				do {
					ret = get_token_internal();
				} while (ret.is_not(m_tok_comment_end));
				ret = get_token_internal();
			}
			else if (ret.is(m_tok_line_comment))
			{
				skipeol();
				ret = get_token_internal();
			}
			else
			{
				return ret;
			}
		}
	}

	ptokenizer::token_t ptokenizer::get_token_internal()
	{
		// skip ws
		pstring::value_type c = getc();
		while (m_whitespace.find(c) != pstring::npos)
		{
			c = getc();
			if (eof())
			{
				return token_t(ENDOFFILE);
			}
		}
		if (m_support_line_markers && c == '#')
			return token_t(LINEMARKER, "#");
		else if (m_number_chars_start.find(c) != pstring::npos)
		{
			// read number while we receive number or identifier chars
			// treat it as an identifier when there are identifier chars in it
			token_type ret = NUMBER;
			pstring tokstr = "";
			while (true) {
				if (m_identifier_chars.find(c) != pstring::npos && m_number_chars.find(c) == pstring::npos)
					ret = IDENTIFIER;
				else if (m_number_chars.find(c) == pstring::npos)
					break;
				tokstr += c;
				c = getc();
			}
			ungetc(c);
			return token_t(ret, tokstr);
		}
		else if (m_identifier_chars.find(c) != pstring::npos)
		{
			// read identifier till non identifier char
			pstring tokstr = "";
			while (m_identifier_chars.find(c) != pstring::npos)
			{
				tokstr += c;
				c = getc();
			}
			ungetc(c);
			auto id = m_tokens.find(tokstr);
			if (id != m_tokens.end())
				return token_t(id->second, tokstr);
			else
				return token_t(IDENTIFIER, tokstr);
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
			// read identifier till first identifier char or ws
			pstring tokstr = "";
			while ((m_identifier_chars.find(c) == pstring::npos) && (m_whitespace.find(c) == pstring::npos))
			{
				tokstr += c;
				// expensive, check for single char tokens
				if (tokstr.length() == 1)
				{
					auto id = m_tokens.find(tokstr);
					if (id != m_tokens.end())
						return token_t(id->second, tokstr);
				}
				c = getc();
			}
			ungetc(c);
			auto id = m_tokens.find(tokstr);
			if (id != m_tokens.end())
				return token_t(id->second, tokstr);
			else
				return token_t(UNKNOWN, tokstr);
		}
	}

	void ptokenizer::error(const pstring &errs)
	{
		pstring s("");
		pstring trail      ("                 from ");
		pstring trail_first("In file included from ");
		pstring e = plib::pfmt("{1}:{2}:0: error: {3}\n")
				(m_source_location.back().file_name(), m_source_location.back().line(), errs);
		m_source_location.pop_back();
		while (m_source_location.size() > 0)
		{
			if (m_source_location.size() == 1)
				trail = trail_first;
			s = trail + plib::pfmt("{1}:{2}:0\n")(m_source_location.back().file_name(), m_source_location.back().line()) + s;
			m_source_location.pop_back();
		}
	verror("\n" + s + e + " " + currentline_str() + "\n");
}

} // namespace plib
