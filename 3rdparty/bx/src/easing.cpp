/*
 * Copyright 2010-2021 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#include "bx_p.h"
#include <bx/easing.h>

namespace bx
{
	static const EaseFn s_easeFunc[] =
	{
		easeLinear,
		easeStep,
		easeSmoothStep,
		easeInQuad,
		easeOutQuad,
		easeInOutQuad,
		easeOutInQuad,
		easeInCubic,
		easeOutCubic,
		easeInOutCubic,
		easeOutInCubic,
		easeInQuart,
		easeOutQuart,
		easeInOutQuart,
		easeOutInQuart,
		easeInQuint,
		easeOutQuint,
		easeInOutQuint,
		easeOutInQuint,
		easeInSine,
		easeOutSine,
		easeInOutSine,
		easeOutInSine,
		easeInExpo,
		easeOutExpo,
		easeInOutExpo,
		easeOutInExpo,
		easeInCirc,
		easeOutCirc,
		easeInOutCirc,
		easeOutInCirc,
		easeInElastic,
		easeOutElastic,
		easeInOutElastic,
		easeOutInElastic,
		easeInBack,
		easeOutBack,
		easeInOutBack,
		easeOutInBack,
		easeInBounce,
		easeOutBounce,
		easeInOutBounce,
		easeOutInBounce,
	};
	BX_STATIC_ASSERT(BX_COUNTOF(s_easeFunc) == Easing::Count);

	EaseFn getEaseFunc(Easing::Enum _enum)
	{
		return s_easeFunc[_enum];
	}

} // namespace bx
