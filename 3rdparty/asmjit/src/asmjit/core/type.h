// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef ASMJIT_CORE_TYPE_H_INCLUDED
#define ASMJIT_CORE_TYPE_H_INCLUDED

#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::Type]
// ============================================================================

//! Provides minimum type-system that is used by \ref asmjit_func and \ref asmjit_compiler.
namespace Type {

//! TypeId.
//!
//! This is an additional information that can be used to describe a value-type
//! of physical or virtual register. it's used mostly by BaseCompiler to describe
//! register representation (the group of data stored in the register and the
//! width used) and it's also used by APIs that allow to describe and work with
//! function signatures.
enum Id : uint32_t {
  kIdVoid         = 0,

  _kIdBaseStart   = 32,
  _kIdBaseEnd     = 44,

  _kIdIntStart    = 32,
  _kIdIntEnd      = 41,

  kIdIntPtr       = 32,
  kIdUIntPtr      = 33,

  kIdI8           = 34,
  kIdU8           = 35,
  kIdI16          = 36,
  kIdU16          = 37,
  kIdI32          = 38,
  kIdU32          = 39,
  kIdI64          = 40,
  kIdU64          = 41,

  _kIdFloatStart  = 42,
  _kIdFloatEnd    = 44,

  kIdF32          = 42,
  kIdF64          = 43,
  kIdF80          = 44,

  _kIdMaskStart   = 45,
  _kIdMaskEnd     = 48,

  kIdMask8        = 45,
  kIdMask16       = 46,
  kIdMask32       = 47,
  kIdMask64       = 48,

  _kIdMmxStart    = 49,
  _kIdMmxEnd      = 50,

  kIdMmx32        = 49,
  kIdMmx64        = 50,

  _kIdVec32Start  = 51,
  _kIdVec32End    = 60,

  kIdI8x4         = 51,
  kIdU8x4         = 52,
  kIdI16x2        = 53,
  kIdU16x2        = 54,
  kIdI32x1        = 55,
  kIdU32x1        = 56,
  kIdF32x1        = 59,

  _kIdVec64Start  = 61,
  _kIdVec64End    = 70,

  kIdI8x8         = 61,
  kIdU8x8         = 62,
  kIdI16x4        = 63,
  kIdU16x4        = 64,
  kIdI32x2        = 65,
  kIdU32x2        = 66,
  kIdI64x1        = 67,
  kIdU64x1        = 68,
  kIdF32x2        = 69,
  kIdF64x1        = 70,

  _kIdVec128Start = 71,
  _kIdVec128End   = 80,

  kIdI8x16        = 71,
  kIdU8x16        = 72,
  kIdI16x8        = 73,
  kIdU16x8        = 74,
  kIdI32x4        = 75,
  kIdU32x4        = 76,
  kIdI64x2        = 77,
  kIdU64x2        = 78,
  kIdF32x4        = 79,
  kIdF64x2        = 80,

  _kIdVec256Start = 81,
  _kIdVec256End   = 90,

  kIdI8x32        = 81,
  kIdU8x32        = 82,
  kIdI16x16       = 83,
  kIdU16x16       = 84,
  kIdI32x8        = 85,
  kIdU32x8        = 86,
  kIdI64x4        = 87,
  kIdU64x4        = 88,
  kIdF32x8        = 89,
  kIdF64x4        = 90,

  _kIdVec512Start = 91,
  _kIdVec512End   = 100,

  kIdI8x64        = 91,
  kIdU8x64        = 92,
  kIdI16x32       = 93,
  kIdU16x32       = 94,
  kIdI32x16       = 95,
  kIdU32x16       = 96,
  kIdI64x8        = 97,
  kIdU64x8        = 98,
  kIdF32x16       = 99,
  kIdF64x8        = 100,

