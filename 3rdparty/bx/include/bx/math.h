/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

// FPU math lib

#ifndef BX_MATH_H_HEADER_GUARD
#define BX_MATH_H_HEADER_GUARD

#include "bx.h"
#include "uint32_t.h"

namespace bx
{
	constexpr float kPi         = 3.1415926535897932384626433832795f;
	constexpr float kPi2        = 6.2831853071795864769252867665590f;
	constexpr float kInvPi      = 1.0f/kPi;
	constexpr float kPiHalf     = 1.5707963267948966192313216916398f;
	constexpr float kPiQuarter  = 0.7853981633974483096156608458199f;
	constexpr float kSqrt2      = 1.4142135623730950488016887242097f;
	constexpr float kLogNat10   = 2.3025850929940456840179914546844f;
	constexpr float kInvLogNat2 = 1.4426950408889634073599246810019f;
	constexpr float kLogNat2Hi  = 0.6931471805599453094172321214582f;
	constexpr float kLogNat2Lo  = 1.90821492927058770002e-10f;
	constexpr float kE          = 2.7182818284590452353602874713527f;
	constexpr float kNearZero   = 1.0f/float(1 << 28);
	constexpr float kFloatMin   = 1.175494e-38f;
	constexpr float kFloatMax   = 3.402823e+38f;
	extern const float kInfinity;

	///
	typedef float (*LerpFn)(float _a, float _b, float _t);

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
	struct Vec3
	{
		float x, y, z;
	};

	/// Returns converted the argument _deg to radians.
	///
	BX_CONST_FUNC float toRad(float _deg);

	/// Returns converted the argument _rad to degrees.
	///
	BX_CONST_FUNC float toDeg(float _rad);

	/// Reinterprets the bit pattern of _a as uint32_t.
	///
	BX_CONST_FUNC uint32_t floatToBits(float _a);

	/// Reinterprets the bit pattern of _a as float.
	///
	BX_CONST_FUNC float bitsToFloat(uint32_t _a);

	/// Reinterprets the bit pattern of _a as uint64_t.
	///
	BX_CONST_FUNC uint64_t doubleToBits(double _a);

	/// Reinterprets the bit pattern of _a as double.
	///
	BX_CONST_FUNC double bitsToDouble(uint64_t _a);

	/// Returns sortable floating point value.
	///
	BX_CONST_FUNC uint32_t floatFlip(uint32_t _value);

	/// Returns true if _f is a number that is NaN.
	///
	BX_CONST_FUNC bool isNan(float _f);

	/// Returns true if _f is a number that is NaN.
	///
	BX_CONST_FUNC bool isNan(double _f);

	/// Returns true if _f is not infinite and is not a NaN.
	///
	BX_CONST_FUNC bool isFinite(float _f);

	/// Returns true if _f is not infinite and is not a NaN.
	///
	BX_CONST_FUNC bool isFinite(double _f);

	/// Returns true if _f is infinite and is not a NaN.
	///
	BX_CONST_FUNC bool isInfinite(float _f);

	/// Returns true if _f is infinite and is not a NaN.
	///
	BX_CONST_FUNC bool isInfinite(double _f);

	/// Returns the largest integer value not greater than _f.
	///
	BX_CONST_FUNC float floor(float _f);

	/// Returns the smallest integer value not less than _f.
	///
	BX_CONST_FUNC float ceil(float _f);

	/// Returns the nearest integer value to _f, rounding halfway cases away from zero,
	///
	BX_CONST_FUNC float round(float _f);

	/// Returns linear interpolation between two values _a and _b.
	///
	BX_CONSTEXPR_FUNC float lerp(float _a, float _b, float _t);

	/// Returns the sign of _a.
	///
	BX_CONSTEXPR_FUNC float sign(float _a);

	/// Returns the absolute of _a.
	///
	BX_CONSTEXPR_FUNC float abs(float _a);

	/// Returns the square of _a.
	///
	BX_CONSTEXPR_FUNC float square(float _a);

