/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_REF_H_HEADER_GUARD
#define BX_FLOAT4_REF_H_HEADER_GUARD

#include <math.h> // sqrtf

namespace bx
{
	typedef union float4_t
	{
		float    fxyzw[4];
		int32_t  ixyzw[4];
		uint32_t uxyzw[4];

	} float4_t;

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			BX_FLOAT4_FORCE_INLINE float4_t float4_swiz_##_x##_y##_z##_w(float4_t _a) \
			{ \
				float4_t result; \
				result.ixyzw[0] = _a.ixyzw[ELEM##_x]; \
				result.ixyzw[1] = _a.ixyzw[ELEM##_y]; \
				result.ixyzw[2] = _a.ixyzw[ELEM##_z]; \
				result.ixyzw[3] = _a.ixyzw[ELEM##_w]; \
				return result; \
			}

#include "float4_swizzle.inl"

#undef IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define IMPLEMENT_TEST(_xyzw, _mask) \
			BX_FLOAT4_FORCE_INLINE bool float4_test_any_##_xyzw(float4_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return 0 != (tmp&(_mask) ); \
			} \
			\
			BX_FLOAT4_FORCE_INLINE bool float4_test_all_##_xyzw(float4_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return (_mask) == (tmp&(_mask) ); \
			}

IMPLEMENT_TEST(x    , 0x1);
IMPLEMENT_TEST(y    , 0x2);
IMPLEMENT_TEST(xy   , 0x3);
IMPLEMENT_TEST(z    , 0x4);
IMPLEMENT_TEST(xz   , 0x5);
IMPLEMENT_TEST(yz   , 0x6);
IMPLEMENT_TEST(xyz  , 0x7);
IMPLEMENT_TEST(w    , 0x8);
IMPLEMENT_TEST(xw   , 0x9);
IMPLEMENT_TEST(yw   , 0xa);
IMPLEMENT_TEST(xyw  , 0xb);
IMPLEMENT_TEST(zw   , 0xc);
IMPLEMENT_TEST(xzw  , 0xd);
IMPLEMENT_TEST(yzw  , 0xe);
IMPLEMENT_TEST(xyzw , 0xf);

#undef IMPLEMENT_TEST

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xyAB(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1];
		result.uxyzw[2] = _b.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_ABxy(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _b.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _a.uxyzw[1];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CDzw(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[3];
		result.uxyzw[2] = _a.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zwCD(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[3];
		result.uxyzw[2] = _b.uxyzw[2];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xAyB(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[0];
		result.uxyzw[2] = _a.uxyzw[1];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_yBxA(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[1];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[0];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zCwD(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[2];
		result.uxyzw[2] = _a.uxyzw[3];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CzDw(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[2];
		result.uxyzw[2] = _b.uxyzw[3];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float float4_x(float4_t _a)
	{
		return _a.fxyzw[0];
	}

	BX_FLOAT4_FORCE_INLINE float float4_y(float4_t _a)
	{
		return _a.fxyzw[1];
	}

	BX_FLOAT4_FORCE_INLINE float float4_z(float4_t _a)
	{
		return _a.fxyzw[2];
	}

	BX_FLOAT4_FORCE_INLINE float float4_w(float4_t _a)
	{
		return _a.fxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		float4_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE void float4_st(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE void float4_stx(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	BX_FLOAT4_FORCE_INLINE void float4_stream(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(float _x, float _y, float _z, float _w)
	{
		float4_t result;
		result.fxyzw[0] = _x;
		result.fxyzw[1] = _y;
		result.fxyzw[2] = _z;
		result.fxyzw[3] = _w;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		float4_t result;
		result.uxyzw[0] = _x;
		result.uxyzw[1] = _y;
		result.uxyzw[2] = _z;
		result.uxyzw[3] = _w;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		float4_t result;
		result.uxyzw[0] = val;
		result.uxyzw[1] = val;
		result.uxyzw[2] = val;
		result.uxyzw[3] = val;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(float _a)
	{
		return float4_ld(_a, _a, _a, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isplat(uint32_t _a)
	{
		return float4_ild(_a, _a, _a, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_zero()
	{
		return float4_ild(0, 0, 0, 0);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_itof(float4_t _a)
	{
		float4_t result;
		result.fxyzw[0] = (float)_a.ixyzw[0];
		result.fxyzw[1] = (float)_a.ixyzw[1];
		result.fxyzw[2] = (float)_a.ixyzw[2];
		result.fxyzw[3] = (float)_a.ixyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ftoi(float4_t _a)
	{
		float4_t result;
		result.ixyzw[0] = (int)_a.fxyzw[0];
		result.ixyzw[1] = (int)_a.fxyzw[1];
		result.ixyzw[2] = (int)_a.fxyzw[2];
		result.ixyzw[3] = (int)_a.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_round(float4_t _a)
	{
		const float4_t tmp    = float4_ftoi(_a);
		const float4_t result = float4_itof(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_add(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] + _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] + _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] + _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] + _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sub(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] - _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] - _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] - _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] - _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] * _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] * _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] * _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] * _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_div(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] / _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] / _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] / _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] / _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rcp_est(float4_t _a)
	{
		float4_t result;
		result.fxyzw[0] = 1.0f / _a.fxyzw[0];
		result.fxyzw[1] = 1.0f / _a.fxyzw[1];
		result.fxyzw[2] = 1.0f / _a.fxyzw[2];
		result.fxyzw[3] = 1.0f / _a.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sqrt(float4_t _a)
	{
		float4_t result;
		result.fxyzw[0] = sqrtf(_a.fxyzw[0]);
		result.fxyzw[1] = sqrtf(_a.fxyzw[1]);
		result.fxyzw[2] = sqrtf(_a.fxyzw[2]);
		result.fxyzw[3] = sqrtf(_a.fxyzw[3]);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rsqrt_est(float4_t _a)
	{
		float4_t result;
		result.fxyzw[0] = 1.0f / sqrtf(_a.fxyzw[0]);
		result.fxyzw[1] = 1.0f / sqrtf(_a.fxyzw[1]);
		result.fxyzw[2] = 1.0f / sqrtf(_a.fxyzw[2]);
		result.fxyzw[3] = 1.0f / sqrtf(_a.fxyzw[3]);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpeq(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.fxyzw[0] == _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] == _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] == _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] == _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmplt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmple(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.fxyzw[0] <= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] <= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] <= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] <= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpgt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpge(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.fxyzw[0] >= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] >= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] >= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] >= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_min(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_max(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.fxyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_and(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] & _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & _b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_andc(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] & ~_b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & ~_b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & ~_b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & ~_b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_or(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] | _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] | _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] | _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] | _b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_xor(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] ^ _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] ^ _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] ^ _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] ^ _b.uxyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sll(float4_t _a, int _count)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] << _count;
		result.uxyzw[1] = _a.uxyzw[1] << _count;
		result.uxyzw[2] = _a.uxyzw[2] << _count;
		result.uxyzw[3] = _a.uxyzw[3] << _count;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_srl(float4_t _a, int _count)
	{
		float4_t result;
		result.uxyzw[0] = _a.uxyzw[0] >> _count;
		result.uxyzw[1] = _a.uxyzw[1] >> _count;
		result.uxyzw[2] = _a.uxyzw[2] >> _count;
		result.uxyzw[3] = _a.uxyzw[3] >> _count;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sra(float4_t _a, int _count)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] >> _count;
		result.ixyzw[1] = _a.ixyzw[1] >> _count;
		result.ixyzw[2] = _a.ixyzw[2] >> _count;
		result.ixyzw[3] = _a.ixyzw[3] >> _count;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpeq(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] == _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] == _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] == _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] == _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmplt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpgt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_imin(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_imax(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_iadd(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] + _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] + _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] + _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] + _b.ixyzw[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isub(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.ixyzw[0] = _a.ixyzw[0] - _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] - _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] - _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] - _b.ixyzw[3];
		return result;
	}

} // namespace bx

