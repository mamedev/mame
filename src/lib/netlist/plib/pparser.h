// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pparser.h
 *
 */

#ifndef PPARSER_H_
#define PPARSER_H_

#include "plists.h"
#include "pstream.h"
#include "pstring.h"

#include "putil.h" // psource_t

//#include <cstdint>
#include <unordered_map>
#include <stack>

namespace plib {
class ptokenizer
{
public:
	template <typename T>
	ptokenizer(T &&strm) // NOLINT(misc-forwarding-reference-overload, bugprone-forwarding-reference-overload)
	: m_strm(std::forward<T>(strm))
	, m_lineno(0)
	, m_cur_line("")
	, m_px(m_cur_line.begin())
	, m_unget(0)
	, m_string('"')
	, m_support_line_markers(true) // FIXME
	{
	}

	COPYASSIGNMOVE(ptokenizer, delete)

	virtual ~ptokenizer() = default;

	enum token_type
	{
		IDENTIFIER,
		NUMBER,
		TOKEN,
		STRING,
		COMMENT,
		LINEMARKER,
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

	token_id_t register_token(const pstring &token)
	{
		token_id_t ret(m_tokens.size());
		m_tokens.emplace(token, ret);
		return ret;
	}

	ptokenizer & identifier_chars(const pstring &s) { m_identifier_chars = s; return *this; }
	ptokenizer & number_chars(const pstring &st, const pstring & rem) { m_number_chars_start = st; m_number_chars = rem; return *this; }
	ptokenizer & string_char(pstring::value_type c) { m_string = c; return *this; }
	ptokenizer & whitespace(const pstring & s) { m_whitespace = s; return *this; }
	ptokenizer & comment(const pstring &start, const pstring &end, const pstring &line)
	{
		m_tok_comment_start = register_token(start);
		m_tok_comment_end = register_token(end);
		m_tok_line_comment = register_token(line);
		return *this;
	}

	token_t get_token_internal();
	void error(const pstring &errs) { verror(errs, currentline_no(), currentline_str()); }

	putf8_reader &stream() { return m_strm; }
protected:
	virtual void verror(const pstring &msg, int line_num, const pstring &line) = 0;

private:
	void skipeol();

	pstring::value_type getc();
	void ungetc(pstring::value_type c);

	bool eof() { return m_strm.eof(); }

	putf8_reader m_strm;

	int m_lineno;
	pstring m_cur_line;
	pstring::const_iterator m_px;
	pstring::value_type m_unget;

	/* tokenizer stuff follows ... */

	pstring m_identifier_chars;
	pstring m_number_chars;
	pstring m_number_chars_start;
	std::unordered_map<pstring, token_id_t> m_tokens;
	pstring m_whitespace;
	pstring::value_type  m_string;

	token_id_t m_tok_comment_start;
	token_id_t m_tok_comment_end;
	token_id_t m_tok_line_comment;
	bool m_support_line_markers;
};


class ppreprocessor : public std::istream
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

	using defines_map_type = std::unordered_map<pstring, define_t>;

	explicit ppreprocessor(psource_collection_t<> &sources, defines_map_type *defines = nullptr);

	COPYASSIGN(ppreprocessor, delete)
	ppreprocessor &operator=(ppreprocessor &&src) = delete;

	ppreprocessor(ppreprocessor &&s) noexcept
	: std::istream(new readbuffer(this))
	, m_defines(std::move(s.m_defines))
	, m_sources(s.m_sources)
	, m_expr_sep(std::move(s.m_expr_sep))
	, m_if_flag(s.m_if_flag)
	, m_if_level(s.m_if_level)
	, m_outbuf(std::move(s.m_outbuf))
	, m_pos(s.m_pos)
	, m_state(s.m_state)
	, m_comment(s.m_comment)
	{
	}

	~ppreprocessor() override
	{
		delete rdbuf();
	}

	template <typename T>
	ppreprocessor & process(T &&istrm)
	{
		m_stack.emplace(input_context(std::forward<T>(istrm),"","<stream>"));
		process_stack();
		return *this;
	}

protected:

	class readbuffer : public std::streambuf
	{
	public:
		readbuffer(ppreprocessor *strm) : m_strm(strm), m_buf() { setg(nullptr, nullptr, nullptr); }
		readbuffer(readbuffer &&rhs) noexcept : m_strm(rhs.m_strm), m_buf()  {}
		COPYASSIGN(readbuffer, delete)
		readbuffer &operator=(readbuffer &&src) = delete;
		~readbuffer() override = default;

		int_type underflow() override
		{
			if (this->gptr() == this->egptr())
			{
				/* clang reports sign error - weird */
				std::size_t bytes = pstring_mem_t_size(m_strm->m_outbuf) - static_cast<std::size_t>(m_strm->m_pos);

				if (bytes > m_buf.size())
					bytes = m_buf.size();
				std::copy(m_strm->m_outbuf.c_str() + m_strm->m_pos, m_strm->m_outbuf.c_str() + m_strm->m_pos + bytes, m_buf.data());
				//printf("%ld\n", (long int)bytes);
				this->setg(m_buf.data(), m_buf.data(), m_buf.data() + bytes);

				m_strm->m_pos += static_cast</*pos_type*/long>(bytes);
			}
			return this->gptr() == this->egptr()
				 ? std::char_traits<char>::eof()
				 : std::char_traits<char>::to_int_type(*this->gptr());
		}

	private:
		ppreprocessor *m_strm;
		std::array<char_type, 1024> m_buf;
	};
	//friend class st;

	int expr(const std::vector<pstring> &sexpr, std::size_t &start, int prio);
	define_t *get_define(const pstring &name);
	pstring replace_macros(const pstring &line);
	virtual void error(const pstring &err);

private:

	enum state_e
	{
		PROCESS,
		LINE_CONTINUATION
	};

	void process_stack();

	pstring process_line(pstring line);
	pstring process_comments(pstring line);

	defines_map_type m_defines;
	psource_collection_t<> &m_sources;
	std::vector<pstring> m_expr_sep;

	std::uint_least64_t m_if_flag; // 31 if levels
	int m_if_level;

	struct input_context
	{
		template <typename T>
		input_context(T &&istrm, const pstring &local_path, const pstring &name)
		: m_reader(std::forward<T>(istrm))
		, m_lineno(0)
		, m_local_path(local_path)
		, m_name(name)
		{}

		putf8_reader m_reader;
		int m_lineno;
		pstring m_local_path;
		pstring m_name;
	};

	std::stack<input_context> m_stack;
	pstring_t<pu8_traits> m_outbuf;
	std::istream::pos_type m_pos;
	state_e m_state;
	pstring m_line;
	bool m_comment;

};

} // namespace plib

#endif /* PPARSER_H_ */
