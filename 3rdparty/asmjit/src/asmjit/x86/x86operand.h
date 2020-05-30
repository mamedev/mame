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

#ifndef ASMJIT_X86_X86OPERAND_H_INCLUDED
#define ASMJIT_X86_X86OPERAND_H_INCLUDED

#include "../core/arch.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [Forward Declarations]
// ============================================================================

class Reg;
class Mem;

class Gp;
class Gpb;
class GpbLo;
class GpbHi;
class Gpw;
class Gpd;
class Gpq;
class Vec;
class Xmm;
class Ymm;
class Zmm;
class Mm;
class KReg;
class SReg;
class CReg;
class DReg;
class St;
class Bnd;
class Rip;

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::RegTraits]
// ============================================================================

//! Register traits (X86).
//!
//! Register traits contains information about a particular register type. It's
//! used by asmjit to setup register information on-the-fly and to populate
//! tables that contain register information (this way it's possible to change
//! register types and groups without having to reorder these tables).
template<uint32_t REG_TYPE>
struct RegTraits : public BaseRegTraits {};

//! \cond
// <--------------------+-----+-------------------------+------------------------+---+---+----------------+
//                      | Reg |        Reg-Type         |        Reg-Group       |Sz |Cnt|     TypeId     |
// <--------------------+-----+-------------------------+------------------------+---+---+----------------+
ASMJIT_DEFINE_REG_TRAITS(GpbLo, BaseReg::kTypeGp8Lo     , BaseReg::kGroupGp      , 1 , 16, Type::kIdI8    );
ASMJIT_DEFINE_REG_TRAITS(GpbHi, BaseReg::kTypeGp8Hi     , BaseReg::kGroupGp      , 1 , 4 , Type::kIdI8    );
ASMJIT_DEFINE_REG_TRAITS(Gpw  , BaseReg::kTypeGp16      , BaseReg::kGroupGp      , 2 , 16, Type::kIdI16   );
ASMJIT_DEFINE_REG_TRAITS(Gpd  , BaseReg::kTypeGp32      , BaseReg::kGroupGp      , 4 , 16, Type::kIdI32   );
ASMJIT_DEFINE_REG_TRAITS(Gpq  , BaseReg::kTypeGp64      , BaseReg::kGroupGp      , 8 , 16, Type::kIdI64   );
ASMJIT_DEFINE_REG_TRAITS(Xmm  , BaseReg::kTypeVec128    , BaseReg::kGroupVec     , 16, 32, Type::kIdI32x4 );
ASMJIT_DEFINE_REG_TRAITS(Ymm  , BaseReg::kTypeVec256    , BaseReg::kGroupVec     , 32, 32, Type::kIdI32x8 );
ASMJIT_DEFINE_REG_TRAITS(Zmm  , BaseReg::kTypeVec512    , BaseReg::kGroupVec     , 64, 32, Type::kIdI32x16);
ASMJIT_DEFINE_REG_TRAITS(Mm   , BaseReg::kTypeOther0    , BaseReg::kGroupOther0  , 8 , 8 , Type::kIdMmx64 );
ASMJIT_DEFINE_REG_TRAITS(KReg , BaseReg::kTypeOther1    , BaseReg::kGroupOther1  , 0 , 8 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(SReg , BaseReg::kTypeCustom + 0, BaseReg::kGroupVirt + 0, 2 , 7 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(CReg , BaseReg::kTypeCustom + 1, BaseReg::kGroupVirt + 1, 0 , 16, Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(DReg , BaseReg::kTypeCustom + 2, BaseReg::kGroupVirt + 2, 0 , 16, Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(St   , BaseReg::kTypeCustom + 3, BaseReg::kGroupVirt + 3, 10, 8 , Type::kIdF80   );
ASMJIT_DEFINE_REG_TRAITS(Bnd  , BaseReg::kTypeCustom + 4, BaseReg::kGroupVirt + 4, 16, 4 , Type::kIdVoid  );
ASMJIT_DEFINE_REG_TRAITS(Rip  , BaseReg::kTypeIP        , BaseReg::kGroupVirt + 5, 0 , 1 , Type::kIdVoid  );
//! \endcond

// ============================================================================
// [asmjit::x86::Reg]
// ============================================================================

//! Register (X86).
class Reg : public BaseReg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Reg, BaseReg)

  //! Register type.
  enum RegType : uint32_t {
    kTypeNone  = BaseReg::kTypeNone,     //!< No register type or invalid register.
    kTypeGpbLo = BaseReg::kTypeGp8Lo,    //!< Low GPB register (AL, BL, CL, DL, ...).
    kTypeGpbHi = BaseReg::kTypeGp8Hi,    //!< High GPB register (AH, BH, CH, DH only).
    kTypeGpw   = BaseReg::kTypeGp16,     //!< GPW register.
    kTypeGpd   = BaseReg::kTypeGp32,     //!< GPD register.
    kTypeGpq   = BaseReg::kTypeGp64,     //!< GPQ register (64-bit).
    kTypeXmm   = BaseReg::kTypeVec128,   //!< XMM register (SSE+).
    kTypeYmm   = BaseReg::kTypeVec256,   //!< YMM register (AVX+).
    kTypeZmm   = BaseReg::kTypeVec512,   //!< ZMM register (AVX512+).
    kTypeMm    = BaseReg::kTypeOther0,   //!< MMX register.
    kTypeKReg  = BaseReg::kTypeOther1,   //!< K register (AVX512+).
    kTypeSReg  = BaseReg::kTypeCustom+0, //!< Segment register (None, ES, CS, SS, DS, FS, GS).
    kTypeCReg  = BaseReg::kTypeCustom+1, //!< Control register (CR).
    kTypeDReg  = BaseReg::kTypeCustom+2, //!< Debug register (DR).
    kTypeSt    = BaseReg::kTypeCustom+3, //!< FPU (x87) register.
    kTypeBnd   = BaseReg::kTypeCustom+4, //!< Bound register (BND).
    kTypeRip   = BaseReg::kTypeIP,       //!< Instruction pointer (EIP, RIP).
    kTypeCount = BaseReg::kTypeCustom+5  //!< Count of register types.
  };

  //! Register group.
  enum RegGroup : uint32_t {
    kGroupGp   = BaseReg::kGroupGp,      //!< GP register group or none (universal).
    kGroupVec  = BaseReg::kGroupVec,     //!< XMM|YMM|ZMM register group (universal).
    kGroupMm   = BaseReg::kGroupOther0,  //!< MMX register group (legacy).
    kGroupKReg = BaseReg::kGroupOther1,  //!< K register group.

    // These are not managed by BaseCompiler nor used by Func-API:
    kGroupSReg = BaseReg::kGroupVirt+0,  //!< Segment register group.
    kGroupCReg = BaseReg::kGroupVirt+1,  //!< Control register group.
    kGroupDReg = BaseReg::kGroupVirt+2,  //!< Debug register group.
    kGroupSt   = BaseReg::kGroupVirt+3,  //!< FPU register group.
    kGroupBnd  = BaseReg::kGroupVirt+4,  //!< Bound register group.
    kGroupRip  = BaseReg::kGroupVirt+5,  //!< Instrucion pointer (IP).
    kGroupCount                          //!< Count of all register groups.
  };

  //! Tests whether the register is a GPB register (8-bit).
  constexpr bool isGpb() const noexcept { return size() == 1; }
  //! Tests whether the register is a low GPB register (8-bit).
  constexpr bool isGpbLo() const noexcept { return hasSignature(RegTraits<kTypeGpbLo>::kSignature); }
  //! Tests whether the register is a high GPB register (8-bit).
  constexpr bool isGpbHi() const noexcept { return hasSignature(RegTraits<kTypeGpbHi>::kSignature); }
  //! Tests whether the register is a GPW register (16-bit).
  constexpr bool isGpw() const noexcept { return hasSignature(RegTraits<kTypeGpw>::kSignature); }
  //! Tests whether the register is a GPD register (32-bit).
  constexpr bool isGpd() const noexcept { return hasSignature(RegTraits<kTypeGpd>::kSignature); }
  //! Tests whether the register is a GPQ register (64-bit).
  constexpr bool isGpq() const noexcept { return hasSignature(RegTraits<kTypeGpq>::kSignature); }
  //! Tests whether the register is an XMM register (128-bit).
  constexpr bool isXmm() const noexcept { return hasSignature(RegTraits<kTypeXmm>::kSignature); }
  //! Tests whether the register is a YMM register (256-bit).
  constexpr bool isYmm() const noexcept { return hasSignature(RegTraits<kTypeYmm>::kSignature); }
  //! Tests whether the register is a ZMM register (512-bit).
  constexpr bool isZmm() const noexcept { return hasSignature(RegTraits<kTypeZmm>::kSignature); }
  //! Tests whether the register is an MMX register (64-bit).
  constexpr bool isMm() const noexcept { return hasSignature(RegTraits<kTypeMm>::kSignature); }
  //! Tests whether the register is a K register (64-bit).
  constexpr bool isKReg() const noexcept { return hasSignature(RegTraits<kTypeKReg>::kSignature); }
  //! Tests whether the register is a segment register.
  constexpr bool isSReg() const noexcept { return hasSignature(RegTraits<kTypeSReg>::kSignature); }
  //! Tests whether the register is a control register.
  constexpr bool isCReg() const noexcept { return hasSignature(RegTraits<kTypeCReg>::kSignature); }
  //! Tests whether the register is a debug register.
  constexpr bool isDReg() const noexcept { return hasSignature(RegTraits<kTypeDReg>::kSignature); }
  //! Tests whether the register is an FPU register (80-bit).
  constexpr bool isSt() const noexcept { return hasSignature(RegTraits<kTypeSt>::kSignature); }
  //! Tests whether the register is a bound register.
  constexpr bool isBnd() const noexcept { return hasSignature(RegTraits<kTypeBnd>::kSignature); }
  //! Tests whether the register is RIP.
  constexpr bool isRip() const noexcept { return hasSignature(RegTraits<kTypeRip>::kSignature); }

  template<uint32_t REG_TYPE>
  inline void setRegT(uint32_t rId) noexcept {
    setSignature(RegTraits<REG_TYPE>::kSignature);
    setId(rId);
  }

  inline void setTypeAndId(uint32_t rType, uint32_t rId) noexcept {
    ASMJIT_ASSERT(rType < kTypeCount);
    setSignature(signatureOf(rType));
    setId(rId);
  }

  static inline uint32_t groupOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t groupOfT() noexcept { return RegTraits<REG_TYPE>::kGroup; }

  static inline uint32_t typeIdOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t typeIdOfT() noexcept { return RegTraits<REG_TYPE>::kTypeId; }

  static inline uint32_t signatureOf(uint32_t rType) noexcept;
  template<uint32_t REG_TYPE>
  static inline uint32_t signatureOfT() noexcept { return RegTraits<REG_TYPE>::kSignature; }

  static inline uint32_t signatureOfVecByType(uint32_t typeId) noexcept {
    return typeId <= Type::_kIdVec128End ? RegTraits<kTypeXmm>::kSignature :
           typeId <= Type::_kIdVec256End ? RegTraits<kTypeYmm>::kSignature : RegTraits<kTypeZmm>::kSignature;
  }

  static inline uint32_t signatureOfVecBySize(uint32_t size) noexcept {
    return size <= 16 ? RegTraits<kTypeXmm>::kSignature :
           size <= 32 ? RegTraits<kTypeYmm>::kSignature : RegTraits<kTypeZmm>::kSignature;
  }

  //! Tests whether the `op` operand is either a low or high 8-bit GPB register.
  static inline bool isGpb(const Operand_& op) noexcept {
    // Check operand type, register group, and size. Not interested in register type.
    const uint32_t kSgn = (Operand::kOpReg << kSignatureOpShift  ) |
                          (1               << kSignatureSizeShift) ;
    return (op.signature() & (kSignatureOpMask | kSignatureSizeMask)) == kSgn;
  }

  static inline bool isGpbLo(const Operand_& op) noexcept { return op.as<Reg>().isGpbLo(); }
  static inline bool isGpbHi(const Operand_& op) noexcept { return op.as<Reg>().isGpbHi(); }
  static inline bool isGpw(const Operand_& op) noexcept { return op.as<Reg>().isGpw(); }
  static inline bool isGpd(const Operand_& op) noexcept { return op.as<Reg>().isGpd(); }
  static inline bool isGpq(const Operand_& op) noexcept { return op.as<Reg>().isGpq(); }
  static inline bool isXmm(const Operand_& op) noexcept { return op.as<Reg>().isXmm(); }
  static inline bool isYmm(const Operand_& op) noexcept { return op.as<Reg>().isYmm(); }
  static inline bool isZmm(const Operand_& op) noexcept { return op.as<Reg>().isZmm(); }
  static inline bool isMm(const Operand_& op) noexcept { return op.as<Reg>().isMm(); }
  static inline bool isKReg(const Operand_& op) noexcept { return op.as<Reg>().isKReg(); }
  static inline bool isSReg(const Operand_& op) noexcept { return op.as<Reg>().isSReg(); }
  static inline bool isCReg(const Operand_& op) noexcept { return op.as<Reg>().isCReg(); }
  static inline bool isDReg(const Operand_& op) noexcept { return op.as<Reg>().isDReg(); }
  static inline bool isSt(const Operand_& op) noexcept { return op.as<Reg>().isSt(); }
  static inline bool isBnd(const Operand_& op) noexcept { return op.as<Reg>().isBnd(); }
  static inline bool isRip(const Operand_& op) noexcept { return op.as<Reg>().isRip(); }

  static inline bool isGpb(const Operand_& op, uint32_t rId) noexcept { return isGpb(op) & (op.id() == rId); }
  static inline bool isGpbLo(const Operand_& op, uint32_t rId) noexcept { return isGpbLo(op) & (op.id() == rId); }
  static inline bool isGpbHi(const Operand_& op, uint32_t rId) noexcept { return isGpbHi(op) & (op.id() == rId); }
  static inline bool isGpw(const Operand_& op, uint32_t rId) noexcept { return isGpw(op) & (op.id() == rId); }
  static inline bool isGpd(const Operand_& op, uint32_t rId) noexcept { return isGpd(op) & (op.id() == rId); }
  static inline bool isGpq(const Operand_& op, uint32_t rId) noexcept { return isGpq(op) & (op.id() == rId); }
  static inline bool isXmm(const Operand_& op, uint32_t rId) noexcept { return isXmm(op) & (op.id() == rId); }
  static inline bool isYmm(const Operand_& op, uint32_t rId) noexcept { return isYmm(op) & (op.id() == rId); }
  static inline bool isZmm(const Operand_& op, uint32_t rId) noexcept { return isZmm(op) & (op.id() == rId); }
  static inline bool isMm(const Operand_& op, uint32_t rId) noexcept { return isMm(op) & (op.id() == rId); }
  static inline bool isKReg(const Operand_& op, uint32_t rId) noexcept { return isKReg(op) & (op.id() == rId); }
  static inline bool isSReg(const Operand_& op, uint32_t rId) noexcept { return isSReg(op) & (op.id() == rId); }
  static inline bool isCReg(const Operand_& op, uint32_t rId) noexcept { return isCReg(op) & (op.id() == rId); }
  static inline bool isDReg(const Operand_& op, uint32_t rId) noexcept { return isDReg(op) & (op.id() == rId); }
  static inline bool isSt(const Operand_& op, uint32_t rId) noexcept { return isSt(op) & (op.id() == rId); }
  static inline bool isBnd(const Operand_& op, uint32_t rId) noexcept { return isBnd(op) & (op.id() == rId); }
  static inline bool isRip(const Operand_& op, uint32_t rId) noexcept { return isRip(op) & (op.id() == rId); }
};

//! General purpose register (X86).
class Gp : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, Reg)

  //! Physical id (X86).
  //!
  //! \note Register indexes have been reduced to only support general purpose
  //! registers. There is no need to have enumerations with number suffix that
  //! expands to the exactly same value as the suffix value itself.
  enum Id : uint32_t {
    kIdAx  = 0,  //!< Physical id of AL|AH|AX|EAX|RAX registers.
    kIdCx  = 1,  //!< Physical id of CL|CH|CX|ECX|RCX registers.
    kIdDx  = 2,  //!< Physical id of DL|DH|DX|EDX|RDX registers.
    kIdBx  = 3,  //!< Physical id of BL|BH|BX|EBX|RBX registers.
    kIdSp  = 4,  //!< Physical id of SPL|SP|ESP|RSP registers.
    kIdBp  = 5,  //!< Physical id of BPL|BP|EBP|RBP registers.
    kIdSi  = 6,  //!< Physical id of SIL|SI|ESI|RSI registers.
    kIdDi  = 7,  //!< Physical id of DIL|DI|EDI|RDI registers.
    kIdR8  = 8,  //!< Physical id of R8B|R8W|R8D|R8 registers (64-bit only).
    kIdR9  = 9,  //!< Physical id of R9B|R9W|R9D|R9 registers (64-bit only).
    kIdR10 = 10, //!< Physical id of R10B|R10W|R10D|R10 registers (64-bit only).
    kIdR11 = 11, //!< Physical id of R11B|R11W|R11D|R11 registers (64-bit only).
    kIdR12 = 12, //!< Physical id of R12B|R12W|R12D|R12 registers (64-bit only).
    kIdR13 = 13, //!< Physical id of R13B|R13W|R13D|R13 registers (64-bit only).
    kIdR14 = 14, //!< Physical id of R14B|R14W|R14D|R14 registers (64-bit only).
    kIdR15 = 15  //!< Physical id of R15B|R15W|R15D|R15 registers (64-bit only).
  };

  //! Casts this register to 8-bit (LO) part.
  inline GpbLo r8() const noexcept;
  //! Casts this register to 8-bit (LO) part.
  inline GpbLo r8Lo() const noexcept;
  //! Casts this register to 8-bit (HI) part.
  inline GpbHi r8Hi() const noexcept;
  //! Casts this register to 16-bit.
  inline Gpw r16() const noexcept;
  //! Casts this register to 32-bit.
  inline Gpd r32() const noexcept;
  //! Casts this register to 64-bit.
  inline Gpq r64() const noexcept;
};

