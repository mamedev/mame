// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PMATH_H_
#define PMATH_H_

///
/// \file pmath.h
///

#include "pconfig.h"
#include "ptypes.h"

#include <algorithm>
#include <cmath>
#include <type_traits>

// `quadmath.h` included by `ptypes.h`

namespace plib
{

	/// \brief Holds constants used repeatedly.
	///
	///  \tparam T floating point type
	///
	///  Using the structure members we can avoid magic numbers in the code.
	///  In addition, this is a typesafe approach.
	///
	template <typename T>
	struct constants
	{
		static constexpr T zero()   noexcept { return static_cast<T>(0); } // NOLINT
		static constexpr T half()   noexcept { return static_cast<T>(0.5); } // NOLINT
		static constexpr T one()    noexcept { return static_cast<T>(1); } // NOLINT
		static constexpr T two()    noexcept { return static_cast<T>(2); } // NOLINT
		static constexpr T three()  noexcept { return static_cast<T>(3); } // NOLINT
		static constexpr T four()   noexcept { return static_cast<T>(4); } // NOLINT
		static constexpr T hundred()noexcept { return static_cast<T>(100); } // NOLINT

		static constexpr T one_thirds()    noexcept { return fraction(one(), three()); }
		static constexpr T two_thirds()    noexcept { return fraction(two(), three()); }

		static constexpr T ln2()  noexcept { return static_cast<T>(0.6931471805599453094172321214581766L); } // NOLINT
		static constexpr T sqrt2()  noexcept { return static_cast<T>(1.4142135623730950488016887242096982L); } // NOLINT
		static constexpr T sqrt3()  noexcept { return static_cast<T>(1.7320508075688772935274463415058723L); } // NOLINT
		static constexpr T sqrt3_2()  noexcept { return static_cast<T>(0.8660254037844386467637231707529362L); } // NOLINT
		static constexpr T pi()     noexcept { return static_cast<T>(3.1415926535897932384626433832795029L); } // NOLINT

		/// \brief Electric constant of vacuum
		///
		static constexpr T eps_0() noexcept { return static_cast<T>(8.854187817e-12); } // NOLINT

		// \brief Relative permittivity of Silicon dioxide
		///
		static constexpr T eps_SiO2() noexcept { return static_cast<T>(3.9); } // NOLINT

		/// \brief Relative permittivity of Silicon
		///
		static constexpr T eps_Si() noexcept { return static_cast<T>(11.7); } // NOLINT

		/// \brief Boltzmann constant
		///
		static constexpr T k_b() noexcept { return static_cast<T>(1.38064852e-23); } // NOLINT

		/// \brief room temperature (gives VT = 0.02585 at T=300)
		///
		static constexpr T T0() noexcept { return static_cast<T>(300); } // NOLINT

		/// \brief Elementary charge
		///
		static constexpr T Q_e() noexcept { return static_cast<T>(1.6021765314e-19); } // NOLINT

		/// \brief Intrinsic carrier concentration in 1/m^3 of Silicon
		///
		static constexpr T NiSi() noexcept { return static_cast<T>(1.45e16); } // NOLINT

		/// \brief clearly identify magic numbers in code
		///
		/// Magic numbers should be avoided. The magic member at least clearly
		/// identifies them and makes it easier to convert them to named constants
		/// later.
		///
		template <typename V>
		static constexpr T magic(V &&v) noexcept { return static_cast<T>(v); }

		template <typename V>
		static constexpr T fraction(V &&v1, V &&v2) noexcept { return static_cast<T>(v1 / v2); }
	};

	/// \brief typesafe reciprocal function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return reciprocal of argument
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	reciprocal(T v) noexcept
	{
		return constants<T>::one() / v;
	}

	/// \brief abs function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return absolute value of argument
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	abs(T v) noexcept
	{
		return std::abs(v);
	}

	/// \brief sqrt function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return absolute value of argument
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	sqrt(T v) noexcept
	{
		return std::sqrt(v);
	}

	/// \brief hypot function
	///
	/// \tparam T type of the arguments
	/// \param  v1 first argument
	/// \param  v2 second argument
	/// \return sqrt(v1*v1+v2*v2)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	hypot(T v1, T v2) noexcept
	{
		return std::hypot(v1, v2);
	}

	/// \brief exp function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return exp(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	exp(T v) noexcept
	{
		return std::exp(v);
	}

	/// \brief log function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return log(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	log(T v) noexcept
	{
		return std::log(v);
	}

	/// \brief tanh function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return tanh(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	tanh(T v) noexcept
	{
		return std::tanh(v);
	}

	/// \brief floor function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return floor(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	floor(T v) noexcept
	{
		return std::floor(v);
	}

	/// \brief log1p function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return log(1 + v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	log1p(T v) noexcept
	{
		return std::log1p(v);
	}

	/// \brief sin function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return sin(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	sin(T v) noexcept
	{
		return std::sin(v);
	}

	/// \brief cos function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return cos(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	cos(T v) noexcept
	{
		return std::cos(v);
	}

	/// \brief trunc function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return trunc(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	trunc(T v) noexcept
	{
		return std::trunc(v);
	}

