namespace glm{
namespace detail
{
/*
	static const __m128 GLM_VAR_USED zero = _mm_setzero_ps();
	static const __m128 GLM_VAR_USED one = _mm_set_ps1(1.0f);
	static const __m128 GLM_VAR_USED minus_one = _mm_set_ps1(-1.0f);
	static const __m128 GLM_VAR_USED two = _mm_set_ps1(2.0f);
	static const __m128 GLM_VAR_USED three = _mm_set_ps1(3.0f);
	static const __m128 GLM_VAR_USED pi = _mm_set_ps1(3.1415926535897932384626433832795f);
	static const __m128 GLM_VAR_USED hundred_eighty = _mm_set_ps1(180.f);
	static const __m128 GLM_VAR_USED pi_over_hundred_eighty = _mm_set_ps1(0.017453292519943295769236907684886f);
	static const __m128 GLM_VAR_USED hundred_eighty_over_pi = _mm_set_ps1(57.295779513082320876798154814105f);

	static const ieee754_QNAN absMask;
	static const __m128 GLM_VAR_USED abs4Mask = _mm_set_ps1(absMask.f);

	static const __m128 GLM_VAR_USED _epi32_sign_mask = _mm_castsi128_ps(_mm_set1_epi32(static_cast<int>(0x80000000)));
	//static const __m128 GLM_VAR_USED _epi32_inv_sign_mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
	//static const __m128 GLM_VAR_USED _epi32_mant_mask = _mm_castsi128_ps(_mm_set1_epi32(0x7F800000));
	//static const __m128 GLM_VAR_USED _epi32_inv_mant_mask = _mm_castsi128_ps(_mm_set1_epi32(0x807FFFFF));
	//static const __m128 GLM_VAR_USED _epi32_min_norm_pos = _mm_castsi128_ps(_mm_set1_epi32(0x00800000));
	static const __m128 GLM_VAR_USED _epi32_0 = _mm_set_ps1(0);
	static const __m128 GLM_VAR_USED _epi32_1 = _mm_set_ps1(1);
	static const __m128 GLM_VAR_USED _epi32_2 = _mm_set_ps1(2);
	static const __m128 GLM_VAR_USED _epi32_3 = _mm_set_ps1(3);
	static const __m128 GLM_VAR_USED _epi32_4 = _mm_set_ps1(4);
	static const __m128 GLM_VAR_USED _epi32_5 = _mm_set_ps1(5);
	static const __m128 GLM_VAR_USED _epi32_6 = _mm_set_ps1(6);
	static const __m128 GLM_VAR_USED _epi32_7 = _mm_set_ps1(7);
	static const __m128 GLM_VAR_USED _epi32_8 = _mm_set_ps1(8);
	static const __m128 GLM_VAR_USED _epi32_9 = _mm_set_ps1(9);
	static const __m128 GLM_VAR_USED _epi32_127 = _mm_set_ps1(127);
	//static const __m128 GLM_VAR_USED _epi32_ninf = _mm_castsi128_ps(_mm_set1_epi32(0xFF800000));
	//static const __m128 GLM_VAR_USED _epi32_pinf = _mm_castsi128_ps(_mm_set1_epi32(0x7F800000));

	static const __m128 GLM_VAR_USED _ps_1_3 = _mm_set_ps1(0.33333333333333333333333333333333f);
	static const __m128 GLM_VAR_USED _ps_0p5 = _mm_set_ps1(0.5f);
	static const __m128 GLM_VAR_USED _ps_1 = _mm_set_ps1(1.0f);
	static const __m128 GLM_VAR_USED _ps_m1 = _mm_set_ps1(-1.0f);
	static const __m128 GLM_VAR_USED _ps_2 = _mm_set_ps1(2.0f);
	static const __m128 GLM_VAR_USED _ps_3 = _mm_set_ps1(3.0f);
	static const __m128 GLM_VAR_USED _ps_127 = _mm_set_ps1(127.0f);
	static const __m128 GLM_VAR_USED _ps_255 = _mm_set_ps1(255.0f);
	static const __m128 GLM_VAR_USED _ps_2pow23 = _mm_set_ps1(8388608.0f);

	static const __m128 GLM_VAR_USED _ps_1_0_0_0 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	static const __m128 GLM_VAR_USED _ps_0_1_0_0 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	static const __m128 GLM_VAR_USED _ps_0_0_1_0 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	static const __m128 GLM_VAR_USED _ps_0_0_0_1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

	static const __m128 GLM_VAR_USED _ps_pi = _mm_set_ps1(3.1415926535897932384626433832795f);
	static const __m128 GLM_VAR_USED _ps_pi2 = _mm_set_ps1(6.283185307179586476925286766560f);
	static const __m128 GLM_VAR_USED _ps_2_pi = _mm_set_ps1(0.63661977236758134307553505349006f);
	static const __m128 GLM_VAR_USED _ps_pi_2 = _mm_set_ps1(1.5707963267948966192313216916398f);
	static const __m128 GLM_VAR_USED _ps_4_pi = _mm_set_ps1(1.2732395447351626861510701069801f);
	static const __m128 GLM_VAR_USED _ps_pi_4 = _mm_set_ps1(0.78539816339744830961566084581988f);

	static const __m128 GLM_VAR_USED _ps_sincos_p0 = _mm_set_ps1(0.15707963267948963959e1f);
	static const __m128 GLM_VAR_USED _ps_sincos_p1 = _mm_set_ps1(-0.64596409750621907082e0f);
	static const __m128 GLM_VAR_USED _ps_sincos_p2 = _mm_set_ps1(0.7969262624561800806e-1f);
	static const __m128 GLM_VAR_USED _ps_sincos_p3 = _mm_set_ps1(-0.468175413106023168e-2f);
	static const __m128 GLM_VAR_USED _ps_tan_p0 = _mm_set_ps1(-1.79565251976484877988e7f);
	static const __m128 GLM_VAR_USED _ps_tan_p1 = _mm_set_ps1(1.15351664838587416140e6f);
	static const __m128 GLM_VAR_USED _ps_tan_p2 = _mm_set_ps1(-1.30936939181383777646e4f);
	static const __m128 GLM_VAR_USED _ps_tan_q0 = _mm_set_ps1(-5.38695755929454629881e7f);
	static const __m128 GLM_VAR_USED _ps_tan_q1 = _mm_set_ps1(2.50083801823357915839e7f);
	static const __m128 GLM_VAR_USED _ps_tan_q2 = _mm_set_ps1(-1.32089234440210967447e6f);
	static const __m128 GLM_VAR_USED _ps_tan_q3 = _mm_set_ps1(1.36812963470692954678e4f);
	static const __m128 GLM_VAR_USED _ps_tan_poleval = _mm_set_ps1(3.68935e19f);
	static const __m128 GLM_VAR_USED _ps_atan_t0 = _mm_set_ps1(-0.91646118527267623468e-1f);
	static const __m128 GLM_VAR_USED _ps_atan_t1 = _mm_set_ps1(-0.13956945682312098640e1f);
	static const __m128 GLM_VAR_USED _ps_atan_t2 = _mm_set_ps1(-0.94393926122725531747e2f);
	static const __m128 GLM_VAR_USED _ps_atan_t3 = _mm_set_ps1(0.12888383034157279340e2f);
	static const __m128 GLM_VAR_USED _ps_atan_s0 = _mm_set_ps1(0.12797564625607904396e1f);
	static const __m128 GLM_VAR_USED _ps_atan_s1 = _mm_set_ps1(0.21972168858277355914e1f);
	static const __m128 GLM_VAR_USED _ps_atan_s2 = _mm_set_ps1(0.68193064729268275701e1f);
	static const __m128 GLM_VAR_USED _ps_atan_s3 = _mm_set_ps1(0.28205206687035841409e2f);

	static const __m128 GLM_VAR_USED _ps_exp_hi = _mm_set_ps1(88.3762626647949f);
	static const __m128 GLM_VAR_USED _ps_exp_lo = _mm_set_ps1(-88.3762626647949f);
	static const __m128 GLM_VAR_USED _ps_exp_rln2 = _mm_set_ps1(1.4426950408889634073599f);
	static const __m128 GLM_VAR_USED _ps_exp_p0 = _mm_set_ps1(1.26177193074810590878e-4f);
	static const __m128 GLM_VAR_USED _ps_exp_p1 = _mm_set_ps1(3.02994407707441961300e-2f);
	static const __m128 GLM_VAR_USED _ps_exp_q0 = _mm_set_ps1(3.00198505138664455042e-6f);
	static const __m128 GLM_VAR_USED _ps_exp_q1 = _mm_set_ps1(2.52448340349684104192e-3f);
	static const __m128 GLM_VAR_USED _ps_exp_q2 = _mm_set_ps1(2.27265548208155028766e-1f);
	static const __m128 GLM_VAR_USED _ps_exp_q3 = _mm_set_ps1(2.00000000000000000009e0f);
	static const __m128 GLM_VAR_USED _ps_exp_c1 = _mm_set_ps1(6.93145751953125e-1f);
	static const __m128 GLM_VAR_USED _ps_exp_c2 = _mm_set_ps1(1.42860682030941723212e-6f);
	static const __m128 GLM_VAR_USED _ps_exp2_hi = _mm_set_ps1(127.4999961853f);
	static const __m128 GLM_VAR_USED _ps_exp2_lo = _mm_set_ps1(-127.4999961853f);
	static const __m128 GLM_VAR_USED _ps_exp2_p0 = _mm_set_ps1(2.30933477057345225087e-2f);
	static const __m128 GLM_VAR_USED _ps_exp2_p1 = _mm_set_ps1(2.02020656693165307700e1f);
	static const __m128 GLM_VAR_USED _ps_exp2_p2 = _mm_set_ps1(1.51390680115615096133e3f);
	static const __m128 GLM_VAR_USED _ps_exp2_q0 = _mm_set_ps1(2.33184211722314911771e2f);
	static const __m128 GLM_VAR_USED _ps_exp2_q1 = _mm_set_ps1(4.36821166879210612817e3f);
	static const __m128 GLM_VAR_USED _ps_log_p0 = _mm_set_ps1(-7.89580278884799154124e-1f);
	static const __m128 GLM_VAR_USED _ps_log_p1 = _mm_set_ps1(1.63866645699558079767e1f);
	static const __m128 GLM_VAR_USED _ps_log_p2 = _mm_set_ps1(-6.41409952958715622951e1f);
	static const __m128 GLM_VAR_USED _ps_log_q0 = _mm_set_ps1(-3.56722798256324312549e1f);
	static const __m128 GLM_VAR_USED _ps_log_q1 = _mm_set_ps1(3.12093766372244180303e2f);
	static const __m128 GLM_VAR_USED _ps_log_q2 = _mm_set_ps1(-7.69691943550460008604e2f);
	static const __m128 GLM_VAR_USED _ps_log_c0 = _mm_set_ps1(0.693147180559945f);
	static const __m128 GLM_VAR_USED _ps_log2_c0 = _mm_set_ps1(1.44269504088896340735992f);
*/

}//namespace detail
}//namespace glm
