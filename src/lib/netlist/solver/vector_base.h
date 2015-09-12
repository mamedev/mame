// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * vector_base.h
 *
 * Base vector operations
 *
 */

#ifndef VECTOR_BASE_H_
#define VECTOR_BASE_H_

#include <algorithm>
#include "plib/pconfig.h"

#if 0
template <unsigned _storage_N>
struct pvector
{
	pvector(unsigned size)
	: m_N(size) { }

	unsigned size() {
		if (_storage_N)
	}

	double m_V[_storage_N];
private:
	unsigned m_N;
};
#endif

#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

template<typename T>
inline void vec_set (const std::size_t n, const T &scalar, T * RESTRICT result)
{
	for ( std::size_t i = 0; i < n; i++ )
		result[i] = scalar;
}

template<typename T>
inline T vecmult (const std::size_t n, const T * RESTRICT a1, const T * RESTRICT a2 )
{
	T value = 0.0;
	for ( std::size_t i = 0; i < n; i++ )
		value = value + a1[i] * a2[i];
	return value;
}

template<typename T>
inline T vecmult2 (const std::size_t n, const T *a1)
{
	T value = 0.0;
	for ( std::size_t i = 0; i < n; i++ )
	{
		const T temp = a1[i];
		value = value + temp * temp;
	}
	return value;
}

template<typename T>
inline void vec_mult_scalar (const std::size_t n, const T * RESTRICT v, const T scalar, T * RESTRICT result)
{
	for ( std::size_t i = 0; i < n; i++ )
	{
		result[i] = scalar * v[i];
	}
}

template<typename T>
inline void vec_add_mult_scalar (const std::size_t n, const T * RESTRICT v, const T scalar, T * RESTRICT result)
{
	for ( std::size_t i = 0; i < n; i++ )
		result[i] += scalar * v[i];
}

inline void vec_add_ip(const std::size_t n, const double * RESTRICT v, double * RESTRICT result)
{
	for ( std::size_t i = 0; i < n; i++ )
		result[i] += v[i];
}

template<typename T>
inline void vec_sub(const std::size_t n, const T * RESTRICT v1, const T * RESTRICT v2, T * RESTRICT result)
{
	for ( std::size_t i = 0; i < n; i++ )
		result[i] = v1[i] - v2[i];
}

template<typename T>
inline void vec_scale (const std::size_t n, T * RESTRICT v, const T scalar)
{
	for ( std::size_t i = 0; i < n; i++ )
		v[i] = scalar * v[i];
}

inline double vec_maxabs(const std::size_t n, const double * RESTRICT v)
{
	double ret = 0.0;
	for ( std::size_t i = 0; i < n; i++ )
		ret = std::max(ret, std::abs(v[i]));

	return ret;
}
#if !defined(__clang__) && !defined(_MSC_VER) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6))
#pragma GCC diagnostic pop
#endif

#endif /* MAT_CR_H_ */