	/// \brief signum function
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \param  r optional argument, if given will return r and -r instead of 1 and -1
	/// \return signum(v)
	///
	template <typename T>
	static constexpr std::enable_if_t<std::is_floating_point<T>::value, T>
	signum(T v, T r = static_cast<T>(1))
	{
		constexpr const T z(static_cast<T>(0));
		return (v > z) ? r : ((v < z) ? -r : v);
	}

	/// \brief pow function
	///
	/// \tparam T1 type of the first argument
	/// \tparam T2 type of the second argument
	/// \param  v argument
	/// \param  p power
	/// \return v^p
	///
	/// FIXME: limited implementation
	///
	template <typename T1, typename T2>
	static inline
	auto pow(T1 v, T2 p) noexcept -> decltype(std::pow(v, p))
	{
		return std::pow(v, p);
	}

#if (PUSE_FLOAT128)
	static constexpr FLOAT128 reciprocal(FLOAT128 v) noexcept
	{
		return constants<FLOAT128>::one() / v;
	}

	static FLOAT128 abs(FLOAT128 v) noexcept
	{
		return fabsq(v);
	}

	static FLOAT128 sqrt(FLOAT128 v) noexcept
	{
		return sqrtq(v);
	}

	static FLOAT128 hypot(FLOAT128 v1, FLOAT128 v2) noexcept
	{
		return hypotq(v1, v2);
	}

	static FLOAT128 exp(FLOAT128 v) noexcept
	{
		return expq(v);
	}

	static FLOAT128 log(FLOAT128 v) noexcept
	{
		return logq(v);
	}

	static FLOAT128 tanh(FLOAT128 v) noexcept
	{
		return tanhq(v);
	}

	static FLOAT128 floor(FLOAT128 v) noexcept
	{
		return floorq(v);
	}

	static FLOAT128 log1p(FLOAT128 v) noexcept
	{
		return log1pq(v);
	}

	static FLOAT128 sin(FLOAT128 v) noexcept
	{
		return sinq(v);
	}

	static FLOAT128 cos(FLOAT128 v) noexcept
	{
		return cosq(v);
	}

	static FLOAT128 trunc(FLOAT128 v) noexcept
	{
		return truncq(v);
	}

	template <typename T>
	static FLOAT128 pow(FLOAT128 v, T p) noexcept
	{
		return powq(v, static_cast<FLOAT128>(p));
	}

	static FLOAT128 pow(FLOAT128 v, int p) noexcept
	{
		if (p==2)
			return v*v;
		else
			return powq(v, static_cast<FLOAT128>(p));
	}

#endif

	/// \brief is argument a power of two?
	///
	/// \tparam T type of the argument
	/// \param  v argument to be checked
	/// \return true if argument is a power of two
	///
	template <typename T>
	constexpr bool is_pow2(T v) noexcept
	{
		static_assert(is_integral<T>::value, "is_pow2 needs integer arguments");
		return !(v & (v-1));
	}

	/// \brief return absolute value of signed argument
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return absolute value of argument
	///
	template<typename T>
	constexpr
	std::enable_if_t<plib::is_integral<T>::value && plib::is_signed<T>::value, T>
	abs(T v) noexcept
	{
		return v < 0 ? -v : v;
	}

	/// \brief return absolute value of unsigned argument
	///
	/// \tparam T type of the argument
	/// \param  v argument
	/// \return argument since it has no sign
	///
	template<typename T>
	constexpr
	std::enable_if_t<plib::is_integral<T>::value && plib::is_unsigned<T>::value, T>
	abs(T v) noexcept
	{
		return v;
	}

	/// \brief return greatest common denominator
	///
	/// Function returns the greatest common denominator of m and n. For known
	/// arguments, this function also works at compile time.
	///
	/// \tparam M type of the first argument
	/// \tparam N type of the second argument
	/// \param  m first argument
	/// \param  n first argument
	/// \return greatest common denominator of m and n
	///
	template<typename M, typename N>
	constexpr typename std::common_type<M, N>::type
	gcd(M m, N n) noexcept //NOLINT(misc-no-recursion)
	{
		static_assert(plib::is_integral<M>::value, "gcd: M must be an integer");
		static_assert(plib::is_integral<N>::value, "gcd: N must be an integer");

		return m == 0 ? plib::abs(n)
			 : n == 0 ? plib::abs(m)
			 : gcd(n, m % n);
	}

	/// \brief return least common multiple
	///
	/// Function returns the least common multiple of m and n. For known
	/// arguments, this function also works at compile time.
	///
	/// \tparam M type of the first argument
	/// \tparam N type of the second argument
	/// \param  m first argument
	/// \param  n first argument
	/// \return least common multiple of m and n
	///
	template<typename M, typename N>
	constexpr typename std::common_type<M, N>::type
	lcm(M m, N n) noexcept
	{
		static_assert(plib::is_integral<M>::value, "lcm: M must be an integer");
		static_assert(plib::is_integral<N>::value, "lcm: N must be an integer");

		return (m != 0 && n != 0) ? (plib::abs(m) / gcd(m, n)) * plib::abs(n) : 0;
	}

	template<class T>
	constexpr const T& clamp( const T& v, const T& low, const T& high)
	{
		gsl_Expects(high >= low);
		return (v < low) ? low : (high < v) ? high : v;
	}

	static_assert(noexcept(constants<double>::one()), "Not evaluated as constexpr");

} // namespace plib

#endif // PMATH_H_
