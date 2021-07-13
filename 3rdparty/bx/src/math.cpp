/*
 * Copyright 2011-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/math.h>
#include <bx/uint32_t.h>

namespace bx
{
	const float kInfinity = bitsToFloat(UINT32_C(0x7f800000) );

	namespace
	{
		constexpr float kSinC2  = -0.16666667163372039794921875f;
		constexpr float kSinC4  =  8.333347737789154052734375e-3f;
		constexpr float kSinC6  = -1.9842604524455964565277099609375e-4f;
		constexpr float kSinC8  =  2.760012648650445044040679931640625e-6f;
		constexpr float kSinC10 = -2.50293279435709337121807038784027099609375e-8f;

		constexpr float kCosC2  = -0.5f;
		constexpr float kCosC4  =  4.166664183139801025390625e-2f;
		constexpr float kCosC6  = -1.388833043165504932403564453125e-3f;
		constexpr float kCosC8  =  2.47562347794882953166961669921875e-5f;
		constexpr float kCosC10 = -2.59630184018533327616751194000244140625e-7f;

	} // namespace

	BX_CONST_FUNC float cos(float _a)
	{
		const float scaled = _a * 2.0f*kInvPi;
		const float real   = floor(scaled);
		const float xx     = _a - real * kPiHalf;
		const int32_t bits = int32_t(real) & 3;

		float c0, c2, c4, c6, c8, c10;

		if (bits == 0
		||  bits == 2)
		{
			c0  = 1.0f;
			c2  = kCosC2;
			c4  = kCosC4;
			c6  = kCosC6;
			c8  = kCosC8;
			c10 = kCosC10;
		}
		else
		{
			c0  = xx;
			c2  = kSinC2;
			c4  = kSinC4;
			c6  = kSinC6;
			c8  = kSinC8;
			c10 = kSinC10;
		}

		const float xsq    = square(xx);
		const float tmp0   = mad(c10,  xsq, c8 );
		const float tmp1   = mad(tmp0, xsq, c6 );
		const float tmp2   = mad(tmp1, xsq, c4 );
		const float tmp3   = mad(tmp2, xsq, c2 );
		const float tmp4   = mad(tmp3, xsq, 1.0);
		const float result = tmp4 * c0;

		return bits == 1 || bits == 2
			? -result
			:  result
			;
	}

	namespace
	{
		constexpr float kAcosC0 =  1.5707288f;
		constexpr float kAcosC1 = -0.2121144f;
		constexpr float kAcosC2 =  0.0742610f;
		constexpr float kAcosC3 = -0.0187293f;

	} // namespace

	BX_CONST_FUNC float acos(float _a)
	{
		const float absa   = abs(_a);
		const float tmp0   = mad(kAcosC3, absa, kAcosC2);
		const float tmp1   = mad(tmp0,    absa, kAcosC1);
		const float tmp2   = mad(tmp1,    absa, kAcosC0);
		const float tmp3   = tmp2 * sqrt(1.0f - absa);
		const float negate = float(_a < 0.0f);
		const float tmp4   = tmp3 - 2.0f*negate*tmp3;
		const float result = negate*kPi + tmp4;

		return result;
	}

	namespace
	{
		constexpr float kAtan2C0 = -0.013480470f;
		constexpr float kAtan2C1 =  0.057477314f;
		constexpr float kAtan2C2 = -0.121239071f;
		constexpr float kAtan2C3 =  0.195635925f;
		constexpr float kAtan2C4 = -0.332994597f;
		constexpr float kAtan2C5 =  0.999995630f;

	} // namespace

	BX_CONST_FUNC float atan2(float _y, float _x)
	{
		const float ax     = abs(_x);
		const float ay     = abs(_y);
		const float maxaxy = max(ax, ay);
		const float minaxy = min(ax, ay);

		if (maxaxy == 0.0f)
		{
			return 0.0f*sign(_y);
		}

		const float mxy    = minaxy / maxaxy;
		const float mxysq  = square(mxy);
		const float tmp0   = mad(kAtan2C0, mxysq, kAtan2C1);
		const float tmp1   = mad(tmp0,     mxysq, kAtan2C2);
		const float tmp2   = mad(tmp1,     mxysq, kAtan2C3);
		const float tmp3   = mad(tmp2,     mxysq, kAtan2C4);
		const float tmp4   = mad(tmp3,     mxysq, kAtan2C5);
		const float tmp5   = tmp4 * mxy;
		const float tmp6   = ay > ax   ? kPiHalf - tmp5 : tmp5;
		const float tmp7   = _x < 0.0f ? kPi     - tmp6 : tmp6;
		const float result = sign(_y)*tmp7;

		return result;
	}

	BX_CONST_FUNC float ldexp(float _a, int32_t _b)
	{
		const uint32_t ftob     = floatToBits(_a);
		const uint32_t masked   = uint32_and(ftob, UINT32_C(0xff800000) );
		const uint32_t expsign0 = uint32_sra(masked, 23);
		const uint32_t tmp      = uint32_iadd(expsign0, _b);
		const uint32_t expsign1 = uint32_sll(tmp, 23);
		const uint32_t mantissa = uint32_and(ftob, UINT32_C(0x007fffff) );
		const uint32_t bits     = uint32_or(mantissa, expsign1);
		const float    result   = bitsToFloat(bits);

		return result;
	}

	float frexp(float _a, int32_t* _outExp)
	{
		const uint32_t ftob     = floatToBits(_a);
		const uint32_t masked0  = uint32_and(ftob, UINT32_C(0x7f800000) );
		const uint32_t exp0     = uint32_srl(masked0, 23);
		const uint32_t masked1  = uint32_and(ftob,   UINT32_C(0x807fffff) );
		const uint32_t bits     = uint32_or(masked1, UINT32_C(0x3f000000) );
		const float    result   = bitsToFloat(bits);

		*_outExp = int32_t(exp0 - 0x7e);

		return result;
	}

	namespace
	{
		constexpr float kExpC0  =  1.66666666666666019037e-01f;
		constexpr float kExpC1  = -2.77777777770155933842e-03f;
		constexpr float kExpC2  =  6.61375632143793436117e-05f;
		constexpr float kExpC3  = -1.65339022054652515390e-06f;
		constexpr float kExpC4  =  4.13813679705723846039e-08f;

	} // namespace

	BX_CONST_FUNC float exp(float _a)
	{
		if (abs(_a) <= kNearZero)
		{
			return _a + 1.0f;
		}

		const float kk     = round(_a*kInvLogNat2);
		const float hi     = _a - kk*kLogNat2Hi;
		const float lo     =      kk*kLogNat2Lo;
		const float hml    = hi - lo;
		const float hmlsq  = square(hml);
		const float tmp0   = mad(kExpC4, hmlsq, kExpC3);
		const float tmp1   = mad(tmp0,   hmlsq, kExpC2);
		const float tmp2   = mad(tmp1,   hmlsq, kExpC1);
		const float tmp3   = mad(tmp2,   hmlsq, kExpC0);
		const float tmp4   = hml - hmlsq * tmp3;
		const float tmp5   = hml*tmp4/(2.0f-tmp4);
		const float tmp6   = 1.0f - ( (lo - tmp5) - hi);
		const float result = ldexp(tmp6, int32_t(kk) );

		return result;
	}

	namespace
	{
		constexpr float kLogC0 = 6.666666666666735130e-01f;
		constexpr float kLogC1 = 3.999999999940941908e-01f;
		constexpr float kLogC2 = 2.857142874366239149e-01f;
		constexpr float kLogC3 = 2.222219843214978396e-01f;
		constexpr float kLogC4 = 1.818357216161805012e-01f;
		constexpr float kLogC5 = 1.531383769920937332e-01f;
		constexpr float kLogC6 = 1.479819860511658591e-01f;

	} // namespace

	BX_CONST_FUNC float log(float _a)
	{
		int32_t exp;
		float ff = frexp(_a, &exp);
		if (ff < kSqrt2*0.5f)
		{
			ff *= 2.0f;
			--exp;
		}

		ff -= 1.0f;
		const float kk     = float(exp);
		const float hi     = kk*kLogNat2Hi;
		const float lo     = kk*kLogNat2Lo;
		const float ss     = ff / (2.0f + ff);
		const float s2     = square(ss);
		const float s4     = square(s2);

		const float tmp0   = mad(kLogC6, s4, kLogC4);
		const float tmp1   = mad(tmp0,   s4, kLogC2);
		const float tmp2   = mad(tmp1,   s4, kLogC0);
		const float t1     = s2*tmp2;

		const float tmp3   = mad(kLogC5, s4, kLogC3);
		const float tmp4   = mad(tmp3,   s4, kLogC1);
		const float t2     = s4*tmp4;

		const float t12    = t1 + t2;
		const float hfsq   = 0.5f*square(ff);
		const float result = hi - ( (hfsq - (ss*(hfsq+t12) + lo) ) - ff);

		return result;
	}

	void mtxLookAt(float* _result, const Vec3& _eye, const Vec3& _at, const Vec3& _up, Handness::Enum _handness)
	{
		const Vec3 view = normalize(
			  Handness::Right == _handness
			? sub(_eye, _at)
			: sub(_at, _eye)
			);
		const Vec3 uxv   = cross(_up, view);
		const Vec3 right = normalize(uxv);
		const Vec3 up    = cross(view, right);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = right.x;
		_result[ 1] = up.x;
		_result[ 2] = view.x;

		_result[ 4] = right.y;
		_result[ 5] = up.y;
		_result[ 6] = view.y;

		_result[ 8] = right.z;
		_result[ 9] = up.z;
		_result[10] = view.z;

		_result[12] = -dot(right, _eye);
		_result[13] = -dot(up,    _eye);
		_result[14] = -dot(view,  _eye);
		_result[15] = 1.0f;
	}

	static void mtxProjXYWH(float* _result, float _x, float _y, float _width, float _height, float _near, float _far, bool _homogeneousNdc, Handness::Enum _handness)
	{
		const float diff = _far-_near;
		const float aa = _homogeneousNdc ? (     _far+_near)/diff : _far/diff;
		const float bb = _homogeneousNdc ? (2.0f*_far*_near)/diff : _near*aa;

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = _width;
		_result[ 5] = _height;
		_result[ 8] = (Handness::Right == _handness) ?    _x :  -_x;
		_result[ 9] = (Handness::Right == _handness) ?    _y :  -_y;
		_result[10] = (Handness::Right == _handness) ?   -aa :   aa;
		_result[11] = (Handness::Right == _handness) ? -1.0f : 1.0f;
		_result[14] = -bb;
	}

	void mtxProj(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _homogeneousNdc, Handness::Enum _handness)
	{
		const float invDiffRl = 1.0f/(_rt - _lt);
		const float invDiffUd = 1.0f/(_ut - _dt);
		const float width  =  2.0f*_near * invDiffRl;
		const float height =  2.0f*_near * invDiffUd;
		const float xx     = (_rt + _lt) * invDiffRl;
		const float yy     = (_ut + _dt) * invDiffUd;
		mtxProjXYWH(_result, xx, yy, width, height, _near, _far, _homogeneousNdc, _handness);
	}

	void mtxProj(float* _result, const float _fov[4], float _near, float _far, bool _homogeneousNdc, Handness::Enum _handness)
	{
		mtxProj(_result, _fov[0], _fov[1], _fov[2], _fov[3], _near, _far, _homogeneousNdc, _handness);
	}

	void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _homogeneousNdc, Handness::Enum _handness)
	{
		const float height = 1.0f/tan(toRad(_fovy)*0.5f);
		const float width  = height * 1.0f/_aspect;
		mtxProjXYWH(_result, 0.0f, 0.0f, width, height, _near, _far, _homogeneousNdc, _handness);
	}

	static void mtxProjInfXYWH(float* _result, float _x, float _y, float _width, float _height, float _near, bool _homogeneousNdc, Handness::Enum _handness, NearFar::Enum _nearFar)
	{
		float aa;
		float bb;
		if (NearFar::Reverse == _nearFar)
		{
			aa = _homogeneousNdc ?       -1.0f :   0.0f;
			bb = _homogeneousNdc ? -2.0f*_near : -_near;
		}
		else
		{
			aa = 1.0f;
			bb = _homogeneousNdc ? 2.0f*_near : _near;
		}

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = _width;
		_result[ 5] = _height;
		_result[ 8] = (Handness::Right == _handness) ?    _x :  -_x;
		_result[ 9] = (Handness::Right == _handness) ?    _y :  -_y;
		_result[10] = (Handness::Right == _handness) ?   -aa :   aa;
		_result[11] = (Handness::Right == _handness) ? -1.0f : 1.0f;
		_result[14] = -bb;
	}

	void mtxProjInf(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _homogeneousNdc, Handness::Enum _handness, NearFar::Enum _nearFar)
	{
		const float invDiffRl = 1.0f/(_rt - _lt);
		const float invDiffUd = 1.0f/(_ut - _dt);
		const float width  =  2.0f*_near * invDiffRl;
		const float height =  2.0f*_near * invDiffUd;
		const float xx     = (_rt + _lt) * invDiffRl;
		const float yy     = (_ut + _dt) * invDiffUd;
		mtxProjInfXYWH(_result, xx, yy, width, height, _near, _homogeneousNdc, _handness, _nearFar);
	}

	void mtxProjInf(float* _result, const float _fov[4], float _near, bool _homogeneousNdc, Handness::Enum _handness, NearFar::Enum _nearFar)
	{
		mtxProjInf(_result, _fov[0], _fov[1], _fov[2], _fov[3], _near, _homogeneousNdc, _handness, _nearFar);
	}

	void mtxProjInf(float* _result, float _fovy, float _aspect, float _near, bool _homogeneousNdc, Handness::Enum _handness, NearFar::Enum _nearFar)
	{
		const float height = 1.0f/tan(toRad(_fovy)*0.5f);
		const float width  = height * 1.0f/_aspect;
		mtxProjInfXYWH(_result, 0.0f, 0.0f, width, height, _near, _homogeneousNdc, _handness, _nearFar);
	}

	void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _homogeneousNdc, Handness::Enum _handness)
	{
		const float aa = 2.0f/(_right - _left);
		const float bb = 2.0f/(_top - _bottom);
		const float cc = (_homogeneousNdc ? 2.0f : 1.0f) / (_far - _near);
		const float dd = (_left + _right )/(_left   - _right);
		const float ee = (_top  + _bottom)/(_bottom - _top  );
		const float ff = _homogeneousNdc
			? (_near + _far)/(_near - _far)
			:  _near        /(_near - _far)
			;

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = aa;
		_result[ 5] = bb;
		_result[10] = Handness::Right == _handness ? -cc : cc;
		_result[12] = dd + _offset;
		_result[13] = ee;
		_result[14] = ff;
		_result[15] = 1.0f;
	}

	void mtxRotateX(float* _result, float _ax)
	{
		const float sx = sin(_ax);
		const float cx = cos(_ax);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = 1.0f;
		_result[ 5] = cx;
		_result[ 6] = -sx;
		_result[ 9] = sx;
		_result[10] = cx;
		_result[15] = 1.0f;
	}

	void mtxRotateY(float* _result, float _ay)
	{
		const float sy = sin(_ay);
		const float cy = cos(_ay);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = cy;
		_result[ 2] = sy;
		_result[ 5] = 1.0f;
		_result[ 8] = -sy;
		_result[10] = cy;
		_result[15] = 1.0f;
	}

	void mtxRotateZ(float* _result, float _az)
	{
		const float sz = sin(_az);
		const float cz = cos(_az);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = cz;
		_result[ 1] = -sz;
		_result[ 4] = sz;
		_result[ 5] = cz;
		_result[10] = 1.0f;
		_result[15] = 1.0f;
	}

	void mtxRotateXY(float* _result, float _ax, float _ay)
	{
		const float sx = sin(_ax);
		const float cx = cos(_ax);
		const float sy = sin(_ay);
		const float cy = cos(_ay);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = cy;
		_result[ 2] = sy;
		_result[ 4] = sx*sy;
		_result[ 5] = cx;
		_result[ 6] = -sx*cy;
		_result[ 8] = -cx*sy;
		_result[ 9] = sx;
		_result[10] = cx*cy;
		_result[15] = 1.0f;
	}

	void mtxRotateXYZ(float* _result, float _ax, float _ay, float _az)
	{
		const float sx = sin(_ax);
		const float cx = cos(_ax);
		const float sy = sin(_ay);
		const float cy = cos(_ay);
		const float sz = sin(_az);
		const float cz = cos(_az);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = cy*cz;
		_result[ 1] = -cy*sz;
		_result[ 2] = sy;
		_result[ 4] = cz*sx*sy + cx*sz;
		_result[ 5] = cx*cz - sx*sy*sz;
		_result[ 6] = -cy*sx;
		_result[ 8] = -cx*cz*sy + sx*sz;
		_result[ 9] = cz*sx + cx*sy*sz;
		_result[10] = cx*cy;
		_result[15] = 1.0f;
	}

	void mtxRotateZYX(float* _result, float _ax, float _ay, float _az)
	{
		const float sx = sin(_ax);
		const float cx = cos(_ax);
		const float sy = sin(_ay);
		const float cy = cos(_ay);
		const float sz = sin(_az);
		const float cz = cos(_az);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = cy*cz;
		_result[ 1] = cz*sx*sy-cx*sz;
		_result[ 2] = cx*cz*sy+sx*sz;
		_result[ 4] = cy*sz;
		_result[ 5] = cx*cz + sx*sy*sz;
		_result[ 6] = -cz*sx + cx*sy*sz;
		_result[ 8] = -sy;
		_result[ 9] = cy*sx;
		_result[10] = cx*cy;
		_result[15] = 1.0f;
	};

	void mtxSRT(float* _result, float _sx, float _sy, float _sz, float _ax, float _ay, float _az, float _tx, float _ty, float _tz)
	{
		const float sx = sin(_ax);
		const float cx = cos(_ax);
		const float sy = sin(_ay);
		const float cy = cos(_ay);
		const float sz = sin(_az);
		const float cz = cos(_az);

		const float sxsz = sx*sz;
		const float cycz = cy*cz;

		_result[ 0] = _sx * (cycz - sxsz*sy);
		_result[ 1] = _sx * -cx*sz;
		_result[ 2] = _sx * (cz*sy + cy*sxsz);
		_result[ 3] = 0.0f;

		_result[ 4] = _sy * (cz*sx*sy + cy*sz);
		_result[ 5] = _sy * cx*cz;
		_result[ 6] = _sy * (sy*sz -cycz*sx);
		_result[ 7] = 0.0f;

		_result[ 8] = _sz * -cx*sy;
		_result[ 9] = _sz * sx;
		_result[10] = _sz * cx*cy;
		_result[11] = 0.0f;

		_result[12] = _tx;
		_result[13] = _ty;
		_result[14] = _tz;
		_result[15] = 1.0f;
	}

	void mtx3Inverse(float* _result, const float* _a)
	{
		const float xx = _a[0];
		const float xy = _a[1];
		const float xz = _a[2];
		const float yx = _a[3];
		const float yy = _a[4];
		const float yz = _a[5];
		const float zx = _a[6];
		const float zy = _a[7];
		const float zz = _a[8];

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
		const float xx = _a[ 0];
		const float xy = _a[ 1];
		const float xz = _a[ 2];
		const float xw = _a[ 3];
		const float yx = _a[ 4];
		const float yy = _a[ 5];
		const float yz = _a[ 6];
		const float yw = _a[ 7];
		const float zx = _a[ 8];
		const float zy = _a[ 9];
		const float zz = _a[10];
		const float zw = _a[11];
		const float wx = _a[12];
		const float wy = _a[13];
		const float wz = _a[14];
		const float ww = _a[15];

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

	void mtx3Cofactor(float* _result, const float* _a)
	{
		const float xx = _a[0];
		const float xy = _a[1];
		const float xz = _a[2];
		const float yx = _a[3];
		const float yy = _a[4];
		const float yz = _a[5];
		const float zx = _a[6];
		const float zy = _a[7];
		const float zz = _a[8];

		_result[0] = +(yy*zz - yz * zy);
		_result[1] = -(yx*zz - yz * zx);
		_result[2] = +(yx*zy - yy * zx);

		_result[3] = -(xy*zz - xz * zy);
		_result[4] = +(xx*zz - xz * zx);
		_result[5] = -(xx*zy - xy * zx);

		_result[6] = +(xy*yz - xz * yy);
		_result[7] = -(xx*yz - xz * yx);
		_result[8] = +(xx*yy - xy * yx);
	}

	void mtxCofactor(float* _result, const float* _a)
	{
		const float xx = _a[0];
		const float xy = _a[1];
		const float xz = _a[2];
		const float xw = _a[3];
		const float yx = _a[4];
		const float yy = _a[5];
		const float yz = _a[6];
		const float yw = _a[7];
		const float zx = _a[8];
		const float zy = _a[9];
		const float zz = _a[10];
		const float zw = _a[11];
		const float wx = _a[12];
		const float wy = _a[13];
		const float wz = _a[14];
		const float ww = _a[15];

		_result[ 0] = +(yy*(zz*ww - wz * zw) - yz * (zy*ww - wy * zw) + yw * (zy*wz - wy * zz) );
		_result[ 1] = -(yx*(zz*ww - wz * zw) - yz * (zx*ww - wx * zw) + yw * (zx*wz - wx * zz) );
		_result[ 2] = +(yx*(zy*ww - wy * zw) - yy * (zx*ww - wx * zw) + yw * (zx*wy - wx * zy) );
		_result[ 3] = -(yx*(zy*wz - wy * zz) - yy * (zx*wz - wx * zz) + yz * (zx*wy - wx * zy) );

		_result[ 4] = -(xy*(zz*ww - wz * zw) - xz * (zy*ww - wy * zw) + xw * (zy*wz - wy * zz) );
		_result[ 5] = +(xx*(zz*ww - wz * zw) - xz * (zx*ww - wx * zw) + xw * (zx*wz - wx * zz) );
		_result[ 6] = -(xx*(zy*ww - wy * zw) - xy * (zx*ww - wx * zw) + xw * (zx*wy - wx * zy) );
		_result[ 7] = +(xx*(zy*wz - wy * zz) - xy * (zx*wz - wx * zz) + xz * (zx*wy - wx * zy) );

		_result[ 8] = +(xy*(yz*ww - wz * yw) - xz * (yy*ww - wy * yw) + xw * (yy*wz - wy * yz) );
		_result[ 9] = -(xx*(yz*ww - wz * yw) - xz * (yx*ww - wx * yw) + xw * (yx*wz - wx * yz) );
		_result[10] = +(xx*(yy*ww - wy * yw) - xy * (yx*ww - wx * yw) + xw * (yx*wy - wx * yy) );
		_result[11] = -(xx*(yy*wz - wy * yz) - xy * (yx*wz - wx * yz) + xz * (yx*wy - wx * yy) );

		_result[12] = -(xy*(yz*zw - zz * yw) - xz * (yy*zw - zy * yw) + xw * (yy*zz - zy * yz) );
		_result[13] = +(xx*(yz*zw - zz * yw) - xz * (yx*zw - zx * yw) + xw * (yx*zz - zx * yz) );
		_result[14] = -(xx*(yy*zw - zy * yw) - xy * (yx*zw - zx * yw) + xw * (yx*zy - zx * yy) );
		_result[15] = +(xx*(yy*zz - zy * yz) - xy * (yx*zz - zx * yz) + xz * (yx*zy - zx * yy) );
	}

	void calcLinearFit2D(float _result[2], const void* _points, uint32_t _stride, uint32_t _numPoints)
	{
		float sumX  = 0.0f;
		float sumY  = 0.0f;
		float sumXX = 0.0f;
		float sumXY = 0.0f;

		const uint8_t* ptr = (const uint8_t*)_points;
		for (uint32_t ii = 0; ii < _numPoints; ++ii, ptr += _stride)
		{
			const float* point = (const float*)ptr;
			float xx = point[0];
			float yy = point[1];
			sumX  += xx;
			sumY  += yy;
			sumXX += xx*xx;
			sumXY += xx*yy;
		}

		// [ sum(x^2) sum(x)    ] [ A ] = [ sum(x*y) ]
		// [ sum(x)   numPoints ] [ B ]   [ sum(y)   ]

		float det = (sumXX*_numPoints - sumX*sumX);
		float invDet = 1.0f/det;

		_result[0] = (-sumX * sumY + _numPoints * sumXY) * invDet;
		_result[1] = (sumXX * sumY - sumX       * sumXY) * invDet;
	}

	void calcLinearFit3D(float _result[3], const void* _points, uint32_t _stride, uint32_t _numPoints)
	{
		float sumX  = 0.0f;
		float sumY  = 0.0f;
		float sumZ  = 0.0f;
		float sumXX = 0.0f;
		float sumXY = 0.0f;
		float sumXZ = 0.0f;
		float sumYY = 0.0f;
		float sumYZ = 0.0f;

		const uint8_t* ptr = (const uint8_t*)_points;
		for (uint32_t ii = 0; ii < _numPoints; ++ii, ptr += _stride)
		{
			const float* point = (const float*)ptr;
			float xx = point[0];
			float yy = point[1];
			float zz = point[2];

			sumX  += xx;
			sumY  += yy;
			sumZ  += zz;
			sumXX += xx*xx;
			sumXY += xx*yy;
			sumXZ += xx*zz;
			sumYY += yy*yy;
			sumYZ += yy*zz;
		}

		// [ sum(x^2) sum(x*y) sum(x)    ] [ A ]   [ sum(x*z) ]
		// [ sum(x*y) sum(y^2) sum(y)    ] [ B ] = [ sum(y*z) ]
		// [ sum(x)   sum(y)   numPoints ] [ C ]   [ sum(z)   ]

		float mtx[9] =
		{
			sumXX, sumXY, sumX,
			sumXY, sumYY, sumY,
			sumX,  sumY,  float(_numPoints),
		};
		float invMtx[9];
		mtx3Inverse(invMtx, mtx);

		_result[0] = invMtx[0]*sumXZ + invMtx[1]*sumYZ + invMtx[2]*sumZ;
		_result[1] = invMtx[3]*sumXZ + invMtx[4]*sumYZ + invMtx[5]*sumZ;
		_result[2] = invMtx[6]*sumXZ + invMtx[7]*sumYZ + invMtx[8]*sumZ;
	}

	void rgbToHsv(float _hsv[3], const float _rgb[3])
	{
		const float rr = _rgb[0];
		const float gg = _rgb[1];
		const float bb = _rgb[2];

		const float s0 = step(bb, gg);

		const float px = lerp(bb,        gg,         s0);
		const float py = lerp(gg,        bb,         s0);
		const float pz = lerp(-1.0f,     0.0f,       s0);
		const float pw = lerp(2.0f/3.0f, -1.0f/3.0f, s0);

		const float s1 = step(px, rr);

		const float qx = lerp(px, rr, s1);
		const float qy = py;
		const float qz = lerp(pw, pz, s1);
		const float qw = lerp(rr, px, s1);

		const float dd = qx - min(qw, qy);
		const float ee = 1.0e-10f;

		_hsv[0] = abs(qz + (qw - qy) / (6.0f * dd + ee) );
		_hsv[1] = dd / (qx + ee);
		_hsv[2] = qx;
	}

	void hsvToRgb(float _rgb[3], const float _hsv[3])
	{
		const float hh = _hsv[0];
		const float ss = _hsv[1];
		const float vv = _hsv[2];

		const float px = abs(fract(hh + 1.0f     ) * 6.0f - 3.0f);
		const float py = abs(fract(hh + 2.0f/3.0f) * 6.0f - 3.0f);
		const float pz = abs(fract(hh + 1.0f/3.0f) * 6.0f - 3.0f);

		_rgb[0] = vv * lerp(1.0f, clamp(px - 1.0f, 0.0f, 1.0f), ss);
		_rgb[1] = vv * lerp(1.0f, clamp(py - 1.0f, 0.0f, 1.0f), ss);
		_rgb[2] = vv * lerp(1.0f, clamp(pz - 1.0f, 0.0f, 1.0f), ss);
	}

} // namespace bx
