/***************************************************************************

    rspcp2s.c

    Universal machine language-based Nintendo/SGI RSP COP2 emulator, with
    SSSE3 SIMD optimizations.
    Written by Harmony of the MESS team.

    Copyright the MESS team.
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "rsp.h"
#include "rspdiv.h"
#include "rspcp2.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

/***************************************************************************
    Helpful Defines
***************************************************************************/

#define VDREG   ((op >> 6) & 0x1f)
#define VS1REG  ((op >> 11) & 0x1f)
#define VS2REG  ((op >> 16) & 0x1f)
#define EL      ((op >> 21) & 0xf)

#define RSVAL   (m_rsp.m_rsp_state->r[RSREG])
#define RTVAL   (m_rsp.m_rsp_state->r[RTREG])
#define RDVAL   (m_rsp.m_rsp_state->r[RDREG])

#define EXTRACT16(reg, value, element) \
	switch((element) & 7) \
	{ \
		case 0: value = _mm_extract_epi16(reg, 0); break; \
		case 1: value = _mm_extract_epi16(reg, 1); break; \
		case 2: value = _mm_extract_epi16(reg, 2); break; \
		case 3: value = _mm_extract_epi16(reg, 3); break; \
		case 4: value = _mm_extract_epi16(reg, 4); break; \
		case 5: value = _mm_extract_epi16(reg, 5); break; \
		case 6: value = _mm_extract_epi16(reg, 6); break; \
		case 7: value = _mm_extract_epi16(reg, 7); break; \
	}


#define INSERT16(reg, value, element) \
	switch((element) & 7) \
	{ \
		case 0: reg = _mm_insert_epi16(reg, value, 0); break; \
		case 1: reg = _mm_insert_epi16(reg, value, 1); break; \
		case 2: reg = _mm_insert_epi16(reg, value, 2); break; \
		case 3: reg = _mm_insert_epi16(reg, value, 3); break; \
		case 4: reg = _mm_insert_epi16(reg, value, 4); break; \
		case 5: reg = _mm_insert_epi16(reg, value, 5); break; \
		case 6: reg = _mm_insert_epi16(reg, value, 6); break; \
		case 7: reg = _mm_insert_epi16(reg, value, 7); break; \
	}


#define VREG_B(reg, offset)         m_v[(reg)].b[(offset)^1]
#define W_VREG_S(reg, offset)       m_v[(reg)].s[(offset)]
#define VREG_S(reg, offset)         (INT16)m_v[(reg)].s[(offset)]

#define VEC_EL_2(x,z)               (vector_elements_2[(x)][(z)])

#define ACCUM(x)        m_accum[x].q

#define CARRY       0
#define COMPARE     1
#define CLIP1       2
#define ZERO        3
#define CLIP2       4

static void cfunc_mfc2(void *param);
static void cfunc_cfc2(void *param);
static void cfunc_mtc2(void *param);
static void cfunc_ctc2(void *param);

inline UINT16 rsp_cop2_simd::ACCUM_H(int x)
{
	UINT16 out;
	EXTRACT16(m_accum_h, out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::ACCUM_M(int x)
{
	UINT16 out;
	EXTRACT16(m_accum_m, out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::ACCUM_L(int x)
{
	UINT16 out;
	EXTRACT16(m_accum_l, out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::ACCUM_LL(int x)
{
	UINT16 out;
	EXTRACT16(m_accum_ll, out, x);
	return out;
}

#define SET_ACCUM_H(v, x) INSERT16(m_accum_h, v, x);
#define SET_ACCUM_M(v, x) INSERT16(m_>accum_m, v, x);
#define SET_ACCUM_L(v, x) INSERT16(m_accum_l, v, x);
#define SET_ACCUM_LL(v, x) INSERT16(m_accum_ll, v, x);

#define GET_VS1(out, i) EXTRACT16(m_xv[VS1REG], out, i);
#define GET_VS2(out, i) EXTRACT16(m_xv[VS2REG], out, VEC_EL_2(EL, i));

inline UINT16 rsp_cop2_simd::CARRY_FLAG(const int x)
{
	UINT16 out;
	EXTRACT16(m_xvflag[CARRY], out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::COMPARE_FLAG(const int x)
{
	UINT16 out;
	EXTRACT16(m_xvflag[COMPARE], out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::CLIP1_FLAG(const int x)
{
	UINT16 out;
	EXTRACT16(m_xvflag[CLIP1], out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::ZERO_FLAG(const int x)
{
	UINT16 out;
	EXTRACT16(m_xvflag[ZERO], out, x);
	return out;
}

inline UINT16 rsp_cop2_simd::CLIP2_FLAG(const int x)
{
	UINT16 out;
	EXTRACT16(m_xvflag[CLIP2], out, x);
	return out;
}

#define CLEAR_CARRY_FLAGS()     { m_xvflag[CARRY] = _mm_setzero_si128(); }
#define CLEAR_COMPARE_FLAGS()   { m_xvflag[COMPARE] = _mm_setzero_si128(); }
#define CLEAR_CLIP1_FLAGS()     { m_xvflag[CLIP1] = _mm_setzero_si128(); }
#define CLEAR_ZERO_FLAGS()      { m_xvflag[ZERO] = _mm_setzero_si128(); }
#define CLEAR_CLIP2_FLAGS()     { m_xvflag[CLIP2] = _mm_setzero_si128(); }

#define SET_CARRY_FLAG(x)       { INSERT16(m_xvflag[CARRY], 0xffff, x); }
#define SET_COMPARE_FLAG(x)     { INSERT16(m_xvflag[COMPARE], 0xffff, x); }
#define SET_CLIP1_FLAG(x)       { INSERT16(m_xvflag[CLIP1], 0xffff, x); }
#define SET_ZERO_FLAG(x)        { INSERT16(m_xvflag[ZERO], 0xffff, x); }
#define SET_CLIP2_FLAG(x)       { INSERT16(m_xvflag[CLIP2], 0xffff, x); }

#define CLEAR_CARRY_FLAG(x)     { INSERT16(m_xvflag[CARRY], 0, x); }
#define CLEAR_COMPARE_FLAG(x)   { INSERT16(m_xvflag[COMPARE], 0, x); }
#define CLEAR_CLIP1_FLAG(x)     { INSERT16(m_xvflag[CLIP1], 0, x); }
#define CLEAR_ZERO_FLAG(x)      { INSERT16(m_xvflag[ZERO], 0, x); }
#define CLEAR_CLIP2_FLAG(x)     { INSERT16(m_xvflag[CLIP2], 0, x); }

#define WRITEBACK_RESULT() { \
		INSERT16(m_xv[VDREG], m_vres[0], 0); \
		INSERT16(m_xv[VDREG], m_vres[1], 1); \
		INSERT16(m_xv[VDREG], m_vres[2], 2); \
		INSERT16(m_xv[VDREG], m_vres[3], 3); \
		INSERT16(m_xv[VDREG], m_vres[4], 4); \
		INSERT16(m_xv[VDREG], m_vres[5], 5); \
		INSERT16(m_xv[VDREG], m_vres[6], 6); \
		INSERT16(m_xv[VDREG], m_vres[7], 7); \
}
#endif

static const int vector_elements_2[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // none
	{ 0, 1, 2, 3, 4, 5, 6, 7 },     // ???
	{ 0, 0, 2, 2, 4, 4, 6, 6 },     // 0q
	{ 1, 1, 3, 3, 5, 5, 7, 7 },     // 1q
	{ 0, 0, 0, 0, 4, 4, 4, 4 },     // 0h
	{ 1, 1, 1, 1, 5, 5, 5, 5 },     // 1h
	{ 2, 2, 2, 2, 6, 6, 6, 6 },     // 2h
	{ 3, 3, 3, 3, 7, 7, 7, 7 },     // 3h
	{ 0, 0, 0, 0, 0, 0, 0, 0 },     // 0
	{ 1, 1, 1, 1, 1, 1, 1, 1 },     // 1
	{ 2, 2, 2, 2, 2, 2, 2, 2 },     // 2
	{ 3, 3, 3, 3, 3, 3, 3, 3 },     // 3
	{ 4, 4, 4, 4, 4, 4, 4, 4 },     // 4
	{ 5, 5, 5, 5, 5, 5, 5, 5 },     // 5
	{ 6, 6, 6, 6, 6, 6, 6, 6 },     // 6
	{ 7, 7, 7, 7, 7, 7, 7, 7 },     // 7
};

static __m128i vec_himask;
static __m128i vec_lomask;
static __m128i vec_hibit;
static __m128i vec_lobit;
static __m128i vec_n32768;
static __m128i vec_32767;
static __m128i vec_flagmask;
static __m128i vec_shiftmask2;
static __m128i vec_shiftmask4;
static __m128i vec_flag_reverse;
static __m128i vec_neg1;
static __m128i vec_zero;
static __m128i vec_shuf[16];
static __m128i vec_shuf_inverse[16];

rsp_cop2_simd::rsp_cop2_simd(rsp_device &rsp, running_machine &machine) : rsp_cop2(rsp, machine)
	: m_accum_h(0)
	, m_accum_m(0)
	, m_accum_l(0)
	, m_accum_ll(0)
#if SIMUL_SIMD
	, m_old_reciprocal_res(0)
	, m_old_reciprocal_high(0)
	, m_old_dp_allowed(0)
	, m_scalar_reciprocal_res(0)
	, m_scalar_reciprocal_high(0)
	, m_scalar_dp_allowed(0)
	, m_simd_reciprocal_res(0)
	, m_simd_reciprocal_high(0)
	, m_simd_dp_allowed(0)
#endif
{
#if SIMUL_SIMD
	memset(m_old_r, 0, sizeof(m_old_r));
	memset(m_old_dmem, 0, sizeof(m_old_dmem));
	memset(m_scalar_r, 0, sizeof(m_scalar_r));
	memset(m_scalar_dmem, 0, sizeof(m_scalar_dmem));
#endif
	memset(m_xv, 0, sizeof(m_xv));
	memset(m_xvflag, 0, sizeof(m_xvflag));

	vec_shuf_inverse[ 0] = _mm_set_epi16(0x0f0e, 0x0d0c, 0x0b0a, 0x0908, 0x0706, 0x0504, 0x0302, 0x0100); // none
	vec_shuf_inverse[ 1] = _mm_set_epi16(0x0f0e, 0x0d0c, 0x0b0a, 0x0908, 0x0706, 0x0504, 0x0302, 0x0100); // ???
	vec_shuf_inverse[ 2] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0908, 0x0908, 0x0504, 0x0504, 0x0100, 0x0100); // 0q
	vec_shuf_inverse[ 3] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0b0a, 0x0b0a, 0x0706, 0x0706, 0x0302, 0x0302); // 1q
	vec_shuf_inverse[ 4] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0100, 0x0100, 0x0100, 0x0100); // 0h
	vec_shuf_inverse[ 5] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0302, 0x0302, 0x0302, 0x0302); // 1h
	vec_shuf_inverse[ 6] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0504, 0x0504, 0x0504, 0x0504); // 2h
	vec_shuf_inverse[ 7] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0706, 0x0706, 0x0706, 0x0706); // 3h
	vec_shuf_inverse[ 8] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100); // 0
	vec_shuf_inverse[ 9] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302); // 1
	vec_shuf_inverse[10] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504); // 2
	vec_shuf_inverse[11] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706); // 3
	vec_shuf_inverse[12] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908); // 4
	vec_shuf_inverse[13] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 5
	vec_shuf_inverse[14] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 6
	vec_shuf_inverse[15] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 7

	vec_shuf[ 0] = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e); // none
	vec_shuf[ 1] = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e); // ???
	vec_shuf[ 2] = _mm_set_epi16(0x0302, 0x0302, 0x0706, 0x0706, 0x0b0a, 0x0b0a, 0x0f0e, 0x0f0e); // 0q
	vec_shuf[ 3] = _mm_set_epi16(0x0100, 0x0100, 0x0504, 0x0706, 0x0908, 0x0908, 0x0d0c, 0x0d0c); // 1q
	vec_shuf[ 4] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 0q
	vec_shuf[ 5] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 1q
	vec_shuf[ 6] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 2q
	vec_shuf[ 7] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0908, 0x0908, 0x0908, 0x0908); // 3q
	vec_shuf[ 8] = _mm_set_epi16(0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e, 0x0f0e); // 0
	vec_shuf[ 9] = _mm_set_epi16(0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c, 0x0d0c); // 1
	vec_shuf[10] = _mm_set_epi16(0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a, 0x0b0a); // 2
	vec_shuf[11] = _mm_set_epi16(0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908, 0x0908); // 3
	vec_shuf[12] = _mm_set_epi16(0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706, 0x0706); // 4
	vec_shuf[13] = _mm_set_epi16(0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504, 0x0504); // 5
	vec_shuf[14] = _mm_set_epi16(0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302, 0x0302); // 6
	vec_shuf[15] = _mm_set_epi16(0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100); // 7
	m_accum_h = _mm_setzero_si128();
	m_accum_m = _mm_setzero_si128();
	m_accum_l = _mm_setzero_si128();
	m_accum_ll = _mm_setzero_si128();
	vec_neg1 = _mm_set_epi64x(0xffffffffffffffffL, 0xffffffffffffffffL);
	vec_zero = _mm_setzero_si128();
	vec_himask = _mm_set_epi64x(0xffff0000ffff0000L, 0xffff0000ffff0000L);
	vec_lomask = _mm_set_epi64x(0x0000ffff0000ffffL, 0x0000ffff0000ffffL);
	vec_hibit = _mm_set_epi64x(0x0001000000010000L, 0x0001000000010000L);
	vec_lobit = _mm_set_epi64x(0x0000000100000001L, 0x0000000100000001L);
	vec_32767 = _mm_set_epi64x(0x7fff7fff7fff7fffL, 0x7fff7fff7fff7fffL);
	vec_n32768 = _mm_set_epi64x(0x8000800080008000L, 0x8000800080008000L);
	vec_flagmask = _mm_set_epi64x(0x0001000100010001L, 0x0001000100010001L);
	vec_shiftmask2 = _mm_set_epi64x(0x0000000300000003L, 0x0000000300000003L);
	vec_shiftmask4 = _mm_set_epi64x(0x000000000000000fL, 0x000000000000000fL);
	vec_flag_reverse = _mm_set_epi16(0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e);
}

