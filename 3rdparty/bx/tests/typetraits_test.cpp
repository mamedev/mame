/*
 * Copyright 2010-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx/blob/master/LICENSE
 */

#include "test.h"
#include <bx/typetraits.h>

BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-private-field");
struct TestClass                {                                                                        };
struct TestClassFinal final     {                                                                        };
struct TestClassMember          { int32_t x;                                                             };
struct TestClassMemberPrivate   { int32_t x; private: int32_t y;                                         };
struct TestClassStaticOnly      { static int32_t x;                                                      };
struct TestClassCtor            {                 TestClassCtor (const TestClassCtor&) {}                };
struct TestClassDefaultCtor     {          TestClassDefaultCtor (const TestClassDefaultCtor&) = default; };
struct TestClassDefaultDtor     {         ~TestClassDefaultDtor () = default;                            };
struct TestClassVirtualDtor     { virtual ~TestClassVirtualDtor () = default;                            };
struct TestClassAbstractBase    { virtual ~TestClassAbstractBase() = 0;                                  };
struct TestClassPolymorphic     { virtual int32_t x() { return 1389; }                                   };
struct TestClassDerivedA        /*                                                                       */
		: TestClass             {                                                                        };
struct TestClassDerivedB        /*                                                                       */
		: TestClassDerivedA     {                                                                        };
struct TestClassDerivedX        /*                                                                       */
		: TestClassVirtualDtor  {                                                                        };
union  TestUnionEmpty           {                                                                        };
union  TestUnion                { int32_t x; float y;                                                    };
enum   TestEnumEmpty            {                                                                        };
enum   TestEnum                 { Enum                                                                   };
BX_PRAGMA_DIAGNOSTIC_POP();

TEST_CASE("type-traits isReference", "")
{
	STATIC_REQUIRE(!bx::isReference<TestClass   >() );
	STATIC_REQUIRE( bx::isReference<TestClass&  >() );
	STATIC_REQUIRE( bx::isReference<TestClass&& >() );
	STATIC_REQUIRE(!bx::isReference<long        >() );
	STATIC_REQUIRE( bx::isReference<long&       >() );
	STATIC_REQUIRE( bx::isReference<long&&      >() );
	STATIC_REQUIRE(!bx::isReference<double*     >() );
	STATIC_REQUIRE( bx::isReference<double*&    >() );
	STATIC_REQUIRE( bx::isReference<double*&&   >() );;
}

TEST_CASE("type-traits isLvalueReference", "")
{
	STATIC_REQUIRE(!bx::isLvalueReference<TestClass   >() );
	STATIC_REQUIRE( bx::isLvalueReference<TestClass&  >() );
	STATIC_REQUIRE(!bx::isLvalueReference<TestClass&& >() );
	STATIC_REQUIRE(!bx::isLvalueReference<long        >() );
	STATIC_REQUIRE( bx::isLvalueReference<long&       >() );
	STATIC_REQUIRE(!bx::isLvalueReference<long&&      >() );
	STATIC_REQUIRE(!bx::isLvalueReference<double*     >() );
	STATIC_REQUIRE( bx::isLvalueReference<double*&    >() );
	STATIC_REQUIRE(!bx::isLvalueReference<double*&&   >() );;
}

TEST_CASE("type-traits isRvalueReference", "")
{
	STATIC_REQUIRE(!bx::isRvalueReference<TestClass   >() );
	STATIC_REQUIRE(!bx::isRvalueReference<TestClass&  >() );
	STATIC_REQUIRE( bx::isRvalueReference<TestClass&& >() );
	STATIC_REQUIRE(!bx::isRvalueReference<long        >() );
	STATIC_REQUIRE(!bx::isRvalueReference<long&       >() );
	STATIC_REQUIRE( bx::isRvalueReference<long&&      >() );
	STATIC_REQUIRE(!bx::isRvalueReference<double*     >() );
	STATIC_REQUIRE(!bx::isRvalueReference<double*&    >() );
	STATIC_REQUIRE( bx::isRvalueReference<double*&&   >() );;
}

TEST_CASE("type-traits isPointer", "")
{
	STATIC_REQUIRE(!bx::isPointer<TestClass                  >() );
	STATIC_REQUIRE(!bx::isPointer<TestClass&                 >() );
	STATIC_REQUIRE( bx::isPointer<TestClass*                 >() );
	STATIC_REQUIRE( bx::isPointer<TestClass const * volatile >() );
	STATIC_REQUIRE(!bx::isPointer<int32_t                    >() );
	STATIC_REQUIRE( bx::isPointer<int32_t*                   >() );
	STATIC_REQUIRE(!bx::isPointer<int32_t[1389]              >() );
}

TEST_CASE("type-traits AddLvalueReferenceT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddLvalueReferenceType<TestClass  >, TestClass& >() );
	STATIC_REQUIRE( bx::isSame<bx::AddLvalueReferenceType<TestClass& >, TestClass& >() );
	STATIC_REQUIRE( bx::isSame<bx::AddLvalueReferenceType<TestClass&&>, TestClass& >() );

	STATIC_REQUIRE( bx::isLvalueReference<bx::AddLvalueReferenceType<TestClass>   >() );
	STATIC_REQUIRE( bx::isLvalueReference<bx::AddLvalueReferenceType<TestClass&>  >() );
	STATIC_REQUIRE( bx::isLvalueReference<bx::AddLvalueReferenceType<TestClass&&> >() );
}