//! Vector register (XMM|YMM|ZMM) (X86).
class Vec : public Reg {
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, Reg)

  //! Casts this register to XMM (clone).
  inline Xmm xmm() const noexcept;
  //! Casts this register to YMM.
  inline Ymm ymm() const noexcept;
  //! Casts this register to ZMM.
  inline Zmm zmm() const noexcept;

  //! Casts this register to a register that has half the size (or XMM if it's already XMM).
  inline Vec half() const noexcept {
    return Vec(type() == kTypeZmm ? signatureOf(kTypeYmm) : signatureOf(kTypeXmm), id());
  }
};

//! Segment register (X86).
class SReg : public Reg {
  ASMJIT_DEFINE_FINAL_REG(SReg, Reg, RegTraits<kTypeSReg>)

  //! X86 segment id.
  enum Id : uint32_t {
    kIdNone = 0, //!< No segment (default).
    kIdEs   = 1, //!< ES segment.
    kIdCs   = 2, //!< CS segment.
    kIdSs   = 3, //!< SS segment.
    kIdDs   = 4, //!< DS segment.
    kIdFs   = 5, //!< FS segment.
    kIdGs   = 6, //!< GS segment.

    //! Count of  segment registers supported by AsmJit.
    //!
    //! \note X86 architecture has 6 segment registers - ES, CS, SS, DS, FS, GS.
    //! X64 architecture lowers them down to just FS and GS. AsmJit supports 7
    //! segment registers - all addressable in both  and X64 modes and one
    //! extra called `SReg::kIdNone`, which is AsmJit specific and means that
    //! there is no segment register specified.
    kIdCount = 7
  };
};

