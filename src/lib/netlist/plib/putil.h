// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef PUTIL_H_
#define PUTIL_H_

///
/// \file putil.h
///

#include "palloc.h"
#include "pexception.h"
#include "pstring.h"

#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <vector>

#define PSTRINGIFY_HELP(y) # y
#define PSTRINGIFY(x) PSTRINGIFY_HELP(x)

// Discussion and background of this MSVC bug: https://github.com/mamedev/mame/issues/6106
///
/// \brief Macro to work around a bug in MSVC treatment of __VA_ARGS__
///
#define PMSVC_VARARG_BUG(MACRO, ARGS) MACRO ARGS

/// \brief Determine number of arguments in __VA_ARGS__
///
/// This macro works up to 16 arguments in __VA_ARGS__
///
/// \returns Number of arguments
///
#define PNARGS(...) PNARGS_1(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define PNARGS_2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define PNARGS_1(...) PMSVC_VARARG_BUG(PNARGS_2, (__VA_ARGS__))

/// \brief Concatenate two arguments after expansion
///
/// \returns Concatenated expanded arguments
///
#define PCONCAT(a, b) PCONCAT_(a, b)

#define PCONCAT_(a, b) a ## b

#define PSTRINGIFY_1(x)                             #x
#define PSTRINGIFY_2(x, x2)                         #x, #x2
#define PSTRINGIFY_3(x, x2, x3)                     #x, #x2, #x3
#define PSTRINGIFY_4(x, x2, x3, x4)                 #x, #x2, #x3, #x4
#define PSTRINGIFY_5(x, x2, x3, x4, x5)             #x, #x2, #x3, #x4, #x5
#define PSTRINGIFY_6(x, x2, x3, x4, x5, x6)         #x, #x2, #x3, #x4, #x5, #x6
#define PSTRINGIFY_7(x, x2, x3, x4, x5, x6, x7)     #x, #x2, #x3, #x4, #x5, #x6, #x7
#define PSTRINGIFY_8(x, x2, x3, x4, x5, x6, x7, x8) #x, #x2, #x3, #x4, #x5, #x6, #x7, #x8
#define PSTRINGIFY_9(x, x2, x3, x4, x5, x6, x7, x8, x9) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9
#define PSTRINGIFY_10(x, x2, x3, x4, x5, x6, x7, x8, x9, xa) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa
#define PSTRINGIFY_11(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb
#define PSTRINGIFY_12(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb, #xc
#define PSTRINGIFY_13(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb, #xc, #xd
#define PSTRINGIFY_14(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb, #xc, #xd, #xe
#define PSTRINGIFY_15(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb, #xc, #xd, #xe, #xf
#define PSTRINGIFY_16(x, x2, x3, x4, x5, x6, x7, x8, x9, xa, xb, xc, xd, xe, xf, x10) \
	#x, #x2, #x3, #x4, #x5, #x6, #x7, #x8, #x9, #xa, #xb, #xc, #xd, #xe, #xf, #x10

/// \brief Individually stringify up to 16 arguments
///
/// PSTRINGIFY_VA(a, b, c) will be expanded to "a", "b", "c"
///
/// \returns List of stringified individual arguments
///
#define PSTRINGIFY_VA(...) PMSVC_VARARG_BUG(PCONCAT, (PSTRINGIFY_, PNARGS(__VA_ARGS__)))(__VA_ARGS__)

/// \brief Dispatch VARARG macro to specialized macros
///
/// ```
/// #define LOCAL_LIB_ENTRY(...) PCALLVARARG(LOCAL_LIB_ENTRY_, __VA_ARGS__)
/// ```
///
/// Will pass varargs depending on number of arguments to
///
/// ```
/// LOCAL_LIB_ENTRY_1(a1)
/// LOCAL_LIB_ENTRY_2(a1 , a2)
/// ```
///
/// \returns result of specialized macro
///
#define PCALLVARARG(MAC, ...) PMSVC_VARARG_BUG(PCONCAT, (MAC, PNARGS(__VA_ARGS__)))(__VA_ARGS__)

// FIXME:: __FUNCTION__ may be not be supported by all compilers.

#define PSOURCELOC() plib::source_location(__FILE__, __LINE__)

namespace plib
{

	/// \brief Source code locations.
	///
	/// The c++20 draft for source locations is based on const char * strings.
	/// It is thus only suitable for c++ source code and not for programmatic
	/// parsing of files. This class is a replacement for dynamic use cases.
	///
	struct source_location
	{
		source_location() noexcept
		: m_file("unknown"), m_func(m_file), m_line(0), m_col(0)
		{ }

		source_location(pstring file, unsigned line) noexcept
		: m_file(std::move(file)), m_func("unknown"), m_line(line), m_col(0)
		{ }

		source_location(pstring file, pstring func, unsigned line) noexcept
		: m_file(std::move(file)), m_func(std::move(func)), m_line(line), m_col(0)
		{ }

		PCOPYASSIGNMOVE(source_location, default)

		~source_location() = default;

		unsigned line() const noexcept { return m_line; }
		unsigned column() const noexcept { return m_col; }
		pstring file_name() const noexcept { return m_file; }
		pstring function_name() const noexcept { return m_func; }

