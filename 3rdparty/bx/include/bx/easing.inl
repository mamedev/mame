/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_EASING_H_HEADER_GUARD
#	error "Must be included from bx/easing.h!"
#endif // BX_EASING_H_HEADER_GUARD

namespace bx
{
	template<EaseFn ease>
	float easeOut(float _t)
	{
		return 1.0f - ease(1.0f - _t);
	}

	template<EaseFn easeFrom0toH, EaseFn easeFromHto1>
	float easeMix(float _t)
	{
		return _t < 0.5f
			? easeFrom0toH(2.0f*_t)*0.5f
			: easeFromHto1(2.0f*_t - 1.0f)*0.5f + 0.5f
			;
	}

	inline float easeLinear(float _t)
	{
		return _t;
	}

	inline float easeInQuad(float _t)
	{
		return fsq(_t);
	}

	inline float easeOutQuad(float _t)
	{
		return easeOut<easeInQuad>(_t);
	}

	inline float easeInOutQuad(float _t)
	{
		return easeMix<easeInQuad, easeOutQuad>(_t);
	}

	inline float easeOutInQuad(float _t)
	{
		return easeMix<easeOutQuad, easeInQuad>(_t);
	}

	inline float easeInCubic(float _t)
	{
		return _t*_t*_t;
	}

	inline float easeOutCubic(float _t)
	{
		return easeOut<easeInCubic>(_t);
	}

	inline float easeInOutCubic(float _t)
	{
		return easeMix<easeInCubic, easeOutCubic>(_t);
	}

	inline float easeOutInCubic(float _t)
	{
		return easeMix<easeOutCubic, easeInCubic>(_t);
	}

	inline float easeInQuart(float _t)
	{
		return _t*_t*_t*_t;
	}

	inline float easeOutQuart(float _t)
	{
		return easeOut<easeInQuart>(_t);
	}

	inline float easeInOutQuart(float _t)
	{
		return easeMix<easeInQuart, easeOutQuart>(_t);
	}

	inline float easeOutInQuart(float _t)
	{
		return easeMix<easeOutQuart, easeInQuart>(_t);
	}

	inline float easeInQuint(float _t)
	{
		return _t*_t*_t*_t*_t;
	}

	inline float easeOutQuint(float _t)
	{
		return easeOut<easeInQuint>(_t);
	}

	inline float easeInOutQuint(float _t)
	{
		return easeMix<easeInQuint, easeOutQuint>(_t);
	}

	inline float easeOutInQuint(float _t)
	{
		return easeMix<easeOutQuint, easeInQuint>(_t);
	}

	inline float easeInSine(float _t)
	{
		return 1.0f - fcos(_t*piHalf);
	}

	inline float easeOutSine(float _t)
	{
		return easeOut<easeInSine>(_t);
	}

	inline float easeInOutSine(float _t)
	{
		return easeMix<easeInSine, easeOutSine>(_t);
	}

	inline float easeOutInSine(float _t)
	{
		return easeMix<easeOutSine, easeInSine>(_t);
	}

	inline float easeInExpo(float _t)
	{
		return fpow(2.0f, 10.0f * (_t - 1.0f) ) - 0.001f;
	}

	inline float easeOutExpo(float _t)
	{
		return easeOut<easeInExpo>(_t);
	}

	inline float easeInOutExpo(float _t)
	{
		return easeMix<easeInExpo, easeOutExpo>(_t);
	}

	inline float easeOutInExpo(float _t)
	{
		return easeMix<easeOutExpo, easeInExpo>(_t);
	}

	inline float easeInCirc(float _t)
	{
		return -(fsqrt(1.0f - _t*_t) - 1.0f);
	}

	inline float easeOutCirc(float _t)
	{
		return easeOut<easeInCirc>(_t);
	}

	inline float easeInOutCirc(float _t)
	{
		return easeMix<easeInCirc, easeOutCirc>(_t);
	}

	inline float easeOutInCirc(float _t)
	{
		return easeMix<easeOutCirc, easeInCirc>(_t);
	}

	inline float easeOutElastic(float _t)
	{
		return fpow(2.0f, -10.0f*_t)*fsin( (_t-0.3f/4.0f)*(2.0f*pi)/0.3f) + 1.0f;
	}

	inline float easeInElastic(float _t)
	{
		return easeOut<easeOutElastic>(_t);
	}

	inline float easeInOutElastic(float _t)
	{
		return easeMix<easeInElastic, easeOutElastic>(_t);
	}

	inline float easeOutInElastic(float _t)
	{
		return easeMix<easeOutElastic, easeInElastic>(_t);
	}

	inline float easeInBack(float _t)
	{
		return easeInCubic(_t) - _t*fsin(_t*pi);
	}

	inline float easeOutBack(float _t)
	{
		return easeOut<easeInBack>(_t);
	}

	inline float easeInOutBack(float _t)
	{
		return easeMix<easeInBack, easeOutBack>(_t);
	}

	inline float easeOutInBack(float _t)
	{
		return easeMix<easeOutBack, easeInBack>(_t);
	}

	inline float easeOutBounce(float _t)
	{
		if (4.0f/11.0f > _t)
		{
			return 121.0f/16.0f*_t*_t;
		}

		if (8.0f/11.0f > _t)
		{
			return 363.0f/40.0f*_t*_t
				 -  99.0f/10.0f*_t
				 +  17.0f/ 5.0f
				 ;
		}

		if (9.0f/10.0f > _t)
		{
			return  4356.0f/ 361.0f*_t*_t
				 - 35442.0f/1805.0f*_t
				 + 16061.0f/1805.0f
				 ;
		}

		return  54.0f/ 5.0f*_t*_t
			 - 513.0f/25.0f*_t
			 + 268.0f/25.0f
			 ;
	}

	inline float easeInBounce(float _t)
	{
		return easeOut<easeOutBounce>(_t);
	}

	inline float easeInOutBounce(float _t)
	{
		return easeMix<easeInBounce, easeOutBounce>(_t);
	}

	inline float easeOutInBounce(float _t)
	{
		return easeMix<easeOutBounce, easeInBounce>(_t);
	}

} // namespace bx
