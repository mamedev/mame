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

#ifndef ASMJIT_CORE_ARCH_H_INCLUDED
#define ASMJIT_CORE_ARCH_H_INCLUDED

#include "../core/globals.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ArchInfo]
// ============================================================================

class ArchInfo {
public:
  union {
    struct {
      //! Architecture id.
      uint8_t _id;
      //! Architecture sub-id.
      uint8_t _subId;
      //! Default size of a general purpose register.
      uint8_t _gpSize;
      //! Count of all general purpose registers.
      uint8_t _gpCount;
    };
    //! Architecture signature (32-bit int).
    uint32_t _signature;
  };

  //! Architecture id.
  enum Id : uint32_t {
    kIdNone  = 0,                        //!< No/Unknown architecture.

    // X86 architectures.
    kIdX86   = 1,                        //!< X86 architecture (32-bit).
    kIdX64   = 2,                        //!< X64 architecture (64-bit) (AMD64).

    // ARM architectures.
    kIdA32   = 3,                        //!< ARM 32-bit architecture (AArch32/ARM/THUMB).
    kIdA64   = 4,                        //!< ARM 64-bit architecture (AArch64).

    //! Architecture detected at compile-time (architecture of the host).
    kIdHost  = ASMJIT_ARCH_X86 == 32 ? kIdX86 :
               ASMJIT_ARCH_X86 == 64 ? kIdX64 :
               ASMJIT_ARCH_ARM == 32 ? kIdA32 :
               ASMJIT_ARCH_ARM == 64 ? kIdA64 : kIdNone
  };

  //! Architecture sub-type or execution mode.
  enum SubType : uint32_t {
    kSubIdNone         = 0,              //!< Default mode (or no specific mode).

    // X86 sub-types.
    kSubIdX86_AVX      = 1,              //!< Code generation uses AVX         by default (VEC instructions).
    kSubIdX86_AVX2     = 2,              //!< Code generation uses AVX2        by default (VEC instructions).
    kSubIdX86_AVX512   = 3,              //!< Code generation uses AVX-512F    by default (+32 vector regs).
    kSubIdX86_AVX512VL = 4,              //!< Code generation uses AVX-512F-VL by default (+VL extensions).

    // ARM sub-types.
    kSubIdA32_Thumb    = 8,              //!< THUMB|THUMBv2 sub-type (only ARM in 32-bit mode).

#if   (ASMJIT_ARCH_X86) && defined(__AVX512VL__)
    kSubIdHost = kSubIdX86_AVX512VL
#elif (ASMJIT_ARCH_X86) && defined(__AVX512F__)
    kSubIdHost = kSubIdX86_AVX512
#elif (ASMJIT_ARCH_X86) && defined(__AVX2__)
    kSubIdHost = kSubIdX86_AVX2
#elif (ASMJIT_ARCH_X86) && defined(__AVX__)
    kSubIdHost = kSubIdX86_AVX
#elif (ASMJIT_ARCH_ARM == 32) && (defined(_M_ARMT) || defined(__thumb__) || defined(__thumb2__))
    kSubIdHost = kSubIdA32_Thumb
#else
    kSubIdHost = 0
#endif
  };

  //! \name Construction & Destruction
  //! \{

  inline ArchInfo() noexcept : _signature(0) {}
  inline ArchInfo(const ArchInfo& other) noexcept : _signature(other._signature) {}
  inline explicit ArchInfo(uint32_t type, uint32_t subType = kSubIdNone) noexcept { init(type, subType); }
  inline explicit ArchInfo(Globals::NoInit_) noexcept {}

  inline static ArchInfo host() noexcept { return ArchInfo(kIdHost, kSubIdHost); }

  inline bool isInitialized() const noexcept { return _id != kIdNone; }

  ASMJIT_API void init(uint32_t type, uint32_t subType = kSubIdNone) noexcept;
  inline void reset() noexcept { _signature = 0; }

  //! \}

  //! \name Overloaded Operators
  //! \{

  inline ArchInfo& operator=(const ArchInfo& other) noexcept = default;

  inline bool operator==(const ArchInfo& other) const noexcept { return _signature == other._signature; }
  inline bool operator!=(const ArchInfo& other) const noexcept { return _signature != other._signature; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the architecture id, see `Id`.
  inline uint32_t archId() const noexcept { return _id; }

  //! Returns the architecture sub-id, see `SubType`.
  //!
  //! X86 & X64
  //! ---------
  //!
  //! Architecture subtype describe the highest instruction-set level that can
  //! be used.
  //!
  //! A32 & A64
  //! ---------
  //!
  //! Architecture mode means the instruction encoding to be used when generating
  //! machine code, thus mode can be used to force generation of THUMB and THUMBv2
  //! encoding or regular ARM encoding.
  inline uint32_t archSubId() const noexcept { return _subId; }

  //! Tests whether this architecture is 32-bit.
  inline bool is32Bit() const noexcept { return _gpSize == 4; }
  //! Tests whether this architecture is 64-bit.
  inline bool is64Bit() const noexcept { return _gpSize == 8; }

  //! Tests whether this architecture is X86, X64.
  inline bool isX86Family() const noexcept { return isX86Family(_id); }
  //! Tests whether this architecture is ARM32 or ARM64.
  inline bool isArmFamily() const noexcept { return isArmFamily(_id); }

  //! Returns the native size of a general-purpose register.
  inline uint32_t gpSize() const noexcept { return _gpSize; }
  //! Returns number of general-purpose registers.
  inline uint32_t gpCount() const noexcept { return _gpCount; }

  //! \}

  //! \name Static Functions
  //! \{

  static inline bool isX86Family(uint32_t archId) noexcept { return archId >= kIdX86 && archId <= kIdX64; }
  static inline bool isArmFamily(uint32_t archId) noexcept { return archId >= kIdA32 && archId <= kIdA64; }

  //! \}
};

// ============================================================================
// [asmjit::ArchRegs]
// ============================================================================

//! Information about all architecture registers.
struct ArchRegs {
  //! Register information and signatures indexed by `BaseReg::RegType`.
  RegInfo regInfo[BaseReg::kTypeMax + 1];
  //! Count (maximum) of registers per `BaseReg::RegType`.
  uint8_t regCount[BaseReg::kTypeMax + 1];
  //! Converts RegType to TypeId, see `Type::Id`.
  uint8_t regTypeToTypeId[BaseReg::kTypeMax + 1];
};

// ============================================================================
// [asmjit::ArchUtils]
// ============================================================================

struct ArchUtils {
  ASMJIT_API static Error typeIdToRegInfo(uint32_t archId, uint32_t& typeIdInOut, RegInfo& regInfo) noexcept;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ARCH_H_INCLUDED
