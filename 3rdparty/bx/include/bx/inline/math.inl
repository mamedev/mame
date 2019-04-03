/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

// FPU math lib

#ifndef BX_MATH_H_HEADER_GUARD
#	error "Must be included from bx/math.h!"
#endif // BX_MATH_H_HEADER_GUARD

#include <bx/simd_t.h>

namespace bx
{
	inline BX_CONST_FUNC float toRad(float _deg)
	{
		return _deg * kPi / 180.0f;
	}

	inline BX_CONST_FUNC float toDeg(float _rad)
	{
		return _rad * 180.0f / kPi;
	}

	inline BX_CONST_FUNC uint32_t floatToBits(float _a)
	{
		union { float f; uint32_t ui; } u = { _a };
		return u.ui;
	}

	inline BX_CONST_FUNC float bitsToFloat(uint32_t _a)
	{
		union { uint32_t ui; float f; } u = { _a };
		return u.f;
	}

	inline BX_CONST_FUNC uint64_t doubleToBits(double _a)
	{
		union { double f; uint64_t ui; } u = { _a };
		return u.ui;
	}

	inline BX_CONST_FUNC double bitsToDouble(uint64_t _a)
	{
		union { uint64_t ui; double f; } u = { _a };
		return u.f;
	}

	inline BX_CONST_FUNC uint32_t floatFlip(uint32_t _value)
	{
		// Reference(s):
		// - http://archive.fo/2012.12.08-212402/http://stereopsis.com/radix.html
		//
		const uint32_t tmp0   = uint32_sra(_value, 31);
		const uint32_t tmp1   = uint32_neg(tmp0);
		const uint32_t mask   = uint32_or(tmp1, 0x80000000);
		const uint32_t result = uint32_xor(_value, mask);
		return result;
	}

	inline BX_CONST_FUNC bool isNan(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp > UINT32_C(0x7f800000);
	}

	inline BX_CONST_FUNC bool isNan(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp > UINT64_C(0x7ff0000000000000);
	}

