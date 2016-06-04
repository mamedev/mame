#if(GLM_COMPILER & GLM_COMPILER_VC)
#pragma warning(push)
#pragma warning(disable : 4510 4512 4610)
#endif

	union ieee754_QNAN
	{
		const float f;
		struct i
		{
			const unsigned int mantissa:23, exp:8, sign:1;
		};

		ieee754_QNAN() : f(0.0)/*, mantissa(0x7FFFFF), exp(0xFF), sign(0x0)*/ {}
	};

#if(GLM_COMPILER & GLM_COMPILER_VC)
#pragma warning(pop)
#endif

static const __m128 GLM_VAR_USED glm_zero = _mm_setzero_ps();
static const __m128 GLM_VAR_USED glm_one = _mm_set_ps1(1.0f);
static const __m128 GLM_VAR_USED glm_half = _mm_set_ps1(0.5f);
static const __m128 GLM_VAR_USED glm_minus_one = _mm_set_ps1(-1.0f);
static const __m128 GLM_VAR_USED glm_two = _mm_set_ps1(2.0f);
static const __m128 GLM_VAR_USED glm_three = _mm_set_ps1(3.0f);

static const ieee754_QNAN glm_abs_mask;
static const __m128 GLM_VAR_USED glm_abs4_mask = _mm_set_ps1(glm_abs_mask.f);
static const __m128 GLM_VAR_USED glm_epi32_sign_mask = _mm_castsi128_ps(_mm_set1_epi32(static_cast<int>(0x80000000)));
static const __m128 GLM_VAR_USED glm_ps_2pow23 = _mm_set_ps1(8388608.0f);
static const __m128 GLM_VAR_USED glm_ps_1 = _mm_set_ps1(1.0f);

GLM_FUNC_QUALIFIER __m128 glm_abs_ps(__m128 x)
{
	return _mm_and_ps(glm_abs4_mask, x);
}

//sign
GLM_FUNC_QUALIFIER __m128 glm_sgn_ps(__m128 x)
{
	__m128 const Cmp0 = _mm_cmplt_ps(x, glm_zero);
	__m128 const Cmp1 = _mm_cmpgt_ps(x, glm_zero);
	__m128 const And0 = _mm_and_ps(Cmp0, glm_minus_one);
	__m128 const And1 = _mm_and_ps(Cmp1, glm_one);
	return _mm_or_ps(And0, And1);
}

//round
GLM_FUNC_QUALIFIER __m128 glm_rnd_ps(__m128 x)
{
	__m128 const and0 = _mm_and_ps(glm_epi32_sign_mask, x);
	__m128 const or0 = _mm_or_ps(and0, glm_ps_2pow23);
	__m128 const add0 = _mm_add_ps(x, or0);
	__m128 const sub0 = _mm_sub_ps(add0, or0);
	return sub0;
}

//floor
GLM_FUNC_QUALIFIER __m128 glm_flr_ps(__m128 x)
{
	__m128 const rnd0 = glm_rnd_ps(x);
	__m128 const cmp0 = _mm_cmplt_ps(x, rnd0);
	__m128 const and0 = _mm_and_ps(cmp0, glm_ps_1);
	__m128 const sub0 = _mm_sub_ps(rnd0, and0);
	return sub0;
}

//trunc
//GLM_FUNC_QUALIFIER __m128 _mm_trc_ps(__m128 v)
//{
//	return __m128();
//}

//roundEven
GLM_FUNC_QUALIFIER __m128 glm_rde_ps(__m128 x)
{
	__m128 const and0 = _mm_and_ps(glm_epi32_sign_mask, x);
	__m128 const or0 = _mm_or_ps(and0, glm_ps_2pow23);
	__m128 const add0 = _mm_add_ps(x, or0);
	__m128 const sub0 = _mm_sub_ps(add0, or0);
	return sub0;
}

GLM_FUNC_QUALIFIER __m128 glm_ceil_ps(__m128 x)
{
	__m128 const rnd0 = glm_rnd_ps(x);
	__m128 const cmp0 = _mm_cmpgt_ps(x, rnd0);
	__m128 const and0 = _mm_and_ps(cmp0, glm_ps_1);
	__m128 const add0 = _mm_add_ps(rnd0, and0);
	return add0;
}

GLM_FUNC_QUALIFIER __m128 glm_frc_ps(__m128 x)
{
	__m128 const flr0 = glm_flr_ps(x);
	__m128 const sub0 = _mm_sub_ps(x, flr0);
	return sub0;
}

GLM_FUNC_QUALIFIER __m128 glm_mod_ps(__m128 x, __m128 y)
{
	__m128 const div0 = _mm_div_ps(x, y);
	__m128 const flr0 = glm_flr_ps(div0);
	__m128 const mul0 = _mm_mul_ps(y, flr0);
	__m128 const sub0 = _mm_sub_ps(x, mul0);
	return sub0;
}

GLM_FUNC_QUALIFIER __m128 glm_clp_ps(__m128 v, __m128 minVal, __m128 maxVal)
{
	__m128 const min0 = _mm_min_ps(v, maxVal);
	__m128 const max0 = _mm_max_ps(min0, minVal);
	return max0;
}

GLM_FUNC_QUALIFIER __m128 glm_mix_ps(__m128 v1, __m128 v2, __m128 a)
{
	__m128 const sub0 = _mm_sub_ps(glm_one, a);
	__m128 const mul0 = _mm_mul_ps(v1, sub0);
	__m128 const mul1 = _mm_mul_ps(v2, a);
	__m128 const add0 = _mm_add_ps(mul0, mul1);
	return add0;
}

//step
GLM_FUNC_QUALIFIER __m128 glm_stp_ps(__m128 edge, __m128 x)
{
	__m128 const cmp = _mm_cmple_ps(x, edge);
	return _mm_movemask_ps(cmp) == 0 ? glm_one : glm_zero;
}

// smoothstep
GLM_FUNC_QUALIFIER __m128 glm_ssp_ps(__m128 edge0, __m128 edge1, __m128 x)
{
	__m128 const sub0 = _mm_sub_ps(x, edge0);
	__m128 const sub1 = _mm_sub_ps(edge1, edge0);
	__m128 const div0 = _mm_sub_ps(sub0, sub1);
	__m128 const clp0 = glm_clp_ps(div0, glm_zero, glm_one);
	__m128 const mul0 = _mm_mul_ps(glm_two, clp0);
	__m128 const sub2 = _mm_sub_ps(glm_three, mul0);
	__m128 const mul1 = _mm_mul_ps(clp0, clp0);
	__m128 const mul2 = _mm_mul_ps(mul1, sub2);
	return mul2;
}

/// \todo
//GLM_FUNC_QUALIFIER __m128 glm_nan_ps(__m128 x)
//{
//	__m128 empty;
//	return empty;
//}

/// \todo
//GLM_FUNC_QUALIFIER __m128 glm_inf_ps(__m128 x)
//{
//	__m128 empty;
//	return empty;
//}

// SSE scalar reciprocal sqrt using rsqrt op, plus one Newton-Rhaphson iteration
// By Elan Ruskin, http://assemblyrequired.crashworks.org/
GLM_FUNC_QUALIFIER __m128 glm_sqrt_wip_ss(__m128 x)
{
	__m128 const recip = _mm_rsqrt_ss(x);  // "estimate" opcode
	__m128 const halfrecip = _mm_mul_ss(glm_half, recip);
	__m128 const threeminus_xrr = _mm_sub_ss(glm_three, _mm_mul_ss(x, _mm_mul_ss(recip, recip)));
	return _mm_mul_ss(halfrecip, threeminus_xrr);
}
