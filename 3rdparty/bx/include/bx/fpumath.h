/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

// FPU math lib

#ifndef BX_FPU_MATH_H_HEADER_GUARD
#define BX_FPU_MATH_H_HEADER_GUARD

#include "bx.h"
#include <math.h>
#include <string.h>

namespace bx
{
	extern const float pi;
	extern const float invPi;
	extern const float piHalf;
	extern const float sqrt2;
	extern const float huge;

	///
	struct Handness
	{
		enum Enum
		{
			Left,
			Right,
		};
	};

	///
	struct NearFar
	{
		enum Enum
		{
			Default,
			Reverse,
		};
	};

	///
	float toRad(float _deg);

	///
	float toDeg(float _rad);

	///
	uint32_t floatToBits(float _a);

	///
	float bitsToFloat(uint32_t _a);

	///
	uint64_t doubleToBits(double _a);

	///
	double bitsToDouble(uint64_t _a);

	///
	bool isNan(float _f);

	///
	bool isNan(double _f);

	///
	bool isFinite(float _f);

	///
	bool isFinite(double _f);

	///
	bool isInfinite(float _f);

	///
	bool isInfinite(double _f);

	///
	float ffloor(float _f);

	///
	float fceil(float _f);

	///
	float fround(float _f);

	///
	float fmin(float _a, float _b);

	///
	float fmax(float _a, float _b);

	///
	float fmin3(float _a, float _b, float _c);

	///
	float fmax3(float _a, float _b, float _c);

	///
	float fclamp(float _a, float _min, float _max);

	///
	float fsaturate(float _a);

	///
	float flerp(float _a, float _b, float _t);

	///
	float fsign(float _a);

	///
	float fabsolute(float _a);

	///
	float fsq(float _a);

	///
	float fsin(float _a);

	///
	float fasin(float _a);

	///
	float fcos(float _a);

	///
	float facos(float _a);

	///
	float fatan2(float _y, float _x);

	///
	float fpow(float _a, float _b);

	///
	float fexp2(float _a);

	///
	float flog(float _a);

	///
	float flog2(float _a);

	///
	float fsqrt(float _a);

	///
	float frsqrt(float _a);

	///
	float ffract(float _a);

	///
	float fmod(float _a, float _b);

	///
	bool fequal(float _a, float _b, float _epsilon);

	///
	bool fequal(const float* __restrict _a, const float* __restrict _b, uint32_t _num, float _epsilon);

	///
	float fwrap(float _a, float _wrap);

	///
	float fstep(float _edge, float _a);

	///
	float fpulse(float _a, float _start, float _end);

	///
	float fsmoothstep(float _a);

	// References:
	//  - Bias And Gain Are Your Friend
	//    http://blog.demofox.org/2012/09/24/bias-and-gain-are-your-friend/
	//  - http://demofox.org/biasgain.html
	///
	float fbias(float _time, float _bias);

	///
	float fgain(float _time, float _gain);

	///
	void vec3Move(float* __restrict _result, const float* __restrict _a);

	///
	void vec3Abs(float* __restrict _result, const float* __restrict _a);

	///
	void vec3Neg(float* __restrict _result, const float* __restrict _a);

