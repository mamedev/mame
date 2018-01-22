// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pparser.h
 *
 */

#ifndef PPARSER_H_
#define PPARSER_H_

#include "pstring.h"
#include "plists.h"
#include "pstream.h"

#include <unordered_map>
#include <cstdint>

namespace plib {
class ptokenizer : nocopyassignmove
{
public:
	explicit ptokenizer(plib::putf8_reader &strm);

	virtual ~ptokenizer();

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

		static constexpr std::size_t npos = static_cast<std::size_t>(-1);

		token_id_t() : m_id(npos) {}
		explicit token_id_t(const std::size_t id) : m_id(id) {}
		std::size_t id() const { return m_id; }
	private:
		std::size_t m_id;
	};

	struct token_t
	{
		explicit token_t(token_type type)
		: m_type(type), m_id(), m_token("")
		{
		}
		token_t(token_type type, const pstring &str)
		: m_type(type), m_id(), m_token(str)
		{
		}
		token_t(const token_id_t &id, const pstring &str)
		: m_type(TOKEN), m_id(id), m_token(str)
		{
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
	void require_token(const token_t &tok, const token_id_t &token_num);

	token_id_t register_token(pstring token)
	{
		token_id_t ret(m_tokens.size());
		m_tokens.emplace(token, ret);
		return ret;
	}

	void set_identifier_chars(pstring s) { m_identifier_chars = s; }
	void set_number_chars(pstring st, pstring rem) { m_number_chars_start = st; m_number_chars = rem; }
	void set_string_char(pstring::code_t c) { m_string = c; }
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
	void ungetc(pstring::code_t c);

	bool eof() { return m_strm.eof(); }

	putf8_reader &m_strm;

	int m_lineno;
	pstring m_cur_line;
	pstring::const_iterator m_px;
	pstring::code_t m_unget;

	/* tokenizer stuff follows ... */

	pstring m_identifier_chars;
	pstring m_number_chars;
	pstring m_number_chars_start;
	std::unordered_map<pstring, token_id_t> m_tokens;
	pstring m_whitespace;
	pstring::code_t  m_string;

	token_id_t m_tok_comment_start;
	token_id_t m_tok_comment_end;
	token_id_t m_tok_line_comment;
};


class ppreprocessor : plib::nocopyassignmove
{
public:

	struct define_t
	{
		define_t(const pstring &name, const pstring &replace)
		: m_name(name), m_replace(replace)
		{}
		pstring m_name;
		pstring m_replace;
	};

	explicit ppreprocessor(std::vector<define_t> *defines = nullptr);
	virtual ~ppreprocessor() {}

	void process(putf8_reader &istrm, putf8_writer &ostrm);

protected:
	double expr(const std::vector<pstring> &sexpr, std::size_t &start, int prio);
	define_t *get_define(const pstring &name);
	pstring replace_macros(const pstring &line);
	virtual void error(const pstring &err);

private:

	pstring process_line(const pstring &line);

	std::unordered_map<pstring, define_t> m_defines;
	std::vector<pstring> m_expr_sep;

	std::uint_least64_t m_ifflag; // 31 if levels
	int m_level;
	int m_lineno;
};

}

#endif /* PPARSER_H_ */
