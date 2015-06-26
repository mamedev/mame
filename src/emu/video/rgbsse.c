// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Ryan Holtz
/***************************************************************************

    rgbsse.c

    SSE optimized RGB utilities.

    WARNING: This code assumes SSE2 or greater capability.

***************************************************************************/

#if (!defined(MAME_DEBUG) || defined(__OPTIMIZE__)) && (defined(__SSE2__) || defined(_MSC_VER)) && defined(PTR64)

#include "emu.h"
#include <emmintrin.h>
#include "rgbsse.h"

/***************************************************************************
    HIGHER LEVEL OPERATIONS
***************************************************************************/

void rgbaint_t::blend(const rgbaint_t& other, UINT8 factor)
{
	m_value = _mm_unpacklo_epi16(m_value, other.m_value);
	m_value = _mm_madd_epi16(m_value, *(__m128i *)&rgbsse_statics.scale_table[factor][0]);
	m_value = _mm_srli_epi32(m_value, 8);
}

void rgbaint_t::scale_and_clamp(const rgbaint_t& scale)
{
	mul(scale);
	sra(8);
	max(0);
	min(255);
}

void rgbaint_t::scale_imm_and_clamp(const INT32 scale)
{
	mul_imm(scale);
	sra(8);
	max(0);
	min(255);
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other, const rgbaint_t& scale2)
{
	rgbaint_t color2(other);
	color2.mul(scale2);

	mul(scale);
	add(color2);
	sra(8);
	max(0);
	min(255);
}

void rgbaint_t::scale_imm_add_and_clamp(const INT32 scale, const rgbaint_t& other)
{
	mul_imm(scale);
	sra(8);
	add(other);
	max(0);
	min(255);
}

void rgbaint_t::scale_add_and_clamp(const rgbaint_t& scale, const rgbaint_t& other)
{
	mul(scale);
	sra(8);
	add(other);
	max(0);
	min(255);
}

#endif // defined(__SSE2__) || defined(_MSC_VER)