	/// Returns the cosine of the argument _a.
	///
	BX_CONST_FUNC float sin(float _a);

	/// Returns hyperbolic sine of the argument _a.
	///
	BX_CONST_FUNC float sinh(float _a);

	/// Returns radian angle between -pi/2 and +pi/2 whose sine is _a.
	///
	BX_CONST_FUNC float asin(float _a);

	/// Returns the cosine of the argument _a.
	///
	BX_CONST_FUNC float cos(float _a);

	/// Returns hyperbolic cosine of the argument _a.
	///
	BX_CONST_FUNC float cosh(float _a);

	/// Returns radian angle between 0 and pi whose cosine is _a.
	///
	BX_CONST_FUNC float acos(float _a);

	/// Returns the circular tangent of the radian argument _a.
	///
	BX_CONST_FUNC float tan(float _a);

	/// Returns hyperbolic tangent of the argument _a.
	///
	BX_CONST_FUNC float tanh(float _a);

	/// Returns radian angle between -pi/2 and +pi/2 whose tangent is _a.
	///
	BX_CONST_FUNC float atan(float _a);

	/// Retruns the inverse tangent of _y/_x.
	///
	BX_CONST_FUNC float atan2(float _y, float _x);

	/// Computes _a raised to the _b power.
	///
	BX_CONST_FUNC float pow(float _a, float _b);

	/// Returns the result of multiplying _a by 2 raised to the power of the exponent.
	///
	BX_CONST_FUNC float ldexp(float _a, int32_t _b);

	/// Returns decomposed given floating point value _a into a normalized fraction and
	/// an integral power of two.
	///
	float frexp(float _a, int32_t* _outExp);

	/// Returns e (2.71828...) raised to the _a power.
	///
	BX_CONST_FUNC float exp(float _a);

	/// Returns 2 raised to the _a power.
	///
	BX_CONST_FUNC float exp2(float _a);

	/// Returns the base e (2.71828...) logarithm of _a.
	///
	BX_CONST_FUNC float log(float _a);

	/// Returns the base 2 logarithm of _a.
	///
	BX_CONST_FUNC float log2(float _a);

	/// Returns the square root of _a.
	///
	BX_CONST_FUNC float sqrt(float _a);

	/// Returns reciprocal square root of _a.
	///
	BX_CONST_FUNC float rsqrt(float _a);

	/// Returns the nearest integer not greater in magnitude than _a.
	///
	BX_CONSTEXPR_FUNC float trunc(float _a);

	/// Returns the fractional (or decimal) part of _a, which is greater than or equal to 0
	/// and less than 1.
	///
	BX_CONSTEXPR_FUNC float fract(float _a);

	/// Returns result of multipla and add (_a * _b + _c).
	///
	BX_CONSTEXPR_FUNC float mad(float _a, float _b, float _c);

	/// Returns the floating-point remainder of the division operation _a/_b.
	///
	BX_CONST_FUNC float mod(float _a, float _b);

	///
	BX_CONSTEXPR_FUNC bool equal(float _a, float _b, float _epsilon);

	///
	BX_CONST_FUNC bool equal(const float* _a, const float* _b, uint32_t _num, float _epsilon);

	///
	BX_CONST_FUNC float wrap(float _a, float _wrap);

	///
	BX_CONSTEXPR_FUNC float step(float _edge, float _a);

	///
	BX_CONSTEXPR_FUNC float pulse(float _a, float _start, float _end);

	///
	BX_CONSTEXPR_FUNC float smoothStep(float _a);

	///
	BX_CONSTEXPR_FUNC float bias(float _time, float _bias);

	///
	BX_CONSTEXPR_FUNC float gain(float _time, float _gain);

	///
	BX_CONST_FUNC float angleDiff(float _a, float _b);

	/// Returns shortest distance linear interpolation between two angles.
	///
	BX_CONST_FUNC float angleLerp(float _a, float _b, float _t);

	///
	Vec3 load(const void* _ptr);

	///
	void store(void* _ptr, const Vec3& _a);