TEST_CASE("type-traits AddRvalueReferenceT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddRvalueReferenceType<TestClass  >, TestClass&& >() );
	STATIC_REQUIRE( bx::isSame<bx::AddRvalueReferenceType<TestClass& >, TestClass&  >() );
	STATIC_REQUIRE( bx::isSame<bx::AddRvalueReferenceType<TestClass&&>, TestClass&& >() );

	STATIC_REQUIRE( bx::isRvalueReference<bx::AddRvalueReferenceType<TestClass>   >() );
	STATIC_REQUIRE(!bx::isRvalueReference<bx::AddRvalueReferenceType<TestClass&>  >() );
	STATIC_REQUIRE( bx::isRvalueReference<bx::AddRvalueReferenceType<TestClass&&> >() );
}

TEST_CASE("type-traits RemoveReferenceT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::RemoveReferenceType<TestClass  >, TestClass >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveReferenceType<TestClass& >, TestClass >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveReferenceType<TestClass&&>, TestClass >() );

	STATIC_REQUIRE(!bx::isReference<bx::RemoveReferenceType<TestClass>   >() );
	STATIC_REQUIRE(!bx::isReference<bx::RemoveReferenceType<TestClass&>  >() );
	STATIC_REQUIRE(!bx::isReference<bx::RemoveReferenceType<TestClass&&> >() );
}

TEST_CASE("type-traits AddPointerT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddPointerType<TestClass  >, TestClass*   >() );
	STATIC_REQUIRE( bx::isSame<bx::AddPointerType<TestClass* >, TestClass**  >() );
	STATIC_REQUIRE( bx::isSame<bx::AddPointerType<TestClass& >, TestClass*   >() );
	STATIC_REQUIRE( bx::isSame<bx::AddPointerType<TestClass**>, TestClass*** >() );

	STATIC_REQUIRE( bx::isPointer<bx::AddPointerType<TestClass>   >() );
	STATIC_REQUIRE( bx::isPointer<bx::AddPointerType<TestClass*>  >() );
	STATIC_REQUIRE( bx::isPointer<bx::AddPointerType<TestClass&>  >() );
	STATIC_REQUIRE( bx::isPointer<bx::AddPointerType<TestClass**> >() );
}

TEST_CASE("type-traits RemovePointerT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::RemovePointerType<TestClass  >, TestClass                        >() );
	STATIC_REQUIRE( bx::isSame<bx::RemovePointerType<TestClass* >, TestClass                        >() );
	STATIC_REQUIRE( bx::isSame<bx::RemovePointerType<TestClass**>, TestClass*                       >() );
	STATIC_REQUIRE( bx::isSame<bx::RemovePointerType<bx::RemovePointerType<TestClass**>>, TestClass >() );

	STATIC_REQUIRE(!bx::isPointer<bx::RemovePointerType<TestClass>                          >() );
	STATIC_REQUIRE(!bx::isPointer<bx::RemovePointerType<TestClass*>                         >() );
	STATIC_REQUIRE( bx::isPointer<bx::RemovePointerType<TestClass**>                        >() );
	STATIC_REQUIRE(!bx::isPointer<bx::RemovePointerType<bx::RemovePointerType<TestClass**>> >() );
}

TEST_CASE("type-traits AddConstT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<               TestClass >, const          TestClass        >() );
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<const          TestClass >, const          TestClass        >() );
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<      volatile TestClass >, const volatile TestClass        >() );
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<const volatile TestClass >, const volatile TestClass        >() );
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<const volatile TestClass*>, const volatile TestClass* const >() );
	STATIC_REQUIRE( bx::isSame<bx::AddConstType<TestClass* const volatile>, TestClass* const volatile       >() );
}

TEST_CASE("type-traits RemoveConstT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::RemoveConstType<               TestClass >,          TestClass >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveConstType<const          TestClass >,          TestClass >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveConstType<      volatile TestClass >, volatile TestClass >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveConstType<const volatile TestClass >, volatile TestClass >() );
	STATIC_REQUIRE(!bx::isSame<bx::RemoveConstType<const volatile TestClass*>, volatile TestClass*>() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveConstType<TestClass* const volatile>, TestClass* volatile>() );
}

TEST_CASE("type-traits AddVolatileT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<               TestClass >,       volatile TestClass           >() );
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<const          TestClass >, const volatile TestClass           >() );
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<      volatile TestClass >,       volatile TestClass           >() );
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<const volatile TestClass >, const volatile TestClass           >() );
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<const volatile TestClass*>, const volatile TestClass* volatile >() );
	STATIC_REQUIRE( bx::isSame<bx::AddVolatileType<TestClass* const volatile>, TestClass* const          volatile >() );
}