void rsp_cop2_simd::state_string_export(const int index, astring &string)
{
	switch (index)
	{
		case RSP_V0:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 0], 7), (UINT16)_mm_extract_epi16(m_xv[ 0], 6), (UINT16)_mm_extract_epi16(m_xv[ 0], 5), (UINT16)_mm_extract_epi16(m_xv[ 0], 4), (UINT16)_mm_extract_epi16(m_xv[ 0], 3), (UINT16)_mm_extract_epi16(m_xv[ 0], 2), (UINT16)_mm_extract_epi16(m_xv[ 0], 1), (UINT16)_mm_extract_epi16(m_xv[ 0], 0));
			break;
		case RSP_V1:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 1], 7), (UINT16)_mm_extract_epi16(m_xv[ 1], 6), (UINT16)_mm_extract_epi16(m_xv[ 1], 5), (UINT16)_mm_extract_epi16(m_xv[ 1], 4), (UINT16)_mm_extract_epi16(m_xv[ 1], 3), (UINT16)_mm_extract_epi16(m_xv[ 1], 2), (UINT16)_mm_extract_epi16(m_xv[ 1], 1), (UINT16)_mm_extract_epi16(m_xv[ 1], 0));
			break;
		case RSP_V2:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 2], 7), (UINT16)_mm_extract_epi16(m_xv[ 2], 6), (UINT16)_mm_extract_epi16(m_xv[ 2], 5), (UINT16)_mm_extract_epi16(m_xv[ 2], 4), (UINT16)_mm_extract_epi16(m_xv[ 2], 3), (UINT16)_mm_extract_epi16(m_xv[ 2], 2), (UINT16)_mm_extract_epi16(m_xv[ 2], 1), (UINT16)_mm_extract_epi16(m_xv[ 2], 0));
			break;
		case RSP_V3:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 3], 7), (UINT16)_mm_extract_epi16(m_xv[ 3], 6), (UINT16)_mm_extract_epi16(m_xv[ 3], 5), (UINT16)_mm_extract_epi16(m_xv[ 3], 4), (UINT16)_mm_extract_epi16(m_xv[ 3], 3), (UINT16)_mm_extract_epi16(m_xv[ 3], 2), (UINT16)_mm_extract_epi16(m_xv[ 3], 1), (UINT16)_mm_extract_epi16(m_xv[ 3], 0));
			break;
		case RSP_V4:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 4], 7), (UINT16)_mm_extract_epi16(m_xv[ 4], 6), (UINT16)_mm_extract_epi16(m_xv[ 4], 5), (UINT16)_mm_extract_epi16(m_xv[ 4], 4), (UINT16)_mm_extract_epi16(m_xv[ 4], 3), (UINT16)_mm_extract_epi16(m_xv[ 4], 2), (UINT16)_mm_extract_epi16(m_xv[ 4], 1), (UINT16)_mm_extract_epi16(m_xv[ 4], 0));
			break;
		case RSP_V5:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 5], 7), (UINT16)_mm_extract_epi16(m_xv[ 5], 6), (UINT16)_mm_extract_epi16(m_xv[ 5], 5), (UINT16)_mm_extract_epi16(m_xv[ 5], 4), (UINT16)_mm_extract_epi16(m_xv[ 5], 3), (UINT16)_mm_extract_epi16(m_xv[ 5], 2), (UINT16)_mm_extract_epi16(m_xv[ 5], 1), (UINT16)_mm_extract_epi16(m_xv[ 5], 0));
			break;
		case RSP_V6:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 6], 7), (UINT16)_mm_extract_epi16(m_xv[ 6], 6), (UINT16)_mm_extract_epi16(m_xv[ 6], 5), (UINT16)_mm_extract_epi16(m_xv[ 6], 4), (UINT16)_mm_extract_epi16(m_xv[ 6], 3), (UINT16)_mm_extract_epi16(m_xv[ 6], 2), (UINT16)_mm_extract_epi16(m_xv[ 6], 1), (UINT16)_mm_extract_epi16(m_xv[ 6], 0));
			break;
		case RSP_V7:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 7], 7), (UINT16)_mm_extract_epi16(m_xv[ 7], 6), (UINT16)_mm_extract_epi16(m_xv[ 7], 5), (UINT16)_mm_extract_epi16(m_xv[ 7], 4), (UINT16)_mm_extract_epi16(m_xv[ 7], 3), (UINT16)_mm_extract_epi16(m_xv[ 7], 2), (UINT16)_mm_extract_epi16(m_xv[ 7], 1), (UINT16)_mm_extract_epi16(m_xv[ 7], 0));
			break;
		case RSP_V8:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 8], 7), (UINT16)_mm_extract_epi16(m_xv[ 8], 6), (UINT16)_mm_extract_epi16(m_xv[ 8], 5), (UINT16)_mm_extract_epi16(m_xv[ 8], 4), (UINT16)_mm_extract_epi16(m_xv[ 8], 3), (UINT16)_mm_extract_epi16(m_xv[ 8], 2), (UINT16)_mm_extract_epi16(m_xv[ 8], 1), (UINT16)_mm_extract_epi16(m_xv[ 8], 0));
			break;
		case RSP_V9:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[ 9], 7), (UINT16)_mm_extract_epi16(m_xv[ 9], 6), (UINT16)_mm_extract_epi16(m_xv[ 9], 5), (UINT16)_mm_extract_epi16(m_xv[ 9], 4), (UINT16)_mm_extract_epi16(m_xv[ 9], 3), (UINT16)_mm_extract_epi16(m_xv[ 9], 2), (UINT16)_mm_extract_epi16(m_xv[ 9], 1), (UINT16)_mm_extract_epi16(m_xv[ 9], 0));
			break;
		case RSP_V10:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[10], 7), (UINT16)_mm_extract_epi16(m_xv[10], 6), (UINT16)_mm_extract_epi16(m_xv[10], 5), (UINT16)_mm_extract_epi16(m_xv[10], 4), (UINT16)_mm_extract_epi16(m_xv[10], 3), (UINT16)_mm_extract_epi16(m_xv[10], 2), (UINT16)_mm_extract_epi16(m_xv[10], 1), (UINT16)_mm_extract_epi16(m_xv[10], 0));
			break;
		case RSP_V11:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[11], 7), (UINT16)_mm_extract_epi16(m_xv[11], 6), (UINT16)_mm_extract_epi16(m_xv[11], 5), (UINT16)_mm_extract_epi16(m_xv[11], 4), (UINT16)_mm_extract_epi16(m_xv[11], 3), (UINT16)_mm_extract_epi16(m_xv[11], 2), (UINT16)_mm_extract_epi16(m_xv[11], 1), (UINT16)_mm_extract_epi16(m_xv[11], 0));
			break;
		case RSP_V12:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[12], 7), (UINT16)_mm_extract_epi16(m_xv[12], 6), (UINT16)_mm_extract_epi16(m_xv[12], 5), (UINT16)_mm_extract_epi16(m_xv[12], 4), (UINT16)_mm_extract_epi16(m_xv[12], 3), (UINT16)_mm_extract_epi16(m_xv[12], 2), (UINT16)_mm_extract_epi16(m_xv[12], 1), (UINT16)_mm_extract_epi16(m_xv[12], 0));
			break;
		case RSP_V13:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[13], 7), (UINT16)_mm_extract_epi16(m_xv[13], 6), (UINT16)_mm_extract_epi16(m_xv[13], 5), (UINT16)_mm_extract_epi16(m_xv[13], 4), (UINT16)_mm_extract_epi16(m_xv[13], 3), (UINT16)_mm_extract_epi16(m_xv[13], 2), (UINT16)_mm_extract_epi16(m_xv[13], 1), (UINT16)_mm_extract_epi16(m_xv[13], 0));
			break;
		case RSP_V14:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[14], 7), (UINT16)_mm_extract_epi16(m_xv[14], 6), (UINT16)_mm_extract_epi16(m_xv[14], 5), (UINT16)_mm_extract_epi16(m_xv[14], 4), (UINT16)_mm_extract_epi16(m_xv[14], 3), (UINT16)_mm_extract_epi16(m_xv[14], 2), (UINT16)_mm_extract_epi16(m_xv[14], 1), (UINT16)_mm_extract_epi16(m_xv[14], 0));
			break;
		case RSP_V15:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[15], 7), (UINT16)_mm_extract_epi16(m_xv[15], 6), (UINT16)_mm_extract_epi16(m_xv[15], 5), (UINT16)_mm_extract_epi16(m_xv[15], 4), (UINT16)_mm_extract_epi16(m_xv[15], 3), (UINT16)_mm_extract_epi16(m_xv[15], 2), (UINT16)_mm_extract_epi16(m_xv[15], 1), (UINT16)_mm_extract_epi16(m_xv[15], 0));
			break;
		case RSP_V16:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[16], 7), (UINT16)_mm_extract_epi16(m_xv[16], 6), (UINT16)_mm_extract_epi16(m_xv[16], 5), (UINT16)_mm_extract_epi16(m_xv[16], 4), (UINT16)_mm_extract_epi16(m_xv[16], 3), (UINT16)_mm_extract_epi16(m_xv[16], 2), (UINT16)_mm_extract_epi16(m_xv[16], 1), (UINT16)_mm_extract_epi16(m_xv[16], 0));
			break;
		case RSP_V17:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[17], 7), (UINT16)_mm_extract_epi16(m_xv[17], 6), (UINT16)_mm_extract_epi16(m_xv[17], 5), (UINT16)_mm_extract_epi16(m_xv[17], 4), (UINT16)_mm_extract_epi16(m_xv[17], 3), (UINT16)_mm_extract_epi16(m_xv[17], 2), (UINT16)_mm_extract_epi16(m_xv[17], 1), (UINT16)_mm_extract_epi16(m_xv[17], 0));
			break;
		case RSP_V18:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[18], 7), (UINT16)_mm_extract_epi16(m_xv[18], 6), (UINT16)_mm_extract_epi16(m_xv[18], 5), (UINT16)_mm_extract_epi16(m_xv[18], 4), (UINT16)_mm_extract_epi16(m_xv[18], 3), (UINT16)_mm_extract_epi16(m_xv[18], 2), (UINT16)_mm_extract_epi16(m_xv[18], 1), (UINT16)_mm_extract_epi16(m_xv[18], 0));
			break;
		case RSP_V19:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[19], 7), (UINT16)_mm_extract_epi16(m_xv[19], 6), (UINT16)_mm_extract_epi16(m_xv[19], 5), (UINT16)_mm_extract_epi16(m_xv[19], 4), (UINT16)_mm_extract_epi16(m_xv[19], 3), (UINT16)_mm_extract_epi16(m_xv[19], 2), (UINT16)_mm_extract_epi16(m_xv[19], 1), (UINT16)_mm_extract_epi16(m_xv[19], 0));
			break;
		case RSP_V20:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[20], 7), (UINT16)_mm_extract_epi16(m_xv[20], 6), (UINT16)_mm_extract_epi16(m_xv[20], 5), (UINT16)_mm_extract_epi16(m_xv[20], 4), (UINT16)_mm_extract_epi16(m_xv[20], 3), (UINT16)_mm_extract_epi16(m_xv[20], 2), (UINT16)_mm_extract_epi16(m_xv[20], 1), (UINT16)_mm_extract_epi16(m_xv[20], 0));
			break;
		case RSP_V21:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[21], 7), (UINT16)_mm_extract_epi16(m_xv[21], 6), (UINT16)_mm_extract_epi16(m_xv[21], 5), (UINT16)_mm_extract_epi16(m_xv[21], 4), (UINT16)_mm_extract_epi16(m_xv[21], 3), (UINT16)_mm_extract_epi16(m_xv[21], 2), (UINT16)_mm_extract_epi16(m_xv[21], 1), (UINT16)_mm_extract_epi16(m_xv[21], 0));
			break;
		case RSP_V22:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[22], 7), (UINT16)_mm_extract_epi16(m_xv[22], 6), (UINT16)_mm_extract_epi16(m_xv[22], 5), (UINT16)_mm_extract_epi16(m_xv[22], 4), (UINT16)_mm_extract_epi16(m_xv[22], 3), (UINT16)_mm_extract_epi16(m_xv[22], 2), (UINT16)_mm_extract_epi16(m_xv[22], 1), (UINT16)_mm_extract_epi16(m_xv[22], 0));
			break;
		case RSP_V23:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[23], 7), (UINT16)_mm_extract_epi16(m_xv[23], 6), (UINT16)_mm_extract_epi16(m_xv[23], 5), (UINT16)_mm_extract_epi16(m_xv[23], 4), (UINT16)_mm_extract_epi16(m_xv[23], 3), (UINT16)_mm_extract_epi16(m_xv[23], 2), (UINT16)_mm_extract_epi16(m_xv[23], 1), (UINT16)_mm_extract_epi16(m_xv[23], 0));
			break;
		case RSP_V24:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[24], 7), (UINT16)_mm_extract_epi16(m_xv[24], 6), (UINT16)_mm_extract_epi16(m_xv[24], 5), (UINT16)_mm_extract_epi16(m_xv[24], 4), (UINT16)_mm_extract_epi16(m_xv[24], 3), (UINT16)_mm_extract_epi16(m_xv[24], 2), (UINT16)_mm_extract_epi16(m_xv[24], 1), (UINT16)_mm_extract_epi16(m_xv[24], 0));
			break;
		case RSP_V25:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[25], 7), (UINT16)_mm_extract_epi16(m_xv[25], 6), (UINT16)_mm_extract_epi16(m_xv[25], 5), (UINT16)_mm_extract_epi16(m_xv[25], 4), (UINT16)_mm_extract_epi16(m_xv[25], 3), (UINT16)_mm_extract_epi16(m_xv[25], 2), (UINT16)_mm_extract_epi16(m_xv[25], 1), (UINT16)_mm_extract_epi16(m_xv[25], 0));
			break;
		case RSP_V26:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[26], 7), (UINT16)_mm_extract_epi16(m_xv[26], 6), (UINT16)_mm_extract_epi16(m_xv[26], 5), (UINT16)_mm_extract_epi16(m_xv[26], 4), (UINT16)_mm_extract_epi16(m_xv[26], 3), (UINT16)_mm_extract_epi16(m_xv[26], 2), (UINT16)_mm_extract_epi16(m_xv[26], 1), (UINT16)_mm_extract_epi16(m_xv[26], 0));
			break;
		case RSP_V27:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[27], 7), (UINT16)_mm_extract_epi16(m_xv[27], 6), (UINT16)_mm_extract_epi16(m_xv[27], 5), (UINT16)_mm_extract_epi16(m_xv[27], 4), (UINT16)_mm_extract_epi16(m_xv[27], 3), (UINT16)_mm_extract_epi16(m_xv[27], 2), (UINT16)_mm_extract_epi16(m_xv[27], 1), (UINT16)_mm_extract_epi16(m_xv[27], 0));
			break;
		case RSP_V28:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[28], 7), (UINT16)_mm_extract_epi16(m_xv[28], 6), (UINT16)_mm_extract_epi16(m_xv[28], 5), (UINT16)_mm_extract_epi16(m_xv[28], 4), (UINT16)_mm_extract_epi16(m_xv[28], 3), (UINT16)_mm_extract_epi16(m_xv[28], 2), (UINT16)_mm_extract_epi16(m_xv[28], 1), (UINT16)_mm_extract_epi16(m_xv[28], 0));
			break;
		case RSP_V29:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[29], 7), (UINT16)_mm_extract_epi16(m_xv[29], 6), (UINT16)_mm_extract_epi16(m_xv[29], 5), (UINT16)_mm_extract_epi16(m_xv[29], 4), (UINT16)_mm_extract_epi16(m_xv[29], 3), (UINT16)_mm_extract_epi16(m_xv[29], 2), (UINT16)_mm_extract_epi16(m_xv[29], 1), (UINT16)_mm_extract_epi16(m_xv[29], 0));
			break;
		case RSP_V30:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[30], 7), (UINT16)_mm_extract_epi16(m_xv[30], 6), (UINT16)_mm_extract_epi16(m_xv[30], 5), (UINT16)_mm_extract_epi16(m_xv[30], 4), (UINT16)_mm_extract_epi16(m_xv[30], 3), (UINT16)_mm_extract_epi16(m_xv[30], 2), (UINT16)_mm_extract_epi16(m_xv[30], 1), (UINT16)_mm_extract_epi16(m_xv[30], 0));
			break;
		case RSP_V31:
			string.printf("%04X|%04X|%04X|%04X|%04X|%04X|%04X|%04X", (UINT16)_mm_extract_epi16(m_xv[31], 7), (UINT16)_mm_extract_epi16(m_xv[31], 6), (UINT16)_mm_extract_epi16(m_xv[31], 5), (UINT16)_mm_extract_epi16(m_xv[31], 4), (UINT16)_mm_extract_epi16(m_xv[31], 3), (UINT16)_mm_extract_epi16(m_xv[31], 2), (UINT16)_mm_extract_epi16(m_xv[31], 1), (UINT16)_mm_extract_epi16(m_xv[31], 0));
			break;
	}
}

