/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
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

	//---
	template<typename Ty> struct IsUnboundedArrayT       : FalseConstant {};
	template<typename Ty> struct IsUnboundedArrayT<Ty[]> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isUnboundedArray()
	{
		return IsUnboundedArrayT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsArrayT : public BoolConstantT<false
		|| IsBoundedArrayT<Ty>::value
		|| IsUnboundedArrayT<Ty>::value
	> {};

	template<typename Ty>
	inline constexpr bool isArray()
	{
		return IsArrayT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsEnumT : public BoolConstantT<__is_enum(Ty)> {};

	template<typename Ty>
	inline constexpr bool isEnum()
	{
		return IsEnumT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsUnionT : public BoolConstantT<__is_union(Ty)> {};

	template<typename Ty>
	inline constexpr bool isUnion()
	{
		return IsUnionT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsAbstractT : public BoolConstantT<__is_abstract(Ty)> {};

	template<typename Ty>
	inline constexpr bool isAbstract()
	{
		return IsAbstractT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsAggregateT : public BoolConstantT<__is_aggregate(Ty)> {};

	template<typename Ty>
	inline constexpr bool isAggregate()
	{
		return IsAggregateT<Ty>::value;
	}

	//---
	template<typename BaseT, typename DerivedT> struct IsBaseOfT : public BoolConstantT<__is_base_of(BaseT, DerivedT)> {};

	template<typename BaseT, typename DerivedT>
	inline constexpr bool isBaseOf()
	{
		return IsBaseOfT<BaseT, DerivedT>::value;
	}

	//---
	template<typename Ty> struct IsPolymorphicT : public BoolConstantT<__is_polymorphic(Ty)> {};

	template<typename Ty>
	inline constexpr bool isPolymorphic()
	{
		return IsPolymorphicT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsDestructorVirtualT : public BoolConstantT<__has_virtual_destructor(Ty)> {};

	template<typename Ty>
	inline constexpr bool isDestructorVirtual()
	{
		return IsDestructorVirtualT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsClassT : public BoolConstantT<__is_class(Ty)> {};

	template<typename Ty>
	inline constexpr bool isClass()
	{
		return IsClassT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsFinalT : public BoolConstantT<__is_final(Ty)> {};

	template<typename Ty>
	inline constexpr bool isFinal()
	{
		return IsFinalT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsEmptyT : public BoolConstantT<__is_empty(Ty)> {};

	template<typename Ty>
	inline constexpr bool isEmpty()
	{
		return IsEmptyT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsStandardLayoutT : public BoolConstantT<__is_standard_layout(Ty)> {};

	template<typename Ty>
	inline constexpr bool isStandardLayout()
	{
		return IsStandardLayoutT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsTrivialT : public BoolConstantT<__is_trivial(Ty)> {};

	template<typename Ty>
	inline constexpr bool isTrivial()
	{
		return IsTrivialT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsPodT : public BoolConstantT<true
		&& IsStandardLayoutT<Ty>::value
		&& IsTrivialT<Ty>::value
	> {};

	template<typename Ty>
	inline constexpr bool isPod()
	{
		return IsPodT<Ty>::value;
	}

	//---
	template<typename Ty, typename FromT> struct IsAssignableT     : public BoolConstantT<__is_assignable(Ty, FromT)> {};
	template<typename Ty>                 struct IsCopyAssignableT : public IsAssignableT<AddLvalueReferenceType<Ty>, AddRvalueReferenceType<const Ty>> {};
	template<typename Ty>                 struct IsMoveAssignableT : public IsAssignableT<AddLvalueReferenceType<Ty>, AddRvalueReferenceType<Ty>> {};

	template<typename Ty, typename FromT>
	inline constexpr bool isAssignable()
	{
		return IsAssignableT<Ty, FromT>::value;
	}

	template<typename Ty>
	inline constexpr bool isCopyAssignable()
	{
		return IsCopyAssignableT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isMoveAssignable()
	{
		return IsMoveAssignableT<Ty>::value;
	}

	//---
	template<typename Ty, typename FromT> struct IsTriviallyAssignableT     : public BoolConstantT<__is_trivially_assignable(Ty, FromT)>                                  {};
	template<typename Ty>                 struct IsTriviallyCopyAssignableT : public IsTriviallyAssignableT<AddLvalueReferenceType<Ty>, AddRvalueReferenceType<const Ty>> {};
	template<typename Ty>                 struct IsTriviallyMoveAssignableT : public IsTriviallyAssignableT<AddLvalueReferenceType<Ty>, AddRvalueReferenceType<Ty>>       {};

	template<typename Ty, typename FromT>
	inline constexpr bool isTriviallyAssignable()
	{
		return IsTriviallyAssignableT<Ty, FromT>::value;
	}

	template<typename Ty>
	inline constexpr bool isTriviallyCopyAssignable()
	{
		return IsTriviallyCopyAssignableT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isTriviallyMoveAssignable()
	{
		return IsTriviallyMoveAssignableT<Ty>::value;
	}

	//---
	template<typename Ty, typename... ArgsT> struct IsConstructibleT     : public BoolConstantT<__is_constructible(Ty, ArgsT...)>  {};
	template<typename Ty>                    struct IsCopyConstructibleT : public IsConstructibleT<Ty, AddLvalueReferenceType<Ty>> {};
	template<typename Ty>                    struct IsMoveConstructibleT : public IsConstructibleT<Ty, AddRvalueReferenceType<Ty>> {};

	template<typename Ty, typename... ArgsT>
	inline constexpr bool isConstructible()
	{
		return IsConstructibleT<Ty, ArgsT...>::value;
	}

	template<typename Ty>
	inline constexpr bool isCopyConstructible()
	{
		return IsCopyConstructibleT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isMoveConstructible()
	{
		return IsMoveConstructibleT<Ty>::value;
	}

	//---
	template<typename Ty, typename... ArgsT> struct IsTriviallyConstructibleT     : public BoolConstantT<__is_trivially_constructible(Ty, ArgsT...)> {};
	template<typename Ty>                    struct IsTriviallyCopyConstructibleT : public IsTriviallyConstructibleT<Ty, AddLvalueReferenceType<Ty>>  {};
	template<typename Ty>                    struct IsTriviallyMoveConstructibleT : public IsTriviallyConstructibleT<Ty, AddRvalueReferenceType<Ty>>  {};

	template<typename Ty, typename... ArgsT>
	constexpr bool isTriviallyConstructible()
	{
		return IsTriviallyConstructibleT<Ty, ArgsT...>::value;
	}

	template<typename Ty>
	inline constexpr bool isTriviallyCopyConstructible()
	{
		return IsTriviallyCopyConstructibleT<Ty>::value;
	}

	template<typename Ty>
	inline constexpr bool isTriviallyMoveConstructible()
	{
		return IsTriviallyMoveConstructibleT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsTriviallyCopyableT : public BoolConstantT<__is_trivially_copyable(Ty)> {};

	template<typename Ty>
	inline constexpr bool isTriviallyCopyable()
	{
		return IsTriviallyCopyableT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsTriviallyDestructibleT : public BoolConstantT<
#if BX_COMPILER_GCC
		__has_trivial_destructor(Ty)
#else
		__is_trivially_destructible(Ty)
#endif // BX_COMPILER_GCC
	> {};

	template<typename Ty>
	inline constexpr bool isTriviallyDestructible()
	{
		return IsTriviallyDestructibleT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsConstT           : FalseConstant {};
	template<typename Ty> struct IsConstT<const Ty> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isConst()
	{
		return IsConstT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsVolatileT              : FalseConstant {};
	template<typename Ty> struct IsVolatileT<volatile Ty> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isVolatile()
	{
		return IsVolatileT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsLvalueReferenceT      : FalseConstant {};
	template<typename Ty> struct IsLvalueReferenceT<Ty&> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isLvalueReference()
	{
		return IsLvalueReferenceT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsRvalueReferenceT       : FalseConstant {};
	template<typename Ty> struct IsRvalueReferenceT<Ty&&> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isRvalueReference()
	{
		return IsRvalueReferenceT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsReferenceT : public BoolConstantT<false
		|| IsLvalueReferenceT<Ty>::value
		|| IsRvalueReferenceT<Ty>::value
	> {};

	template<typename Ty>
	inline constexpr bool isReference()
	{
		return IsReferenceT<Ty>::value;
	}

	//---
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

	//---
	template<typename Ty> struct IsSignedT : public BoolConstantT<Ty(-1) < Ty(0)> {};

	template<typename Ty>
	inline constexpr bool isSigned()
	{
		return IsSignedT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsUnsignedT : public BoolConstantT<Ty(0) < Ty(-1)> {};

	template<typename Ty>
	inline constexpr bool isUnsigned()
	{
		return IsUnsignedT<Ty>::value;
	}

	//---
	template<typename Ty> struct MakeSignedT { using Type = Ty; };
	template<typename Ty>  using MakeSignedType = typename MakeSignedT<Ty>::Type;

	template<typename Ty> struct MakeSignedT<const          Ty> : AddConstT   <MakeSignedType<Ty>> {};
	template<typename Ty> struct MakeSignedT<volatile       Ty> : AddVolatileT<MakeSignedType<Ty>> {};
	template<typename Ty> struct MakeSignedT<const volatile Ty> : AddCvT      <MakeSignedType<Ty>> {};

	template<>            struct MakeSignedT<         char     > { using Type = signed char;      };
	template<>            struct MakeSignedT<  signed char     > { using Type = signed char;      };
	template<>            struct MakeSignedT<unsigned char     > { using Type = signed char;      };
	template<>            struct MakeSignedT<         short    > { using Type = signed short;     };
	template<>            struct MakeSignedT<unsigned short    > { using Type = signed short;     };
	template<>            struct MakeSignedT<         int      > { using Type = signed int;       };
	template<>            struct MakeSignedT<unsigned int      > { using Type = signed int;       };
	template<>            struct MakeSignedT<         long     > { using Type = signed long;      };
	template<>            struct MakeSignedT<unsigned long     > { using Type = signed long;      };
	template<>            struct MakeSignedT<         long long> { using Type = signed long long; };
	template<>            struct MakeSignedT<unsigned long long> { using Type = signed long long; };

	template<typename Ty>
	inline constexpr auto asSigned(Ty _value)
	{
		return MakeSignedType<Ty>(_value);
	}

	//---
	template<typename Ty> struct MakeUnsignedT { using Type = Ty; };
	template<typename Ty>  using MakeUnsignedType = typename MakeUnsignedT<Ty>::Type;

	template<typename Ty> struct MakeUnsignedT<const          Ty> : AddConstT   <MakeUnsignedType<Ty>> {};
	template<typename Ty> struct MakeUnsignedT<volatile       Ty> : AddVolatileT<MakeUnsignedType<Ty>> {};
	template<typename Ty> struct MakeUnsignedT<const volatile Ty> : AddCvT      <MakeUnsignedType<Ty>> {};

	template<>            struct MakeUnsignedT<         char     > { using Type = unsigned char;      };
	template<>            struct MakeUnsignedT<  signed char     > { using Type = unsigned char;      };
	template<>            struct MakeUnsignedT<unsigned char     > { using Type = unsigned char;      };
	template<>            struct MakeUnsignedT<         short    > { using Type = unsigned short;     };
	template<>            struct MakeUnsignedT<unsigned short    > { using Type = unsigned short;     };
	template<>            struct MakeUnsignedT<         int      > { using Type = unsigned int;       };
	template<>            struct MakeUnsignedT<unsigned int      > { using Type = unsigned int;       };
	template<>            struct MakeUnsignedT<         long     > { using Type = unsigned long;      };
	template<>            struct MakeUnsignedT<unsigned long     > { using Type = unsigned long;      };
	template<>            struct MakeUnsignedT<         long long> { using Type = unsigned long long; };
	template<>            struct MakeUnsignedT<unsigned long long> { using Type = unsigned long long; };

	template<typename Ty>
	inline constexpr auto asUnsigned(Ty _value)
	{
		return MakeUnsignedType<Ty>(_value);
	}

	//---
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

	//---
	template<typename Ty> struct IsFloatingPointT              : FalseConstant {};
	template<>            struct IsFloatingPointT<float      > :  TrueConstant {};
	template<>            struct IsFloatingPointT<     double> :  TrueConstant {};
	template<>            struct IsFloatingPointT<long double> :  TrueConstant {};

	template<typename Ty>
	inline constexpr bool isFloatingPoint()
	{
		return IsFloatingPointT<Ty>::value;
	}

	//---
	template<typename Ty> struct IsArithmeticT : BoolConstantT<false
		|| IsIntegerT<Ty>::value
		|| IsFloatingPointT<Ty>::value
	>
	{};

	template<typename Ty>
	inline constexpr bool isArithmetic()
	{
		return IsArithmeticT<Ty>::value;
	}

	//---
	template<typename Ty, typename Uy> struct IsSameT         : FalseConstant {};
	template<typename Ty>              struct IsSameT<Ty, Ty> :  TrueConstant {};

	template<typename Ty, typename Uy>
	inline constexpr bool isSame()
	{
		return IsSameT<Ty, Uy>::value;
	}

	//---
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
