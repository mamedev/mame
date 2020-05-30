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

#ifndef ASMJIT_X86_X86COMPILER_H_INCLUDED
#define ASMJIT_X86_X86COMPILER_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/datatypes.h"
#include "../core/type.h"
#include "../x86/x86emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::Compiler]
// ============================================================================

//! Architecture-dependent asm-compiler (X86).
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  typedef BaseCompiler Base;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Compiler() noexcept;

  //! \}

  //! \name Virtual Registers
  //! \{

#ifndef ASMJIT_NO_LOGGING
# define ASMJIT_NEW_REG_FMT(OUT, PARAM, FORMAT, ARGS)                         \
    _newRegFmt(OUT, PARAM, FORMAT, ARGS)
#else
# define ASMJIT_NEW_REG_FMT(OUT, PARAM, FORMAT, ARGS)                         \
    DebugUtils::unused(FORMAT);                                               \
    DebugUtils::unused(std::forward<Args>(args)...);                          \
    _newReg(OUT, PARAM)
#endif

#define ASMJIT_NEW_REG_CUSTOM(FUNC, REG)                                      \
    inline REG FUNC(uint32_t typeId) {                                        \
      REG reg(Globals::NoInit);                                               \
      _newReg(reg, typeId);                                                   \
      return reg;                                                             \
    }                                                                         \
                                                                              \
    template<typename... Args>                                                \
    inline REG FUNC(uint32_t typeId, const char* fmt, Args&&... args) {       \
      REG reg(Globals::NoInit);                                               \
      ASMJIT_NEW_REG_FMT(reg, typeId, fmt, std::forward<Args>(args)...);      \
      return reg;                                                             \
    }

