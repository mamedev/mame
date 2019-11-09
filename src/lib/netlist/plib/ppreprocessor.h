// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PPREPROCESSOR_H_
#define PPREPROCESSOR_H_

///
/// \file ppreprocessor.h
///

#include "plists.h"
#include "pstream.h"
#include "pstring.h"

#include "putil.h" // psource_t

#include <istream>
#include <unordered_map>
#include <vector>

namespace plib {

	class ppreprocessor : public std::istream
	{
	public:

		using string_list = std::vector<pstring>;

		struct define_t
		{
			define_t(const pstring &name, const pstring &replace)
			: m_name(name), m_replace(replace), m_has_params(false)
			{}
			explicit define_t(const pstring &name)
			: m_name(name), m_replace(""), m_has_params(false)
			{}
			pstring m_name;
			pstring m_replace;
			bool m_has_params;
			string_list m_params;
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
        , m_stack(std::move(s.m_stack))
		, m_outbuf(std::move(s.m_outbuf))
		, m_pos(s.m_pos)
		, m_state(s.m_state)
		, m_line(std::move(s.m_line))
		, m_comment(s.m_comment)
		, m_debug_out(s.m_debug_out)
		{
		}

		~ppreprocessor() override
		{
			delete rdbuf();
		}

		template <typename T>
		ppreprocessor & process(T &&istrm)
		{
			m_stack.emplace_back(input_context(std::forward<T>(istrm),"","<stream>"));
			process_stack();
			return *this;
		}

		[[noreturn]] void error(const pstring &err);

	protected:

		class readbuffer : public std::streambuf
		{
		public:
			explicit readbuffer(ppreprocessor *strm) : m_strm(strm), m_buf() 
			{ setg(nullptr, nullptr, nullptr); }
			readbuffer(readbuffer &&rhs) noexcept : m_strm(rhs.m_strm), m_buf()  {}
			COPYASSIGN(readbuffer, delete)
			readbuffer &operator=(readbuffer &&src) = delete;
			~readbuffer() override = default;

			int_type underflow() override
			{
				if (this->gptr() == this->egptr())
				{
					// clang reports sign error - weird
					std::size_t bytes = pstring_mem_t_size(m_strm->m_outbuf) - static_cast<std::size_t>(m_strm->m_pos);

					if (bytes > m_buf.size())
						bytes = m_buf.size();
					std::copy(m_strm->m_outbuf.c_str() + m_strm->m_pos, m_strm->m_outbuf.c_str() + m_strm->m_pos + bytes, m_buf.data());
					//printf("%ld\n", (long int)bytes);
					this->setg(m_buf.data(), m_buf.data(), m_buf.data() + bytes);

					m_strm->m_pos += static_cast<long>(bytes);
				}

				return this->gptr() == this->egptr()
					 ? std::char_traits<char>::eof()
					 : std::char_traits<char>::to_int_type(*this->gptr());
			}

		private:
			ppreprocessor *m_strm;
			std::array<char_type, 1024> m_buf;
		};
		define_t *get_define(const pstring &name);
		pstring replace_macros(const pstring &line);

	private:

		enum state_e
		{
			PROCESS,
			LINE_CONTINUATION
		};

		void push_out(const pstring &s);

		void process_stack();

		string_list tokenize(const pstring &str, const string_list &sep, bool remove_ws, bool concat);
		bool is_valid_token(const pstring &str);

		std::pair<pstring,bool> process_line(pstring line);
		pstring process_comments(pstring line);

		defines_map_type m_defines;
		psource_collection_t<> &m_sources;
		string_list m_expr_sep;

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

		// vector used as stack because we need to loop through stack
		std::vector<input_context> m_stack;
		pstring_t<pu8_traits> m_outbuf;
		std::istream::pos_type m_pos;
		state_e m_state;
		pstring m_line;
		bool m_comment;
		bool m_debug_out;

	};

} // namespace plib

#endif // PPREPROCESSOR_H_