  kIdCount        = 101,
  kIdMax          = 255
};

struct TypeData {
  uint8_t baseOf[kIdMax + 1];
  uint8_t sizeOf[kIdMax + 1];
};
ASMJIT_VARAPI const TypeData _typeData;

static constexpr bool isVoid(uint32_t typeId) noexcept { return typeId == 0; }
static constexpr bool isValid(uint32_t typeId) noexcept { return typeId >= _kIdIntStart && typeId <= _kIdVec512End; }
static constexpr bool isBase(uint32_t typeId) noexcept { return typeId >= _kIdBaseStart && typeId <= _kIdBaseEnd; }
static constexpr bool isAbstract(uint32_t typeId) noexcept { return typeId >= kIdIntPtr && typeId <= kIdUIntPtr; }

static constexpr bool isInt(uint32_t typeId) noexcept { return typeId >= _kIdIntStart && typeId <= _kIdIntEnd; }
static constexpr bool isInt8(uint32_t typeId) noexcept { return typeId == kIdI8; }
static constexpr bool isUInt8(uint32_t typeId) noexcept { return typeId == kIdU8; }
static constexpr bool isInt16(uint32_t typeId) noexcept { return typeId == kIdI16; }
static constexpr bool isUInt16(uint32_t typeId) noexcept { return typeId == kIdU16; }
static constexpr bool isInt32(uint32_t typeId) noexcept { return typeId == kIdI32; }
static constexpr bool isUInt32(uint32_t typeId) noexcept { return typeId == kIdU32; }
static constexpr bool isInt64(uint32_t typeId) noexcept { return typeId == kIdI64; }
static constexpr bool isUInt64(uint32_t typeId) noexcept { return typeId == kIdU64; }

static constexpr bool isGp8(uint32_t typeId) noexcept { return typeId >= kIdI8 && typeId <= kIdU8; }
static constexpr bool isGp16(uint32_t typeId) noexcept { return typeId >= kIdI16 && typeId <= kIdU16; }
static constexpr bool isGp32(uint32_t typeId) noexcept { return typeId >= kIdI32 && typeId <= kIdU32; }
static constexpr bool isGp64(uint32_t typeId) noexcept { return typeId >= kIdI64 && typeId <= kIdU64; }

static constexpr bool isFloat(uint32_t typeId) noexcept { return typeId >= _kIdFloatStart && typeId <= _kIdFloatEnd; }
static constexpr bool isFloat32(uint32_t typeId) noexcept { return typeId == kIdF32; }
static constexpr bool isFloat64(uint32_t typeId) noexcept { return typeId == kIdF64; }
static constexpr bool isFloat80(uint32_t typeId) noexcept { return typeId == kIdF80; }

static constexpr bool isMask(uint32_t typeId) noexcept { return typeId >= _kIdMaskStart && typeId <= _kIdMaskEnd; }
static constexpr bool isMask8(uint32_t typeId) noexcept { return typeId == kIdMask8; }
static constexpr bool isMask16(uint32_t typeId) noexcept { return typeId == kIdMask16; }
static constexpr bool isMask32(uint32_t typeId) noexcept { return typeId == kIdMask32; }
static constexpr bool isMask64(uint32_t typeId) noexcept { return typeId == kIdMask64; }

static constexpr bool isMmx(uint32_t typeId) noexcept { return typeId >= _kIdMmxStart && typeId <= _kIdMmxEnd; }
static constexpr bool isMmx32(uint32_t typeId) noexcept { return typeId == kIdMmx32; }
static constexpr bool isMmx64(uint32_t typeId) noexcept { return typeId == kIdMmx64; }

static constexpr bool isVec(uint32_t typeId) noexcept { return typeId >= _kIdVec32Start && typeId <= _kIdVec512End; }
static constexpr bool isVec32(uint32_t typeId) noexcept { return typeId >= _kIdVec32Start && typeId <= _kIdVec32End; }
static constexpr bool isVec64(uint32_t typeId) noexcept { return typeId >= _kIdVec64Start && typeId <= _kIdVec64End; }
static constexpr bool isVec128(uint32_t typeId) noexcept { return typeId >= _kIdVec128Start && typeId <= _kIdVec128End; }
static constexpr bool isVec256(uint32_t typeId) noexcept { return typeId >= _kIdVec256Start && typeId <= _kIdVec256End; }
static constexpr bool isVec512(uint32_t typeId) noexcept { return typeId >= _kIdVec512Start && typeId <= _kIdVec512End; }

//! IdOfT<> template allows to get a TypeId of a C++ `T` type.
template<typename T> struct IdOfT { /* Fail if not specialized. */ };

//! \cond
template<typename T> struct IdOfT<T*> {
  enum : uint32_t { kTypeId = kIdUIntPtr };
};

template<typename T> struct IdOfT<T&> {
  enum : uint32_t { kTypeId = kIdUIntPtr };
};

template<typename T>
struct IdOfIntT {
  static constexpr uint32_t kTypeId =
    sizeof(T) == 1 ? (std::is_signed<T>::value ? kIdI8  : kIdU8 ) :
    sizeof(T) == 2 ? (std::is_signed<T>::value ? kIdI16 : kIdU16) :
    sizeof(T) == 4 ? (std::is_signed<T>::value ? kIdI32 : kIdU32) :
    sizeof(T) == 8 ? (std::is_signed<T>::value ? kIdI64 : kIdU64) : kIdVoid;
};

template<uint32_t TYPE_ID>
struct BaseOfTypeId {
  static constexpr uint32_t kTypeId =
    isBase  (TYPE_ID) ? TYPE_ID :
    isMask8 (TYPE_ID) ? kIdU8   :
    isMask16(TYPE_ID) ? kIdU16  :
    isMask32(TYPE_ID) ? kIdU32  :
    isMask64(TYPE_ID) ? kIdU64  :
    isMmx32 (TYPE_ID) ? kIdI32  :
    isMmx64 (TYPE_ID) ? kIdI64  :
    isVec32 (TYPE_ID) ? TYPE_ID + kIdI8 - _kIdVec32Start  :
    isVec64 (TYPE_ID) ? TYPE_ID + kIdI8 - _kIdVec64Start  :
    isVec128(TYPE_ID) ? TYPE_ID + kIdI8 - _kIdVec128Start :
    isVec256(TYPE_ID) ? TYPE_ID + kIdI8 - _kIdVec256Start :
    isVec512(TYPE_ID) ? TYPE_ID + kIdI8 - _kIdVec512Start : 0;
};

template<uint32_t TYPE_ID>
struct SizeOfTypeId {
  static constexpr uint32_t kTypeSize =
    isInt8   (TYPE_ID) ?  1 :
    isUInt8  (TYPE_ID) ?  1 :
    isInt16  (TYPE_ID) ?  2 :
    isUInt16 (TYPE_ID) ?  2 :
    isInt32  (TYPE_ID) ?  4 :
    isUInt32 (TYPE_ID) ?  4 :
    isInt64  (TYPE_ID) ?  8 :
    isUInt64 (TYPE_ID) ?  8 :
    isFloat32(TYPE_ID) ?  4 :
    isFloat64(TYPE_ID) ?  8 :
    isFloat80(TYPE_ID) ? 10 :
    isMask8  (TYPE_ID) ?  1 :
    isMask16 (TYPE_ID) ?  2 :
    isMask32 (TYPE_ID) ?  4 :
    isMask64 (TYPE_ID) ?  8 :
    isMmx32  (TYPE_ID) ?  4 :
    isMmx64  (TYPE_ID) ?  8 :
    isVec32  (TYPE_ID) ?  4 :
    isVec64  (TYPE_ID) ?  8 :
    isVec128 (TYPE_ID) ? 16 :
    isVec256 (TYPE_ID) ? 32 :
    isVec512 (TYPE_ID) ? 64 : 0;
};
//! \endcond

static inline uint32_t baseOf(uint32_t typeId) noexcept {
  ASMJIT_ASSERT(typeId <= kIdMax);
  return _typeData.baseOf[typeId];
}

static inline uint32_t sizeOf(uint32_t typeId) noexcept {
  ASMJIT_ASSERT(typeId <= kIdMax);
  return _typeData.sizeOf[typeId];
}

//! Returns offset needed to convert a `kIntPtr` and `kUIntPtr` TypeId
//! into a type that matches `gpSize` (general-purpose register size).
//! If you find such TypeId it's then only about adding the offset to it.
//!
//! For example:
//!
//! ```
//! uint32_t gpSize = '4' or '8';
//! uint32_t deabstractDelta = Type::deabstractDeltaOfSize(gpSize);
//!
//! uint32_t typeId = 'some type-id';
//!
//! // Normalize some typeId into a non-abstract typeId.
//! if (Type::isAbstract(typeId)) typeId += deabstractDelta;
//!
//! // The same, but by using Type::deabstract() function.
//! typeId = Type::deabstract(typeId, deabstractDelta);
//! ```
static constexpr uint32_t deabstractDeltaOfSize(uint32_t gpSize) noexcept {
  return gpSize >= 8 ? kIdI64 - kIdIntPtr : kIdI32 - kIdIntPtr;
}

static constexpr uint32_t deabstract(uint32_t typeId, uint32_t deabstractDelta) noexcept {
  return isAbstract(typeId) ? typeId + deabstractDelta : typeId;
}

//! bool as C++ type-name.
struct Bool {};
//! int8_t as C++ type-name.
struct I8 {};
//! uint8_t as C++ type-name.
struct U8 {};
//! int16_t as C++ type-name.
struct I16 {};
//! uint16_t as C++ type-name.
struct U16 {};
//! int32_t as C++ type-name.
struct I32 {};
//! uint32_t as C++ type-name.
struct U32 {};
//! int64_t as C++ type-name.
struct I64 {};
//! uint64_t as C++ type-name.
struct U64 {};
//! intptr_t as C++ type-name.
struct IPtr {};
//! uintptr_t as C++ type-name.
struct UPtr {};
//! float as C++ type-name.
struct F32 {};
//! double as C++ type-name.
struct F64 {};

} // {Type}

