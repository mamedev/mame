// license:BSD-3-Clause
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

#define PSTRINGIFY_HELP(y) #y
#define PSTRINGIFY(x) PSTRINGIFY_HELP(x)

// Discussion and background of this MSVC bug:
// https://github.com/mamedev/mame/issues/6106
///
/// \brief Macro to work around a bug in MSVC treatment of __VA_ARGS__
///
#define PMSVC_VARARG_BUG(MACRO, ARGS) MACRO ARGS

// clang-format off

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

// clang-format on

/// \brief Individually stringify up to 16 arguments
///
/// PSTRINGIFY_VA(a, b, c) will be expanded to "a", "b", "c"
///
/// \returns List of stringified individual arguments
///
#define PSTRINGIFY_VA(...)                                                     \
	PMSVC_VARARG_BUG(PCONCAT, (PSTRINGIFY_, PNARGS(__VA_ARGS__)))(__VA_ARGS__)

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
#define PCALLVARARG(MAC, ...)                                                  \
	PMSVC_VARARG_BUG(PCONCAT, (MAC, PNARGS(__VA_ARGS__)))(__VA_ARGS__)

// FIXME:: __FUNCTION__ may be not be supported by all compilers.

#define PSOURCELOC() plib::source_location(__FILE__, __LINE__)

namespace plib
{

	namespace util
	{
		pstring basename(const pstring &filename, const pstring &suffix = "");
		pstring path(const pstring &filename);
		bool    exists(const pstring &filename);
		pstring build_path(std::initializer_list<pstring> list);
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
		std::size_t index_of(C &con, const typename C::value_type &elem)
		{
			auto it = std::find(con.begin(), con.end(), elem);
			if (it != con.end())
				return narrow_cast<std::size_t>(it - con.begin());
			return npos;
		}

		template <class C>
		void insert_at(C &con, const std::size_t index,
			const typename C::value_type &elem)
		{
			con.insert(con.begin() + narrow_cast<std::ptrdiff_t>(index), elem);
		}

		template <class C>
		void remove(C &con, const typename C::value_type &elem)
		{
			con.erase(std::remove(con.begin(), con.end(), elem), con.end());
		}
	} // namespace container

	/// \brief Consistent hash implementation
	///
	/// Hash implementations in c++ standard libraries may differ and deliver
	/// different results. This hash can be used as a replacement hash in e.g.
	/// std::map to deliver consistent results.
	///
	/// \tparam V result type
	/// \tparam T buffer element type
	/// \param buf pointer to buffer for which hash should be calculated
	/// \param size number of buffer elements
	/// \return the hash of the buffer
	///
	template <typename V, typename T>
	constexpr V hash(const T *buf, std::size_t size) noexcept
	{
		V result = 5381; // NOLINT
		for (const T *p = buf; p != buf + size; p++)
			result = ((result << 5) + result) ^ (result >> (32 - 5))
					 ^ narrow_cast<std::size_t>(*p); // NOLINT
		return result;
	}

	/// \brief Execute code at end of block
	///
	/// The class can be used to execute code at the end of a block. It follows
	/// the same design as std::lock_guard.
	///
	/// Since the functor is executed in the destructor of the class the
	/// execution is protected by a try catch block. If an exception is raised,
	/// \ref plib::terminate is called.
	///
	/// \tparam F type of functor - will be derived by template deduction guide
	///
	template <typename F>
	struct functor_guard
	{
		/// \brief constructor
		///
		/// \param f functor to execute
		functor_guard(F &&f)
		: m_f(std::move(f))
		{
		}
		/// \brief destructor
		///
		~functor_guard()
		{
			try
			{
				m_f();
			}
			catch (...)
			{
				plib::terminate("exception raised in lambda_guard");
			}
		}

	private:
		F m_f;
	};

	/// \brief template deduction guide
	///
	template <class F>
	functor_guard(F f) -> functor_guard<F>;

} // namespace plib

#endif // PUTIL_H_