	///
	BX_CONSTEXPR_FUNC Vec3 abs(const Vec3&  _a);

	///
	BX_CONSTEXPR_FUNC Vec3 neg(const Vec3&  _a);

	///
	BX_CONSTEXPR_FUNC Vec3 add(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 add(const Vec3&  _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 sub(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 sub(const Vec3&  _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mul(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mul(const Vec3&  _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mad(const Vec3& _a, const Vec3& _b, const Vec3& _c);

	///
	BX_CONSTEXPR_FUNC float dot(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 cross(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONST_FUNC float length(const Vec3&  _a);

	///
	BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3&  _a, const Vec3&  _b, float _t);

	///
	BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3&  _a, const Vec3&  _b, const Vec3&  _t);

	///
	BX_CONST_FUNC Vec3 normalize(const Vec3&  _a);

	///
	BX_CONSTEXPR_FUNC Vec3 min(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 max(const Vec3&  _a, const Vec3&  _b);

	///
	BX_CONSTEXPR_FUNC Vec3 rcp(const Vec3&  _a);

	///
	void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3& _n);

	///
	void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3& _n, float _angle);

	///
	BX_CONST_FUNC Vec3 fromLatLong(float _u, float _v);

	///
	void toLatLong(float* _outU, float* _outV, const Vec3&  _dir);

	///
	void vec3Add(float* _result, const float* _a, const float* _b);

	///
	void vec3Sub(float* _result, const float* _a, const float* _b);

	///
	void vec3Mul(float* _result, const float* _a, const float* _b);

	///
	void vec3Mul(float* _result, const float* _a, float _b);

	///
	float vec3Dot(const float* _a, const float* _b);

	///
	void vec3Cross(float* _result, const float* _a, const float* _b);

	///
	float vec3Length(const float* _a);

	///
	float vec3Norm(float* _result, const float* _a);

	/// Calculate tangent frame from normal.
	///
	void vec3TangentFrame(const float* _n, float* _t, float* _b);

	/// Calculate tangent frame from normal and angle.
	///
	void vec3TangentFrame(const float* _n, float* _t, float* _b, float _angle);

	///
	void vec3FromLatLong(float* _vec, float _u, float _v);

	/// Convert direction to 2D latitude and longitude.
	///
	/// @param[out] _outU U-coordinate.
	/// @param[out] _outV V-coordinate.
	/// @param[in]  _dir Normalized direction vector.
	///
	void vec3ToLatLong(float* _outU, float* _outV, const float* _dir);

	///
	void quatIdentity(float* _result);

	///
	void quatMove(float* _result, const float* _a);

	///
	void quatMulXYZ(float* _result, const float* _qa, const float* _qb);

	///
	void quatMul(float* _result, const float* _qa, const float* _qb);

	///
	void quatInvert(float* _result, const float* _quat);

	///
	float quatDot(const float* _a, const float* _b);

	///
	void quatNorm(float* _result, const float* _quat);

	///
	void quatToEuler(float* _result, const float* _quat);

	///
	void quatRotateAxis(float* _result, const float* _axis, float _angle);

	///
	void quatRotateX(float* _result, float _ax);

	///
	void quatRotateY(float* _result, float _ay);

	///
	void quatRotateZ(float* _result, float _az);

	///
	void vec3MulQuat(float* _result, const float* _vec, const float* _quat);

	///
	void mtxIdentity(float* _result);

	///
	void mtxTranslate(float* _result, float _tx, float _ty, float _tz);

	///
	void mtxScale(float* _result, float _sx, float _sy, float _sz);

	///
	void mtxScale(float* _result, float _scale);

	///
	void mtxFromNormal(float* _result, const float* _normal, float _scale, const float* _pos);

	///
	void mtxFromNormal(float* _result, const float* _normal, float _scale, const float* _pos, float _angle);

	///
	void mtxQuat(float* _result, const float* _quat);

	///
	void mtxQuatTranslation(float* _result, const float* _quat, const float* _translation);

	///
	void mtxQuatTranslationHMD(float* _result, const float* _quat, const float* _translation);

	///
	void mtxLookAtLh(float* _result, const Vec3& _eye, const Vec3& _at, const Vec3& _up = { 0.0f, 1.0f, 0.0f });

	///
	void mtxLookAtRh(float* _result, const Vec3& _eye, const Vec3& _at, const Vec3& _up = { 0.0f, 1.0f, 0.0f });

	///
	void mtxLookAt(float* _result, const Vec3& _eye, const Vec3& _at, const Vec3& _up = { 0.0f, 1.0f, 0.0f });

	///
	void mtxProj(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc);

	///
	void mtxProj(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc);

	///
	void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);

	///
	void mtxProjLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc);

	///
	void mtxProjLh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc);

	///
	void mtxProjLh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);

	///
	void mtxProjRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, float _far, bool _oglNdc);

