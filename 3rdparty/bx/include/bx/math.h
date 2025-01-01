/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_MATH_H_HEADER_GUARD
#define BX_MATH_H_HEADER_GUARD

#include "bx.h"
#include "uint32_t.h"

namespace bx
{
	///
	typedef float (*LerpFn)(float _a, float _b, float _t);

	///
	struct Handedness
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
		Vec3() = delete;

		///
		Vec3(InitNoneTag);

		///
		constexpr Vec3(InitZeroTag);

		///
		constexpr Vec3(InitIdentityTag);

		///
		explicit constexpr Vec3(float _v);

		///
		constexpr Vec3(float _x, float _y, float _z);

		float x, y, z;
	};

	///
	struct Plane
	{
		Plane() = delete;

		///
		Plane(InitNoneTag);

		///
		constexpr Plane(InitZeroTag);

		///
		constexpr Plane(InitIdentityTag);

		///
		constexpr Plane(Vec3 _normal, float _dist);

		Vec3  normal;
		float dist;
	};

	///
	struct Quaternion
	{
		Quaternion() = delete;

		///
		Quaternion(InitNoneTag);

		///
		constexpr Quaternion(InitZeroTag);

		///
		constexpr Quaternion(InitIdentityTag);

		///
		constexpr Quaternion(float _x, float _y, float _z, float _w);

		float x, y, z, w;
	};

	/// Returns converted the argument _deg to radians.
	///
	BX_CONSTEXPR_FUNC float toRad(float _deg);

	/// Returns converted the argument _rad to degrees.
	///
	BX_CONSTEXPR_FUNC float toDeg(float _rad);

	/// Reinterprets the bit pattern of _a as uint32_t.
	///
	BX_CONSTEXPR_FUNC uint32_t floatToBits(float _a);

	/// Reinterprets the bit pattern of _a as float.
	///
	BX_CONSTEXPR_FUNC float bitsToFloat(uint32_t _a);

	/// Reinterprets the bit pattern of _a as uint64_t.
	///
	BX_CONSTEXPR_FUNC uint64_t doubleToBits(double _a);

	/// Reinterprets the bit pattern of _a as double.
	///
	BX_CONSTEXPR_FUNC double bitsToDouble(uint64_t _a);

	/// Returns sortable floating point value.
	///
	BX_CONSTEXPR_FUNC uint32_t floatFlip(uint32_t _value);

	/// Returns true if _f is a number that is NaN.
	///
	BX_CONSTEXPR_FUNC bool isNan(float _f);

	/// Returns true if _f is a number that is NaN.
	///
	BX_CONSTEXPR_FUNC bool isNan(double _f);

	/// Returns true if _f is not infinite and is not a NaN.
	///
	BX_CONSTEXPR_FUNC bool isFinite(float _f);

	/// Returns true if _f is not infinite and is not a NaN.
	///
	BX_CONSTEXPR_FUNC bool isFinite(double _f);

	/// Returns true if _f is infinite and is not a NaN.
	///
	BX_CONSTEXPR_FUNC bool isInfinite(float _f);

	/// Returns true if _f is infinite and is not a NaN.
	///
	BX_CONSTEXPR_FUNC bool isInfinite(double _f);

	/// Returns the largest integer value not greater than _f.
	///
	BX_CONSTEXPR_FUNC float floor(float _f);

	/// Returns the smallest integer value not less than _f.
	///
	BX_CONSTEXPR_FUNC float ceil(float _f);

	/// Returns the nearest integer value to _f, rounding halfway cases away from zero,
	///
	BX_CONSTEXPR_FUNC float round(float _f);

	/// Returns linear interpolation between two values _a and _b.
	///
	BX_CONSTEXPR_FUNC float lerp(float _a, float _b, float _t);

	/// Returns inverse linear interpolation of _value between two values _a and _b.
	///
	BX_CONSTEXPR_FUNC float invLerp(float _a, float _b, float _value);

	/// Extracts the sign of value `_a`.
	///
	/// @param[in] _a Value.
	///
	/// @returns -1 if `_a` is less than zero, 0 if `_a` is equal to 0, or +1 if `_a` is greater than zero.
	///
	BX_CONSTEXPR_FUNC float sign(float _a);

	/// Returns `true` if the velue `_a` is negative.
	///
	/// @param[in] _a Value.
	///
	/// @returns `true` if `_a` is less than zero, otherwise returns `false`.
	///
	BX_CONSTEXPR_FUNC bool signBit(float _a);

	/// Returns value with the magnitude `_value`, and the sign of `_sign`.
	///
	/// @param[in] _value Value.
	/// @param[in] _sign Sign.
	///
	/// @returns Value with the magnitude `_value`, and the sign of `_sign`.
	///
	BX_CONSTEXPR_FUNC float copySign(float _value, float _sign);

	/// Returns the absolute of _a.
	///
	BX_CONSTEXPR_FUNC float abs(float _a);

	/// Returns the square of _a.
	///
	BX_CONSTEXPR_FUNC float square(float _a);

	/// Returns the both sine and cosine of the argument _a.
	///
	/// @remarks The function calculates cosine, and then approximates sine based on the cosine
	///   result. Therefore calculation of sine is less accurate than calling `bx::sin` function.
	///
	void sinCosApprox(float& _outSinApprox, float& _outCos, float _a);

	/// Returns the sine of the argument _a.
	///
	BX_CONSTEXPR_FUNC float sin(float _a);

	/// Returns hyperbolic sine of the argument _a.
	///
	BX_CONSTEXPR_FUNC float sinh(float _a);

	/// Returns radian angle between -pi/2 and +pi/2 whose sine is _a.
	///
	BX_CONSTEXPR_FUNC float asin(float _a);

	/// Returns the cosine of the argument _a.
	///
	BX_CONSTEXPR_FUNC float cos(float _a);

	/// Returns hyperbolic cosine of the argument _a.
	///
	BX_CONSTEXPR_FUNC float cosh(float _a);

	/// Returns radian angle between 0 and pi whose cosine is _a.
	///
	BX_CONSTEXPR_FUNC float acos(float _a);

	/// Returns the circular tangent of the radian argument _a.
	///
	BX_CONSTEXPR_FUNC float tan(float _a);

	/// Returns hyperbolic tangent of the argument _a.
	///
	BX_CONSTEXPR_FUNC float tanh(float _a);

	/// Returns radian angle between -pi/2 and +pi/2 whose tangent is _a.
	///
	BX_CONSTEXPR_FUNC float atan(float _a);

	/// Returns the inverse tangent of _y/_x.
	///
	BX_CONSTEXPR_FUNC float atan2(float _y, float _x);

	/// Computes _a raised to the _b power.
	///
	BX_CONSTEXPR_FUNC float pow(float _a, float _b);

	/// Returns the result of multiplying _a by 2 raised to the power of the exponent `_a * (2^_b)`.
	///
	BX_CONSTEXPR_FUNC float ldexp(float _a, int32_t _b);

	/// Returns decomposed given floating point value _a into a normalized fraction and
	/// an integral power of two.
	///
	float frexp(float _a, int32_t* _outExp);

	/// Returns e (2.71828...) raised to the _a power.
	///
	BX_CONSTEXPR_FUNC float exp(float _a);

	/// Returns 2 raised to the _a power.
	///
	BX_CONSTEXPR_FUNC float exp2(float _a);

	/// Returns the base e (2.71828...) logarithm of _a.
	///
	BX_CONSTEXPR_FUNC float log(float _a);

	/// Returns the base 2 logarithm of _a.
	///
	BX_CONSTEXPR_FUNC float log2(float _a);

	/// Count number of bits set.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t countBits(Ty _val);

	/// Count number of leading zeros.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t countLeadingZeros(Ty _val);

	/// Count number of trailing zeros.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t countTrailingZeros(Ty _val);

	/// Find first set.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t findFirstSet(Ty _val);

	/// Find last set.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t findLastSet(Ty _val);

	/// Returns the next smallest integer base 2 logarithm of _a.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t ceilLog2(Ty _a);

	/// Returns the next biggest integer base 2 logarithm of _a.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC uint8_t floorLog2(Ty _a);

	/// Returns the next smallest power of two value.
	///
	template<typename Ty>
	BX_CONSTEXPR_FUNC Ty nextPow2(Ty _a);

	/// Returns the square root of _a.
	///
	BX_CONSTEXPR_FUNC float sqrt(float _a);

	/// Returns reciprocal square root of _a.
	///
	BX_CONSTEXPR_FUNC float rsqrt(float _a);

	/// Returns the nearest integer not greater in magnitude than _a.
	///
	BX_CONSTEXPR_FUNC float trunc(float _a);

	/// Returns the fractional (or decimal) part of _a, which is greater than or equal to 0
	/// and less than 1.
	///
	BX_CONSTEXPR_FUNC float fract(float _a);

	/// Returns result of negated multiply-sub operation -(_a * _b - _c) -> _c - _a * _b.
	///
	BX_CONSTEXPR_FUNC float nms(float _a, float _b, float _c);

	/// Returns result of addition (_a + _b).
	///
	BX_CONSTEXPR_FUNC float add(float _a, float _b);

	/// Returns result of subtracion (_a - _b).
	///
	BX_CONSTEXPR_FUNC float sub(float _a, float _b);

	/// Returns result of multiply (_a * _b).
	///
	BX_CONSTEXPR_FUNC float mul(float _a, float _b);

	/// Returns result of multiply and add (_a * _b + _c).
	///
	BX_CONSTEXPR_FUNC float mad(float _a, float _b, float _c);

	/// Returns reciprocal of _a.
	///
	BX_CONSTEXPR_FUNC float rcp(float _a);

	/// Returns reciprocal of _a. Avoids divide by zero.
	///
	BX_CONSTEXPR_FUNC float rcpSafe(float _a);

	/// Returns result of division (_a / _b).
	///
	BX_CONSTEXPR_FUNC float div(float _a, float _b);

	/// Returns result of division (_a / _b). Avoids divide by zero.
	///
	BX_CONSTEXPR_FUNC float divSafe(float _a, float _b);

	/// Returns result of division (_a / _b), and rounds upwards.
	///
	BX_CONSTEXPR_FUNC float ceilDiv(float _a, float _b);

	/// Returns result of division (_a / _b), and rounds upwards. Avoids divide by zero.
	///
	BX_CONSTEXPR_FUNC float ceilDivSafe(float _a, float _b);

	/// Returns the floating-point remainder of the division operation _a/_b.
	///
	BX_CONSTEXPR_FUNC float mod(float _a, float _b);

	///
	BX_CONSTEXPR_FUNC bool isEqual(float _a, float _b, float _epsilon);

	///
	BX_CONST_FUNC bool isEqual(const float* _a, const float* _b, uint32_t _num, float _epsilon);

	///
	BX_CONSTEXPR_FUNC float wrap(float _a, float _wrap);

	///
	BX_CONSTEXPR_FUNC float step(float _edge, float _a);

	///
	BX_CONSTEXPR_FUNC float pulse(float _a, float _start, float _end);

	///
	BX_CONSTEXPR_FUNC float smoothStep(float _a);

	///
	BX_CONSTEXPR_FUNC float invSmoothStep(float _a);

	///
	BX_CONSTEXPR_FUNC float bias(float _time, float _bias);

	///
	BX_CONSTEXPR_FUNC float gain(float _time, float _gain);

	///
	BX_CONSTEXPR_FUNC float angleDiff(float _a, float _b);

	/// Returns shortest distance linear interpolation between two angles.
	///
	BX_CONSTEXPR_FUNC float angleLerp(float _a, float _b, float _t);

	///
	template<typename Ty>
	Ty load(const void* _ptr);

	///
	template<typename Ty>
	void store(void* _ptr, const Ty& _a);

	///
	BX_CONSTEXPR_FUNC Vec3 round(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC Vec3 abs(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC Vec3 neg(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC Vec3 add(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 add(const Vec3 _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 sub(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 sub(const Vec3 _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 div(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 divSafe(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 div(const Vec3 _a, float _b);

	///
	BX_CONSTEXPR_FUNC Vec3 divSafe(const Vec3 _a, float _b);

	/// Returns result of negated multiply-sub operation -(_a * _b - _c) -> _c - _a * _b.
	///
	BX_CONSTEXPR_FUNC Vec3 nms(const Vec3 _a, const float _b, const Vec3 _c);

	/// Returns result of negated multiply-sub operation -(_a * _b - _c) -> _c - _a * _b.
	///
	BX_CONSTEXPR_FUNC Vec3 nms(const Vec3 _a, const Vec3 _b, const Vec3 _c);

	///
	BX_CONSTEXPR_FUNC Vec3 mad(const Vec3 _a, const float _b, const Vec3 _c);

	///
	BX_CONSTEXPR_FUNC Vec3 mad(const Vec3 _a, const Vec3 _b, const Vec3 _c);

	///
	BX_CONSTEXPR_FUNC float dot(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 cross(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC float length(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC float distanceSq(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC float distance(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3 _a, const Vec3 _b, float _t);

	///
	BX_CONSTEXPR_FUNC Vec3 lerp(const Vec3 _a, const Vec3 _b, const Vec3 _t);

	///
	BX_CONSTEXPR_FUNC Vec3 normalize(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC Vec3 min(const Vec3 _a, const Vec3 _b);

	///
	BX_CONSTEXPR_FUNC Vec3 max(const Vec3 _a, const Vec3 _b);

	/// Returns component wise reciprocal of _a.
	///
	BX_CONSTEXPR_FUNC Vec3 rcp(const Vec3 _a);

	/// Returns component wise reciprocal of _a.
	///
	BX_CONSTEXPR_FUNC Vec3 rcpSafe(const Vec3 _a);

	///
	BX_CONSTEXPR_FUNC bool isEqual(const Vec3 _a, const Vec3 _b, float _epsilon);

	///
	void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3 _n);

	///
	void calcTangentFrame(Vec3& _outT, Vec3& _outB, const Vec3 _n, float _angle);

	///
	BX_CONSTEXPR_FUNC Vec3 fromLatLong(float _u, float _v);

	///
	void toLatLong(float* _outU, float* _outV, const Vec3 _dir);

	///
	BX_CONSTEXPR_FUNC Quaternion invert(const Quaternion _a);

	///
	BX_CONSTEXPR_FUNC Vec3 mulXyz(const Quaternion _a, const Quaternion _b);

	///
	BX_CONSTEXPR_FUNC Quaternion add(const Quaternion _a, const Quaternion _b);

	///
	BX_CONSTEXPR_FUNC Quaternion sub(const Quaternion _a, const Quaternion _b);

	///
	BX_CONSTEXPR_FUNC Quaternion mul(const Quaternion _a, float _b);

	///
	BX_CONSTEXPR_FUNC Quaternion mul(const Quaternion _a, const Quaternion _b);

	///
	BX_CONSTEXPR_FUNC Vec3 mul(const Vec3 _v, const Quaternion _q);

	///
	BX_CONSTEXPR_FUNC float dot(const Quaternion _a, const Quaternion _b);

	///
	BX_CONSTEXPR_FUNC Quaternion normalize(const Quaternion _a);

	///
	BX_CONSTEXPR_FUNC Quaternion lerp(const Quaternion _a, const Quaternion _b, float _t);

	///
	BX_CONST_FUNC Quaternion fromEuler(const Vec3 _euler);

	///
	BX_CONST_FUNC Vec3 toEuler(const Quaternion _a);

	///
	BX_CONST_FUNC Vec3 toXAxis(const Quaternion _a);

	///
	BX_CONST_FUNC Vec3 toYAxis(const Quaternion _a);

	///
	BX_CONST_FUNC Vec3 toZAxis(const Quaternion _a);

	///
	BX_CONST_FUNC Quaternion fromAxisAngle(const Vec3 _axis, float _angle);

	///
	void toAxisAngle(Vec3& _outAxis, float& _outAngle, const Quaternion _a);

	///
	BX_CONST_FUNC Quaternion rotateX(float _ax);

	///
	BX_CONST_FUNC Quaternion rotateY(float _ay);

	///
	BX_CONST_FUNC Quaternion rotateZ(float _az);

	///
	BX_CONSTEXPR_FUNC bool isEqual(const Quaternion _a, const Quaternion _b, float _epsilon);

	///
	void mtxIdentity(float* _result);

	///
	void mtxTranslate(float* _result, float _tx, float _ty, float _tz);

	///
	void mtxScale(float* _result, float _sx, float _sy, float _sz);

	///
	void mtxScale(float* _result, float _scale);

	///
	void mtxFromNormal(
		  float* _result
		, const Vec3& _normal
		, float _scale
		, const Vec3& _pos
		);

	///
	void mtxFromNormal(
		  float* _result
		, const Vec3& _normal
		, float _scale
		, const Vec3& _pos
		, float _angle
		);

	///
	void mtxFromQuaternion(float* _result, const Quaternion& _rotation);

	///
	void mtxFromQuaternion(float* _result, const Quaternion& _rotation, const Vec3& _translation);

	///
	void mtxLookAt(
		  float* _result
		, const Vec3& _eye
		, const Vec3& _at
		, const Vec3& _up = { 0.0f, 1.0f, 0.0f }
		, Handedness::Enum _handedness = Handedness::Left
		);

	///
	void mtxProj(
		  float* _result
		, float _ut
		, float _dt
		, float _lt
		, float _rt
		, float _near
		, float _far
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		);

	///
	void mtxProj(
		  float* _result
		, const float _fov[4]
		, float _near
		, float _far
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		);

	///
	void mtxProj(
		  float* _result
		, float _fovy
		, float _aspect
		, float _near
		, float _far
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		);

	///
	void mtxProjInf(
		  float* _result
		, const float _fov[4]
		, float _near
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		, NearFar::Enum _nearFar = NearFar::Default
		);

	///
	void mtxProjInf(
		  float* _result
		, float _ut
		, float _dt
		, float _lt
		, float _rt
		, float _near
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		, NearFar::Enum _nearFar = NearFar::Default
		);

	///
	void mtxProjInf(
		  float* _result
		, float _fovy
		, float _aspect
		, float _near
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		, NearFar::Enum _nearFar = NearFar::Default
		);

	///
	void mtxOrtho(
		  float* _result
		, float _left
		, float _right
		, float _bottom
		, float _top
		, float _near
		, float _far
		, float _offset
		, bool _homogeneousNdc
		, Handedness::Enum _handedness = Handedness::Left
		);

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
	void mtxSRT(
		  float* _result
		, float _sx
		, float _sy
		, float _sz
		, float _ax
		, float _ay
		, float _az
		, float _tx
		, float _ty
		, float _tz
		);

	///
	Vec3 mul(const Vec3& _vec, const float* _mat);

	///
	Vec3 mulXyz0(const Vec3& _vec, const float* _mat);

	///
	Vec3 mulH(const Vec3& _vec, const float* _mat);

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

	///
	void mtx3Cofactor(float* _result, const float* _a);

	///
	void mtxCofactor(float* _result, const float* _a);

	///
	Vec3 calcNormal(const Vec3& _va, const Vec3& _vb, const Vec3& _vc);

	///
	void calcPlane(Plane& _outPlane, const Vec3& _va, const Vec3& _vb, const Vec3& _vc);

	///
	void calcPlane(Plane& _outPlane, const Vec3& _normal, const Vec3& _pos);

	///
	BX_CONSTEXPR_FUNC float distance(const Plane& _plane, const Vec3& _pos);

	///
	BX_CONSTEXPR_FUNC bool isEqual(const Plane& _a, const Plane& _b, float _epsilon);

	///
	void calcLinearFit2D(float _result[2], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void calcLinearFit3D(float _result[3], const void* _points, uint32_t _stride, uint32_t _numPoints);

	///
	void rgbToHsv(float _hsv[3], const float _rgb[3]);

	///
	void hsvToRgb(float _rgb[3], const float _hsv[3]);

	///
	BX_CONSTEXPR_FUNC float toLinear(float _a);

	///
	BX_CONSTEXPR_FUNC float toGamma(float _a);

} // namespace bx

#include "inline/math.inl"

#endif // BX_MATH_H_HEADER_GUARD
