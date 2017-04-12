/*
 * Copyright 2010-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_H_HEADER_GUARD
#	error "Must be included from bx/bx.h!"
#endif // BX_H_HEADER_GUARD

namespace bx
{
	template<bool>
	inline bool isEnabled()
	{
		return true;
	}

	template<>
	inline bool isEnabled<false>()
	{
		return false;
	}

	inline bool ignoreC4127(bool _x)
	{
		return _x;
	}

	template<typename Ty>
	inline void xchg(Ty& _a, Ty& _b)
	{
		Ty tmp = _a; _a = _b; _b = tmp;
	}

} // namespace bx