//! GPB low or high register (X86).
class Gpb : public Gp { ASMJIT_DEFINE_ABSTRACT_REG(Gpb, Gp) };
//! GPB low register (X86).
class GpbLo : public Gpb { ASMJIT_DEFINE_FINAL_REG(GpbLo, Gpb, RegTraits<kTypeGpbLo>) };
//! GPB high register (X86).
class GpbHi : public Gpb { ASMJIT_DEFINE_FINAL_REG(GpbHi, Gpb, RegTraits<kTypeGpbHi>) };
//! GPW register (X86).
class Gpw : public Gp { ASMJIT_DEFINE_FINAL_REG(Gpw, Gp, RegTraits<kTypeGpw>) };
//! GPD register (X86).
class Gpd : public Gp { ASMJIT_DEFINE_FINAL_REG(Gpd, Gp, RegTraits<kTypeGpd>) };
//! GPQ register (X86_64).
class Gpq : public Gp { ASMJIT_DEFINE_FINAL_REG(Gpq, Gp, RegTraits<kTypeGpq>) };

//! 128-bit XMM register (SSE+).
class Xmm : public Vec {
  ASMJIT_DEFINE_FINAL_REG(Xmm, Vec, RegTraits<kTypeXmm>)
  //! Casts this register to a register that has half the size (XMM).
  inline Xmm half() const noexcept { return Xmm(id()); }
};