	inline BX_CONST_FUNC bool isFinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp < UINT32_C(0x7f800000);
	}

	inline BX_CONST_FUNC bool isFinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp < UINT64_C(0x7ff0000000000000);
	}

	inline BX_CONST_FUNC bool isInfinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp == UINT32_C(0x7f800000);
	}

	inline BX_CONST_FUNC bool isInfinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp == UINT64_C(0x7ff0000000000000);
	}

	inline BX_CONST_FUNC float round(float _f)
	{
		return floor(_f + 0.5f);
	}

	inline BX_CONST_FUNC float ceil(float _a)
	{
		return -floor(-_a);
	}

	inline BX_CONSTEXPR_FUNC float lerp(float _a, float _b, float _t)
	{
		return _a + (_b - _a) * _t;
	}

	inline BX_CONSTEXPR_FUNC float sign(float _a)
	{
		return _a < 0.0f ? -1.0f : 1.0f;
	}

	inline BX_CONSTEXPR_FUNC float abs(float _a)
	{
		return _a < 0.0f ? -_a : _a;
	}

	inline BX_CONSTEXPR_FUNC float square(float _a)
	{
		return _a * _a;
	}

	inline BX_CONST_FUNC float sin(float _a)
	{
		return cos(_a - kPiHalf);
	}

	inline BX_CONST_FUNC float sinh(float _a)
	{
		return 0.5f*(exp(_a) - exp(-_a) );
	}

	inline BX_CONST_FUNC float asin(float _a)
	{
		return kPiHalf - acos(_a);
	}

	inline BX_CONST_FUNC float cosh(float _a)
	{
		return 0.5f*(exp(_a) + exp(-_a) );
	}

	inline BX_CONST_FUNC float tan(float _a)
	{
		return sin(_a) / cos(_a);
	}

	inline BX_CONST_FUNC float tanh(float _a)
	{
		const float tmp0   = exp(2.0f*_a);
		const float tmp1   = tmp0 - 1.0f;
		const float tmp2   = tmp0 + 1.0f;
		const float result = tmp1 / tmp2;

		return result;
	}

	inline BX_CONST_FUNC float atan(float _a)
	{
		return atan2(_a, 1.0f);
	}

	inline BX_CONST_FUNC float pow(float _a, float _b)
	{
		return exp(_b * log(_a) );
	}

	inline BX_CONST_FUNC float exp2(float _a)
	{
		return pow(2.0f, _a);
	}

	inline BX_CONST_FUNC float log2(float _a)
	{
		return log(_a) * kInvLogNat2;
	}

	inline BX_CONST_FUNC float rsqrtRef(float _a)
	{
		return pow(_a, -0.5f);
	}

	inline BX_CONST_FUNC float sqrtRef(float _a)
	{
		if (_a < kNearZero)
		{
			return 0.0f;
		}

		return 1.0f/rsqrtRef(_a);
	}

	inline BX_CONST_FUNC float sqrtSimd(float _a)
	{
		const simd128_t aa    = simd_splat(_a);
		const simd128_t sqrta = simd_sqrt(aa);
		float result;
		simd_stx(&result, sqrta);

		return result;
	}

	inline BX_CONST_FUNC float sqrt(float _a)
	{
#if BX_CONFIG_SUPPORTS_SIMD
		return sqrtSimd(_a);
#else
		return sqrtRef(_a);
#endif // BX_CONFIG_SUPPORTS_SIMD
	}

	inline BX_CONST_FUNC float rsqrtSimd(float _a)
	{
		if (_a < kNearZero)
		{
			return 0.0f;
		}

		const simd128_t aa     = simd_splat(_a);
		const simd128_t rsqrta = simd_rsqrt_nr(aa);
		float result;
		simd_stx(&result, rsqrta);

		return result;
	}

	inline BX_CONST_FUNC float rsqrt(float _a)
	{
#if BX_CONFIG_SUPPORTS_SIMD
		return rsqrtSimd(_a);
#else
		return rsqrtRef(_a);
#endif // BX_CONFIG_SUPPORTS_SIMD
	}

	inline BX_CONSTEXPR_FUNC float trunc(float _a)
	{
		return float(int(_a) );
	}

	inline BX_CONSTEXPR_FUNC float fract(float _a)
	{
		return _a - trunc(_a);
	}

	inline BX_CONSTEXPR_FUNC float mad(float _a, float _b, float _c)
	{
		return _a * _b + _c;
	}

	inline BX_CONST_FUNC float mod(float _a, float _b)
	{
		return _a - _b * floor(_a / _b);
	}

	inline BX_CONSTEXPR_FUNC bool equal(float _a, float _b, float _epsilon)
	{
		// Reference(s):
		// - https://web.archive.org/web/20181103180318/http://realtimecollisiondetection.net/blog/?p=89
		//
		const float lhs = abs(_a - _b);
		const float rhs = _epsilon * max(1.0f, abs(_a), abs(_b) );
		return lhs <= rhs;
	}

	inline BX_CONST_FUNC bool equal(const float* _a, const float* _b, uint32_t _num, float _epsilon)
	{
		bool result = equal(_a[0], _b[0], _epsilon);
		for (uint32_t ii = 1; result && ii < _num; ++ii)
		{
			result = equal(_a[ii], _b[ii], _epsilon);
		}
		return result;
	}

	inline BX_CONST_FUNC float wrap(float _a, float _wrap)
	{
		const float tmp0   = mod(_a, _wrap);
		const float result = tmp0 < 0.0f ? _wrap + tmp0 : tmp0;
		return result;
	}

	inline BX_CONSTEXPR_FUNC float step(float _edge, float _a)
	{
		return _a < _edge ? 0.0f : 1.0f;
	}

	inline BX_CONSTEXPR_FUNC float pulse(float _a, float _start, float _end)
	{
		return step(_a, _start) - step(_a, _end);
	}

	inline BX_CONSTEXPR_FUNC float smoothStep(float _a)
	{
		return square(_a)*(3.0f - 2.0f*_a);
	}

	inline BX_CONSTEXPR_FUNC float bias(float _time, float _bias)
	{
		return _time / ( ( (1.0f/_bias - 2.0f)*(1.0f - _time) ) + 1.0f);
	}

	inline BX_CONSTEXPR_FUNC float gain(float _time, float _gain)
	{
		// Reference(s):
		// - Bias And Gain Are Your Friend
		//   https://web.archive.org/web/20181126040535/https://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
		//   https://web.archive.org/web/20181126040558/http://demofox.org/biasgain.html
		//
		if (_time < 0.5f)
		{
			return bias(_time * 2.0f, _gain) * 0.5f;
		}

		return bias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
	}

	inline BX_CONST_FUNC float angleDiff(float _a, float _b)
	{
		const float dist = wrap(_b - _a, kPi2);
		return wrap(dist*2.0f, kPi2) - dist;
	}

	inline BX_CONST_FUNC float angleLerp(float _a, float _b, float _t)
	{
		return _a + angleDiff(_a, _b) * _t;
	}

	inline Vec3 load(const void* _ptr)
	{
		const float* ptr = reinterpret_cast<const float*>(_ptr);
		return
		{
			ptr[0],
			ptr[1],
			ptr[2],
		};
	}

	inline void store(void* _ptr, const Vec3& _a)
	{
		float* ptr = reinterpret_cast<float*>(_ptr);
		ptr[0] = _a.x;
		ptr[1] = _a.y;
		ptr[2] = _a.z;
	}

	inline BX_CONSTEXPR_FUNC Vec3 abs(const Vec3&  _a)
	{
		return
		{
			abs(_a.x),
			abs(_a.y),
			abs(_a.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 neg(const Vec3&  _a)
	{
		return
		{
			-_a.x,
			-_a.y,
			-_a.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 add(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			_a.x + _b.x,
			_a.y + _b.y,
			_a.z + _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 add(const Vec3&  _a, float _b)
	{
		return
		{
			_a.x + _b,
			_a.y + _b,
			_a.z + _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 sub(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			_a.x - _b.x,
			_a.y - _b.y,
			_a.z - _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 sub(const Vec3&  _a, float _b)
	{
		return
		{
			_a.x - _b,
			_a.y - _b,
			_a.z - _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mul(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			_a.x * _b.x,
			_a.y * _b.y,
			_a.z * _b.z,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mul(const Vec3&  _a, float _b)
	{
		return
		{
			_a.x * _b,
			_a.y * _b,
			_a.z * _b,
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 mad(const Vec3& _a, const Vec3& _b, const Vec3& _c)
	{
		return add(mul(_a, _b), _c);
	}

	inline BX_CONSTEXPR_FUNC float dot(const Vec3&  _a, const Vec3&  _b)
	{
		return _a.x*_b.x + _a.y*_b.y + _a.z*_b.z;
	}

	inline BX_CONSTEXPR_FUNC Vec3 cross(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			_a.y*_b.z - _a.z*_b.y,
			_a.z*_b.x - _a.x*_b.z,
			_a.x*_b.y - _a.y*_b.x,
		};
	}

	inline BX_CONST_FUNC float length(const Vec3&  _a)
	{
		return sqrt(dot(_a, _a) );
	}

	inline BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3&  _a, const Vec3&  _b, float _t)
	{
		return
		{
			lerp(_a.x, _b.x, _t),
			lerp(_a.y, _b.y, _t),
			lerp(_a.z, _b.z, _t),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3&  _a, const Vec3&  _b, const Vec3&  _t)
	{
		return
		{
			lerp(_a.x, _b.x, _t.x),
			lerp(_a.y, _b.y, _t.y),
			lerp(_a.z, _b.z, _t.z),
		};
	}

	inline BX_CONST_FUNC Vec3 normalize(const Vec3&  _a)
	{
		const float invLen = 1.0f/length(_a);
		const Vec3  result = mul(_a, invLen);
		return result;
	}

	inline BX_CONSTEXPR_FUNC Vec3 min(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			min(_a.x, _b.x),
			min(_a.y, _b.y),
			min(_a.z, _b.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 max(const Vec3&  _a, const Vec3&  _b)
	{
		return
		{
			max(_a.x, _b.x),
			max(_a.y, _b.y),
			max(_a.z, _b.z),
		};
	}

	inline BX_CONSTEXPR_FUNC Vec3 rcp(const Vec3&  _a)
	{
		return
		{
			1.0f / _a.x,
			1.0f / _a.y,
			1.0f / _a.z,
		};
	}

	inline void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3& _n)
	{
		const float nx = _n.x;
		const float ny = _n.y;
		const float nz = _n.z;

		if (abs(nx) > abs(nz) )
		{
			float invLen = 1.0f / sqrt(nx*nx + nz*nz);
			_outT.x = -nz * invLen;
			_outT.y =  0.0f;
			_outT.z =  nx * invLen;
		}
		else
		{
			float invLen = 1.0f / sqrt(ny*ny + nz*nz);
			_outT.x =  0.0f;
			_outT.y =  nz * invLen;
			_outT.z = -ny * invLen;
		}

		_outB = cross(_n, _outT);
	}

	inline void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3& _n, float _angle)
	{
		calcTangentFrame(_outT, _outB, _n);

		const float sa = sin(_angle);
		const float ca = cos(_angle);

		_outT.x = -sa * _outB.x + ca * _outT.x;
		_outT.y = -sa * _outB.y + ca * _outT.y;
		_outT.z = -sa * _outB.z + ca * _outT.z;

		_outB = cross(_n, _outT);
	}

	inline BX_CONST_FUNC Vec3 fromLatLong(float _u, float _v)
	{
		Vec3 result;
		const float phi   = _u * kPi2;
		const float theta = _v * kPi;

		const float st = sin(theta);
		const float sp = sin(phi);
		const float ct = cos(theta);
		const float cp = cos(phi);

		result.x = -st*sp;
		result.y =  ct;
		result.z = -st*cp;
		return result;
	}

	inline void toLatLong(float* _outU, float* _outV, const Vec3&  _dir)
	{
		const float phi   = atan2(_dir.x, _dir.z);
		const float theta = acos(_dir.y);

		*_outU = (bx::kPi + phi)/bx::kPi2;
		*_outV = theta*bx::kInvPi;
	}

	inline void vec3Add(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] + _b[0];
		_result[1] = _a[1] + _b[1];
		_result[2] = _a[2] + _b[2];
	}

	inline void vec3Sub(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] - _b[0];
		_result[1] = _a[1] - _b[1];
		_result[2] = _a[2] - _b[2];
	}

	inline void vec3Mul(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] * _b[0];
		_result[1] = _a[1] * _b[1];
		_result[2] = _a[2] * _b[2];
	}

	inline void vec3Mul(float* _result, const float* _a, float _b)
	{
		_result[0] = _a[0] * _b;
		_result[1] = _a[1] * _b;
		_result[2] = _a[2] * _b;
	}

	inline float vec3Dot(const float* _a, const float* _b)
	{
		return _a[0]*_b[0] + _a[1]*_b[1] + _a[2]*_b[2];
	}

	inline void vec3Cross(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[1]*_b[2] - _a[2]*_b[1];
		_result[1] = _a[2]*_b[0] - _a[0]*_b[2];
		_result[2] = _a[0]*_b[1] - _a[1]*_b[0];
	}

	inline float vec3Length(const float* _a)
	{
		return sqrt(vec3Dot(_a, _a) );
	}

	inline float vec3Norm(float* _result, const float* _a)
	{
		const float len = vec3Length(_a);
		const float invLen = 1.0f/len;
		_result[0] = _a[0] * invLen;
		_result[1] = _a[1] * invLen;
		_result[2] = _a[2] * invLen;
		return len;
	}

	inline void vec3TangentFrame(const float* _n, float* _t, float* _b)
	{
		const float nx = _n[0];
		const float ny = _n[1];
		const float nz = _n[2];

		if (abs(nx) > abs(nz) )
		{
			float invLen = 1.0f / sqrt(nx*nx + nz*nz);
			_t[0] = -nz * invLen;
			_t[1] =  0.0f;
			_t[2] =  nx * invLen;
		}
		else
		{
			float invLen = 1.0f / sqrt(ny*ny + nz*nz);
			_t[0] =  0.0f;
			_t[1] =  nz * invLen;
			_t[2] = -ny * invLen;
		}

		vec3Cross(_b, _n, _t);
	}

	inline void vec3TangentFrame(const float* _n, float* _t, float* _b, float _angle)
	{
		vec3TangentFrame(_n, _t, _b);

		const float sa = sin(_angle);
		const float ca = cos(_angle);

		_t[0] = -sa * _b[0] + ca * _t[0];
		_t[1] = -sa * _b[1] + ca * _t[1];
		_t[2] = -sa * _b[2] + ca * _t[2];

		vec3Cross(_b, _n, _t);
	}

	inline void vec3FromLatLong(float* _vec, float _u, float _v)
	{
		const float phi   = _u * kPi2;
		const float theta = _v * kPi;

		const float st = sin(theta);
		const float sp = sin(phi);
		const float ct = cos(theta);
		const float cp = cos(phi);

		_vec[0] = -st*sp;
		_vec[1] =  ct;
		_vec[2] = -st*cp;
	}

	inline void vec3ToLatLong(float* _outU, float* _outV, const float* _dir)
	{
		const float phi   = atan2(_dir[0], _dir[2]);
		const float theta = acos(_dir[1]);

		*_outU = (bx::kPi + phi)/bx::kPi2;
		*_outV = theta*bx::kInvPi;
	}

	inline void quatIdentity(float* _result)
	{
		_result[0] = 0.0f;
		_result[1] = 0.0f;
		_result[2] = 0.0f;
		_result[3] = 1.0f;
	}

	inline void quatMove(float* _result, const float* _a)
	{
		_result[0] = _a[0];
		_result[1] = _a[1];
		_result[2] = _a[2];
		_result[3] = _a[3];
	}

	inline void quatMulXYZ(float* _result, const float* _qa, const float* _qb)
	{
		const float ax = _qa[0];
		const float ay = _qa[1];
		const float az = _qa[2];
		const float aw = _qa[3];

		const float bx = _qb[0];
		const float by = _qb[1];
		const float bz = _qb[2];
		const float bw = _qb[3];

		_result[0] = aw * bx + ax * bw + ay * bz - az * by;
		_result[1] = aw * by - ax * bz + ay * bw + az * bx;
		_result[2] = aw * bz + ax * by - ay * bx + az * bw;
	}

	inline void quatMul(float* _result, const float* _qa, const float* _qb)
	{
		const float ax = _qa[0];
		const float ay = _qa[1];
		const float az = _qa[2];
		const float aw = _qa[3];

		const float bx = _qb[0];
		const float by = _qb[1];
		const float bz = _qb[2];
		const float bw = _qb[3];

		_result[0] = aw * bx + ax * bw + ay * bz - az * by;
		_result[1] = aw * by - ax * bz + ay * bw + az * bx;
		_result[2] = aw * bz + ax * by - ay * bx + az * bw;
		_result[3] = aw * bw - ax * bx - ay * by - az * bz;
	}

	inline void quatInvert(float* _result, const float* _quat)
	{
		_result[0] = -_quat[0];
		_result[1] = -_quat[1];
		_result[2] = -_quat[2];
		_result[3] =  _quat[3];
	}

	inline float quatDot(const float* _a, const float* _b)
	{
		return _a[0]*_b[0]
			 + _a[1]*_b[1]
			 + _a[2]*_b[2]
			 + _a[3]*_b[3]
			 ;
	}

	inline void quatNorm(float* _result, const float* _quat)
	{
		const float norm = quatDot(_quat, _quat);
		if (0.0f < norm)
		{
			const float invNorm = 1.0f / sqrt(norm);
			_result[0] = _quat[0] * invNorm;
			_result[1] = _quat[1] * invNorm;
			_result[2] = _quat[2] * invNorm;
			_result[3] = _quat[3] * invNorm;
		}
		else
		{
			quatIdentity(_result);
		}
	}

	inline void quatToEuler(float* _result, const float* _quat)
	{
		const float x = _quat[0];
		const float y = _quat[1];
		const float z = _quat[2];
		const float w = _quat[3];

		const float yy = y * y;
		const float zz = z * z;

		const float xx = x * x;
		_result[0] = atan2(2.0f * (x * w - y * z), 1.0f - 2.0f * (xx + zz) );
		_result[1] = atan2(2.0f * (y * w + x * z), 1.0f - 2.0f * (yy + zz) );
		_result[2] = asin (2.0f * (x * y + z * w) );
	}

	inline void quatRotateAxis(float* _result, const float* _axis, float _angle)
	{
		const float ha = _angle * 0.5f;
		const float ca = cos(ha);
		const float sa = sin(ha);
		_result[0] = _axis[0] * sa;
		_result[1] = _axis[1] * sa;
		_result[2] = _axis[2] * sa;
		_result[3] = ca;
	}

	inline void quatRotateX(float* _result, float _ax)
	{
		const float hx = _ax * 0.5f;
		const float cx = cos(hx);
		const float sx = sin(hx);
		_result[0] = sx;
		_result[1] = 0.0f;
		_result[2] = 0.0f;
		_result[3] = cx;
	}

	inline void quatRotateY(float* _result, float _ay)
	{
		const float hy = _ay * 0.5f;
		const float cy = cos(hy);
		const float sy = sin(hy);
		_result[0] = 0.0f;
		_result[1] = sy;
		_result[2] = 0.0f;
		_result[3] = cy;
	}

	inline void quatRotateZ(float* _result, float _az)
	{
		const float hz = _az * 0.5f;
		const float cz = cos(hz);
		const float sz = sin(hz);
		_result[0] = 0.0f;
		_result[1] = 0.0f;
		_result[2] = sz;
		_result[3] = cz;
	}

	inline void vec3MulQuat(float* _result, const float* _vec, const float* _quat)
	{
		float tmp0[4];
		quatInvert(tmp0, _quat);

		float qv[4];
		qv[0] = _vec[0];
		qv[1] = _vec[1];
		qv[2] = _vec[2];
		qv[3] = 0.0f;

		float tmp1[4];
		quatMul(tmp1, tmp0, qv);

		quatMulXYZ(_result, tmp1, _quat);
	}

	inline void mtxIdentity(float* _result)
	{
		memSet(_result, 0, sizeof(float)*16);
		_result[0] = _result[5] = _result[10] = _result[15] = 1.0f;
	}

	inline void mtxTranslate(float* _result, float _tx, float _ty, float _tz)
	{
		mtxIdentity(_result);
		_result[12] = _tx;
		_result[13] = _ty;
		_result[14] = _tz;
	}

	inline void mtxScale(float* _result, float _sx, float _sy, float _sz)
	{
		memSet(_result, 0, sizeof(float) * 16);
		_result[0]  = _sx;
		_result[5]  = _sy;
		_result[10] = _sz;
		_result[15] = 1.0f;
	}

	inline void mtxScale(float* _result, float _scale)
	{
		mtxScale(_result, _scale, _scale, _scale);
	}

	inline void mtxFromNormal(float* _result, const float* _normal, float _scale, const float* _pos)
	{
		float tangent[3];
		float bitangent[3];
		vec3TangentFrame(_normal, tangent, bitangent);

		vec3Mul(&_result[ 0], bitangent, _scale);
		vec3Mul(&_result[ 4], _normal,   _scale);
		vec3Mul(&_result[ 8], tangent,   _scale);

		_result[ 3] = 0.0f;
		_result[ 7] = 0.0f;
		_result[11] = 0.0f;
		_result[12] = _pos[0];
		_result[13] = _pos[1];
		_result[14] = _pos[2];
		_result[15] = 1.0f;
	}

	inline void mtxFromNormal(float* _result, const float* _normal, float _scale, const float* _pos, float _angle)
	{
		float tangent[3];
		float bitangent[3];
		vec3TangentFrame(_normal, tangent, bitangent, _angle);

		vec3Mul(&_result[ 0], bitangent, _scale);
		vec3Mul(&_result[ 4], _normal,   _scale);
		vec3Mul(&_result[ 8], tangent,   _scale);

		_result[ 3] = 0.0f;
		_result[ 7] = 0.0f;
		_result[11] = 0.0f;
		_result[12] = _pos[0];
		_result[13] = _pos[1];
		_result[14] = _pos[2];
		_result[15] = 1.0f;
	}

	inline void mtxQuat(float* _result, const float* _quat)
	{
		const float x = _quat[0];
		const float y = _quat[1];
		const float z = _quat[2];
		const float w = _quat[3];

		const float x2  =  x + x;
		const float y2  =  y + y;
		const float z2  =  z + z;
		const float x2x = x2 * x;
		const float x2y = x2 * y;
		const float x2z = x2 * z;
		const float x2w = x2 * w;
		const float y2y = y2 * y;
		const float y2z = y2 * z;
		const float y2w = y2 * w;
		const float z2z = z2 * z;
		const float z2w = z2 * w;

		_result[ 0] = 1.0f - (y2y + z2z);
		_result[ 1] =         x2y - z2w;
		_result[ 2] =         x2z + y2w;
		_result[ 3] = 0.0f;

		_result[ 4] =         x2y + z2w;
		_result[ 5] = 1.0f - (x2x + z2z);
		_result[ 6] =         y2z - x2w;
		_result[ 7] = 0.0f;

		_result[ 8] =         x2z - y2w;
		_result[ 9] =         y2z + x2w;
		_result[10] = 1.0f - (x2x + y2y);
		_result[11] = 0.0f;

		_result[12] = 0.0f;
		_result[13] = 0.0f;
		_result[14] = 0.0f;
		_result[15] = 1.0f;
	}

	inline void mtxQuatTranslation(float* _result, const float* _quat, const float* _translation)
	{
		mtxQuat(_result, _quat);
		_result[12] = -(_result[0]*_translation[0] + _result[4]*_translation[1] + _result[ 8]*_translation[2]);
		_result[13] = -(_result[1]*_translation[0] + _result[5]*_translation[1] + _result[ 9]*_translation[2]);
		_result[14] = -(_result[2]*_translation[0] + _result[6]*_translation[1] + _result[10]*_translation[2]);
	}

	inline void mtxQuatTranslationHMD(float* _result, const float* _quat, const float* _translation)
	{
		float quat[4];
		quat[0] = -_quat[0];
		quat[1] = -_quat[1];
		quat[2] =  _quat[2];
		quat[3] =  _quat[3];
		mtxQuatTranslation(_result, quat, _translation);
	}

	inline Vec3 mul(const Vec3& _vec, const float* _mat)
	{
		Vec3 result;
		result.x = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8] + _mat[12];
		result.y = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9] + _mat[13];
		result.z = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14];
		return result;
	}

	inline Vec3 mulXyz0(const Vec3& _vec, const float* _mat)
	{
		Vec3 result;
		result.x = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8];
		result.y = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9];
		result.z = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10];
		return result;
	}

	inline Vec3 mulH(const Vec3& _vec, const float* _mat)
	{
		const float xx   = _vec.x * _mat[0] + _vec.y * _mat[4] + _vec.z * _mat[ 8] + _mat[12];
		const float yy   = _vec.x * _mat[1] + _vec.y * _mat[5] + _vec.z * _mat[ 9] + _mat[13];
		const float zz   = _vec.x * _mat[2] + _vec.y * _mat[6] + _vec.z * _mat[10] + _mat[14];
		const float ww   = _vec.x * _mat[3] + _vec.y * _mat[7] + _vec.z * _mat[11] + _mat[15];
		const float invW = sign(ww) / ww;

		Vec3 result =
		{
			xx * invW,
			yy * invW,
			zz * invW,
		};

		return result;
	}

	inline void vec3MulMtx(float* _result, const float* _vec, const float* _mat)
	{
		_result[0] = _vec[0] * _mat[ 0] + _vec[1] * _mat[4] + _vec[2] * _mat[ 8] + _mat[12];
		_result[1] = _vec[0] * _mat[ 1] + _vec[1] * _mat[5] + _vec[2] * _mat[ 9] + _mat[13];
		_result[2] = _vec[0] * _mat[ 2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _mat[14];
	}

	inline void vec3MulMtxXyz0(float* _result, const float* _vec, const float* _mat)
	{
		_result[0] = _vec[0] * _mat[ 0] + _vec[1] * _mat[4] + _vec[2] * _mat[ 8];
		_result[1] = _vec[0] * _mat[ 1] + _vec[1] * _mat[5] + _vec[2] * _mat[ 9];
		_result[2] = _vec[0] * _mat[ 2] + _vec[1] * _mat[6] + _vec[2] * _mat[10];
	}

	inline void vec3MulMtxH(float* _result, const float* _vec, const float* _mat)
	{
		float xx = _vec[0] * _mat[ 0] + _vec[1] * _mat[4] + _vec[2] * _mat[ 8] + _mat[12];
		float yy = _vec[0] * _mat[ 1] + _vec[1] * _mat[5] + _vec[2] * _mat[ 9] + _mat[13];
		float zz = _vec[0] * _mat[ 2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _mat[14];
		float ww = _vec[0] * _mat[ 3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _mat[15];
		float invW = sign(ww)/ww;
		_result[0] = xx*invW;
		_result[1] = yy*invW;
		_result[2] = zz*invW;
	}

	inline void vec4Mul(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] * _b[0];
		_result[1] = _a[1] * _b[1];
		_result[2] = _a[2] * _b[2];
		_result[3] = _a[3] * _b[3];
	}

	inline void vec4Mul(float* _result, const float* _a, float _b)
	{
		_result[0] = _a[0] * _b;
		_result[1] = _a[1] * _b;
		_result[2] = _a[2] * _b;
		_result[3] = _a[3] * _b;
	}

	inline void vec4MulMtx(float* _result, const float* _vec, const float* _mat)
	{
		_result[0] = _vec[0] * _mat[ 0] + _vec[1] * _mat[4] + _vec[2] * _mat[ 8] + _vec[3] * _mat[12];
		_result[1] = _vec[0] * _mat[ 1] + _vec[1] * _mat[5] + _vec[2] * _mat[ 9] + _vec[3] * _mat[13];
		_result[2] = _vec[0] * _mat[ 2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _vec[3] * _mat[14];
		_result[3] = _vec[0] * _mat[ 3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _vec[3] * _mat[15];
	}

	inline void mtxMul(float* _result, const float* _a, const float* _b)
	{
		vec4MulMtx(&_result[ 0], &_a[ 0], _b);
		vec4MulMtx(&_result[ 4], &_a[ 4], _b);
		vec4MulMtx(&_result[ 8], &_a[ 8], _b);
		vec4MulMtx(&_result[12], &_a[12], _b);
	}

	inline void mtxTranspose(float* _result, const float* _a)
	{
		_result[ 0] = _a[ 0];
		_result[ 4] = _a[ 1];
		_result[ 8] = _a[ 2];
		_result[12] = _a[ 3];
		_result[ 1] = _a[ 4];
		_result[ 5] = _a[ 5];
		_result[ 9] = _a[ 6];
		_result[13] = _a[ 7];
		_result[ 2] = _a[ 8];
		_result[ 6] = _a[ 9];
		_result[10] = _a[10];
		_result[14] = _a[11];
		_result[ 3] = _a[12];
		_result[ 7] = _a[13];
		_result[11] = _a[14];
		_result[15] = _a[15];
	}

	/// Convert LH to RH projection matrix and vice versa.
	inline void mtxProjFlipHandedness(float* _dst, const float* _src)
	{
		_dst[ 0] = -_src[ 0];
		_dst[ 1] = -_src[ 1];
		_dst[ 2] = -_src[ 2];
		_dst[ 3] = -_src[ 3];
		_dst[ 4] =  _src[ 4];
		_dst[ 5] =  _src[ 5];
		_dst[ 6] =  _src[ 6];
		_dst[ 7] =  _src[ 7];
		_dst[ 8] = -_src[ 8];
		_dst[ 9] = -_src[ 9];
		_dst[10] = -_src[10];
		_dst[11] = -_src[11];
		_dst[12] =  _src[12];
		_dst[13] =  _src[13];
		_dst[14] =  _src[14];
		_dst[15] =  _src[15];
	}

	/// Convert LH to RH view matrix and vice versa.
	inline void mtxViewFlipHandedness(float* _dst, const float* _src)
	{
		_dst[ 0] = -_src[ 0];
		_dst[ 1] =  _src[ 1];
		_dst[ 2] = -_src[ 2];
		_dst[ 3] =  _src[ 3];
		_dst[ 4] = -_src[ 4];
		_dst[ 5] =  _src[ 5];
		_dst[ 6] = -_src[ 6];
		_dst[ 7] =  _src[ 7];
		_dst[ 8] = -_src[ 8];
		_dst[ 9] =  _src[ 9];
		_dst[10] = -_src[10];
		_dst[11] =  _src[11];
		_dst[12] = -_src[12];
		_dst[13] =  _src[13];
		_dst[14] = -_src[14];
		_dst[15] =  _src[15];
	}

	inline void calcNormal(float _result[3], const float _va[3], const float _vb[3], const float _vc[3])
	{
		float ba[3];
		vec3Sub(ba, _vb, _va);

		float ca[3];
		vec3Sub(ca, _vc, _va);

		float baxca[3];
		vec3Cross(baxca, ba, ca);

		vec3Norm(_result, baxca);
	}

	inline void calcPlane(float _result[4], const float _va[3], const float _vb[3], const float _vc[3])
	{
		float normal[3];
		calcNormal(normal, _va, _vb, _vc);
		calcPlane(_result, normal, _va);
	}

	inline void calcPlane(float _result[4], const float _normal[3], const float _pos[3])
	{
		_result[0] = _normal[0];
		_result[1] = _normal[1];
		_result[2] = _normal[2];
		_result[3] = -vec3Dot(_normal, _pos);
	}

	inline BX_CONST_FUNC float toLinear(float _a)
	{
		const float lo     = _a / 12.92f;
		const float hi     = pow( (_a + 0.055f) / 1.055f, 2.4f);
		const float result = lerp(hi, lo, _a <= 0.04045f);
		return result;
	}

	inline BX_CONST_FUNC float toGamma(float _a)
	{
		const float lo     = _a * 12.92f;
		const float hi     = pow(abs(_a), 1.0f/2.4f) * 1.055f - 0.055f;
		const float result = lerp(hi, lo, _a <= 0.0031308f);
		return result;
	}

} // namespace bx
