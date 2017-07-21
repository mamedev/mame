/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_EASING_H_HEADER_GUARD
#define BX_EASING_H_HEADER_GUARD

#include "fpumath.h"

// Reference:
// http://easings.net/
// http://robertpenner.com/easing/

namespace bx
{
	///
	struct Easing
	{
		enum Enum
		{
			Linear,
			InQuad,
			OutQuad,
			InOutQuad,
			OutInQuad,
			InCubic,
			OutCubic,
			InOutCubic,
			OutInCubic,
			InQuart,
			OutQuart,
			InOutQuart,
			OutInQuart,
			InQuint,
			OutQuint,
			InOutQuint,
			OutInQuint,
			InSine,
			OutSine,
			InOutSine,
			OutInSine,
			InExpo,
			OutExpo,
			InOutExpo,
			OutInExpo,
			InCirc,
			OutCirc,
			InOutCirc,
			OutInCirc,
			InElastic,
			OutElastic,
			InOutElastic,
			OutInElastic,
			InBack,
			OutBack,
			InOutBack,
			OutInBack,
			InBounce,
			OutBounce,
			InOutBounce,
			OutInBounce,

			Count
		};
	};

	///
	typedef float (*EaseFn)(float _t);

	///
	float easeLinear(float _t);

	///
	float easeInQuad(float _t);

	///
	float easeOutQuad(float _t);

	///
	float easeInOutQuad(float _t);

	///
	float easeOutInQuad(float _t);

	///
	float easeInCubic(float _t);

	///
	float easeOutCubic(float _t);

	///
	float easeInOutCubic(float _t);

	///
	float easeOutInCubic(float _t);

	///
	float easeInQuart(float _t);

	///
	float easeOutQuart(float _t);

	///
	float easeInOutQuart(float _t);

	///
	float easeOutInQuart(float _t);

	///
	float easeInQuint(float _t);

	///
	float easeOutQuint(float _t);

	///
	float easeInOutQuint(float _t);

	///
	float easeOutInQuint(float _t);

	///
	float easeInSine(float _t);

	///
	float easeOutSine(float _t);

	///
	float easeInOutSine(float _t);

	///
	float easeOutInSine(float _t);

	///
	float easeInExpo(float _t);

	///
	float easeOutExpo(float _t);

	///
	float easeInOutExpo(float _t);

	///
	float easeOutInExpo(float _t);

	///
	float easeInCirc(float _t);

	///
	float easeOutCirc(float _t);

	///
	float easeInOutCirc(float _t);

	///
	float easeOutInCirc(float _t);

	///
	float easeOutElastic(float _t);

	///
	float easeInElastic(float _t);

	///
	float easeInOutElastic(float _t);

	///
	float easeOutInElastic(float _t);

	///
	float easeInBack(float _t);

	///
	float easeOutBack(float _t);

	///
	float easeInOutBack(float _t);

	///
	float easeOutInBack(float _t);

	///
	float easeOutBounce(float _t);

	///
	float easeInBounce(float _t);

	///
	float easeInOutBounce(float _t);

	///
	float easeOutInBounce(float _t);

} // namespace bx

#include "inline/easing.inl"

#endif // BX_EASING_H_HEADER_GUARD
