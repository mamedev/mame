/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_EASING_H_HEADER_GUARD
#	error "Must be included from bx/easing.h!"
#endif // BX_EASING_H_HEADER_GUARD

 // Reference(s):
 // - https://web.archive.org/web/20181126040153/https://easings.net/
 // - https://web.archive.org/web/20181126040212/http://robertpenner.com/easing/
 //
namespace bx
{
	template<EaseFn ease>
	BX_CONST_FUNC float easeOut(float _t)
	{
		return 1.0f - ease(1.0f - _t);
	}

	template<EaseFn easeFrom0toH, EaseFn easeFromHto1>
	BX_CONST_FUNC float easeMix(float _t)
	{
		return _t < 0.5f
			? easeFrom0toH(2.0f*_t)*0.5f
			: easeFromHto1(2.0f*_t - 1.0f)*0.5f + 0.5f
			;
	}

	inline BX_CONST_FUNC float easeLinear(float _t)
	{
		return _t;
	}

	inline BX_CONST_FUNC float easeStep(float _t)
	{
		return _t < 0.5f ? 0.0f : 1.0f;
	}

	inline BX_CONST_FUNC float easeSmoothStep(float _t)
	{
		return square(_t)*(3.0f - 2.0f*_t);
	}

	inline BX_CONST_FUNC float easeInQuad(float _t)
	{
		return square(_t);
	}

	inline BX_CONST_FUNC float easeOutQuad(float _t)
	{
		return easeOut<easeInQuad>(_t);
	}

	inline BX_CONST_FUNC float easeInOutQuad(float _t)
	{
		return easeMix<easeInQuad, easeOutQuad>(_t);
	}

	inline BX_CONST_FUNC float easeOutInQuad(float _t)
	{
		return easeMix<easeOutQuad, easeInQuad>(_t);
	}

	inline BX_CONST_FUNC float easeInCubic(float _t)
	{
		return _t*_t*_t;
	}

	inline BX_CONST_FUNC float easeOutCubic(float _t)
	{
		return easeOut<easeInCubic>(_t);
	}

	inline BX_CONST_FUNC float easeInOutCubic(float _t)
	{
		return easeMix<easeInCubic, easeOutCubic>(_t);
	}

	inline BX_CONST_FUNC float easeOutInCubic(float _t)
	{
		return easeMix<easeOutCubic, easeInCubic>(_t);
	}

	inline BX_CONST_FUNC float easeInQuart(float _t)
	{
		return _t*_t*_t*_t;
	}

	inline BX_CONST_FUNC float easeOutQuart(float _t)
	{
		return easeOut<easeInQuart>(_t);
	}

	inline BX_CONST_FUNC float easeInOutQuart(float _t)
	{
		return easeMix<easeInQuart, easeOutQuart>(_t);
	}

	inline BX_CONST_FUNC float easeOutInQuart(float _t)
	{
		return easeMix<easeOutQuart, easeInQuart>(_t);
	}

	inline BX_CONST_FUNC float easeInQuint(float _t)
	{
		return _t*_t*_t*_t*_t;
	}

	inline BX_CONST_FUNC float easeOutQuint(float _t)
	{
		return easeOut<easeInQuint>(_t);
	}

	inline BX_CONST_FUNC float easeInOutQuint(float _t)
	{
		return easeMix<easeInQuint, easeOutQuint>(_t);
	}

	inline BX_CONST_FUNC float easeOutInQuint(float _t)
	{
		return easeMix<easeOutQuint, easeInQuint>(_t);
	}

	inline BX_CONST_FUNC float easeInSine(float _t)
	{
		return 1.0f - cos(_t*kPiHalf);
	}

	inline BX_CONST_FUNC float easeOutSine(float _t)
	{
		return easeOut<easeInSine>(_t);
	}

	inline BX_CONST_FUNC float easeInOutSine(float _t)
	{
		return easeMix<easeInSine, easeOutSine>(_t);
	}

	inline BX_CONST_FUNC float easeOutInSine(float _t)
	{
		return easeMix<easeOutSine, easeInSine>(_t);
	}

	inline BX_CONST_FUNC float easeInExpo(float _t)
	{
		return pow(2.0f, 10.0f * (_t - 1.0f) ) - 0.001f;
	}

	inline BX_CONST_FUNC float easeOutExpo(float _t)
	{
		return easeOut<easeInExpo>(_t);
	}

	inline BX_CONST_FUNC float easeInOutExpo(float _t)
	{
		return easeMix<easeInExpo, easeOutExpo>(_t);
	}

	inline BX_CONST_FUNC float easeOutInExpo(float _t)
	{
		return easeMix<easeOutExpo, easeInExpo>(_t);
	}

	inline BX_CONST_FUNC float easeInCirc(float _t)
	{
		return -(sqrt(1.0f - _t*_t) - 1.0f);
	}

	inline BX_CONST_FUNC float easeOutCirc(float _t)
	{
		return easeOut<easeInCirc>(_t);
	}

	inline BX_CONST_FUNC float easeInOutCirc(float _t)
	{
		return easeMix<easeInCirc, easeOutCirc>(_t);
	}

	inline BX_CONST_FUNC float easeOutInCirc(float _t)
	{
		return easeMix<easeOutCirc, easeInCirc>(_t);
	}

	inline BX_CONST_FUNC float easeOutElastic(float _t)
	{
		return pow(2.0f, -10.0f*_t)*sin( (_t-0.3f/4.0f)*(2.0f*kPi)/0.3f) + 1.0f;
	}

	inline BX_CONST_FUNC float easeInElastic(float _t)
	{
		return easeOut<easeOutElastic>(_t);
	}

	inline BX_CONST_FUNC float easeInOutElastic(float _t)
	{
		return easeMix<easeInElastic, easeOutElastic>(_t);
	}

	inline BX_CONST_FUNC float easeOutInElastic(float _t)
	{
		return easeMix<easeOutElastic, easeInElastic>(_t);
	}

	inline BX_CONST_FUNC float easeInBack(float _t)
	{
		return easeInCubic(_t) - _t*sin(_t*kPi);
	}

	inline BX_CONST_FUNC float easeOutBack(float _t)
	{
		return easeOut<easeInBack>(_t);
	}

	inline BX_CONST_FUNC float easeInOutBack(float _t)
	{
		return easeMix<easeInBack, easeOutBack>(_t);
	}

	inline BX_CONST_FUNC float easeOutInBack(float _t)
	{
		return easeMix<easeOutBack, easeInBack>(_t);
	}

	inline BX_CONST_FUNC float easeOutBounce(float _t)
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

	inline BX_CONST_FUNC float easeInBounce(float _t)
	{
		return easeOut<easeOutBounce>(_t);
	}

	inline BX_CONST_FUNC float easeInOutBounce(float _t)
	{
		return easeMix<easeInBounce, easeOutBounce>(_t);
	}

	inline BX_CONST_FUNC float easeOutInBounce(float _t)
	{
		return easeMix<easeOutBounce, easeInBounce>(_t);
	}

} // namespace bx
