/*
 * Copyright 2010-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_TYPETRAITS_H_HEADER_GUARD
#	error "Must be included from bx/typetraits.h!"
#endif // BX_TYPETRAITS_H_HEADER_GUARD

namespace bx
{
	// Reference(s):
	//
	// - GCC type traits compiler intrisics
	//   https://gcc.gnu.org/onlinedocs/gcc-11.1.0/gcc/Type-Traits.html
	//
	// - Clang type trait primitives
	//   https://clang.llvm.org/docs/LanguageExtensions.html#type-trait-primitives

	//---
	template<typename Ty, Ty ValueT>
	struct IntegralConstantT
	{
		using ValueType = Ty;
		static constexpr Ty value = ValueT;
	};

	template<bool BoolT>
	using BoolConstantT = IntegralConstantT<bool, BoolT>;
	using FalseConstant = BoolConstantT<false>;
	using TrueConstant  = BoolConstantT<true >;

	//---
	template<typename Ty> struct TypeIdentityT { using Type = Ty; };
	template<typename Ty>  using TypeIdentityType = typename TypeIdentityT<Ty>::Type;

	//---
	template<bool BoolT, typename Ty, typename Uy> struct ConditionalT                { using Type = Ty; };
	template<            typename Ty, typename Uy> struct ConditionalT<false, Ty, Uy> { using Type = Uy; };
	template<bool BoolT, typename Ty, typename Uy>  using ConditionalType = typename ConditionalT<BoolT, Ty, Uy>::Type;

	//---
	template<bool BoolT, typename Ty = void> struct EnableIfT           {                  };
	template<            typename Ty       > struct EnableIfT<true, Ty> { using Type = Ty; };
	template<bool BoolT, typename Ty       >  using EnableIfType = typename EnableIfT<BoolT, Ty>::Type;

	//---
	template<typename Ty> struct AddLvalueReferenceT                      { using Type = Ty&;                 };
	template<typename Ty> struct AddLvalueReferenceT<Ty&>                 { using Type = Ty&;                 };
	template<>            struct AddLvalueReferenceT<               void> { using Type =                void; };
	template<>            struct AddLvalueReferenceT<const          void> { using Type = const          void; };
	template<>            struct AddLvalueReferenceT<volatile       void> { using Type = volatile       void; };
	template<>            struct AddLvalueReferenceT<const volatile void> { using Type = const volatile void; };
	template<typename Ty>  using AddLvalueReferenceType = typename AddLvalueReferenceT<Ty>::Type;

	//---
	template<typename Ty> struct AddRvalueReferenceT                      { using Type = Ty&&;                };
	template<typename Ty> struct AddRvalueReferenceT<Ty&>                 { using Type = Ty&;                 };
	template<>            struct AddRvalueReferenceT<               void> { using Type =                void; };
	template<>            struct AddRvalueReferenceT<const          void> { using Type = const          void; };
	template<>            struct AddRvalueReferenceT<volatile       void> { using Type = volatile       void; };
	template<>            struct AddRvalueReferenceT<const volatile void> { using Type = const volatile void; };
	template<typename Ty>  using AddRvalueReferenceType = typename AddRvalueReferenceT<Ty>::Type;

	//---
	template<typename Ty> struct RemoveReferenceT       { using Type = Ty; };
	template<typename Ty> struct RemoveReferenceT<Ty&>  { using Type = Ty; };
	template<typename Ty> struct RemoveReferenceT<Ty&&> { using Type = Ty; };
	template<typename Ty>  using RemoveReferenceType = typename RemoveReferenceT<Ty>::Type;

	//---
	template<typename Ty> struct AddPointerT { using Type = TypeIdentityType<RemoveReferenceType<Ty>*>; };
	template<typename Ty>  using AddPointerType = typename AddPointerT<Ty>::Type;

	//---
	template<typename Ty> struct RemovePointerT                                    { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<               Ty*               > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<         const Ty*               > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<      volatile Ty*               > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<const volatile Ty*               > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<const volatile Ty* const         > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<               Ty* const         > { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<               Ty*       volatile> { using Type = Ty; };
	template<typename Ty> struct RemovePointerT<               Ty* const volatile> { using Type = Ty; };
	template<typename Ty>  using RemovePointerType = typename RemovePointerT<Ty>::Type;

	//---
	template<typename Ty> struct AddCvT { using Type = const volatile Ty; };
	template<typename Ty>  using AddCvType = typename AddCvT<Ty>::Type;

	//---
	template<typename Ty> struct RemoveCvT                    { using Type = Ty; };
	template<typename Ty> struct RemoveCvT<const Ty>          { using Type = Ty; };
	template<typename Ty> struct RemoveCvT<volatile Ty>       { using Type = Ty; };
	template<typename Ty> struct RemoveCvT<const volatile Ty> { using Type = Ty; };
	template<typename Ty>  using RemoveCvType = typename RemoveCvT<Ty>::Type;

	//---
	template<typename Ty> struct AddConstT { using Type = const Ty; };
	template<typename Ty>  using AddConstType = typename AddConstT<Ty>::Type;

	//---
	template<typename Ty> struct RemoveConstT           { using Type = Ty; };
	template<typename Ty> struct RemoveConstT<const Ty> { using Type = Ty; };
	template<typename Ty>  using RemoveConstType = typename RemoveConstT<Ty>::Type;

	//---
	template<typename Ty> struct AddVolatileT { using Type = volatile Ty; };
	template<typename Ty>  using AddVolatileType = typename AddVolatileT<Ty>::Type;

	//---
	template<typename Ty> struct RemoveVolatileT              { using Type = Ty; };
	template<typename Ty> struct RemoveVolatileT<volatile Ty> { using Type = Ty; };
	template<typename Ty>  using RemoveVolatileType = typename RemoveVolatileT<Ty>::Type;

	//---

	template<typename Ty>               struct IsBoundedArrayT            : FalseConstant {};
	template<typename Ty, size_t SizeT> struct IsBoundedArrayT<Ty[SizeT]> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isBoundedArray()
	{
		return IsBoundedArrayT<Ty>::value;
	}

	template<typename Ty> struct IsUnboundedArrayT       : FalseConstant {};
	template<typename Ty> struct IsUnboundedArrayT<Ty[]> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isUnboundedArray()
	{
		return IsUnboundedArrayT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isArray()
	{
		return isBoundedArray<Ty>()
			|| isUnboundedArray<Ty>()
			;
	}

	template<typename Ty>
	inline constexpr bool isEnum()
	{
		return !!__is_enum(Ty);
	}

	template<typename Ty>
	inline constexpr bool isUnion()
	{
		return !!__is_union(Ty);
	}

	template<typename Ty>
	inline constexpr bool isAbstract()
	{
		return !!__is_abstract(Ty);
	}

	template<typename Ty>
	inline constexpr bool isAggregate()
	{
		return !!__is_aggregate(Ty);
	}

	template<typename BaseT, typename DerivedT>
	inline constexpr bool isBaseOf()
	{
		return !!__is_base_of(BaseT, DerivedT);
	}

	template<typename Ty>
	inline constexpr bool isPolymorphic()
	{
		return !!__is_polymorphic(Ty);
	}

	template<typename Ty>
	inline constexpr bool isDestructorVirtual()
	{
		return __has_virtual_destructor(Ty);
	}

	template<typename Ty>
	inline constexpr bool isClass()
	{
		return !!__is_class(Ty);
	}

	template<typename Ty>
	inline constexpr bool isFinal()
	{
		return !!__is_final(Ty);
	}

	template<typename Ty>
	inline constexpr bool isEmpty()
	{
		return !!__is_empty(Ty);
	}

	template<typename Ty>
	inline constexpr bool isStandardLayout()
	{
		return !!__is_standard_layout(Ty);
	}

	template<typename Ty>
	inline constexpr bool isTrivial()
	{
		return !!__is_trivial(Ty);
	}

	template<typename Ty>
	inline constexpr bool isPod()
	{
		return isStandardLayout<Ty>()
			&& isTrivial<Ty>()
			;
	}

	template<typename Ty, typename FromT>
	inline constexpr bool isAssignable()
	{
		return !!__is_assignable(Ty, FromT);
	}

	template<typename Ty>
	inline constexpr bool isCopyAssignable()
	{
		return isAssignable<
				  AddLvalueReferenceType<Ty>
				, AddLvalueReferenceType<const Ty>
				>();
	}

	template<typename Ty>
	inline constexpr bool isMoveAssignable()
	{
		return isAssignable<
				  AddLvalueReferenceType<Ty>
				, AddRvalueReferenceType<Ty>
				>();
	}

	template<typename Ty, typename FromT>
	inline constexpr bool isTriviallyAssignable()
	{
		return !!__is_trivially_assignable(Ty, FromT);
	}

	template<typename Ty>
	inline constexpr bool isTriviallyCopyAssignable()
	{
		return isTriviallyAssignable<
				  AddLvalueReferenceType<Ty>
				, AddRvalueReferenceType<const Ty>
				>();
	}

	template<typename Ty>
	inline constexpr bool isTriviallyMoveAssignable()
	{
		return isTriviallyAssignable<
				  AddLvalueReferenceType<Ty>
				, AddRvalueReferenceType<Ty>
				>();
	}

	template<typename Ty, typename... ArgsT>
	inline constexpr bool isConstructible()
	{
		return !!__is_constructible(Ty, ArgsT...);
	}

	template<typename Ty>
	inline constexpr bool isCopyConstructible()
	{
		return isConstructible<Ty, AddLvalueReferenceType<Ty>>();
	}

	template<typename Ty>
	inline constexpr bool isMoveConstructible()
	{
		return isConstructible<Ty, AddRvalueReferenceType<Ty>>();
	}

	template<typename Ty, typename... ArgsT>
	inline constexpr bool isTriviallyConstructible()
	{
		return !!__is_trivially_constructible(Ty, ArgsT...);
	}

	template<typename Ty>
	inline constexpr bool isTriviallyCopyConstructible()
	{
		return isTriviallyConstructible<Ty, AddLvalueReferenceType<Ty>>();
	}

	template<typename Ty>
	inline constexpr bool isTriviallyMoveConstructible()
	{
		return isTriviallyConstructible<Ty, AddRvalueReferenceType<Ty>>();
	}

	template<typename Ty>
	inline constexpr bool isTriviallyCopyable()
	{
		return !!__is_trivially_copyable(Ty);
	}

	template<typename Ty>
	inline constexpr bool isTriviallyDestructible()
	{
#if BX_COMPILER_GCC
		return !!__has_trivial_destructor(Ty);
#else
		return !!__is_trivially_destructible(Ty);
#endif // BX_COMPILER_GCC
	}

	template<typename Ty> struct IsConstT           : FalseConstant {};
	template<typename Ty> struct IsConstT<const Ty> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isConst()
	{
		return IsConstT<Ty>::value;
	}

	template<typename Ty> struct IsVolatileT              : FalseConstant {};
	template<typename Ty> struct IsVolatileT<volatile Ty> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isVolatile()
	{
		return IsVolatileT<Ty>::value;
	}

	template<typename Ty> struct IsLvalueReferenceT      : FalseConstant {};
	template<typename Ty> struct IsLvalueReferenceT<Ty&> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isLvalueReference()
	{
		return IsLvalueReferenceT<Ty>::value;
	}

	template<typename Ty> struct IsRvalueReferenceT       : FalseConstant {};
	template<typename Ty> struct IsRvalueReferenceT<Ty&&> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isRvalueReference()
	{
		return IsRvalueReferenceT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isReference()
	{
		return isLvalueReference<Ty>()
			|| isRvalueReference<Ty>()
			;
	}

	template<typename Ty> struct IsPointerT                     : FalseConstant {};
	template<typename Ty> struct IsPointerT<Ty*>                :  TrueConstant {};
	template<typename Ty> struct IsPointerT<Ty* const>          :  TrueConstant {};
	template<typename Ty> struct IsPointerT<Ty* volatile>       :  TrueConstant {};
	template<typename Ty> struct IsPointerT<Ty* const volatile> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isPointer()
	{
		return IsPointerT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isSigned()
	{
		return Ty(-1) < Ty(0);
	}

	template<typename Ty>
	inline constexpr bool isUnsigned()
	{
		return Ty(-1) > Ty(0);
	}

	template<typename Ty> struct IsIntegerT                     : FalseConstant {};
	template<>            struct IsIntegerT<bool              > :  TrueConstant {};
	template<>            struct IsIntegerT<char              > :  TrueConstant {};
	template<>            struct IsIntegerT<char16_t          > :  TrueConstant {};
	template<>            struct IsIntegerT<char32_t          > :  TrueConstant {};
	template<>            struct IsIntegerT<wchar_t           > :  TrueConstant {};
	template<>            struct IsIntegerT<  signed char     > :  TrueConstant {};
	template<>            struct IsIntegerT<unsigned char     > :  TrueConstant {};
	template<>            struct IsIntegerT<         short    > :  TrueConstant {};
	template<>            struct IsIntegerT<unsigned short    > :  TrueConstant {};
	template<>            struct IsIntegerT<         int      > :  TrueConstant {};
	template<>            struct IsIntegerT<unsigned int      > :  TrueConstant {};
	template<>            struct IsIntegerT<         long     > :  TrueConstant {};
	template<>            struct IsIntegerT<unsigned long     > :  TrueConstant {};
	template<>            struct IsIntegerT<         long long> :  TrueConstant {};
	template<>            struct IsIntegerT<unsigned long long> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isInteger()
	{
		return IsIntegerT<Ty>::value;
	}

	template<typename Ty> struct IsFloatingPointT              : FalseConstant {};
	template<>            struct IsFloatingPointT<float      > :  TrueConstant {};
	template<>            struct IsFloatingPointT<     double> :  TrueConstant {};
	template<>            struct IsFloatingPointT<long double> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isFloatingPoint()
	{
		return IsFloatingPointT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isArithmetic()
	{
		return isInteger<Ty>()
			|| isFloatingPoint<Ty>()
			;
	}

	template<typename Ty, typename Uy> struct IsSameT         : FalseConstant {};
	template<typename Ty>              struct IsSameT<Ty, Ty> :  TrueConstant {};

	template<typename Ty, typename Uy>
	inline constexpr bool isSame()
	{
		return IsSameT<Ty, Uy>::value;
	}

	template<typename Ty>
	inline constexpr RemoveReferenceType<Ty>&& move(Ty&& _a)
	{
		return static_cast<RemoveReferenceType<Ty>&&>(_a);
	}

	template<typename Ty>
	inline constexpr Ty&& forward(RemoveReferenceType<Ty>& _a)
	{
		return static_cast<Ty&&>(_a);
	}

	template<typename Ty>
	inline constexpr Ty&& forward(RemoveReferenceType<Ty>&& _a)
	{
		BX_STATIC_ASSERT(!isLvalueReference<Ty>(), "Can not forward an Rvalue as an Lvalue.");
		return static_cast<Ty&&>(_a);
	}

} // namespace bx
