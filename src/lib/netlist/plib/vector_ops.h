// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef PLIB_VECTOR_OPS_H_
#define PLIB_VECTOR_OPS_H_

///
/// \file vector_ops.h
///
/// Base vector operations
///
///
#include "pconfig.h"
#include "pgsl.h"
#include "pmath.h"

#include <algorithm>
#include <array>
#include <type_traits>

#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#if !(__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ > 3))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif

namespace plib
{
	template<typename VT, typename T>
	void vec_set_scalar(VT &v, T && scalar) noexcept
	{
		const std::size_t n(v.size());
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			v[i] = s;
	}

	template<typename VT, typename VS>
	void vec_set(VT &v, const VS & source) noexcept
	{
		const std::size_t n(v.size());
		gsl_Expects(n <= source.size());
		for ( std::size_t i = 0; i < n; i++ )
			v[i] = source[i];
	}

	template<typename T, typename V1, typename V2>
	T vec_mult(const V1 & v1, const V2 & v2 ) noexcept
	{
		const std::size_t n(v1.size());
		gsl_Expects(n <= v2.size());
		T ret(plib::constants<T>::zero());
		for (std::size_t i = 0; i < n ; i++ )
		{
			ret += v1[i] * v2[i]; // NOLINT
		}
		return ret;
	}

	template<typename T, typename VT>
	T vec_mult2(const VT &v) noexcept
	{
		const std::size_t n(v.size());
		T ret(plib::constants<T>::zero());
		for (std::size_t i = 0; i < n ; i++ )
		{
			ret += v[i] * v[i];
		}
		return ret;
	}

	template<typename T, typename VT>
	T vec_sum(const VT &v) noexcept
	{
		const std::size_t n(v.size());
		T ret(plib::constants<T>::zero());
		for (std::size_t i = 0; i < n ; i++ )
		{
			ret += v[i];
		}

		return ret;
	}

	template<typename VV, typename T, typename VR>
	void vec_mult_scalar(VR & result, const VV & v, T && scalar) noexcept
	{
		const std::size_t n(result.size());
		gsl_Expects(n <= v.size());
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			result[i] = s * v[i];
	}

	template<typename VR, typename VV, typename T>
	void vec_add_mult_scalar(VR & result, const VV & v, T && scalar) noexcept
	{
		const std::size_t n(result.size());
		gsl_Expects(n <= v.size());
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += s * v[i];
	}

	template<typename T>
	void vec_add_mult_scalar_p(const std::size_t n, T * result, const T * v, T scalar) noexcept
	{
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += scalar * v[i];
	}

	template<typename R, typename V>
	void vec_add_ip(R & result, const V & v) noexcept
	{
		const std::size_t n(result.size());
		gsl_Expects(n <= v.size());
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += v[i];
	}

	template<typename VR, typename V1, typename V2>
	void vec_sub(VR & result, const V1 &v1, const V2 & v2) noexcept
	{
		const std::size_t n(result.size());
		gsl_Expects(n <= v1.size());
		gsl_Expects(n <= v2.size());
		for ( std::size_t i = 0; i < n; i++ )
			result[i] = v1[i] - v2[i];
	}

	template<typename V, typename T>
	void vec_scale(V & v, T &&scalar) noexcept
	{
		const std::size_t n(v.size());
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			v[i] *= s;
	}

	template<typename T, typename V>
	T vec_max_abs(const V & v) noexcept
	{
		const std::size_t n(v.size());
		T ret(plib::constants<T>::zero());
		for ( std::size_t i = 0; i < n; i++ )
			ret = std::max(ret, plib::abs(v[i]));

		return ret;
	}
} // namespace plib

#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#if !(__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ > 3))
#pragma GCC diagnostic pop
#endif
#endif

#endif // PLIB_VECTOR_OPS_H_
