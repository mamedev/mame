// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PTOKENIZER_H_
#define PTOKENIZER_H_

///
/// \file ptokenizer.h
///

#include "plists.h"
#include "pstream.h"
#include "pstring.h"

#include "putil.h" // psource_t

#include <unordered_map>
#include <vector>

namespace plib {

	class ptokenizer
	{
	public:
		template <typename T>
		explicit ptokenizer(T &&strm) // NOLINT(misc-forwarding-reference-overload, bugprone-forwarding-reference-overload)
		: m_strm(std::forward<T>(strm))
		, m_cur_line("")
		, m_px(m_cur_line.begin())
		, m_unget(0)
		, m_string('"')
		, m_support_line_markers(true) // FIXME
		{
			// add a first entry to the stack
			m_source_location.emplace_back(plib::source_location("Unknown", 0));
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

			bool is(const token_id_t &tok_id) const noexcept { return m_id.id() == tok_id.id(); }
			bool is_not(const token_id_t &tok_id) const noexcept { return !is(tok_id); }

			bool is_type(const token_type type) const noexcept { return m_type == type; }

			token_type type() const noexcept { return m_type; }

			pstring str() const noexcept { return m_token; }

		private:
			token_type m_type;
			token_id_t m_id;
			pstring m_token;
		};

		pstring currentline_str() const;

		// tokenizer stuff follows ...

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
		void error(const pstring &errs);

		putf8_reader &stream() { return m_strm; }
	protected:
		virtual void verror(const pstring &msg) = 0;

	private:
		void skipeol();

		pstring::value_type getc();
		void ungetc(pstring::value_type c);

		bool eof() const { return m_strm.eof(); }

		putf8_reader m_strm;

		pstring m_cur_line;
		pstring::const_iterator m_px;
		pstring::value_type m_unget;

		// tokenizer stuff follows ...

		pstring m_identifier_chars;
		pstring m_number_chars;
		pstring m_number_chars_start;
		std::unordered_map<pstring, token_id_t> m_tokens;
		pstring m_whitespace;
		pstring::value_type  m_string;

		token_id_t m_tok_comment_start;
		token_id_t m_tok_comment_end;
		token_id_t m_tok_line_comment;

		// source locations, vector used as stack because we need to loop through stack

		bool m_support_line_markers;
		std::vector<plib::source_location> m_source_location;
	};

} // namespace plib

#endif // PTOKENIZER_H_