	///
	void mtxProjRh(float* _result, const float _fov[4], float _near, float _far, bool _oglNdc);

	///
	void mtxProjRh(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);

	///
	void mtxProjInf(float* _result, const float _fov[4], float _near, bool _oglNdc);

	///
	void mtxProjInf(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc);

	///
	void mtxProjInf(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc);

	///
	void mtxProjInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc);

	///
	void mtxProjInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc);

	///
	void mtxProjInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc);

	///
	void mtxProjInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc);

	///
	void mtxProjInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc);

	///
	void mtxProjInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc);

	///
	void mtxProjRevInfLh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc);

	///
	void mtxProjRevInfLh(float* _result, const float _fov[4], float _near, bool _oglNdc);

	///
	void mtxProjRevInfLh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc);

	///
	void mtxProjRevInfRh(float* _result, float _ut, float _dt, float _lt, float _rt, float _near, bool _oglNdc);

	///
	void mtxProjRevInfRh(float* _result, const float _fov[4], float _near, bool _oglNdc);

	///
	void mtxProjRevInfRh(float* _result, float _fovy, float _aspect, float _near, bool _oglNdc);

	///
	void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc);

	///
	void mtxOrthoLh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc);

	///
	void mtxOrthoRh(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _oglNdc);

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
	void vec3MulMtx(float* _result, const float* _vec, const float* _mat);

	///
	void vec3MulMtxXyz0(float* _result, const float* _vec, const float* _mat);

	///
	void vec3MulMtxH(float* _result, const float* _vec, const float* _mat);

	///
	void vec4Mul(float* _result, const float* _a, const float* _b);

	///
	void vec4Mul(float* _result, const float* _a, float _b);

	///
	void vec4MulMtx(float* _result, const float* _vec, const float* _mat);

	///
	void mtxMul(float* _result, const float* _a, const float* _b);

	///
	void mtxTranspose(float* _result, const float* _a);

	///
	void mtx3Inverse(float* _result, const float* _a);

	///
	void mtxInverse(float* _result, const float* _a);

	/// Convert LH to RH projection matrix and vice versa.
	///
	void mtxProjFlipHandedness(float* _dst, const float* _src);

	/// Convert LH to RH view matrix and vice versa.
	///
	void mtxViewFlipHandedness(float* _dst, const float* _src);

	///
	void calcNormal(float _result[3], const float _va[3], const float _vb[3], const float _vc[3]);

	///
	void calcPlane(float _result[4], const float _va[3], const float _vb[3], const float _vc[3]);

	///
	void calcPlane(float _result[4], const float _normal[3], const float _pos[3]);

	///
	void calcLinearFit2D(float _result[2], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void calcLinearFit3D(float _result[3], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void rgbToHsv(float _hsv[3], const float _rgb[3]);

	///
	void hsvToRgb(float _rgb[3], const float _hsv[3]);

	///
	BX_CONST_FUNC float toLinear(float _a);

	///
	BX_CONST_FUNC float toGamma(float _a);

} // namespace bx

#include "inline/math.inl"

#endif // BX_MATH_H_HEADER_GUARD
