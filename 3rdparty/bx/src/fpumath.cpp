/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include <bx/fpumath.h>
#include <math.h>

namespace bx
{
	const float pi     = 3.14159265358979323846f;
	const float invPi  = 1.0f/3.14159265358979323846f;
	const float piHalf = 1.57079632679489661923f;
	const float sqrt2  = 1.41421356237309504880f;
#if BX_COMPILER_MSVC
	const float huge   = float(HUGE_VAL);
#else
	const float huge   = HUGE_VALF;
#endif // BX_COMPILER_MSVC

	float fabsolute(float _a)
	{
		return ::fabsf(_a);
	}

	float fsin(float _a)
	{
		return ::sinf(_a);
	}

	float fasin(float _a)
	{
		return ::asinf(_a);
	}

	float fcos(float _a)
	{
		return ::cosf(_a);
	}

	float ftan(float _a)
	{
		return ::tanf(_a);
	}

	float facos(float _a)
	{
		return ::acosf(_a);
	}

	float fatan2(float _y, float _x)
	{
		return ::atan2f(_y, _x);
	}

	float fpow(float _a, float _b)
	{
		return ::powf(_a, _b);
	}

	float flog(float _a)
	{
		return ::logf(_a);
	}

	float fsqrt(float _a)
	{
		return ::sqrtf(_a);
	}

	float ffloor(float _f)
	{
		return ::floorf(_f);
	}

	float fceil(float _f)
	{
		return ::ceilf(_f);
	}

	float fmod(float _a, float _b)
	{
		return ::fmodf(_a, _b);
	}

	void mtx3Inverse(float* _result, const float* _a)
	{
		float xx = _a[0];
		float xy = _a[1];
		float xz = _a[2];
		float yx = _a[3];
		float yy = _a[4];
		float yz = _a[5];
		float zx = _a[6];
		float zy = _a[7];
		float zz = _a[8];

		float det = 0.0f;
		det += xx * (yy*zz - yz*zy);
		det -= xy * (yx*zz - yz*zx);
		det += xz * (yx*zy - yy*zx);

		float invDet = 1.0f/det;

		_result[0] = +(yy*zz - yz*zy) * invDet;
		_result[1] = -(xy*zz - xz*zy) * invDet;
		_result[2] = +(xy*yz - xz*yy) * invDet;

		_result[3] = -(yx*zz - yz*zx) * invDet;
		_result[4] = +(xx*zz - xz*zx) * invDet;
		_result[5] = -(xx*yz - xz*yx) * invDet;

		_result[6] = +(yx*zy - yy*zx) * invDet;
		_result[7] = -(xx*zy - xy*zx) * invDet;
		_result[8] = +(xx*yy - xy*yx) * invDet;
	}

	void mtxInverse(float* _result, const float* _a)
	{
		float xx = _a[ 0];
		float xy = _a[ 1];
		float xz = _a[ 2];
		float xw = _a[ 3];
		float yx = _a[ 4];
		float yy = _a[ 5];
		float yz = _a[ 6];
		float yw = _a[ 7];
		float zx = _a[ 8];
		float zy = _a[ 9];
		float zz = _a[10];
		float zw = _a[11];
		float wx = _a[12];
		float wy = _a[13];
		float wz = _a[14];
		float ww = _a[15];

		float det = 0.0f;
		det += xx * (yy*(zz*ww - zw*wz) - yz*(zy*ww - zw*wy) + yw*(zy*wz - zz*wy) );
		det -= xy * (yx*(zz*ww - zw*wz) - yz*(zx*ww - zw*wx) + yw*(zx*wz - zz*wx) );
		det += xz * (yx*(zy*ww - zw*wy) - yy*(zx*ww - zw*wx) + yw*(zx*wy - zy*wx) );
		det -= xw * (yx*(zy*wz - zz*wy) - yy*(zx*wz - zz*wx) + yz*(zx*wy - zy*wx) );

		float invDet = 1.0f/det;

		_result[ 0] = +(yy*(zz*ww - wz*zw) - yz*(zy*ww - wy*zw) + yw*(zy*wz - wy*zz) ) * invDet;
		_result[ 1] = -(xy*(zz*ww - wz*zw) - xz*(zy*ww - wy*zw) + xw*(zy*wz - wy*zz) ) * invDet;
		_result[ 2] = +(xy*(yz*ww - wz*yw) - xz*(yy*ww - wy*yw) + xw*(yy*wz - wy*yz) ) * invDet;
		_result[ 3] = -(xy*(yz*zw - zz*yw) - xz*(yy*zw - zy*yw) + xw*(yy*zz - zy*yz) ) * invDet;

		_result[ 4] = -(yx*(zz*ww - wz*zw) - yz*(zx*ww - wx*zw) + yw*(zx*wz - wx*zz) ) * invDet;
		_result[ 5] = +(xx*(zz*ww - wz*zw) - xz*(zx*ww - wx*zw) + xw*(zx*wz - wx*zz) ) * invDet;
		_result[ 6] = -(xx*(yz*ww - wz*yw) - xz*(yx*ww - wx*yw) + xw*(yx*wz - wx*yz) ) * invDet;
		_result[ 7] = +(xx*(yz*zw - zz*yw) - xz*(yx*zw - zx*yw) + xw*(yx*zz - zx*yz) ) * invDet;

		_result[ 8] = +(yx*(zy*ww - wy*zw) - yy*(zx*ww - wx*zw) + yw*(zx*wy - wx*zy) ) * invDet;
		_result[ 9] = -(xx*(zy*ww - wy*zw) - xy*(zx*ww - wx*zw) + xw*(zx*wy - wx*zy) ) * invDet;
		_result[10] = +(xx*(yy*ww - wy*yw) - xy*(yx*ww - wx*yw) + xw*(yx*wy - wx*yy) ) * invDet;
		_result[11] = -(xx*(yy*zw - zy*yw) - xy*(yx*zw - zx*yw) + xw*(yx*zy - zx*yy) ) * invDet;

		_result[12] = -(yx*(zy*wz - wy*zz) - yy*(zx*wz - wx*zz) + yz*(zx*wy - wx*zy) ) * invDet;
		_result[13] = +(xx*(zy*wz - wy*zz) - xy*(zx*wz - wx*zz) + xz*(zx*wy - wx*zy) ) * invDet;
		_result[14] = -(xx*(yy*wz - wy*yz) - xy*(yx*wz - wx*yz) + xz*(yx*wy - wx*yy) ) * invDet;
		_result[15] = +(xx*(yy*zz - zy*yz) - xy*(yx*zz - zx*yz) + xz*(yx*zy - zx*yy) ) * invDet;
	}

} // namespace bx
