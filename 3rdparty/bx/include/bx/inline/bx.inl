/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_H_HEADER_GUARD
#	error "Must be included from bx/bx.h!"
#endif // BX_H_HEADER_GUARD

namespace bx
{
	// Reference(S):
	// - https://web.archive.org/web/20181115035420/http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
	//
	template<typename Ty, size_t NumT>
	char (&CountOfRequireArrayArgumentT(const Ty (&)[NumT]) )[NumT];

	template<bool B>
	struct isEnabled
	{
		// Template for avoiding MSVC: C4127: conditional expression is constant
		static constexpr bool value = B;
	};

	inline constexpr bool ignoreC4127(bool _x)
	{
		return _x;
	}

	template<typename Ty>
	inline Ty* addressOf(Ty& _a)
	{
		return reinterpret_cast<Ty*>(
				&const_cast<char&>(
					reinterpret_cast<const volatile char&>(_a)
				)
			);
	}

	template<typename Ty>
	inline const Ty* addressOf(const Ty& _a)
	{
		return reinterpret_cast<const Ty*>(
				&const_cast<char&>(
					reinterpret_cast<const volatile char&>(_a)
				)
			);
	}

	template<typename Ty>
	inline Ty* addressOf(void* _ptr, ptrdiff_t _offsetInBytes)
	{
		return (Ty*)( (uint8_t*)_ptr + _offsetInBytes);
	}

	template<typename Ty>
	inline const Ty* addressOf(const void* _ptr, ptrdiff_t _offsetInBytes)
	{
		return (const Ty*)( (const uint8_t*)_ptr + _offsetInBytes);
	}

	template<typename Ty>
	inline void swap(Ty& _a, Ty& _b)
	{
		Ty tmp = move(_a); _a = move(_b); _b = move(tmp);
	}

	template<typename Ty>
	struct LimitsT<Ty, true>
	{
		static constexpr Ty max = ( ( (Ty(1) << ( (sizeof(Ty) * 8) - 2) ) - Ty(1) ) << 1) | Ty(1);
		static constexpr Ty min = -max - Ty(1);
	};

	template<typename Ty>
	struct LimitsT<Ty, false>
	{
		static constexpr Ty min = 0;
		static constexpr Ty max = Ty(-1);
	};

	template<>
	struct LimitsT<float, true>
	{
		static constexpr float min = -kFloatLargest;
		static constexpr float max =  kFloatLargest;
	};

	template<>
	struct LimitsT<double, true>
	{
		static constexpr double min = -kDoubleLargest;
		static constexpr double max =  kDoubleLargest;
	};

	template<typename Ty>
	inline constexpr Ty max()
	{
		return bx::LimitsT<Ty>::max;
	}

	template<typename Ty>
	inline constexpr Ty min()
	{
		return bx::LimitsT<Ty>::min;
	}

	template<typename Ty>
	inline constexpr Ty min(const Ty& _a, const TypeIdentityType<Ty>& _b)
	{
		return _a < _b ? _a : _b;
	}

	template<typename Ty>
	inline constexpr Ty max(const Ty& _a, const TypeIdentityType<Ty>& _b)
	{
		return _a > _b ? _a : _b;
	}

	template<typename Ty, typename... Args>
	inline constexpr Ty min(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args)
	{
		return min(min(_a, _b), _args...);
	}

	template<typename Ty, typename... Args>
	inline constexpr Ty max(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args)
	{
		return max(max(_a, _b), _args...);
	}

	template<typename Ty, typename... Args>
	inline constexpr Ty mid(const Ty& _a, const TypeIdentityType<Ty>& _b, const Args&... _args)
	{
		return max(min(_a, _b), min(max(_a, _b), _args...) );
	}

	template<typename Ty>
	inline constexpr Ty clamp(const Ty& _a, const TypeIdentityType<Ty>& _min, const TypeIdentityType<Ty>& _max)
	{
		return max(min(_a, _max), _min);
	}

	template<typename Ty>
	inline constexpr bool isPowerOf2(Ty _a)
	{
		return _a && !(_a & (_a - 1) );
	}

	template <typename Ty, typename FromT>
	inline constexpr Ty bitCast(const FromT& _from)
	{
		static_assert(sizeof(Ty) == sizeof(FromT)
			, "bx::bitCast failed! Ty and FromT must be the same size."
			);
		static_assert(isTriviallyConstructible<Ty>()
			, "bx::bitCast failed! Destination target must be trivially constructible."
			);

		Ty to;
		memCopy(&to, &_from, sizeof(Ty) );

		return to;
	}

	template<typename Ty, typename FromT>
	inline constexpr Ty narrowCast(const FromT& _from, Location _location)
	{
		Ty to = static_cast<Ty>(_from);
		BX_ASSERT_LOC(_location, static_cast<FromT>(to) == _from
			, "bx::narrowCast failed! Value is truncated!"
			);
		return to;
	}

} // namespace bx