//! 256-bit YMM register (AVX+).
class Ymm : public Vec {
  ASMJIT_DEFINE_FINAL_REG(Ymm, Vec, RegTraits<kTypeYmm>)
  //! Casts this register to a register that has half the size (XMM).
  inline Xmm half() const noexcept { return Xmm(id()); }
};

//! 512-bit ZMM register (AVX512+).
class Zmm : public Vec {
  ASMJIT_DEFINE_FINAL_REG(Zmm, Vec, RegTraits<kTypeZmm>)
  //! Casts this register to a register that has half the size (YMM).
  inline Ymm half() const noexcept { return Ymm(id()); }
};

//! 64-bit MMX register (MMX+).
class Mm : public Reg { ASMJIT_DEFINE_FINAL_REG(Mm, Reg, RegTraits<kTypeMm>) };
//! 64-bit K register (AVX512+).
class KReg : public Reg { ASMJIT_DEFINE_FINAL_REG(KReg, Reg, RegTraits<kTypeKReg>) };
//! 32-bit or 64-bit control register (X86).
class CReg : public Reg { ASMJIT_DEFINE_FINAL_REG(CReg, Reg, RegTraits<kTypeCReg>) };
//! 32-bit or 64-bit debug register (X86).
class DReg : public Reg { ASMJIT_DEFINE_FINAL_REG(DReg, Reg, RegTraits<kTypeDReg>) };
//! 80-bit FPU register (X86).
class St : public Reg { ASMJIT_DEFINE_FINAL_REG(St, Reg, RegTraits<kTypeSt>) };
//! 128-bit BND register (BND+).
class Bnd : public Reg { ASMJIT_DEFINE_FINAL_REG(Bnd, Reg, RegTraits<kTypeBnd>) };
//! RIP register (X86).
class Rip : public Reg { ASMJIT_DEFINE_FINAL_REG(Rip, Reg, RegTraits<kTypeRip>) };

//! \cond
inline GpbLo Gp::r8() const noexcept { return GpbLo(id()); }
inline GpbLo Gp::r8Lo() const noexcept { return GpbLo(id()); }
inline GpbHi Gp::r8Hi() const noexcept { return GpbHi(id()); }
inline Gpw Gp::r16() const noexcept { return Gpw(id()); }
inline Gpd Gp::r32() const noexcept { return Gpd(id()); }
inline Gpq Gp::r64() const noexcept { return Gpq(id()); }
inline Xmm Vec::xmm() const noexcept { return Xmm(id()); }
inline Ymm Vec::ymm() const noexcept { return Ymm(id()); }
inline Zmm Vec::zmm() const noexcept { return Zmm(id()); }
//! \endcond

// ============================================================================
// [asmjit::x86::Mem]
// ============================================================================

//! Memory operand.
class Mem : public BaseMem {
public:
  //! Additional bits of operand's signature used by `Mem`.
  enum AdditionalBits : uint32_t {
    kSignatureMemSegmentShift   = 16,
    kSignatureMemSegmentMask    = 0x07u << kSignatureMemSegmentShift,

    kSignatureMemShiftShift     = 19,
    kSignatureMemShiftMask      = 0x03u << kSignatureMemShiftShift,

    kSignatureMemBroadcastShift = 21,
    kSignatureMemBroadcastMask  = 0x7u << kSignatureMemBroadcastShift
  };

  enum Broadcast : uint32_t {
    kBroadcast1To1 = 0,
    kBroadcast1To2 = 1,
    kBroadcast1To4 = 2,
    kBroadcast1To8 = 3,
    kBroadcast1To16 = 4,
    kBroadcast1To32 = 5,
    kBroadcast1To64 = 6
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Creates a default `Mem` operand that points to [0].
  constexpr Mem() noexcept
    : BaseMem() {}

  constexpr Mem(const Mem& other) noexcept
    : BaseMem(other) {}

  //! \cond INTERNAL
  //!
  //! A constructor used internally to create `Mem` operand from `Decomposed` data.
  constexpr explicit Mem(const Decomposed& d) noexcept
    : BaseMem(d) {}
  //! \endcond

  constexpr Mem(const Label& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { Label::kLabelTag, base.id(), 0, 0, off, size, flags }) {}

  constexpr Mem(const Label& base, const BaseReg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { Label::kLabelTag, base.id(), index.type(), index.id(), off, size, flags | (shift << kSignatureMemShiftShift) }) {}

  constexpr Mem(const BaseReg& base, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { base.type(), base.id(), 0, 0, off, size, flags }) {}

  constexpr Mem(const BaseReg& base, const BaseReg& index, uint32_t shift, int32_t off, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { base.type(), base.id(), index.type(), index.id(), off, size, flags | (shift << kSignatureMemShiftShift) }) {}

  constexpr explicit Mem(uint64_t base, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { 0, uint32_t(base >> 32), 0, 0, int32_t(uint32_t(base & 0xFFFFFFFFu)), size, flags }) {}

  constexpr Mem(uint64_t base, const BaseReg& index, uint32_t shift = 0, uint32_t size = 0, uint32_t flags = 0) noexcept
    : BaseMem(Decomposed { 0, uint32_t(base >> 32), index.type(), index.id(), int32_t(uint32_t(base & 0xFFFFFFFFu)), size, flags | (shift << kSignatureMemShiftShift) }) {}

  constexpr Mem(Globals::Init_, uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) noexcept
    : BaseMem(Globals::Init, u0, u1, u2, u3) {}

  inline explicit Mem(Globals::NoInit_) noexcept
    : BaseMem(Globals::NoInit) {}

  //! Clones the memory operand.
  constexpr Mem clone() const noexcept { return Mem(*this); }

  //! Creates a new copy of this memory operand adjusted by `off`.
  inline Mem cloneAdjusted(int64_t off) const noexcept {
    Mem result(*this);
    result.addOffset(off);
    return result;
  }

  //! Converts memory `baseType` and `baseId` to `x86::Reg` instance.
  //!
  //! The memory must have a valid base register otherwise the result will be wrong.
  inline Reg baseReg() const noexcept { return Reg::fromTypeAndId(baseType(), baseId()); }