#define float4_shuf_xAzC float4_shuf_xAzC_ni
#define float4_shuf_yBwD float4_shuf_yBwD_ni
#define float4_rcp float4_rcp_ni
#define float4_orx float4_orx_ni
#define float4_orc float4_orc_ni
#define float4_neg float4_neg_ni
#define float4_madd float4_madd_ni
#define float4_nmsub float4_nmsub_ni
#define float4_div_nr float4_div_nr_ni
#define float4_selb float4_selb_ni
#define float4_sels float4_sels_ni
#define float4_not float4_not_ni
#define float4_abs float4_abs_ni
#define float4_clamp float4_clamp_ni
#define float4_lerp float4_lerp_ni
#define float4_rsqrt float4_rsqrt_ni
#define float4_rsqrt_nr float4_rsqrt_nr_ni
#define float4_rsqrt_carmack float4_rsqrt_carmack_ni
#define float4_sqrt_nr float4_sqrt_nr_ni
#define float4_log2 float4_log2_ni
#define float4_exp2 float4_exp2_ni
#define float4_pow float4_pow_ni
#define float4_cross3 float4_cross3_ni
#define float4_normalize3 float4_normalize3_ni
#define float4_dot3 float4_dot3_ni
#define float4_dot float4_dot_ni
#define float4_ceil float4_ceil_ni
#define float4_floor float4_floor_ni
#include "float4_ni.h"

#endif // BX_FLOAT4_REF_H_HEADER_GUARD