TEST_CASE("type-traits RemoveVolatileT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::RemoveVolatileType<               TestClass >,       TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveVolatileType<const          TestClass >, const TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveVolatileType<      volatile TestClass >,       TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveVolatileType<const volatile TestClass >, const TestClass  >() );
	STATIC_REQUIRE(!bx::isSame<bx::RemoveVolatileType<const volatile TestClass*>, const TestClass* >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveVolatileType<TestClass* const volatile>, TestClass* const >() );
}

TEST_CASE("type-traits AddCvT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<               TestClass >, const volatile TestClass                 >() );
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<const          TestClass >, const volatile TestClass                 >() );
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<      volatile TestClass >, const volatile TestClass                 >() );
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<const volatile TestClass >, const volatile TestClass                 >() );
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<const volatile TestClass*>, const volatile TestClass* const volatile >() );
	STATIC_REQUIRE( bx::isSame<bx::AddCvType<TestClass* const volatile>, TestClass* const volatile                >() );
}

TEST_CASE("type-traits RemoveCvT", "")
{
	STATIC_REQUIRE( bx::isSame<bx::RemoveCvType<               TestClass >, TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveCvType<const          TestClass >, TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveCvType<      volatile TestClass >, TestClass  >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveCvType<const volatile TestClass >, TestClass  >() );
	STATIC_REQUIRE(!bx::isSame<bx::RemoveCvType<const volatile TestClass*>, TestClass* >() );
	STATIC_REQUIRE( bx::isSame<bx::RemoveCvType<TestClass* const volatile>, TestClass* >() );
}