/***************************************************************************
    Vector Load Instructions
***************************************************************************/

// LBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Load 1 byte to vector byte index

inline void rsp_cop2_simd::lbv()
{
	UINT32 op = m_op;

	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;

	UINT16 element;
	EXTRACT16(m_xv[dest], element, (index >> 1));
	element &= 0xff00 >> ((1-(index & 1)) * 8);
	element |= m_rsp.DM_READ8(ea) << ((1-(index & 1)) * 8);
	INSERT16(m_xv[dest], element, (index >> 1));
}

static void cfunc_lbv(void *param)
{
	((rsp_cop2 *)param)->lbv();
}


// LSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Loads 2 bytes starting from vector byte index

inline void rsp_cop2_simd::lsv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xe;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);
	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		UINT16 element;
		EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= m_rsp.DM_READ8(ea) << ((1 - (i & 1)) * 8);
		INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_lsv(void *param)
{
	((rsp_cop2 *)param)->lsv();
}


// LLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Loads 4 bytes starting from vector byte index

inline void rsp_cop2_simd::llv()
{
	UINT32 op = m_op;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xc;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		UINT16 element;
		EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= m_rsp.DM_READ8(ea) << ((1 - (i & 1)) * 8);
		INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_llv(void *param)
{
	((rsp_cop2 *)param)->llv();
}
#endif


// LDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Loads 8 bytes starting from vector byte index

inline void rsp_cop2_simd::ldv()
{
	UINT32 op = m_op;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;

	for (int i = index; i < end; i++)
	{
		UINT16 element;
		EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= m_rsp.DM_READ8(ea) << ((1 - (i & 1)) * 8);
		INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_ldv(void *param)
{
	((rsp_cop2 *)param)->ldv();
}
#endif


// LQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Loads up to 16 bytes starting from vector byte index

inline void rsp_cop2_simd::lqv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = 16 - (ea & 0xf);
	if (end > 16) end = 16;

	for (int i = 0; i < end; i++)
	{
		UINT16 element;
		EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1 - (i & 1)) * 8);
		element |= m_rsp.DM_READ8(ea) << ((1 - (i & 1)) * 8);
		INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_lqv(void *param)
{
	((rsp_cop2 *)param)->lqv();
}


// LRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

inline void rsp_cop2_simd::lrv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	index = 16 - ((ea & 0xf) - index);
	ea &= ~0xf;

	for (int i = index; i < 16; i++)
	{
		UINT16 element;
		EXTRACT16(m_xv[dest], element, (i >> 1));
		element &= 0xff00 >> ((1-(i & 1)) * 8);
		element |= m_rsp.DM_READ8(ea) << ((1-(i & 1)) * 8);
		INSERT16(m_xv[dest], element, (i >> 1));
		ea++;
	}
}

static void cfunc_lrv(void *param)
{
	((rsp_cop2 *)param)->lrv();
}


// LPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the upper 8 bits of each element

inline void rsp_cop2_simd::lpv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		INSERT16(m_xv[dest], m_rsp.DM_READ8(ea + (((16-index) + i) & 0xf)) << 8, i);
	}
}

static void cfunc_lpv(void *param)
{
	((rsp_cop2 *)param)->lpv();
}


// LUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element

inline void rsp_cop2_simd::luv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	for (int i = 0; i < 8; i++)
	{
		INSERT16(m_xv[dest], m_rsp.DM_READ8(ea + (((16-index) + i) & 0xf)) << 7, i);
	}
}

static void cfunc_luv(void *param)
{
	((rsp_cop2 *)param)->luv();
}


// LHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of each element, with 2-byte stride