#define ASMJIT_NEW_REG_TYPED(FUNC, REG, TYPE_ID)                              \
    inline REG FUNC() {                                                       \
      REG reg(Globals::NoInit);                                               \
      _newReg(reg, TYPE_ID);                                                  \
      return reg;                                                             \
    }                                                                         \
                                                                              \
    template<typename... Args>                                                \
    inline REG FUNC(const char* fmt, Args&&... args) {                        \
      REG reg(Globals::NoInit);                                               \
      ASMJIT_NEW_REG_FMT(reg, TYPE_ID, fmt, std::forward<Args>(args)...);     \
      return reg;                                                             \
    }

  template<typename RegT>
  inline RegT newSimilarReg(const RegT& ref) {
    RegT reg(Globals::NoInit);
    _newReg(reg, ref);
    return reg;
  }

  template<typename RegT, typename... Args>
  inline RegT newSimilarReg(const RegT& ref, const char* fmt, Args&&... args) {
    RegT reg(Globals::NoInit);
    ASMJIT_NEW_REG_FMT(reg, ref, fmt, std::forward<Args>(args)...);
    return reg;
  }

  ASMJIT_NEW_REG_CUSTOM(newReg    , Reg )
  ASMJIT_NEW_REG_CUSTOM(newGp     , Gp  )
  ASMJIT_NEW_REG_CUSTOM(newVec    , Vec )
  ASMJIT_NEW_REG_CUSTOM(newK      , KReg)

  ASMJIT_NEW_REG_TYPED(newI8     , Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_TYPED(newU8     , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_TYPED(newI16    , Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_TYPED(newU16    , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_TYPED(newI32    , Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_TYPED(newU32    , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_TYPED(newI64    , Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_TYPED(newU64    , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_TYPED(newInt8   , Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_TYPED(newUInt8  , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_TYPED(newInt16  , Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_TYPED(newUInt16 , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_TYPED(newInt32  , Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_TYPED(newUInt32 , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_TYPED(newInt64  , Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_TYPED(newUInt64 , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_TYPED(newIntPtr , Gp  , Type::kIdIntPtr )
  ASMJIT_NEW_REG_TYPED(newUIntPtr, Gp  , Type::kIdUIntPtr)

  ASMJIT_NEW_REG_TYPED(newGpb    , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_TYPED(newGpw    , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_TYPED(newGpd    , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_TYPED(newGpq    , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_TYPED(newGpz    , Gp  , Type::kIdUIntPtr)
  ASMJIT_NEW_REG_TYPED(newXmm    , Xmm , Type::kIdI32x4  )
  ASMJIT_NEW_REG_TYPED(newXmmSs  , Xmm , Type::kIdF32x1  )
  ASMJIT_NEW_REG_TYPED(newXmmSd  , Xmm , Type::kIdF64x1  )
  ASMJIT_NEW_REG_TYPED(newXmmPs  , Xmm , Type::kIdF32x4  )
  ASMJIT_NEW_REG_TYPED(newXmmPd  , Xmm , Type::kIdF64x2  )
  ASMJIT_NEW_REG_TYPED(newYmm    , Ymm , Type::kIdI32x8  )
  ASMJIT_NEW_REG_TYPED(newYmmPs  , Ymm , Type::kIdF32x8  )
  ASMJIT_NEW_REG_TYPED(newYmmPd  , Ymm , Type::kIdF64x4  )
  ASMJIT_NEW_REG_TYPED(newZmm    , Zmm , Type::kIdI32x16 )
  ASMJIT_NEW_REG_TYPED(newZmmPs  , Zmm , Type::kIdF32x16 )
  ASMJIT_NEW_REG_TYPED(newZmmPd  , Zmm , Type::kIdF64x8  )
  ASMJIT_NEW_REG_TYPED(newMm     , Mm  , Type::kIdMmx64  )
  ASMJIT_NEW_REG_TYPED(newKb     , KReg, Type::kIdMask8  )
  ASMJIT_NEW_REG_TYPED(newKw     , KReg, Type::kIdMask16 )
  ASMJIT_NEW_REG_TYPED(newKd     , KReg, Type::kIdMask32 )
  ASMJIT_NEW_REG_TYPED(newKq     , KReg, Type::kIdMask64 )

#undef ASMJIT_NEW_REG_TYPED
#undef ASMJIT_NEW_REG_CUSTOM
#undef ASMJIT_NEW_REG_FMT

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new memory chunk allocated on the current function's stack.
  inline Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _newStack(m, size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  inline Mem newConst(uint32_t scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _newConst(m, scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool.
  inline Mem newByteConst(uint32_t scope, uint8_t val) noexcept { return newConst(scope, &val, 1); }
  //! Put a WORD `val` to a constant-pool.
  inline Mem newWordConst(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newDWordConst(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newQWordConst(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  inline Mem newInt16Const(uint32_t scope, int16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  inline Mem newUInt16Const(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newInt32Const(uint32_t scope, int32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newUInt32Const(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newInt64Const(uint32_t scope, int64_t val) noexcept { return newConst(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newUInt64Const(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  inline Mem newFloatConst(uint32_t scope, float val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  inline Mem newDoubleConst(uint32_t scope, double val) noexcept { return newConst(scope, &val, 8); }

  //! Put a MMX `val` to a constant-pool.
  inline Mem newMmConst(uint32_t scope, const Data64& val) noexcept { return newConst(scope, &val, 8); }
  //! Put a XMM `val` to a constant-pool.
  inline Mem newXmmConst(uint32_t scope, const Data128& val) noexcept { return newConst(scope, &val, 16); }
  //! Put a YMM `val` to a constant-pool.
  inline Mem newYmmConst(uint32_t scope, const Data256& val) noexcept { return newConst(scope, &val, 32); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  inline Compiler& unfollow() noexcept { _instOptions |= Inst::kOptionUnfollow; return *this; }
  //! Tell the compiler that the destination variable will be overwritten.
  inline Compiler& overwrite() noexcept { _instOptions |= Inst::kOptionOverwrite; return *this; }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Call a function.
  inline FuncCallNode* call(const Gp& target, const FuncSignature& sign) { return addCall(Inst::kIdCall, target, sign); }
  //! \overload
  inline FuncCallNode* call(const Mem& target, const FuncSignature& sign) { return addCall(Inst::kIdCall, target, sign); }
  //! \overload
  inline FuncCallNode* call(const Label& target, const FuncSignature& sign) { return addCall(Inst::kIdCall, target, sign); }
  //! \overload
  inline FuncCallNode* call(const Imm& target, const FuncSignature& sign) { return addCall(Inst::kIdCall, target, sign); }
  //! \overload
  inline FuncCallNode* call(uint64_t target, const FuncSignature& sign) { return addCall(Inst::kIdCall, Imm(int64_t(target)), sign); }

  //! Return.
  inline FuncRetNode* ret() { return addRet(Operand(), Operand()); }
  //! \overload
  inline FuncRetNode* ret(const BaseReg& o0) { return addRet(o0, Operand()); }
  //! \overload
  inline FuncRetNode* ret(const BaseReg& o0, const BaseReg& o1) { return addRet(o0, o1); }

  //! \}

  //! \name Jump Tables Support
  //! \{

  using EmitterExplicitT<Compiler>::jmp;

  inline Error jmp(const BaseReg& target, JumpAnnotation* annotation) { return emitAnnotatedJump(Inst::kIdJmp, target, annotation); }
  inline Error jmp(const BaseMem& target, JumpAnnotation* annotation) { return emitAnnotatedJump(Inst::kIdJmp, target, annotation); }

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_X86_X86COMPILER_H_INCLUDED
