/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/math.h>

#include <math.h>

namespace bx
{
	const float kPi         = 3.1415926535897932384626433832795f;
	const float kPi2        = 6.2831853071795864769252867665590f;
	const float kInvPi      = 1.0f/kPi;
	const float kPiHalf     = 1.5707963267948966192313216916398f;
	const float kSqrt2      = 1.4142135623730950488016887242097f;
	const float kInvLogNat2 = 1.4426950408889634073599246810019f;
#if BX_COMPILER_MSVC
	const float kHuge = float(HUGE_VAL);
#else
	const float kHuge = HUGE_VALF;
#endif // BX_COMPILER_MSVC

	float fabs(float _a)
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

	void mtxLookAtImpl(float* _result, const float* _eye, const float* _view, const float* _up)
	{
		float up[3] = { 0.0f, 1.0f, 0.0f };
		if (NULL != _up)
		{
			up[0] = _up[0];
			up[1] = _up[1];
			up[2] = _up[2];
		}

		float tmp[4];
		vec3Cross(tmp, up, _view);

		float right[4];
		vec3Norm(right, tmp);

		vec3Cross(up, _view, right);

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = right[0];
		_result[ 1] = up[0];
		_result[ 2] = _view[0];

		_result[ 4] = right[1];
		_result[ 5] = up[1];
		_result[ 6] = _view[1];

		_result[ 8] = right[2];
		_result[ 9] = up[2];
		_result[10] = _view[2];

		_result[12] = -vec3Dot(right, _eye);
		_result[13] = -vec3Dot(up, _eye);
		_result[14] = -vec3Dot(_view, _eye);
		_result[15] = 1.0f;
	}

	void mtxLookAtLh(float* _result, const float* _eye, const float* _at, const float* _up)
	{
		float tmp[4];
		vec3Sub(tmp, _at, _eye);

		float view[4];
		vec3Norm(view, tmp);

		mtxLookAtImpl(_result, _eye, view, _up);
	}

	void mtxLookAtRh(float* _result, const float* _eye, const float* _at, const float* _up)
	{
		float tmp[4];
		vec3Sub(tmp, _eye, _at);

		float view[4];
		vec3Norm(view, tmp);

		mtxLookAtImpl(_result, _eye, view, _up);
	}

	void mtxLookAt(float* _result, const float* _eye, const float* _at, const float* _up)
	{
		mtxLookAtLh(_result, _eye, _at, _up);
	}

	template<Handness::Enum HandnessT>
	void mtxProjXYWH(float* _result, float _x, float _y, float _width, float _height, float _near, float _far, bool _oglNdc)
	{
		const float diff = _far-_near;
		const float aa = _oglNdc ? (     _far+_near)/diff : _far/diff;
		const float bb = _oglNdc ? (2.0f*_far*_near)/diff : _near*aa;

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = _width;
		_result[ 5] = _height;
		_result[ 8] = (Handness::Right == HandnessT) ?    _x :  -_x;
		_result[ 9] = (Handness::Right == HandnessT) ?    _y :  -_y;
		_result[10] = (Handness::Right == HandnessT) ?   -aa :   aa;
		_result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
		_result[14] = -bb;
	}

	template<Handness::Enum HandnessT>
	void mtxProjImpl(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc)
	{
		const float invDiffRl = 1.0f/(_rt - _lt);
		const float invDiffUd = 1.0f/(_ut - _dt);
		const float width  =  2.0f*_near * invDiffRl;
		const float height =  2.0f*_near * invDiffUd;
		const float xx     = (_rt + _lt) * invDiffRl;
		const float yy     = (_ut + _dt) * invDiffUd;
		mtxProjXYWH<HandnessT>(_result, xx, yy, width, height, _near, _far, _oglNdc);
	}

	template<Handness::Enum HandnessT>
	void mtxProjImpl(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<HandnessT>(_result, _fov[0], _fov[1], _fov[2], _fov[3], _near, _far, _oglNdc);
	}

	template<Handness::Enum HandnessT>
	void mtxProjImpl(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc)
	{
		const float height = 1.0f/ftan(toRad(_fovy)*0.5f);
		const float width  = height * 1.0f/_aspect;
		mtxProjXYWH<HandnessT>(_result, 0.0f, 0.0f, width, height, _near, _far, _oglNdc);
	}

