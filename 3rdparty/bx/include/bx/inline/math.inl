/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

// FPU math lib

#ifndef BX_FPU_MATH_H_HEADER_GUARD
#	error "Must be included from bx/fpumath.h!"
#endif // BX_FPU_MATH_H_HEADER_GUARD

namespace bx
{
	inline float toRad(float _deg)
	{
		return _deg * kPi / 180.0f;
	}

	inline float toDeg(float _rad)
	{
		return _rad * 180.0f / kPi;
	}

	inline uint32_t floatToBits(float _a)
	{
		union { float f; uint32_t ui; } u = { _a };
		return u.ui;
	}

	inline float bitsToFloat(uint32_t _a)
	{
		union { uint32_t ui; float f; } u = { _a };
		return u.f;
	}

	inline uint64_t doubleToBits(double _a)
	{
		union { double f; uint64_t ui; } u = { _a };
		return u.ui;
	}

	inline double bitsToDouble(uint64_t _a)
	{
		union { uint64_t ui; double f; } u = { _a };
		return u.f;
	}

	inline uint32_t floatFlip(uint32_t _value)
	{
		// Reference:
		// http://archive.fo/2012.12.08-212402/http://stereopsis.com/radix.html
		const uint32_t tmp0   = uint32_sra(_value, 31);
		const uint32_t tmp1   = uint32_neg(tmp0);
		const uint32_t mask   = uint32_or(tmp1, 0x80000000);
		const uint32_t result = uint32_xor(_value, mask);
		return result;
	}

	inline bool isNan(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp > UINT32_C(0x7f800000);
	}

	inline bool isNan(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp > UINT64_C(0x7ff0000000000000);
	}