// ============================================================================
// [ASMJIT_DEFINE_TYPE_ID]
// ============================================================================

//! \cond
#define ASMJIT_DEFINE_TYPE_ID(T, TYPE_ID)  \
namespace Type {                           \
  template<>                               \
  struct IdOfT<T> {                        \
    enum : uint32_t { kTypeId = TYPE_ID }; \
  };                                       \
}

ASMJIT_DEFINE_TYPE_ID(bool              , IdOfIntT<bool              >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(char              , IdOfIntT<char              >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(signed char       , IdOfIntT<signed char       >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned char     , IdOfIntT<unsigned char     >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(short             , IdOfIntT<short             >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned short    , IdOfIntT<unsigned short    >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(int               , IdOfIntT<int               >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned int      , IdOfIntT<unsigned int      >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(long              , IdOfIntT<long              >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned long     , IdOfIntT<unsigned long     >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(long long         , IdOfIntT<long long         >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(unsigned long long, IdOfIntT<unsigned long long>::kTypeId);

#if ASMJIT_CXX_HAS_NATIVE_WCHAR_T
ASMJIT_DEFINE_TYPE_ID(wchar_t           , IdOfIntT<wchar_t           >::kTypeId);
#endif

#if ASMJIT_CXX_HAS_UNICODE_LITERALS
ASMJIT_DEFINE_TYPE_ID(char16_t          , IdOfIntT<char16_t          >::kTypeId);
ASMJIT_DEFINE_TYPE_ID(char32_t          , IdOfIntT<char32_t          >::kTypeId);
#endif

ASMJIT_DEFINE_TYPE_ID(void              , kIdVoid);
ASMJIT_DEFINE_TYPE_ID(float             , kIdF32);
ASMJIT_DEFINE_TYPE_ID(double            , kIdF64);

ASMJIT_DEFINE_TYPE_ID(Bool              , kIdU8);
ASMJIT_DEFINE_TYPE_ID(I8                , kIdI8);
ASMJIT_DEFINE_TYPE_ID(U8                , kIdU8);
ASMJIT_DEFINE_TYPE_ID(I16               , kIdI16);
ASMJIT_DEFINE_TYPE_ID(U16               , kIdU16);
ASMJIT_DEFINE_TYPE_ID(I32               , kIdI32);
ASMJIT_DEFINE_TYPE_ID(U32               , kIdU32);
ASMJIT_DEFINE_TYPE_ID(I64               , kIdI64);
ASMJIT_DEFINE_TYPE_ID(U64               , kIdU64);
ASMJIT_DEFINE_TYPE_ID(IPtr              , kIdIntPtr);
ASMJIT_DEFINE_TYPE_ID(UPtr              , kIdUIntPtr);
ASMJIT_DEFINE_TYPE_ID(F32               , kIdF32);
ASMJIT_DEFINE_TYPE_ID(F64               , kIdF64);
//! \endcond

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_TYPE_H_INCLUDED