	void mtxProj(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far, _oglNdc);
	}

	void mtxProj(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
	}

	void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far, _oglNdc);
	}

	void mtxProjLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far, _oglNdc);
	}

	void mtxProjLh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
	}

	void mtxProjLh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far, _oglNdc);
	}

	void mtxProjRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Right>(_result, _ut, _dt, _lt, _rt, _near, _far, _oglNdc);
	}

	void mtxProjRh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Right>(_result, _fov, _near, _far, _oglNdc);
	}

	void mtxProjRh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc)
	{
		mtxProjImpl<Handness::Right>(_result, _fovy, _aspect, _near, _far, _oglNdc);
	}

	template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
	void mtxProjInfXYWH(float* _result, float _x, float _y, float _width, float _height, float _near, bool _oglNdc)
	{
		float aa;
		float bb;
		if (BX_ENABLED(NearFar::Reverse == NearFarT) )
		{
			aa = _oglNdc ?       -1.0f :   0.0f;
			bb = _oglNdc ? -2.0f*_near : -_near;
		}
		else
		{
			aa = 1.0f;
			bb = _oglNdc ? 2.0f*_near : _near;
		}

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = _width;
		_result[ 5] = _height;
		_result[ 8] = (Handness::Right == HandnessT) ?    _x :  -_x;
		_result[ 9] = (Handness::Right == HandnessT) ?    _y :  -_y;
		_result[10] = (Handness::Right == HandnessT) ?   -aa :   aa;
		_result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
		_result[14] = -bb;
	}

	template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
	void mtxProjInfImpl(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		const float invDiffRl = 1.0f/(_rt - _lt);
		const float invDiffUd = 1.0f/(_ut - _dt);
		const float width  =  2.0f*_near * invDiffRl;
		const float height =  2.0f*_near * invDiffUd;
		const float xx     = (_rt + _lt) * invDiffRl;
		const float yy     = (_ut + _dt) * invDiffUd;
		mtxProjInfXYWH<NearFarT,HandnessT>(_result, xx, yy, width, height, _near, _oglNdc);
	}

	template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
	void mtxProjInfImpl(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFarT,HandnessT>(_result, _fov[0], _fov[1], _fov[2], _fov[3], _near, _oglNdc);
	}

	template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
	void mtxProjInfImpl(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		const float height = 1.0f/ftan(toRad(_fovy)*0.5f);
		const float width  = height * 1.0f/_aspect;
		mtxProjInfXYWH<NearFarT,HandnessT>(_result, 0.0f, 0.0f, width, height, _near, _oglNdc);
	}

	void mtxProjInf(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _fov, _near, _oglNdc);
	}

	void mtxProjInf(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _oglNdc);
	}

	void mtxProjInf(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _fovy, _aspect, _near, _oglNdc);
	}

	void mtxProjInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _oglNdc);
	}

	void mtxProjInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _fov, _near, _oglNdc);
	}

	void mtxProjInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Left>(_result, _fovy, _aspect, _near, _oglNdc);
	}

	void mtxProjInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Right>(_result, _ut, _dt, _lt, _rt, _near, _oglNdc);
	}

	void mtxProjInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Right>(_result, _fov, _near, _oglNdc);
	}

	void mtxProjInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Default,Handness::Right>(_result, _fovy, _aspect, _near, _oglNdc);
	}

	void mtxProjRevInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _oglNdc);
	}

	void mtxProjRevInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Left>(_result, _fov, _near, _oglNdc);
	}

	void mtxProjRevInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Left>(_result, _fovy, _aspect, _near, _oglNdc);
	}

	void mtxProjRevInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Right>(_result, _ut, _dt, _lt, _rt, _near, _oglNdc);
	}

	void mtxProjRevInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Right>(_result, _fov, _near, _oglNdc);
	}

	void mtxProjRevInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc)
	{
		mtxProjInfImpl<NearFar::Reverse,Handness::Right>(_result, _fovy, _aspect, _near, _oglNdc);
	}

	template<Handness::Enum HandnessT>
	void mtxOrthoImpl(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc)
	{
		const float aa = 2.0f/(_right - _left);
		const float bb = 2.0f/(_top - _bottom);
		const float cc = (_oglNdc ? 2.0f : 1.0f) / (_far - _near);
		const float dd = (_left + _right )/(_left   - _right);
		const float ee = (_top  + _bottom)/(_bottom - _top  );
		const float ff = _oglNdc
			? (_near + _far)/(_near - _far)
			:  _near        /(_near - _far)
			;

		memSet(_result, 0, sizeof(float)*16);
		_result[ 0] = aa;
		_result[ 5] = bb;
		_result[10] = (Handness::Right == HandnessT) ? -cc : cc;
		_result[12] = dd + _offset;
		_result[13] = ee;
		_result[14] = ff;
		_result[15] = 1.0f;
	}

	void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc)
	{
		mtxOrthoImpl<Handness::Left>(_result, _left, _right, _bottom, _top, _near, _far, _offset, _oglNdc);
	}

	void mtxOrthoLh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc)
	{
		mtxOrthoImpl<Handness::Left>(_result, _left, _right, _bottom, _top, _near, _far, _offset, _oglNdc);
	}

	void mtxOrthoRh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc)
	{
		mtxOrthoImpl<Handness::Right>(_result, _left, _right, _bottom, _top, _near, _far, _offset, _oglNdc);
	}

	void mtxRotateX(float* _result, float _ax)
	{
		const float sx = fsin(_ax);
		const float cx = fcos(_ax);

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
		const float sy = fsin(_ay);
		const float cy = fcos(_ay);

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
		const float sz = fsin(_az);
		const float cz = fcos(_az);

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
		const float sx = fsin(_ax);
		const float cx = fcos(_ax);
		const float sy = fsin(_ay);
		const float cy = fcos(_ay);

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
		const float sx = fsin(_ax);
		const float cx = fcos(_ax);
		const float sy = fsin(_ay);
		const float cy = fcos(_ay);
		const float sz = fsin(_az);
		const float cz = fcos(_az);

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
		const float sx = fsin(_ax);
		const float cx = fcos(_ax);
		const float sy = fsin(_ay);
		const float cy = fcos(_ay);
		const float sz = fsin(_az);
		const float cz = fcos(_az);

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
		const float sx = fsin(_ax);
		const float cx = fcos(_ax);
		const float sy = fsin(_ay);
		const float cy = fcos(_ay);
		const float sz = fsin(_az);
		const float cz = fcos(_az);

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

		const float s0 = fstep(bb, gg);

		const float px = flerp(bb,        gg,         s0);
		const float py = flerp(gg,        bb,         s0);
		const float pz = flerp(-1.0f,     0.0f,       s0);
		const float pw = flerp(2.0f/3.0f, -1.0f/3.0f, s0);

		const float s1 = fstep(px, rr);

		const float qx = flerp(px, rr, s1);
		const float qy = py;
		const float qz = flerp(pw, pz, s1);
		const float qw = flerp(rr, px, s1);

		const float dd = qx - fmin(qw, qy);
		const float ee = 1.0e-10f;

		_hsv[0] = fabs(qz + (qw - qy) / (6.0f * dd + ee) );
		_hsv[1] = dd / (qx + ee);
		_hsv[2] = qx;
	}

	void hsvToRgb(float _rgb[3], const float _hsv[3])
	{
		const float hh = _hsv[0];
		const float ss = _hsv[1];
		const float vv = _hsv[2];

		const float px = fabs(ffract(hh + 1.0f     ) * 6.0f - 3.0f);
		const float py = fabs(ffract(hh + 2.0f/3.0f) * 6.0f - 3.0f);
		const float pz = fabs(ffract(hh + 1.0f/3.0f) * 6.0f - 3.0f);

		_rgb[0] = vv * flerp(1.0f, fsaturate(px - 1.0f), ss);
		_rgb[1] = vv * flerp(1.0f, fsaturate(py - 1.0f), ss);
		_rgb[2] = vv * flerp(1.0f, fsaturate(pz - 1.0f), ss);
	}

} // namespace bx
