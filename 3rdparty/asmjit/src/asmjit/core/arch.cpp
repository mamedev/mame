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

#include "../core/api-build_p.h"
#include "../core/arch.h"
#include "../core/support.h"
#include "../core/type.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86operand.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/armoperand.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ArchInfo]
// ============================================================================

// NOTE: Keep `const constexpr` otherwise MSC would not compile this code correctly.
static const constexpr uint32_t archInfoTable[] = {
  // <--------------------+---------------------+-------------------+-------+
  //                      | Type                | SubType           | GPInfo|
  // <--------------------+---------------------+-------------------+-------+
  Support::bytepack32_4x8(ArchInfo::kIdNone  , ArchInfo::kSubIdNone, 0,  0),
  Support::bytepack32_4x8(ArchInfo::kIdX86   , ArchInfo::kSubIdNone, 4,  8),
  Support::bytepack32_4x8(ArchInfo::kIdX64   , ArchInfo::kSubIdNone, 8, 16),
  Support::bytepack32_4x8(ArchInfo::kIdA32   , ArchInfo::kSubIdNone, 4, 16),
  Support::bytepack32_4x8(ArchInfo::kIdA64   , ArchInfo::kSubIdNone, 8, 32)
};

ASMJIT_FAVOR_SIZE void ArchInfo::init(uint32_t id, uint32_t subId) noexcept {
  uint32_t index = id < ASMJIT_ARRAY_SIZE(archInfoTable) ? id : uint32_t(0);

  // Make sure the `archInfoTable` array is correctly indexed.
  _signature = archInfoTable[index];
  ASMJIT_ASSERT(_id == index);

  // Even if the architecture is not known we setup its id and sub-id,
  // however, such architecture is not really useful.
  _id = uint8_t(id);
  _subId = uint8_t(subId);
}

// ============================================================================
// [asmjit::ArchUtils]
// ============================================================================

ASMJIT_FAVOR_SIZE Error ArchUtils::typeIdToRegInfo(uint32_t archId, uint32_t& typeIdInOut, RegInfo& regInfo) noexcept {
  uint32_t typeId = typeIdInOut;

  // Zero the signature so it's clear in case that typeId is not invalid.
  regInfo._signature = 0;

  // TODO: Move to X86 backend.
#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId)) {
    // Passed RegType instead of TypeId?
    if (typeId <= BaseReg::kTypeMax)
      typeId = x86::opData.archRegs.regTypeToTypeId[typeId];

    if (ASMJIT_UNLIKELY(!Type::isValid(typeId)))
      return DebugUtils::errored(kErrorInvalidTypeId);

    // First normalize architecture dependent types.
    if (Type::isAbstract(typeId)) {
      if (typeId == Type::kIdIntPtr)
        typeId = (archId == ArchInfo::kIdX86) ? Type::kIdI32 : Type::kIdI64;
      else
        typeId = (archId == ArchInfo::kIdX86) ? Type::kIdU32 : Type::kIdU64;
    }

    // Type size helps to construct all groupss of registers. If the size is zero
    // then the TypeId is invalid.
    uint32_t size = Type::sizeOf(typeId);
    if (ASMJIT_UNLIKELY(!size))
      return DebugUtils::errored(kErrorInvalidTypeId);

    if (ASMJIT_UNLIKELY(typeId == Type::kIdF80))
      return DebugUtils::errored(kErrorInvalidUseOfF80);

    uint32_t regType = 0;

    switch (typeId) {
      case Type::kIdI8:
      case Type::kIdU8:
        regType = x86::Reg::kTypeGpbLo;
        break;

      case Type::kIdI16:
      case Type::kIdU16:
        regType = x86::Reg::kTypeGpw;
        break;

      case Type::kIdI32:
      case Type::kIdU32:
        regType = x86::Reg::kTypeGpd;
        break;

      case Type::kIdI64:
      case Type::kIdU64:
        if (archId == ArchInfo::kIdX86)
          return DebugUtils::errored(kErrorInvalidUseOfGpq);

        regType = x86::Reg::kTypeGpq;
        break;

      // F32 and F64 are always promoted to use vector registers.
      case Type::kIdF32:
        typeId = Type::kIdF32x1;
        regType = x86::Reg::kTypeXmm;
        break;

      case Type::kIdF64:
        typeId = Type::kIdF64x1;
        regType = x86::Reg::kTypeXmm;
        break;

      // Mask registers {k}.
      case Type::kIdMask8:
      case Type::kIdMask16:
      case Type::kIdMask32:
      case Type::kIdMask64:
        regType = x86::Reg::kTypeKReg;
        break;

      // MMX registers.
      case Type::kIdMmx32:
      case Type::kIdMmx64:
        regType = x86::Reg::kTypeMm;
        break;

      // XMM|YMM|ZMM registers.
      default:
        if (size <= 16)
          regType = x86::Reg::kTypeXmm;
        else if (size == 32)
          regType = x86::Reg::kTypeYmm;
        else
          regType = x86::Reg::kTypeZmm;
        break;
    }

    typeIdInOut = typeId;
    regInfo._signature = x86::opData.archRegs.regInfo[regType].signature();
    return kErrorOk;
  }
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_END_NAMESPACE
