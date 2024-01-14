/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#ifndef BX_TYPETRAITS_H_HEADER_GUARD
#define BX_TYPETRAITS_H_HEADER_GUARD

namespace bx
{
	/// Returns true if type `Ty` is array type with known bound, otherwise returns false.
	template<typename Ty>
	constexpr bool isBoundedArray();

	/// Returns true if type `Ty` is array type with unknown bound, otherwise returns false.
	template<typename Ty>
	constexpr bool isUnboundedArray();

	/// Returns true if type `Ty` is array type (bounded, or unbounded array), otherwise returns false.
	template<typename Ty>
	constexpr bool isArray();

	/// Returns true if type `Ty` is enumeration type, otherwise returns false.
	template<typename Ty>
	constexpr bool isEnum();

	/// Returns true if type `Ty` is union type, otherwise returns false.
	template<typename Ty>
	constexpr bool isUnion();

	/// Returns true if type `Ty` is non-union class type, otherwise returns false.
	template<typename Ty>
	constexpr bool isClass();

	/// Returns true if type `Ty` is abstract class type, otherwise returns false.
	template<typename Ty>
	constexpr bool isAbstract();

	/// Returns true if type `Ty` is aggregate class type, otherwise returns false.
	template<typename Ty>
	constexpr bool isAggregate();

	/// Returns true if type `DerivedT` is derived from `BaseT` type, or if both are the same
	/// non-union type, otherwise returns false.
	template<typename BaseT, typename DerivedT>
	constexpr bool isBaseOf();

	/// Returns true if type `Ty` is polymorphic class (if it has virtual function table) type,
	/// otherwise returns false.
	template<typename Ty>
	constexpr bool isPolymorphic();

	/// Returns true if type `Ty` has virtual destructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isDestructorVirtual();

	/// Returns true if type `Ty` is class type with final specifier, otherwise returns false.
	template<typename Ty>
	constexpr bool isFinal();

	/// Returns true if type `Ty` is non-union class type with no non-static
	/// members (a.k.a. sizeof(Ty) is zero), otherwise returns false.
	template<typename Ty>
	constexpr bool isEmpty();

	/// Returns true if type `Ty` is standard-layout type, otherwise returns false.
	template<typename Ty>
	constexpr bool isStandardLayout();

	/// Returns true if type `Ty` is trivial type, otherwise returns false.
	template<typename Ty>
	constexpr bool isTrivial();

	/// Returns true if type `Ty` is POD (standard-layout, and trivial type), otherwise returns false.
	template<typename Ty>
	constexpr bool isPod();

	/// Returns true if type `FromT` can be assigned to type `Ty`, otherwise returns false.
	template<typename Ty, typename FromT>
	constexpr bool isAssignable();

	/// Returns true if type `Ty` has copy assignment operator, otherwise returns false.
	template<typename Ty, typename Uy>
	constexpr bool isCopyAssignable();

	/// Returns true if type `Ty` has move assignment operator, otherwise returns false.
	template<typename Ty, typename Uy>
	constexpr bool isMoveAssignable();

	/// Returns true if type `FromT` can be trivially assigned to type `Ty`, otherwise returns false.
	template<typename Ty, typename FromT>
	constexpr bool isTriviallyAssignable();

	/// Returns true if type `Ty` is trivially copy-assignable type, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyCopyAssignable();

	/// Returns true if type `Ty` is trivially move-assignable type, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyMoveAssignable();

	/// Returns true if type `Ty` is constructible when the specified argument types are used,
	/// otherwise returns false.
	template<typename Ty, typename... ArgsT>
	constexpr bool isConstructible();

	/// Returns true if type `Ty` has copy constructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isCopyConstructible();

	/// Returns true if type `Ty` has move constructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isMoveConstructible();

	/// Returns true if type `Ty` is trivially constructible when the specified argument types are used,
	/// otherwise returns false.
	template<typename T, typename... ArgsT>
	constexpr bool isTriviallyConstructible();

	/// Returns true if type `Ty` has trivial copy constructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyCopyConstructible();

	/// Returns true if type `Ty` has trivial move constructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyMoveConstructible();

	/// Returns true if type `Ty` is trivially copyable / POD type, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyCopyable();

	/// Returns true if type `Ty` has trivial destructor, otherwise returns false.
	template<typename Ty>
	constexpr bool isTriviallyDestructible();

	/// Returns true if type `Ty` is const-qualified type, otherwise returns false.
	template<typename Ty>
	constexpr bool isConst();

	/// Returns true if type `Ty` is volatile-qualified type, otherwise returns false.
	template<typename Ty>
	constexpr bool isVolatile();

	/// Returns true if type `Ty` is an reference type, otherwise returns false.
	template<typename Ty>
	constexpr bool isReference();

	/// Returns true if type `Ty` is an Lvalue reference type, otherwise returns false.
	template<typename Ty>
	constexpr bool isLvalueReference();

	/// Returns true if type `Ty` is an Rvalue reference type, otherwise returns false.
	template<typename Ty>
	constexpr bool isRvalueReference();

	/// Returns true if type `Ty` is an pointer type, otherwise returns false.
	template<typename Ty>
	constexpr bool isPointer();

