// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PTOKENIZER_H_
#define PTOKENIZER_H_

///
/// \file ptokenizer.h
///

#include "pstream.h"
#include "pstring.h"

#include "penum.h"
#include "psource.h"

#include <unordered_map>
#include <vector>

namespace plib {

	namespace detail {

		PENUM(token_type,
			IDENTIFIER,
			NUMBER,
			TOKEN,
			STRING,
			COMMENT,
			LINEMARKER,
			SOURCELINE,
			UNKNOWN,
			ENDOFFILE
		)

		struct token_id_t
		{
		public:

			static constexpr std::size_t npos = static_cast<std::size_t>(-1);

			token_id_t()
			: m_id(npos)
			{}

			explicit token_id_t(std::size_t id, const pstring &name)
			: m_id(id)
			, m_name(name)
			{}

			token_id_t(const token_id_t &) = default;
			token_id_t &operator=(const token_id_t &) = default;
			token_id_t(token_id_t &&) noexcept = default;
			token_id_t &operator=(token_id_t &&) noexcept = default;

			~token_id_t() = default;

			constexpr std::size_t id() const { return m_id; }
			constexpr const pstring & name() const { return m_name; }
		private:
			std::size_t m_id;
			pstring     m_name;
		};

		struct token_t
		{
			explicit token_t(token_type type)
			: m_type(type), m_id(token_id_t::npos), m_token("")
			{
			}
			token_t(token_type type, const pstring &str)
			: m_type(type), m_id(token_id_t::npos), m_token(str)
			{
			}
			token_t(const token_id_t &id)
			: m_type(token_type::TOKEN), m_id(id.id()), m_token(id.name())
			{
			}
			token_t(const token_id_t &id, const pstring &str)
			: m_type(token_type::TOKEN), m_id(id.id()), m_token(str)
			{
			}

			token_t(const token_t &) = default;
			token_t &operator=(const token_t &) = default;
			token_t(token_t &&) noexcept = default;
			token_t &operator=(token_t &&) noexcept = default;

			~token_t() = default;

			constexpr bool is(const token_id_t &tok_id) const noexcept { return m_id == tok_id.id(); }
			constexpr bool is_not(const token_id_t &tok_id) const noexcept { return !is(tok_id); }
			constexpr bool is_type(const token_type type) const noexcept { return m_type == type; }
			constexpr token_type type() const noexcept { return m_type; }
			constexpr const pstring &str() const noexcept { return m_token; }

		private:
			token_type m_type;
			std::size_t m_id;
			pstring m_token;
		};

		class token_store_t : public std::vector<token_t>
		{
			using std::vector<token_t>::vector;
		};

	} // namespace detail

	class tokenizer_t
	{
	public:
		using token_type = detail::token_type;
		using token_id_t = detail::token_id_t;
		using token_t = detail::token_t;
		using token_store_t = detail::token_store_t;

		explicit tokenizer_t() // NOLINT(misc-forwarding-reference-overload, bugprone-forwarding-reference-overload)
		: m_strm(nullptr)
		, m_unget(0)
		, m_string('"')
		, m_support_line_markers(true) // FIXME
		, m_token_queue(nullptr)
		{
			clear();
		}

		tokenizer_t(const tokenizer_t &) = delete;
		tokenizer_t &operator=(const tokenizer_t &) = delete;
		tokenizer_t(tokenizer_t &&) noexcept = delete;
		tokenizer_t &operator=(tokenizer_t &&) noexcept = delete;

		virtual ~tokenizer_t() = default;

		// tokenizer stuff follows ...

		token_id_t register_token(const pstring &token)
		{
			token_id_t ret(m_tokens.size(), token);
			m_tokens.emplace(token, ret);
			return ret;
		}

		tokenizer_t & identifier_chars(const pstring &s) { m_identifier_chars = s; return *this; }
		tokenizer_t & number_chars(const pstring &st, const pstring & rem) { m_number_chars_start = st; m_number_chars = rem; return *this; }
		tokenizer_t & string_char(pstring::value_type c) { m_string = c; return *this; }
		tokenizer_t & whitespace(const pstring & s) { m_whitespace = s; return *this; }
		tokenizer_t & comment(const pstring &start, const pstring &end, const pstring &line)
		{
			m_tok_comment_start = register_token(start);
			m_tok_comment_end = register_token(end);
			m_tok_line_comment = register_token(line);
			return *this;
		}

		void append_to_store(putf8_reader *reader, token_store_t &store);

	private:

		void clear()
		{
			m_cur_line = "";
			m_px = m_cur_line.begin();
			m_unget = 0;
		}

		token_t get_token_internal();

		// get internal token with comment processing
		token_t get_token_comment();

		void skip_eol();

		pstring::value_type getc();
		void ungetc(pstring::value_type c);

		bool eof() const { return m_strm->eof(); }

		putf8_reader *m_strm;

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

	protected:
		bool m_support_line_markers;
		token_store_t *m_token_queue;
	};

	class token_reader_t
	{
	public:

		using token_t = tokenizer_t::token_t;
		using token_type = tokenizer_t::token_type;
		using token_id_t = tokenizer_t::token_id_t;
		using token_store = tokenizer_t::token_store_t;

		explicit token_reader_t()
		: m_idx(0)
		, m_token_store(nullptr)
		{
			// add a first entry to the stack
			m_source_location.emplace_back(plib::source_location("Unknown", 0));
		}

		token_reader_t(const token_reader_t &) = delete;
		token_reader_t &operator=(const token_reader_t &) = delete;
		token_reader_t(token_reader_t &&) noexcept = delete;
		token_reader_t &operator=(token_reader_t &&) noexcept = delete;

		virtual ~token_reader_t() = default;

		void set_token_source(const token_store *store)
		{
			m_token_store = store;
		}

		pstring current_line_str() const;

		// tokenizer stuff follows ...

		token_t get_token();
		token_t get_token_raw(); // includes line information
		pstring get_string();
		pstring get_identifier();
		pstring get_identifier_or_number();

		double get_number_double();
		long get_number_long();

		void require_token(const token_id_t &token_num);
		void require_token(const token_t &tok, const token_id_t &token_num);

		void error(const perrmsg &errs);

		plib::source_location location() { return m_source_location.back(); }

		pstring current_line() const { return m_line; }

	protected:
		virtual void verror(const pstring &msg) = 0;

	private:
		bool process_line_token(const token_t &tok);

		token_t get_token_queue()
		{
			if (m_idx < m_token_store->size())
				return (*m_token_store)[m_idx++];
			return token_t(token_type::ENDOFFILE);
		}

		// source locations, vector used as stack because we need to loop through stack

		std::vector<plib::source_location> m_source_location;
		pstring m_line;
		std::size_t m_idx;
		const token_store * m_token_store;
	};

} // namespace plib

#endif // PTOKENIZER_H_
