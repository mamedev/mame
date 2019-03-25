// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * putil.h
 *
 */

#ifndef PUTIL_H_
#define PUTIL_H_

#include "pstring.h"

#include <algorithm>
#include <initializer_list>
#include <vector>

#define PSTRINGIFY_HELP(y) # y
#define PSTRINGIFY(x) PSTRINGIFY_HELP(x)


namespace plib
{

	namespace util
	{
		const pstring buildpath(std::initializer_list<pstring> list );
		const pstring environment(const pstring &var, const pstring &default_val);
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

	/* May be further specialized .... This is the generic version */
	template <typename T>
	struct constants
	{
		static constexpr T zero() noexcept { return static_cast<T>(0); }
		static constexpr T one()  noexcept { return static_cast<T>(1); }
		static constexpr T two()  noexcept { return static_cast<T>(2); }

		/*!
		 * \brief Electric constant of vacuum
		 */
		static constexpr T eps_0() noexcept { return static_cast<T>(8.854187817e-12); }
		/*!
		 * \brief Relative permittivity of Silicon dioxide
		 */
		static constexpr T eps_SiO2() noexcept { return static_cast<T>(3.9); }
		/*!
		 * \brief Relative permittivity of Silicon
		 */
		static constexpr T eps_Si() noexcept { return static_cast<T>(11.7); }
		/*!
		 * \brief Boltzmann constant
		 */
		static constexpr T k_b() noexcept { return static_cast<T>(1.38064852e-23); }
		/*!
		 * \brief room temperature (gives VT = 0.02585 at T=300)
		 */
		static constexpr T T0() noexcept { return static_cast<T>(300); }
		/*!
		 * \brief Elementary charge
		 */
		static constexpr T Q_e() noexcept { return static_cast<T>(1.6021765314e-19); }
		/*!
		 * \brief Intrinsic carrier concentration in 1/m^3 of Silicon
		 */
		static constexpr T NiSi() noexcept { return static_cast<T>(1.45e16); }

		template <typename V>
		static constexpr const T cast(V &&v) noexcept { return static_cast<T>(v); }
	};

	static_assert(noexcept(constants<double>::one()) == true, "Not evaluated as constexpr");


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
		static int from_string_int(const char *str, const char *x);
		static std::string nthstr(int n, const char *str);
	};

} // namespace plib

#define P_ENUM(ename, ...) \
	struct ename : public plib::penum_base { \
		enum E { __VA_ARGS__ }; \
		ename (E v) : m_v(v) { } \
		bool set_from_string (const std::string &s) { \
			static char const *const strings = # __VA_ARGS__; \
			int f = from_string_int(strings, s.c_str()); \
			if (f>=0) { m_v = static_cast<E>(f); return true; } else { return false; } \
		} \
		operator E() const {return m_v;} \
		bool operator==(const ename &rhs) const {return m_v == rhs.m_v;} \
		bool operator==(const E &rhs) const {return m_v == rhs;} \
		std::string name() const { \
			static char const *const strings = # __VA_ARGS__; \
			return nthstr(static_cast<int>(m_v), strings); \
		} \
		private: E m_v; };


#endif /* PUTIL_H_ */
