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
#include "../plib/pconfig.h"

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

inline void vec_set (const unsigned n, const double &scalar, double * RESTRICT result)
{
	for ( unsigned i = 0; i < n; i++ )
		result[i] = scalar;
}

inline double vecmult (const unsigned n, const double * RESTRICT a1, const double * RESTRICT a2 )
{
	double value = 0.0;

	for ( unsigned i = 0; i < n; i++ )
		value = value + a1[i] * a2[i];
	return value;
}


inline double vecmult2 (const unsigned n, const double *a1)
{
	double value = 0.0;

	for ( unsigned i = 0; i < n; i++ )
	{
		const double temp = a1[i];
		value = value + temp * temp;
	}
	return value;
}

inline void vec_mult_scalar (const int n, const double * RESTRICT v, const double scalar, double * RESTRICT result)
{
	for ( unsigned i = 0; i < n; i++ )
	{
		result[i] = scalar * v[i];
	}
}

inline void vec_add_mult_scalar (const int n, const double * RESTRICT v, const double scalar, double * RESTRICT result)
{
	for ( unsigned i = 0; i < n; i++ )
		result[i] += scalar * v[i];
}

inline void vec_add_ip(const int n, const double * RESTRICT v, double * RESTRICT result)
{
	for ( unsigned i = 0; i < n; i++ )
		result[i] += v[i];
}

inline void vec_sub(const int n, const double * RESTRICT v1, const double * RESTRICT v2, double * RESTRICT result)
{
	for ( unsigned i = 0; i < n; i++ )
		result[i] = v1[i] - v2[i];
}

inline void vec_scale (const int n, double * RESTRICT v, const double scalar)
{
	for ( unsigned i = 0; i < n; i++ )
		v[i] = scalar * v[i];
}

inline double vec_maxabs(const int n, const double * RESTRICT v)
{
	double ret = 0.0;
	for ( unsigned i = 0; i < n; i++ )
		ret = std::max(ret, std::abs(v[i]));

	return ret;
}



#endif /* MAT_CR_H_ */