inline void rsp_cop2_simd::lhv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	for (int i = 0; i < 8; i++)
	{
		INSERT16(m_xv[dest], m_rsp.DM_READ8(ea + (((16-index) + (i<<1)) & 0xf)) << 7, i);
	}
}

static void cfunc_lhv(void *param)
{
	((rsp_cop2 *)param)->lhv();
}


// LFV
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

inline void rsp_cop2_simd::lfv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...

	int end = (index >> 1) + 4;

	for (int i = index >> 1; i < end; i++)
	{
		INSERT16(m_xv[dest], m_rsp.DM_READ8(ea) << 7, i);
		ea += 4;
	}
}

static void cfunc_lfv(void *param)
{
	((rsp_cop2 *)param)->lfv();
}


// LWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

inline void rsp_cop2_simd::lwv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = (16 - index) + 16;

	UINT8 val[16];
	for (int i = (16 - index); i < end; i++)
	{
		val[i & 0xf] = m_rsp.DM_READ8(ea);
		ea += 4;
	}

	m_xv[dest] = _mm_set_epi8(val[15], val[14], val[13], val[12], val[11], val[10], val[ 9], val[ 8],
									val[ 7], val[ 6], val[ 5], val[ 4], val[ 3], val[ 2], val[ 1], val[ 0]);
}

static void cfunc_lwv(void *param)
{
	((rsp_cop2 *)param)->lwv();
}


// LTV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Loads one element to maximum of 8 vectors, while incrementing element index

inline void rsp_cop2_simd::ltv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);

	// FIXME: has a small problem with odd indices

	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
	{
		ve = 32;
	}

	int element = 7 - (index >> 1);

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (int i = vs; i < ve; i++)
	{
		element = (8 - (index >> 1) + (i - vs)) << 1;
		UINT16 value = (m_rsp.DM_READ8(ea) << 8) | m_rsp.DM_READ8(ea + 1);
		INSERT16(m_xv[i], value, (element >> 1));
		ea += 2;
	}
}

static void cfunc_ltv(void *param)
{
	((rsp_cop2 *)param)->ltv();
}


/***************************************************************************
    Vector Store Instructions
***************************************************************************/

// SBV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
// --------------------------------------------------
//
// Stores 1 byte from vector byte index

inline void rsp_cop2_simd::sbv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + offset : offset;
	UINT16 value;
	EXTRACT16(m_xv[dest], value, (index >> 1));
	value >>= (1-(index & 1)) * 8;
	m_rsp.DM_WRITE8(ea, (UINT8)value);
}

static void cfunc_sbv(void *param)
{
	((rsp_cop2 *)param)->sbv();
}


// SSV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
// --------------------------------------------------
//
// Stores 2 bytes starting from vector byte index

inline void rsp_cop2_simd::ssv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 2) : (offset * 2);

	int end = index + 2;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		m_rsp.DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_ssv(void *param)
{
	((rsp_cop2 *)param)->ssv();
}


// SLV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
// --------------------------------------------------
//
// Stores 4 bytes starting from vector byte index

inline void rsp_cop2_simd::slv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 4) : (offset * 4);

	int end = index + 4;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		m_rsp.DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_slv(void *param)
{
	((rsp_cop2 *)param)->slv();
}


// SDV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
// --------------------------------------------------
//
// Stores 8 bytes starting from vector byte index

inline void rsp_cop2_simd::sdv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0x8;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);

	int end = index + 8;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1 - (i & 1)) * 8;
		m_rsp.DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_sdv(void *param)
{
	((rsp_cop2 *)param)->sdv();
}


// SQV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

inline void rsp_cop2_simd::sqv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int end = index + (16 - (ea & 0xf));
	for (int i=index; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, (i >> 1));
		value >>= (1-(i & 1)) * 8;
		m_rsp.DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_sqv(void *param)
{
	((rsp_cop2 *)param)->sqv();
}


// SRV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
// --------------------------------------------------
//
// Stores up to 16 bytes starting from right side until 16-byte boundary

inline void rsp_cop2_simd::srv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);

	int end = index + (ea & 0xf);
	int o = (16 - (ea & 0xf)) & 0xf;
	ea &= ~0xf;

	for (int i = index; i < end; i++)
	{
		UINT32 bi = (i + o) & 0xf;
		UINT16 value;
		EXTRACT16(m_xv[dest], value, (bi >> 1));
		value >>= (1-(bi & 1)) * 8;
		m_rsp.DM_WRITE8(ea, (UINT8)value);
		ea++;
	}
}

static void cfunc_srv(void *param)
{
	((rsp_cop2 *)param)->srv();
}


// SPV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
// --------------------------------------------------
//
// Stores upper 8 bits of each element

inline void rsp_cop2_simd::spv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			UINT16 value;
			EXTRACT16(m_xv[dest], value, i);
			m_rsp.DM_WRITE8(ea, (UINT8)(value >> 8));
		}
		else
		{
			UINT16 value;
			EXTRACT16(m_xv[dest], value, i);
			m_rsp.DM_WRITE8(ea, (UINT8)(value >> 7));
		}
		ea++;
	}
}

static void cfunc_spv(void *param)
{
	((rsp_cop2 *)param)->spv();
}


// SUV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element

inline void rsp_cop2_simd::suv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 8) : (offset * 8);
	int end = index + 8;
	for (int i=index; i < end; i++)
	{
		if ((i & 0xf) < 8)
		{
			UINT16 value;
			EXTRACT16(m_xv[dest], value, i);
			m_rsp.DM_WRITE8(ea, (UINT8)(value >> 7));
		}
		else
		{
			UINT16 value;
			EXTRACT16(m_xv[dest], value, i);
			m_rsp.DM_WRITE8(ea, (UINT8)(value >> 8));
		}
		ea++;
	}
}

static void cfunc_suv(void *param)
{
	((rsp_cop2 *)param)->suv();
}


// SHV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of each element, with 2-byte stride

inline void rsp_cop2_simd::shv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	for (int i=0; i < 8; i++)
	{
		int element = index + (i << 1);
		UINT16 value;
		EXTRACT16(m_xv[dest], value, element >> 1);
		m_rsp.DM_WRITE8(ea, (value >> 7) & 0x00ff);
		ea += 2;
	}
}

static void cfunc_shv(void *param)
{
	((rsp_cop2 *)param)->shv();
}


// SFV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
// --------------------------------------------------
//
// Stores bits 14-7 of upper or lower quad, with 4-byte stride

inline void rsp_cop2_simd::sfv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = (index >> 1) + 4;

	for (int i = index>>1; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, i);
		m_rsp.DM_WRITE8(ea + (eaoffset & 0xf), (value >> 7) & 0x00ff);
		eaoffset += 4;
	}
}

static void cfunc_sfv(void *param)
{
	((rsp_cop2 *)param)->sfv();
}


// SWV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
// --------------------------------------------------
//
// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
// after byte index 15

inline void rsp_cop2_simd::swv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = ea & 0xf;
	ea &= ~0xf;

	int end = index + 16;
	for (int i = index; i < end; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[dest], value, i >> 1);
		m_rsp.DM_WRITE8(ea + (eaoffset & 0xf), (value >> ((1-(i & 1)) * 8)) & 0xff);
		eaoffset++;
	}
}

static void cfunc_swv(void *param)
{
	((rsp_cop2 *)param)->swv();
}


// STV
//
// 31       25      20      15      10     6        0
// --------------------------------------------------
// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
// --------------------------------------------------
//
// Stores one element from maximum of 8 vectors, while incrementing element index

inline void rsp_cop2_simd::stv()
{
	UINT32 op = m_op;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);

	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
	{
		ve = 32;
	}

	int element = 8 - (index >> 1);

	UINT32 ea = (base) ? m_rsp.m_rsp_state->r[base] + (offset * 16) : (offset * 16);
	int eaoffset = (ea & 0xf) + (element * 2);
	ea &= ~0xf;

	for (int i = vs; i < ve; i++)
	{
		UINT16 value;
		EXTRACT16(m_xv[i], value, element);
		m_rsp.DM_WRITE16(ea + (eaoffset & 0xf), value);
		eaoffset += 2;
		element++;
	}
}

static void cfunc_stv(void *param)
{
	((rsp_cop2 *)param)->stv();
}


/***************************************************************************
    SIMD Accelerators
***************************************************************************/

/* ============================================================================
* RSPPackLo32to16: Pack LSBs of 32-bit vectors to 16-bits without saturation.
* TODO: 5 SSE2 operations is kind of expensive just to truncate values?
* ========================================================================= */
INLINE __m128i RSPPackLo32to16(__m128i vectorLow, __m128i vectorHigh)
{
	vectorLow = _mm_slli_epi32(vectorLow, 16);
	vectorHigh = _mm_slli_epi32(vectorHigh, 16);
	vectorLow = _mm_srai_epi32(vectorLow, 16);
	vectorHigh = _mm_srai_epi32(vectorHigh, 16);
	return _mm_packs_epi32(vectorLow, vectorHigh);
}

/* ============================================================================
* RSPPackHi32to16: Pack MSBs of 32-bit vectors to 16-bits without saturation.
* ========================================================================= */
INLINE __m128i RSPPackHi32to16(__m128i vectorLow, __m128i vectorHigh)
{
	vectorLow = _mm_srai_epi32(vectorLow, 16);
	vectorHigh = _mm_srai_epi32(vectorHigh, 16);
	return _mm_packs_epi32(vectorLow, vectorHigh);
}

/* ============================================================================
* RSPSignExtend16to32: Sign-extend 16-bit slices to 32-bit slices.
* ========================================================================= */
INLINE void RSPSignExtend16to32(__m128i source, __m128i *vectorLow, __m128i *vectorHigh)
{
	__m128i vMask = _mm_srai_epi16(source, 15);
	*vectorHigh = _mm_unpackhi_epi16(source, vMask);
	*vectorLow = _mm_unpacklo_epi16(source, vMask);
}

/* ============================================================================
* RSPZeroExtend16to32: Zero-extend 16-bit slices to 32-bit slices.
* ========================================================================= */
INLINE void RSPZeroExtend16to32(__m128i source, __m128i *vectorLow, __m128i *vectorHigh)
{
	*vectorHigh = _mm_unpackhi_epi16(source, _mm_setzero_si128());
	*vectorLow = _mm_unpacklo_epi16(source, _mm_setzero_si128());
}

/* ============================================================================
* _mm_mullo_epi32: SSE2 lacks _mm_mullo_epi32, define it manually.
* TODO/WARNING/DISCLAIMER: Assumes one argument is positive.
* ========================================================================= */
INLINE __m128i _mm_mullo_epi32(__m128i a, __m128i b)
{
	__m128i a4 = _mm_srli_si128(a, 4);
	__m128i b4 = _mm_srli_si128(b, 4);
	__m128i ba = _mm_mul_epu32(b, a);
	__m128i b4a4 = _mm_mul_epu32(b4, a4);

	__m128i mask = _mm_setr_epi32(~0, 0, ~0, 0);
	__m128i baMask = _mm_and_si128(ba, mask);
	__m128i b4a4Mask = _mm_and_si128(b4a4, mask);
	__m128i b4a4MaskShift = _mm_slli_si128(b4a4Mask, 4);

	return _mm_or_si128(baMask, b4a4MaskShift);
}

