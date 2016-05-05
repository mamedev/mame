#pragma once

#include "common.h"

GLM_FUNC_QUALIFIER __m128 glm_dot_ps(__m128 v1, __m128 v2)
{
#	if GLM_ARCH & GLM_ARCH_AVX
		return _mm_dp_ps(v1, v2, 0xff);
#	else
		__m128 const mul0 = _mm_mul_ps(v1, v2);
		__m128 const swp0 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(2, 3, 0, 1));
		__m128 const add0 = _mm_add_ps(mul0, swp0);
		__m128 const swp1 = _mm_shuffle_ps(add0, add0, _MM_SHUFFLE(0, 1, 2, 3));
		__m128 const add1 = _mm_add_ps(add0, swp1);
		return add1;
#	endif
}

GLM_FUNC_QUALIFIER __m128 glm_dot_ss(__m128 v1, __m128 v2)
{
	__m128 const mul0 = _mm_mul_ps(v1, v2);
	__m128 const mov0 = _mm_movehl_ps(mul0, mul0);
	__m128 const add0 = _mm_add_ps(mov0, mul0);
	__m128 const swp1 = _mm_shuffle_ps(add0, add0, 1);
	__m128 const add1 = _mm_add_ss(add0, swp1);
	return add1;
}

GLM_FUNC_QUALIFIER __m128 glm_len_ps(__m128 x)
{
	__m128 const dot0 = glm_dot_ps(x, x);
	__m128 const sqt0 = _mm_sqrt_ps(dot0);
	return sqt0;
}

GLM_FUNC_QUALIFIER __m128 glm_dst_ps(__m128 p0, __m128 p1)
{
	__m128 sub0 = _mm_sub_ps(p0, p1);
	__m128 len0 = glm_len_ps(sub0);
	return len0;
}

GLM_FUNC_QUALIFIER __m128 glm_xpd_ps(__m128 v1, __m128 v2)
{
	__m128 swp0 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 swp1 = _mm_shuffle_ps(v1, v1, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 swp2 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 swp3 = _mm_shuffle_ps(v2, v2, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 mul0 = _mm_mul_ps(swp0, swp3);
	__m128 mul1 = _mm_mul_ps(swp1, swp2);
	__m128 sub0 = _mm_sub_ps(mul0, mul1);
	return sub0;
}

GLM_FUNC_QUALIFIER __m128 glm_nrm_ps(__m128 v)
{
	__m128 dot0 = glm_dot_ps(v, v);
	__m128 isr0 = _mm_rsqrt_ps(dot0);
	__m128 mul0 = _mm_mul_ps(v, isr0);
	return mul0;
}

GLM_FUNC_QUALIFIER __m128 glm_ffd_ps(__m128 N, __m128 I, __m128 Nref)
{
	__m128 dot0 = glm_dot_ps(Nref, I);
	__m128 sgn0 = glm_sgn_ps(dot0);
	__m128 mul0 = _mm_mul_ps(sgn0, glm_minus_one);
	__m128 mul1 = _mm_mul_ps(N, mul0);
	return mul1;
}

GLM_FUNC_QUALIFIER __m128 glm_rfe_ps(__m128 I, __m128 N)
{
	__m128 dot0 = glm_dot_ps(N, I);
	__m128 mul0 = _mm_mul_ps(N, dot0);
	__m128 mul1 = _mm_mul_ps(mul0, glm_two);
	__m128 sub0 = _mm_sub_ps(I, mul1);
	return sub0;
}

GLM_FUNC_QUALIFIER __m128 glm_rfa_ps(__m128 I, __m128 N, __m128 eta)
{
	__m128 dot0 = glm_dot_ps(N, I);
	__m128 mul0 = _mm_mul_ps(eta, eta);
	__m128 mul1 = _mm_mul_ps(dot0, dot0);
	__m128 sub0 = _mm_sub_ps(glm_one, mul0);
	__m128 sub1 = _mm_sub_ps(glm_one, mul1);
	__m128 mul2 = _mm_mul_ps(sub0, sub1);
	
	if(_mm_movemask_ps(_mm_cmplt_ss(mul2, glm_zero)) == 0)
		return glm_zero;

	__m128 sqt0 = _mm_sqrt_ps(mul2);
	__m128 mul3 = _mm_mul_ps(eta, dot0);
	__m128 add0 = _mm_add_ps(mul3, sqt0);
	__m128 mul4 = _mm_mul_ps(add0, N);
	__m128 mul5 = _mm_mul_ps(eta, I);
	__m128 sub2 = _mm_sub_ps(mul5, mul4);

	return sub2;
}