  //! Converts memory `indexType` and `indexId` to `x86::Reg` instance.
  //!
  //! The memory must have a valid index register otherwise the result will be wrong.
  inline Reg indexReg() const noexcept { return Reg::fromTypeAndId(indexType(), indexId()); }

  constexpr Mem _1to1() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To1 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to2() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To2 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to4() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To4 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to8() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To8 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to16() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To16 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to32() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To32 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }
  constexpr Mem _1to64() const noexcept { return Mem(Globals::Init, (_signature & ~kSignatureMemBroadcastMask) | (kBroadcast1To64 << kSignatureMemBroadcastShift), _baseId, _data[0], _data[1]); }

  // --------------------------------------------------------------------------
  // [Mem]
  // --------------------------------------------------------------------------

  using BaseMem::setIndex;

  inline void setIndex(const BaseReg& index, uint32_t shift) noexcept {
    setIndex(index);
    setShift(shift);
  }

  //! Tests whether the memory operand has a segment override.
  constexpr bool hasSegment() const noexcept { return _hasSignaturePart<kSignatureMemSegmentMask>(); }
  //! Returns the associated segment override as `SReg` operand.
  constexpr SReg segment() const noexcept { return SReg(segmentId()); }
  //! Returns segment override register id, see `SReg::Id`.
  constexpr uint32_t segmentId() const noexcept { return _getSignaturePart<kSignatureMemSegmentMask>(); }

  //! Sets the segment override to `seg`.
  inline void setSegment(const SReg& seg) noexcept { setSegment(seg.id()); }
  //! Sets the segment override to `id`.
  inline void setSegment(uint32_t rId) noexcept { _setSignaturePart<kSignatureMemSegmentMask>(rId); }
  //! Resets the segment override.
  inline void resetSegment() noexcept { _setSignaturePart<kSignatureMemSegmentMask>(0); }

  //! Tests whether the memory operand has shift (aka scale) value.
  constexpr bool hasShift() const noexcept { return _hasSignaturePart<kSignatureMemShiftMask>(); }
  //! Returns the memory operand's shift (aka scale) value.
  constexpr uint32_t shift() const noexcept { return _getSignaturePart<kSignatureMemShiftMask>(); }
  //! Sets the memory operand's shift (aka scale) value.
  inline void setShift(uint32_t shift) noexcept { _setSignaturePart<kSignatureMemShiftMask>(shift); }
  //! Resets the memory operand's shift (aka scale) value to zero.
  inline void resetShift() noexcept { _setSignaturePart<kSignatureMemShiftMask>(0); }

  //! Tests whether the memory operand has broadcast {1tox}.
  constexpr bool hasBroadcast() const noexcept { return _hasSignaturePart<kSignatureMemBroadcastMask>(); }
  //! Returns the memory operand's broadcast.
  constexpr uint32_t getBroadcast() const noexcept { return _getSignaturePart<kSignatureMemBroadcastMask>(); }
  //! Sets the memory operand's broadcast.
  inline void setBroadcast(uint32_t bcst) noexcept { _setSignaturePart<kSignatureMemBroadcastMask>(bcst); }
  //! Resets the memory operand's broadcast to none.
  inline void resetBroadcast() noexcept { _setSignaturePart<kSignatureMemBroadcastMask>(0); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline Mem& operator=(const Mem& other) noexcept = default;
};

// ============================================================================
// [asmjit::x86::OpData]
// ============================================================================

struct OpData {
  //! Information about all architecture registers.
  ArchRegs archRegs;
};
ASMJIT_VARAPI const OpData opData;

//! \cond
// ... Reg methods that require `opData`.
inline uint32_t Reg::groupOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= BaseReg::kTypeMax);
  return opData.archRegs.regInfo[rType].group();
}

inline uint32_t Reg::typeIdOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= BaseReg::kTypeMax);
  return opData.archRegs.regTypeToTypeId[rType];
}

inline uint32_t Reg::signatureOf(uint32_t rType) noexcept {
  ASMJIT_ASSERT(rType <= BaseReg::kTypeMax);
  return opData.archRegs.regInfo[rType].signature();
}
//! \endcond

// ============================================================================
// [asmjit::x86::regs]
// ============================================================================