		source_location &operator ++() noexcept
		{
			++m_line;
			return *this;
		}

	private:
		pstring m_file;
		pstring m_func;
		unsigned m_line;
		unsigned m_col;
	};

	/// \brief Base source class.
	///
	/// Pure virtual class all other source implementations are based on.
	/// Sources provide an abstraction to read input from a variety of
	/// sources, e.g. files, memory, remote locations.
	///
	class psource_t
	{
	public:

		using stream_ptr = std::unique_ptr<std::istream>;

		psource_t() noexcept = default;

		PCOPYASSIGNMOVE(psource_t, delete)

		virtual ~psource_t() noexcept = default;

		virtual stream_ptr stream(const pstring &name) = 0;
	private:
	};

	/// \brief Generic string source.
	///
	/// Will return the given string when name matches.
	/// Is used in preprocessor code to eliminate inclusion of certain files.
	///
	class psource_str_t : public psource_t
	{
	public:
		psource_str_t(pstring name, pstring str)
		: m_name(std::move(name)), m_str(std::move(str))
		{}

		PCOPYASSIGNMOVE(psource_str_t, delete)
		~psource_str_t() noexcept override = default;

		typename psource_t::stream_ptr stream(const pstring &name) override
		{
			if (name == m_name)
				return std::make_unique<std::stringstream>(m_str);

			return psource_t::stream_ptr(nullptr);
		}
	private:
		pstring m_name;
		pstring m_str;
	};

	/// \brief Generic sources collection.
	///
	/// \tparam ARENA memory arena, defaults to aligned_arena
	///
	template <typename ARENA = aligned_arena>
	class psource_collection_t
	{
	public:
		using source_type = std::unique_ptr<psource_t>;
		using list_t = std::vector<source_type>;

		psource_collection_t() noexcept = default;

		PCOPYASSIGNMOVE(psource_collection_t, delete)
		virtual ~psource_collection_t() noexcept = default;

		void add_source(source_type &&src)
		{
			m_collection.push_back(std::move(src));
		}

		template <typename S = psource_t>
		typename psource_t::stream_ptr get_stream(pstring name)
		{
			for (auto &s : m_collection)
			{
				auto *source(dynamic_cast<S *>(s.get()));
				if (source)
				{
					auto strm = source->stream(name);
					if (strm)
						return strm;
				}
			}
			return typename S::stream_ptr(nullptr);
		}

		template <typename S, typename F>
		bool for_all(F lambda)
		{
			for (auto &s : m_collection)
			{
				auto *source(dynamic_cast<S *>(s.get()));
				if (source)
				{
					if (lambda(source))
						return true;
				}
			}
			return false;
		}

	private:
		list_t m_collection;
	};

	namespace util
	{
		pstring basename(const pstring &filename, const pstring &suffix = "");
		pstring path(const pstring &filename);
		bool    exists(const pstring &filename);
		pstring buildpath(std::initializer_list<pstring> list );
		pstring environment(const pstring &var, const pstring &default_val);
	} // namespace util

	namespace container
	{
		template <class C, class T>
		bool contains(C &con, const T &elem)
		{
			return std::find(con.begin(), con.end(), elem) != con.end();
		}

		static constexpr const std::size_t npos = static_cast<std::size_t>(-1);
		template <class C>
		std::size_t indexof(C &con, const typename C::value_type &elem)
		{
			auto it = std::find(con.begin(), con.end(), elem);
			if (it != con.end())
				return narrow_cast<std::size_t>(it - con.begin());
			return npos;
		}

		template <class C>
		void insert_at(C &con, const std::size_t index, const typename C::value_type &elem)
		{
			con.insert(con.begin() + narrow_cast<std::ptrdiff_t>(index), elem);
		}

		template <class C>
		void remove(C &con, const typename C::value_type &elem)
		{
			con.erase(std::remove(con.begin(), con.end(), elem), con.end());
		}
	} // namespace container

	template <class C>
	struct indexed_compare
	{
		explicit indexed_compare(const C& target): m_target(target) {}

		bool operator()(int a, int b) const { return m_target[a] < m_target[b]; }

		const C& m_target;
	};

	// ----------------------------------------------------------------------------------------
	// string list
	// ----------------------------------------------------------------------------------------

	std::vector<pstring> psplit(const pstring &str, const pstring &onstr, bool ignore_empty = false);
	std::vector<pstring> psplit(const pstring &str, const std::vector<pstring> &onstrl);
	std::vector<std::string> psplit_r(const std::string &stri,
			const std::string &token,
			std::size_t maxsplit);

	// ----------------------------------------------------------------------------------------
	// simple hash
	// ----------------------------------------------------------------------------------------

	template <typename T>
	std::size_t hash(const T *buf, std::size_t size)
	{
		std::size_t result = 5381; // NOLINT
		for (const T* p = buf; p != buf + size; p++)
			result = ((result << 5) + result ) ^ (result >> (32 - 5)) ^ narrow_cast<std::size_t>(*p); // NOLINT
		return result;
	}

} // namespace plib

#endif // PUTIL_H_