	/// Returns true if type `Ty` is signed integer or floating-point type, otherwise returns false.
	template<typename Ty>
	constexpr bool isSigned();

	/// Returns true if type `Ty` is unsigned integer type, otherwise returns false.
	template<typename Ty>
	constexpr bool isUnsigned();

	/// Returns true if type `Ty` is integer type, otherwise returns false.
	template<typename Ty>
	constexpr bool isInteger();

	/// Returns true if type `Ty` is floating-point type, otherwise returns false.
	template<typename Ty>
	constexpr bool isFloatingPoint();

	/// Returns true if type `Ty` is arithmetic type, otherwise returns false.
	template<typename Ty>
	constexpr bool isArithmetic();

	/// Returns true if type `Ty` and type `Uy` are the same type, otherwise returns false.
	template<typename Ty, typename Uy>
	constexpr bool isSame();

} // namespace bx

#include "inline/typetraits.inl"

namespace bx
{
	/// Compile-time constant of specified type `Ty` with specified value `ValueT`.
	template<typename Ty, Ty ValueT>
	struct IntegralConstantT;

	/// Alias of IntegralConstantT<bool, bool ValueT>.
	template<bool BoolT>
	using BoolConstantT = IntegralConstantT<bool, BoolT>;

	/// Alias of BoolConstantT<bool, false>.
	using FalseConstant = BoolConstantT<false>;

	/// Alias of BoolConstantT<bool, true>.
	using TrueConstant = BoolConstantT<true>;

	/// Provides the member `Type` that names `Ty`.
	template<typename Ty>
	using TypeIdentityType = typename TypeIdentityT<Ty>::Type;

	/// Conditionally selects `Ty` if `BoolT` is true, otherwise selects `Uy` type.
	template<bool BoolT, typename Ty, typename Uy>
	using ConditionalType = typename ConditionalT<BoolT, Ty, Uy>::Type;

	/// If `BoolT` condition is true it provides `Ty` as `Type`, otherwise `Type` is undefined.
	template<bool BoolT, typename Ty>
	using EnableIfType = typename EnableIfT<BoolT, Ty>::Type;

	/// If `Ty` is an object type or a function type that has no cv- or ref- qualifier, provides
	/// a member `Type` which is `Ty&`. If `Ty` is an Rvalue reference to some type `Uy`, then
	/// type is `Uy&`, otherwise type is `Ty`.
	template<typename Ty>
	using AddLvalueReferenceType = typename AddLvalueReferenceT<Ty>::Type;

	/// If `Ty` is an object type or a function type that has no cv- or ref- qualifier, provides
	/// a member `Type` which is `Ty&&`, otherwise type is `Ty`.
	template<typename Ty>
	using AddRvalueReferenceType = typename AddRvalueReferenceT<Ty>::Type;

	/// If the type `Ty` is a reference type, provides the member typedef `Type`
	/// which is the type referred to by `Ty`, otherwise type is `Ty`.
	template<typename Ty>
	using RemoveReferenceType = typename RemoveReferenceT<Ty>::Type;

	/// If the type `Ty` is a reference type, provides the member typedef `Type`
	/// which is a pointer to the referred type.
	template<typename Ty>
	using AddPointerType = typename AddPointerT<Ty>::Type;

	/// If the type `Ty` is a pointer type, provides the member typedef `Type`
	/// which is the type pointed to by `Ty`, otherwise type is `Ty`.
	template<typename Ty>
	using RemovePointerType = typename RemovePointerT<Ty>::Type;

	/// Adds both `const`, and `volatile` to type `Ty`.
	template<typename Ty>
	using AddCvType = typename AddCvT<Ty>::Type;

	/// Removes the topmost `const`, or the topmost `volatile`, or both, from type `Ty` if present.
	template<typename Ty>
	using RemoveCvType = typename RemoveCvT<Ty>::Type;

	/// Adds `const` to type `Ty`.
	template<typename Ty>
	using AddConstType = typename AddConstT<Ty>::Type;

	/// Removes the topmost `const` from type `Ty` if present.
	template<typename Ty>
	using RemoveConstType = typename RemoveConstT<Ty>::Type;

	/// Adds `volatile` to type `Ty`.
	template<typename Ty>
	using AddVolatileType = typename AddVolatileT<Ty>::Type;

	/// Removes the topmost `volatile` from type `Ty` if present.
	template<typename Ty>
	using RemoveVolatileType = typename RemoveVolatileT<Ty>::Type;

	/// Removes reference from type `Ty` if present.
	template<typename Ty>
	constexpr RemoveReferenceType<Ty>&& move(Ty&& _a);

	/// Forwards Lvalues as either Lvalues or as Rvalues.
	template<typename Ty>
	constexpr Ty&& forward(RemoveReferenceType<Ty>& _type);

	/// Forwards Rvalues as Rvalues and prohibits forwarding of Rvalues as Lvalues.
	template<typename Ty>
	constexpr Ty&& forward(RemoveReferenceT<Ty>&& _type);

	/// Converts any type `Ty` to a reference type, making it possible to use member functions
	/// in decltype expressions without the need to go through constructors.
	template<typename Ty>
	AddRvalueReferenceType<Ty> declVal();

} // namespace bx

#endif // BX_TYPETRAITS_H_HEADER_GUARD