	inline bool isFinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp < UINT32_C(0x7f800000);
	}

	inline bool isFinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp < UINT64_C(0x7ff0000000000000);
	}

	inline bool isInfinite(float _f)
	{
		const uint32_t tmp = floatToBits(_f) & INT32_MAX;
		return tmp == UINT32_C(0x7f800000);
	}

	inline bool isInfinite(double _f)
	{
		const uint64_t tmp = doubleToBits(_f) & INT64_MAX;
		return tmp == UINT64_C(0x7ff0000000000000);
	}

	inline float fround(float _f)
	{
		return ffloor(_f + 0.5f);
	}

	inline float fmin(float _a, float _b)
	{
		return _a < _b ? _a : _b;
	}

	inline float fmax(float _a, float _b)
	{
		return _a > _b ? _a : _b;
	}

	inline float fmin3(float _a, float _b, float _c)
	{
		return fmin(_a, fmin(_b, _c) );
	}

	inline float fmax3(float _a, float _b, float _c)
	{
		return fmax(_a, fmax(_b, _c) );
	}

	inline float fclamp(float _a, float _min, float _max)
	{
		return fmin(fmax(_a, _min), _max);
	}

	inline float fsaturate(float _a)
	{
		return fclamp(_a, 0.0f, 1.0f);
	}

	inline float flerp(float _a, float _b, float _t)
	{
		return _a + (_b - _a) * _t;
	}

	inline float fsign(float _a)
	{
		return _a < 0.0f ? -1.0f : 1.0f;
	}

	inline float fsq(float _a)
	{
		return _a * _a;
	}

	inline float fexp2(float _a)
	{
		return fpow(2.0f, _a);
	}

	inline float flog2(float _a)
	{
		return flog(_a) * kInvLogNat2;
	}

	inline float frsqrt(float _a)
	{
		return 1.0f/fsqrt(_a);
	}

	inline float ffract(float _a)
	{
		return _a - ffloor(_a);
	}

	inline bool fequal(float _a, float _b, float _epsilon)
	{
		// http://realtimecollisiondetection.net/blog/?p=89
		const float lhs = fabs(_a - _b);
		const float rhs = _epsilon * fmax3(1.0f, fabs(_a), fabs(_b) );
		return lhs <= rhs;
	}

	inline bool fequal(const float* _a, const float* _b, uint32_t _num, float _epsilon)
	{
		bool equal = fequal(_a[0], _b[0], _epsilon);
		for (uint32_t ii = 1; equal && ii < _num; ++ii)
		{
			equal = fequal(_a[ii], _b[ii], _epsilon);
		}
		return equal;
	}

	inline float fwrap(float _a, float _wrap)
	{
		const float mod    = fmod(_a, _wrap);
		const float result = mod < 0.0f ? _wrap + mod : mod;
		return result;
	}

	inline float fstep(float _edge, float _a)
	{
		return _a < _edge ? 0.0f : 1.0f;
	}

	inline float fpulse(float _a, float _start, float _end)
	{
		return fstep(_a, _start) - fstep(_a, _end);
	}

	inline float fsmoothstep(float _a)
	{
		return fsq(_a)*(3.0f - 2.0f*_a);
	}

	inline float fbias(float _time, float _bias)
	{
		return _time / ( ( (1.0f/_bias - 2.0f)*(1.0f - _time) ) + 1.0f);
	}

	inline float fgain(float _time, float _gain)
	{
		if (_time < 0.5f)
		{
			return fbias(_time * 2.0f, _gain) * 0.5f;
		}

		return fbias(_time * 2.0f - 1.0f, 1.0f - _gain) * 0.5f + 0.5f;
	}

	inline float angleDiff(float _a, float _b)
	{
		const float dist = fwrap(_b - _a, kPi2);
		return fwrap(dist*2.0f, kPi2) - dist;
	}

	inline float angleLerp(float _a, float _b, float _t)
	{
		return _a + angleDiff(_a, _b) * _t;
	}

	inline void vec3Move(float* _result, const float* _a)
	{
		_result[0] = _a[0];
		_result[1] = _a[1];
		_result[2] = _a[2];
	}

	inline void vec3Abs(float* _result, const float* _a)
	{
		_result[0] = fabs(_a[0]);
		_result[1] = fabs(_a[1]);
		_result[2] = fabs(_a[2]);
	}

	inline void vec3Neg(float* _result, const float* _a)
	{
		_result[0] = -_a[0];
		_result[1] = -_a[1];
		_result[2] = -_a[2];
	}

	inline void vec3Add(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] + _b[0];
		_result[1] = _a[1] + _b[1];
		_result[2] = _a[2] + _b[2];
	}

	inline void vec3Add(float* _result, const float* _a, float _b)
	{
		_result[0] = _a[0] + _b;
		_result[1] = _a[1] + _b;
		_result[2] = _a[2] + _b;
	}

	inline void vec3Sub(float* _result, const float* _a, const float* _b)
	{
		_result[0] = _a[0] - _b[0];
		_result[1] = _a[1] - _b[1];
		_result[2] = _a[2] - _b[2];
	}

	inline void vec3Sub(float* _result, const float* _a, float _b)
	{
		_result[0] = _a[0] - _b;
		_result[1] = _a[1] - _b;
		_result[2] = _a[2] - _b;
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
		return fsqrt(vec3Dot(_a, _a) );
	}

	inline void vec3Lerp(float* _result, const float* _a, const float* _b, float _t)
	{
		_result[0] = flerp(_a[0], _b[0], _t);
		_result[1] = flerp(_a[1], _b[1], _t);
		_result[2] = flerp(_a[2], _b[2], _t);
	}

	inline void vec3Lerp(float* _result, const float* _a, const float* _b, const float* _c)
	{
		_result[0] = flerp(_a[0], _b[0], _c[0]);
		_result[1] = flerp(_a[1], _b[1], _c[1]);
		_result[2] = flerp(_a[2], _b[2], _c[2]);
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

	inline void vec3Min(float* _result, const float* _a, const float* _b)
	{
		_result[0] = fmin(_a[0], _b[0]);
		_result[1] = fmin(_a[1], _b[1]);
		_result[2] = fmin(_a[2], _b[2]);
	}

	inline void vec3Max(float* _result, const float* _a, const float* _b)
	{
		_result[0] = fmax(_a[0], _b[0]);
		_result[1] = fmax(_a[1], _b[1]);
		_result[2] = fmax(_a[2], _b[2]);
	}

	inline void vec3Rcp(float* _result, const float* _a)
	{
		_result[0] = 1.0f / _a[0];
		_result[1] = 1.0f / _a[1];
		_result[2] = 1.0f / _a[2];
	}

	inline void vec3TangentFrame(const float* _n, float* _t, float* _b)
	{
		const float nx = _n[0];
		const float ny = _n[1];
		const float nz = _n[2];

		if (bx::fabs(nx) > bx::fabs(nz) )
		{
			float invLen = 1.0f / bx::fsqrt(nx*nx + nz*nz);
			_t[0] = -nz * invLen;
			_t[1] =  0.0f;
			_t[2] =  nx * invLen;
		}
		else
		{
			float invLen = 1.0f / bx::fsqrt(ny*ny + nz*nz);
			_t[0] =  0.0f;
			_t[1] =  nz * invLen;
			_t[2] = -ny * invLen;
		}

		bx::vec3Cross(_b, _n, _t);
	}

	inline void vec3TangentFrame(const float* _n, float* _t, float* _b, float _angle)
	{
		vec3TangentFrame(_n, _t, _b);

		const float sa = fsin(_angle);
		const float ca = fcos(_angle);

		_t[0] = -sa * _b[0] + ca * _t[0];
		_t[1] = -sa * _b[1] + ca * _t[1];
		_t[2] = -sa * _b[2] + ca * _t[2];

		bx::vec3Cross(_b, _n, _t);
	}

	inline void vec3FromLatLong(float* _vec, float _u, float _v)
	{
		const float phi   = _u * bx::kPi2;
		const float theta = _v * bx::kPi;

		const float st = bx::fsin(theta);
		const float sp = bx::fsin(phi);
		const float ct = bx::fcos(theta);
		const float cp = bx::fcos(phi);

		_vec[0] = -st*sp;
		_vec[1] = ct;
		_vec[2] = -st*cp;
	}

	inline void vec3ToLatLong(float* _u, float* _v, const float* _vec)
	{
		const float phi   = bx::fatan2(_vec[0], _vec[2]);
		const float theta = bx::facos(_vec[1]);

		*_u = (bx::kPi + phi)*bx::kInvPi*0.5f;
		*_v = theta*bx::kInvPi;
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
			const float invNorm = 1.0f / fsqrt(norm);
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
		_result[0] = fatan2(2.0f * (x * w - y * z), 1.0f - 2.0f * (xx + zz) );
		_result[1] = fatan2(2.0f * (y * w + x * z), 1.0f - 2.0f * (yy + zz) );
		_result[2] = fasin (2.0f * (x * y + z * w) );
	}

	inline void quatRotateAxis(float* _result, const float* _axis, float _angle)
	{
		const float ha = _angle * 0.5f;
		const float ca = fcos(ha);
		const float sa = fsin(ha);
		_result[0] = _axis[0] * sa;
		_result[1] = _axis[1] * sa;
		_result[2] = _axis[2] * sa;
		_result[3] = ca;
	}

	inline void quatRotateX(float* _result, float _ax)
	{
		const float hx = _ax * 0.5f;
		const float cx = fcos(hx);
		const float sx = fsin(hx);
		_result[0] = sx;
		_result[1] = 0.0f;
		_result[2] = 0.0f;
		_result[3] = cx;
	}

	inline void quatRotateY(float* _result, float _ay)
	{
		const float hy = _ay * 0.5f;
		const float cy = fcos(hy);
		const float sy = fsin(hy);
		_result[0] = 0.0f;
		_result[1] = sy;
		_result[2] = 0.0f;
		_result[3] = cy;
	}

	inline void quatRotateZ(float* _result, float _az)
	{
		const float hz = _az * 0.5f;
		const float cz = fcos(hz);
		const float sz = fsin(hz);
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
		float invW = fsign(ww)/ww;
		_result[0] = xx*invW;
		_result[1] = yy*invW;
		_result[2] = zz*invW;
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

	inline void calcNormal(float _result[3], float _va[3], float _vb[3], float _vc[3])
	{
		float ba[3];
		vec3Sub(ba, _vb, _va);

		float ca[3];
		vec3Sub(ca, _vc, _va);

		float baxca[3];
		vec3Cross(baxca, ba, ca);

		vec3Norm(_result, baxca);
	}

	inline void calcPlane(float _result[4], float _va[3], float _vb[3], float _vc[3])
	{
		float normal[3];
		calcNormal(normal, _va, _vb, _vc);

		_result[0] = normal[0];
		_result[1] = normal[1];
		_result[2] = normal[2];
		_result[3] = -vec3Dot(normal, _va);
	}

} // namespace bx