namespace regs {

//! Creates an 8-bit low GPB register operand.
static constexpr GpbLo gpb(uint32_t rId) noexcept { return GpbLo(rId); }
//! Creates an 8-bit low GPB register operand.
static constexpr GpbLo gpb_lo(uint32_t rId) noexcept { return GpbLo(rId); }
//! Creates an 8-bit high GPB register operand.
static constexpr GpbHi gpb_hi(uint32_t rId) noexcept { return GpbHi(rId); }
//! Creates a 16-bit GPW register operand.
static constexpr Gpw gpw(uint32_t rId) noexcept { return Gpw(rId); }
//! Creates a 32-bit GPD register operand.
static constexpr Gpd gpd(uint32_t rId) noexcept { return Gpd(rId); }
//! Creates a 64-bit GPQ register operand (64-bit).
static constexpr Gpq gpq(uint32_t rId) noexcept { return Gpq(rId); }
//! Creates a 128-bit XMM register operand.
static constexpr Xmm xmm(uint32_t rId) noexcept { return Xmm(rId); }
//! Creates a 256-bit YMM register operand.
static constexpr Ymm ymm(uint32_t rId) noexcept { return Ymm(rId); }
//! Creates a 512-bit ZMM register operand.
static constexpr Zmm zmm(uint32_t rId) noexcept { return Zmm(rId); }
//! Creates a 64-bit Mm register operand.
static constexpr Mm mm(uint32_t rId) noexcept { return Mm(rId); }
//! Creates a 64-bit K register operand.
static constexpr KReg k(uint32_t rId) noexcept { return KReg(rId); }
//! Creates a 32-bit or 64-bit control register operand.
static constexpr CReg cr(uint32_t rId) noexcept { return CReg(rId); }
//! Creates a 32-bit or 64-bit debug register operand.
static constexpr DReg dr(uint32_t rId) noexcept { return DReg(rId); }
//! Creates an 80-bit st register operand.
static constexpr St st(uint32_t rId) noexcept { return St(rId); }
//! Creates a 128-bit bound register operand.
static constexpr Bnd bnd(uint32_t rId) noexcept { return Bnd(rId); }

static constexpr Gp al(GpbLo::kSignature, Gp::kIdAx);
static constexpr Gp bl(GpbLo::kSignature, Gp::kIdBx);
static constexpr Gp cl(GpbLo::kSignature, Gp::kIdCx);
static constexpr Gp dl(GpbLo::kSignature, Gp::kIdDx);
static constexpr Gp spl(GpbLo::kSignature, Gp::kIdSp);
static constexpr Gp bpl(GpbLo::kSignature, Gp::kIdBp);
static constexpr Gp sil(GpbLo::kSignature, Gp::kIdSi);
static constexpr Gp dil(GpbLo::kSignature, Gp::kIdDi);
static constexpr Gp r8b(GpbLo::kSignature, Gp::kIdR8);
static constexpr Gp r9b(GpbLo::kSignature, Gp::kIdR9);
static constexpr Gp r10b(GpbLo::kSignature, Gp::kIdR10);
static constexpr Gp r11b(GpbLo::kSignature, Gp::kIdR11);
static constexpr Gp r12b(GpbLo::kSignature, Gp::kIdR12);
static constexpr Gp r13b(GpbLo::kSignature, Gp::kIdR13);
static constexpr Gp r14b(GpbLo::kSignature, Gp::kIdR14);
static constexpr Gp r15b(GpbLo::kSignature, Gp::kIdR15);

static constexpr Gp ah(GpbHi::kSignature, Gp::kIdAx);
static constexpr Gp bh(GpbHi::kSignature, Gp::kIdBx);
static constexpr Gp ch(GpbHi::kSignature, Gp::kIdCx);
static constexpr Gp dh(GpbHi::kSignature, Gp::kIdDx);

static constexpr Gp ax(Gpw::kSignature, Gp::kIdAx);
static constexpr Gp bx(Gpw::kSignature, Gp::kIdBx);
static constexpr Gp cx(Gpw::kSignature, Gp::kIdCx);
static constexpr Gp dx(Gpw::kSignature, Gp::kIdDx);
static constexpr Gp sp(Gpw::kSignature, Gp::kIdSp);
static constexpr Gp bp(Gpw::kSignature, Gp::kIdBp);
static constexpr Gp si(Gpw::kSignature, Gp::kIdSi);
static constexpr Gp di(Gpw::kSignature, Gp::kIdDi);
static constexpr Gp r8w(Gpw::kSignature, Gp::kIdR8);
static constexpr Gp r9w(Gpw::kSignature, Gp::kIdR9);
static constexpr Gp r10w(Gpw::kSignature, Gp::kIdR10);
static constexpr Gp r11w(Gpw::kSignature, Gp::kIdR11);
static constexpr Gp r12w(Gpw::kSignature, Gp::kIdR12);
static constexpr Gp r13w(Gpw::kSignature, Gp::kIdR13);
static constexpr Gp r14w(Gpw::kSignature, Gp::kIdR14);
static constexpr Gp r15w(Gpw::kSignature, Gp::kIdR15);

static constexpr Gp eax(Gpd::kSignature, Gp::kIdAx);
static constexpr Gp ebx(Gpd::kSignature, Gp::kIdBx);
static constexpr Gp ecx(Gpd::kSignature, Gp::kIdCx);
static constexpr Gp edx(Gpd::kSignature, Gp::kIdDx);
static constexpr Gp esp(Gpd::kSignature, Gp::kIdSp);
static constexpr Gp ebp(Gpd::kSignature, Gp::kIdBp);
static constexpr Gp esi(Gpd::kSignature, Gp::kIdSi);
static constexpr Gp edi(Gpd::kSignature, Gp::kIdDi);
static constexpr Gp r8d(Gpd::kSignature, Gp::kIdR8);
static constexpr Gp r9d(Gpd::kSignature, Gp::kIdR9);
static constexpr Gp r10d(Gpd::kSignature, Gp::kIdR10);
static constexpr Gp r11d(Gpd::kSignature, Gp::kIdR11);
static constexpr Gp r12d(Gpd::kSignature, Gp::kIdR12);
static constexpr Gp r13d(Gpd::kSignature, Gp::kIdR13);
static constexpr Gp r14d(Gpd::kSignature, Gp::kIdR14);
static constexpr Gp r15d(Gpd::kSignature, Gp::kIdR15);

static constexpr Gp rax(Gpq::kSignature, Gp::kIdAx);
static constexpr Gp rbx(Gpq::kSignature, Gp::kIdBx);
static constexpr Gp rcx(Gpq::kSignature, Gp::kIdCx);
static constexpr Gp rdx(Gpq::kSignature, Gp::kIdDx);
static constexpr Gp rsp(Gpq::kSignature, Gp::kIdSp);
static constexpr Gp rbp(Gpq::kSignature, Gp::kIdBp);
static constexpr Gp rsi(Gpq::kSignature, Gp::kIdSi);
static constexpr Gp rdi(Gpq::kSignature, Gp::kIdDi);
static constexpr Gp r8(Gpq::kSignature, Gp::kIdR8);
static constexpr Gp r9(Gpq::kSignature, Gp::kIdR9);
static constexpr Gp r10(Gpq::kSignature, Gp::kIdR10);
static constexpr Gp r11(Gpq::kSignature, Gp::kIdR11);
static constexpr Gp r12(Gpq::kSignature, Gp::kIdR12);
static constexpr Gp r13(Gpq::kSignature, Gp::kIdR13);
static constexpr Gp r14(Gpq::kSignature, Gp::kIdR14);
static constexpr Gp r15(Gpq::kSignature, Gp::kIdR15);

static constexpr Xmm xmm0(0);
static constexpr Xmm xmm1(1);
static constexpr Xmm xmm2(2);
static constexpr Xmm xmm3(3);
static constexpr Xmm xmm4(4);
static constexpr Xmm xmm5(5);
static constexpr Xmm xmm6(6);
static constexpr Xmm xmm7(7);
static constexpr Xmm xmm8(8);
static constexpr Xmm xmm9(9);
static constexpr Xmm xmm10(10);
static constexpr Xmm xmm11(11);
static constexpr Xmm xmm12(12);
static constexpr Xmm xmm13(13);
static constexpr Xmm xmm14(14);
static constexpr Xmm xmm15(15);
static constexpr Xmm xmm16(16);
static constexpr Xmm xmm17(17);
static constexpr Xmm xmm18(18);
static constexpr Xmm xmm19(19);
static constexpr Xmm xmm20(20);
static constexpr Xmm xmm21(21);
static constexpr Xmm xmm22(22);
static constexpr Xmm xmm23(23);
static constexpr Xmm xmm24(24);
static constexpr Xmm xmm25(25);
static constexpr Xmm xmm26(26);
static constexpr Xmm xmm27(27);
static constexpr Xmm xmm28(28);
static constexpr Xmm xmm29(29);
static constexpr Xmm xmm30(30);
static constexpr Xmm xmm31(31);

static constexpr Ymm ymm0(0);
static constexpr Ymm ymm1(1);
static constexpr Ymm ymm2(2);
static constexpr Ymm ymm3(3);
static constexpr Ymm ymm4(4);
static constexpr Ymm ymm5(5);
static constexpr Ymm ymm6(6);
static constexpr Ymm ymm7(7);
static constexpr Ymm ymm8(8);
static constexpr Ymm ymm9(9);
static constexpr Ymm ymm10(10);
static constexpr Ymm ymm11(11);
static constexpr Ymm ymm12(12);
static constexpr Ymm ymm13(13);
static constexpr Ymm ymm14(14);
static constexpr Ymm ymm15(15);
static constexpr Ymm ymm16(16);
static constexpr Ymm ymm17(17);
static constexpr Ymm ymm18(18);
static constexpr Ymm ymm19(19);
static constexpr Ymm ymm20(20);
static constexpr Ymm ymm21(21);
static constexpr Ymm ymm22(22);
static constexpr Ymm ymm23(23);
static constexpr Ymm ymm24(24);
static constexpr Ymm ymm25(25);
static constexpr Ymm ymm26(26);
static constexpr Ymm ymm27(27);
static constexpr Ymm ymm28(28);
static constexpr Ymm ymm29(29);
static constexpr Ymm ymm30(30);
static constexpr Ymm ymm31(31);

static constexpr Zmm zmm0(0);
static constexpr Zmm zmm1(1);
static constexpr Zmm zmm2(2);
static constexpr Zmm zmm3(3);
static constexpr Zmm zmm4(4);
static constexpr Zmm zmm5(5);
static constexpr Zmm zmm6(6);
static constexpr Zmm zmm7(7);
static constexpr Zmm zmm8(8);
static constexpr Zmm zmm9(9);
static constexpr Zmm zmm10(10);
static constexpr Zmm zmm11(11);
static constexpr Zmm zmm12(12);
static constexpr Zmm zmm13(13);
static constexpr Zmm zmm14(14);
static constexpr Zmm zmm15(15);
static constexpr Zmm zmm16(16);
static constexpr Zmm zmm17(17);
static constexpr Zmm zmm18(18);
static constexpr Zmm zmm19(19);
static constexpr Zmm zmm20(20);
static constexpr Zmm zmm21(21);
static constexpr Zmm zmm22(22);
static constexpr Zmm zmm23(23);
static constexpr Zmm zmm24(24);
static constexpr Zmm zmm25(25);
static constexpr Zmm zmm26(26);
static constexpr Zmm zmm27(27);
static constexpr Zmm zmm28(28);
static constexpr Zmm zmm29(29);
static constexpr Zmm zmm30(30);
static constexpr Zmm zmm31(31);

static constexpr Mm mm0(0);
static constexpr Mm mm1(1);
static constexpr Mm mm2(2);
static constexpr Mm mm3(3);
static constexpr Mm mm4(4);
static constexpr Mm mm5(5);
static constexpr Mm mm6(6);
static constexpr Mm mm7(7);

static constexpr KReg k0(0);
static constexpr KReg k1(1);
static constexpr KReg k2(2);
static constexpr KReg k3(3);
static constexpr KReg k4(4);
static constexpr KReg k5(5);
static constexpr KReg k6(6);
static constexpr KReg k7(7);

static constexpr SReg no_seg(SReg::kIdNone);
static constexpr SReg es(SReg::kIdEs);
static constexpr SReg cs(SReg::kIdCs);
static constexpr SReg ss(SReg::kIdSs);
static constexpr SReg ds(SReg::kIdDs);
static constexpr SReg fs(SReg::kIdFs);
static constexpr SReg gs(SReg::kIdGs);

static constexpr CReg cr0(0);
static constexpr CReg cr1(1);
static constexpr CReg cr2(2);
static constexpr CReg cr3(3);
static constexpr CReg cr4(4);
static constexpr CReg cr5(5);
static constexpr CReg cr6(6);
static constexpr CReg cr7(7);
static constexpr CReg cr8(8);
static constexpr CReg cr9(9);
static constexpr CReg cr10(10);
static constexpr CReg cr11(11);
static constexpr CReg cr12(12);
static constexpr CReg cr13(13);
static constexpr CReg cr14(14);
static constexpr CReg cr15(15);

static constexpr DReg dr0(0);
static constexpr DReg dr1(1);
static constexpr DReg dr2(2);
static constexpr DReg dr3(3);
static constexpr DReg dr4(4);
static constexpr DReg dr5(5);
static constexpr DReg dr6(6);
static constexpr DReg dr7(7);
static constexpr DReg dr8(8);
static constexpr DReg dr9(9);
static constexpr DReg dr10(10);
static constexpr DReg dr11(11);
static constexpr DReg dr12(12);
static constexpr DReg dr13(13);
static constexpr DReg dr14(14);
static constexpr DReg dr15(15);

static constexpr St st0(0);
static constexpr St st1(1);
static constexpr St st2(2);
static constexpr St st3(3);
static constexpr St st4(4);
static constexpr St st5(5);
static constexpr St st6(6);
static constexpr St st7(7);

static constexpr Bnd bnd0(0);
static constexpr Bnd bnd1(1);
static constexpr Bnd bnd2(2);
static constexpr Bnd bnd3(3);

static constexpr Rip rip(0);

} // {regs}

