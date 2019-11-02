// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * putil.h
 *
 */

#ifndef PUTIL_H_
#define PUTIL_H_

#include "palloc.h"
#include "pexception.h"
#include "pstring.h"

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>

#define PSTRINGIFY_HELP(y) # y
#define PSTRINGIFY(x) PSTRINGIFY_HELP(x)

// FIXME:: __FUNCTION__ may be not be supported by all compilers.

#define PSOURCELOC() plib::source_location(__FILE__, __LINE__)

namespace plib
{

	/**! Source code locations
	 *
	 * The c++20 draft for source locations is based on const char * strings.
	 * It is thus only suitable for c++ source code and not for programmatic
	 * parsing of files. This class is a replacement for dynamic use cases.
	 *
	 */
	struct source_location
	{
		source_location() noexcept
		: m_file("unknown"), m_func(m_file), m_line(0), m_col(0)
		{ }

		source_location(pstring file, unsigned line) noexcept
		: m_file(std::move(file)), m_func("unknown"), m_line(line), m_col(0)
		{ }

		source_location(pstring file, pstring func, unsigned line) noexcept
		: m_file(std::move(file)), m_func(func), m_line(line), m_col(0)
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

	/**! Base source class
	 *
	 * Pure virtual class all other source implementations are based on.
	 * Sources provide an abstraction to read input from a variety of
	 * sources, e.g. files, memory, remote locations.
	 *
	 */
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

	/**! Generic string source
	 *
	 * Will return the given string when name matches.
	 * Is used in preprocessor code to eliminate inclusion of certain files.
	 *
	 * @tparam TS base stream class. Default is psource_t
	 */
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

	/**! Generic sources collection
	 *
	 * @tparam TS base stream class. Default is psource_t
	 */
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

	/* May be further specialized .... This is the generic version */
	template <typename T>
	struct constants
	{
		static inline constexpr T zero()   noexcept { return static_cast<T>(0); }
		static inline constexpr T one()    noexcept { return static_cast<T>(1); }
		static inline constexpr T two()    noexcept { return static_cast<T>(2); }
		static inline constexpr T sqrt2()  noexcept { return static_cast<T>(1.414213562373095048801688724209); }
		static inline constexpr T pi()     noexcept { return static_cast<T>(3.14159265358979323846264338327950); }

		/*!
		 * \brief Electric constant of vacuum
		 */
		static inline constexpr T eps_0() noexcept { return static_cast<T>(8.854187817e-12); }
		/*!
		 * \brief Relative permittivity of Silicon dioxide
		 */
		static inline constexpr T eps_SiO2() noexcept { return static_cast<T>(3.9); }
		/*!
		 * \brief Relative permittivity of Silicon
		 */
		static inline constexpr T eps_Si() noexcept { return static_cast<T>(11.7); }
		/*!
		 * \brief Boltzmann constant
		 */
		static inline constexpr T k_b() noexcept { return static_cast<T>(1.38064852e-23); }
		/*!
		 * \brief room temperature (gives VT = 0.02585 at T=300)
		 */
		static inline constexpr T T0() noexcept { return static_cast<T>(300); }
		/*!
		 * \brief Elementary charge
		 */
		static inline constexpr T Q_e() noexcept { return static_cast<T>(1.6021765314e-19); }
		/*!
		 * \brief Intrinsic carrier concentration in 1/m^3 of Silicon
		 */
		static inline constexpr T NiSi() noexcept { return static_cast<T>(1.45e16); }

		template <typename V>
		static inline constexpr const T cast(V &&v) noexcept { return static_cast<T>(v); }
	};

	/*! typesafe reciprocal function
	 *
	 * @tparam T type of the argument
	 * @param  v argument
	 * @return reciprocal of argument
	 */
	template <typename T>
	static inline constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type
	reciprocal(T v) noexcept
	{
		return constants<T>::one() / v;
	}

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

	// ----------------------------------------------------------------------------------------
	// number conversions
	// ----------------------------------------------------------------------------------------

	template <typename T, typename S>
	T pstonum_locale(const std::locale &loc, const S &arg, std::size_t *idx)
	{
		std::stringstream ss;
		ss.imbue(loc);
		ss << arg;
		auto len(ss.tellp());
		T x(constants<T>::zero());
		if (ss >> x)
		{
			auto pos(ss.tellg());
			if (pos == static_cast<decltype(pos)>(-1))
				pos = len;
			*idx = static_cast<std::size_t>(pos);
		}
		else
			*idx = constants<std::size_t>::zero();
		//printf("%s, %f, %lu %ld\n", arg, (double)x, *idx, (long int) ss.tellg());
		return x;
	}

	template <typename T, typename E = void>
	struct pstonum_helper;

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type>
	{
		template <typename S>
		long long operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<long long>(loc, arg, idx);
		}
	};

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value>::type>
	{
		template <typename S>
		unsigned long long operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			//return std::stoll(arg, idx);
			return pstonum_locale<unsigned long long>(loc, arg, idx);
		}
	};

	template<typename T>
	struct pstonum_helper<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
	{
		template <typename S>
		long double operator()(std::locale loc, const S &arg, std::size_t *idx)
		{
			return pstonum_locale<long double>(loc, arg, idx);
		}
	};

	template<typename T, typename S>
	T pstonum(const S &arg, const std::locale &loc = std::locale::classic())
	{
		decltype(arg.c_str()) cstr = arg.c_str();
		std::size_t idx(0);
		auto ret = pstonum_helper<T>()(loc, cstr, &idx);
		using ret_type = decltype(ret);
		if (ret >= static_cast<ret_type>(std::numeric_limits<T>::lowest())
			&& ret <= static_cast<ret_type>(std::numeric_limits<T>::max()))
			//&& (ret == T(0) || std::abs(ret) >= std::numeric_limits<T>::min() ))
		{
			if (cstr[idx] != 0)
				throw pexception(pstring("Continuation after numeric value ends: ") + pstring(cstr));
		}
		else
		{
			throw pexception(pstring("Out of range: ") + pstring(cstr));
		}
		return static_cast<T>(ret);
	}

	template<typename R, typename T>
	R pstonum_ne(const T &str, bool &err, std::locale loc = std::locale::classic()) noexcept
	{
		try
		{
			err = false;
			return pstonum<R>(str, loc);
		}
		catch (...)
		{
			err = true;
			return R(0);
		}
	}

	template<typename R, typename T>
	R pstonum_ne_def(const T &str, R def, std::locale loc = std::locale::classic()) noexcept
	{
		try
		{
			return pstonum<R>(str, loc);
		}
		catch (...)
		{
			return def;
		}
	}

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


#endif /* PUTIL_H_ */