/* ============================================================================
* RSPClampLowToVal: Clamps the low word of the accumulator.
* ========================================================================= */
INLINE __m128i RSPClampLowToVal(__m128i vaccLow, __m128i vaccMid, __m128i vaccHigh)
{
	__m128i setMask = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());
	__m128i negCheck, useValMask, negVal, posVal;

	/* Compute some common values ahead of time. */
	negCheck = _mm_cmplt_epi16(vaccHigh, _mm_setzero_si128());

	/* If accmulator < 0, clamp to val if val != TMin. */
	useValMask = _mm_and_si128(vaccHigh, _mm_srai_epi16(vaccMid, 15));
	useValMask = _mm_cmpeq_epi16(useValMask, setMask);
	negVal = _mm_and_si128(useValMask, vaccLow);

	/* Otherwise, clamp to ~0 if any high bits are set. */
	useValMask = _mm_or_si128(vaccHigh, _mm_srai_epi16(vaccMid, 15));
	useValMask = _mm_cmpeq_epi16(useValMask, _mm_setzero_si128());
	posVal = _mm_and_si128(useValMask, vaccLow);

	negVal = _mm_and_si128(negCheck, negVal);
	posVal = _mm_andnot_si128(negCheck, posVal);
	return _mm_or_si128(negVal, posVal);
}


/***************************************************************************
    Vector Opcodes
***************************************************************************/

// VMULF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer * 2

inline void rsp_cop2_simd::vmulf()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		if (s1 == -32768 && s2 == -32768)
		{
			// overflow
			SET_ACCUM_H(0, i);
			SET_ACCUM_M(-32768, i);
			SET_ACCUM_L(-32768, i);
			m_vres[i] = 0x7fff;
		}
		else
		{
			INT64 r =  s1 * s2 * 2;
			r += 0x8000;    // rounding ?
			SET_ACCUM_H((r < 0) ? 0xffff : 0, i);
			SET_ACCUM_M((INT16)(r >> 16), i);
			SET_ACCUM_L((UINT16)(r), i);
			m_vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_vmulf(void *param)
{
	((rsp_cop2 *)param)->vmulf();
}


// VMULU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
// ------------------------------------------------------
//

inline void rsp_cop2_simd::vmulu()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT64 r = s1 * s2 * 2;
		r += 0x8000;    // rounding ?

		SET_ACCUM_H((UINT16)(r >> 32), i);
		SET_ACCUM_M((UINT16)(r >> 16), i);
		SET_ACCUM_L((UINT16)(r), i);

		if (r < 0)
		{
			m_vres[i] = 0;
		}
		else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
		{
			m_vres[i] = -1;
		}
		else
		{
			m_vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
}

static void cfunc_vmulu(void *param)
{
	((rsp_cop2 *)param)->vmulu();
}


// VMUDL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
// ------------------------------------------------------
//
// Multiplies signed integer by unsigned fraction
// The result is added into accumulator
// The middle slice of accumulator is stored into destination element

inline void rsp_cop2_simd::vmudl()
{
	int op = m_op;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	__m128i unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	__m128i unpackHi = _mm_mulhi_epu16(vsReg, vtReg);
	__m128i loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	__m128i hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);

	m_xv[VDREG] = m_accum_l = RSPPackHi32to16(loProduct, hiProduct);

	m_accum_m = _mm_setzero_si128();
	m_accum_h = _mm_setzero_si128();
}

static void cfunc_vmudl(void *param)
{
	((rsp_cop2 *)param)->vmudl();
}


// VMUDM
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
// ------------------------------------------------------
//
// Multiplies signed integer by unsigned fraction
// The result is stored into accumulator
// The middle slice of accumulator is stored into destination element

inline void rsp_cop2_simd::vmudm()
{
	int op = m_op;

	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPSignExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPZeroExtend16to32(vtReg, &vtRegLo, &vtRegHi);

	/* Begin accumulating the products. */
	__m128i loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	__m128i hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);
	m_accum_l = RSPPackLo32to16(loProduct, hiProduct);
	m_accum_m = m_xv[VDREG] = RSPPackHi32to16(loProduct, hiProduct);

	loProduct = _mm_cmplt_epi32(loProduct, _mm_setzero_si128());
	hiProduct = _mm_cmplt_epi32(hiProduct, _mm_setzero_si128());
	m_accum_h = _mm_packs_epi32(loProduct, hiProduct);
}

static void cfunc_vmudm(void *param)
{
	((rsp_cop2 *)param)->vmudm();
}


// VMUDN
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
// ------------------------------------------------------
//
// Multiplies unsigned fraction by signed integer
// The result is stored into accumulator
// The low slice of accumulator is stored into destination element

inline void rsp_cop2_simd::vmudn()
{
	int op = m_op;

	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPZeroExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPSignExtend16to32(vtReg, &vtRegLo, &vtRegHi);

	/* Begin accumulating the products. */
	__m128i loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	__m128i hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);
	m_xv[VDREG] = m_accum_l = RSPPackLo32to16(loProduct, hiProduct);
	m_accum_m = RSPPackHi32to16(loProduct, hiProduct);
	m_accum_h = _mm_cmplt_epi16(m_accum_m, _mm_setzero_si128());
}

static void cfunc_vmudn(void *param)
{
	((rsp_cop2 *)param)->vmudn();
}


// VMUDH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer
// The result is stored into highest 32 bits of accumulator, the low slice is zero
// The highest 32 bits of accumulator is saturated into destination element

inline void rsp_cop2_simd::vmudh()
{
	int op = m_op;

	__m128i vaccLow, vaccHigh;
	__m128i unpackLo, unpackHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Multiply the sources, accumulate the product. */
	unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	vaccHigh = _mm_unpackhi_epi16(unpackLo, unpackHi);
	vaccLow = _mm_unpacklo_epi16(unpackLo, unpackHi);

	/* Pack the accumulator and result back up. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_l = _mm_setzero_si128();
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_vmudh(void *param)
{
	((rsp_cop2 *)param)->vmudh();
}


// VMACF
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
// ------------------------------------------------------
//

inline void rsp_cop2_simd::vmacf()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		INT32 s1 = (INT32)(INT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		INT32 r = s1 * s2;

		UINT64 q = (UINT64)(UINT16)ACCUM_LL(i);
		q |= (((UINT64)(UINT16)ACCUM_L(i)) << 16);
		q |= (((UINT64)(UINT16)ACCUM_M(i)) << 32);
		q |= (((UINT64)(UINT16)ACCUM_H(i)) << 48);

		q += (INT64)(r) << 17;
		SET_ACCUM_LL((UINT16)q, i);
		SET_ACCUM_L((UINT16)(q >> 16), i);
		SET_ACCUM_M((UINT16)(q >> 32), i);
		SET_ACCUM_H((UINT16)(q >> 48), i);

		m_vres[i] = SATURATE_ACCUM(i, 1, 0x8000, 0x7fff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_vmacf(void *param)
{
	((rsp_cop2 *)param)->vmacf();
}


// VMACU
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
// ------------------------------------------------------
//

inline void rsp_cop2_simd::vmacu()
{
	int op = m_op;

	__m128i loProduct, hiProduct, unpackLo, unpackHi;
	__m128i vaccHigh;
	__m128i vdReg, vdRegLo, vdRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	__m128i vaccLow = m_accum_l;

	/* Unpack to obtain for 32-bit precision. */
	RSPZeroExtend16to32(vaccLow, &vaccLow, &vaccHigh);

	/* Begin accumulating the products. */
	unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);
	loProduct = _mm_slli_epi32(loProduct, 1);
	hiProduct = _mm_slli_epi32(hiProduct, 1);

	vdRegLo = _mm_srli_epi32(loProduct, 16);
	vdRegHi = _mm_srli_epi32(hiProduct, 16);
	vdRegLo = _mm_slli_epi32(vdRegLo, 16);
	vdRegHi = _mm_slli_epi32(vdRegHi, 16);
	vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
	vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);

	vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
	vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

	m_accum_l = vdReg = RSPPackLo32to16(vaccLow, vaccHigh);

	/* Multiply the MSB of sources, accumulate the product. */
	vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	loProduct = _mm_srai_epi32(loProduct, 16);
	hiProduct = _mm_srai_epi32(hiProduct, 16);
	vaccLow = _mm_srai_epi32(vaccLow, 16);
	vaccHigh = _mm_srai_epi32(vaccHigh, 16);

	vaccLow = _mm_add_epi32(loProduct, vaccLow);
	vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
	vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
	vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

	/* Clamp the accumulator and write it all out. */
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_vmacu(void *param)
{
	((rsp_cop2 *)param)->vmacu();
}


// VMADL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
// ------------------------------------------------------
//
// Multiplies unsigned fraction by unsigned fraction
// Adds the higher 16 bits of the 32-bit result to accumulator
// The low slice of accumulator is stored into destination element

inline void rsp_cop2_simd::vmadl()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		UINT32 s1 = w1;
		UINT32 s2 = w2;

		UINT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
		UINT32 r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

		SET_ACCUM_L((UINT16)r2, i);
		SET_ACCUM_M((UINT16)r3, i);
		SET_ACCUM_H(ACCUM_H(i) + (INT16)(r3 >> 16), i);

		m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_vmadl(void *param)
{
	((rsp_cop2 *)param)->vmadl();
}


// VMADM
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
// ------------------------------------------------------
//
// Multiplies signed fraction by unsigned fraction
// Adds the higher 16 bits of the 32-bit result to accumulator
// The medium slice of accumulator is stored into destination element

inline void rsp_cop2_simd::vmadm()
{
	int op = m_op;

	__m128i vaccLow, vaccHigh, loProduct, hiProduct;
	__m128i vsRegLo, vsRegHi, vtRegLo, vtRegHi, vdRegLo, vdRegHi;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	RSPSignExtend16to32(vsReg, &vsRegLo, &vsRegHi);
	RSPZeroExtend16to32(vtReg, &vtRegLo, &vtRegHi);
	RSPZeroExtend16to32(m_accum_l, &vaccLow, &vaccHigh);

	/* Begin accumulating the products. */
	loProduct = _mm_mullo_epi32(vsRegLo, vtRegLo);
	hiProduct = _mm_mullo_epi32(vsRegHi, vtRegHi);

	vdRegLo = _mm_srli_epi32(loProduct, 16);
	vdRegHi = _mm_srli_epi32(hiProduct, 16);
	vdRegLo = _mm_slli_epi32(vdRegLo, 16);
	vdRegHi = _mm_slli_epi32(vdRegHi, 16);
	vdRegLo = _mm_xor_si128(vdRegLo, loProduct);
	vdRegHi = _mm_xor_si128(vdRegHi, hiProduct);
	vaccLow = _mm_add_epi32(vaccLow, vdRegLo);
	vaccHigh = _mm_add_epi32(vaccHigh, vdRegHi);

	m_accum_l = m_xv[VDREG] = RSPPackLo32to16(vaccLow, vaccHigh);

	/* Multiply the MSB of sources, accumulate the product. */
	vdRegLo = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	vdRegHi = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	loProduct = _mm_srai_epi32(loProduct, 16);
	hiProduct = _mm_srai_epi32(hiProduct, 16);
	vaccLow = _mm_srai_epi32(vaccLow, 16);
	vaccHigh = _mm_srai_epi32(vaccHigh, 16);

	vaccLow = _mm_add_epi32(loProduct, vaccLow);
	vaccHigh = _mm_add_epi32(hiProduct, vaccHigh);
	vaccLow = _mm_add_epi32(vdRegLo, vaccLow);
	vaccHigh = _mm_add_epi32(vdRegHi, vaccHigh);

	/* Clamp the accumulator and write it all out. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_vmadm(void *param)
{
	((rsp_cop2 *)param)->vmadm();
}


// VMADN
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
// ------------------------------------------------------
//
// Multiplies unsigned fraction by signed fraction
// Adds the 32-bit result to the medium and high slices of the accumulator
// The low slice of accumulator is saturated into destination element

inline void rsp_cop2_simd::vmadn()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);
		INT32 s1 = (UINT16)w1;
		INT32 s2 = (INT32)(INT16)w2;

		UINT64 q = (UINT64)ACCUM_LL(i);
		q |= (((UINT64)ACCUM_L(i)) << 16);
		q |= (((UINT64)ACCUM_M(i)) << 32);
		q |= (((UINT64)ACCUM_H(i)) << 48);
		q += (INT64)(s1*s2) << 16;

		SET_ACCUM_LL((UINT16)q, i);
		SET_ACCUM_L((UINT16)(q >> 16), i);
		SET_ACCUM_M((UINT16)(q >> 32), i);
		SET_ACCUM_H((UINT16)(q >> 48), i);

		m_vres[i] = SATURATE_ACCUM(i, 0, 0x0000, 0xffff);
	}
	WRITEBACK_RESULT();
}

static void cfunc_vmadn(void *param)
{
	((rsp_cop2 *)param)->vmadn();
}


// VMADH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
// ------------------------------------------------------
//
// Multiplies signed integer by signed integer
// The result is added into highest 32 bits of accumulator, the low slice is zero
// The highest 32 bits of accumulator is saturated into destination element

inline void rsp_cop2_simd::vmadh()
{
	int op = m_op;

	__m128i vsReg = m_xv[VS1REG];
	__m128i vtReg = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	/* Unpack to obtain for 32-bit precision. */
	__m128i vaccLow = _mm_unpacklo_epi16(m_accum_m, m_accum_h);
	__m128i vaccHigh = _mm_unpackhi_epi16(m_accum_m, m_accum_h);

	/* Multiply the sources, accumulate the product. */
	__m128i unpackLo = _mm_mullo_epi16(vsReg, vtReg);
	__m128i unpackHi = _mm_mulhi_epi16(vsReg, vtReg);
	__m128i loProduct = _mm_unpacklo_epi16(unpackLo, unpackHi);
	__m128i hiProduct = _mm_unpackhi_epi16(unpackLo, unpackHi);
	vaccLow = _mm_add_epi32(vaccLow, loProduct);
	vaccHigh = _mm_add_epi32(vaccHigh, hiProduct);

	/* Pack the accumulator and result back up. */
	m_xv[VDREG] = _mm_packs_epi32(vaccLow, vaccHigh);
	m_accum_m = RSPPackLo32to16(vaccLow, vaccHigh);
	m_accum_h = RSPPackHi32to16(vaccLow, vaccHigh);
}