// Make `x86::regs` accessible through `x86` namespace as well.
using namespace regs;

// ============================================================================
// [asmjit::x86::ptr]
// ============================================================================

//! Creates `[base.reg + offset]` memory operand.
static constexpr Mem ptr(const Gp& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}
//! Creates `[base.reg + (index << shift) + offset]` memory operand (scalar index).
static constexpr Mem ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
//! Creates `[base.reg + (index << shift) + offset]` memory operand (vector index).
static constexpr Mem ptr(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[base + offset]` memory operand.
static constexpr Mem ptr(const Label& base, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, offset, size);
}
//! Creates `[base + (index << shift) + offset]` memory operand.
static constexpr Mem ptr(const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}
//! Creates `[base + (index << shift) + offset]` memory operand.
static constexpr Mem ptr(const Label& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, offset, size);
}

//! Creates `[rip + offset]` memory operand.
static constexpr Mem ptr(const Rip& rip_, int32_t offset = 0, uint32_t size = 0) noexcept {
  return Mem(rip_, offset, size);
}

//! Creates `[base]` absolute memory operand.
static constexpr Mem ptr(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size);
}
//! Creates `[base + (index.reg << shift)]` absolute memory operand.
static constexpr Mem ptr(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}
//! Creates `[base + (index.reg << shift)]` absolute memory operand.
static constexpr Mem ptr(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size);
}

//! Creates `[base]` absolute memory operand (absolute).
static constexpr Mem ptr_abs(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, BaseMem::kSignatureMemAbs);
}
//! Creates `[base + (index.reg << shift)]` absolute memory operand (absolute).
static constexpr Mem ptr_abs(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, BaseMem::kSignatureMemAbs);
}
//! Creates `[base + (index.reg << shift)]` absolute memory operand (absolute).
static constexpr Mem ptr_abs(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, BaseMem::kSignatureMemAbs);
}

//! Creates `[base]` relative memory operand (relative).
static constexpr Mem ptr_rel(uint64_t base, uint32_t size = 0) noexcept {
  return Mem(base, size, BaseMem::kSignatureMemRel);
}
//! Creates `[base + (index.reg << shift)]` relative memory operand (relative).
static constexpr Mem ptr_rel(uint64_t base, const Reg& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, BaseMem::kSignatureMemRel);
}
//! Creates `[base + (index.reg << shift)]` relative memory operand (relative).
static constexpr Mem ptr_rel(uint64_t base, const Vec& index, uint32_t shift = 0, uint32_t size = 0) noexcept {
  return Mem(base, index, shift, size, BaseMem::kSignatureMemRel);
}

#define ASMJIT_MEM_PTR(FUNC, SIZE)                                                    \
  /*! Creates `[base + offset]` memory operand. */                                    \
  static constexpr Mem FUNC(const Gp& base, int32_t offset = 0) noexcept {            \
    return Mem(base, offset, SIZE);                                                   \
  }                                                                                   \
  /*! Creates `[base + (index << shift) + offset]` memory operand. */                 \
  static constexpr Mem FUNC(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return Mem(base, index, shift, offset, SIZE);                                     \
  }                                                                                   \
  /*! Creates `[base + (vec_index << shift) + offset]` memory operand. */             \
  static constexpr Mem FUNC(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return Mem(base, index, shift, offset, SIZE);                                     \
  }                                                                                   \
  /*! Creates `[base + offset]` memory operand. */                                    \
  static constexpr Mem FUNC(const Label& base, int32_t offset = 0) noexcept {         \
    return Mem(base, offset, SIZE);                                                   \
  }                                                                                   \
  /*! Creates `[base + (index << shift) + offset]` memory operand. */                 \
  static constexpr Mem FUNC(const Label& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) noexcept { \
    return Mem(base, index, shift, offset, SIZE);                                     \
  }                                                                                   \
  /*! Creates `[rip + offset]` memory operand. */                                     \
  static constexpr Mem FUNC(const Rip& rip_, int32_t offset = 0) noexcept {           \
    return Mem(rip_, offset, SIZE);                                                   \
  }                                                                                   \
  /*! Creates `[ptr]` memory operand. */                                              \
  static constexpr Mem FUNC(uint64_t base) noexcept {                                 \
    return Mem(base, SIZE);                                                           \
  }                                                                                   \
  /*! Creates `[base + (index << shift) + offset]` memory operand. */                 \
  static constexpr Mem FUNC(uint64_t base, const Gp& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE);                                             \
  }                                                                                   \
  /*! Creates `[base + (vec_index << shift) + offset]` memory operand. */             \
  static constexpr Mem FUNC(uint64_t base, const Vec& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE);                                             \
  }                                                                                   \
                                                                                      \
  /*! Creates `[base + offset]` memory operand (absolute). */                         \
  static constexpr Mem FUNC##_abs(uint64_t base) noexcept {                           \
    return Mem(base, SIZE, BaseMem::kSignatureMemAbs);                                \
  }                                                                                   \
  /*! Creates `[base + (index << shift) + offset]` memory operand (absolute). */      \
  static constexpr Mem FUNC##_abs(uint64_t base, const Gp& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE, BaseMem::kSignatureMemAbs);                  \
  }                                                                                   \
  /*! Creates `[base + (vec_index << shift) + offset]` memory operand (absolute). */  \
  static constexpr Mem FUNC##_abs(uint64_t base, const Vec& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE, BaseMem::kSignatureMemAbs);                  \
  }                                                                                   \
                                                                                      \
  /*! Creates `[base + offset]` memory operand (relative). */                         \
  static constexpr Mem FUNC##_rel(uint64_t base) noexcept {                           \
    return Mem(base, SIZE, BaseMem::kSignatureMemRel);                                \
  }                                                                                   \
  /*! Creates `[base + (index << shift) + offset]` memory operand (relative). */      \
  static constexpr Mem FUNC##_rel(uint64_t base, const Gp& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE, BaseMem::kSignatureMemRel);                  \
  }                                                                                   \
  /*! Creates `[base + (vec_index << shift) + offset]` memory operand (relative). */  \
  static constexpr Mem FUNC##_rel(uint64_t base, const Vec& index, uint32_t shift = 0) noexcept { \
    return Mem(base, index, shift, SIZE, BaseMem::kSignatureMemRel);                  \
  }

// Definition of memory operand constructors that use platform independent naming.
ASMJIT_MEM_PTR(ptr_8, 1)
ASMJIT_MEM_PTR(ptr_16, 2)
ASMJIT_MEM_PTR(ptr_32, 4)
ASMJIT_MEM_PTR(ptr_48, 6)
ASMJIT_MEM_PTR(ptr_64, 8)
ASMJIT_MEM_PTR(ptr_80, 10)
ASMJIT_MEM_PTR(ptr_128, 16)
ASMJIT_MEM_PTR(ptr_256, 32)
ASMJIT_MEM_PTR(ptr_512, 64)

// Definition of memory operand constructors that use X86-specific convention.
ASMJIT_MEM_PTR(byte_ptr, 1)
ASMJIT_MEM_PTR(word_ptr, 2)
ASMJIT_MEM_PTR(dword_ptr, 4)
ASMJIT_MEM_PTR(qword_ptr, 8)
ASMJIT_MEM_PTR(tword_ptr, 10)
ASMJIT_MEM_PTR(oword_ptr, 16)
ASMJIT_MEM_PTR(dqword_ptr, 16)
ASMJIT_MEM_PTR(qqword_ptr, 32)
ASMJIT_MEM_PTR(xmmword_ptr, 16)
ASMJIT_MEM_PTR(ymmword_ptr, 32)
ASMJIT_MEM_PTR(zmmword_ptr, 64)

#undef ASMJIT_MEM_PTR

//! \}

ASMJIT_END_SUB_NAMESPACE

// ============================================================================
// [asmjit::Type::IdOfT<x86::Reg>]
// ============================================================================

//! \cond INTERNAL

ASMJIT_BEGIN_NAMESPACE
ASMJIT_DEFINE_TYPE_ID(x86::Gpb, kIdI8);
ASMJIT_DEFINE_TYPE_ID(x86::Gpw, kIdI16);
ASMJIT_DEFINE_TYPE_ID(x86::Gpd, kIdI32);
ASMJIT_DEFINE_TYPE_ID(x86::Gpq, kIdI64);
ASMJIT_DEFINE_TYPE_ID(x86::Mm , kIdMmx64);
ASMJIT_DEFINE_TYPE_ID(x86::Xmm, kIdI32x4);
ASMJIT_DEFINE_TYPE_ID(x86::Ymm, kIdI32x8);
ASMJIT_DEFINE_TYPE_ID(x86::Zmm, kIdI32x16);
ASMJIT_END_NAMESPACE

//! \endcond

#endif // ASMJIT_X86_X86OPERAND_H_INCLUDED
