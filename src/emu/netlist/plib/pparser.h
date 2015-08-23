// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pparser.h
 *
 */

#ifndef PPARSER_H_
#define PPARSER_H_

#include "pconfig.h"
#include "pstring.h"
#include "plists.h"
#include "pstream.h"

class ptokenizer
{
	P_PREVENT_COPYING(ptokenizer)
public:
	virtual ~ptokenizer() {}

	ptokenizer(pistream &strm)
	: m_strm(strm), m_lineno(1), m_px(0), m_string('"')
	{}

	enum token_type
	{
		IDENTIFIER,
		NUMBER,
		TOKEN,
		STRING,
		COMMENT,
		UNKNOWN,
		ENDOFFILE
	};

	struct token_id_t
	{
	public:
		token_id_t() : m_id(-2) {}
		token_id_t(const int id) : m_id(id) {}
		int id() const { return m_id; }
	private:
		int m_id;
	};

	struct token_t
	{
		token_t(token_type type)
		{
			m_type = type;
			m_id = token_id_t(-1);
			m_token ="";
		}
		token_t(token_type type, const pstring &str)
		{
			m_type = type;
			m_id = token_id_t(-1);
			m_token = str;
		}
		token_t(const token_id_t id, const pstring &str)
		{
			m_type = TOKEN;
			m_id = id;
			m_token = str;
		}

		bool is(const token_id_t &tok_id) const { return m_id.id() == tok_id.id(); }
		bool is_not(const token_id_t &tok_id) const { return !is(tok_id); }

		bool is_type(const token_type type) const { return m_type == type; }

		pstring str() const { return m_token; }

	private:
		token_type m_type;
		token_id_t m_id;
		pstring m_token;
	};


	int currentline_no() { return m_lineno; }
	pstring currentline_str();

	/* tokenizer stuff follows ... */

	token_t get_token();
	pstring get_string();
	pstring get_identifier();
	pstring get_identifier_or_number();
	double get_number_double();
	long get_number_long();

	void require_token(const token_id_t &token_num);
	void require_token(const token_t tok, const token_id_t &token_num);

	token_id_t register_token(pstring token)
	{
		m_tokens.add(token);
		return token_id_t(m_tokens.size() - 1);
	}

	void set_identifier_chars(pstring s) { m_identifier_chars = s; }
	void set_number_chars(pstring st, pstring rem) { m_number_chars_start = st; m_number_chars = rem; }
	void set_string_char(char c) { m_string = c; }
	void set_whitespace(pstring s) { m_whitespace = s; }
	void set_comment(pstring start, pstring end, pstring line)
	{
		m_tok_comment_start = register_token(start);
		m_tok_comment_end = register_token(end);
		m_tok_line_comment = register_token(line);
	}

	token_t get_token_internal();
	void error(const pstring &errs);

protected:
	virtual void verror(const pstring &msg, int line_num, const pstring &line) = 0;

private:
	void skipeol();

	pstring::code_t getc();
	void ungetc();

	bool eof() { return m_strm.eof(); }

	pistream &m_strm;

	int m_lineno;
	pstring m_cur_line;
	unsigned m_px;

	/* tokenizer stuff follows ... */

	pstring m_identifier_chars;
	pstring m_number_chars;
	pstring m_number_chars_start;
	plist_t<pstring> m_tokens;
	pstring m_whitespace;
	pstring::code_t  m_string;

	token_id_t m_tok_comment_start;
	token_id_t m_tok_comment_end;
	token_id_t m_tok_line_comment;
};


class ppreprocessor
{
	P_PREVENT_COPYING(ppreprocessor)
public:

	struct define_t
	{
		define_t() { };
		define_t(const pstring &name, const pstring &replace)
		: m_name(name), m_replace(replace)
		{}
		pstring m_name;
		pstring m_replace;
	};

	ppreprocessor();
	virtual ~ppreprocessor() {}

	template<class ISTR, class OSTR>
	OSTR &process(ISTR &istrm, OSTR &ostrm)
	{
		return dynamic_cast<OSTR &>(process_i(istrm, ostrm));
	}

protected:

	postream &process_i(pistream &istrm, postream &ostrm);

	double expr(const pstring_list_t &sexpr, std::size_t &start, int prio);

	define_t *get_define(const pstring &name);

	pstring replace_macros(const pstring &line);

	virtual void error(const pstring &err);

private:

	pstring process_line(const pstring &line);

	phashmap_t<pstring, define_t> m_defines;
	pstring_list_t m_expr_sep;

	//pstringbuffer m_ret;
	UINT32 m_ifflag; // 31 if levels
	int m_level;
	int m_lineno;
};

#endif /* PPARSER_H_ */