TEST_CASE("type-traits isTriviallyCopyable", "")
{
	STATIC_REQUIRE( bx::isTriviallyCopyable<int32_t              >() );
	STATIC_REQUIRE( bx::isTriviallyCopyable<TestClass            >() );
	STATIC_REQUIRE(!bx::isTriviallyCopyable<TestClassCtor        >() );
	STATIC_REQUIRE( bx::isTriviallyCopyable<TestClassDefaultCtor >() );
	STATIC_REQUIRE( bx::isTriviallyCopyable<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isTriviallyCopyable<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isTriviallyConstructible", "")
{
	STATIC_REQUIRE( bx::isTriviallyConstructible<int32_t              >() );
	STATIC_REQUIRE( bx::isTriviallyConstructible<TestClass            >() );
	STATIC_REQUIRE(!bx::isTriviallyConstructible<TestClassCtor        >() );
	STATIC_REQUIRE(!bx::isTriviallyConstructible<TestClassDefaultCtor >() );
	STATIC_REQUIRE( bx::isTriviallyConstructible<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isTriviallyConstructible<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isTriviallyDestructible", "")
{
	STATIC_REQUIRE( bx::isTriviallyDestructible<int32_t              >() );
	STATIC_REQUIRE( bx::isTriviallyDestructible<TestClass            >() );
	STATIC_REQUIRE( bx::isTriviallyDestructible<TestClassCtor        >() );
	STATIC_REQUIRE( bx::isTriviallyDestructible<TestClassDefaultCtor >() );
	STATIC_REQUIRE( bx::isTriviallyDestructible<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isTriviallyDestructible<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isConst", "")
{
	STATIC_REQUIRE(!bx::isConst<char>() );
	STATIC_REQUIRE( bx::isConst<const char>() );
	STATIC_REQUIRE( bx::isConst<bx::AddConstType<char>>() );
}

TEST_CASE("type-traits isVolatile", "")
{
	STATIC_REQUIRE(!bx::isVolatile<char>() );
	STATIC_REQUIRE( bx::isVolatile<volatile char>() );
	STATIC_REQUIRE( bx::isVolatile<bx::AddVolatileType<char>>() );
}

TEST_CASE("type-traits isSigned", "")
{
	STATIC_REQUIRE(!bx::isSigned<bool                   >() );
	STATIC_REQUIRE( bx::isSigned<char                   >() );
	STATIC_REQUIRE( bx::isSigned<signed char            >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned char          >() );
	STATIC_REQUIRE( bx::isSigned<short                  >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned short         >() );
	STATIC_REQUIRE( bx::isSigned<int                    >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned int           >() );
	STATIC_REQUIRE( bx::isSigned<long                   >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned long          >() );
	STATIC_REQUIRE( bx::isSigned<long long              >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned long long     >() );
	STATIC_REQUIRE( bx::isSigned<long long int          >() );
	STATIC_REQUIRE(!bx::isSigned<unsigned long long int >() );

	STATIC_REQUIRE( bx::isSigned<int8_t                 >() );
	STATIC_REQUIRE(!bx::isSigned<uint8_t                >() );
	STATIC_REQUIRE( bx::isSigned<int16_t                >() );
	STATIC_REQUIRE(!bx::isSigned<uint16_t               >() );
	STATIC_REQUIRE( bx::isSigned<int32_t                >() );
	STATIC_REQUIRE(!bx::isSigned<uint32_t               >() );
	STATIC_REQUIRE( bx::isSigned<int64_t                >() );
	STATIC_REQUIRE(!bx::isSigned<uint64_t               >() );
	STATIC_REQUIRE( bx::isSigned<intmax_t               >() );
	STATIC_REQUIRE(!bx::isSigned<uintmax_t              >() );
	STATIC_REQUIRE(!bx::isSigned<uintptr_t              >() );
	STATIC_REQUIRE( bx::isSigned<ptrdiff_t              >() );
	STATIC_REQUIRE(!bx::isSigned<size_t                 >() );

	STATIC_REQUIRE( bx::isSigned<float                  >() );
	STATIC_REQUIRE( bx::isSigned<double                 >() );
	STATIC_REQUIRE( bx::isSigned<long double            >() );
}

TEST_CASE("type-traits isUnsigned", "")
{
	STATIC_REQUIRE( bx::isUnsigned<bool                   >() );
	STATIC_REQUIRE(!bx::isUnsigned<char                   >() );
	STATIC_REQUIRE(!bx::isUnsigned<signed char            >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned char          >() );
	STATIC_REQUIRE(!bx::isUnsigned<short                  >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned short         >() );
	STATIC_REQUIRE(!bx::isUnsigned<int                    >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned int           >() );
	STATIC_REQUIRE(!bx::isUnsigned<long                   >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned long          >() );
	STATIC_REQUIRE(!bx::isUnsigned<long long              >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned long long     >() );
	STATIC_REQUIRE(!bx::isUnsigned<long long int          >() );
	STATIC_REQUIRE( bx::isUnsigned<unsigned long long int >() );

	STATIC_REQUIRE(!bx::isUnsigned<int8_t                 >() );
	STATIC_REQUIRE( bx::isUnsigned<uint8_t                >() );
	STATIC_REQUIRE(!bx::isUnsigned<int16_t                >() );
	STATIC_REQUIRE( bx::isUnsigned<uint16_t               >() );
	STATIC_REQUIRE(!bx::isUnsigned<int32_t                >() );
	STATIC_REQUIRE( bx::isUnsigned<uint32_t               >() );
	STATIC_REQUIRE(!bx::isUnsigned<int64_t                >() );
	STATIC_REQUIRE( bx::isUnsigned<uint64_t               >() );
	STATIC_REQUIRE(!bx::isUnsigned<intmax_t               >() );
	STATIC_REQUIRE( bx::isUnsigned<uintmax_t              >() );
	STATIC_REQUIRE( bx::isUnsigned<uintptr_t              >() );
	STATIC_REQUIRE(!bx::isUnsigned<ptrdiff_t              >() );
	STATIC_REQUIRE( bx::isUnsigned<size_t                 >() );

	STATIC_REQUIRE(!bx::isUnsigned<float                  >() );
	STATIC_REQUIRE(!bx::isUnsigned<double                 >() );
	STATIC_REQUIRE(!bx::isUnsigned<long double            >() );
}

TEST_CASE("type-traits MakeSignedT", "")
{
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<char                   >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<signed char            >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned char          >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<short                  >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned short         >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<int                    >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned int           >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<long                   >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned long          >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<long long              >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned long long     >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<long long int          >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<unsigned long long int >::Type >() );

	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<int8_t                 >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uint8_t                >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<int16_t                >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uint16_t               >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<int32_t                >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uint32_t               >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<int64_t                >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uint64_t               >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<intmax_t               >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uintmax_t              >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<uintptr_t              >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<ptrdiff_t              >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<size_t                 >::Type >() );

	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<float                  >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<double                 >::Type >() );
	STATIC_REQUIRE(bx::isSigned<bx::MakeSignedT<long double            >::Type >() );

	using charType = bx::MakeSignedType<unsigned char>;
	using intType  = bx::MakeSignedType<unsigned int>;
	using longType = bx::MakeSignedType<volatile unsigned long>;

	STATIC_REQUIRE(true
		&& bx::isSame<charType, signed char>()
		&& bx::isSame<intType, signed int>()
		&& bx::isSame<longType, volatile signed long>()
		);
}

TEST_CASE("type-traits MakeUnsignedT", "")
{
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<char                   >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<signed char            >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned char          >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<short                  >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned short         >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<int                    >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned int           >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<long                   >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned long          >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<long long              >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned long long     >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<long long int          >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<unsigned long long int >::Type >() );

	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<int8_t                 >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uint8_t                >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<int16_t                >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uint16_t               >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<int32_t                >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uint32_t               >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<int64_t                >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uint64_t               >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<intmax_t               >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uintmax_t              >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<uintptr_t              >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<ptrdiff_t              >::Type >() );
	STATIC_REQUIRE(bx::isUnsigned<bx::MakeUnsignedT<size_t                 >::Type >() );

	using ucharType = bx::MakeUnsignedType<char>;
	using uintType  = bx::MakeUnsignedType<int>;
	using ulongType = bx::MakeUnsignedType<volatile long>;

	STATIC_REQUIRE(true
		&& bx::isSame<ucharType, unsigned char>()
		&& bx::isSame<uintType, unsigned int>()
		&& bx::isSame<ulongType, volatile unsigned long>()
		);
}

TEST_CASE("type-traits isInteger", "")
{
	STATIC_REQUIRE( bx::isInteger<bool                   >() );
	STATIC_REQUIRE( bx::isInteger<char                   >() );
	STATIC_REQUIRE( bx::isInteger<signed char            >() );
	STATIC_REQUIRE( bx::isInteger<unsigned char          >() );
	STATIC_REQUIRE( bx::isInteger<short                  >() );
	STATIC_REQUIRE( bx::isInteger<unsigned short         >() );
	STATIC_REQUIRE( bx::isInteger<int                    >() );
	STATIC_REQUIRE( bx::isInteger<unsigned int           >() );
	STATIC_REQUIRE( bx::isInteger<long                   >() );
	STATIC_REQUIRE( bx::isInteger<unsigned long          >() );
	STATIC_REQUIRE( bx::isInteger<long long              >() );
	STATIC_REQUIRE( bx::isInteger<unsigned long long     >() );
	STATIC_REQUIRE( bx::isInteger<long long int          >() );
	STATIC_REQUIRE( bx::isInteger<unsigned long long int >() );

	STATIC_REQUIRE( bx::isInteger<int8_t                 >() );
	STATIC_REQUIRE( bx::isInteger<uint8_t                >() );
	STATIC_REQUIRE( bx::isInteger<int16_t                >() );
	STATIC_REQUIRE( bx::isInteger<uint16_t               >() );
	STATIC_REQUIRE( bx::isInteger<int32_t                >() );
	STATIC_REQUIRE( bx::isInteger<uint32_t               >() );
	STATIC_REQUIRE( bx::isInteger<int64_t                >() );
	STATIC_REQUIRE( bx::isInteger<uint64_t               >() );
	STATIC_REQUIRE( bx::isInteger<intmax_t               >() );
	STATIC_REQUIRE( bx::isInteger<uintmax_t              >() );
	STATIC_REQUIRE( bx::isInteger<uintptr_t              >() );
	STATIC_REQUIRE( bx::isInteger<ptrdiff_t              >() );
	STATIC_REQUIRE( bx::isInteger<size_t                 >() );

	STATIC_REQUIRE(!bx::isInteger<float                  >() );
	STATIC_REQUIRE(!bx::isInteger<double                 >() );
	STATIC_REQUIRE(!bx::isInteger<long double            >() );

	STATIC_REQUIRE(!bx::isInteger<TestClass            >() );
	STATIC_REQUIRE(!bx::isInteger<TestClassCtor        >() );
	STATIC_REQUIRE(!bx::isInteger<TestClassDefaultCtor >() );
	STATIC_REQUIRE(!bx::isInteger<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isInteger<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isFloatingPoint", "")
{
	STATIC_REQUIRE(!bx::isFloatingPoint<bool                   >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<char                   >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<signed char            >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned char          >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<short                  >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned short         >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<int                    >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned int           >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<long                   >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned long          >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<long long              >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned long long     >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<long long int          >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<unsigned long long int >() );

	STATIC_REQUIRE(!bx::isFloatingPoint<int8_t                 >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uint8_t                >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<int16_t                >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uint16_t               >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<int32_t                >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uint32_t               >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<int64_t                >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uint64_t               >() );

	STATIC_REQUIRE(!bx::isFloatingPoint<intmax_t               >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uintmax_t              >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<uintptr_t              >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<ptrdiff_t              >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<size_t                 >() );

	STATIC_REQUIRE( bx::isFloatingPoint<float                  >() );
	STATIC_REQUIRE( bx::isFloatingPoint<double                 >() );
	STATIC_REQUIRE( bx::isFloatingPoint<long double            >() );

	STATIC_REQUIRE(!bx::isFloatingPoint<TestClass            >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<TestClassCtor        >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<TestClassDefaultCtor >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isFloatingPoint<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isArithmetic", "")
{
	STATIC_REQUIRE( bx::isArithmetic<bool                   >() );
	STATIC_REQUIRE( bx::isArithmetic<char                   >() );
	STATIC_REQUIRE( bx::isArithmetic<signed char            >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned char          >() );
	STATIC_REQUIRE( bx::isArithmetic<short                  >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned short         >() );
	STATIC_REQUIRE( bx::isArithmetic<int                    >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned int           >() );
	STATIC_REQUIRE( bx::isArithmetic<long                   >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned long          >() );
	STATIC_REQUIRE( bx::isArithmetic<long long              >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned long long     >() );
	STATIC_REQUIRE( bx::isArithmetic<long long int          >() );
	STATIC_REQUIRE( bx::isArithmetic<unsigned long long int >() );

	STATIC_REQUIRE( bx::isArithmetic<int8_t                 >() );
	STATIC_REQUIRE( bx::isArithmetic<uint8_t                >() );
	STATIC_REQUIRE( bx::isArithmetic<int16_t                >() );
	STATIC_REQUIRE( bx::isArithmetic<uint16_t               >() );
	STATIC_REQUIRE( bx::isArithmetic<int32_t                >() );
	STATIC_REQUIRE( bx::isArithmetic<uint32_t               >() );
	STATIC_REQUIRE( bx::isArithmetic<int64_t                >() );
	STATIC_REQUIRE( bx::isArithmetic<uint64_t               >() );
	STATIC_REQUIRE( bx::isArithmetic<intmax_t               >() );
	STATIC_REQUIRE( bx::isArithmetic<uintmax_t              >() );
	STATIC_REQUIRE( bx::isArithmetic<uintptr_t              >() );
	STATIC_REQUIRE( bx::isArithmetic<ptrdiff_t              >() );
	STATIC_REQUIRE( bx::isArithmetic<size_t                 >() );

	STATIC_REQUIRE( bx::isArithmetic<float                  >() );
	STATIC_REQUIRE( bx::isArithmetic<double                 >() );
	STATIC_REQUIRE( bx::isArithmetic<long double            >() );

	STATIC_REQUIRE(!bx::isArithmetic<TestClass            >() );
	STATIC_REQUIRE(!bx::isArithmetic<TestClassCtor        >() );
	STATIC_REQUIRE(!bx::isArithmetic<TestClassDefaultCtor >() );
	STATIC_REQUIRE(!bx::isArithmetic<TestClassDefaultDtor >() );
	STATIC_REQUIRE(!bx::isArithmetic<TestClassVirtualDtor >() );
}

TEST_CASE("type-traits isBoundedArray", "")
{
	STATIC_REQUIRE(!bx::isBoundedArray<TestClass       >() );
	STATIC_REQUIRE(!bx::isBoundedArray<TestClass[]     >() );
	STATIC_REQUIRE( bx::isBoundedArray<TestClass[1389] >() );
	STATIC_REQUIRE(!bx::isBoundedArray<float           >() );
	STATIC_REQUIRE(!bx::isBoundedArray<int             >() );
	STATIC_REQUIRE(!bx::isBoundedArray<int[]           >() );
	STATIC_REQUIRE( bx::isBoundedArray<int[1389]       >() );
}

TEST_CASE("type-traits isUnboundedArray", "")
{
	STATIC_REQUIRE(!bx::isUnboundedArray<TestClass       >() );
	STATIC_REQUIRE( bx::isUnboundedArray<TestClass[]     >() );
	STATIC_REQUIRE(!bx::isUnboundedArray<TestClass[1389] >() );
	STATIC_REQUIRE(!bx::isUnboundedArray<float           >() );
	STATIC_REQUIRE(!bx::isUnboundedArray<int32_t         >() );
	STATIC_REQUIRE( bx::isUnboundedArray<int32_t[]       >() );
	STATIC_REQUIRE(!bx::isUnboundedArray<int32_t[1389]   >() );
}

TEST_CASE("type-traits isArray", "")
{
	STATIC_REQUIRE(!bx::isArray<TestClass       >() );
	STATIC_REQUIRE( bx::isArray<TestClass[]     >() );
	STATIC_REQUIRE( bx::isArray<TestClass[1389] >() );
	STATIC_REQUIRE(!bx::isArray<float           >() );
	STATIC_REQUIRE(!bx::isArray<int32_t         >() );
	STATIC_REQUIRE( bx::isArray<int32_t[]       >() );
	STATIC_REQUIRE( bx::isArray<int32_t[1389]   >() );
	STATIC_REQUIRE( bx::isArray<TestUnion[]     >() );
}

TEST_CASE("type-traits isEnum", "")
{
	STATIC_REQUIRE(!bx::isEnum<TestClass      >() );
	STATIC_REQUIRE(!bx::isEnum<TestUnion      >() );
	STATIC_REQUIRE(!bx::isEnum<TestUnionEmpty >() );
	STATIC_REQUIRE(!bx::isEnum<TestUnion[]    >() );
	STATIC_REQUIRE(!bx::isEnum<int32_t[]      >() );
	STATIC_REQUIRE( bx::isEnum<TestEnumEmpty  >() );
	STATIC_REQUIRE( bx::isEnum<TestEnum       >() );
}

TEST_CASE("type-traits isUnion", "")
{
	STATIC_REQUIRE(!bx::isUnion<TestClass      >() );
	STATIC_REQUIRE( bx::isUnion<TestUnion      >() );
	STATIC_REQUIRE( bx::isUnion<TestUnionEmpty >() );
	STATIC_REQUIRE(!bx::isUnion<TestUnion[]    >() );
	STATIC_REQUIRE(!bx::isUnion<int32_t[]      >() );
	STATIC_REQUIRE(!bx::isUnion<TestEnumEmpty  >() );
	STATIC_REQUIRE(!bx::isUnion<TestEnum       >() );
}

TEST_CASE("type-traits isClass", "")
{
	STATIC_REQUIRE( bx::isClass<TestClass              >() );
	STATIC_REQUIRE( bx::isClass<TestClassFinal         >() );
	STATIC_REQUIRE( bx::isClass<TestClassCtor          >() );
	STATIC_REQUIRE( bx::isClass<TestClassMember        >() );
	STATIC_REQUIRE( bx::isClass<TestClassMemberPrivate >() );
	STATIC_REQUIRE( bx::isClass<TestClassStaticOnly    >() );
	STATIC_REQUIRE( bx::isClass<TestClassDefaultCtor   >() );
	STATIC_REQUIRE( bx::isClass<TestClassDefaultDtor   >() );
	STATIC_REQUIRE( bx::isClass<TestClassVirtualDtor   >() );
	STATIC_REQUIRE( bx::isClass<TestClassAbstractBase  >() );
	STATIC_REQUIRE( bx::isClass<TestClassDerivedA      >() );
	STATIC_REQUIRE( bx::isClass<TestClassDerivedB      >() );
	STATIC_REQUIRE(!bx::isClass<TestUnion              >() );
	STATIC_REQUIRE(!bx::isClass<TestUnion[]            >() );
	STATIC_REQUIRE(!bx::isClass<int32_t[]              >() );
}

TEST_CASE("type-traits isFinal", "")
{
	STATIC_REQUIRE(!bx::isFinal<TestClass              >() );
	STATIC_REQUIRE( bx::isFinal<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassCtor          >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassMemberPrivate >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassStaticOnly    >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassDefaultCtor   >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassDefaultDtor   >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassVirtualDtor   >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassAbstractBase  >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassPolymorphic   >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassDerivedA      >() );
	STATIC_REQUIRE(!bx::isFinal<TestClassDerivedB      >() );
	STATIC_REQUIRE(!bx::isFinal<TestUnion              >() );
	STATIC_REQUIRE(!bx::isFinal<TestUnionEmpty         >() );
	STATIC_REQUIRE(!bx::isFinal<TestUnion[]            >() );
	STATIC_REQUIRE(!bx::isFinal<int32_t[]              >() );
}

TEST_CASE("type-traits isEmpty", "")
{
	STATIC_REQUIRE( bx::isEmpty<TestClass             >() );
	STATIC_REQUIRE( bx::isEmpty<TestClassFinal        >() );
	STATIC_REQUIRE( bx::isEmpty<TestClassCtor         >() );
	STATIC_REQUIRE( bx::isEmpty<TestClassDefaultCtor  >() );
	STATIC_REQUIRE( bx::isEmpty<TestClassDefaultDtor  >() );
	STATIC_REQUIRE(!bx::isEmpty<TestClassVirtualDtor  >() );
	STATIC_REQUIRE(!bx::isEmpty<TestClassAbstractBase >() );
	STATIC_REQUIRE(!bx::isEmpty<TestClassPolymorphic  >() );
	STATIC_REQUIRE(!bx::isEmpty<TestUnion             >() );
	STATIC_REQUIRE(!bx::isEmpty<TestUnionEmpty        >() );
	STATIC_REQUIRE(!bx::isEmpty<TestUnion[]           >() );
	STATIC_REQUIRE(!bx::isEmpty<int32_t[]             >() );
}

TEST_CASE("type-traits isStandardLayout", "")
{
	STATIC_REQUIRE( bx::isStandardLayout<TestClass              >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassFinal         >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassCtor          >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isStandardLayout<TestClassMemberPrivate >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassStaticOnly    >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassDefaultCtor   >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassDefaultDtor   >() );
	STATIC_REQUIRE(!bx::isStandardLayout<TestClassVirtualDtor   >() );
	STATIC_REQUIRE(!bx::isStandardLayout<TestClassAbstractBase  >() );
	STATIC_REQUIRE(!bx::isStandardLayout<TestClassPolymorphic   >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassDerivedA      >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestClassDerivedB      >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestUnion              >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestUnionEmpty         >() );
	STATIC_REQUIRE( bx::isStandardLayout<TestUnion[]            >() );
	STATIC_REQUIRE( bx::isStandardLayout<int32_t[]              >() );
}

TEST_CASE("type-traits isTrivial", "")
{
	STATIC_REQUIRE( bx::isTrivial<TestClass              >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isTrivial<TestClassCtor          >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassMember        >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassMemberPrivate >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassStaticOnly    >() );
	STATIC_REQUIRE(!bx::isTrivial<TestClassDefaultCtor   >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassDefaultDtor   >() );
	STATIC_REQUIRE(!bx::isTrivial<TestClassVirtualDtor   >() );
	STATIC_REQUIRE(!bx::isTrivial<TestClassAbstractBase  >() );
	STATIC_REQUIRE(!bx::isTrivial<TestClassPolymorphic   >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassDerivedA      >() );
	STATIC_REQUIRE( bx::isTrivial<TestClassDerivedB      >() );
	STATIC_REQUIRE( bx::isTrivial<TestUnion              >() );
	STATIC_REQUIRE( bx::isTrivial<TestUnionEmpty         >() );
	STATIC_REQUIRE( bx::isTrivial<TestUnion[]            >() );
	STATIC_REQUIRE( bx::isTrivial<int32_t[]              >() );
}

TEST_CASE("type-traits isPod", "")
{
	STATIC_REQUIRE( bx::isPod<TestClass              >() );
	STATIC_REQUIRE( bx::isPod<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isPod<TestClassCtor          >() );
	STATIC_REQUIRE( bx::isPod<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isPod<TestClassMemberPrivate >() );
	STATIC_REQUIRE( bx::isPod<TestClassStaticOnly    >() );
	STATIC_REQUIRE(!bx::isPod<TestClassDefaultCtor   >() );
	STATIC_REQUIRE( bx::isPod<TestClassDefaultDtor   >() );
	STATIC_REQUIRE(!bx::isPod<TestClassVirtualDtor   >() );
	STATIC_REQUIRE(!bx::isPod<TestClassPolymorphic   >() );
	STATIC_REQUIRE(!bx::isPod<TestClassAbstractBase  >() );
	STATIC_REQUIRE( bx::isPod<TestClassDerivedA      >() );
	STATIC_REQUIRE( bx::isPod<TestClassDerivedB      >() );
	STATIC_REQUIRE( bx::isPod<TestUnion              >() );
	STATIC_REQUIRE( bx::isPod<TestUnionEmpty         >() );
	STATIC_REQUIRE( bx::isPod<TestUnion[]            >() );
	STATIC_REQUIRE( bx::isPod<int32_t[]              >() );
}

TEST_CASE("type-traits isPolymorphic", "")
{
	STATIC_REQUIRE(!bx::isPolymorphic<TestClass              >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassCtor          >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassMemberPrivate >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassStaticOnly    >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassDefaultCtor   >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassDefaultDtor   >() );
	STATIC_REQUIRE( bx::isPolymorphic<TestClassVirtualDtor   >() );
	STATIC_REQUIRE( bx::isPolymorphic<TestClassAbstractBase  >() );
	STATIC_REQUIRE( bx::isPolymorphic<TestClassPolymorphic   >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassDerivedA      >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestClassDerivedB      >() );
	STATIC_REQUIRE( bx::isPolymorphic<TestClassDerivedX      >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestUnion              >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestUnionEmpty         >() );
	STATIC_REQUIRE(!bx::isPolymorphic<TestUnion[]            >() );
	STATIC_REQUIRE(!bx::isPolymorphic<int32_t[]              >() );
}

TEST_CASE("type-traits isDestructorVirtual", "")
{
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClass              >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassCtor          >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassMemberPrivate >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassStaticOnly    >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassDefaultCtor   >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassDefaultDtor   >() );
	STATIC_REQUIRE( bx::isDestructorVirtual<TestClassVirtualDtor   >() );
	STATIC_REQUIRE( bx::isDestructorVirtual<TestClassAbstractBase  >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassPolymorphic   >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassDerivedA      >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestClassDerivedB      >() );
	STATIC_REQUIRE( bx::isDestructorVirtual<TestClassDerivedX      >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestUnion              >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestUnionEmpty         >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<TestUnion[]            >() );
	STATIC_REQUIRE(!bx::isDestructorVirtual<int32_t[]              >() );
}

TEST_CASE("type-traits isBaseOf", "")
{
	STATIC_REQUIRE( bx::isBaseOf<TestClass,         TestClass         >() );
	STATIC_REQUIRE( bx::isBaseOf<TestClass,         TestClassDerivedA >() );
	STATIC_REQUIRE( bx::isBaseOf<TestClass,         TestClassDerivedB >() );
	STATIC_REQUIRE(!bx::isBaseOf<TestClass,         TestClassFinal    >() );
	STATIC_REQUIRE(!bx::isBaseOf<TestClassDerivedB, TestClassDerivedA >() );
	STATIC_REQUIRE(!bx::isBaseOf<TestClassDerivedB, TestClassDerivedX >() );
	STATIC_REQUIRE(!bx::isBaseOf<int32_t,           int32_t           >() );
}

TEST_CASE("type-traits isAggregate", "")
{
	STATIC_REQUIRE( bx::isAggregate<TestClass              >() );
	STATIC_REQUIRE( bx::isAggregate<TestClassFinal         >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassCtor          >() );
	STATIC_REQUIRE( bx::isAggregate<TestClassMember        >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassMemberPrivate >() );
	STATIC_REQUIRE( bx::isAggregate<TestClassStaticOnly    >() );
#if __cplusplus < BX_LANGUAGE_CPP20
	STATIC_REQUIRE( bx::isAggregate<TestClassDefaultCtor   >() );
#else
	STATIC_REQUIRE(!bx::isAggregate<TestClassDefaultCtor   >() );
#endif
	STATIC_REQUIRE( bx::isAggregate<TestClassDefaultDtor   >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassVirtualDtor   >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassAbstractBase  >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassPolymorphic   >() );
#if __cplusplus < BX_LANGUAGE_CPP17
	STATIC_REQUIRE(!bx::isAggregate<TestClassDerivedA      >() );
	STATIC_REQUIRE(!bx::isAggregate<TestClassDerivedB      >() );
#else
	STATIC_REQUIRE( bx::isAggregate<TestClassDerivedA      >() );
	STATIC_REQUIRE( bx::isAggregate<TestClassDerivedB      >() );
#endif
	STATIC_REQUIRE(!bx::isAggregate<TestClassDerivedX      >() );
	STATIC_REQUIRE( bx::isAggregate<TestUnion              >() );
	STATIC_REQUIRE( bx::isAggregate<TestUnionEmpty         >() );
	STATIC_REQUIRE( bx::isAggregate<TestUnion[]            >() );
	STATIC_REQUIRE( bx::isAggregate<int32_t[]              >() );
}