	///
	void vec3Add(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Add(float* __restrict _result, const float* __restrict _a, float _b);

	///
	void vec3Sub(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Sub(float* __restrict _result, const float* __restrict _a, float _b);

	///
	void vec3Mul(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Mul(float* __restrict _result, const float* __restrict _a, float _b);

	///
	float vec3Dot(const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Cross(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	float vec3Length(const float* _a);

	///
	void vec3Lerp(float* __restrict _result, const float* __restrict _a, const float* __restrict _b, float _t);

	///
	void vec3Lerp(float* __restrict _result, const float* __restrict _a, const float* __restrict _b, const float* __restrict _c);

	///
	float vec3Norm(float* __restrict _result, const float* __restrict _a);

	///
	void vec3Min(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Max(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void vec3Rcp(float* __restrict _result, const float* __restrict _a);

	/// Calculate tangent frame from normal.
	///
	void vec3TangentFrame(const float* __restrict _n, float* __restrict _t, float* __restrict _b);

	/// Calculate tangent frame from normal and angle.
	///
	void vec3TangentFrame(const float* __restrict _n, float* __restrict _t, float* __restrict _b, float _angle);

	///
	void quatIdentity(float* _result);

	///
	void quatMove(float* __restrict _result, const float* __restrict _a);

	///
	void quatMulXYZ(float* __restrict _result, const float* __restrict _qa, const float* __restrict _qb);

	///
	void quatMul(float* __restrict _result, const float* __restrict _qa, const float* __restrict _qb);

	///
	void quatInvert(float* __restrict _result, const float* __restrict _quat);

	///
	float quatDot(const float* __restrict _a, const float* __restrict _b);

	///
	void quatNorm(float* __restrict _result, const float* __restrict _quat);

	///
	void quatToEuler(float* __restrict _result, const float* __restrict _quat);

	///
	void quatRotateAxis(float* __restrict _result, const float* _axis, float _angle);

	///
	void quatRotateX(float* _result, float _ax);

	///
	void quatRotateY(float* _result, float _ay);

	///
	void quatRotateZ(float* _result, float _az);

	///
	void vec3MulQuat(float* __restrict _result, const float* __restrict _vec, const float* __restrict _quat);

	///
	void mtxIdentity(float* _result);

	///
	void mtxTranslate(float* _result, float _tx, float _ty, float _tz);

	///
	void mtxScale(float* _result, float _sx, float _sy, float _sz);

	///
	void mtxScale(float* _result, float _scale);

	///
	void mtxFromNormal(float* __restrict _result, const float* __restrict _normal, float _scale, const float* __restrict _pos);

	///
	void mtxFromNormal(float* __restrict _result, const float* __restrict _normal, float _scale, const float* __restrict _pos, float _angle);

	///
	void mtxQuat(float* __restrict _result, const float* __restrict _quat);

	///
	void mtxQuatTranslation(float* __restrict _result, const float* __restrict _quat, const float* __restrict _translation);

	///
	void mtxQuatTranslationHMD(float* __restrict _result, const float* __restrict _quat, const float* __restrict _translation);

	///
	void mtxLookAtLh(float* __restrict _result, const float* __restrict _eye, const float* __restrict _at, const float* __restrict _up = NULL);;

	///
	void mtxLookAtRh(float* __restrict _result, const float* __restrict _eye, const float* __restrict _at, const float* __restrict _up = NULL);

	///
	void mtxLookAt(float* __restrict _result, const float* __restrict _eye, const float* __restrict _at, const float* __restrict _up = NULL);

	///
	void mtxProj(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProj(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc = false);

	///
	void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjLh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjLh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjRh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjRh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc = false);

	///
	void mtxProjInf(float* _result, const float _fov[4], float _near, bool _oglNdc = false);

	///
	void mtxProjInf(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc = false);

	///
	void mtxProjInf(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc = false);

	///
	void mtxProjInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc = false);

	///
	void mtxProjInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc = false);

	///
	void mtxProjInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc = false);

	///
	void mtxProjInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc = false);

	///
	void mtxProjInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc = false);

	///
	void mtxProjInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc = false);

	///
	void mtxProjRevInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc = false);

	///
	void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset = 0.0f, bool _oglNdc = false);

	///
	void mtxOrthoLh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset = 0.0f, bool _oglNdc = false);

	///
	void mtxOrthoRh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset = 0.0f, bool _oglNdc = false);

	///
	void mtxRotateX(float* _result, float _ax);

	///
	void mtxRotateY(float* _result, float _ay);

	///
	void mtxRotateZ(float* _result, float _az);

	///
	void mtxRotateXY(float* _result, float _ax, float _ay);

	///
	void mtxRotateXYZ(float* _result, float _ax, float _ay, float _az);

	///
	void mtxRotateZYX(float* _result, float _ax, float _ay, float _az);

	///
	void mtxSRT(float* _result, float _sx, float _sy, float _sz, float _ax, float _ay, float _az, float _tx, float _ty, float _tz);

	///
	void vec3MulMtx(float* __restrict _result, const float* __restrict _vec, const float* __restrict _mat);

	///
	void vec3MulMtxH(float* __restrict _result, const float* __restrict _vec, const float* __restrict _mat);

	///
	void vec4MulMtx(float* __restrict _result, const float* __restrict _vec, const float* __restrict _mat);

	///
	void mtxMul(float* __restrict _result, const float* __restrict _a, const float* __restrict _b);

	///
	void mtxTranspose(float* __restrict _result, const float* __restrict _a);

	///
	void mtx3Inverse(float* __restrict _result, const float* __restrict _a);

	///
	void mtxInverse(float* __restrict _result, const float* __restrict _a);

	/// Convert LH to RH projection matrix and vice versa.
	///
	void mtxProjFlipHandedness(float* __restrict _dst, const float* __restrict _src);

	/// Convert LH to RH view matrix and vice versa.
	///
	void mtxViewFlipHandedness(float* __restrict _dst, const float* __restrict _src);

	///
	void calcNormal(float _result[3], float _va[3], float _vb[3], float _vc[3]);

	///
	void calcPlane(float _result[4], float _va[3], float _vb[3], float _vc[3]);

	///
	void calcLinearFit2D(float _result[2], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void calcLinearFit3D(float _result[3], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void rgbToHsv(float _hsv[3], const float _rgb[3]);

	///
	void hsvToRgb(float _rgb[3], const float _hsv[3]);

} // namespace bx

#include "fpumath.inl"

#endif // BX_FPU_MATH_H_HEADER_GUARD