static void cfunc_vmadh(void *param)
{
	((rsp_cop2 *)param)->vmadh();
}


// VADD
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
// ------------------------------------------------------
//
// Adds two vector registers and carry flag, the result is saturated to 32767

inline void rsp_cop2_simd::vadd()
{
	int op = m_op;

	__m128i shuffled = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i carry = _mm_and_si128(m_xvflag[CARRY], vec_flagmask);
	m_accum_l = _mm_add_epi16(_mm_add_epi16(m_xv[VS1REG], shuffled), carry);

	__m128i addvec = _mm_adds_epi16(m_xv[VS1REG], shuffled);

	carry = _mm_and_si128(carry, _mm_xor_si128(_mm_cmpeq_epi16(addvec, vec_32767), vec_neg1));
	carry = _mm_and_si128(carry, _mm_xor_si128(_mm_cmpeq_epi16(addvec, vec_n32768), vec_neg1));

	m_xv[VDREG] = _mm_add_epi16(addvec, carry);

	m_xvflag[ZERO] = vec_zero;
	m_xvflag[CARRY] = vec_zero;
}

static void cfunc_vadd(void *param)
{
	((rsp_cop2 *)param)->vadd();
}


// VSUB
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
// ------------------------------------------------------
//
// Subtracts two vector registers and carry flag, the result is saturated to -32768
// TODO: check VS2REG == VDREG

inline void rsp_cop2_simd::vsub()
{
	int op = m_op;

	__m128i shuffled = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i carry = _mm_and_si128(m_xvflag[CARRY], vec_flagmask);
	__m128i unsat = _mm_sub_epi16(m_xv[VS1REG], shuffled);

	__m128i vs2neg = _mm_cmplt_epi16(shuffled, vec_zero);
	__m128i vs2pos = _mm_cmpeq_epi16(vs2neg, vec_zero);

	__m128i saturated = _mm_subs_epi16(m_xv[VS1REG], shuffled);
	__m128i carry_mask = _mm_cmpeq_epi16(unsat, saturated);
	carry_mask = _mm_and_si128(vs2neg, carry_mask);

	vs2neg = _mm_and_si128(carry_mask, carry);
	vs2pos = _mm_and_si128(vs2pos, carry);
	__m128i dest_carry = _mm_or_si128(vs2neg, vs2pos);
	m_xv[VDREG] = _mm_subs_epi16(saturated, dest_carry);

	m_accum_l = _mm_sub_epi16(unsat, carry);

	m_xvflag[ZERO] = _mm_setzero_si128();
	m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_vsub(void *param)
{
	((rsp_cop2 *)param)->vsub();
}


// VABS
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
// ------------------------------------------------------
//
// Changes the sign of source register 2 if source register 1 is negative and stores the result to destination register

inline void rsp_cop2_simd::vabs()
{
	int op = m_op;

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i negs2 = _mm_sub_epi16(_mm_setzero_si128(), shuf2);
	__m128i s2_n32768 = _mm_cmpeq_epi16(shuf2, vec_n32768);
	__m128i s1_lz = _mm_cmplt_epi16(m_xv[VS1REG], _mm_setzero_si128());

	__m128i result_gz = _mm_and_si128(shuf2, _mm_cmpgt_epi16(m_xv[VS1REG], _mm_setzero_si128()));
	__m128i result_n32768 = _mm_and_si128(s1_lz, _mm_and_si128(vec_32767, s2_n32768));
	__m128i result_negs2 = _mm_and_si128(s1_lz, _mm_and_si128(negs2, _mm_xor_si128(s2_n32768, vec_neg1)));
	m_xv[VDREG] = m_accum_l = _mm_or_si128(result_gz, _mm_or_si128(result_n32768, result_negs2));
}

static void cfunc_vabs(void *param)
{
	((rsp_cop2 *)param)->vabs();
}


// VADDC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
// ------------------------------------------------------
//
// Adds two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

inline void rsp_cop2_simd::vaddc()
{
	int op = m_op;

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i vec7531 = _mm_and_si128(m_xv[VS1REG], vec_lomask);
	__m128i vec6420 = _mm_srli_epi32(m_xv[VS1REG], 16);
	__m128i shuf7531 = _mm_and_si128(shuf2, vec_lomask);
	__m128i shuf6420 = _mm_srli_epi32(shuf2, 16);
	__m128i sum7531 = _mm_add_epi32(vec7531, shuf7531);
	__m128i sum6420 = _mm_add_epi32(vec6420, shuf6420);

	__m128i over7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_himask);
	__m128i over6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_himask);

	sum7531 = _mm_and_si128(sum7531, vec_lomask);
	sum6420 = _mm_and_si128(sum6420, vec_lomask);

	m_xvflag[CARRY] = _mm_or_si128(over6420, _mm_srli_epi32(over7531, 16));
	m_accum_l = m_xv[VDREG] = _mm_or_si128(_mm_slli_epi32(sum6420, 16), sum7531);
}

static void cfunc_vaddc(void *param)
{
	((rsp_cop2 *)param)->vaddc();
}


// VSUBC
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
// ------------------------------------------------------
//
// Subtracts two vector registers, the carry out is stored into carry register
// TODO: check VS2REG = VDREG

inline void rsp_cop2_simd::vsubc()
{
	int op = m_op;

	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	__m128i shuf2 = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i vec7531 = _mm_and_si128(m_xv[VS1REG], vec_lomask);
	__m128i vec6420 = _mm_srli_epi32(m_xv[VS1REG], 16);
	__m128i shuf7531 = _mm_and_si128(shuf2, vec_lomask);
	__m128i shuf6420 = _mm_srli_epi32(shuf2, 16);
	__m128i sum7531 = _mm_sub_epi32(vec7531, shuf7531);
	__m128i sum6420 = _mm_sub_epi32(vec6420, shuf6420);

	__m128i over7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_himask);
	__m128i over6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_himask);
	sum7531 = _mm_and_si128(sum7531, vec_lomask);
	sum6420 = _mm_and_si128(sum6420, vec_lomask);
	__m128i zero7531 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum7531, _mm_setzero_si128()), vec_neg1), vec_lomask);
	__m128i zero6420 = _mm_and_si128(_mm_xor_si128(_mm_cmpeq_epi16(sum6420, _mm_setzero_si128()), vec_neg1), vec_lomask);

	m_xvflag[CARRY] = _mm_or_si128(over6420, _mm_srli_epi32(over7531, 16));
	m_xvflag[ZERO] = _mm_or_si128(_mm_slli_epi32(zero6420, 16), zero7531);

	m_accum_l = m_xv[VDREG] = _mm_or_si128(_mm_slli_epi32(sum6420, 16), sum7531);
}

static void cfunc_vsubc(void *param)
{
	((rsp_cop2 *)param)->vsubc();
}


// VADDB
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010110 |
// ------------------------------------------------------
//
// Adds two vector registers bytewise with rounding
inline void rsp_cop2_simd::vaddb()
{
	const int op = m_op;
	const int round = (EL == 0) ? 0 : (1 << (EL - 1));

	for (int i = 0; i < 8; i++)
	{
		UINT16 w1, w2;
		GET_VS1(w1, i);
		GET_VS2(w2, i);

		UINT8 hb1 = w1 >> 8;
		UINT8 lb1 = w1 & 0xff;
		UINT8 hb2 = w2 >> 8;
		UINT8 lb2 = w2 & 0xff;

		UINT16 hs = hb1 + hb2 + round;
		UINT16 ls = lb1 + lb2 + round;

		SET_ACCUM_L((hs << 8) | ls, i);

		hs >>= EL;
		if (hs > 255)
		{
			hs = 255;
		}

		ls >>= EL;
		if (ls > 255)
		{
			ls = 255;
		}

		m_vres[i] = 0; // VD writeback disabled on production hardware
		// m_vres[i] = (hs << 8) | ls;
	}
	WRITEBACK_RESULT();
}

static void cfunc_vaddb(void *param)
{
	((rsp_cop2 *)param)->vaddb();
}


// VSAW
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
// ------------------------------------------------------
//
// Stores high, middle or low slice of accumulator to destination vector

inline void rsp_cop2_simd::vsaw()
{
	int op = m_op;

	switch (EL)
	{
		case 0x08:      // VSAWH
		{
			m_xv[VDREG] = m_accum_h;
			break;
		}
		case 0x09:      // VSAWM
		{
			m_xv[VDREG] = m_accum_m;
			break;
		}
		case 0x0a:      // VSAWL
		{
			m_xv[VDREG] = m_accum_l;
			break;
		}
		default:        // Unsupported, writes 0 to VD
		{
		}
	}
}

static void cfunc_vsaw(void *param)
{
	((rsp_cop2 *)param)->vsaw();
}


// VLT
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are less than VS2
// Moves the element in VS2 to destination vector

