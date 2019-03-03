// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * vector_ops.h
 *
 * Base vector operations
 *
 */

#ifndef PLIB_VECTOR_OPS_H_
#define PLIB_VECTOR_OPS_H_

#include <algorithm>
#include <cmath>
#include <type_traits>

#include "pconfig.h"

#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#if !(__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ > 3))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif

namespace plib
{
	template<typename VT, typename T>
	void vec_set_scalar (const std::size_t n, VT &v, T && scalar)
	{
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			v[i] = s;
	}

	template<typename VT, typename VS>
	void vec_set (const std::size_t n, VT &v, const VS & source)
	{
		for ( std::size_t i = 0; i < n; i++ )
			v[i] = source[i];
	}

	template<typename T, typename V1, typename V2>
	T vec_mult (const std::size_t n, const V1 & v1, const V2 & v2 )
	{
		using b8 = T[8];
		PALIGNAS_VECTOROPT() b8 value = {0};
		for (std::size_t i = 0; i < n ; i++ )
		{
			value[i & 7] += v1[i] * v2[i];
		}
		return value[0] + value[1] + value[2] + value[3] + value[4] + value[5] + value[6] + value[7];
	}

	template<typename T, typename VT>
	T vec_mult2 (const std::size_t n, const VT &v)
	{
		using b8 = T[8];
		PALIGNAS_VECTOROPT() b8 value = {0};
		for (std::size_t i = 0; i < n ; i++ )
		{
			value[i & 7] += v[i] * v[i];
		}
		return value[0] + value[1] + value[2] + value[3] + value[4] + value[5] + value[6] + value[7];
	}

	template<typename T, typename VT>
	T vec_sum (const std::size_t n, const VT &v)
	{
		if (n<8)
		{
			T value(0);
			for (std::size_t i = 0; i < n ; i++ )
				value += v[i];

			return value;
		}
		else
		{
			using b8 = T[8];
			PALIGNAS_VECTOROPT() b8 value = {0};
			for (std::size_t i = 0; i < n ; i++ )
				value[i & 7] += v[i];

			return ((value[0] + value[1]) + (value[2] + value[3])) + ((value[4] + value[5]) + (value[6] + value[7]));
		}
	}

	template<typename VV, typename T, typename VR>
	void vec_mult_scalar (const std::size_t n, const VV & v, T && scalar, VR & result)
	{
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			result[i] = s * v[i];
	}

	template<typename VV, typename T, typename VR>
	void vec_add_mult_scalar (const std::size_t n, const VV & v, T && scalar, VR & result)
	{
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += s * v[i];
	}

	template<typename T>
	void vec_add_mult_scalar_p(const std::size_t & n, const T * v, T scalar, T * result)
	{
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += scalar * v[i];
	}

	template<typename V, typename R>
	void vec_add_ip(const std::size_t n, const V & v, R & result)
	{
		for ( std::size_t i = 0; i < n; i++ )
			result[i] += v[i];
	}

	template<typename V1, typename V2, typename VR>
	void vec_sub(const std::size_t n, const V1 &v1, const V2 & v2, VR & result)
	{
		for ( std::size_t i = 0; i < n; i++ )
			result[i] = v1[i] - v2[i];
	}

	template<typename V, typename T>
	void vec_scale(const std::size_t n, V & v, T &&scalar)
	{
		const typename std::remove_reference<decltype(v[0])>::type s(std::forward<T>(scalar));
		for ( std::size_t i = 0; i < n; i++ )
			v[i] *= s;
	}

	template<typename T, typename V>
	T vec_maxabs(const std::size_t n, const V & v)
	{
		T ret = 0.0;
		for ( std::size_t i = 0; i < n; i++ )
			ret = std::max(ret, std::abs(v[i]));

		return ret;
	}
} // namespace plib

#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#if !(__GNUC__ > 7 || (__GNUC__ == 7 && __GNUC_MINOR__ > 3))
#pragma GCC diagnostic pop
#endif
#endif

#endif /* PLIB_VECTOR_OPS_H_ */
