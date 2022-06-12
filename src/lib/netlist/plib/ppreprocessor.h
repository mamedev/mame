// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PPREPROCESSOR_H_
#define PPREPROCESSOR_H_

///
/// \file ppreprocessor.h
///

#include "pstream.h"
#include "pstring.h"

#include "psource.h"
#include "putil.h"

#include <istream>
#include <unordered_map>
#include <vector>

namespace plib {

	class ppreprocessor
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

		explicit ppreprocessor(psource_collection_t &sources, defines_map_type *defines = nullptr);

		PCOPYASSIGNMOVE(ppreprocessor, delete)

		~ppreprocessor() = default;

		/// \brief process stream
		///
		/// \param filename a filename or identifier identifying the stream.
		///
		/// FIXME: this is sub-optimal. Refactor input_context into `pinput`_context
		/// and pass this to `ppreprocessor`.
		///
		template <typename T>
		pstring process(T &&istrm, const pstring &filename)
		{
			m_outbuf.clear();
			m_stack.emplace_back(input_context(istrm.release_stream(),plib::util::path(filename), filename));
			process_stack();
			return m_outbuf;
		}

		[[noreturn]] void error(const pstring &err) noexcept(false);

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
		static bool is_valid_token(const pstring &str);

		std::pair<pstring,bool> process_line(const pstring &line_in);
		pstring process_comments(const pstring &line);

		defines_map_type m_defines;
		psource_collection_t &m_sources;
		string_list m_expr_sep;

		std::uint_least64_t m_if_flag; // 63 if levels
		std::uint_least64_t m_if_seen; // 63 if levels
		std::uint_least64_t m_elif; // 63 if levels - for #elif
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
		pstring m_outbuf;
		state_e m_state;
		pstring m_line;
		bool m_comment;
		bool m_debug_out;

	};

} // namespace plib

#endif // PPREPROCESSOR_H_