inline void rsp_cop2_simd::vlt()
{
	int op = m_op;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zc_mask = _mm_and_si128(m_xvflag[ZERO], m_xvflag[CARRY]);
	__m128i lt_mask = _mm_cmplt_epi16(m_xv[VS1REG], shuf);
	__m128i eq_mask = _mm_and_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), zc_mask);

	m_xvflag[COMPARE] = _mm_or_si128(lt_mask, eq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_void vlt(void *param)
{
	((rsp_cop2 *)param)->vlt();
}


// VEQ
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_cop2_simd::veq()
{
	int op = m_op;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zero_mask = _mm_cmpeq_epi16(m_xvflag[ZERO], _mm_setzero_si128());
	__m128i eq_mask = _mm_cmpeq_epi16(m_xv[VS1REG], shuf);

	m_xvflag[COMPARE] = _mm_and_si128(zero_mask, eq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_veq(void *param)
{
	((rsp_cop2 *)param)->veq();
}


// VNE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are not equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_cop2_simd::vne()
{
	int op = m_op;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i neq_mask = _mm_xor_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), vec_neg1);

	m_xvflag[COMPARE] = _mm_or_si128(m_xvflag[ZERO], neq_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_vne(void *param)
{
	((rsp_cop2 *)param)->vne();
}


// VGE
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
// ------------------------------------------------------
//
// Sets compare flags if elements in VS1 are greater or equal with VS2
// Moves the element in VS2 to destination vector

inline void rsp_cop2_simd::vge()
{
	int op = m_op;

	m_xvflag[COMPARE] = m_xvflag[CLIP2] = _mm_setzero_si128();

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i zero_mask = _mm_cmpeq_epi16(m_xvflag[ZERO], _mm_setzero_si128());
	__m128i carry_mask = _mm_cmpeq_epi16(m_xvflag[CARRY], _mm_setzero_si128());
	__m128i flag_mask = _mm_or_si128(zero_mask, carry_mask);
	__m128i eq_mask = _mm_and_si128(_mm_cmpeq_epi16(m_xv[VS1REG], shuf), flag_mask);
	__m128i gt_mask = _mm_cmpgt_epi16(m_xv[VS1REG], shuf);
	m_xvflag[COMPARE] = _mm_or_si128(eq_mask, gt_mask);

	__m128i result = _mm_and_si128(m_xv[VS1REG], m_xvflag[COMPARE]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, _mm_xor_si128(m_xvflag[COMPARE], vec_neg1)));

	m_xvflag[ZERO] = m_xvflag[CARRY] = _mm_setzero_si128();
}

static void cfunc_vge(void *param)
{
	((rsp_cop2 *)param)->vge();
}


// VCL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
// ------------------------------------------------------
//
// Vector clip low

inline void rsp_cop2_simd::vcl()
{
	int op = m_op;

	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if (CARRY_FLAG(i) != 0)
		{
			if (ZERO_FLAG(i) != 0)
			{
				if (COMPARE_FLAG(i) != 0)
				{
					SET_ACCUM_L(-(UINT16)s2, i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
				}
			}
			else
			{
				if (CLIP1_FLAG(i) != 0)
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
					{
						SET_ACCUM_L(s1, i);
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(-((UINT16)s2), i);
						SET_COMPARE_FLAG(i);
					}
				}
				else
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
					{
						SET_ACCUM_L(s1, i);
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						SET_ACCUM_L(-((UINT16)s2), i);
						SET_COMPARE_FLAG(i);
					}
				}
			}
		}
		else
		{
			if (ZERO_FLAG(i) != 0)
			{
				if (CLIP2_FLAG(i) != 0)
				{
					SET_ACCUM_L(s2, i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
				}
			}
			else
			{
				if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
				{
					SET_ACCUM_L(s2, i);
					SET_CLIP2_FLAG(i);
				}
				else
				{
					SET_ACCUM_L(s1, i);
					CLEAR_CLIP2_FLAG(i);
				}
			}
		}
		m_vres[i] = ACCUM_L(i);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	CLEAR_CLIP1_FLAGS();
	WRITEBACK_RESULT();
}

static void cfunc_vcl(void *param)
{
	((rsp_cop2 *)param)->vcl();
}


// VCH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
// ------------------------------------------------------
//
// Vector clip high

inline void rsp_cop2_simd::vch()
{
	int op = m_op;

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	UINT32 vce = 0;
	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if ((s1 ^ s2) < 0)
		{
			vce = (s1 + s2 == -1);
			SET_CARRY_FLAG(i);
			if (s2 < 0)
			{
				SET_CLIP2_FLAG(i);
			}

			if ((s1 + s2) <= 0)
			{
				SET_COMPARE_FLAG(i);
				m_vres[i] = -((UINT16)s2);
			}
			else
			{
				m_vres[i] = s1;
			}

			if ((s1 + s2) != 0 && s1 != ~s2)
			{
				SET_ZERO_FLAG(i);
			}
		}//sign
		else
		{
			vce = 0;
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				SET_CLIP2_FLAG(i);
				m_vres[i] = s2;
			}
			else
			{
				m_vres[i] = s1;
			}

			if ((s1 - s2) != 0 && s1 != ~s2)
			{
				SET_ZERO_FLAG(i);
			}
		}
		if (vce)
		{
			SET_CLIP1_FLAG(i);
		}
		SET_ACCUM_L(m_vres[i], i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_vch(void *param)
{
	((rsp_cop2 *)param)->vch();
}


// VCR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
// ------------------------------------------------------
//
// Vector clip reverse

inline void rsp_cop2_simd::vcr()
{
	int op = m_op;

	CLEAR_CARRY_FLAGS();
	CLEAR_COMPARE_FLAGS();
	CLEAR_CLIP1_FLAGS();
	CLEAR_ZERO_FLAGS();
	CLEAR_CLIP2_FLAGS();

	for (int i = 0; i < 8; i++)
	{
		INT16 s1, s2;
		GET_VS1(s1, i);
		GET_VS2(s2, i);

		if ((INT16)(s1 ^ s2) < 0)
		{
			if (s2 < 0)
			{
				SET_CLIP2_FLAG(i);
			}
			if ((s1 + s2) <= 0)
			{
				SET_ACCUM_L(~((UINT16)s2), i);
				SET_COMPARE_FLAG(i);
			}
			else
			{
				SET_ACCUM_L(s1, i);
			}
		}
		else
		{
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				SET_ACCUM_L(s2, i);
				SET_CLIP2_FLAG(i);
			}
			else
			{
				SET_ACCUM_L(s1, i);
			}
		}

		m_vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}

static void cfunc_vcr(void *param)
{
	((rsp_cop2 *)param)->vcr();
}


// VMRG
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
// ------------------------------------------------------
//
// Merges two vectors according to compare flags

inline void rsp_cop2_simd::vmrg()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	__m128i s2mask = _mm_cmpeq_epi16(m_xvflag[COMPARE], _mm_setzero_si128());
	__m128i s1mask = _mm_xor_si128(s2mask, vec_neg1);
	__m128i result = _mm_and_si128(m_xv[VS1REG], s1mask);
	m_xv[VDREG] = _mm_or_si128(result, _mm_and_si128(shuf, s2mask));
	m_accum_l = m_xv[VDREG];
}

static void cfunc_vmrg(void *param)
{
	((rsp_cop2 *)param)->vmrg();
}


// VAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
// ------------------------------------------------------
//
// Bitwise AND of two vector registers

inline void rsp_cop2_simd::vand()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_and_si128(m_xv[VS1REG], shuf);
}

static void cfunc_vand(void *param)
{
	((rsp_cop2 *)param)->vand();
}


// VNAND
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
// ------------------------------------------------------
//
// Bitwise NOT AND of two vector registers

inline void rsp_cop2_simd::vnand()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_xor_si128(_mm_and_si128(m_xv[VS1REG], shuf), vec_neg1);
}

static void cfunc_vnand(void *param)
{
	((rsp_cop2 *)param)->vnand();
}


// VOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
// ------------------------------------------------------
//
// Bitwise OR of two vector registers

inline void rsp_cop2_simd::vor()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_or_si128(m_xv[VS1REG], shuf);
}

static void cfunc_vor_simd(void *param)
{
	((rsp_cop2 *)param)->vor();
}


// VNOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
// ------------------------------------------------------
//
// Bitwise NOT OR of two vector registers

inline void rsp_cop2_simd::vnor()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_xor_si128(_mm_or_si128(m_xv[VS1REG], shuf), vec_neg1);
}

static void cfunc_vnor(void *param)
{
	((rsp_cop2 *)param)->vnor();
}


// VXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
// ------------------------------------------------------
//
// Bitwise XOR of two vector registers

inline void rsp_cop2_simd::vxor()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_xor_si128(m_xv[VS1REG], shuf);
}

static void cfunc_vxor(void *param)
{
	((rsp_cop2 *)param)->vxor();
}


// VNXOR
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
// ------------------------------------------------------
//
// Bitwise NOT XOR of two vector registers

inline void rsp_cop2_simd::vnxor()
{
	int op = m_op;

	__m128i shuf = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
	m_accum_l = m_xv[VDREG] = _mm_xor_si128(_mm_xor_si128(m_xv[VS1REG], shuf), vec_neg1);
}

static void cfunc_vnxor(void *param)
{
	((rsp_cop2 *)param)->vnxor();
}


// VRCP
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
// ------------------------------------------------------
//
// Calculates reciprocal

