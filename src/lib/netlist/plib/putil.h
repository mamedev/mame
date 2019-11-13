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
#include <vector>

#define PSTRINGIFY_HELP(y) # y
#define PSTRINGIFY(x) PSTRINGIFY_HELP(x)

#define PNARGS_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define PNARGS(...) PNARGS_(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define PCONCAT_(a, b) a ## b
#define PCONCAT(a, b) PCONCAT_(a, b)

#define PSTRINGIFY_1(x)                     #x
#define PSTRINGIFY_2(x, x2)                 #x, #x2
#define PSTRINGIFY_3(x, ...)                #x, PSTRINGIFY_2(__VA_ARGS__)
#define PSTRINGIFY_4(x, ...)                #x, PSTRINGIFY_3(__VA_ARGS__)
#define PSTRINGIFY_5(x, ...)                #x, PSTRINGIFY_4(__VA_ARGS__)
#define PSTRINGIFY_6(x, ...)                #x, PSTRINGIFY_5(__VA_ARGS__)
#define PSTRINGIFY_7(x, ...)                #x, PSTRINGIFY_6(__VA_ARGS__)
#define PSTRINGIFY_8(x, ...)                #x, PSTRINGIFY_7(__VA_ARGS__)
#define PSTRINGIFY_9(x, ...)                #x, PSTRINGIFY_8(__VA_ARGS__)
#define PSTRINGIFY_10(x, ...)               #x, PSTRINGIFY_9(__VA_ARGS__)
#define PSTRINGIFY_11(x, ...)               #x, PSTRINGIFY_10(__VA_ARGS__)
#define PSTRINGIFY_12(x, ...)               #x, PSTRINGIFY_11(__VA_ARGS__)
#define PSTRINGIFY_13(x, ...)               #x, PSTRINGIFY_12(__VA_ARGS__)
#define PSTRINGIFY_14(x, ...)               #x, PSTRINGIFY_13(__VA_ARGS__)
#define PSTRINGIFY_15(x, ...)               #x, PSTRINGIFY_14(__VA_ARGS__)
#define PSTRINGIFY_16(x, ...)               #x, PSTRINGIFY_15(__VA_ARGS__)

#define PSTRINGIFY_VA(...) PCONCAT(PSTRINGIFY_, PNARGS(__VA_ARGS__))(__VA_ARGS__)

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

		using stream_ptr = plib::unique_ptr<std::istream>;

		psource_t() noexcept = default;

		COPYASSIGNMOVE(psource_t, delete)

		virtual ~psource_t() noexcept = default;

		virtual stream_ptr stream(const pstring &name) = 0;
	private:
	};

	/// \brief Generic string source.
	///
	/// Will return the given string when name matches.
	/// Is used in preprocessor code to eliminate inclusion of certain files.
	///
	/// \tparam TS base stream class. Default is psource_t
	///
	template <typename TS = psource_t>
	class psource_str_t : public TS
	{
	public:
		psource_str_t(pstring name, pstring str)
		: m_name(std::move(name)), m_str(std::move(str))
		{}

		COPYASSIGNMOVE(psource_str_t, delete)
		~psource_str_t() noexcept override = default;

		typename TS::stream_ptr stream(const pstring &name) override
		{
			if (name == m_name)
				return plib::make_unique<std::stringstream>(m_str);
			else
				return typename TS::stream_ptr(nullptr);
		}
	private:
		pstring m_name;
		pstring m_str;
	};

	/// \brief Generic sources collection.
	///
	/// \tparam TS base stream class. Default is psource_t
	///
	template <typename TS = psource_t>
	class psource_collection_t
	{
	public:
		using source_type = plib::unique_ptr<TS>;
		using list_t = std::vector<source_type>;

		psource_collection_t() noexcept = default;

		COPYASSIGNMOVE(psource_collection_t, delete)
		virtual ~psource_collection_t() noexcept = default;

		void add_source(source_type &&src)
		{
			m_collection.push_back(std::move(src));
		}

		template <typename S = TS>
		typename S::stream_ptr get_stream(pstring name)
		{
			for (auto &s : m_collection)
			{
				auto source(dynamic_cast<S *>(s.get()));
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
				auto source(dynamic_cast<S *>(s.get()));
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
		pstring basename(const pstring &filename);
		pstring path(const pstring &filename);
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
				return static_cast<std::size_t>(it - con.begin());
			return npos;
		}

		template <class C>
		void insert_at(C &con, const std::size_t index, const typename C::value_type &elem)
		{
			con.insert(con.begin() + static_cast<std::ptrdiff_t>(index), elem);
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
			const std::size_t maxsplit);

	//============================================================
	//  penum - strongly typed enumeration
	//============================================================

	struct penum_base
	{
	protected:
		static int from_string_int(const pstring &str, const pstring &x);
		static std::string nthstr(int n, const pstring &str);
	};

} // namespace plib

#define P_ENUM(ename, ...) \
	struct ename : public plib::penum_base { \
		enum E { __VA_ARGS__ }; \
		ename (E v) : m_v(v) { } \
		template <typename T> explicit ename(T val) { m_v = static_cast<E>(val); } \
		bool set_from_string (const pstring &s) { \
			int f = from_string_int(strings(), s); \
			if (f>=0) { m_v = static_cast<E>(f); return true; } else { return false; } \
		} \
		operator E() const noexcept {return m_v;} \
		bool operator==(const ename &rhs) const noexcept {return m_v == rhs.m_v;} \
		bool operator==(const E &rhs) const noexcept {return m_v == rhs;} \
		std::string name() const { \
			return nthstr(static_cast<int>(m_v), strings()); \
		} \
		private: E m_v; \
		static pstring strings() {\
			static const pstring lstrings = # __VA_ARGS__; \
			return lstrings; \
		} \
	};

#endif // PUTIL_H_