inline void rsp_cop2_simd::vrcp()
{
	int op = m_op;

	INT32 shifter = 0;
	UINT16 urec;
	INT32 rec;
	EXTRACT16(m_xv[VS2REG], urec, EL);
	rec = (INT16)urec;
	INT32 datainput = (rec < 0) ? (-rec) : rec;
	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0x10;
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_vrcp(void *param)
{
	((rsp_cop2 *)param)->vrcp();
}


// VRCPL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
// ------------------------------------------------------
//
// Calculates reciprocal low part

inline void rsp_cop2_simd::vrcpl()
{
	int op = m_op;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	INT32 shifter = 0;

	UINT16 urec;
	EXTRACT16(m_xv[VS2REG], urec, EL);
	INT32 rec = (INT16)urec;
	INT32 datainput = rec;

	if (m_dp_allowed)
	{
		rec = (rec & 0x0000ffff) | m_reciprocal_high;
		datainput = rec;

		if (rec < 0)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
	}
	else if (datainput < 0)
	{
		datainput = -datainput;

		shifter = 0x10;
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	temp ^= rec >> 31;

	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);

	for (int i = 0; i < 8; i++)
	{
		INT16 val;
		EXTRACT16(m_xv[VS2REG], val, VEC_EL_2(EL, i));
		SET_ACCUM_L(val, i);
	}
}

static void cfunc_vrcpl(void *param)
{
	((rsp_cop2 *)param)->vrcpl();
}


// VRCPH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
// ------------------------------------------------------
//
// Calculates reciprocal high part

inline void rsp_cop2_simd::vrcph()
{
	int op = m_op;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	UINT16 rcph;
	EXTRACT16(m_xv[VS2REG], rcph, EL);
	m_reciprocal_high = rcph << 16;
	m_dp_allowed = 1;

	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	INSERT16(m_xv[VDREG], (INT16)(m_reciprocal_res >> 16), VS1REG);
}

static void cfunc_vrcph(void *param)
{
	((rsp_cop2 *)param)->vrcph();
}


// VMOV
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
// ------------------------------------------------------
//
// Moves element from vector to destination vector

inline void rsp_cop2_simd::vmov()
{
	int op = m_op;

	INT16 val;
	EXTRACT16(m_xv[VS2REG], val, EL);
	INSERT16(m_xv[VDREG], val, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_vmov(void *param)
{
	((rsp_cop2 *)param)->vmov();
}


// VRSQ
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110100 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root

inline void rsp_cop2_simd::vrsq()
{
	int op = m_op;

	INT32 shifter = 0;
	INT32 rec = (INT16)VREG_S(VS2REG, EL & 7);
	INT32 datainput = (rec < 0) ? (-rec) : (rec);

	if (rec < 0)
	{
		if (rec < -32768)
		{
			datainput = ~datainput;
		}
		else
		{
			datainput = -datainput;
		}
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0;
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	if (rec < 0)
	{
		if (m_dp_allowed)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
		else
		{
			datainput = -datainput;
		}
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0;
	}

	address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	fetchval = rsp_divtable[address];
	temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0x0000;
	}
	rec = temp;

	W_VREG_S(VDREG, VS1REG & 7) = (UINT16)rec;
	for (int i = 0; i < 8; i++)
	{
		SET_ACCUM_L(VREG_S(VS2REG, VEC_EL_2(EL, i)), i);
	}
}

static void cfunc_vrsq(void *param)
{
	((rsp_cop2 *)param)->vrsq();
}


// VRSQL
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root low part

inline void rsp_cop2_simd::vrsql()
{
	int op = m_op;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	INT32 shifter = 0;
	UINT16 val;
	EXTRACT16(m_xv[VS2REG], val, EL);
	INT32 rec = (INT16)val;
	INT32 datainput = rec;

	if (m_dp_allowed)
	{
		rec = (rec & 0x0000ffff) | m_reciprocal_high;
		datainput = rec;

		if (rec < 0)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
	}
	else if (datainput < 0)
	{
		datainput = -datainput;

		shifter = 0x10;
	}

	if (datainput)
	{
		for (int i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	temp ^= rec >> 31;

	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	m_reciprocal_res = rec;
	m_dp_allowed = 0;

	INSERT16(m_xv[VDREG], (UINT16)rec, VS1REG);
	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);
}

static void cfunc_vrsql(void *param)
{
	((rsp_cop2 *)param)->vrsql();
}


// VRSQH
//
// 31       25  24     20      15      10      5        0
// ------------------------------------------------------
// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
// ------------------------------------------------------
//
// Calculates reciprocal square-root high part

inline void rsp_cop2_simd::vrsqh()
{
	int op = m_op;

#if SIMUL_SIMD
	m_old_reciprocal_res = m_reciprocal_res;
	m_old_reciprocal_high = m_reciprocal_high;
	m_old_dp_allowed = m_dp_allowed;
#endif

	UINT16 val;
	EXTRACT16(m_xv[VS2REG], val, EL);
	m_reciprocal_high = val << 16;
	m_dp_allowed = 1;

	m_accum_l = _mm_shuffle_epi8(m_xv[VS2REG], vec_shuf_inverse[EL]);

	INSERT16(m_xv[VDREG], (INT16)(m_reciprocal_res >> 16), VS1REG); // store high part
}

static void cfunc_vrsqh(void *param)
{
	((rsp_cop2 *)param)->vrsqh();
}


/***************************************************************************
    Vector Flag Reading/Writing
***************************************************************************/

inline void rsp_cop2_simd::mfc2()
{
	UINT32 op = m_op;
	int el = (op >> 7) & 0xf;

	UINT16 out;
	EXTRACT16(m_xv[VS1REG], out, (el >> 1));
	out >>= (1 - (el & 1)) * 8;
	out &= 0x00ff;

	el++;

	UINT16 temp;
	EXTRACT16(m_xv[VS1REG], temp, (el >> 1));
	temp >>= (1 - (el & 1)) * 8;
	temp &= 0x00ff;

	m_rsp.m_rsp_state->r[RTREG] = (INT32)(INT16)((out << 8) | temp);
}

static void cfunc_mfc2(void *param)
{
	((rsp_cop2 *)param)->mfc2();
}


inline void rsp_cop2_simd::cfc2()
{
	UINT32 op = m_op;
	if (RTREG)
	{
		switch(RDREG)
		{
			case 0:
				RTVAL = ((CARRY_FLAG(0) & 1) << 0) |
						((CARRY_FLAG(1) & 1) << 1) |
						((CARRY_FLAG(2) & 1) << 2) |
						((CARRY_FLAG(3) & 1) << 3) |
						((CARRY_FLAG(4) & 1) << 4) |
						((CARRY_FLAG(5) & 1) << 5) |
						((CARRY_FLAG(6) & 1) << 6) |
						((CARRY_FLAG(7) & 1) << 7) |
						((ZERO_FLAG(0) & 1) << 8) |
						((ZERO_FLAG(1) & 1) << 9) |
						((ZERO_FLAG(2) & 1) << 10) |
						((ZERO_FLAG(3) & 1) << 11) |
						((ZERO_FLAG(4) & 1) << 12) |
						((ZERO_FLAG(5) & 1) << 13) |
						((ZERO_FLAG(6) & 1) << 14) |
						((ZERO_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 1:
				RTVAL = ((COMPARE_FLAG(0) & 1) << 0) |
						((COMPARE_FLAG(1) & 1) << 1) |
						((COMPARE_FLAG(2) & 1) << 2) |
						((COMPARE_FLAG(3) & 1) << 3) |
						((COMPARE_FLAG(4) & 1) << 4) |
						((COMPARE_FLAG(5) & 1) << 5) |
						((COMPARE_FLAG(6) & 1) << 6) |
						((COMPARE_FLAG(7) & 1) << 7) |
						((CLIP2_FLAG(0) & 1) << 8) |
						((CLIP2_FLAG(1) & 1) << 9) |
						((CLIP2_FLAG(2) & 1) << 10) |
						((CLIP2_FLAG(3) & 1) << 11) |
						((CLIP2_FLAG(4) & 1) << 12) |
						((CLIP2_FLAG(5) & 1) << 13) |
						((CLIP2_FLAG(6) & 1) << 14) |
						((CLIP2_FLAG(7) & 1) << 15);
				if (RTVAL & 0x8000) RTVAL |= 0xffff0000;
				break;
			case 2:
				RTVAL = ((CLIP1_FLAG(0) & 1) << 0) |
						((CLIP1_FLAG(1) & 1) << 1) |
						((CLIP1_FLAG(2) & 1) << 2) |
						((CLIP1_FLAG(3) & 1) << 3) |
						((CLIP1_FLAG(4) & 1) << 4) |
						((CLIP1_FLAG(5) & 1) << 5) |
						((CLIP1_FLAG(6) & 1) << 6) |
						((CLIP1_FLAG(7) & 1) << 7);
				break;
		}
	}
}

static void cfunc_cfc2(void *param)
{
	((rsp_cop2 *)param)->cfc2();
}


inline void rsp_cop2_simd::mtc2()
{
	UINT32 op = m_op;
	int el = (op >> 7) & 0xf;
	INSERT16(m_xv[VS1REG], RTVAL, el >> 1);
}

static void cfunc_mtc2(void *param)
{
	((rsp_cop2 *)param)->mtc2();
}


inline void rsp_cop2_simd::ctc2()
{
	UINT32 op = m_op;
	switch(RDREG)
	{
		case 0:
			CLEAR_CARRY_FLAGS();
			CLEAR_ZERO_FLAGS();
			m_vflag[0][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[0][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[0][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[0][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[0][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[0][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[0][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[0][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0))  { SET_CARRY_FLAG(0); }
			if (RTVAL & (1 << 1))  { SET_CARRY_FLAG(1); }
			if (RTVAL & (1 << 2))  { SET_CARRY_FLAG(2); }
			if (RTVAL & (1 << 3))  { SET_CARRY_FLAG(3); }
			if (RTVAL & (1 << 4))  { SET_CARRY_FLAG(4); }
			if (RTVAL & (1 << 5))  { SET_CARRY_FLAG(5); }
			if (RTVAL & (1 << 6))  { SET_CARRY_FLAG(6); }
			if (RTVAL & (1 << 7))  { SET_CARRY_FLAG(7); }
			m_vflag[3][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[3][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[3][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[3][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[3][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[3][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[3][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[3][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_ZERO_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_ZERO_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_ZERO_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_ZERO_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_ZERO_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_ZERO_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_ZERO_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_ZERO_FLAG(7); }
			break;
		case 1:
			CLEAR_COMPARE_FLAGS();
			CLEAR_CLIP2_FLAGS();
			m_vflag[1][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[1][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[1][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[1][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[1][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[1][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[1][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[1][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_COMPARE_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_COMPARE_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_COMPARE_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_COMPARE_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_COMPARE_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_COMPARE_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_COMPARE_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_COMPARE_FLAG(7); }
			m_vflag[4][0] = ((RTVAL >> 8) & 1) ? 0xffff : 0;
			m_vflag[4][1] = ((RTVAL >> 9) & 1) ? 0xffff : 0;
			m_vflag[4][2] = ((RTVAL >> 10) & 1) ? 0xffff : 0;
			m_vflag[4][3] = ((RTVAL >> 11) & 1) ? 0xffff : 0;
			m_vflag[4][4] = ((RTVAL >> 12) & 1) ? 0xffff : 0;
			m_vflag[4][5] = ((RTVAL >> 13) & 1) ? 0xffff : 0;
			m_vflag[4][6] = ((RTVAL >> 14) & 1) ? 0xffff : 0;
			m_vflag[4][7] = ((RTVAL >> 15) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 8))  { SET_CLIP2_FLAG(0); }
			if (RTVAL & (1 << 9))  { SET_CLIP2_FLAG(1); }
			if (RTVAL & (1 << 10)) { SET_CLIP2_FLAG(2); }
			if (RTVAL & (1 << 11)) { SET_CLIP2_FLAG(3); }
			if (RTVAL & (1 << 12)) { SET_CLIP2_FLAG(4); }
			if (RTVAL & (1 << 13)) { SET_CLIP2_FLAG(5); }
			if (RTVAL & (1 << 14)) { SET_CLIP2_FLAG(6); }
			if (RTVAL & (1 << 15)) { SET_CLIP2_FLAG(7); }
			break;
		case 2:
			CLEAR_CLIP1_FLAGS();
			m_vflag[2][0] = ((RTVAL >> 0) & 1) ? 0xffff : 0;
			m_vflag[2][1] = ((RTVAL >> 1) & 1) ? 0xffff : 0;
			m_vflag[2][2] = ((RTVAL >> 2) & 1) ? 0xffff : 0;
			m_vflag[2][3] = ((RTVAL >> 3) & 1) ? 0xffff : 0;
			m_vflag[2][4] = ((RTVAL >> 4) & 1) ? 0xffff : 0;
			m_vflag[2][5] = ((RTVAL >> 5) & 1) ? 0xffff : 0;
			m_vflag[2][6] = ((RTVAL >> 6) & 1) ? 0xffff : 0;
			m_vflag[2][7] = ((RTVAL >> 7) & 1) ? 0xffff : 0;
			if (RTVAL & (1 << 0)) { SET_CLIP1_FLAG(0); }
			if (RTVAL & (1 << 1)) { SET_CLIP1_FLAG(1); }
			if (RTVAL & (1 << 2)) { SET_CLIP1_FLAG(2); }
			if (RTVAL & (1 << 3)) { SET_CLIP1_FLAG(3); }
			if (RTVAL & (1 << 4)) { SET_CLIP1_FLAG(4); }
			if (RTVAL & (1 << 5)) { SET_CLIP1_FLAG(5); }
			if (RTVAL & (1 << 6)) { SET_CLIP1_FLAG(6); }
			if (RTVAL & (1 << 7)) { SET_CLIP1_FLAG(7); }
			break;
	}
}

static void cfunc_ctc2(void *param)
{
	((rsp_cop2 *)param)->ctc2();
}


/***************************************************************************
    COP2 Opcode Compilation
***************************************************************************/

int rsp_cop2_simd::generate_cop2(drcuml_block *block, rsp_device::compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:  /* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_mfc2, this);             // callc   mfc2
			}
			return TRUE;

		case 0x02:  /* CFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&m_op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_cfc2, this);             // callc   cfc2
			}
			return TRUE;

		case 0x04:  /* MTCz */
			UML_MOV(block, mem(&m_op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_mtc2, this);             // callc   mtc2
			return TRUE;

		case 0x06:  /* CTCz */
			UML_MOV(block, mem(&m_op), desc->opptr.l[0]);   // mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_ctc2, this);             // callc   ctc2
			return TRUE;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return generate_vector_opcode(block, compiler, desc);
	}
	return FALSE;
}
