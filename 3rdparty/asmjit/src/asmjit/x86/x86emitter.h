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

#ifndef ASMJIT_X86_X86EMITTER_H_INCLUDED
#define ASMJIT_X86_X86EMITTER_H_INCLUDED

#include "../core/emitter.h"
#include "../core/support.h"
#include "../x86/x86globals.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

#define ASMJIT_INST_0x(NAME, ID) \
  inline Error NAME() { return _emitter()->emit(Inst::kId##ID); }

#define ASMJIT_INST_1x(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return _emitter()->emit(Inst::kId##ID, o0); }

#define ASMJIT_INST_1i(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return _emitter()->emit(Inst::kId##ID, o0); } \
  inline Error NAME(int o0) { return _emitter()->emit(Inst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(unsigned int o0) { return _emitter()->emit(Inst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(int64_t o0) { return _emitter()->emit(Inst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(uint64_t o0) { return _emitter()->emit(Inst::kId##ID, Support::asInt(o0)); }

#define ASMJIT_INST_1c(NAME, ID, CONV, T0) \
  inline Error NAME(uint32_t cc, const T0& o0) { return _emitter()->emit(CONV(cc), o0); } \
  inline Error NAME##a(const T0& o0) { return _emitter()->emit(Inst::kId##ID##a, o0); } \
  inline Error NAME##ae(const T0& o0) { return _emitter()->emit(Inst::kId##ID##ae, o0); } \
  inline Error NAME##b(const T0& o0) { return _emitter()->emit(Inst::kId##ID##b, o0); } \
  inline Error NAME##be(const T0& o0) { return _emitter()->emit(Inst::kId##ID##be, o0); } \
  inline Error NAME##c(const T0& o0) { return _emitter()->emit(Inst::kId##ID##c, o0); } \
  inline Error NAME##e(const T0& o0) { return _emitter()->emit(Inst::kId##ID##e, o0); } \
  inline Error NAME##g(const T0& o0) { return _emitter()->emit(Inst::kId##ID##g, o0); } \
  inline Error NAME##ge(const T0& o0) { return _emitter()->emit(Inst::kId##ID##ge, o0); } \
  inline Error NAME##l(const T0& o0) { return _emitter()->emit(Inst::kId##ID##l, o0); } \
  inline Error NAME##le(const T0& o0) { return _emitter()->emit(Inst::kId##ID##le, o0); } \
  inline Error NAME##na(const T0& o0) { return _emitter()->emit(Inst::kId##ID##na, o0); } \
  inline Error NAME##nae(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nae, o0); } \
  inline Error NAME##nb(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nb, o0); } \
  inline Error NAME##nbe(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nbe, o0); } \
  inline Error NAME##nc(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nc, o0); } \
  inline Error NAME##ne(const T0& o0) { return _emitter()->emit(Inst::kId##ID##ne, o0); } \
  inline Error NAME##ng(const T0& o0) { return _emitter()->emit(Inst::kId##ID##ng, o0); } \
  inline Error NAME##nge(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nge, o0); } \
  inline Error NAME##nl(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nl, o0); } \
  inline Error NAME##nle(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nle, o0); } \
  inline Error NAME##no(const T0& o0) { return _emitter()->emit(Inst::kId##ID##no, o0); } \
  inline Error NAME##np(const T0& o0) { return _emitter()->emit(Inst::kId##ID##np, o0); } \
  inline Error NAME##ns(const T0& o0) { return _emitter()->emit(Inst::kId##ID##ns, o0); } \
  inline Error NAME##nz(const T0& o0) { return _emitter()->emit(Inst::kId##ID##nz, o0); } \
  inline Error NAME##o(const T0& o0) { return _emitter()->emit(Inst::kId##ID##o, o0); } \
  inline Error NAME##p(const T0& o0) { return _emitter()->emit(Inst::kId##ID##p, o0); } \
  inline Error NAME##pe(const T0& o0) { return _emitter()->emit(Inst::kId##ID##pe, o0); } \
  inline Error NAME##po(const T0& o0) { return _emitter()->emit(Inst::kId##ID##po, o0); } \
  inline Error NAME##s(const T0& o0) { return _emitter()->emit(Inst::kId##ID##s, o0); } \
  inline Error NAME##z(const T0& o0) { return _emitter()->emit(Inst::kId##ID##z, o0); }

#define ASMJIT_INST_2x(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID, o0, o1); }

#define ASMJIT_INST_2i(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID, o0, o1); } \
  inline Error NAME(const T0& o0, int o1) { return _emitter()->emit(Inst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, unsigned int o1) { return _emitter()->emit(Inst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, int64_t o1) { return _emitter()->emit(Inst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, uint64_t o1) { return _emitter()->emit(Inst::kId##ID, o0, Support::asInt(o1)); }

#define ASMJIT_INST_2c(NAME, ID, CONV, T0, T1) \
  inline Error NAME(uint32_t cc, const T0& o0, const T1& o1) { return _emitter()->emit(CONV(cc), o0, o1); } \
  inline Error NAME##a(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##a, o0, o1); } \
  inline Error NAME##ae(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##ae, o0, o1); } \
  inline Error NAME##b(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##b, o0, o1); } \
  inline Error NAME##be(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##be, o0, o1); } \
  inline Error NAME##c(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##c, o0, o1); } \
  inline Error NAME##e(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##e, o0, o1); } \
  inline Error NAME##g(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##g, o0, o1); } \
  inline Error NAME##ge(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##ge, o0, o1); } \
  inline Error NAME##l(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##l, o0, o1); } \
  inline Error NAME##le(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##le, o0, o1); } \
  inline Error NAME##na(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##na, o0, o1); } \
  inline Error NAME##nae(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nae, o0, o1); } \
  inline Error NAME##nb(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nb, o0, o1); } \
  inline Error NAME##nbe(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nbe, o0, o1); } \
  inline Error NAME##nc(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nc, o0, o1); } \
  inline Error NAME##ne(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##ne, o0, o1); } \
  inline Error NAME##ng(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##ng, o0, o1); } \
  inline Error NAME##nge(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nge, o0, o1); } \
  inline Error NAME##nl(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nl, o0, o1); } \
  inline Error NAME##nle(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nle, o0, o1); } \
  inline Error NAME##no(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##no, o0, o1); } \
  inline Error NAME##np(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##np, o0, o1); } \
  inline Error NAME##ns(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##ns, o0, o1); } \
  inline Error NAME##nz(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##nz, o0, o1); } \
  inline Error NAME##o(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##o, o0, o1); } \
  inline Error NAME##p(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##p, o0, o1); } \
  inline Error NAME##pe(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##pe, o0, o1); } \
  inline Error NAME##po(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##po, o0, o1); } \
  inline Error NAME##s(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##s, o0, o1); } \
  inline Error NAME##z(const T0& o0, const T1& o1) { return _emitter()->emit(Inst::kId##ID##z, o0, o1); }

#define ASMJIT_INST_3x(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2); }

#define ASMJIT_INST_3i(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2); } \
  inline Error NAME(const T0& o0, const T1& o1, int o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, unsigned int o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, int64_t o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, uint64_t o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, Support::asInt(o2)); }

#define ASMJIT_INST_3ii(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2); } \
  inline Error NAME(const T0& o0, int o1, int o2) { return _emitter()->emit(Inst::kId##ID, o0, Imm(o1), Support::asInt(o2)); }

#define ASMJIT_INST_4x(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3); }

#define ASMJIT_INST_4i(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, int o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, unsigned int o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, int64_t o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, uint64_t o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, Support::asInt(o3)); }

#define ASMJIT_INST_4ii(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3); } \
  inline Error NAME(const T0& o0, const T1& o1, int o2, int o3) { return _emitter()->emit(Inst::kId##ID, o0, o1, Imm(o2), Support::asInt(o3)); }

#define ASMJIT_INST_5x(NAME, ID, T0, T1, T2, T3, T4) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, o4); }

#define ASMJIT_INST_5i(NAME, ID, T0, T1, T2, T3, T4) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, o4); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, int o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, Support::asInt(o4)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, unsigned int o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, Support::asInt(o4)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, int64_t o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, Support::asInt(o4)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, uint64_t o4) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, Support::asInt(o4)); }

#define ASMJIT_INST_6x(NAME, ID, T0, T1, T2, T3, T4, T5) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3, const T4& o4, const T5& o5) { return _emitter()->emit(Inst::kId##ID, o0, o1, o2, o3, o4, o5); }

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::EmitterExplicitT]
// ============================================================================

template<typename This>
struct EmitterExplicitT {
  //! \cond
  // These typedefs are used to describe implicit operands passed explicitly.
  typedef Gp AL;
  typedef Gp AH;
  typedef Gp CL;
  typedef Gp AX;
  typedef Gp DX;

  typedef Gp EAX;
  typedef Gp EBX;
  typedef Gp ECX;
  typedef Gp EDX;

  typedef Gp RAX;
  typedef Gp RBX;
  typedef Gp RCX;
  typedef Gp RDX;

  typedef Gp ZAX;
  typedef Gp ZBX;
  typedef Gp ZCX;
  typedef Gp ZDX;

  typedef Mem DS_ZAX; // ds:[zax]
  typedef Mem DS_ZDI; // ds:[zdi]
  typedef Mem ES_ZDI; // es:[zdi]
  typedef Mem DS_ZSI; // ds:[zsi]

  typedef Xmm XMM0;

  // These two are unfortunately reported by the sanitizer. We know what we do,
  // however, the sanitizer doesn't. I have tried to use reinterpret_cast instead,
  // but that would generate bad code when compiled by MSC.
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF inline This* _emitter() noexcept { return static_cast<This*>(this); }
  ASMJIT_ATTRIBUTE_NO_SANITIZE_UNDEF inline const This* _emitter() const noexcept { return static_cast<const This*>(this); }

  //! \endcond

  //! \name Native Registers
  //! \{

  //! Returns either GPD or GPQ register of the given `id` depending on the emitter's architecture.
  inline Gp gpz(uint32_t id) const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), id); }

  inline Gp zax() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdAx); }
  inline Gp zcx() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdCx); }
  inline Gp zdx() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdDx); }
  inline Gp zbx() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdBx); }
  inline Gp zsp() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdSp); }
  inline Gp zbp() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdBp); }
  inline Gp zsi() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdSi); }
  inline Gp zdi() const noexcept { return Gp(_emitter()->_gpRegInfo.signature(), Gp::kIdDi); }

  //! \}

  //! \name Native Pointers
  //! \{

  //! Creates a target dependent pointer of which base register's id is `baseId`.
  inline Mem ptr_base(uint32_t baseId, int32_t off = 0, uint32_t size = 0) const noexcept {
    return Mem(Mem::Decomposed { _emitter()->_gpRegInfo.type(), baseId, 0, 0, off, size, 0 });
  }

  inline Mem ptr_zax(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdAx, off, size); }
  inline Mem ptr_zcx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdCx, off, size); }
  inline Mem ptr_zdx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdDx, off, size); }
  inline Mem ptr_zbx(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdBx, off, size); }
  inline Mem ptr_zsp(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdSp, off, size); }
  inline Mem ptr_zbp(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdBp, off, size); }
  inline Mem ptr_zsi(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdSi, off, size); }
  inline Mem ptr_zdi(int32_t off = 0, uint32_t size = 0) const noexcept { return ptr_base(Gp::kIdDi, off, size); }

  //! Creates an `intptr_t` memory operand depending on the current architecture.
  inline Mem intptr_ptr(const Gp& base, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Gp& base, const Gp& index, uint32_t shift = 0, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Gp& base, const Vec& index, uint32_t shift = 0, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, const Gp& index, uint32_t shift, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Label& base, const Vec& index, uint32_t shift, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(const Rip& rip, int32_t offset = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(rip, offset, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(uint64_t base) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr(uint64_t base, const Gp& index, uint32_t shift = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, nativeGpSize);
  }
  //! \overload
  inline Mem intptr_ptr_abs(uint64_t base) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, nativeGpSize, BaseMem::kSignatureMemAbs);
  }
  //! \overload
  inline Mem intptr_ptr_abs(uint64_t base, const Gp& index, uint32_t shift = 0) const noexcept {
    uint32_t nativeGpSize = _emitter()->gpSize();
    return Mem(base, index, shift, nativeGpSize, BaseMem::kSignatureMemAbs);
  }

  //! \}

  //! \name Embed
  //! \{

  //! Adds 8-bit integer data to the CodeBuffer.
  inline Error db(uint8_t x) { return _emitter()->embed(&x, 1); }
  //! Adds 16-bit integer data to the CodeBuffer.
  inline Error dw(uint16_t x) { return _emitter()->embed(&x, 2); }
  //! Adds 32-bit integer data to the CodeBuffer.
  inline Error dd(uint32_t x) { return _emitter()->embed(&x, 4); }
  //! Adds 64-bit integer data to the CodeBuffer.
  inline Error dq(uint64_t x) { return _emitter()->embed(&x, 8); }

  //! Adds 8-bit integer data to the CodeBuffer.
  inline Error dint8(int8_t x) { return _emitter()->embed(&x, sizeof(int8_t)); }
  //! Adds 8-bit integer data to the CodeBuffer.
  inline Error duint8(uint8_t x) { return _emitter()->embed(&x, sizeof(uint8_t)); }

  //! Adds 16-bit integer data to the CodeBuffer.
  inline Error dint16(int16_t x) { return _emitter()->embed(&x, sizeof(int16_t)); }
  //! Adds 16-bit integer data to the CodeBuffer.
  inline Error duint16(uint16_t x) { return _emitter()->embed(&x, sizeof(uint16_t)); }

  //! Adds 32-bit integer data to the CodeBuffer.
  inline Error dint32(int32_t x) { return _emitter()->embed(&x, sizeof(int32_t)); }
  //! Adds 32-bit integer data to the CodeBuffer.
  inline Error duint32(uint32_t x) { return _emitter()->embed(&x, sizeof(uint32_t)); }

  //! Adds 64-bit integer data to the CodeBuffer.
  inline Error dint64(int64_t x) { return _emitter()->embed(&x, sizeof(int64_t)); }
  //! Adds 64-bit integer data to the CodeBuffer.
  inline Error duint64(uint64_t x) { return _emitter()->embed(&x, sizeof(uint64_t)); }

  //! Adds float data to the CodeBuffer.
  inline Error dfloat(float x) { return _emitter()->embed(&x, sizeof(float)); }
  //! Adds double data to the CodeBuffer.
  inline Error ddouble(double x) { return _emitter()->embed(&x, sizeof(double)); }

  //! Adds MMX data to the CodeBuffer.
  inline Error dmm(const Data64& x) { return _emitter()->embed(&x, sizeof(Data64)); }
  //! Adds XMM data to the CodeBuffer.
  inline Error dxmm(const Data128& x) { return _emitter()->embed(&x, sizeof(Data128)); }
  //! Adds YMM data to the CodeBuffer.
  inline Error dymm(const Data256& x) { return _emitter()->embed(&x, sizeof(Data256)); }

  //! Adds data in a given structure instance to the CodeBuffer.
  template<typename T>
  inline Error dstruct(const T& x) { return _emitter()->embed(&x, uint32_t(sizeof(T))); }

  //! \}

protected:
  //! \cond
  inline This& _addInstOptions(uint32_t options) noexcept {
    _emitter()->addInstOptions(options);
    return *_emitter();
  }
  //! \endcond

public:
  //! \name Short/Long Form Options
  //! \{

  //! Force short form of jmp/jcc instruction.
  inline This& short_() noexcept { return _addInstOptions(Inst::kOptionShortForm); }
  //! Force long form of jmp/jcc instruction.
  inline This& long_() noexcept { return _addInstOptions(Inst::kOptionLongForm); }

  //! \}

  //! \name Encoding Options
  //! \{

  //! Prefer MOD_MR encoding over MOD_RM (the default) when encoding instruction
  //! that allows both. This option is only applicable to instructions where both
  //! operands are registers.
  inline This& mod_mr() noexcept { return _addInstOptions(Inst::kOptionModMR); }

  //! \}

  //! \name Prefix Options
  //! \{

  //! Condition is likely to be taken (has only benefit on P4).
  inline This& taken() noexcept { return _addInstOptions(Inst::kOptionTaken); }
  //! Condition is unlikely to be taken (has only benefit on P4).
  inline This& notTaken() noexcept { return _addInstOptions(Inst::kOptionNotTaken); }

  //! Use LOCK prefix.
  inline This& lock() noexcept { return _addInstOptions(Inst::kOptionLock); }
  //! Use XACQUIRE prefix.
  inline This& xacquire() noexcept { return _addInstOptions(Inst::kOptionXAcquire); }
  //! Use XRELEASE prefix.
  inline This& xrelease() noexcept { return _addInstOptions(Inst::kOptionXRelease); }

  //! Use BND/REPNE prefix.
  //!
  //! \note This is the same as using `repne()` or `repnz()` prefix.
  inline This& bnd() noexcept { return _addInstOptions(Inst::kOptionRepne); }

  //! Use REP/REPZ prefix.
  //!
  //! \note This is the same as using `repe()` or `repz()` prefix.
  inline This& rep(const Gp& zcx) noexcept {
    _emitter()->_extraReg.init(zcx);
    return _addInstOptions(Inst::kOptionRep);
  }

  //! Use REP/REPE prefix.
  //!
  //! \note This is the same as using `rep()` or `repz()` prefix.
  inline This& repe(const Gp& zcx) noexcept { return rep(zcx); }

  //! Use REP/REPE prefix.
  //!
  //! \note This is the same as using `rep()` or `repe()` prefix.
  inline This& repz(const Gp& zcx) noexcept { return rep(zcx); }

  //! Use REPNE prefix.
  //!
  //! \note This is the same as using `bnd()` or `repnz()` prefix.
  inline This& repne(const Gp& zcx) noexcept {
    _emitter()->_extraReg.init(zcx);
    return _addInstOptions(Inst::kOptionRepne);
  }

  //! Use REPNE prefix.
  //!
  //! \note This is the same as using `bnd()` or `repne()` prefix.
  inline This& repnz(const Gp& zcx) noexcept { return repne(zcx); }

  //! \}

  //! \name REX Options
  //! \{

  //! Force REX prefix to be emitted even when it's not needed (X86_64).
  //!
  //! \note Don't use when using high 8-bit registers as REX prefix makes them
  //! inaccessible and `x86::Assembler` would fail to encode such instruction.
  inline This& rex() noexcept { return _addInstOptions(Inst::kOptionRex); }

  //! Force REX.B prefix (X64) [It exists for special purposes only].
  inline This& rex_b() noexcept { return _addInstOptions(Inst::kOptionOpCodeB); }
  //! Force REX.X prefix (X64) [It exists for special purposes only].
  inline This& rex_x() noexcept { return _addInstOptions(Inst::kOptionOpCodeX); }
  //! Force REX.R prefix (X64) [It exists for special purposes only].
  inline This& rex_r() noexcept { return _addInstOptions(Inst::kOptionOpCodeR); }
  //! Force REX.W prefix (X64) [It exists for special purposes only].
  inline This& rex_w() noexcept { return _addInstOptions(Inst::kOptionOpCodeW); }

  //! \}

  //! \name VEX and EVEX Options
  //! \{

  //! Force 3-byte VEX prefix (AVX+).
  inline This& vex3() noexcept { return _addInstOptions(Inst::kOptionVex3); }
  //! Force 4-byte EVEX prefix (AVX512+).
  inline This& evex() noexcept { return _addInstOptions(Inst::kOptionEvex); }

  //! \}

  //! \name AVX-512 Options & Masking
  //! \{

  //! Use masking {k} (AVX512+).
  inline This& k(const KReg& kreg) noexcept {
    _emitter()->_extraReg.init(kreg);
    return *_emitter();
  }

  //! Use zeroing instead of merging (AVX512+).
  inline This& z() noexcept { return _addInstOptions(Inst::kOptionZMask); }

  //! Suppress all exceptions (AVX512+).
  inline This& sae() noexcept { return _addInstOptions(Inst::kOptionSAE); }
  //! Static rounding mode {rn} (round-to-nearest even) and {sae} (AVX512+).
  inline This& rn_sae() noexcept { return _addInstOptions(Inst::kOptionER | Inst::kOptionRN_SAE); }
  //! Static rounding mode {rd} (round-down, toward -inf) and {sae} (AVX512+).
  inline This& rd_sae() noexcept { return _addInstOptions(Inst::kOptionER | Inst::kOptionRD_SAE); }
  //! Static rounding mode {ru} (round-up, toward +inf) and {sae} (AVX512+).
  inline This& ru_sae() noexcept { return _addInstOptions(Inst::kOptionER | Inst::kOptionRU_SAE); }
  //! Static rounding mode {rz} (round-toward-zero, truncate) and {sae} (AVX512+).
  inline This& rz_sae() noexcept { return _addInstOptions(Inst::kOptionER | Inst::kOptionRZ_SAE); }

  //! \}

  //! \name Base Instructions & GP Extensions
  //! \{

  ASMJIT_INST_2x(adc, Adc, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(adc, Adc, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(adc, Adc, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(adc, Adc, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(adc, Adc, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(add, Add, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(add, Add, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(add, Add, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(add, Add, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(add, Add, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(and_, And, Gp, Gp)                                    // ANY
  ASMJIT_INST_2x(and_, And, Gp, Mem)                                   // ANY
  ASMJIT_INST_2i(and_, And, Gp, Imm)                                   // ANY
  ASMJIT_INST_2x(and_, And, Mem, Gp)                                   // ANY
  ASMJIT_INST_2i(and_, And, Mem, Imm)                                  // ANY
  ASMJIT_INST_2x(arpl, Arpl, Gp, Gp)                                   // X86
  ASMJIT_INST_2x(arpl, Arpl, Mem, Gp)                                  // X86
  ASMJIT_INST_2x(bound, Bound, Gp, Mem)                                // X86
  ASMJIT_INST_2x(bsf, Bsf, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(bsf, Bsf, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(bsr, Bsr, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(bsr, Bsr, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(bswap, Bswap, Gp)                                     // ANY
  ASMJIT_INST_2x(bt, Bt, Gp, Gp)                                       // ANY
  ASMJIT_INST_2i(bt, Bt, Gp, Imm)                                      // ANY
  ASMJIT_INST_2x(bt, Bt, Mem, Gp)                                      // ANY
  ASMJIT_INST_2i(bt, Bt, Mem, Imm)                                     // ANY
  ASMJIT_INST_2x(btc, Btc, Gp, Gp)                                     // ANY
  ASMJIT_INST_2i(btc, Btc, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(btc, Btc, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(btc, Btc, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(btr, Btr, Gp, Gp)                                     // ANY
  ASMJIT_INST_2i(btr, Btr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(btr, Btr, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(btr, Btr, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(bts, Bts, Gp, Gp)                                     // ANY
  ASMJIT_INST_2i(bts, Bts, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(bts, Bts, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(bts, Bts, Mem, Imm)                                   // ANY
  ASMJIT_INST_1x(cbw, Cbw, AX)                                         // ANY       [EXPLICIT] AX      <- Sign Extend AL
  ASMJIT_INST_2x(cdq, Cdq, EDX, EAX)                                   // ANY       [EXPLICIT] EDX:EAX <- Sign Extend EAX
  ASMJIT_INST_1x(cdqe, Cdqe, EAX)                                      // X64       [EXPLICIT] RAX     <- Sign Extend EAX
  ASMJIT_INST_2x(cqo, Cqo, RDX, RAX)                                   // X64       [EXPLICIT] RDX:RAX <- Sign Extend RAX
  ASMJIT_INST_2x(cwd, Cwd, DX, AX)                                     // ANY       [EXPLICIT] DX:AX   <- Sign Extend AX
  ASMJIT_INST_1x(cwde, Cwde, EAX)                                      // ANY       [EXPLICIT] EAX     <- Sign Extend AX
  ASMJIT_INST_1x(call, Call, Gp)                                       // ANY
  ASMJIT_INST_1x(call, Call, Mem)                                      // ANY
  ASMJIT_INST_1x(call, Call, Label)                                    // ANY
  ASMJIT_INST_1i(call, Call, Imm)                                      // ANY
  ASMJIT_INST_0x(clc, Clc)                                             // ANY
  ASMJIT_INST_0x(cld, Cld)                                             // ANY
  ASMJIT_INST_0x(cli, Cli)                                             // ANY
  ASMJIT_INST_0x(clts, Clts)                                           // ANY
  ASMJIT_INST_0x(cmc, Cmc)                                             // ANY
  ASMJIT_INST_2c(cmov, Cmov, Condition::toCmovcc, Gp, Gp)              // CMOV
  ASMJIT_INST_2c(cmov, Cmov, Condition::toCmovcc, Gp, Mem)             // CMOV
  ASMJIT_INST_2x(cmp, Cmp, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(cmp, Cmp, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(cmp, Cmp, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(cmp, Cmp, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(cmp, Cmp, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(cmps, Cmps, DS_ZSI, ES_ZDI)                           // ANY       [EXPLICIT]
  ASMJIT_INST_3x(cmpxchg, Cmpxchg, Gp, Gp, ZAX)                        // I486      [EXPLICIT]
  ASMJIT_INST_3x(cmpxchg, Cmpxchg, Mem, Gp, ZAX)                       // I486      [EXPLICIT]
  ASMJIT_INST_5x(cmpxchg16b, Cmpxchg16b, Mem, RDX, RAX, RCX, RBX);     // CMPXCHG16B[EXPLICIT] m == EDX:EAX ? m <- ECX:EBX
  ASMJIT_INST_5x(cmpxchg8b, Cmpxchg8b, Mem, EDX, EAX, ECX, EBX);       // CMPXCHG8B [EXPLICIT] m == RDX:RAX ? m <- RCX:RBX
  ASMJIT_INST_4x(cpuid, Cpuid, EAX, EBX, ECX, EDX)                     // I486      [EXPLICIT] EAX:EBX:ECX:EDX  <- CPUID[EAX:ECX]
  ASMJIT_INST_1x(daa, Daa, Gp)                                         // X86       [EXPLICIT]
  ASMJIT_INST_1x(das, Das, Gp)                                         // X86       [EXPLICIT]
  ASMJIT_INST_1x(dec, Dec, Gp)                                         // ANY
  ASMJIT_INST_1x(dec, Dec, Mem)                                        // ANY
  ASMJIT_INST_2x(div, Div, Gp, Gp)                                     // ANY       [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / r8
  ASMJIT_INST_2x(div, Div, Gp, Mem)                                    // ANY       [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / m8
  ASMJIT_INST_3x(div, Div, Gp, Gp, Gp)                                 // ANY       [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / r16|r32|r64
  ASMJIT_INST_3x(div, Div, Gp, Gp, Mem)                                // ANY       [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / m16|m32|m64
  ASMJIT_INST_0x(emms, Emms)                                           // MMX
  ASMJIT_INST_2x(enter, Enter, Imm, Imm)                               // ANY
  ASMJIT_INST_0x(hlt, Hlt)                                             // ANY
  ASMJIT_INST_2x(idiv, Idiv, Gp, Gp)                                   // ANY       [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / r8
  ASMJIT_INST_2x(idiv, Idiv, Gp, Mem)                                  // ANY       [EXPLICIT]  AH[Rem]: AL[Quot] <- AX / m8
  ASMJIT_INST_3x(idiv, Idiv, Gp, Gp, Gp)                               // ANY       [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / r16|r32|r64
  ASMJIT_INST_3x(idiv, Idiv, Gp, Gp, Mem)                              // ANY       [EXPLICIT] xDX[Rem]:xAX[Quot] <- xDX:xAX / m16|m32|m64
  ASMJIT_INST_2x(imul, Imul, Gp, Gp)                                   // ANY       [EXPLICIT] AX <- AL * r8 | ra <- ra * rb
  ASMJIT_INST_2x(imul, Imul, Gp, Mem)                                  // ANY       [EXPLICIT] AX <- AL * m8 | ra <- ra * m16|m32|m64
  ASMJIT_INST_2i(imul, Imul, Gp, Imm)                                  // ANY
  ASMJIT_INST_3i(imul, Imul, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3i(imul, Imul, Gp, Mem, Imm)                             // ANY
  ASMJIT_INST_3x(imul, Imul, Gp, Gp, Gp)                               // ANY       [EXPLICIT] xDX:xAX <- xAX * r16|r32|r64
  ASMJIT_INST_3x(imul, Imul, Gp, Gp, Mem)                              // ANY       [EXPLICIT] xDX:xAX <- xAX * m16|m32|m64
  ASMJIT_INST_2i(in, In, ZAX, Imm)                                     // ANY
  ASMJIT_INST_2x(in, In, ZAX, DX)                                      // ANY
  ASMJIT_INST_1x(inc, Inc, Gp)                                         // ANY
  ASMJIT_INST_1x(inc, Inc, Mem)                                        // ANY
  ASMJIT_INST_2x(ins, Ins, ES_ZDI, DX)                                 // ANY
  ASMJIT_INST_1i(int_, Int, Imm)                                       // ANY
  ASMJIT_INST_0x(int3, Int3)                                           // ANY
  ASMJIT_INST_0x(into, Into)                                           // ANY
  ASMJIT_INST_0x(invd, Invd)                                           // ANY
  ASMJIT_INST_1x(invlpg, Invlpg, Mem)                                  // ANY
  ASMJIT_INST_2x(invpcid, Invpcid, Gp, Mem)                            // ANY
  ASMJIT_INST_1c(j, J, Condition::toJcc, Label)                        // ANY
  ASMJIT_INST_1c(j, J, Condition::toJcc, Imm)                          // ANY
  ASMJIT_INST_1c(j, J, Condition::toJcc, uint64_t)                     // ANY
  ASMJIT_INST_2x(jecxz, Jecxz, Gp, Label)                              // ANY       [EXPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_2x(jecxz, Jecxz, Gp, Imm)                                // ANY       [EXPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_2x(jecxz, Jecxz, Gp, uint64_t)                           // ANY       [EXPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(jmp, Jmp, Gp)                                         // ANY
  ASMJIT_INST_1x(jmp, Jmp, Mem)                                        // ANY
  ASMJIT_INST_1x(jmp, Jmp, Label)                                      // ANY
  ASMJIT_INST_1x(jmp, Jmp, Imm)                                        // ANY
  ASMJIT_INST_1x(jmp, Jmp, uint64_t)                                   // ANY
  ASMJIT_INST_1x(lahf, Lahf, AH)                                       // LAHFSAHF  [EXPLICIT] AH <- EFL
  ASMJIT_INST_2x(lar, Lar, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(lar, Lar, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(ldmxcsr, Ldmxcsr, Mem)                                // SSE
  ASMJIT_INST_2x(lds, Lds, Gp, Mem)                                    // X86
  ASMJIT_INST_2x(lea, Lea, Gp, Mem)                                    // ANY
  ASMJIT_INST_0x(leave, Leave)                                         // ANY
  ASMJIT_INST_2x(les, Les, Gp, Mem)                                    // X86
  ASMJIT_INST_0x(lfence, Lfence)                                       // SSE2
  ASMJIT_INST_2x(lfs, Lfs, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(lgdt, Lgdt, Mem)                                      // ANY
  ASMJIT_INST_2x(lgs, Lgs, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(lidt, Lidt, Mem)                                      // ANY
  ASMJIT_INST_1x(lldt, Lldt, Gp)                                       // ANY
  ASMJIT_INST_1x(lldt, Lldt, Mem)                                      // ANY
  ASMJIT_INST_1x(lmsw, Lmsw, Gp)                                       // ANY
  ASMJIT_INST_1x(lmsw, Lmsw, Mem)                                      // ANY
  ASMJIT_INST_2x(lods, Lods, ZAX, DS_ZSI)                              // ANY       [EXPLICIT]
  ASMJIT_INST_2x(loop, Loop, ZCX, Label)                               // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_2x(loop, Loop, ZCX, Imm)                                 // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_2x(loop, Loop, ZCX, uint64_t)                            // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_2x(loope, Loope, ZCX, Label)                             // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_2x(loope, Loope, ZCX, Imm)                               // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_2x(loope, Loope, ZCX, uint64_t)                          // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_2x(loopne, Loopne, ZCX, Label)                           // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_2x(loopne, Loopne, ZCX, Imm)                             // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_2x(loopne, Loopne, ZCX, uint64_t)                        // ANY       [EXPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_2x(lsl, Lsl, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(lsl, Lsl, Gp, Mem)                                    // ANY
  ASMJIT_INST_2x(lss, Lss, Gp, Mem)                                    // ANY
  ASMJIT_INST_1x(ltr, Ltr, Gp)                                         // ANY
  ASMJIT_INST_1x(ltr, Ltr, Mem)                                        // ANY
  ASMJIT_INST_0x(mfence, Mfence)                                       // SSE2
  ASMJIT_INST_2x(mov, Mov, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(mov, Mov, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(mov, Mov, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(mov, Mov, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, CReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, CReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, DReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, DReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Gp, SReg)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, Mem, SReg)                                  // ANY
  ASMJIT_INST_2x(mov, Mov, SReg, Gp)                                   // ANY
  ASMJIT_INST_2x(mov, Mov, SReg, Mem)                                  // ANY
  ASMJIT_INST_2x(movnti, Movnti, Mem, Gp)                              // SSE2
  ASMJIT_INST_2x(movs, Movs, ES_ZDI, DS_ZSI)                           // ANY       [EXPLICIT]
  ASMJIT_INST_2x(movsx, Movsx, Gp, Gp)                                 // ANY
  ASMJIT_INST_2x(movsx, Movsx, Gp, Mem)                                // ANY
  ASMJIT_INST_2x(movsxd, Movsxd, Gp, Gp)                               // X64
  ASMJIT_INST_2x(movsxd, Movsxd, Gp, Mem)                              // X64
  ASMJIT_INST_2x(movzx, Movzx, Gp, Gp)                                 // ANY
  ASMJIT_INST_2x(movzx, Movzx, Gp, Mem)                                // ANY
  ASMJIT_INST_2x(mul, Mul, AX, Gp)                                     // ANY       [EXPLICIT] AX      <-  AL * r8
  ASMJIT_INST_2x(mul, Mul, AX, Mem)                                    // ANY       [EXPLICIT] AX      <-  AL * m8
  ASMJIT_INST_3x(mul, Mul, ZDX, ZAX, Gp)                               // ANY       [EXPLICIT] xDX:xAX <- xAX * r16|r32|r64
  ASMJIT_INST_3x(mul, Mul, ZDX, ZAX, Mem)                              // ANY       [EXPLICIT] xDX:xAX <- xAX * m16|m32|m64
  ASMJIT_INST_1x(neg, Neg, Gp)                                         // ANY
  ASMJIT_INST_1x(neg, Neg, Mem)                                        // ANY
  ASMJIT_INST_0x(nop, Nop)                                             // ANY
  ASMJIT_INST_1x(nop, Nop, Gp)                                         // ANY
  ASMJIT_INST_1x(nop, Nop, Mem)                                        // ANY
  ASMJIT_INST_1x(not_, Not, Gp)                                        // ANY
  ASMJIT_INST_1x(not_, Not, Mem)                                       // ANY
  ASMJIT_INST_2x(or_, Or, Gp, Gp)                                      // ANY
  ASMJIT_INST_2x(or_, Or, Gp, Mem)                                     // ANY
  ASMJIT_INST_2i(or_, Or, Gp, Imm)                                     // ANY
  ASMJIT_INST_2x(or_, Or, Mem, Gp)                                     // ANY
  ASMJIT_INST_2i(or_, Or, Mem, Imm)                                    // ANY
  ASMJIT_INST_2x(out, Out, Imm, ZAX)                                   // ANY
  ASMJIT_INST_2i(out, Out, DX, ZAX)                                    // ANY
  ASMJIT_INST_2i(outs, Outs, DX, DS_ZSI)                               // ANY
  ASMJIT_INST_0x(pause, Pause)                                         // SSE2
  ASMJIT_INST_1x(pop, Pop, Gp)                                         // ANY
  ASMJIT_INST_1x(pop, Pop, Mem)                                        // ANY
  ASMJIT_INST_1x(pop, Pop, SReg);                                      // ANY
  ASMJIT_INST_0x(popa, Popa)                                           // X86
  ASMJIT_INST_0x(popad, Popad)                                         // X86
  ASMJIT_INST_0x(popf, Popf)                                           // ANY
  ASMJIT_INST_0x(popfd, Popfd)                                         // X86
  ASMJIT_INST_0x(popfq, Popfq)                                         // X64
  ASMJIT_INST_1x(prefetch, Prefetch, Mem)                              // 3DNOW
  ASMJIT_INST_1x(prefetchnta, Prefetchnta, Mem)                        // SSE
  ASMJIT_INST_1x(prefetcht0, Prefetcht0, Mem)                          // SSE
  ASMJIT_INST_1x(prefetcht1, Prefetcht1, Mem)                          // SSE
  ASMJIT_INST_1x(prefetcht2, Prefetcht2, Mem)                          // SSE
  ASMJIT_INST_1x(prefetchw, Prefetchw, Mem)                            // PREFETCHW
  ASMJIT_INST_1x(prefetchwt1, Prefetchwt1, Mem)                        // PREFETCHW1
  ASMJIT_INST_1x(push, Push, Gp)                                       // ANY
  ASMJIT_INST_1x(push, Push, Mem)                                      // ANY
  ASMJIT_INST_1x(push, Push, SReg)                                     // ANY
  ASMJIT_INST_1i(push, Push, Imm)                                      // ANY
  ASMJIT_INST_0x(pusha, Pusha)                                         // X86
  ASMJIT_INST_0x(pushad, Pushad)                                       // X86
  ASMJIT_INST_0x(pushf, Pushf)                                         // ANY
  ASMJIT_INST_0x(pushfd, Pushfd)                                       // X86
  ASMJIT_INST_0x(pushfq, Pushfq)                                       // X64
  ASMJIT_INST_2x(rcl, Rcl, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(rcl, Rcl, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(rcl, Rcl, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(rcl, Rcl, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(rcr, Rcr, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(rcr, Rcr, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(rcr, Rcr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(rcr, Rcr, Mem, Imm)                                   // ANY
  ASMJIT_INST_3x(rdmsr, Rdmsr, EDX, EAX, ECX)                          // MSR       [EXPLICIT] RDX:EAX     <- MSR[ECX]
  ASMJIT_INST_3x(rdpmc, Rdpmc, EDX, EAX, ECX)                          // ANY       [EXPLICIT] RDX:EAX     <- PMC[ECX]
  ASMJIT_INST_2x(rdtsc, Rdtsc, EDX, EAX)                               // RDTSC     [EXPLICIT] EDX:EAX     <- Counter
  ASMJIT_INST_3x(rdtscp, Rdtscp, EDX, EAX, ECX)                        // RDTSCP    [EXPLICIT] EDX:EAX:EXC <- Counter
  ASMJIT_INST_2x(rol, Rol, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(rol, Rol, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(rol, Rol, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(rol, Rol, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(ror, Ror, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(ror, Ror, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(ror, Ror, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(ror, Ror, Mem, Imm)                                   // ANY
  ASMJIT_INST_0x(rsm, Rsm)                                             // X86
  ASMJIT_INST_2x(sbb, Sbb, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(sbb, Sbb, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(sbb, Sbb, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sbb, Sbb, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(sbb, Sbb, Mem, Imm)                                   // ANY
  ASMJIT_INST_1x(sahf, Sahf, AH)                                       // LAHFSAHF  [EXPLICIT] EFL <- AH
  ASMJIT_INST_2x(sal, Sal, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(sal, Sal, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(sal, Sal, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(sal, Sal, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(sar, Sar, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(sar, Sar, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(sar, Sar, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(sar, Sar, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(scas, Scas, ZAX, ES_ZDI)                              // ANY       [EXPLICIT]
  ASMJIT_INST_1c(set, Set, Condition::toSetcc, Gp)                     // ANY
  ASMJIT_INST_1c(set, Set, Condition::toSetcc, Mem)                    // ANY
  ASMJIT_INST_0x(sfence, Sfence)                                       // SSE
  ASMJIT_INST_1x(sgdt, Sgdt, Mem)                                      // ANY
  ASMJIT_INST_2x(shl, Shl, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(shl, Shl, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(shl, Shl, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(shl, Shl, Mem, Imm)                                   // ANY
  ASMJIT_INST_2x(shr, Shr, Gp, CL)                                     // ANY
  ASMJIT_INST_2x(shr, Shr, Mem, CL)                                    // ANY
  ASMJIT_INST_2i(shr, Shr, Gp, Imm)                                    // ANY
  ASMJIT_INST_2i(shr, Shr, Mem, Imm)                                   // ANY
  ASMJIT_INST_3x(shld, Shld, Gp, Gp, CL)                               // ANY
  ASMJIT_INST_3x(shld, Shld, Mem, Gp, CL)                              // ANY
  ASMJIT_INST_3i(shld, Shld, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3i(shld, Shld, Mem, Gp, Imm)                             // ANY
  ASMJIT_INST_3x(shrd, Shrd, Gp, Gp, CL)                               // ANY
  ASMJIT_INST_3x(shrd, Shrd, Mem, Gp, CL)                              // ANY
  ASMJIT_INST_3i(shrd, Shrd, Gp, Gp, Imm)                              // ANY
  ASMJIT_INST_3i(shrd, Shrd, Mem, Gp, Imm)                             // ANY
  ASMJIT_INST_1x(sidt, Sidt, Mem)                                      // ANY
  ASMJIT_INST_1x(sldt, Sldt, Gp)                                       // ANY
  ASMJIT_INST_1x(sldt, Sldt, Mem)                                      // ANY
  ASMJIT_INST_1x(smsw, Smsw, Gp)                                       // ANY
  ASMJIT_INST_1x(smsw, Smsw, Mem)                                      // ANY
  ASMJIT_INST_0x(stc, Stc)                                             // ANY
  ASMJIT_INST_0x(std, Std)                                             // ANY
  ASMJIT_INST_0x(sti, Sti)                                             // ANY
  ASMJIT_INST_1x(stmxcsr, Stmxcsr, Mem)                                // SSE
  ASMJIT_INST_2x(stos, Stos, ES_ZDI, ZAX)                              // ANY       [EXPLICIT]
  ASMJIT_INST_1x(str, Str, Gp)                                         // ANY
  ASMJIT_INST_1x(str, Str, Mem)                                        // ANY
  ASMJIT_INST_2x(sub, Sub, Gp, Gp)                                     // ANY
  ASMJIT_INST_2x(sub, Sub, Gp, Mem)                                    // ANY
  ASMJIT_INST_2i(sub, Sub, Gp, Imm)                                    // ANY
  ASMJIT_INST_2x(sub, Sub, Mem, Gp)                                    // ANY
  ASMJIT_INST_2i(sub, Sub, Mem, Imm)                                   // ANY
  ASMJIT_INST_0x(swapgs, Swapgs)                                       // X64
  ASMJIT_INST_2x(test, Test, Gp, Gp)                                   // ANY
  ASMJIT_INST_2i(test, Test, Gp, Imm)                                  // ANY
  ASMJIT_INST_2x(test, Test, Mem, Gp)                                  // ANY
  ASMJIT_INST_2i(test, Test, Mem, Imm)                                 // ANY
  ASMJIT_INST_0x(ud2, Ud2)                                             // ANY
  ASMJIT_INST_1x(verr, Verr, Gp)                                       // ANY
  ASMJIT_INST_1x(verr, Verr, Mem)                                      // ANY
  ASMJIT_INST_1x(verw, Verw, Gp)                                       // ANY
  ASMJIT_INST_1x(verw, Verw, Mem)                                      // ANY
  ASMJIT_INST_3x(wrmsr, Wrmsr, EDX, EAX, ECX)                          // MSR       [EXPLICIT] RDX:EAX     -> MSR[ECX]
  ASMJIT_INST_2x(xadd, Xadd, Gp, Gp)                                   // ANY
  ASMJIT_INST_2x(xadd, Xadd, Mem, Gp)                                  // ANY
  ASMJIT_INST_2x(xchg, Xchg, Gp, Gp)                                   // ANY
  ASMJIT_INST_2x(xchg, Xchg, Mem, Gp)                                  // ANY
  ASMJIT_INST_2x(xchg, Xchg, Gp, Mem)                                  // ANY
  ASMJIT_INST_2x(xor_, Xor, Gp, Gp)                                    // ANY
  ASMJIT_INST_2x(xor_, Xor, Gp, Mem)                                   // ANY
  ASMJIT_INST_2i(xor_, Xor, Gp, Imm)                                   // ANY
  ASMJIT_INST_2x(xor_, Xor, Mem, Gp)                                   // ANY
  ASMJIT_INST_2i(xor_, Xor, Mem, Imm)                                  // ANY

  //! \}

  //! \name ADX Instructions
  //! \{

  ASMJIT_INST_2x(adcx, Adcx, Gp, Gp)                                   // ADX
  ASMJIT_INST_2x(adcx, Adcx, Gp, Mem)                                  // ADX
  ASMJIT_INST_2x(adox, Adox, Gp, Gp)                                   // ADX
  ASMJIT_INST_2x(adox, Adox, Gp, Mem)                                  // ADX

  //! \}

  //! \name BMI Instructions
  //! \{

  ASMJIT_INST_3x(andn, Andn, Gp, Gp, Gp)                               // BMI
  ASMJIT_INST_3x(andn, Andn, Gp, Gp, Mem)                              // BMI
  ASMJIT_INST_3x(bextr, Bextr, Gp, Gp, Gp)                             // BMI
  ASMJIT_INST_3x(bextr, Bextr, Gp, Mem, Gp)                            // BMI
  ASMJIT_INST_2x(blsi, Blsi, Gp, Gp)                                   // BMI
  ASMJIT_INST_2x(blsi, Blsi, Gp, Mem)                                  // BMI
  ASMJIT_INST_2x(blsmsk, Blsmsk, Gp, Gp)                               // BMI
  ASMJIT_INST_2x(blsmsk, Blsmsk, Gp, Mem)                              // BMI
  ASMJIT_INST_2x(blsr, Blsr, Gp, Gp)                                   // BMI
  ASMJIT_INST_2x(blsr, Blsr, Gp, Mem)                                  // BMI
  ASMJIT_INST_2x(tzcnt, Tzcnt, Gp, Gp)                                 // BMI
  ASMJIT_INST_2x(tzcnt, Tzcnt, Gp, Mem)                                // BMI

  //! \}

  //! \name BMI2 Instructions
  //! \{

  ASMJIT_INST_3x(bzhi, Bzhi, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(bzhi, Bzhi, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_4x(mulx, Mulx, Gp, Gp, Gp, ZDX)                          // BMI2      [EXPLICIT]
  ASMJIT_INST_4x(mulx, Mulx, Gp, Gp, Mem, ZDX)                         // BMI2      [EXPLICIT]
  ASMJIT_INST_3x(pdep, Pdep, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(pdep, Pdep, Gp, Gp, Mem)                              // BMI2
  ASMJIT_INST_3x(pext, Pext, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(pext, Pext, Gp, Gp, Mem)                              // BMI2
  ASMJIT_INST_3i(rorx, Rorx, Gp, Gp, Imm)                              // BMI2
  ASMJIT_INST_3i(rorx, Rorx, Gp, Mem, Imm)                             // BMI2
  ASMJIT_INST_3x(sarx, Sarx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(sarx, Sarx, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_3x(shlx, Shlx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(shlx, Shlx, Gp, Mem, Gp)                              // BMI2
  ASMJIT_INST_3x(shrx, Shrx, Gp, Gp, Gp)                               // BMI2
  ASMJIT_INST_3x(shrx, Shrx, Gp, Mem, Gp)                              // BMI2

  //! \}

  //! \name CL Instructions
  //! \{

  ASMJIT_INST_1x(cldemote, Cldemote, Mem)                              // CLDEMOTE
  ASMJIT_INST_1x(clflush, Clflush, Mem)                                // CLFLUSH
  ASMJIT_INST_1x(clflushopt, Clflushopt, Mem)                          // CLFLUSH_OPT
  ASMJIT_INST_1x(clwb, Clwb, Mem)                                      // CLWB
  ASMJIT_INST_1x(clzero, Clzero, DS_ZAX)                               // CLZERO    [EXPLICIT]
  ASMJIT_INST_0x(wbnoinvd, Wbnoinvd)                                   // WBNOINVD

  //! \}

  //! \name CRC32 Instructions
  //! \{

  ASMJIT_INST_2x(crc32, Crc32, Gp, Gp)                                 // SSE4_2
  ASMJIT_INST_2x(crc32, Crc32, Gp, Mem)                                // SSE4_2

  //! \}

  //! \name ENQCMD Instructions
  //! \{

  ASMJIT_INST_2x(enqcmd, Enqcmd, Mem, Mem)                             // ENQCMD
  ASMJIT_INST_2x(enqcmds, Enqcmds, Mem, Mem)                           // ENQCMD

  //! \}

  //! \name FSGSBASE Instructions
  //! \{

  ASMJIT_INST_1x(rdfsbase, Rdfsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(rdgsbase, Rdgsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(wrfsbase, Wrfsbase, Gp)                               // FSGSBASE
  ASMJIT_INST_1x(wrgsbase, Wrgsbase, Gp)                               // FSGSBASE

  //! \}

  //! \name FXSR & XSAVE Instructions
  //! \{

  ASMJIT_INST_1x(fxrstor, Fxrstor, Mem)                                // FXSR
  ASMJIT_INST_1x(fxrstor64, Fxrstor64, Mem)                            // FXSR
  ASMJIT_INST_1x(fxsave, Fxsave, Mem)                                  // FXSR
  ASMJIT_INST_1x(fxsave64, Fxsave64, Mem)                              // FXSR
  ASMJIT_INST_3x(xgetbv, Xgetbv, EDX, EAX, ECX)                        // XSAVE     [EXPLICIT] EDX:EAX <- XCR[ECX]
  ASMJIT_INST_3x(xsetbv, Xsetbv, EDX, EAX, ECX)                        // XSAVE     [EXPLICIT] XCR[ECX] <- EDX:EAX

  //! \}

  //! \name LWP Instructions
  //! \{

  ASMJIT_INST_1x(llwpcb, Llwpcb, Gp)                                   // LWP
  ASMJIT_INST_3i(lwpins, Lwpins, Gp, Gp, Imm)                          // LWP
  ASMJIT_INST_3i(lwpins, Lwpins, Gp, Mem, Imm)                         // LWP
  ASMJIT_INST_3i(lwpval, Lwpval, Gp, Gp, Imm)                          // LWP
  ASMJIT_INST_3i(lwpval, Lwpval, Gp, Mem, Imm)                         // LWP
  ASMJIT_INST_1x(slwpcb, Slwpcb, Gp)                                   // LWP

  //! \}

  //! \name LZCNT Instructions
  //! \{

  ASMJIT_INST_2x(lzcnt, Lzcnt, Gp, Gp)                                 // LZCNT
  ASMJIT_INST_2x(lzcnt, Lzcnt, Gp, Mem)                                // LZCNT

  //! \}

  //! \name MOVBE Instructions
  //! \{

  ASMJIT_INST_2x(movbe, Movbe, Gp, Mem)                                // MOVBE
  ASMJIT_INST_2x(movbe, Movbe, Mem, Gp)                                // MOVBE

  //! \}

  //! \name MOVDIRI & MOVDIR64B Instructions
  //! \{

  ASMJIT_INST_2x(movdiri, Movdiri, Mem, Gp)                            // MOVDIRI
  ASMJIT_INST_2x(movdir64b, Movdir64b, Mem, Mem)                       // MOVDIR64B

  //! \}

  //! \name MPX Extensions
  //! \{

  ASMJIT_INST_2x(bndcl, Bndcl, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcl, Bndcl, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndcn, Bndcn, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcn, Bndcn, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndcu, Bndcu, Bnd, Gp)                                // MPX
  ASMJIT_INST_2x(bndcu, Bndcu, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndldx, Bndldx, Bnd, Mem)                             // MPX
  ASMJIT_INST_2x(bndmk, Bndmk, Bnd, Mem)                               // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Bnd, Bnd)                             // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Bnd, Mem)                             // MPX
  ASMJIT_INST_2x(bndmov, Bndmov, Mem, Bnd)                             // MPX
  ASMJIT_INST_2x(bndstx, Bndstx, Mem, Bnd)                             // MPX

  //! \}

  //! \name POPCNT Instructions
  //! \{

  ASMJIT_INST_2x(popcnt, Popcnt, Gp, Gp)                               // POPCNT
  ASMJIT_INST_2x(popcnt, Popcnt, Gp, Mem)                              // POPCNT

  //! \}

  //! \name RDRAND & RDSEED Instructions
  //! \{

  ASMJIT_INST_1x(rdrand, Rdrand, Gp)                                   // RDRAND
  ASMJIT_INST_1x(rdseed, Rdseed, Gp)                                   // RDSEED

  //! \}

  //! \name RTM & TSX Instructions
  //! \{

  ASMJIT_INST_0x(xabort, Xabort)                                       // RTM
  ASMJIT_INST_1x(xbegin, Xbegin, Label)                                // RTM
  ASMJIT_INST_1x(xbegin, Xbegin, Imm)                                  // RTM
  ASMJIT_INST_1x(xbegin, Xbegin, uint64_t)                             // RTM
  ASMJIT_INST_0x(xend, Xend)                                           // RTM
  ASMJIT_INST_0x(xtest, Xtest)                                         // TSX

  //! \}

  //! \name SMAP Instructions
  //! \{

  ASMJIT_INST_0x(clac, Clac)                                           // SMAP
  ASMJIT_INST_0x(stac, Stac)                                           // SMAP

  //! \}

  //! \name SVM Instructions
  //! \{

  ASMJIT_INST_0x(clgi, Clgi)                                           // SVM
  ASMJIT_INST_2x(invlpga, Invlpga, Gp, Gp)                             // SVM       [EXPLICIT] <eax|rax, ecx>
  ASMJIT_INST_1x(skinit, Skinit, Gp)                                   // SKINIT    [EXPLICIT] <eax>
  ASMJIT_INST_0x(stgi, Stgi)                                           // SKINIT
  ASMJIT_INST_1x(vmload, Vmload, Gp)                                   // SVM       [EXPLICIT] <zax>
  ASMJIT_INST_0x(vmmcall, Vmmcall)                                     // SVM
  ASMJIT_INST_1x(vmrun, Vmrun, Gp)                                     // SVM       [EXPLICIT] <zax>
  ASMJIT_INST_1x(vmsave, Vmsave, Gp)                                   // SVM       [EXPLICIT] <zax>

  //! \}

  //! \name TBM Instructions
  //! \{

  ASMJIT_INST_2x(blcfill, Blcfill, Gp, Gp)                             // TBM
  ASMJIT_INST_2x(blcfill, Blcfill, Gp, Mem)                            // TBM
  ASMJIT_INST_2x(blci, Blci, Gp, Gp)                                   // TBM
  ASMJIT_INST_2x(blci, Blci, Gp, Mem)                                  // TBM
  ASMJIT_INST_2x(blcic, Blcic, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(blcic, Blcic, Gp, Mem)                                // TBM
  ASMJIT_INST_2x(blcmsk, Blcmsk, Gp, Gp)                               // TBM
  ASMJIT_INST_2x(blcmsk, Blcmsk, Gp, Mem)                              // TBM
  ASMJIT_INST_2x(blcs, Blcs, Gp, Gp)                                   // TBM
  ASMJIT_INST_2x(blcs, Blcs, Gp, Mem)                                  // TBM
  ASMJIT_INST_2x(blsfill, Blsfill, Gp, Gp)                             // TBM
  ASMJIT_INST_2x(blsfill, Blsfill, Gp, Mem)                            // TBM
  ASMJIT_INST_2x(blsic, Blsic, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(blsic, Blsic, Gp, Mem)                                // TBM
  ASMJIT_INST_2x(t1mskc, T1mskc, Gp, Gp)                               // TBM
  ASMJIT_INST_2x(t1mskc, T1mskc, Gp, Mem)                              // TBM
  ASMJIT_INST_2x(tzmsk, Tzmsk, Gp, Gp)                                 // TBM
  ASMJIT_INST_2x(tzmsk, Tzmsk, Gp, Mem)                                // TBM

  //! \}

  //! \name VMX Instructions
  //! \{

  ASMJIT_INST_2x(invept, Invept, Gp, Mem)                              // VMX
  ASMJIT_INST_2x(invvpid, Invvpid, Gp, Mem)                            // VMX
  ASMJIT_INST_0x(vmcall, Vmcall)                                       // VMX
  ASMJIT_INST_1x(vmclear, Vmclear, Mem)                                // VMX
  ASMJIT_INST_0x(vmfunc, Vmfunc)                                       // VMX
  ASMJIT_INST_0x(vmlaunch, Vmlaunch)                                   // VMX
  ASMJIT_INST_1x(vmptrld, Vmptrld, Mem)                                // VMX
  ASMJIT_INST_1x(vmptrst, Vmptrst, Mem)                                // VMX
  ASMJIT_INST_2x(vmread, Vmread, Mem, Gp)                              // VMX
  ASMJIT_INST_0x(vmresume, Vmresume)                                   // VMX
  ASMJIT_INST_2x(vmwrite, Vmwrite, Gp, Mem)                            // VMX
  ASMJIT_INST_1x(vmxon, Vmxon, Mem)                                    // VMX

  //! \}

  //! \name Other GP Instructions
  //! \{

  ASMJIT_INST_0x(getsec, Getsec)                                       // SMX
  ASMJIT_INST_0x(pcommit, Pcommit)                                     // PCOMMIT
  ASMJIT_INST_1x(rdpid, Rdpid, Gp)                                     // RDPID

  //! \}

  //! \name FPU Instructions
  //! \{

  ASMJIT_INST_0x(f2xm1, F2xm1)                                         // FPU
  ASMJIT_INST_0x(fabs, Fabs)                                           // FPU
  ASMJIT_INST_2x(fadd, Fadd, St, St)                                   // FPU
  ASMJIT_INST_1x(fadd, Fadd, Mem)                                      // FPU
  ASMJIT_INST_1x(faddp, Faddp, St)                                     // FPU
  ASMJIT_INST_0x(faddp, Faddp)                                         // FPU
  ASMJIT_INST_1x(fbld, Fbld, Mem)                                      // FPU
  ASMJIT_INST_1x(fbstp, Fbstp, Mem)                                    // FPU
  ASMJIT_INST_0x(fchs, Fchs)                                           // FPU
  ASMJIT_INST_0x(fclex, Fclex)                                         // FPU
  ASMJIT_INST_1x(fcmovb, Fcmovb, St)                                   // FPU
  ASMJIT_INST_1x(fcmovbe, Fcmovbe, St)                                 // FPU
  ASMJIT_INST_1x(fcmove, Fcmove, St)                                   // FPU
  ASMJIT_INST_1x(fcmovnb, Fcmovnb, St)                                 // FPU
  ASMJIT_INST_1x(fcmovnbe, Fcmovnbe, St)                               // FPU
  ASMJIT_INST_1x(fcmovne, Fcmovne, St)                                 // FPU
  ASMJIT_INST_1x(fcmovnu, Fcmovnu, St)                                 // FPU
  ASMJIT_INST_1x(fcmovu, Fcmovu, St)                                   // FPU
  ASMJIT_INST_1x(fcom, Fcom, St)                                       // FPU
  ASMJIT_INST_0x(fcom, Fcom)                                           // FPU
  ASMJIT_INST_1x(fcom, Fcom, Mem)                                      // FPU
  ASMJIT_INST_1x(fcomp, Fcomp, St)                                     // FPU
  ASMJIT_INST_0x(fcomp, Fcomp)                                         // FPU
  ASMJIT_INST_1x(fcomp, Fcomp, Mem)                                    // FPU
  ASMJIT_INST_0x(fcompp, Fcompp)                                       // FPU
  ASMJIT_INST_1x(fcomi, Fcomi, St)                                     // FPU
  ASMJIT_INST_1x(fcomip, Fcomip, St)                                   // FPU
  ASMJIT_INST_0x(fcos, Fcos)                                           // FPU
  ASMJIT_INST_0x(fdecstp, Fdecstp)                                     // FPU
  ASMJIT_INST_2x(fdiv, Fdiv, St, St)                                   // FPU
  ASMJIT_INST_1x(fdiv, Fdiv, Mem)                                      // FPU
  ASMJIT_INST_1x(fdivp, Fdivp, St)                                     // FPU
  ASMJIT_INST_0x(fdivp, Fdivp)                                         // FPU
  ASMJIT_INST_2x(fdivr, Fdivr, St, St)                                 // FPU
  ASMJIT_INST_1x(fdivr, Fdivr, Mem)                                    // FPU
  ASMJIT_INST_1x(fdivrp, Fdivrp, St)                                   // FPU
  ASMJIT_INST_0x(fdivrp, Fdivrp)                                       // FPU
  ASMJIT_INST_1x(ffree, Ffree, St)                                     // FPU
  ASMJIT_INST_1x(fiadd, Fiadd, Mem)                                    // FPU
  ASMJIT_INST_1x(ficom, Ficom, Mem)                                    // FPU
  ASMJIT_INST_1x(ficomp, Ficomp, Mem)                                  // FPU
  ASMJIT_INST_1x(fidiv, Fidiv, Mem)                                    // FPU
  ASMJIT_INST_1x(fidivr, Fidivr, Mem)                                  // FPU
  ASMJIT_INST_1x(fild, Fild, Mem)                                      // FPU
  ASMJIT_INST_1x(fimul, Fimul, Mem)                                    // FPU
  ASMJIT_INST_0x(fincstp, Fincstp)                                     // FPU
  ASMJIT_INST_0x(finit, Finit)                                         // FPU
  ASMJIT_INST_1x(fisub, Fisub, Mem)                                    // FPU
  ASMJIT_INST_1x(fisubr, Fisubr, Mem)                                  // FPU
  ASMJIT_INST_0x(fninit, Fninit)                                       // FPU
  ASMJIT_INST_1x(fist, Fist, Mem)                                      // FPU
  ASMJIT_INST_1x(fistp, Fistp, Mem)                                    // FPU
  ASMJIT_INST_1x(fisttp, Fisttp, Mem)                                  // FPU+SSE3
  ASMJIT_INST_1x(fld, Fld, Mem)                                        // FPU
  ASMJIT_INST_1x(fld, Fld, St)                                         // FPU
  ASMJIT_INST_0x(fld1, Fld1)                                           // FPU
  ASMJIT_INST_0x(fldl2t, Fldl2t)                                       // FPU
  ASMJIT_INST_0x(fldl2e, Fldl2e)                                       // FPU
  ASMJIT_INST_0x(fldpi, Fldpi)                                         // FPU
  ASMJIT_INST_0x(fldlg2, Fldlg2)                                       // FPU
  ASMJIT_INST_0x(fldln2, Fldln2)                                       // FPU
  ASMJIT_INST_0x(fldz, Fldz)                                           // FPU
  ASMJIT_INST_1x(fldcw, Fldcw, Mem)                                    // FPU
  ASMJIT_INST_1x(fldenv, Fldenv, Mem)                                  // FPU
  ASMJIT_INST_2x(fmul, Fmul, St, St)                                   // FPU
  ASMJIT_INST_1x(fmul, Fmul, Mem)                                      // FPU
  ASMJIT_INST_1x(fmulp, Fmulp, St)                                     // FPU
  ASMJIT_INST_0x(fmulp, Fmulp)                                         // FPU
  ASMJIT_INST_0x(fnclex, Fnclex)                                       // FPU
  ASMJIT_INST_0x(fnop, Fnop)                                           // FPU
  ASMJIT_INST_1x(fnsave, Fnsave, Mem)                                  // FPU
  ASMJIT_INST_1x(fnstenv, Fnstenv, Mem)                                // FPU
  ASMJIT_INST_1x(fnstcw, Fnstcw, Mem)                                  // FPU
  ASMJIT_INST_0x(fpatan, Fpatan)                                       // FPU
  ASMJIT_INST_0x(fprem, Fprem)                                         // FPU
  ASMJIT_INST_0x(fprem1, Fprem1)                                       // FPU
  ASMJIT_INST_0x(fptan, Fptan)                                         // FPU
  ASMJIT_INST_0x(frndint, Frndint)                                     // FPU
  ASMJIT_INST_1x(frstor, Frstor, Mem)                                  // FPU
  ASMJIT_INST_1x(fsave, Fsave, Mem)                                    // FPU
  ASMJIT_INST_0x(fscale, Fscale)                                       // FPU
  ASMJIT_INST_0x(fsin, Fsin)                                           // FPU
  ASMJIT_INST_0x(fsincos, Fsincos)                                     // FPU
  ASMJIT_INST_0x(fsqrt, Fsqrt)                                         // FPU
  ASMJIT_INST_1x(fst, Fst, Mem)                                        // FPU
  ASMJIT_INST_1x(fst, Fst, St)                                         // FPU
  ASMJIT_INST_1x(fstp, Fstp, Mem)                                      // FPU
  ASMJIT_INST_1x(fstp, Fstp, St)                                       // FPU
  ASMJIT_INST_1x(fstcw, Fstcw, Mem)                                    // FPU
  ASMJIT_INST_1x(fstenv, Fstenv, Mem)                                  // FPU
  ASMJIT_INST_2x(fsub, Fsub, St, St)                                   // FPU
  ASMJIT_INST_1x(fsub, Fsub, Mem)                                      // FPU
  ASMJIT_INST_1x(fsubp, Fsubp, St)                                     // FPU
  ASMJIT_INST_0x(fsubp, Fsubp)                                         // FPU
  ASMJIT_INST_2x(fsubr, Fsubr, St, St)                                 // FPU
  ASMJIT_INST_1x(fsubr, Fsubr, Mem)                                    // FPU
  ASMJIT_INST_1x(fsubrp, Fsubrp, St)                                   // FPU
  ASMJIT_INST_0x(fsubrp, Fsubrp)                                       // FPU
  ASMJIT_INST_0x(ftst, Ftst)                                           // FPU
  ASMJIT_INST_1x(fucom, Fucom, St)                                     // FPU
  ASMJIT_INST_0x(fucom, Fucom)                                         // FPU
  ASMJIT_INST_1x(fucomi, Fucomi, St)                                   // FPU
  ASMJIT_INST_1x(fucomip, Fucomip, St)                                 // FPU
  ASMJIT_INST_1x(fucomp, Fucomp, St)                                   // FPU
  ASMJIT_INST_0x(fucomp, Fucomp)                                       // FPU
  ASMJIT_INST_0x(fucompp, Fucompp)                                     // FPU
  ASMJIT_INST_0x(fwait, Fwait)                                         // FPU
  ASMJIT_INST_0x(fxam, Fxam)                                           // FPU
  ASMJIT_INST_1x(fxch, Fxch, St)                                       // FPU
  ASMJIT_INST_0x(fxtract, Fxtract)                                     // FPU
  ASMJIT_INST_0x(fyl2x, Fyl2x)                                         // FPU
  ASMJIT_INST_0x(fyl2xp1, Fyl2xp1)                                     // FPU
  ASMJIT_INST_1x(fstsw, Fstsw, Gp)                                     // FPU
  ASMJIT_INST_1x(fstsw, Fstsw, Mem)                                    // FPU
  ASMJIT_INST_1x(fnstsw, Fnstsw, Gp)                                   // FPU
  ASMJIT_INST_1x(fnstsw, Fnstsw, Mem)                                  // FPU

  //! \}

  //! \name MMX & SSE+ Instructions
  //! \{

  ASMJIT_INST_2x(addpd, Addpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(addpd, Addpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(addps, Addps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(addps, Addps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(addsd, Addsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(addsd, Addsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(addss, Addss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(addss, Addss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(addsubpd, Addsubpd, Xmm, Xmm)                         // SSE3
  ASMJIT_INST_2x(addsubpd, Addsubpd, Xmm, Mem)                         // SSE3
  ASMJIT_INST_2x(addsubps, Addsubps, Xmm, Xmm)                         // SSE3
  ASMJIT_INST_2x(addsubps, Addsubps, Xmm, Mem)                         // SSE3
  ASMJIT_INST_2x(andnpd, Andnpd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(andnpd, Andnpd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(andnps, Andnps, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(andnps, Andnps, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(andpd, Andpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(andpd, Andpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(andps, Andps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(andps, Andps, Xmm, Mem)                               // SSE
  ASMJIT_INST_3i(blendpd, Blendpd, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(blendpd, Blendpd, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3i(blendps, Blendps, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(blendps, Blendps, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3x(blendvpd, Blendvpd, Xmm, Xmm, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvpd, Blendvpd, Xmm, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvps, Blendvps, Xmm, Xmm, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(blendvps, Blendvps, Xmm, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3i(cmppd, Cmppd, Xmm, Xmm, Imm)                          // SSE2
  ASMJIT_INST_3i(cmppd, Cmppd, Xmm, Mem, Imm)                          // SSE2
  ASMJIT_INST_3i(cmpps, Cmpps, Xmm, Xmm, Imm)                          // SSE
  ASMJIT_INST_3i(cmpps, Cmpps, Xmm, Mem, Imm)                          // SSE
  ASMJIT_INST_3i(cmpsd, Cmpsd, Xmm, Xmm, Imm)                          // SSE2
  ASMJIT_INST_3i(cmpsd, Cmpsd, Xmm, Mem, Imm)                          // SSE2
  ASMJIT_INST_3i(cmpss, Cmpss, Xmm, Xmm, Imm)                          // SSE
  ASMJIT_INST_3i(cmpss, Cmpss, Xmm, Mem, Imm)                          // SSE
  ASMJIT_INST_2x(comisd, Comisd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(comisd, Comisd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(comiss, Comiss, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(comiss, Comiss, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(cvtdq2pd, Cvtdq2pd, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtdq2pd, Cvtdq2pd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtdq2ps, Cvtdq2ps, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtdq2ps, Cvtdq2ps, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpd2dq, Cvtpd2dq, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtpd2dq, Cvtpd2dq, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpd2pi, Cvtpd2pi, Mm, Xmm)                          // SSE2
  ASMJIT_INST_2x(cvtpd2pi, Cvtpd2pi, Mm, Mem)                          // SSE2
  ASMJIT_INST_2x(cvtpd2ps, Cvtpd2ps, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtpd2ps, Cvtpd2ps, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpi2pd, Cvtpi2pd, Xmm, Mm)                          // SSE2
  ASMJIT_INST_2x(cvtpi2pd, Cvtpi2pd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtpi2ps, Cvtpi2ps, Xmm, Mm)                          // SSE
  ASMJIT_INST_2x(cvtpi2ps, Cvtpi2ps, Xmm, Mem)                         // SSE
  ASMJIT_INST_2x(cvtps2dq, Cvtps2dq, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtps2dq, Cvtps2dq, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtps2pd, Cvtps2pd, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtps2pd, Cvtps2pd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtps2pi, Cvtps2pi, Mm, Xmm)                          // SSE
  ASMJIT_INST_2x(cvtps2pi, Cvtps2pi, Mm, Mem)                          // SSE
  ASMJIT_INST_2x(cvtsd2si, Cvtsd2si, Gp, Xmm)                          // SSE2
  ASMJIT_INST_2x(cvtsd2si, Cvtsd2si, Gp, Mem)                          // SSE2
  ASMJIT_INST_2x(cvtsd2ss, Cvtsd2ss, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtsd2ss, Cvtsd2ss, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtsi2sd, Cvtsi2sd, Xmm, Gp)                          // SSE2
  ASMJIT_INST_2x(cvtsi2sd, Cvtsi2sd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtsi2ss, Cvtsi2ss, Xmm, Gp)                          // SSE
  ASMJIT_INST_2x(cvtsi2ss, Cvtsi2ss, Xmm, Mem)                         // SSE
  ASMJIT_INST_2x(cvtss2sd, Cvtss2sd, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(cvtss2sd, Cvtss2sd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(cvtss2si, Cvtss2si, Gp, Xmm)                          // SSE
  ASMJIT_INST_2x(cvtss2si, Cvtss2si, Gp, Mem)                          // SSE
  ASMJIT_INST_2x(cvttpd2pi, Cvttpd2pi, Mm, Xmm)                        // SSE2
  ASMJIT_INST_2x(cvttpd2pi, Cvttpd2pi, Mm, Mem)                        // SSE2
  ASMJIT_INST_2x(cvttpd2dq, Cvttpd2dq, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(cvttpd2dq, Cvttpd2dq, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(cvttps2dq, Cvttps2dq, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(cvttps2dq, Cvttps2dq, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(cvttps2pi, Cvttps2pi, Mm, Xmm)                        // SSE
  ASMJIT_INST_2x(cvttps2pi, Cvttps2pi, Mm, Mem)                        // SSE
  ASMJIT_INST_2x(cvttsd2si, Cvttsd2si, Gp, Xmm)                        // SSE2
  ASMJIT_INST_2x(cvttsd2si, Cvttsd2si, Gp, Mem)                        // SSE2
  ASMJIT_INST_2x(cvttss2si, Cvttss2si, Gp, Xmm)                        // SSE
  ASMJIT_INST_2x(cvttss2si, Cvttss2si, Gp, Mem)                        // SSE
  ASMJIT_INST_2x(divpd, Divpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(divpd, Divpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(divps, Divps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(divps, Divps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(divsd, Divsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(divsd, Divsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(divss, Divss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(divss, Divss, Xmm, Mem)                               // SSE
  ASMJIT_INST_3i(dppd, Dppd, Xmm, Xmm, Imm)                            // SSE4_1
  ASMJIT_INST_3i(dppd, Dppd, Xmm, Mem, Imm)                            // SSE4_1
  ASMJIT_INST_3i(dpps, Dpps, Xmm, Xmm, Imm)                            // SSE4_1
  ASMJIT_INST_3i(dpps, Dpps, Xmm, Mem, Imm)                            // SSE4_1
  ASMJIT_INST_3i(extractps, Extractps, Gp, Xmm, Imm)                   // SSE4_1
  ASMJIT_INST_3i(extractps, Extractps, Mem, Xmm, Imm)                  // SSE4_1
  ASMJIT_INST_2x(extrq, Extrq, Xmm, Xmm)                               // SSE4A
  ASMJIT_INST_3ii(extrq, Extrq, Xmm, Imm, Imm)                         // SSE4A
  ASMJIT_INST_3i(gf2p8affineinvqb, Gf2p8affineinvqb, Xmm, Xmm, Imm)    // GFNI
  ASMJIT_INST_3i(gf2p8affineinvqb, Gf2p8affineinvqb, Xmm, Mem, Imm)    // GFNI
  ASMJIT_INST_3i(gf2p8affineqb, Gf2p8affineqb, Xmm, Xmm, Imm)          // GFNI
  ASMJIT_INST_3i(gf2p8affineqb, Gf2p8affineqb, Xmm, Mem, Imm)          // GFNI
  ASMJIT_INST_2x(gf2p8mulb, Gf2p8mulb, Xmm, Xmm)                       // GFNI
  ASMJIT_INST_2x(gf2p8mulb, Gf2p8mulb, Xmm, Mem)                       // GFNI
  ASMJIT_INST_2x(haddpd, Haddpd, Xmm, Xmm)                             // SSE3
  ASMJIT_INST_2x(haddpd, Haddpd, Xmm, Mem)                             // SSE3
  ASMJIT_INST_2x(haddps, Haddps, Xmm, Xmm)                             // SSE3
  ASMJIT_INST_2x(haddps, Haddps, Xmm, Mem)                             // SSE3
  ASMJIT_INST_2x(hsubpd, Hsubpd, Xmm, Xmm)                             // SSE3
  ASMJIT_INST_2x(hsubpd, Hsubpd, Xmm, Mem)                             // SSE3
  ASMJIT_INST_2x(hsubps, Hsubps, Xmm, Xmm)                             // SSE3
  ASMJIT_INST_2x(hsubps, Hsubps, Xmm, Mem)                             // SSE3
  ASMJIT_INST_3i(insertps, Insertps, Xmm, Xmm, Imm)                    // SSE4_1
  ASMJIT_INST_3i(insertps, Insertps, Xmm, Mem, Imm)                    // SSE4_1
  ASMJIT_INST_2x(insertq, Insertq, Xmm, Xmm)                           // SSE4A
  ASMJIT_INST_4ii(insertq, Insertq, Xmm, Xmm, Imm, Imm)                // SSE4A
  ASMJIT_INST_2x(lddqu, Lddqu, Xmm, Mem)                               // SSE3
  ASMJIT_INST_3x(maskmovq, Maskmovq, Mm, Mm, DS_ZDI)                   // SSE  [EXPLICIT]
  ASMJIT_INST_3x(maskmovdqu, Maskmovdqu, Xmm, Xmm, DS_ZDI)             // SSE2 [EXPLICIT]
  ASMJIT_INST_2x(maxpd, Maxpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(maxpd, Maxpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(maxps, Maxps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(maxps, Maxps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(maxsd, Maxsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(maxsd, Maxsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(maxss, Maxss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(maxss, Maxss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(minpd, Minpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(minpd, Minpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(minps, Minps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(minps, Minps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(minsd, Minsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(minsd, Minsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(minss, Minss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(minss, Minss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(movapd, Movapd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(movapd, Movapd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movapd, Movapd, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movaps, Movaps, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(movaps, Movaps, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(movaps, Movaps, Mem, Xmm)                             // SSE
  ASMJIT_INST_2x(movd, Movd, Mem, Mm)                                  // MMX
  ASMJIT_INST_2x(movd, Movd, Mem, Xmm)                                 // SSE
  ASMJIT_INST_2x(movd, Movd, Gp, Mm)                                   // MMX
  ASMJIT_INST_2x(movd, Movd, Gp, Xmm)                                  // SSE
  ASMJIT_INST_2x(movd, Movd, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(movd, Movd, Xmm, Mem)                                 // SSE
  ASMJIT_INST_2x(movd, Movd, Mm, Gp)                                   // MMX
  ASMJIT_INST_2x(movd, Movd, Xmm, Gp)                                  // SSE
  ASMJIT_INST_2x(movddup, Movddup, Xmm, Xmm)                           // SSE3
  ASMJIT_INST_2x(movddup, Movddup, Xmm, Mem)                           // SSE3
  ASMJIT_INST_2x(movdq2q, Movdq2q, Mm, Xmm)                            // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movdqa, Movdqa, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movdqu, Movdqu, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movhlps, Movhlps, Xmm, Xmm)                           // SSE
  ASMJIT_INST_2x(movhpd, Movhpd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movhpd, Movhpd, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movhps, Movhps, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(movhps, Movhps, Mem, Xmm)                             // SSE
  ASMJIT_INST_2x(movlhps, Movlhps, Xmm, Xmm)                           // SSE
  ASMJIT_INST_2x(movlpd, Movlpd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movlpd, Movlpd, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movlps, Movlps, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(movlps, Movlps, Mem, Xmm)                             // SSE
  ASMJIT_INST_2x(movmskps, Movmskps, Gp, Xmm)                          // SSE2
  ASMJIT_INST_2x(movmskpd, Movmskpd, Gp, Xmm)                          // SSE2
  ASMJIT_INST_2x(movntdq, Movntdq, Mem, Xmm)                           // SSE2
  ASMJIT_INST_2x(movntdqa, Movntdqa, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(movntpd, Movntpd, Mem, Xmm)                           // SSE2
  ASMJIT_INST_2x(movntps, Movntps, Mem, Xmm)                           // SSE
  ASMJIT_INST_2x(movntsd, Movntsd, Mem, Xmm)                           // SSE4A
  ASMJIT_INST_2x(movntss, Movntss, Mem, Xmm)                           // SSE4A
  ASMJIT_INST_2x(movntq, Movntq, Mem, Mm)                              // SSE
  ASMJIT_INST_2x(movq, Movq, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Xmm, Xmm)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Mem, Mm)                                  // MMX
  ASMJIT_INST_2x(movq, Movq, Mem, Xmm)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(movq, Movq, Xmm, Mem)                                 // SSE
  ASMJIT_INST_2x(movq, Movq, Gp, Mm)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Gp, Xmm)                                  // SSE+X64.
  ASMJIT_INST_2x(movq, Movq, Mm, Gp)                                   // MMX
  ASMJIT_INST_2x(movq, Movq, Xmm, Gp)                                  // SSE+X64.
  ASMJIT_INST_2x(movq2dq, Movq2dq, Xmm, Mm)                            // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(movsd, Movsd, Mem, Xmm)                               // SSE2
  ASMJIT_INST_2x(movshdup, Movshdup, Xmm, Xmm)                         // SSE3
  ASMJIT_INST_2x(movshdup, Movshdup, Xmm, Mem)                         // SSE3
  ASMJIT_INST_2x(movsldup, Movsldup, Xmm, Xmm)                         // SSE3
  ASMJIT_INST_2x(movsldup, Movsldup, Xmm, Mem)                         // SSE3
  ASMJIT_INST_2x(movss, Movss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(movss, Movss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(movss, Movss, Mem, Xmm)                               // SSE
  ASMJIT_INST_2x(movupd, Movupd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(movupd, Movupd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(movupd, Movupd, Mem, Xmm)                             // SSE2
  ASMJIT_INST_2x(movups, Movups, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(movups, Movups, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(movups, Movups, Mem, Xmm)                             // SSE
  ASMJIT_INST_3i(mpsadbw, Mpsadbw, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(mpsadbw, Mpsadbw, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_2x(mulpd, Mulpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(mulpd, Mulpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(mulps, Mulps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(mulps, Mulps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(mulsd, Mulsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(mulsd, Mulsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(mulss, Mulss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(mulss, Mulss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(orpd, Orpd, Xmm, Xmm)                                 // SSE2
  ASMJIT_INST_2x(orpd, Orpd, Xmm, Mem)                                 // SSE2
  ASMJIT_INST_2x(orps, Orps, Xmm, Xmm)                                 // SSE
  ASMJIT_INST_2x(orps, Orps, Xmm, Mem)                                 // SSE
  ASMJIT_INST_2x(packssdw, Packssdw, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packssdw, Packssdw, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packssdw, Packssdw, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(packssdw, Packssdw, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(packsswb, Packsswb, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packsswb, Packsswb, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packsswb, Packsswb, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(packsswb, Packsswb, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(packusdw, Packusdw, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(packusdw, Packusdw, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(packuswb, Packuswb, Mm, Mm)                           // MMX
  ASMJIT_INST_2x(packuswb, Packuswb, Mm, Mem)                          // MMX
  ASMJIT_INST_2x(packuswb, Packuswb, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(packuswb, Packuswb, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(pabsb, Pabsb, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Xmm, Xmm)                               // SSSE3
  ASMJIT_INST_2x(pabsb, Pabsb, Xmm, Mem)                               // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Xmm, Xmm)                               // SSSE3
  ASMJIT_INST_2x(pabsd, Pabsd, Xmm, Mem)                               // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Mm, Mm)                                 // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Mm, Mem)                                // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Xmm, Xmm)                               // SSSE3
  ASMJIT_INST_2x(pabsw, Pabsw, Xmm, Mem)                               // SSSE3
  ASMJIT_INST_2x(paddb, Paddb, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddb, Paddb, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddb, Paddb, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(paddb, Paddb, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(paddd, Paddd, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddd, Paddd, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddd, Paddd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(paddd, Paddd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Mm, Mm)                                 // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Mm, Mem)                                // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(paddq, Paddq, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(paddsb, Paddsb, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(paddsb, Paddsb, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(paddsb, Paddsb, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(paddsb, Paddsb, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(paddsw, Paddsw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(paddsw, Paddsw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(paddsw, Paddsw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(paddsw, Paddsw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(paddusb, Paddusb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(paddusb, Paddusb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(paddusb, Paddusb, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(paddusb, Paddusb, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(paddusw, Paddusw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(paddusw, Paddusw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(paddusw, Paddusw, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(paddusw, Paddusw, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(paddw, Paddw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(paddw, Paddw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(paddw, Paddw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(paddw, Paddw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_3i(palignr, Palignr, Mm, Mm, Imm)                        // SSSE3
  ASMJIT_INST_3i(palignr, Palignr, Mm, Mem, Imm)                       // SSSE3
  ASMJIT_INST_3i(palignr, Palignr, Xmm, Xmm, Imm)                      // SSSE3
  ASMJIT_INST_3i(palignr, Palignr, Xmm, Mem, Imm)                      // SSSE3
  ASMJIT_INST_2x(pand, Pand, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(pand, Pand, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(pand, Pand, Xmm, Xmm)                                 // SSE2
  ASMJIT_INST_2x(pand, Pand, Xmm, Mem)                                 // SSE2
  ASMJIT_INST_2x(pandn, Pandn, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(pandn, Pandn, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(pandn, Pandn, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(pandn, Pandn, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(pavgb, Pavgb, Mm, Mm)                                 // SSE
  ASMJIT_INST_2x(pavgb, Pavgb, Mm, Mem)                                // SSE
  ASMJIT_INST_2x(pavgb, Pavgb, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(pavgb, Pavgb, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(pavgw, Pavgw, Mm, Mm)                                 // SSE
  ASMJIT_INST_2x(pavgw, Pavgw, Mm, Mem)                                // SSE
  ASMJIT_INST_2x(pavgw, Pavgw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(pavgw, Pavgw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_3x(pblendvb, Pblendvb, Xmm, Xmm, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3x(pblendvb, Pblendvb, Xmm, Mem, XMM0)                   // SSE4_1 [EXPLICIT]
  ASMJIT_INST_3i(pblendw, Pblendw, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(pblendw, Pblendw, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3i(pclmulqdq, Pclmulqdq, Xmm, Xmm, Imm)                  // PCLMULQDQ.
  ASMJIT_INST_3i(pclmulqdq, Pclmulqdq, Xmm, Mem, Imm)                  // PCLMULQDQ.
  ASMJIT_INST_6x(pcmpestri, Pcmpestri, Xmm, Xmm, Imm, ECX, EAX, EDX)   // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestri, Pcmpestri, Xmm, Mem, Imm, ECX, EAX, EDX)   // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestrm, Pcmpestrm, Xmm, Xmm, Imm, XMM0, EAX, EDX)  // SSE4_2 [EXPLICIT]
  ASMJIT_INST_6x(pcmpestrm, Pcmpestrm, Xmm, Mem, Imm, XMM0, EAX, EDX)  // SSE4_2 [EXPLICIT]
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpeqb, Pcmpeqb, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpeqd, Pcmpeqd, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpeqq, Pcmpeqq, Xmm, Xmm)                           // SSE4_1
  ASMJIT_INST_2x(pcmpeqq, Pcmpeqq, Xmm, Mem)                           // SSE4_1
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpeqw, Pcmpeqw, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpgtb, Pcmpgtb, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpgtd, Pcmpgtd, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pcmpgtq, Pcmpgtq, Xmm, Xmm)                           // SSE4_2.
  ASMJIT_INST_2x(pcmpgtq, Pcmpgtq, Xmm, Mem)                           // SSE4_2.
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pcmpgtw, Pcmpgtw, Xmm, Mem)                           // SSE2
  ASMJIT_INST_4x(pcmpistri, Pcmpistri, Xmm, Xmm, Imm, ECX)             // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistri, Pcmpistri, Xmm, Mem, Imm, ECX)             // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistrm, Pcmpistrm, Xmm, Xmm, Imm, XMM0)            // SSE4_2 [EXPLICIT]
  ASMJIT_INST_4x(pcmpistrm, Pcmpistrm, Xmm, Mem, Imm, XMM0)            // SSE4_2 [EXPLICIT]
  ASMJIT_INST_3i(pextrb, Pextrb, Gp, Xmm, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pextrb, Pextrb, Mem, Xmm, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pextrd, Pextrd, Gp, Xmm, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pextrd, Pextrd, Mem, Xmm, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pextrq, Pextrq, Gp, Xmm, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pextrq, Pextrq, Mem, Xmm, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pextrw, Pextrw, Gp, Mm, Imm)                          // SSE
  ASMJIT_INST_3i(pextrw, Pextrw, Gp, Xmm, Imm)                         // SSE2
  ASMJIT_INST_3i(pextrw, Pextrw, Mem, Xmm, Imm)                        // SSE4_1
  ASMJIT_INST_2x(phaddd, Phaddd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(phaddd, Phaddd, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Mm, Mm)                             // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Mm, Mem)                            // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Xmm, Xmm)                           // SSSE3
  ASMJIT_INST_2x(phaddsw, Phaddsw, Xmm, Mem)                           // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(phaddw, Phaddw, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(phminposuw, Phminposuw, Xmm, Xmm)                     // SSE4_1
  ASMJIT_INST_2x(phminposuw, Phminposuw, Xmm, Mem)                     // SSE4_1
  ASMJIT_INST_2x(phsubd, Phsubd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(phsubd, Phsubd, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Mm, Mm)                             // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Mm, Mem)                            // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Xmm, Xmm)                           // SSSE3
  ASMJIT_INST_2x(phsubsw, Phsubsw, Xmm, Mem)                           // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(phsubw, Phsubw, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_3i(pinsrb, Pinsrb, Xmm, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pinsrb, Pinsrb, Xmm, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pinsrd, Pinsrd, Xmm, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pinsrd, Pinsrd, Xmm, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pinsrq, Pinsrq, Xmm, Gp, Imm)                         // SSE4_1
  ASMJIT_INST_3i(pinsrq, Pinsrq, Xmm, Mem, Imm)                        // SSE4_1
  ASMJIT_INST_3i(pinsrw, Pinsrw, Mm, Gp, Imm)                          // SSE
  ASMJIT_INST_3i(pinsrw, Pinsrw, Mm, Mem, Imm)                         // SSE
  ASMJIT_INST_3i(pinsrw, Pinsrw, Xmm, Gp, Imm)                         // SSE2
  ASMJIT_INST_3i(pinsrw, Pinsrw, Xmm, Mem, Imm)                        // SSE2
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Mm, Mm)                         // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Mm, Mem)                        // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Xmm, Xmm)                       // SSSE3
  ASMJIT_INST_2x(pmaddubsw, Pmaddubsw, Xmm, Mem)                       // SSSE3
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pmaddwd, Pmaddwd, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pmaxsb, Pmaxsb, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsb, Pmaxsb, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsd, Pmaxsd, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsd, Pmaxsd, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pmaxsw, Pmaxsw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pmaxub, Pmaxub, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pmaxub, Pmaxub, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pmaxub, Pmaxub, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pmaxub, Pmaxub, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pmaxud, Pmaxud, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmaxud, Pmaxud, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmaxuw, Pmaxuw, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmaxuw, Pmaxuw, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsb, Pminsb, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pminsb, Pminsb, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsd, Pminsd, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pminsd, Pminsd, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminsw, Pminsw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pminsw, Pminsw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pminsw, Pminsw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pminsw, Pminsw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pminub, Pminub, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(pminub, Pminub, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(pminub, Pminub, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pminub, Pminub, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pminud, Pminud, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pminud, Pminud, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pminuw, Pminuw, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pminuw, Pminuw, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmovmskb, Pmovmskb, Gp, Mm)                           // SSE
  ASMJIT_INST_2x(pmovmskb, Pmovmskb, Gp, Xmm)                          // SSE2
  ASMJIT_INST_2x(pmovsxbd, Pmovsxbd, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbd, Pmovsxbd, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbq, Pmovsxbq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbq, Pmovsxbq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbw, Pmovsxbw, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxbw, Pmovsxbw, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxdq, Pmovsxdq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxdq, Pmovsxdq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwd, Pmovsxwd, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwd, Pmovsxwd, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwq, Pmovsxwq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovsxwq, Pmovsxwq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbd, Pmovzxbd, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbd, Pmovzxbd, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbq, Pmovzxbq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbq, Pmovzxbq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbw, Pmovzxbw, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxbw, Pmovzxbw, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxdq, Pmovzxdq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxdq, Pmovzxdq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwd, Pmovzxwd, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwd, Pmovzxwd, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwq, Pmovzxwq, Xmm, Xmm)                         // SSE4_1
  ASMJIT_INST_2x(pmovzxwq, Pmovzxwq, Xmm, Mem)                         // SSE4_1
  ASMJIT_INST_2x(pmuldq, Pmuldq, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmuldq, Pmuldq, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Mm, Mm)                           // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Mm, Mem)                          // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Xmm, Xmm)                         // SSSE3
  ASMJIT_INST_2x(pmulhrsw, Pmulhrsw, Xmm, Mem)                         // SSSE3
  ASMJIT_INST_2x(pmulhw, Pmulhw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(pmulhw, Pmulhw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(pmulhw, Pmulhw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pmulhw, Pmulhw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Mm, Mm)                             // SSE
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Mm, Mem)                            // SSE
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pmulhuw, Pmulhuw, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(pmulld, Pmulld, Xmm, Xmm)                             // SSE4_1
  ASMJIT_INST_2x(pmulld, Pmulld, Xmm, Mem)                             // SSE4_1
  ASMJIT_INST_2x(pmullw, Pmullw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(pmullw, Pmullw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(pmullw, Pmullw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(pmullw, Pmullw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Mm, Mm)                             // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Mm, Mem)                            // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(pmuludq, Pmuludq, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(por, Por, Mm, Mm)                                     // MMX
  ASMJIT_INST_2x(por, Por, Mm, Mem)                                    // MMX
  ASMJIT_INST_2x(por, Por, Xmm, Xmm)                                   // SSE2
  ASMJIT_INST_2x(por, Por, Xmm, Mem)                                   // SSE2
  ASMJIT_INST_2x(psadbw, Psadbw, Mm, Mm)                               // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Mm, Mem)                              // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(psadbw, Psadbw, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(pslld, Pslld, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(pslld, Pslld, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(pslld, Pslld, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(pslld, Pslld, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(pslld, Pslld, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(pslld, Pslld, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2i(pslldq, Pslldq, Xmm, Imm)                             // SSE2
  ASMJIT_INST_2x(psllq, Psllq, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psllq, Psllq, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psllq, Psllq, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psllq, Psllq, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psllq, Psllq, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psllq, Psllq, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(psllw, Psllw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psllw, Psllw, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psllw, Psllw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psllw, Psllw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psllw, Psllw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psllw, Psllw, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(psrad, Psrad, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrad, Psrad, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psrad, Psrad, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrad, Psrad, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psrad, Psrad, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psrad, Psrad, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(psraw, Psraw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psraw, Psraw, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psraw, Psraw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psraw, Psraw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psraw, Psraw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psraw, Psraw, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(pshufb, Pshufb, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(pshufb, Pshufb, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_3i(pshufd, Pshufd, Xmm, Xmm, Imm)                        // SSE2
  ASMJIT_INST_3i(pshufd, Pshufd, Xmm, Mem, Imm)                        // SSE2
  ASMJIT_INST_3i(pshufhw, Pshufhw, Xmm, Xmm, Imm)                      // SSE2
  ASMJIT_INST_3i(pshufhw, Pshufhw, Xmm, Mem, Imm)                      // SSE2
  ASMJIT_INST_3i(pshuflw, Pshuflw, Xmm, Xmm, Imm)                      // SSE2
  ASMJIT_INST_3i(pshuflw, Pshuflw, Xmm, Mem, Imm)                      // SSE2
  ASMJIT_INST_3i(pshufw, Pshufw, Mm, Mm, Imm)                          // SSE
  ASMJIT_INST_3i(pshufw, Pshufw, Mm, Mem, Imm)                         // SSE
  ASMJIT_INST_2x(psignb, Psignb, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(psignb, Psignb, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(psignd, Psignd, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Mm, Mm)                               // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Mm, Mem)                              // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Xmm, Xmm)                             // SSSE3
  ASMJIT_INST_2x(psignw, Psignw, Xmm, Mem)                             // SSSE3
  ASMJIT_INST_2x(psrld, Psrld, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrld, Psrld, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psrld, Psrld, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrld, Psrld, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psrld, Psrld, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psrld, Psrld, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2i(psrldq, Psrldq, Xmm, Imm)                             // SSE2
  ASMJIT_INST_2x(psrlq, Psrlq, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrlq, Psrlq, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psrlq, Psrlq, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrlq, Psrlq, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psrlq, Psrlq, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psrlq, Psrlq, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(psrlw, Psrlw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psrlw, Psrlw, Mm, Mem)                                // MMX
  ASMJIT_INST_2i(psrlw, Psrlw, Mm, Imm)                                // MMX
  ASMJIT_INST_2x(psrlw, Psrlw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psrlw, Psrlw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2i(psrlw, Psrlw, Xmm, Imm)                               // SSE2
  ASMJIT_INST_2x(psubb, Psubb, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubb, Psubb, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubb, Psubb, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psubb, Psubb, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(psubd, Psubd, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubd, Psubd, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubd, Psubd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psubd, Psubd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Mm, Mm)                                 // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Mm, Mem)                                // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psubq, Psubq, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(psubsb, Psubsb, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(psubsb, Psubsb, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(psubsb, Psubsb, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(psubsb, Psubsb, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(psubsw, Psubsw, Mm, Mm)                               // MMX
  ASMJIT_INST_2x(psubsw, Psubsw, Mm, Mem)                              // MMX
  ASMJIT_INST_2x(psubsw, Psubsw, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(psubsw, Psubsw, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(psubusb, Psubusb, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(psubusb, Psubusb, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(psubusb, Psubusb, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(psubusb, Psubusb, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(psubusw, Psubusw, Mm, Mm)                             // MMX
  ASMJIT_INST_2x(psubusw, Psubusw, Mm, Mem)                            // MMX
  ASMJIT_INST_2x(psubusw, Psubusw, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(psubusw, Psubusw, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(psubw, Psubw, Mm, Mm)                                 // MMX
  ASMJIT_INST_2x(psubw, Psubw, Mm, Mem)                                // MMX
  ASMJIT_INST_2x(psubw, Psubw, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(psubw, Psubw, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(ptest, Ptest, Xmm, Xmm)                               // SSE4_1
  ASMJIT_INST_2x(ptest, Ptest, Xmm, Mem)                               // SSE4_1
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpckhbw, Punpckhbw, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpckhdq, Punpckhdq, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckhqdq, Punpckhqdq, Xmm, Xmm)                     // SSE2
  ASMJIT_INST_2x(punpckhqdq, Punpckhqdq, Xmm, Mem)                     // SSE2
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpckhwd, Punpckhwd, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpcklbw, Punpcklbw, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(punpckldq, Punpckldq, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpckldq, Punpckldq, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpckldq, Punpckldq, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpckldq, Punpckldq, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(punpcklqdq, Punpcklqdq, Xmm, Xmm)                     // SSE2
  ASMJIT_INST_2x(punpcklqdq, Punpcklqdq, Xmm, Mem)                     // SSE2
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Mm, Mm)                         // MMX
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Mm, Mem)                        // MMX
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Xmm, Xmm)                       // SSE2
  ASMJIT_INST_2x(punpcklwd, Punpcklwd, Xmm, Mem)                       // SSE2
  ASMJIT_INST_2x(pxor, Pxor, Mm, Mm)                                   // MMX
  ASMJIT_INST_2x(pxor, Pxor, Mm, Mem)                                  // MMX
  ASMJIT_INST_2x(pxor, Pxor, Xmm, Xmm)                                 // SSE2
  ASMJIT_INST_2x(pxor, Pxor, Xmm, Mem)                                 // SSE2
  ASMJIT_INST_2x(rcpps, Rcpps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(rcpps, Rcpps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(rcpss, Rcpss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(rcpss, Rcpss, Xmm, Mem)                               // SSE
  ASMJIT_INST_3i(roundpd, Roundpd, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundpd, Roundpd, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundps, Roundps, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundps, Roundps, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundsd, Roundsd, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundsd, Roundsd, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundss, Roundss, Xmm, Xmm, Imm)                      // SSE4_1
  ASMJIT_INST_3i(roundss, Roundss, Xmm, Mem, Imm)                      // SSE4_1
  ASMJIT_INST_2x(rsqrtps, Rsqrtps, Xmm, Xmm)                           // SSE
  ASMJIT_INST_2x(rsqrtps, Rsqrtps, Xmm, Mem)                           // SSE
  ASMJIT_INST_2x(rsqrtss, Rsqrtss, Xmm, Xmm)                           // SSE
  ASMJIT_INST_2x(rsqrtss, Rsqrtss, Xmm, Mem)                           // SSE
  ASMJIT_INST_3i(shufpd, Shufpd, Xmm, Xmm, Imm)                        // SSE2
  ASMJIT_INST_3i(shufpd, Shufpd, Xmm, Mem, Imm)                        // SSE2
  ASMJIT_INST_3i(shufps, Shufps, Xmm, Xmm, Imm)                        // SSE
  ASMJIT_INST_3i(shufps, Shufps, Xmm, Mem, Imm)                        // SSE
  ASMJIT_INST_2x(sqrtpd, Sqrtpd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(sqrtpd, Sqrtpd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(sqrtps, Sqrtps, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(sqrtps, Sqrtps, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(sqrtsd, Sqrtsd, Xmm, Xmm)                             // SSE2
  ASMJIT_INST_2x(sqrtsd, Sqrtsd, Xmm, Mem)                             // SSE2
  ASMJIT_INST_2x(sqrtss, Sqrtss, Xmm, Xmm)                             // SSE
  ASMJIT_INST_2x(sqrtss, Sqrtss, Xmm, Mem)                             // SSE
  ASMJIT_INST_2x(subpd, Subpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(subpd, Subpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(subps, Subps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(subps, Subps, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(subsd, Subsd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(subsd, Subsd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(subss, Subss, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(subss, Subss, Xmm, Mem)                               // SSE
  ASMJIT_INST_2x(ucomisd, Ucomisd, Xmm, Xmm)                           // SSE2
  ASMJIT_INST_2x(ucomisd, Ucomisd, Xmm, Mem)                           // SSE2
  ASMJIT_INST_2x(ucomiss, Ucomiss, Xmm, Xmm)                           // SSE
  ASMJIT_INST_2x(ucomiss, Ucomiss, Xmm, Mem)                           // SSE
  ASMJIT_INST_2x(unpckhpd, Unpckhpd, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(unpckhpd, Unpckhpd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(unpckhps, Unpckhps, Xmm, Xmm)                         // SSE
  ASMJIT_INST_2x(unpckhps, Unpckhps, Xmm, Mem)                         // SSE
  ASMJIT_INST_2x(unpcklpd, Unpcklpd, Xmm, Xmm)                         // SSE2
  ASMJIT_INST_2x(unpcklpd, Unpcklpd, Xmm, Mem)                         // SSE2
  ASMJIT_INST_2x(unpcklps, Unpcklps, Xmm, Xmm)                         // SSE
  ASMJIT_INST_2x(unpcklps, Unpcklps, Xmm, Mem)                         // SSE
  ASMJIT_INST_2x(xorpd, Xorpd, Xmm, Xmm)                               // SSE2
  ASMJIT_INST_2x(xorpd, Xorpd, Xmm, Mem)                               // SSE2
  ASMJIT_INST_2x(xorps, Xorps, Xmm, Xmm)                               // SSE
  ASMJIT_INST_2x(xorps, Xorps, Xmm, Mem)                               // SSE

  //! \}

  //! \name 3DNOW and GEODE Instructions (Deprecated)
  //! \{

  ASMJIT_INST_2x(pavgusb, Pavgusb, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pavgusb, Pavgusb, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pf2id, Pf2id, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pf2id, Pf2id, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pf2iw, Pf2iw, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pf2iw, Pf2iw, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfacc, Pfacc, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfacc, Pfacc, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfadd, Pfadd, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfadd, Pfadd, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfcmpeq, Pfcmpeq, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpeq, Pfcmpeq, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfcmpge, Pfcmpge, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpge, Pfcmpge, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfcmpgt, Pfcmpgt, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfcmpgt, Pfcmpgt, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfmax, Pfmax, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmax, Pfmax, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfmin, Pfmin, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmin, Pfmin, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfmul, Pfmul, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfmul, Pfmul, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfnacc, Pfnacc, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pfnacc, Pfnacc, Mm, Mem)                              // 3DNOW
  ASMJIT_INST_2x(pfpnacc, Pfpnacc, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfpnacc, Pfpnacc, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfrcp, Pfrcp, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfrcp, Pfrcp, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfrcpit1, Pfrcpit1, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrcpit1, Pfrcpit1, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrcpit2, Pfrcpit2, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrcpit2, Pfrcpit2, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrcpv, Pfrcpv, Mm, Mm)                               // GEODE
  ASMJIT_INST_2x(pfrcpv, Pfrcpv, Mm, Mem)                              // GEODE
  ASMJIT_INST_2x(pfrsqit1, Pfrsqit1, Mm, Mm)                           // 3DNOW
  ASMJIT_INST_2x(pfrsqit1, Pfrsqit1, Mm, Mem)                          // 3DNOW
  ASMJIT_INST_2x(pfrsqrt, Pfrsqrt, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pfrsqrt, Pfrsqrt, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pfrsqrtv, Pfrsqrtv, Mm, Mm)                           // GEODE
  ASMJIT_INST_2x(pfrsqrtv, Pfrsqrtv, Mm, Mem)                          // GEODE
  ASMJIT_INST_2x(pfsub, Pfsub, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pfsub, Pfsub, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pfsubr, Pfsubr, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pfsubr, Pfsubr, Mm, Mem)                              // 3DNOW
  ASMJIT_INST_2x(pi2fd, Pi2fd, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pi2fd, Pi2fd, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pi2fw, Pi2fw, Mm, Mm)                                 // 3DNOW
  ASMJIT_INST_2x(pi2fw, Pi2fw, Mm, Mem)                                // 3DNOW
  ASMJIT_INST_2x(pmulhrw, Pmulhrw, Mm, Mm)                             // 3DNOW
  ASMJIT_INST_2x(pmulhrw, Pmulhrw, Mm, Mem)                            // 3DNOW
  ASMJIT_INST_2x(pswapd, Pswapd, Mm, Mm)                               // 3DNOW
  ASMJIT_INST_2x(pswapd, Pswapd, Mm, Mem)                              // 3DNOW
  ASMJIT_INST_0x(femms, Femms)                                         // 3DNOW

  //! \}

  //! \name AESNI Instructions
  //! \{

  ASMJIT_INST_2x(aesdec, Aesdec, Xmm, Xmm)                             // AESNI
  ASMJIT_INST_2x(aesdec, Aesdec, Xmm, Mem)                             // AESNI
  ASMJIT_INST_2x(aesdeclast, Aesdeclast, Xmm, Xmm)                     // AESNI
  ASMJIT_INST_2x(aesdeclast, Aesdeclast, Xmm, Mem)                     // AESNI
  ASMJIT_INST_2x(aesenc, Aesenc, Xmm, Xmm)                             // AESNI
  ASMJIT_INST_2x(aesenc, Aesenc, Xmm, Mem)                             // AESNI
  ASMJIT_INST_2x(aesenclast, Aesenclast, Xmm, Xmm)                     // AESNI
  ASMJIT_INST_2x(aesenclast, Aesenclast, Xmm, Mem)                     // AESNI
  ASMJIT_INST_2x(aesimc, Aesimc, Xmm, Xmm)                             // AESNI
  ASMJIT_INST_2x(aesimc, Aesimc, Xmm, Mem)                             // AESNI
  ASMJIT_INST_3i(aeskeygenassist, Aeskeygenassist, Xmm, Xmm, Imm)      // AESNI
  ASMJIT_INST_3i(aeskeygenassist, Aeskeygenassist, Xmm, Mem, Imm)      // AESNI

  //! \}

  //! \name SHA Instructions
  //! \{

  ASMJIT_INST_2x(sha1msg1, Sha1msg1, Xmm, Xmm)                         // SHA
  ASMJIT_INST_2x(sha1msg1, Sha1msg1, Xmm, Mem)                         // SHA
  ASMJIT_INST_2x(sha1msg2, Sha1msg2, Xmm, Xmm)                         // SHA
  ASMJIT_INST_2x(sha1msg2, Sha1msg2, Xmm, Mem)                         // SHA
  ASMJIT_INST_2x(sha1nexte, Sha1nexte, Xmm, Xmm)                       // SHA
  ASMJIT_INST_2x(sha1nexte, Sha1nexte, Xmm, Mem)                       // SHA
  ASMJIT_INST_3i(sha1rnds4, Sha1rnds4, Xmm, Xmm, Imm)                  // SHA
  ASMJIT_INST_3i(sha1rnds4, Sha1rnds4, Xmm, Mem, Imm)                  // SHA
  ASMJIT_INST_2x(sha256msg1, Sha256msg1, Xmm, Xmm)                     // SHA
  ASMJIT_INST_2x(sha256msg1, Sha256msg1, Xmm, Mem)                     // SHA
  ASMJIT_INST_2x(sha256msg2, Sha256msg2, Xmm, Xmm)                     // SHA
  ASMJIT_INST_2x(sha256msg2, Sha256msg2, Xmm, Mem)                     // SHA
  ASMJIT_INST_3x(sha256rnds2, Sha256rnds2, Xmm, Xmm, XMM0)             // SHA [EXPLICIT]
  ASMJIT_INST_3x(sha256rnds2, Sha256rnds2, Xmm, Mem, XMM0)             // SHA [EXPLICIT]

  //! \}

  //! \name AVX, FMA, and AVX512 Instructions
  //! \{

  ASMJIT_INST_3x(kaddb, Kaddb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kaddd, Kaddd, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kaddq, Kaddq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kaddw, Kaddw, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kandb, Kandb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kandd, Kandd, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kandnb, Kandnb, KReg, KReg, KReg)                     // AVX512_DQ
  ASMJIT_INST_3x(kandnd, Kandnd, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kandnq, Kandnq, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kandnw, Kandnw, KReg, KReg, KReg)                     // AVX512_F
  ASMJIT_INST_3x(kandq, Kandq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kandw, Kandw, KReg, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, KReg)                             // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, Mem)                              // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, KReg, Gp)                               // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, Mem, KReg)                              // AVX512_DQ
  ASMJIT_INST_2x(kmovb, Kmovb, Gp, KReg)                               // AVX512_DQ
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, Mem)                              // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, KReg, Gp)                               // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, Mem, KReg)                              // AVX512_BW
  ASMJIT_INST_2x(kmovd, Kmovd, Gp, KReg)                               // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, Mem)                              // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, KReg, Gp)                               // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, Mem, KReg)                              // AVX512_BW
  ASMJIT_INST_2x(kmovq, Kmovq, Gp, KReg)                               // AVX512_BW
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, KReg)                             // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, Mem)                              // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, KReg, Gp)                               // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, Mem, KReg)                              // AVX512_F
  ASMJIT_INST_2x(kmovw, Kmovw, Gp, KReg)                               // AVX512_F
  ASMJIT_INST_2x(knotb, Knotb, KReg, KReg)                             // AVX512_DQ
  ASMJIT_INST_2x(knotd, Knotd, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(knotq, Knotq, KReg, KReg)                             // AVX512_BW
  ASMJIT_INST_2x(knotw, Knotw, KReg, KReg)                             // AVX512_F
  ASMJIT_INST_3x(korb, Korb, KReg, KReg, KReg)                         // AVX512_DQ
  ASMJIT_INST_3x(kord, Kord, KReg, KReg, KReg)                         // AVX512_BW
  ASMJIT_INST_3x(korq, Korq, KReg, KReg, KReg)                         // AVX512_BW
  ASMJIT_INST_2x(kortestb, Kortestb, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_2x(kortestd, Kortestd, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_2x(kortestq, Kortestq, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_2x(kortestw, Kortestw, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_3x(korw, Korw, KReg, KReg, KReg)                         // AVX512_F
  ASMJIT_INST_3i(kshiftlb, Kshiftlb, KReg, KReg, Imm)                  // AVX512_DQ
  ASMJIT_INST_3i(kshiftld, Kshiftld, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3i(kshiftlq, Kshiftlq, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3i(kshiftlw, Kshiftlw, KReg, KReg, Imm)                  // AVX512_F
  ASMJIT_INST_3i(kshiftrb, Kshiftrb, KReg, KReg, Imm)                  // AVX512_DQ
  ASMJIT_INST_3i(kshiftrd, Kshiftrd, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3i(kshiftrq, Kshiftrq, KReg, KReg, Imm)                  // AVX512_BW
  ASMJIT_INST_3i(kshiftrw, Kshiftrw, KReg, KReg, Imm)                  // AVX512_F
  ASMJIT_INST_2x(ktestb, Ktestb, KReg, KReg)                           // AVX512_DQ
  ASMJIT_INST_2x(ktestd, Ktestd, KReg, KReg)                           // AVX512_BW
  ASMJIT_INST_2x(ktestq, Ktestq, KReg, KReg)                           // AVX512_BW
  ASMJIT_INST_2x(ktestw, Ktestw, KReg, KReg)                           // AVX512_DQ
  ASMJIT_INST_3x(kunpckbw, Kunpckbw, KReg, KReg, KReg)                 // AVX512_F
  ASMJIT_INST_3x(kunpckdq, Kunpckdq, KReg, KReg, KReg)                 // AVX512_BW
  ASMJIT_INST_3x(kunpckwd, Kunpckwd, KReg, KReg, KReg)                 // AVX512_BW
  ASMJIT_INST_3x(kxnorb, Kxnorb, KReg, KReg, KReg)                     // AVX512_DQ
  ASMJIT_INST_3x(kxnord, Kxnord, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kxnorq, Kxnorq, KReg, KReg, KReg)                     // AVX512_BW
  ASMJIT_INST_3x(kxnorw, Kxnorw, KReg, KReg, KReg)                     // AVX512_F
  ASMJIT_INST_3x(kxorb, Kxorb, KReg, KReg, KReg)                       // AVX512_DQ
  ASMJIT_INST_3x(kxord, Kxord, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kxorq, Kxorq, KReg, KReg, KReg)                       // AVX512_BW
  ASMJIT_INST_3x(kxorw, Kxorw, KReg, KReg, KReg)                       // AVX512_F
  ASMJIT_INST_6x(v4fmaddps, V4fmaddps, Zmm, Zmm, Zmm, Zmm, Zmm, Mem)   // AVX512_4FMAPS{kz}
  ASMJIT_INST_6x(v4fmaddss, V4fmaddss, Xmm, Xmm, Xmm, Xmm, Xmm, Mem)   // AVX512_4FMAPS{kz}
  ASMJIT_INST_6x(v4fnmaddps, V4fnmaddps, Zmm, Zmm, Zmm, Zmm, Zmm, Mem) // AVX512_4FMAPS{kz}
  ASMJIT_INST_6x(v4fnmaddss, V4fnmaddss, Xmm, Xmm, Xmm, Xmm, Xmm, Mem) // AVX512_4FMAPS{kz}
  ASMJIT_INST_3x(vaddpd, Vaddpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vaddpd, Vaddpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vaddpd, Vaddpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vaddpd, Vaddpd, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vaddpd, Vaddpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vaddpd, Vaddpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vaddps, Vaddps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vaddps, Vaddps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vaddps, Vaddps, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vaddps, Vaddps, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vaddps, Vaddps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vaddps, Vaddps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vaddsd, Vaddsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddsd, Vaddsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddss, Vaddss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddss, Vaddss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Xmm, Xmm, Xmm)                  // AVX
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Xmm, Xmm, Mem)                  // AVX
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Ymm, Ymm, Ymm)                  // AVX
  ASMJIT_INST_3x(vaddsubpd, Vaddsubpd, Ymm, Ymm, Mem)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Xmm, Xmm, Xmm)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Xmm, Xmm, Mem)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Ymm, Ymm, Ymm)                  // AVX
  ASMJIT_INST_3x(vaddsubps, Vaddsubps, Ymm, Ymm, Mem)                  // AVX
  ASMJIT_INST_3x(vaesdec, Vaesdec, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vaesdec, Vaesdec, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vaesdec, Vaesdec, Ymm, Ymm, Ymm)                      // VAES AVX512_VL
  ASMJIT_INST_3x(vaesdec, Vaesdec, Ymm, Ymm, Mem)                      // VAES AVX512_VL
  ASMJIT_INST_3x(vaesdec, Vaesdec, Zmm, Zmm, Zmm)                      // VAES
  ASMJIT_INST_3x(vaesdec, Vaesdec, Zmm, Zmm, Mem)                      // VAES
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Xmm, Xmm, Xmm)              // AVX
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Xmm, Xmm, Mem)              // AVX
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Ymm, Ymm, Ymm)              // VAES AVX512_VL
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Ymm, Ymm, Mem)              // VAES AVX512_VL
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Zmm, Zmm, Zmm)              // VAES
  ASMJIT_INST_3x(vaesdeclast, Vaesdeclast, Zmm, Zmm, Mem)              // VAES
  ASMJIT_INST_3x(vaesenc, Vaesenc, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vaesenc, Vaesenc, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vaesenc, Vaesenc, Ymm, Ymm, Ymm)                      // VAES AVX512_VL
  ASMJIT_INST_3x(vaesenc, Vaesenc, Ymm, Ymm, Mem)                      // VAES AVX512_VL
  ASMJIT_INST_3x(vaesenc, Vaesenc, Zmm, Zmm, Zmm)                      // VAES
  ASMJIT_INST_3x(vaesenc, Vaesenc, Zmm, Zmm, Mem)                      // VAES
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Xmm, Xmm, Xmm)              // AVX
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Xmm, Xmm, Mem)              // AVX
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Ymm, Ymm, Ymm)              // VAES AVX512_VL
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Ymm, Ymm, Mem)              // VAES AVX512_VL
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Zmm, Zmm, Zmm)              // VAES
  ASMJIT_INST_3x(vaesenclast, Vaesenclast, Zmm, Zmm, Mem)              // VAES
  ASMJIT_INST_2x(vaesimc, Vaesimc, Xmm, Xmm)                           // AVX
  ASMJIT_INST_2x(vaesimc, Vaesimc, Xmm, Mem)                           // AVX
  ASMJIT_INST_3i(vaeskeygenassist, Vaeskeygenassist, Xmm, Xmm, Imm)    // AVX
  ASMJIT_INST_3i(vaeskeygenassist, Vaeskeygenassist, Xmm, Mem, Imm)    // AVX
  ASMJIT_INST_4i(valignd, Valignd, Xmm, Xmm, Xmm, Imm)                 //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(valignd, Valignd, Xmm, Xmm, Mem, Imm)                 //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(valignd, Valignd, Ymm, Ymm, Ymm, Imm)                 //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(valignd, Valignd, Ymm, Ymm, Mem, Imm)                 //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(valignd, Valignd, Zmm, Zmm, Zmm, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(valignd, Valignd, Zmm, Zmm, Mem, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(valignq, Valignq, Xmm, Xmm, Xmm, Imm)                 //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(valignq, Valignq, Xmm, Xmm, Mem, Imm)                 //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(valignq, Valignq, Ymm, Ymm, Ymm, Imm)                 //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(valignq, Valignq, Ymm, Ymm, Mem, Imm)                 //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(valignq, Valignq, Zmm, Zmm, Zmm, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(valignq, Valignq, Zmm, Zmm, Mem, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vandnpd, Vandnpd, Xmm, Xmm, Xmm)                      // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandnpd, Vandnpd, Xmm, Xmm, Mem)                      // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandnpd, Vandnpd, Ymm, Ymm, Ymm)                      // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandnpd, Vandnpd, Ymm, Ymm, Mem)                      // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandnpd, Vandnpd, Zmm, Zmm, Zmm)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandnpd, Vandnpd, Zmm, Zmm, Mem)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandnps, Vandnps, Xmm, Xmm, Xmm)                      // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandnps, Vandnps, Xmm, Xmm, Mem)                      // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandnps, Vandnps, Ymm, Ymm, Ymm)                      // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandnps, Vandnps, Ymm, Ymm, Mem)                      // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandnps, Vandnps, Zmm, Zmm, Zmm)                      //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandnps, Vandnps, Zmm, Zmm, Mem)                      //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandpd, Vandpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandpd, Vandpd, Xmm, Xmm, Mem)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandpd, Vandpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandpd, Vandpd, Ymm, Ymm, Mem)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vandpd, Vandpd, Zmm, Zmm, Zmm)                        //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandpd, Vandpd, Zmm, Zmm, Mem)                        //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vandps, Vandps, Xmm, Xmm, Xmm)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandps, Vandps, Xmm, Xmm, Mem)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandps, Vandps, Ymm, Ymm, Ymm)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandps, Vandps, Ymm, Ymm, Mem)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vandps, Vandps, Zmm, Zmm, Zmm)                        //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vandps, Vandps, Zmm, Zmm, Mem)                        //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vblendmb, Vblendmb, Xmm, Xmm, Xmm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmb, Vblendmb, Xmm, Xmm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmb, Vblendmb, Ymm, Ymm, Ymm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmb, Vblendmb, Ymm, Ymm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmb, Vblendmb, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vblendmb, Vblendmb, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vblendmd, Vblendmd, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmd, Vblendmd, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmd, Vblendmd, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmd, Vblendmd, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmd, Vblendmd, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vblendmd, Vblendmd, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmpd, Vblendmpd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmps, Vblendmps, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmps, Vblendmps, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmps, Vblendmps, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmps, Vblendmps, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vblendmps, Vblendmps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vblendmps, Vblendmps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vblendmq, Vblendmq, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmq, Vblendmq, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmq, Vblendmq, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmq, Vblendmq, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vblendmq, Vblendmq, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmq, Vblendmq, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vblendmw, Vblendmw, Xmm, Xmm, Xmm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmw, Vblendmw, Xmm, Xmm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmw, Vblendmw, Ymm, Ymm, Ymm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmw, Vblendmw, Ymm, Ymm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vblendmw, Vblendmw, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vblendmw, Vblendmw, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_4i(vblendpd, Vblendpd, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vblendpd, Vblendpd, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vblendpd, Vblendpd, Ymm, Ymm, Ymm, Imm)               // AVX
  ASMJIT_INST_4i(vblendpd, Vblendpd, Ymm, Ymm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vblendps, Vblendps, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vblendps, Vblendps, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vblendps, Vblendps, Ymm, Ymm, Ymm, Imm)               // AVX
  ASMJIT_INST_4i(vblendps, Vblendps, Ymm, Ymm, Mem, Imm)               // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Xmm, Xmm, Xmm, Xmm)             // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Xmm, Xmm, Mem, Xmm)             // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Ymm, Ymm, Ymm, Ymm)             // AVX
  ASMJIT_INST_4x(vblendvpd, Vblendvpd, Ymm, Ymm, Mem, Ymm)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Xmm, Xmm, Xmm, Xmm)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Xmm, Xmm, Mem, Xmm)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Ymm, Ymm, Ymm, Ymm)             // AVX
  ASMJIT_INST_4x(vblendvps, Vblendvps, Ymm, Ymm, Mem, Ymm)             // AVX
  ASMJIT_INST_2x(vbroadcastf128, Vbroadcastf128, Ymm, Mem)             // AVX
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Ymm, Xmm)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Ymm, Mem)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Zmm, Xmm)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf32x2, Vbroadcastf32x2, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf32x4, Vbroadcastf32x4, Ymm, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastf32x4, Vbroadcastf32x4, Zmm, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastf32x8, Vbroadcastf32x8, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf64x2, Vbroadcastf64x2, Ymm, Mem)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcastf64x2, Vbroadcastf64x2, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcastf64x4, Vbroadcastf64x4, Zmm, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti128, Vbroadcasti128, Ymm, Mem)             // AVX2
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Xmm, Xmm)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Xmm, Mem)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Ymm, Xmm)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Ymm, Mem)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Zmm, Xmm)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti32x2, Vbroadcasti32x2, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti32x4, Vbroadcasti32x4, Ymm, Mem)           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcasti32x4, Vbroadcasti32x4, Zmm, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti32x8, Vbroadcasti32x8, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Ymm, Xmm)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Ymm, Mem)           //      AVX512_DQ{kz}-VL
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Zmm, Xmm)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x2, Vbroadcasti64x2, Zmm, Mem)           //      AVX512_DQ{kz}
  ASMJIT_INST_2x(vbroadcasti64x4, Vbroadcasti64x4, Zmm, Xmm)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcasti64x4, Vbroadcasti64x4, Zmm, Mem)           //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Ymm, Mem)                 // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Ymm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Zmm, Xmm)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastsd, Vbroadcastsd, Zmm, Mem)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Xmm, Mem)                 // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Xmm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Ymm, Mem)                 // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Ymm, Xmm)                 // AVX2 AVX512_F{kz}
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Zmm, Xmm)                 //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vbroadcastss, Vbroadcastss, Zmm, Mem)                 //      AVX512_F{kz}-VL
  ASMJIT_INST_4i(vcmppd, Vcmppd, Xmm, Xmm, Xmm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmppd, Vcmppd, Xmm, Xmm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmppd, Vcmppd, Ymm, Ymm, Ymm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmppd, Vcmppd, Ymm, Ymm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Xmm, Mem, Imm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Ymm, Ymm, Imm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Ymm, Mem, Imm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Zmm, Zmm, Imm)                  //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_4i(vcmppd, Vcmppd, KReg, Zmm, Mem, Imm)                  //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_4i(vcmpps, Vcmpps, Xmm, Xmm, Xmm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpps, Vcmpps, Xmm, Xmm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpps, Vcmpps, Ymm, Ymm, Ymm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpps, Vcmpps, Ymm, Ymm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Xmm, Mem, Imm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Ymm, Ymm, Imm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Ymm, Mem, Imm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Zmm, Zmm, Imm)                  //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vcmpps, Vcmpps, KReg, Zmm, Mem, Imm)                  //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vcmpsd, Vcmpsd, Xmm, Xmm, Xmm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpsd, Vcmpsd, Xmm, Xmm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpsd, Vcmpsd, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vcmpsd, Vcmpsd, KReg, Xmm, Mem, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vcmpss, Vcmpss, Xmm, Xmm, Xmm, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpss, Vcmpss, Xmm, Xmm, Mem, Imm)                   // AVX
  ASMJIT_INST_4i(vcmpss, Vcmpss, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vcmpss, Vcmpss, KReg, Xmm, Mem, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcomisd, Vcomisd, Xmm, Xmm)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomisd, Vcomisd, Xmm, Mem)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomiss, Vcomiss, Xmm, Xmm)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcomiss, Vcomiss, Xmm, Mem)                           // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Xmm, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Mem, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Ymm, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Mem, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Zmm, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompresspd, Vcompresspd, Mem, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompressps, Vcompressps, Xmm, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompressps, Vcompressps, Mem, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompressps, Vcompressps, Ymm, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompressps, Vcompressps, Mem, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcompressps, Vcompressps, Zmm, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcompressps, Vcompressps, Mem, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Xmm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Xmm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Ymm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Ymm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Zmm, Ymm)                       //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtdq2pd, Vcvtdq2pd, Zmm, Mem)                       //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Xmm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Xmm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Ymm, Ymm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Ymm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Zmm, Zmm)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtdq2ps, Vcvtdq2ps, Zmm, Mem)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Xmm, Xmm, Xmm)        //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Xmm, Xmm, Mem)        //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Ymm, Ymm, Ymm)        //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Ymm, Ymm, Mem)        //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Zmm, Zmm, Zmm)        //      AVX512_BF16{kz|b32}
  ASMJIT_INST_3x(vcvtne2ps2bf16, Vcvtne2ps2bf16, Zmm, Zmm, Mem)        //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Xmm, Xmm)               //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Xmm, Ymm)               //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Xmm, Mem)               //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Ymm, Zmm)               //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtneps2bf16, Vcvtneps2bf16, Ymm, Mem)               //      AVX512_BF16{kz|b32}
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Xmm, Xmm)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Xmm, Mem)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Xmm, Ymm)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Ymm, Zmm)                       //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2dq, Vcvtpd2dq, Ymm, Mem)                       //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Xmm, Xmm)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Xmm, Mem)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Xmm, Ymm)                       // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Ymm, Zmm)                       //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2ps, Vcvtpd2ps, Ymm, Mem)                       //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Xmm, Xmm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Xmm, Mem)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Ymm, Ymm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Ymm, Mem)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Zmm, Zmm)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2qq, Vcvtpd2qq, Zmm, Mem)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Xmm, Xmm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Xmm, Mem)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Xmm, Ymm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Ymm, Zmm)                     //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2udq, Vcvtpd2udq, Ymm, Mem)                     //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Xmm, Xmm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Xmm, Mem)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Ymm, Ymm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Ymm, Mem)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Zmm, Zmm)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtpd2uqq, Vcvtpd2uqq, Zmm, Mem)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Xmm, Xmm)                       // F16C AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Xmm, Mem)                       // F16C AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Ymm, Xmm)                       // F16C AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Ymm, Mem)                       // F16C AVX512_F{kz}-VL
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Zmm, Ymm)                       //      AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcvtph2ps, Vcvtph2ps, Zmm, Mem)                       //      AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Xmm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Xmm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Ymm, Ymm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Ymm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Zmm, Zmm)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2dq, Vcvtps2dq, Zmm, Mem)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Xmm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Xmm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Ymm, Xmm)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Ymm, Mem)                       // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Zmm, Ymm)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2pd, Vcvtps2pd, Zmm, Mem)                       //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Xmm, Xmm, Imm)                  // F16C AVX512_F{kz}-VL
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Mem, Xmm, Imm)                  // F16C AVX512_F{kz}-VL
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Xmm, Ymm, Imm)                  // F16C AVX512_F{kz}-VL
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Mem, Ymm, Imm)                  // F16C AVX512_F{kz}-VL
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Ymm, Zmm, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3i(vcvtps2ph, Vcvtps2ph, Mem, Zmm, Imm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Xmm, Xmm)                       //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Xmm, Mem)                       //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Ymm, Xmm)                       //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Ymm, Mem)                       //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Zmm, Ymm)                       //      AVX512_DQ{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2qq, Vcvtps2qq, Zmm, Mem)                       //      AVX512_DQ{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Xmm, Xmm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Xmm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Ymm, Ymm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Ymm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Zmm, Zmm)                     //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2udq, Vcvtps2udq, Zmm, Mem)                     //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Xmm, Xmm)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Xmm, Mem)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Ymm, Xmm)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Ymm, Mem)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Zmm, Ymm)                     //      AVX512_DQ{kz|er|b32}
  ASMJIT_INST_2x(vcvtps2uqq, Vcvtps2uqq, Zmm, Mem)                     //      AVX512_DQ{kz|er|b32}
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Xmm, Xmm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Xmm, Mem)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Ymm, Ymm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Ymm, Mem)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Zmm, Zmm)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtqq2pd, Vcvtqq2pd, Zmm, Mem)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Xmm, Xmm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Xmm, Mem)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Xmm, Ymm)                       //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Ymm, Zmm)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtqq2ps, Vcvtqq2ps, Ymm, Mem)                       //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtsd2si, Vcvtsd2si, Gp, Xmm)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtsd2si, Vcvtsd2si, Gp, Mem)                        // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsd2ss, Vcvtsd2ss, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vcvtsd2ss, Vcvtsd2ss, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|er}
  ASMJIT_INST_2x(vcvtsd2usi, Vcvtsd2usi, Gp, Xmm)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvtsd2usi, Vcvtsd2usi, Gp, Mem)                      //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2sd, Vcvtsi2sd, Xmm, Xmm, Gp)                   // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2sd, Vcvtsi2sd, Xmm, Xmm, Mem)                  // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2ss, Vcvtsi2ss, Xmm, Xmm, Gp)                   // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtsi2ss, Vcvtsi2ss, Xmm, Xmm, Mem)                  // AVX  AVX512_F{er}
  ASMJIT_INST_3x(vcvtss2sd, Vcvtss2sd, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_3x(vcvtss2sd, Vcvtss2sd, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|sae}
  ASMJIT_INST_2x(vcvtss2si, Vcvtss2si, Gp, Xmm)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2si, Vcvtss2si, Gp, Mem)                        // AVX  AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2usi, Vcvtss2usi, Gp, Xmm)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvtss2usi, Vcvtss2usi, Gp, Mem)                      //      AVX512_F{er}
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Xmm, Xmm)                     // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Xmm, Mem)                     // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Xmm, Ymm)                     // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Ymm, Zmm)                     //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2dq, Vcvttpd2dq, Ymm, Mem)                     //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Xmm, Xmm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Xmm, Mem)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Ymm, Ymm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Ymm, Mem)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Zmm, Zmm)                     //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2qq, Vcvttpd2qq, Zmm, Mem)                     //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Xmm, Xmm)                   //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Xmm, Mem)                   //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Xmm, Ymm)                   //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Ymm, Zmm)                   //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2udq, Vcvttpd2udq, Ymm, Mem)                   //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Xmm, Xmm)                   //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Xmm, Mem)                   //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Ymm, Ymm)                   //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Ymm, Mem)                   //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Zmm, Zmm)                   //      AVX512_DQ{kz|sae|b64}
  ASMJIT_INST_2x(vcvttpd2uqq, Vcvttpd2uqq, Zmm, Mem)                   //      AVX512_DQ{kz|sae|b64}
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Xmm, Xmm)                     // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Xmm, Mem)                     // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Ymm, Ymm)                     // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Ymm, Mem)                     // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Zmm, Zmm)                     //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2dq, Vcvttps2dq, Zmm, Mem)                     //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Xmm, Xmm)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Xmm, Mem)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Ymm, Xmm)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Ymm, Mem)                     //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Zmm, Ymm)                     //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2qq, Vcvttps2qq, Zmm, Mem)                     //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Xmm, Xmm)                   //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Xmm, Mem)                   //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Ymm, Ymm)                   //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Ymm, Mem)                   //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Zmm, Zmm)                   //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2udq, Vcvttps2udq, Zmm, Mem)                   //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Xmm, Xmm)                   //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Xmm, Mem)                   //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Ymm, Xmm)                   //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Ymm, Mem)                   //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Zmm, Ymm)                   //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_2x(vcvttps2uqq, Vcvttps2uqq, Zmm, Mem)                   //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_2x(vcvttsd2si, Vcvttsd2si, Gp, Xmm)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2si, Vcvttsd2si, Gp, Mem)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2usi, Vcvttsd2usi, Gp, Xmm)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttsd2usi, Vcvttsd2usi, Gp, Mem)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2si, Vcvttss2si, Gp, Xmm)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2si, Vcvttss2si, Gp, Mem)                      // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2usi, Vcvttss2usi, Gp, Xmm)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvttss2usi, Vcvttss2usi, Gp, Mem)                    //      AVX512_F{sae}
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Xmm, Xmm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Xmm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Ymm, Xmm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Ymm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Zmm, Ymm)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtudq2pd, Vcvtudq2pd, Zmm, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Xmm, Xmm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Xmm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Ymm, Ymm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Ymm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Zmm, Zmm)                     //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtudq2ps, Vcvtudq2ps, Zmm, Mem)                     //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Xmm, Xmm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Xmm, Mem)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Ymm, Ymm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Ymm, Mem)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Zmm, Zmm)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtuqq2pd, Vcvtuqq2pd, Zmm, Mem)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Xmm, Xmm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Xmm, Mem)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Xmm, Ymm)                     //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Ymm, Zmm)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_2x(vcvtuqq2ps, Vcvtuqq2ps, Ymm, Mem)                     //      AVX512_DQ{kz|er|b64}
  ASMJIT_INST_3x(vcvtusi2sd, Vcvtusi2sd, Xmm, Xmm, Gp)                 //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2sd, Vcvtusi2sd, Xmm, Xmm, Mem)                //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2ss, Vcvtusi2ss, Xmm, Xmm, Gp)                 //      AVX512_F{er}
  ASMJIT_INST_3x(vcvtusi2ss, Vcvtusi2ss, Xmm, Xmm, Mem)                //      AVX512_F{er}
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Xmm, Xmm, Xmm, Imm)             //      AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Xmm, Xmm, Mem, Imm)             //      AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Ymm, Ymm, Ymm, Imm)             //      AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Ymm, Ymm, Mem, Imm)             //      AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Zmm, Zmm, Zmm, Imm)             //      AVX512_BW{kz}
  ASMJIT_INST_4i(vdbpsadbw, Vdbpsadbw, Zmm, Zmm, Mem, Imm)             //      AVX512_BW{kz}
  ASMJIT_INST_3x(vdivpd, Vdivpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vdivpd, Vdivpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vdivpd, Vdivpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vdivpd, Vdivpd, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vdivpd, Vdivpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vdivpd, Vdivpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vdivps, Vdivps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vdivps, Vdivps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vdivps, Vdivps, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vdivps, Vdivps, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vdivps, Vdivps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vdivps, Vdivps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vdivsd, Vdivsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivsd, Vdivsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivss, Vdivss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdivss, Vdivss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Xmm, Xmm, Xmm)                  //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Xmm, Xmm, Mem)                  //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Ymm, Ymm, Ymm)                  //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Ymm, Ymm, Mem)                  //      AVX512_BF16{kz|b32}-VL
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Zmm, Zmm, Zmm)                  //      AVX512_BF16{kz|b32}
  ASMJIT_INST_3x(vdpbf16ps, Vdpbf16ps, Zmm, Zmm, Mem)                  //      AVX512_BF16{kz|b32}
  ASMJIT_INST_4i(vdppd, Vdppd, Xmm, Xmm, Xmm, Imm)                     // AVX
  ASMJIT_INST_4i(vdppd, Vdppd, Xmm, Xmm, Mem, Imm)                     // AVX
  ASMJIT_INST_4i(vdpps, Vdpps, Xmm, Xmm, Xmm, Imm)                     // AVX
  ASMJIT_INST_4i(vdpps, Vdpps, Xmm, Xmm, Mem, Imm)                     // AVX
  ASMJIT_INST_4i(vdpps, Vdpps, Ymm, Ymm, Ymm, Imm)                     // AVX
  ASMJIT_INST_4i(vdpps, Vdpps, Ymm, Ymm, Mem, Imm)                     // AVX
  ASMJIT_INST_2x(vexp2pd, Vexp2pd, Zmm, Zmm)                           //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vexp2pd, Vexp2pd, Zmm, Mem)                           //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vexp2ps, Vexp2ps, Zmm, Zmm)                           //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_2x(vexp2ps, Vexp2ps, Zmm, Mem)                           //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandpd, Vexpandpd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandps, Vexpandps, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandps, Vexpandps, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandps, Vexpandps, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandps, Vexpandps, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vexpandps, Vexpandps, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vexpandps, Vexpandps, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_3i(vextractf128, Vextractf128, Xmm, Ymm, Imm)            // AVX
  ASMJIT_INST_3i(vextractf128, Vextractf128, Mem, Ymm, Imm)            // AVX
  ASMJIT_INST_3i(vextractf32x4, Vextractf32x4, Xmm, Ymm, Imm)          //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vextractf32x4, Vextractf32x4, Mem, Ymm, Imm)          //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vextractf32x4, Vextractf32x4, Xmm, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextractf32x4, Vextractf32x4, Mem, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextractf32x8, Vextractf32x8, Ymm, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextractf32x8, Vextractf32x8, Mem, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextractf64x2, Vextractf64x2, Xmm, Ymm, Imm)          //      AVX512_DQ{kz}-VL
  ASMJIT_INST_3i(vextractf64x2, Vextractf64x2, Mem, Ymm, Imm)          //      AVX512_DQ{kz}-VL
  ASMJIT_INST_3i(vextractf64x2, Vextractf64x2, Xmm, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextractf64x2, Vextractf64x2, Mem, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextractf64x4, Vextractf64x4, Ymm, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextractf64x4, Vextractf64x4, Mem, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextracti128, Vextracti128, Xmm, Ymm, Imm)            // AVX2
  ASMJIT_INST_3i(vextracti128, Vextracti128, Mem, Ymm, Imm)            // AVX2
  ASMJIT_INST_3i(vextracti32x4, Vextracti32x4, Xmm, Ymm, Imm)          //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vextracti32x4, Vextracti32x4, Mem, Ymm, Imm)          //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vextracti32x4, Vextracti32x4, Xmm, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextracti32x4, Vextracti32x4, Mem, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextracti32x8, Vextracti32x8, Ymm, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextracti32x8, Vextracti32x8, Mem, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextracti64x2, Vextracti64x2, Xmm, Ymm, Imm)          //      AVX512_DQ{kz}-VL
  ASMJIT_INST_3i(vextracti64x2, Vextracti64x2, Mem, Ymm, Imm)          //      AVX512_DQ{kz}-VL
  ASMJIT_INST_3i(vextracti64x2, Vextracti64x2, Xmm, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextracti64x2, Vextracti64x2, Mem, Zmm, Imm)          //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vextracti64x4, Vextracti64x4, Ymm, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextracti64x4, Vextracti64x4, Mem, Zmm, Imm)          //      AVX512_F{kz}
  ASMJIT_INST_3i(vextractps, Vextractps, Gp, Xmm, Imm)                 // AVX  AVX512_F
  ASMJIT_INST_3i(vextractps, Vextractps, Mem, Xmm, Imm)                // AVX  AVX512_F
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Ymm, Ymm, Ymm, Imm)         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Ymm, Ymm, Mem, Imm)         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Zmm, Zmm, Zmm, Imm)         //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_4i(vfixupimmpd, Vfixupimmpd, Zmm, Zmm, Mem, Imm)         //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Ymm, Ymm, Ymm, Imm)         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Ymm, Ymm, Mem, Imm)         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Zmm, Zmm, Zmm, Imm)         //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vfixupimmps, Vfixupimmps, Zmm, Zmm, Mem, Imm)         //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vfixupimmsd, Vfixupimmsd, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vfixupimmsd, Vfixupimmsd, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vfixupimmss, Vfixupimmss, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vfixupimmss, Vfixupimmss, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd132pd, Vfmadd132pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd132ps, Vfmadd132ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd132sd, Vfmadd132sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132sd, Vfmadd132sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132ss, Vfmadd132ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd132ss, Vfmadd132ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd213pd, Vfmadd213pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd213ps, Vfmadd213ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd213sd, Vfmadd213sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213sd, Vfmadd213sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213ss, Vfmadd213ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd213ss, Vfmadd213ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd231pd, Vfmadd231pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd231ps, Vfmadd231ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmadd231sd, Vfmadd231sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231sd, Vfmadd231sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231ss, Vfmadd231ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmadd231ss, Vfmadd231ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub132pd, Vfmaddsub132pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmaddsub132ps, Vfmaddsub132ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub213pd, Vfmaddsub213pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmaddsub213ps, Vfmaddsub213ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub231pd, Vfmaddsub231pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmaddsub231ps, Vfmaddsub231ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub132pd, Vfmsub132pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub132ps, Vfmsub132ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub132sd, Vfmsub132sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132sd, Vfmsub132sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132ss, Vfmsub132ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub132ss, Vfmsub132ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub213pd, Vfmsub213pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub213ps, Vfmsub213ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub213sd, Vfmsub213sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213sd, Vfmsub213sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213ss, Vfmsub213ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub213ss, Vfmsub213ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub231pd, Vfmsub231pd, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Ymm, Ymm, Ymm)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Ymm, Ymm, Mem)              // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Zmm, Zmm, Zmm)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub231ps, Vfmsub231ps, Zmm, Zmm, Mem)              // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsub231sd, Vfmsub231sd, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231sd, Vfmsub231sd, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231ss, Vfmsub231ss, Xmm, Xmm, Xmm)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsub231ss, Vfmsub231ss, Xmm, Xmm, Mem)              // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd132pd, Vfmsubadd132pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsubadd132ps, Vfmsubadd132ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd213pd, Vfmsubadd213pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsubadd213ps, Vfmsubadd213ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd231pd, Vfmsubadd231pd, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Xmm, Xmm, Xmm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Xmm, Xmm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Ymm, Ymm, Ymm)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Ymm, Ymm, Mem)        // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Zmm, Zmm, Zmm)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfmsubadd231ps, Vfmsubadd231ps, Zmm, Zmm, Mem)        // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd132pd, Vfnmadd132pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd132ps, Vfnmadd132ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd132sd, Vfnmadd132sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132sd, Vfnmadd132sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132ss, Vfnmadd132ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd132ss, Vfnmadd132ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd213pd, Vfnmadd213pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd213ps, Vfnmadd213ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd213sd, Vfnmadd213sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213sd, Vfnmadd213sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213ss, Vfnmadd213ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd213ss, Vfnmadd213ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd231pd, Vfnmadd231pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd231ps, Vfnmadd231ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmadd231sd, Vfnmadd231sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231sd, Vfnmadd231sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231ss, Vfnmadd231ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmadd231ss, Vfnmadd231ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub132pd, Vfnmsub132pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub132ps, Vfnmsub132ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub132sd, Vfnmsub132sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132sd, Vfnmsub132sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132ss, Vfnmsub132ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub132ss, Vfnmsub132ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub213pd, Vfnmsub213pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub213ps, Vfnmsub213ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub213sd, Vfnmsub213sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213sd, Vfnmsub213sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213ss, Vfnmsub213ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub213ss, Vfnmsub213ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub231pd, Vfnmsub231pd, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Ymm, Ymm, Ymm)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Ymm, Ymm, Mem)            // FMA  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Zmm, Zmm, Zmm)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub231ps, Vfnmsub231ps, Zmm, Zmm, Mem)            // FMA  AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vfnmsub231sd, Vfnmsub231sd, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231sd, Vfnmsub231sd, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231ss, Vfnmsub231ss, Xmm, Xmm, Xmm)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3x(vfnmsub231ss, Vfnmsub231ss, Xmm, Xmm, Mem)            // FMA  AVX512_F{kz|er}
  ASMJIT_INST_3i(vfpclasspd, Vfpclasspd, KReg, Xmm, Imm)               //      AVX512_DQ{k|b64}-VL
  ASMJIT_INST_3i(vfpclasspd, Vfpclasspd, KReg, Mem, Imm)               //      AVX512_DQ{k|b64} AVX512_DQ{k|b64}-VL
  ASMJIT_INST_3i(vfpclasspd, Vfpclasspd, KReg, Ymm, Imm)               //      AVX512_DQ{k|b64}-VL
  ASMJIT_INST_3i(vfpclasspd, Vfpclasspd, KReg, Zmm, Imm)               //      AVX512_DQ{k|b64}
  ASMJIT_INST_3i(vfpclassps, Vfpclassps, KReg, Xmm, Imm)               //      AVX512_DQ{k|b32}-VL
  ASMJIT_INST_3i(vfpclassps, Vfpclassps, KReg, Mem, Imm)               //      AVX512_DQ{k|b32} AVX512_DQ{k|b32}-VL
  ASMJIT_INST_3i(vfpclassps, Vfpclassps, KReg, Ymm, Imm)               //      AVX512_DQ{k|b32}-VL
  ASMJIT_INST_3i(vfpclassps, Vfpclassps, KReg, Zmm, Imm)               //      AVX512_DQ{k|b32}
  ASMJIT_INST_3i(vfpclasssd, Vfpclasssd, KReg, Xmm, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3i(vfpclasssd, Vfpclasssd, KReg, Mem, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3i(vfpclassss, Vfpclassss, KReg, Xmm, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3i(vfpclassss, Vfpclassss, KReg, Mem, Imm)               //      AVX512_DQ{k}
  ASMJIT_INST_3x(vgatherdpd, Vgatherdpd, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vgatherdpd, Vgatherdpd, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vgatherdpd, Vgatherdpd, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherdpd, Vgatherdpd, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherdpd, Vgatherdpd, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherdps, Vgatherdps, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vgatherdps, Vgatherdps, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vgatherdps, Vgatherdps, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherdps, Vgatherdps, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherdps, Vgatherdps, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_1x(vgatherpf0dpd, Vgatherpf0dpd, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf0dps, Vgatherpf0dps, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf0qpd, Vgatherpf0qpd, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf0qps, Vgatherpf0qps, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf1dpd, Vgatherpf1dpd, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf1dps, Vgatherpf1dps, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf1qpd, Vgatherpf1qpd, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_1x(vgatherpf1qps, Vgatherpf1qps, Mem)                    //      AVX512_PF{k}
  ASMJIT_INST_3x(vgatherqpd, Vgatherqpd, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vgatherqpd, Vgatherqpd, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vgatherqpd, Vgatherqpd, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherqpd, Vgatherqpd, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherqpd, Vgatherqpd, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vgatherqps, Vgatherqps, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_2x(vgatherqps, Vgatherqps, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherqps, Vgatherqps, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vgatherqps, Vgatherqps, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Xmm, Xmm)                       //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Xmm, Mem)                       //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Ymm, Ymm)                       //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Ymm, Mem)                       //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Zmm, Zmm)                       //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vgetexppd, Vgetexppd, Zmm, Mem)                       //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Xmm, Xmm)                       //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Xmm, Mem)                       //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Ymm, Ymm)                       //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Ymm, Mem)                       //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Zmm, Zmm)                       //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_2x(vgetexpps, Vgetexpps, Zmm, Mem)                       //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3x(vgetexpsd, Vgetexpsd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpsd, Vgetexpsd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpss, Vgetexpss, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3x(vgetexpss, Vgetexpss, Xmm, Xmm, Mem)                  //      AVX512_F{kz|sae}
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Xmm, Xmm, Imm)                //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Xmm, Mem, Imm)                //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Ymm, Ymm, Imm)                //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Ymm, Mem, Imm)                //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Zmm, Zmm, Imm)                //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3i(vgetmantpd, Vgetmantpd, Zmm, Mem, Imm)                //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Xmm, Xmm, Imm)                //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Xmm, Mem, Imm)                //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Ymm, Ymm, Imm)                //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Ymm, Mem, Imm)                //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Zmm, Zmm, Imm)                //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3i(vgetmantps, Vgetmantps, Zmm, Mem, Imm)                //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vgetmantsd, Vgetmantsd, Xmm, Xmm, Xmm, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vgetmantsd, Vgetmantsd, Xmm, Xmm, Mem, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vgetmantss, Vgetmantss, Xmm, Xmm, Xmm, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vgetmantss, Vgetmantss, Xmm, Xmm, Mem, Imm)           //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Xmm,Xmm,Xmm,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Xmm,Xmm,Mem,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Ymm,Ymm,Ymm,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Ymm,Ymm,Mem,Imm) // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Zmm,Zmm,Zmm,Imm) //      AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineinvqb, Vgf2p8affineinvqb,Zmm,Zmm,Mem,Imm) //      AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Xmm, Xmm, Xmm, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Xmm, Xmm, Mem, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Ymm, Ymm, Ymm, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Ymm, Ymm, Mem, Imm)   // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Zmm, Zmm, Zmm, Imm)   //      AVX512_VL{kz} GFNI
  ASMJIT_INST_4i(vgf2p8affineqb, Vgf2p8affineqb, Zmm, Zmm, Mem, Imm)   //      AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Xmm, Xmm, Xmm)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Xmm, Xmm, Mem)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Ymm, Ymm, Ymm)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Ymm, Ymm, Mem)                // AVX  AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Zmm, Zmm, Zmm)                //      AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vgf2p8mulb, Vgf2p8mulb, Zmm, Zmm, Mem)                //      AVX512_VL{kz} GFNI
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Ymm, Ymm, Ymm)                      // AVX
  ASMJIT_INST_3x(vhaddpd, Vhaddpd, Ymm, Ymm, Mem)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Ymm, Ymm, Ymm)                      // AVX
  ASMJIT_INST_3x(vhaddps, Vhaddps, Ymm, Ymm, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Ymm, Ymm, Ymm)                      // AVX
  ASMJIT_INST_3x(vhsubpd, Vhsubpd, Ymm, Ymm, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Ymm, Ymm, Ymm)                      // AVX
  ASMJIT_INST_3x(vhsubps, Vhsubps, Ymm, Ymm, Mem)                      // AVX
  ASMJIT_INST_4i(vinsertf128, Vinsertf128, Ymm, Ymm, Xmm, Imm)         // AVX
  ASMJIT_INST_4i(vinsertf128, Vinsertf128, Ymm, Ymm, Mem, Imm)         // AVX
  ASMJIT_INST_4i(vinsertf32x4, Vinsertf32x4, Ymm, Ymm, Xmm, Imm)       //      AVX512_F{kz}-VL
  ASMJIT_INST_4i(vinsertf32x4, Vinsertf32x4, Ymm, Ymm, Mem, Imm)       //      AVX512_F{kz}-VL
  ASMJIT_INST_4i(vinsertf32x4, Vinsertf32x4, Zmm, Zmm, Xmm, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinsertf32x4, Vinsertf32x4, Zmm, Zmm, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinsertf32x8, Vinsertf32x8, Zmm, Zmm, Ymm, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinsertf32x8, Vinsertf32x8, Zmm, Zmm, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinsertf64x2, Vinsertf64x2, Ymm, Ymm, Xmm, Imm)       //      AVX512_DQ{kz}-VL
  ASMJIT_INST_4i(vinsertf64x2, Vinsertf64x2, Ymm, Ymm, Mem, Imm)       //      AVX512_DQ{kz}-VL
  ASMJIT_INST_4i(vinsertf64x2, Vinsertf64x2, Zmm, Zmm, Xmm, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinsertf64x2, Vinsertf64x2, Zmm, Zmm, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinsertf64x4, Vinsertf64x4, Zmm, Zmm, Ymm, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinsertf64x4, Vinsertf64x4, Zmm, Zmm, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinserti128, Vinserti128, Ymm, Ymm, Xmm, Imm)         // AVX2
  ASMJIT_INST_4i(vinserti128, Vinserti128, Ymm, Ymm, Mem, Imm)         // AVX2
  ASMJIT_INST_4i(vinserti32x4, Vinserti32x4, Ymm, Ymm, Xmm, Imm)       //      AVX512_F{kz}-VL
  ASMJIT_INST_4i(vinserti32x4, Vinserti32x4, Ymm, Ymm, Mem, Imm)       //      AVX512_F{kz}-VL
  ASMJIT_INST_4i(vinserti32x4, Vinserti32x4, Zmm, Zmm, Xmm, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinserti32x4, Vinserti32x4, Zmm, Zmm, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinserti32x8, Vinserti32x8, Zmm, Zmm, Ymm, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinserti32x8, Vinserti32x8, Zmm, Zmm, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinserti64x2, Vinserti64x2, Ymm, Ymm, Xmm, Imm)       //      AVX512_DQ{kz}-VL
  ASMJIT_INST_4i(vinserti64x2, Vinserti64x2, Ymm, Ymm, Mem, Imm)       //      AVX512_DQ{kz}-VL
  ASMJIT_INST_4i(vinserti64x2, Vinserti64x2, Zmm, Zmm, Xmm, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinserti64x2, Vinserti64x2, Zmm, Zmm, Mem, Imm)       //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vinserti64x4, Vinserti64x4, Zmm, Zmm, Ymm, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinserti64x4, Vinserti64x4, Zmm, Zmm, Mem, Imm)       //      AVX512_F{kz}
  ASMJIT_INST_4i(vinsertps, Vinsertps, Xmm, Xmm, Xmm, Imm)             // AVX  AVX512_F
  ASMJIT_INST_4i(vinsertps, Vinsertps, Xmm, Xmm, Mem, Imm)             // AVX  AVX512_F
  ASMJIT_INST_2x(vlddqu, Vlddqu, Xmm, Mem)                             // AVX
  ASMJIT_INST_2x(vlddqu, Vlddqu, Ymm, Mem)                             // AVX
  ASMJIT_INST_1x(vldmxcsr, Vldmxcsr, Mem)                              // AVX
  ASMJIT_INST_3x(vmaskmovdqu, Vmaskmovdqu, Xmm, Xmm, DS_ZDI)           // AVX  [EXPLICIT]
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Mem, Xmm, Xmm)                // AVX
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Mem, Ymm, Ymm)                // AVX
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Xmm, Xmm, Mem)                // AVX
  ASMJIT_INST_3x(vmaskmovpd, Vmaskmovpd, Ymm, Ymm, Mem)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Mem, Xmm, Xmm)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Mem, Ymm, Ymm)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Xmm, Xmm, Mem)                // AVX
  ASMJIT_INST_3x(vmaskmovps, Vmaskmovps, Ymm, Ymm, Mem)                // AVX
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3x(vmaxpd, Vmaxpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3x(vmaxps, Vmaxps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmaxps, Vmaxps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmaxps, Vmaxps, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmaxps, Vmaxps, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmaxps, Vmaxps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3x(vmaxps, Vmaxps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3x(vmaxsd, Vmaxsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vmaxsd, Vmaxsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vmaxss, Vmaxss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vmaxss, Vmaxss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vminpd, Vminpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vminpd, Vminpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vminpd, Vminpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vminpd, Vminpd, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vminpd, Vminpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3x(vminpd, Vminpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3x(vminps, Vminps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vminps, Vminps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vminps, Vminps, Ymm, Ymm, Ymm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vminps, Vminps, Ymm, Ymm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vminps, Vminps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3x(vminps, Vminps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3x(vminsd, Vminsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vminsd, Vminsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vminss, Vminss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_3x(vminss, Vminss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|sae}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Xmm, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Xmm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Mem, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Ymm, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Ymm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Mem, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovapd, Vmovapd, Zmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovapd, Vmovapd, Zmm, Mem)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovapd, Vmovapd, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Xmm, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Xmm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Mem, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Ymm, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Ymm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Mem, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovaps, Vmovaps, Zmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Zmm, Mem)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovaps, Vmovaps, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovd, Vmovd, Gp, Xmm)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Mem, Xmm)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Xmm, Gp)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovd, Vmovd, Xmm, Mem)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovddup, Vmovddup, Xmm, Xmm)                         // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovddup, Vmovddup, Xmm, Mem)                         // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovddup, Vmovddup, Ymm, Ymm)                         // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovddup, Vmovddup, Ymm, Mem)                         // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovddup, Vmovddup, Zmm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovddup, Vmovddup, Zmm, Mem)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Xmm, Xmm)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Xmm, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Mem, Xmm)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Ymm, Ymm)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Ymm, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqa, Vmovdqa, Mem, Ymm)                           // AVX
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa32, Vmovdqa32, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqa64, Vmovdqa64, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Xmm, Xmm)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Xmm, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Mem, Xmm)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Ymm, Ymm)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Ymm, Mem)                           // AVX
  ASMJIT_INST_2x(vmovdqu, Vmovdqu, Mem, Ymm)                           // AVX
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Xmm, Xmm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Xmm, Mem)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Mem, Xmm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Ymm, Ymm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Ymm, Mem)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Mem, Ymm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Zmm, Zmm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Zmm, Mem)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu16, Vmovdqu16, Mem, Zmm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu32, Vmovdqu32, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu64, Vmovdqu64, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Xmm, Xmm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Xmm, Mem)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Mem, Xmm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Ymm, Ymm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Ymm, Mem)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Mem, Ymm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Zmm, Zmm)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Zmm, Mem)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vmovdqu8, Vmovdqu8, Mem, Zmm)                         //      AVX512_BW{kz}
  ASMJIT_INST_3x(vmovhlps, Vmovhlps, Xmm, Xmm, Xmm)                    // AVX  AVX512_F
  ASMJIT_INST_2x(vmovhpd, Vmovhpd, Mem, Xmm)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovhpd, Vmovhpd, Xmm, Xmm, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovhps, Vmovhps, Mem, Xmm)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovhps, Vmovhps, Xmm, Xmm, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlhps, Vmovlhps, Xmm, Xmm, Xmm)                    // AVX  AVX512_F
  ASMJIT_INST_2x(vmovlpd, Vmovlpd, Mem, Xmm)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlpd, Vmovlpd, Xmm, Xmm, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovlps, Vmovlps, Mem, Xmm)                           // AVX  AVX512_F
  ASMJIT_INST_3x(vmovlps, Vmovlps, Xmm, Xmm, Mem)                      // AVX  AVX512_F
  ASMJIT_INST_2x(vmovmskpd, Vmovmskpd, Gp, Xmm)                        // AVX
  ASMJIT_INST_2x(vmovmskpd, Vmovmskpd, Gp, Ymm)                        // AVX
  ASMJIT_INST_2x(vmovmskps, Vmovmskps, Gp, Xmm)                        // AVX
  ASMJIT_INST_2x(vmovmskps, Vmovmskps, Gp, Ymm)                        // AVX
  ASMJIT_INST_2x(vmovntdq, Vmovntdq, Mem, Xmm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntdq, Vmovntdq, Mem, Ymm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntdq, Vmovntdq, Mem, Zmm)                         //      AVX512_F
  ASMJIT_INST_2x(vmovntdqa, Vmovntdqa, Xmm, Mem)                       // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntdqa, Vmovntdqa, Ymm, Mem)                       // AVX2 AVX512_F-VL
  ASMJIT_INST_2x(vmovntdqa, Vmovntdqa, Zmm, Mem)                       //      AVX512_F
  ASMJIT_INST_2x(vmovntpd, Vmovntpd, Mem, Xmm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntpd, Vmovntpd, Mem, Ymm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntpd, Vmovntpd, Mem, Zmm)                         //      AVX512_F
  ASMJIT_INST_2x(vmovntps, Vmovntps, Mem, Xmm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntps, Vmovntps, Mem, Ymm)                         // AVX  AVX512_F-VL
  ASMJIT_INST_2x(vmovntps, Vmovntps, Mem, Zmm)                         //      AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Gp, Xmm)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Mem, Xmm)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Xmm, Mem)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Xmm, Gp)                                // AVX  AVX512_F
  ASMJIT_INST_2x(vmovq, Vmovq, Xmm, Xmm)                               // AVX  AVX512_F
  ASMJIT_INST_2x(vmovsd, Vmovsd, Mem, Xmm)                             // AVX  AVX512_F
  ASMJIT_INST_2x(vmovsd, Vmovsd, Xmm, Mem)                             // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vmovsd, Vmovsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Ymm, Ymm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Ymm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovshdup, Vmovshdup, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Ymm, Ymm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Ymm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovsldup, Vmovsldup, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovss, Vmovss, Mem, Xmm)                             // AVX  AVX512_F
  ASMJIT_INST_2x(vmovss, Vmovss, Xmm, Mem)                             // AVX  AVX512_F{kz}
  ASMJIT_INST_3x(vmovss, Vmovss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Xmm, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Xmm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Mem, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Ymm, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Ymm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Mem, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovupd, Vmovupd, Zmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Zmm, Mem)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovupd, Vmovupd, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Xmm, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Xmm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Mem, Xmm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Ymm, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Ymm, Mem)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Mem, Ymm)                           // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vmovups, Vmovups, Zmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Zmm, Mem)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vmovups, Vmovups, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_4i(vmpsadbw, Vmpsadbw, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vmpsadbw, Vmpsadbw, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vmpsadbw, Vmpsadbw, Ymm, Ymm, Ymm, Imm)               // AVX2
  ASMJIT_INST_4i(vmpsadbw, Vmpsadbw, Ymm, Ymm, Mem, Imm)               // AVX2
  ASMJIT_INST_3x(vmulpd, Vmulpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmulpd, Vmulpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmulpd, Vmulpd, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmulpd, Vmulpd, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vmulpd, Vmulpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vmulpd, Vmulpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vmulps, Vmulps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmulps, Vmulps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmulps, Vmulps, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmulps, Vmulps, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vmulps, Vmulps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vmulps, Vmulps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vmulsd, Vmulsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulsd, Vmulsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulss, Vmulss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vmulss, Vmulss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vorpd, Vorpd, Xmm, Xmm, Xmm)                          // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vorpd, Vorpd, Xmm, Xmm, Mem)                          // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vorpd, Vorpd, Ymm, Ymm, Ymm)                          // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vorpd, Vorpd, Ymm, Ymm, Mem)                          // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vorpd, Vorpd, Zmm, Zmm, Zmm)                          //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vorpd, Vorpd, Zmm, Zmm, Mem)                          //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vorps, Vorps, Xmm, Xmm, Xmm)                          // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vorps, Vorps, Xmm, Xmm, Mem)                          // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vorps, Vorps, Ymm, Ymm, Ymm)                          // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vorps, Vorps, Ymm, Ymm, Mem)                          // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vorps, Vorps, Zmm, Zmm, Zmm)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vorps, Vorps, Zmm, Zmm, Mem)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_6x(vp4dpwssd, Vp4dpwssd, Zmm, Zmm, Zmm, Zmm, Zmm, Mem)   // AVX512_4FMAPS{kz}
  ASMJIT_INST_6x(vp4dpwssds, Vp4dpwssds, Zmm, Zmm, Zmm, Zmm, Zmm, Mem) // AVX512_4FMAPS{kz}
  ASMJIT_INST_2x(vpabsb, Vpabsb, Xmm, Xmm)                             // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsb, Vpabsb, Xmm, Mem)                             // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsb, Vpabsb, Ymm, Ymm)                             // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsb, Vpabsb, Ymm, Mem)                             // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsb, Vpabsb, Zmm, Zmm)                             //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsb, Vpabsb, Zmm, Mem)                             //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsd, Vpabsd, Xmm, Xmm)                             // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsd, Vpabsd, Xmm, Mem)                             // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsd, Vpabsd, Ymm, Ymm)                             // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsd, Vpabsd, Ymm, Mem)                             // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsd, Vpabsd, Zmm, Zmm)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsd, Vpabsd, Zmm, Mem)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsq, Vpabsq, Xmm, Xmm)                             //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsq, Vpabsq, Xmm, Mem)                             //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsq, Vpabsq, Ymm, Ymm)                             //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsq, Vpabsq, Ymm, Mem)                             //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpabsq, Vpabsq, Zmm, Zmm)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsq, Vpabsq, Zmm, Mem)                             //      AVX512_F{kz}
  ASMJIT_INST_2x(vpabsw, Vpabsw, Xmm, Xmm)                             // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsw, Vpabsw, Xmm, Mem)                             // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsw, Vpabsw, Ymm, Ymm)                             // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsw, Vpabsw, Ymm, Mem)                             // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpabsw, Vpabsw, Zmm, Zmm)                             //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpabsw, Vpabsw, Zmm, Mem)                             //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Xmm, Xmm, Xmm)                  // AVX  AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Xmm, Xmm, Mem)                  // AVX  AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Ymm, Ymm, Ymm)                  // AVX2 AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Ymm, Ymm, Mem)                  // AVX2 AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Zmm, Zmm, Zmm)                  //      AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackssdw, Vpackssdw, Zmm, Zmm, Mem)                  //      AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Xmm, Xmm, Xmm)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Xmm, Xmm, Mem)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Ymm, Ymm, Ymm)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Ymm, Ymm, Mem)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Zmm, Zmm, Zmm)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpacksswb, Vpacksswb, Zmm, Zmm, Mem)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Xmm, Xmm, Xmm)                  // AVX  AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Xmm, Xmm, Mem)                  // AVX  AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Ymm, Ymm, Ymm)                  // AVX2 AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Ymm, Ymm, Mem)                  // AVX2 AVX512_BW{kz|b32}-VL
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Zmm, Zmm, Zmm)                  //      AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackusdw, Vpackusdw, Zmm, Zmm, Mem)                  //      AVX512_BW{kz|b32}
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Xmm, Xmm, Xmm)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Xmm, Xmm, Mem)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Ymm, Ymm, Ymm)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Ymm, Ymm, Mem)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Zmm, Zmm, Zmm)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpackuswb, Vpackuswb, Zmm, Zmm, Mem)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddb, Vpaddb, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddb, Vpaddb, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddb, Vpaddb, Ymm, Ymm, Ymm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddb, Vpaddb, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddb, Vpaddb, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddb, Vpaddb, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddd, Vpaddd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpaddd, Vpaddd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpaddd, Vpaddd, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpaddd, Vpaddd, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpaddd, Vpaddd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpaddd, Vpaddd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpaddq, Vpaddq, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpaddq, Vpaddq, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpaddq, Vpaddq, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpaddq, Vpaddq, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpaddq, Vpaddq, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpaddq, Vpaddq, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsb, Vpaddsb, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddsw, Vpaddsw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusb, Vpaddusb, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddusw, Vpaddusw, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddw, Vpaddw, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddw, Vpaddw, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddw, Vpaddw, Ymm, Ymm, Ymm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddw, Vpaddw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpaddw, Vpaddw, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpaddw, Vpaddw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_4i(vpalignr, Vpalignr, Xmm, Xmm, Xmm, Imm)               // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vpalignr, Vpalignr, Xmm, Xmm, Mem, Imm)               // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vpalignr, Vpalignr, Ymm, Ymm, Ymm, Imm)               // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vpalignr, Vpalignr, Ymm, Ymm, Mem, Imm)               // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_4i(vpalignr, Vpalignr, Zmm, Zmm, Zmm, Imm)               //      AVX512_BW{kz}
  ASMJIT_INST_4i(vpalignr, Vpalignr, Zmm, Zmm, Mem, Imm)               //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpand, Vpand, Xmm, Xmm, Xmm)                          // AVX
  ASMJIT_INST_3x(vpand, Vpand, Xmm, Xmm, Mem)                          // AVX
  ASMJIT_INST_3x(vpand, Vpand, Ymm, Ymm, Ymm)                          // AVX2
  ASMJIT_INST_3x(vpand, Vpand, Ymm, Ymm, Mem)                          // AVX2
  ASMJIT_INST_3x(vpandd, Vpandd, Xmm, Xmm, Xmm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandd, Vpandd, Xmm, Xmm, Mem)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandd, Vpandd, Ymm, Ymm, Ymm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandd, Vpandd, Ymm, Ymm, Mem)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandd, Vpandd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandd, Vpandd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandn, Vpandn, Xmm, Xmm, Xmm)                        // AVX
  ASMJIT_INST_3x(vpandn, Vpandn, Xmm, Xmm, Mem)                        // AVX
  ASMJIT_INST_3x(vpandn, Vpandn, Ymm, Ymm, Ymm)                        // AVX2
  ASMJIT_INST_3x(vpandn, Vpandn, Ymm, Ymm, Mem)                        // AVX2
  ASMJIT_INST_3x(vpandnd, Vpandnd, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandnd, Vpandnd, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandnd, Vpandnd, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandnd, Vpandnd, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpandnd, Vpandnd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandnd, Vpandnd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpandnq, Vpandnq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandnq, Vpandnq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandnq, Vpandnq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandnq, Vpandnq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandnq, Vpandnq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandnq, Vpandnq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandq, Vpandq, Xmm, Xmm, Xmm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandq, Vpandq, Xmm, Xmm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandq, Vpandq, Ymm, Ymm, Ymm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandq, Vpandq, Ymm, Ymm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpandq, Vpandq, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpandq, Vpandq, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpavgb, Vpavgb, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgb, Vpavgb, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgb, Vpavgb, Ymm, Ymm, Ymm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgb, Vpavgb, Ymm, Ymm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgb, Vpavgb, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgb, Vpavgb, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgw, Vpavgw, Xmm, Xmm, Xmm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgw, Vpavgw, Xmm, Xmm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgw, Vpavgw, Ymm, Ymm, Ymm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgw, Vpavgw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpavgw, Vpavgw, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpavgw, Vpavgw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_4i(vpblendd, Vpblendd, Xmm, Xmm, Xmm, Imm)               // AVX2
  ASMJIT_INST_4i(vpblendd, Vpblendd, Xmm, Xmm, Mem, Imm)               // AVX2
  ASMJIT_INST_4i(vpblendd, Vpblendd, Ymm, Ymm, Ymm, Imm)               // AVX2
  ASMJIT_INST_4i(vpblendd, Vpblendd, Ymm, Ymm, Mem, Imm)               // AVX2
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Xmm, Xmm, Xmm, Xmm)             // AVX
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Xmm, Xmm, Mem, Xmm)             // AVX
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Ymm, Ymm, Ymm, Ymm)             // AVX2
  ASMJIT_INST_4x(vpblendvb, Vpblendvb, Ymm, Ymm, Mem, Ymm)             // AVX2
  ASMJIT_INST_4i(vpblendw, Vpblendw, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vpblendw, Vpblendw, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vpblendw, Vpblendw, Ymm, Ymm, Ymm, Imm)               // AVX2
  ASMJIT_INST_4i(vpblendw, Vpblendw, Ymm, Ymm, Mem, Imm)               // AVX2
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Xmm, Xmm)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Xmm, Mem)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Ymm, Xmm)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Ymm, Mem)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Xmm, Gp)                  //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Ymm, Gp)                  //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Zmm, Gp)                  //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Zmm, Xmm)                 //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastb, Vpbroadcastb, Zmm, Mem)                 //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Xmm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Xmm, Mem)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Ymm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Ymm, Mem)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Xmm, Gp)                  //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Ymm, Gp)                  //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Zmm, Gp)                  //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Zmm, Xmm)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastd, Vpbroadcastd, Zmm, Mem)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastmb2d, Vpbroadcastmb2d, Xmm, KReg)          //      AVX512_CD-VL
  ASMJIT_INST_2x(vpbroadcastmb2d, Vpbroadcastmb2d, Ymm, KReg)          //      AVX512_CD-VL
  ASMJIT_INST_2x(vpbroadcastmb2d, Vpbroadcastmb2d, Zmm, KReg)          //      AVX512_CD
  ASMJIT_INST_2x(vpbroadcastmb2q, Vpbroadcastmb2q, Xmm, KReg)          //      AVX512_CD-VL
  ASMJIT_INST_2x(vpbroadcastmb2q, Vpbroadcastmb2q, Ymm, KReg)          //      AVX512_CD-VL
  ASMJIT_INST_2x(vpbroadcastmb2q, Vpbroadcastmb2q, Zmm, KReg)          //      AVX512_CD
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Xmm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Xmm, Mem)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Ymm, Xmm)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Ymm, Mem)                 // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Xmm, Gp)                  //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Ymm, Gp)                  //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Zmm, Gp)                  //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Zmm, Xmm)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastq, Vpbroadcastq, Zmm, Mem)                 //      AVX512_F{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Xmm, Xmm)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Xmm, Mem)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Ymm, Xmm)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Ymm, Mem)                 // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Xmm, Gp)                  //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Ymm, Gp)                  //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Zmm, Gp)                  //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Zmm, Xmm)                 //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpbroadcastw, Vpbroadcastw, Zmm, Mem)                 //      AVX512_BW{kz}
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Xmm, Xmm, Xmm, Imm)           // AVX  AVX512_F-VL
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Xmm, Xmm, Mem, Imm)           // AVX  AVX512_F-VL
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Ymm, Ymm, Ymm, Imm)           //      AVX512_F-VL VPCLMULQDQ
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Ymm, Ymm, Mem, Imm)           //      AVX512_F-VL VPCLMULQDQ
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Zmm, Zmm, Zmm, Imm)           //      AVX512_F    VPCLMULQDQ
  ASMJIT_INST_4i(vpclmulqdq, Vpclmulqdq, Zmm, Zmm, Mem, Imm)           //      AVX512_F    VPCLMULQDQ
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Xmm, Xmm, Imm)                  //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Xmm, Mem, Imm)                  //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Ymm, Ymm, Imm)                  //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Ymm, Mem, Imm)                  //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Zmm, Zmm, Imm)                  //      AVX512_BW{k}
  ASMJIT_INST_4i(vpcmpb, Vpcmpb, KReg, Zmm, Mem, Imm)                  //      AVX512_BW{k}
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Xmm, Mem, Imm)                  //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Ymm, Ymm, Imm)                  //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Ymm, Mem, Imm)                  //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Zmm, Zmm, Imm)                  //      AVX512_F{k|b32}
  ASMJIT_INST_4i(vpcmpd, Vpcmpd, KReg, Zmm, Mem, Imm)                  //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqb, Vpcmpeqb, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Xmm, Xmm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Xmm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Ymm, Ymm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Ymm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Zmm, Zmm)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqd, Vpcmpeqd, KReg, Zmm, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Xmm, Xmm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Xmm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Ymm, Ymm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Ymm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Zmm, Zmm)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpeqq, Vpcmpeqq, KReg, Zmm, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpeqw, Vpcmpeqw, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_6x(vpcmpestri, Vpcmpestri, Xmm, Xmm, Imm, ECX, EAX, EDX) // AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestri, Vpcmpestri, Xmm, Mem, Imm, ECX, EAX, EDX) // AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestrm, Vpcmpestrm, Xmm, Xmm, Imm, XMM0, EAX, EDX)// AVX  [EXPLICIT]
  ASMJIT_INST_6x(vpcmpestrm, Vpcmpestrm, Xmm, Mem, Imm, XMM0, EAX, EDX)// AVX  [EXPLICIT]
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtb, Vpcmpgtb, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Xmm, Xmm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Xmm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Ymm, Ymm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Ymm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Zmm, Zmm)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpgtd, Vpcmpgtd, KReg, Zmm, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Xmm, Xmm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Xmm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Ymm, Ymm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Ymm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Zmm, Zmm)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpgtq, Vpcmpgtq, KReg, Zmm, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vpcmpgtw, Vpcmpgtw, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_4x(vpcmpistri, Vpcmpistri, Xmm, Xmm, Imm, ECX)           // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistri, Vpcmpistri, Xmm, Mem, Imm, ECX)           // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistrm, Vpcmpistrm, Xmm, Xmm, Imm, XMM0)          // AVX  [EXPLICIT]
  ASMJIT_INST_4x(vpcmpistrm, Vpcmpistrm, Xmm, Mem, Imm, XMM0)          // AVX  [EXPLICIT]
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Xmm, Xmm, Imm)                  //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Xmm, Mem, Imm)                  //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Ymm, Ymm, Imm)                  //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Ymm, Mem, Imm)                  //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Zmm, Zmm, Imm)                  //      AVX512_F{k|b64}
  ASMJIT_INST_4i(vpcmpq, Vpcmpq, KReg, Zmm, Mem, Imm)                  //      AVX512_F{k|b64}
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Xmm, Xmm, Imm)                //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Xmm, Mem, Imm)                //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Ymm, Ymm, Imm)                //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Ymm, Mem, Imm)                //      AVX512_BW{k}-VL
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Zmm, Zmm, Imm)                //      AVX512_BW{k}
  ASMJIT_INST_4i(vpcmpub, Vpcmpub, KReg, Zmm, Mem, Imm)                //      AVX512_BW{k}
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Xmm, Xmm, Imm)                //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Xmm, Mem, Imm)                //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Ymm, Ymm, Imm)                //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Ymm, Mem, Imm)                //      AVX512_F{k|b32}-VL
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Zmm, Zmm, Imm)                //      AVX512_F{k|b32}
  ASMJIT_INST_4i(vpcmpud, Vpcmpud, KReg, Zmm, Mem, Imm)                //      AVX512_F{k|b32}
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Xmm, Xmm, Imm)                //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Xmm, Mem, Imm)                //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Ymm, Ymm, Imm)                //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Ymm, Mem, Imm)                //      AVX512_F{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Zmm, Zmm, Imm)                //      AVX512_F{k|b64}
  ASMJIT_INST_4i(vpcmpuq, Vpcmpuq, KReg, Zmm, Mem, Imm)                //      AVX512_F{k|b64}
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Xmm, Xmm, Imm)                //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Xmm, Mem, Imm)                //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Ymm, Ymm, Imm)                //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Ymm, Mem, Imm)                //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Zmm, Zmm, Imm)                //      AVX512_BW{k|b64}
  ASMJIT_INST_4i(vpcmpuw, Vpcmpuw, KReg, Zmm, Mem, Imm)                //      AVX512_BW{k|b64}
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Xmm, Xmm, Imm)                  //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Xmm, Mem, Imm)                  //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Ymm, Ymm, Imm)                  //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Ymm, Mem, Imm)                  //      AVX512_BW{k|b64}-VL
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Zmm, Zmm, Imm)                  //      AVX512_BW{k|b64}
  ASMJIT_INST_4i(vpcmpw, Vpcmpw, KReg, Zmm, Mem, Imm)                  //      AVX512_BW{k|b64}
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Xmm, Xmm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Mem, Xmm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Ymm, Ymm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Mem, Ymm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Zmm, Zmm)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressb, Vpcompressb, Mem, Zmm)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Xmm, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Mem, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Ymm, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Mem, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Zmm, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressd, Vpcompressd, Mem, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Xmm, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Mem, Xmm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Ymm, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Mem, Ymm)                   //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Zmm, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressq, Vpcompressq, Mem, Zmm)                   //      AVX512_F{kz}
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Xmm, Xmm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Mem, Xmm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Ymm, Ymm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Mem, Ymm)                   //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Zmm, Zmm)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpcompressw, Vpcompressw, Mem, Zmm)                   //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Xmm, Xmm)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Xmm, Mem)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Ymm, Ymm)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Ymm, Mem)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Zmm, Zmm)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictd, Vpconflictd, Zmm, Mem)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Xmm, Xmm)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Xmm, Mem)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Ymm, Ymm)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Ymm, Mem)                   //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Zmm, Zmm)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vpconflictq, Vpconflictq, Zmm, Mem)                   //      AVX512_CD{kz|b32}
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Xmm, Xmm, Xmm)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Xmm, Xmm, Mem)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Ymm, Ymm, Ymm)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Ymm, Ymm, Mem)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Zmm, Zmm, Zmm)                    //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusd, Vpdpbusd, Zmm, Zmm, Mem)                    //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Xmm, Xmm, Xmm)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Xmm, Xmm, Mem)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Ymm, Ymm, Ymm)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Ymm, Ymm, Mem)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Zmm, Zmm, Zmm)                  //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpbusds, Vpdpbusds, Zmm, Zmm, Mem)                  //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Xmm, Xmm, Xmm)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Xmm, Xmm, Mem)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Ymm, Ymm, Ymm)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Ymm, Ymm, Mem)                    //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Zmm, Zmm, Zmm)                    //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssd, Vpdpwssd, Zmm, Zmm, Mem)                    //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Xmm, Xmm, Xmm)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Xmm, Xmm, Mem)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Ymm, Ymm, Ymm)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Ymm, Ymm, Mem)                  //      AVX512_VNNI{kz|b32}-VL
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Zmm, Zmm, Zmm)                  //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_3x(vpdpwssds, Vpdpwssds, Zmm, Zmm, Mem)                  //      AVX512_VNNI{kz|b32}
  ASMJIT_INST_4i(vperm2f128, Vperm2f128, Ymm, Ymm, Ymm, Imm)           // AVX
  ASMJIT_INST_4i(vperm2f128, Vperm2f128, Ymm, Ymm, Mem, Imm)           // AVX
  ASMJIT_INST_4i(vperm2i128, Vperm2i128, Ymm, Ymm, Ymm, Imm)           // AVX2
  ASMJIT_INST_4i(vperm2i128, Vperm2i128, Ymm, Ymm, Mem, Imm)           // AVX2
  ASMJIT_INST_3x(vpermb, Vpermb, Xmm, Xmm, Xmm)                        //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermb, Vpermb, Xmm, Xmm, Mem)                        //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermb, Vpermb, Ymm, Ymm, Ymm)                        //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermb, Vpermb, Ymm, Ymm, Mem)                        //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermb, Vpermb, Zmm, Zmm, Zmm)                        //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermb, Vpermb, Zmm, Zmm, Mem)                        //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermd, Vpermd, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermd, Vpermd, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermd, Vpermd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermd, Vpermd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Xmm, Xmm, Xmm)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Xmm, Xmm, Mem)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Ymm, Ymm, Ymm)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Ymm, Ymm, Mem)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Zmm, Zmm, Zmm)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermi2b, Vpermi2b, Zmm, Zmm, Mem)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2d, Vpermi2d, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2pd, Vpermi2pd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2ps, Vpermi2ps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2q, Vpermi2q, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Xmm, Xmm, Xmm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Xmm, Xmm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Ymm, Ymm, Ymm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Ymm, Ymm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermi2w, Vpermi2w, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Xmm, Xmm, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Xmm, Mem, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Ymm, Ymm, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Ymm, Mem, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilpd, Vpermilpd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Zmm, Zmm, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpermilpd, Vpermilpd, Zmm, Mem, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilps, Vpermilps, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilps, Vpermilps, Xmm, Xmm, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilps, Vpermilps, Xmm, Mem, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilps, Vpermilps, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilps, Vpermilps, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilps, Vpermilps, Ymm, Ymm, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermilps, Vpermilps, Ymm, Mem, Imm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermilps, Vpermilps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermilps, Vpermilps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpermilps, Vpermilps, Zmm, Zmm, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpermilps, Vpermilps, Zmm, Mem, Imm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpermpd, Vpermpd, Ymm, Ymm, Imm)                      // AVX2
  ASMJIT_INST_3i(vpermpd, Vpermpd, Ymm, Mem, Imm)                      // AVX2
  ASMJIT_INST_3x(vpermps, Vpermps, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vpermps, Vpermps, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3i(vpermq, Vpermq, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermq, Vpermq, Ymm, Mem, Imm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermq, Vpermq, Ymm, Ymm, Ymm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermq, Vpermq, Ymm, Ymm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermq, Vpermq, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermq, Vpermq, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermq, Vpermq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpermq, Vpermq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Xmm, Xmm, Xmm)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Xmm, Xmm, Mem)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Ymm, Ymm, Ymm)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Ymm, Ymm, Mem)                    //      AVX512_VBMI{kz}-VL
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Zmm, Zmm, Zmm)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermt2b, Vpermt2b, Zmm, Zmm, Mem)                    //      AVX512_VBMI{kz}
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2d, Vpermt2d, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2pd, Vpermt2pd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2ps, Vpermt2ps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Xmm, Xmm, Xmm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Xmm, Xmm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Ymm, Ymm, Ymm)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Ymm, Ymm, Mem)                    //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2q, Vpermt2q, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Xmm, Xmm, Xmm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Xmm, Xmm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Ymm, Ymm, Ymm)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Ymm, Ymm, Mem)                    //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermt2w, Vpermt2w, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermw, Vpermw, Xmm, Xmm, Xmm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermw, Vpermw, Xmm, Xmm, Mem)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermw, Vpermw, Ymm, Ymm, Ymm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermw, Vpermw, Ymm, Ymm, Mem)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpermw, Vpermw, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpermw, Vpermw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Xmm, Xmm)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Xmm, Mem)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Ymm, Ymm)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Ymm, Mem)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Zmm, Zmm)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandb, Vpexpandb, Zmm, Mem)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandd, Vpexpandd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Xmm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Ymm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Ymm, Mem)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Zmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandq, Vpexpandq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Xmm, Xmm)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Xmm, Mem)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Ymm, Ymm)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Ymm, Mem)                       //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Zmm, Zmm)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_2x(vpexpandw, Vpexpandw, Zmm, Mem)                       //      AVX512_VBMI2{kz}
  ASMJIT_INST_3i(vpextrb, Vpextrb, Gp, Xmm, Imm)                       // AVX  AVX512_BW
  ASMJIT_INST_3i(vpextrb, Vpextrb, Mem, Xmm, Imm)                      // AVX  AVX512_BW
  ASMJIT_INST_3i(vpextrd, Vpextrd, Gp, Xmm, Imm)                       // AVX  AVX512_DQ
  ASMJIT_INST_3i(vpextrd, Vpextrd, Mem, Xmm, Imm)                      // AVX  AVX512_DQ
  ASMJIT_INST_3i(vpextrq, Vpextrq, Gp, Xmm, Imm)                       // AVX  AVX512_DQ
  ASMJIT_INST_3i(vpextrq, Vpextrq, Mem, Xmm, Imm)                      // AVX  AVX512_DQ
  ASMJIT_INST_3i(vpextrw, Vpextrw, Gp, Xmm, Imm)                       // AVX  AVX512_BW
  ASMJIT_INST_3i(vpextrw, Vpextrw, Mem, Xmm, Imm)                      // AVX  AVX512_BW
  ASMJIT_INST_3x(vpgatherdd, Vpgatherdd, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vpgatherdd, Vpgatherdd, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vpgatherdd, Vpgatherdd, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherdd, Vpgatherdd, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherdd, Vpgatherdd, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherdq, Vpgatherdq, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vpgatherdq, Vpgatherdq, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vpgatherdq, Vpgatherdq, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherdq, Vpgatherdq, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherdq, Vpgatherdq, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherqd, Vpgatherqd, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_2x(vpgatherqd, Vpgatherqd, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherqd, Vpgatherqd, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherqd, Vpgatherqd, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vpgatherqq, Vpgatherqq, Xmm, Mem, Xmm)                // AVX2
  ASMJIT_INST_3x(vpgatherqq, Vpgatherqq, Ymm, Mem, Ymm)                // AVX2
  ASMJIT_INST_2x(vpgatherqq, Vpgatherqq, Xmm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherqq, Vpgatherqq, Ymm, Mem)                     //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpgatherqq, Vpgatherqq, Zmm, Mem)                     //      AVX512_F{k}
  ASMJIT_INST_3x(vphaddd, Vphaddd, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vphaddd, Vphaddd, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vphaddd, Vphaddd, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vphaddd, Vphaddd, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vphaddsw, Vphaddsw, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vphaddw, Vphaddw, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vphaddw, Vphaddw, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vphaddw, Vphaddw, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vphaddw, Vphaddw, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_2x(vphminposuw, Vphminposuw, Xmm, Xmm)                   // AVX
  ASMJIT_INST_2x(vphminposuw, Vphminposuw, Xmm, Mem)                   // AVX
  ASMJIT_INST_3x(vphsubd, Vphsubd, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vphsubd, Vphsubd, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vphsubd, Vphsubd, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vphsubd, Vphsubd, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Ymm, Ymm, Ymm)                    // AVX2
  ASMJIT_INST_3x(vphsubsw, Vphsubsw, Ymm, Ymm, Mem)                    // AVX2
  ASMJIT_INST_3x(vphsubw, Vphsubw, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vphsubw, Vphsubw, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vphsubw, Vphsubw, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vphsubw, Vphsubw, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_4i(vpinsrb, Vpinsrb, Xmm, Xmm, Gp, Imm)                  // AVX  AVX512_BW{kz}
  ASMJIT_INST_4i(vpinsrb, Vpinsrb, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_BW{kz}
  ASMJIT_INST_4i(vpinsrd, Vpinsrd, Xmm, Xmm, Gp, Imm)                  // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4i(vpinsrd, Vpinsrd, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4i(vpinsrq, Vpinsrq, Xmm, Xmm, Gp, Imm)                  // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4i(vpinsrq, Vpinsrq, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_DQ{kz}
  ASMJIT_INST_4i(vpinsrw, Vpinsrw, Xmm, Xmm, Gp, Imm)                  // AVX  AVX512_BW{kz}
  ASMJIT_INST_4i(vpinsrw, Vpinsrw, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_BW{kz}
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Xmm, Xmm)                         //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Xmm, Mem)                         //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Ymm, Ymm)                         //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Ymm, Mem)                         //      AVX512_CD{kz|b32}-VL
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Zmm, Zmm)                         //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vplzcntd, Vplzcntd, Zmm, Mem)                         //      AVX512_CD{kz|b32}
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Xmm, Xmm)                         //      AVX512_CD{kz|b64}-VL
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Xmm, Mem)                         //      AVX512_CD{kz|b64}-VL
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Ymm, Ymm)                         //      AVX512_CD{kz|b64}-VL
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Ymm, Mem)                         //      AVX512_CD{kz|b64}-VL
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Zmm, Zmm)                         //      AVX512_CD{kz|b64}
  ASMJIT_INST_2x(vplzcntq, Vplzcntq, Zmm, Mem)                         //      AVX512_CD{kz|b64}
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Xmm, Xmm, Xmm)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Xmm, Xmm, Mem)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Ymm, Ymm, Ymm)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Ymm, Ymm, Mem)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Zmm, Zmm, Zmm)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52huq, Vpmadd52huq, Zmm, Zmm, Mem)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Xmm, Xmm, Xmm)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Xmm, Xmm, Mem)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Ymm, Ymm, Ymm)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Ymm, Ymm, Mem)              //      AVX512_IFMA{kz|b64}-VL
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Zmm, Zmm, Zmm)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmadd52luq, Vpmadd52luq, Zmm, Zmm, Mem)              //      AVX512_IFMA{kz|b64}
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Xmm, Xmm, Xmm)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Xmm, Xmm, Mem)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Ymm, Ymm, Ymm)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Ymm, Ymm, Mem)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Zmm, Zmm, Zmm)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddubsw, Vpmaddubsw, Zmm, Zmm, Mem)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaddwd, Vpmaddwd, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Mem, Xmm, Xmm)                // AVX2
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Mem, Ymm, Ymm)                // AVX2
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Xmm, Xmm, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaskmovd, Vpmaskmovd, Ymm, Ymm, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Mem, Xmm, Xmm)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Mem, Ymm, Ymm)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Xmm, Xmm, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaskmovq, Vpmaskmovq, Ymm, Ymm, Mem)                // AVX2
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsb, Vpmaxsb, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxsd, Vpmaxsd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxsq, Vpmaxsq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxsw, Vpmaxsw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxub, Vpmaxub, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxud, Vpmaxud, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxuq, Vpmaxuq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmaxuw, Vpmaxuw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsb, Vpminsb, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsb, Vpminsb, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsb, Vpminsb, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsb, Vpminsb, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsb, Vpminsb, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsb, Vpminsb, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsd, Vpminsd, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminsd, Vpminsd, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminsd, Vpminsd, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminsd, Vpminsd, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminsd, Vpminsd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminsd, Vpminsd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminsq, Vpminsq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminsq, Vpminsq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminsq, Vpminsq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminsq, Vpminsq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminsq, Vpminsq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminsq, Vpminsq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminsw, Vpminsw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsw, Vpminsw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsw, Vpminsw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsw, Vpminsw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminsw, Vpminsw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminsw, Vpminsw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminub, Vpminub, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminub, Vpminub, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminub, Vpminub, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminub, Vpminub, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminub, Vpminub, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminub, Vpminub, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminud, Vpminud, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminud, Vpminud, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminud, Vpminud, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminud, Vpminud, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpminud, Vpminud, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminud, Vpminud, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpminuq, Vpminuq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminuq, Vpminuq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminuq, Vpminuq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminuq, Vpminuq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpminuq, Vpminuq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminuq, Vpminuq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpminuw, Vpminuw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminuw, Vpminuw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminuw, Vpminuw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminuw, Vpminuw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpminuw, Vpminuw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpminuw, Vpminuw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovb2m, Vpmovb2m, KReg, Xmm)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovb2m, Vpmovb2m, KReg, Ymm)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovb2m, Vpmovb2m, KReg, Zmm)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovd2m, Vpmovd2m, KReg, Xmm)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovd2m, Vpmovd2m, KReg, Ymm)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovd2m, Vpmovd2m, KReg, Zmm)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Xmm, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Mem, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Xmm, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Mem, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Xmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdb, Vpmovdb, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Xmm, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Mem, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Xmm, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Mem, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Ymm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovdw, Vpmovdw, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovm2b, Vpmovm2b, Xmm, KReg)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovm2b, Vpmovm2b, Ymm, KReg)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovm2b, Vpmovm2b, Zmm, KReg)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovm2d, Vpmovm2d, Xmm, KReg)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovm2d, Vpmovm2d, Ymm, KReg)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovm2d, Vpmovm2d, Zmm, KReg)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovm2q, Vpmovm2q, Xmm, KReg)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovm2q, Vpmovm2q, Ymm, KReg)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovm2q, Vpmovm2q, Zmm, KReg)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovm2w, Vpmovm2w, Xmm, KReg)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovm2w, Vpmovm2w, Ymm, KReg)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovm2w, Vpmovm2w, Zmm, KReg)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovmskb, Vpmovmskb, Gp, Xmm)                        // AVX
  ASMJIT_INST_2x(vpmovmskb, Vpmovmskb, Gp, Ymm)                        // AVX2
  ASMJIT_INST_2x(vpmovq2m, Vpmovq2m, KReg, Xmm)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovq2m, Vpmovq2m, KReg, Ymm)                        //      AVX512_DQ-VL
  ASMJIT_INST_2x(vpmovq2m, Vpmovq2m, KReg, Zmm)                        //      AVX512_DQ
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Xmm, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Mem, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Xmm, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Mem, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Xmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqb, Vpmovqb, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Xmm, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Mem, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Xmm, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Mem, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Ymm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqd, Vpmovqd, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Xmm, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Mem, Xmm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Xmm, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Mem, Ymm)                           //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Xmm, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovqw, Vpmovqw, Mem, Zmm)                           //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Xmm, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Mem, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Xmm, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Mem, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Xmm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdb, Vpmovsdb, Mem, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Xmm, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Mem, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Xmm, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Mem, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Ymm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsdw, Vpmovsdw, Mem, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Xmm, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Mem, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Xmm, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Mem, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Xmm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqb, Vpmovsqb, Mem, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Xmm, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Mem, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Xmm, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Mem, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Ymm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqd, Vpmovsqd, Mem, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Xmm, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Mem, Xmm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Xmm, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Mem, Ymm)                         //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Xmm, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsqw, Vpmovsqw, Mem, Zmm)                         //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Xmm, Xmm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Mem, Xmm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Xmm, Ymm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Mem, Ymm)                         //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Ymm, Zmm)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovswb, Vpmovswb, Mem, Zmm)                         //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbd, Vpmovsxbd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbq, Vpmovsxbq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Xmm, Xmm)                       // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Xmm, Mem)                       // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Ymm, Xmm)                       // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Ymm, Mem)                       // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Zmm, Ymm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxbw, Vpmovsxbw, Zmm, Mem)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Zmm, Ymm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxdq, Vpmovsxdq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Zmm, Ymm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwd, Vpmovsxwd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovsxwq, Vpmovsxwq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Xmm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Xmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdb, Vpmovusdb, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Xmm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Ymm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusdw, Vpmovusdw, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Xmm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Xmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqb, Vpmovusqb, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Xmm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Ymm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqd, Vpmovusqd, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Xmm, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Mem, Xmm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Xmm, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Mem, Ymm)                       //      AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Xmm, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovusqw, Vpmovusqw, Mem, Zmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Xmm, Xmm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Mem, Xmm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Xmm, Ymm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Mem, Ymm)                       //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Ymm, Zmm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovuswb, Vpmovuswb, Mem, Zmm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovw2m, Vpmovw2m, KReg, Xmm)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovw2m, Vpmovw2m, KReg, Ymm)                        //      AVX512_BW-VL
  ASMJIT_INST_2x(vpmovw2m, Vpmovw2m, KReg, Zmm)                        //      AVX512_BW
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Xmm, Xmm)                           //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Mem, Xmm)                           //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Xmm, Ymm)                           //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Mem, Ymm)                           //      AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Ymm, Zmm)                           //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovwb, Vpmovwb, Mem, Zmm)                           //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbd, Vpmovzxbd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbq, Vpmovzxbq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Xmm, Xmm)                       // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Xmm, Mem)                       // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Ymm, Xmm)                       // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Ymm, Mem)                       // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Zmm, Ymm)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxbw, Vpmovzxbw, Zmm, Mem)                       //      AVX512_BW{kz}
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Zmm, Ymm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxdq, Vpmovzxdq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Zmm, Ymm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwd, Vpmovzxwd, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Xmm, Xmm)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Xmm, Mem)                       // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Ymm, Xmm)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Ymm, Mem)                       // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Zmm, Xmm)                       //      AVX512_F{kz}
  ASMJIT_INST_2x(vpmovzxwq, Vpmovzxwq, Zmm, Mem)                       //      AVX512_F{kz}
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmuldq, Vpmuldq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Xmm, Xmm, Xmm)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Xmm, Xmm, Mem)                  // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Ymm, Ymm, Ymm)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Ymm, Ymm, Mem)                  // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Zmm, Zmm, Zmm)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhrsw, Vpmulhrsw, Zmm, Zmm, Mem)                  //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhuw, Vpmulhuw, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulhw, Vpmulhw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmulld, Vpmulld, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmulld, Vpmulld, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmulld, Vpmulld, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmulld, Vpmulld, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpmulld, Vpmulld, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmulld, Vpmulld, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpmullq, Vpmullq, Xmm, Xmm, Xmm)                      //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vpmullq, Vpmullq, Xmm, Xmm, Mem)                      //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vpmullq, Vpmullq, Ymm, Ymm, Ymm)                      //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vpmullq, Vpmullq, Ymm, Ymm, Mem)                      //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vpmullq, Vpmullq, Zmm, Zmm, Zmm)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vpmullq, Vpmullq, Zmm, Zmm, Mem)                      //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vpmullw, Vpmullw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmullw, Vpmullw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmullw, Vpmullw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmullw, Vpmullw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpmullw, Vpmullw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmullw, Vpmullw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Xmm, Xmm, Xmm)        //      AVX512_VBMI{kz|b64}-VL
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Xmm, Xmm, Mem)        //      AVX512_VBMI{kz|b64}-VL
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Ymm, Ymm, Ymm)        //      AVX512_VBMI{kz|b64}-VL
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Ymm, Ymm, Mem)        //      AVX512_VBMI{kz|b64}-VL
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Zmm, Zmm, Zmm)        //      AVX512_VBMI{kz|b64}
  ASMJIT_INST_3x(vpmultishiftqb, Vpmultishiftqb, Zmm, Zmm, Mem)        //      AVX512_VBMI{kz|b64}
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Xmm, Xmm, Xmm)                    // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Xmm, Xmm, Mem)                    // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Ymm, Ymm, Ymm)                    // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Ymm, Ymm, Mem)                    // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Zmm, Zmm, Zmm)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpmuludq, Vpmuludq, Zmm, Zmm, Mem)                    //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Xmm, Xmm)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Xmm, Mem)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Ymm, Ymm)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Ymm, Mem)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Zmm, Zmm)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntb, Vpopcntb, Zmm, Mem)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Xmm, Xmm)                         //      AVX512_VPOPCNTDQ{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Xmm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Ymm, Ymm)                         //      AVX512_VPOPCNTDQ{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Ymm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Zmm, Zmm)                         //      AVX512_VPOPCNTDQ{kz|b32}
  ASMJIT_INST_2x(vpopcntd, Vpopcntd, Zmm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b32}
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Xmm, Xmm)                         //      AVX512_VPOPCNTDQ{kz|b64}-VL
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Xmm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b64}-VL
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Ymm, Ymm)                         //      AVX512_VPOPCNTDQ{kz|b64}-VL
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Ymm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b64}-VL
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Zmm, Zmm)                         //      AVX512_VPOPCNTDQ{kz|b64}
  ASMJIT_INST_2x(vpopcntq, Vpopcntq, Zmm, Mem)                         //      AVX512_VPOPCNTDQ{kz|b64}
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Xmm, Xmm)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Xmm, Mem)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Ymm, Ymm)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Ymm, Mem)                         //      AVX512_BITALG{kz|b32}-VL
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Zmm, Zmm)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_2x(vpopcntw, Vpopcntw, Zmm, Mem)                         //      AVX512_BITALG{kz|b32}
  ASMJIT_INST_3x(vpor, Vpor, Xmm, Xmm, Xmm)                            // AVX
  ASMJIT_INST_3x(vpor, Vpor, Xmm, Xmm, Mem)                            // AVX
  ASMJIT_INST_3x(vpor, Vpor, Ymm, Ymm, Ymm)                            // AVX2
  ASMJIT_INST_3x(vpor, Vpor, Ymm, Ymm, Mem)                            // AVX2
  ASMJIT_INST_3x(vpord, Vpord, Xmm, Xmm, Xmm)                          //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpord, Vpord, Xmm, Xmm, Mem)                          //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpord, Vpord, Ymm, Ymm, Ymm)                          //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpord, Vpord, Ymm, Ymm, Mem)                          //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpord, Vpord, Zmm, Zmm, Zmm)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpord, Vpord, Zmm, Zmm, Mem)                          //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vporq, Vporq, Xmm, Xmm, Xmm)                          //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vporq, Vporq, Xmm, Xmm, Mem)                          //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vporq, Vporq, Ymm, Ymm, Ymm)                          //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vporq, Vporq, Ymm, Ymm, Mem)                          //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vporq, Vporq, Zmm, Zmm, Zmm)                          //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vporq, Vporq, Zmm, Zmm, Mem)                          //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vprold, Vprold, Xmm, Xmm, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprold, Vprold, Xmm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprold, Vprold, Ymm, Ymm, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprold, Vprold, Ymm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprold, Vprold, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vprold, Vprold, Zmm, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vprolq, Vprolq, Xmm, Xmm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprolq, Vprolq, Xmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprolq, Vprolq, Ymm, Ymm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprolq, Vprolq, Ymm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprolq, Vprolq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vprolq, Vprolq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprolvd, Vprolvd, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprolvd, Vprolvd, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprolvd, Vprolvd, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprolvd, Vprolvd, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprolvd, Vprolvd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprolvd, Vprolvd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprolvq, Vprolvq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprolvq, Vprolvq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprolvq, Vprolvq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprolvq, Vprolvq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprolvq, Vprolvq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprolvq, Vprolvq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vprord, Vprord, Xmm, Xmm, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprord, Vprord, Xmm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprord, Vprord, Ymm, Ymm, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprord, Vprord, Ymm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vprord, Vprord, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vprord, Vprord, Zmm, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vprorq, Vprorq, Xmm, Xmm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprorq, Vprorq, Xmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprorq, Vprorq, Ymm, Ymm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprorq, Vprorq, Ymm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vprorq, Vprorq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vprorq, Vprorq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprorvd, Vprorvd, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprorvd, Vprorvd, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprorvd, Vprorvd, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprorvd, Vprorvd, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vprorvd, Vprorvd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprorvd, Vprorvd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vprorvq, Vprorvq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprorvq, Vprorvq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprorvq, Vprorvq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprorvq, Vprorvq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vprorvq, Vprorvq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vprorvq, Vprorvq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW-VL
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW-VL
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW-VL
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW-VL
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Zmm, Zmm, Zmm)                      //      AVX512_BW
  ASMJIT_INST_3x(vpsadbw, Vpsadbw, Zmm, Zmm, Mem)                      //      AVX512_BW
  ASMJIT_INST_2x(vpscatterdd, Vpscatterdd, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterdd, Vpscatterdd, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterdd, Vpscatterdd, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterdq, Vpscatterdq, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterdq, Vpscatterdq, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterdq, Vpscatterdq, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterqd, Vpscatterqd, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterqd, Vpscatterqd, Mem, Ymm)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vpscatterqq, Vpscatterqq, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterqq, Vpscatterqq, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vpscatterqq, Vpscatterqq, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_4i(vpshldd, Vpshldd, Xmm, Xmm, Xmm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshldd, Vpshldd, Xmm, Xmm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshldd, Vpshldd, Ymm, Ymm, Ymm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshldd, Vpshldd, Ymm, Ymm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshldd, Vpshldd, Zmm, Zmm, Zmm, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4i(vpshldd, Vpshldd, Zmm, Zmm, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvd, Vpshldvd, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvq, Vpshldvq, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshldvw, Vpshldvw, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Xmm, Xmm, Xmm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Xmm, Xmm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Ymm, Ymm, Ymm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Ymm, Ymm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Zmm, Zmm, Zmm, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4i(vpshrdd, Vpshrdd, Zmm, Zmm, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvd, Vpshrdvd, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvq, Vpshrdvq, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Xmm, Xmm, Xmm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Xmm, Xmm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Ymm, Ymm, Ymm)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Ymm, Ymm, Mem)                    //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Zmm, Zmm, Zmm)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshrdvw, Vpshrdvw, Zmm, Zmm, Mem)                    //      AVX512_VBMI2{kz}
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Xmm, Xmm, Xmm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Xmm, Xmm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Ymm, Ymm, Ymm, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Ymm, Ymm, Mem, Imm)                 //      AVX512_VBMI2{kz}-VL
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Zmm, Zmm, Zmm, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_4i(vpshrdw, Vpshrdw, Zmm, Zmm, Mem, Imm)                 //      AVX512_VBMI2{kz}
  ASMJIT_INST_3x(vpshufb, Vpshufb, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpshufb, Vpshufb, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpshufb, Vpshufb, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpshufb, Vpshufb, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpshufb, Vpshufb, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpshufb, Vpshufb, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Xmm, Xmm)           //      AVX512_BITALG{k}-VL
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Xmm, Mem)           //      AVX512_BITALG{k}-VL
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Ymm, Ymm)           //      AVX512_BITALG{k}-VL
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Ymm, Mem)           //      AVX512_BITALG{k}-VL
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Zmm, Zmm)           //      AVX512_BITALG{k}
  ASMJIT_INST_3x(vpshufbitqmb, Vpshufbitqmb, KReg, Zmm, Mem)           //      AVX512_BITALG{k}
  ASMJIT_INST_3i(vpshufd, Vpshufd, Xmm, Xmm, Imm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpshufd, Vpshufd, Xmm, Mem, Imm)                      // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpshufd, Vpshufd, Ymm, Ymm, Imm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpshufd, Vpshufd, Ymm, Mem, Imm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpshufd, Vpshufd, Zmm, Zmm, Imm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpshufd, Vpshufd, Zmm, Mem, Imm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Xmm, Xmm, Imm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Xmm, Mem, Imm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Ymm, Ymm, Imm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Ymm, Mem, Imm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Zmm, Zmm, Imm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpshufhw, Vpshufhw, Zmm, Mem, Imm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Xmm, Xmm, Imm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Xmm, Mem, Imm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Ymm, Ymm, Imm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Ymm, Mem, Imm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Zmm, Zmm, Imm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpshuflw, Vpshuflw, Zmm, Mem, Imm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsignb, Vpsignb, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vpsignb, Vpsignb, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vpsignb, Vpsignb, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vpsignb, Vpsignb, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3x(vpsignd, Vpsignd, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vpsignd, Vpsignd, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vpsignd, Vpsignd, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vpsignd, Vpsignd, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3x(vpsignw, Vpsignw, Xmm, Xmm, Xmm)                      // AVX
  ASMJIT_INST_3x(vpsignw, Vpsignw, Xmm, Xmm, Mem)                      // AVX
  ASMJIT_INST_3x(vpsignw, Vpsignw, Ymm, Ymm, Ymm)                      // AVX2
  ASMJIT_INST_3x(vpsignw, Vpsignw, Ymm, Ymm, Mem)                      // AVX2
  ASMJIT_INST_3i(vpslld, Vpslld, Xmm, Xmm, Imm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpslld, Vpslld, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpslld, Vpslld, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpslld, Vpslld, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpslld, Vpslld, Ymm, Ymm, Xmm)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpslld, Vpslld, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpslld, Vpslld, Xmm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpslld, Vpslld, Ymm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpslld, Vpslld, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpslld, Vpslld, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpslld, Vpslld, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpslld, Vpslld, Zmm, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpslldq, Vpslldq, Xmm, Xmm, Imm)                      // AVX  AVX512_BW-VL
  ASMJIT_INST_3i(vpslldq, Vpslldq, Ymm, Ymm, Imm)                      // AVX2 AVX512_BW-VL
  ASMJIT_INST_3i(vpslldq, Vpslldq, Xmm, Mem, Imm)                      //      AVX512_BW-VL
  ASMJIT_INST_3i(vpslldq, Vpslldq, Ymm, Mem, Imm)                      //      AVX512_BW-VL
  ASMJIT_INST_3i(vpslldq, Vpslldq, Zmm, Zmm, Imm)                      //      AVX512_BW
  ASMJIT_INST_3i(vpslldq, Vpslldq, Zmm, Mem, Imm)                      //      AVX512_BW
  ASMJIT_INST_3i(vpsllq, Vpsllq, Xmm, Xmm, Imm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllq, Vpsllq, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsllq, Vpsllq, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsllq, Vpsllq, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllq, Vpsllq, Ymm, Ymm, Xmm)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsllq, Vpsllq, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsllq, Vpsllq, Xmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpsllq, Vpsllq, Ymm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllq, Vpsllq, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsllq, Vpsllq, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpsllq, Vpsllq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpsllq, Vpsllq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Xmm, Xmm, Xmm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Xmm, Xmm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsllvd, Vpsllvd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Xmm, Xmm, Xmm)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Xmm, Xmm, Mem)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvq, Vpsllvq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Xmm, Xmm, Xmm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Xmm, Xmm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Ymm, Ymm, Ymm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Ymm, Ymm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllvw, Vpsllvw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsllw, Vpsllw, Xmm, Xmm, Imm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllw, Vpsllw, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllw, Vpsllw, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsllw, Vpsllw, Ymm, Ymm, Imm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllw, Vpsllw, Ymm, Ymm, Xmm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllw, Vpsllw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsllw, Vpsllw, Xmm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsllw, Vpsllw, Ymm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsllw, Vpsllw, Zmm, Zmm, Xmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsllw, Vpsllw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsllw, Vpsllw, Zmm, Zmm, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsllw, Vpsllw, Zmm, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsrad, Vpsrad, Xmm, Xmm, Imm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrad, Vpsrad, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrad, Vpsrad, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrad, Vpsrad, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrad, Vpsrad, Ymm, Ymm, Xmm)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrad, Vpsrad, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrad, Vpsrad, Xmm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpsrad, Vpsrad, Ymm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrad, Vpsrad, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsrad, Vpsrad, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpsrad, Vpsrad, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpsrad, Vpsrad, Zmm, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Xmm, Xmm, Xmm)                        //      AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsraq, Vpsraq, Xmm, Xmm, Mem)                        //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsraq, Vpsraq, Xmm, Xmm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpsraq, Vpsraq, Xmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsraq, Vpsraq, Ymm, Ymm, Xmm)                        //      AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsraq, Vpsraq, Ymm, Ymm, Mem)                        //      AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsraq, Vpsraq, Ymm, Ymm, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpsraq, Vpsraq, Ymm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsraq, Vpsraq, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsraq, Vpsraq, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpsraq, Vpsraq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpsraq, Vpsraq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravd, Vpsravd, Xmm, Xmm, Xmm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsravd, Vpsravd, Xmm, Xmm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsravd, Vpsravd, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsravd, Vpsravd, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsravd, Vpsravd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsravd, Vpsravd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsravq, Vpsravq, Xmm, Xmm, Xmm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsravq, Vpsravq, Xmm, Xmm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsravq, Vpsravq, Ymm, Ymm, Ymm)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsravq, Vpsravq, Ymm, Ymm, Mem)                      //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsravq, Vpsravq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravq, Vpsravq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsravw, Vpsravw, Xmm, Xmm, Xmm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsravw, Vpsravw, Xmm, Xmm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsravw, Vpsravw, Ymm, Ymm, Ymm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsravw, Vpsravw, Ymm, Ymm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsravw, Vpsravw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsravw, Vpsravw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsraw, Vpsraw, Xmm, Xmm, Imm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsraw, Vpsraw, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsraw, Vpsraw, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsraw, Vpsraw, Ymm, Ymm, Imm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsraw, Vpsraw, Ymm, Ymm, Xmm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsraw, Vpsraw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsraw, Vpsraw, Xmm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsraw, Vpsraw, Ymm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsraw, Vpsraw, Zmm, Zmm, Xmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsraw, Vpsraw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsraw, Vpsraw, Zmm, Zmm, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsraw, Vpsraw, Zmm, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsrld, Vpsrld, Xmm, Xmm, Imm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrld, Vpsrld, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrld, Vpsrld, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrld, Vpsrld, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrld, Vpsrld, Ymm, Ymm, Xmm)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrld, Vpsrld, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrld, Vpsrld, Xmm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vpsrld, Vpsrld, Ymm, Mem, Imm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrld, Vpsrld, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsrld, Vpsrld, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpsrld, Vpsrld, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpsrld, Vpsrld, Zmm, Mem, Imm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Xmm, Xmm, Imm)                      // AVX  AVX512_BW-VL
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Ymm, Ymm, Imm)                      // AVX2 AVX512_BW-VL
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Xmm, Mem, Imm)                      //      AVX512_BW-VL
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Ymm, Mem, Imm)                      //      AVX512_BW-VL
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Zmm, Zmm, Imm)                      //      AVX512_BW
  ASMJIT_INST_3i(vpsrldq, Vpsrldq, Zmm, Mem, Imm)                      //      AVX512_BW
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Xmm, Xmm, Imm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Ymm, Ymm, Imm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Ymm, Ymm, Xmm)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz}-VL
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Xmm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Ymm, Mem, Imm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Zmm, Zmm, Xmm)                        //      AVX512_F{kz}
  ASMJIT_INST_3x(vpsrlq, Vpsrlq, Zmm, Zmm, Mem)                        //      AVX512_F{kz}
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Zmm, Zmm, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3i(vpsrlq, Vpsrlq, Zmm, Mem, Imm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Xmm, Xmm, Xmm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Xmm, Xmm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrlvd, Vpsrlvd, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Xmm, Xmm, Xmm)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Xmm, Xmm, Mem)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Ymm, Ymm, Ymm)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Ymm, Ymm, Mem)                      // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Zmm, Zmm, Zmm)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvq, Vpsrlvq, Zmm, Zmm, Mem)                      //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Xmm, Xmm, Xmm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Xmm, Xmm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Ymm, Ymm, Ymm)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Ymm, Ymm, Mem)                      //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlvw, Vpsrlvw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Xmm, Xmm, Imm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Ymm, Ymm, Imm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Ymm, Ymm, Xmm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Xmm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Ymm, Mem, Imm)                        //      AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Zmm, Zmm, Xmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsrlw, Vpsrlw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Zmm, Zmm, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3i(vpsrlw, Vpsrlw, Zmm, Mem, Imm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubb, Vpsubb, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubb, Vpsubb, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubb, Vpsubb, Ymm, Ymm, Ymm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubb, Vpsubb, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubb, Vpsubb, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubb, Vpsubb, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubd, Vpsubd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsubd, Vpsubd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsubd, Vpsubd, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsubd, Vpsubd, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpsubd, Vpsubd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsubd, Vpsubd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpsubq, Vpsubq, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsubq, Vpsubq, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsubq, Vpsubq, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsubq, Vpsubq, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpsubq, Vpsubq, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsubq, Vpsubq, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsb, Vpsubsb, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Xmm, Xmm, Xmm)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Xmm, Xmm, Mem)                      // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Ymm, Ymm, Ymm)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Ymm, Ymm, Mem)                      // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Zmm, Zmm, Zmm)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubsw, Vpsubsw, Zmm, Zmm, Mem)                      //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusb, Vpsubusb, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Xmm, Xmm, Xmm)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Xmm, Xmm, Mem)                    // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Ymm, Ymm, Ymm)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Ymm, Ymm, Mem)                    // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Zmm, Zmm, Zmm)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubusw, Vpsubusw, Zmm, Zmm, Mem)                    //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubw, Vpsubw, Xmm, Xmm, Xmm)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubw, Vpsubw, Xmm, Xmm, Mem)                        // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubw, Vpsubw, Ymm, Ymm, Ymm)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubw, Vpsubw, Ymm, Ymm, Mem)                        // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpsubw, Vpsubw, Zmm, Zmm, Zmm)                        //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpsubw, Vpsubw, Zmm, Zmm, Mem)                        //      AVX512_BW{kz}
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Xmm, Xmm, Xmm, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Xmm, Xmm, Mem, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vpternlogd, Vpternlogd, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Xmm, Xmm, Xmm, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Xmm, Xmm, Mem, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vpternlogq, Vpternlogq, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vptest, Vptest, Xmm, Xmm)                             // AVX
  ASMJIT_INST_2x(vptest, Vptest, Xmm, Mem)                             // AVX
  ASMJIT_INST_2x(vptest, Vptest, Ymm, Ymm)                             // AVX
  ASMJIT_INST_2x(vptest, Vptest, Ymm, Mem)                             // AVX
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmb, Vptestmb, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Xmm, Xmm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Xmm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Ymm, Ymm)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Ymm, Mem)                   //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Zmm, Zmm)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestmd, Vptestmd, KReg, Zmm, Mem)                   //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Xmm, Xmm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Xmm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Ymm, Ymm)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Ymm, Mem)                   //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Zmm, Zmm)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestmq, Vptestmq, KReg, Zmm, Mem)                   //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Xmm, Xmm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Xmm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Ymm, Ymm)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Ymm, Mem)                   //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Zmm, Zmm)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestmw, Vptestmw, KReg, Zmm, Mem)                   //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Xmm, Xmm)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Xmm, Mem)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Ymm, Ymm)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Ymm, Mem)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Zmm, Zmm)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmb, Vptestnmb, KReg, Zmm, Mem)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Xmm, Xmm)                 //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Xmm, Mem)                 //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Ymm, Ymm)                 //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Ymm, Mem)                 //      AVX512_F{k|b32}-VL
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Zmm, Zmm)                 //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestnmd, Vptestnmd, KReg, Zmm, Mem)                 //      AVX512_F{k|b32}
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Xmm, Xmm)                 //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Xmm, Mem)                 //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Ymm, Ymm)                 //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Ymm, Mem)                 //      AVX512_F{k|b64}-VL
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Zmm, Zmm)                 //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestnmq, Vptestnmq, KReg, Zmm, Mem)                 //      AVX512_F{k|b64}
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Xmm, Xmm)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Xmm, Mem)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Ymm, Ymm)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Ymm, Mem)                 //      AVX512_BW{k}-VL
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Zmm, Zmm)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vptestnmw, Vptestnmw, KReg, Zmm, Mem)                 //      AVX512_BW{k}
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Xmm, Xmm, Xmm)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Xmm, Xmm, Mem)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Ymm, Ymm, Ymm)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Ymm, Ymm, Mem)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Zmm, Zmm, Zmm)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhbw, Vpunpckhbw, Zmm, Zmm, Mem)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Xmm, Xmm, Xmm)                // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Xmm, Xmm, Mem)                // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Ymm, Ymm, Ymm)                // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Ymm, Ymm, Mem)                // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Zmm, Zmm, Zmm)                //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckhdq, Vpunpckhdq, Zmm, Zmm, Mem)                //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Xmm, Xmm, Xmm)              // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Xmm, Xmm, Mem)              // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Ymm, Ymm, Ymm)              // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Ymm, Ymm, Mem)              // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Zmm, Zmm, Zmm)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpckhqdq, Vpunpckhqdq, Zmm, Zmm, Mem)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Xmm, Xmm, Xmm)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Xmm, Xmm, Mem)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Ymm, Ymm, Ymm)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Ymm, Ymm, Mem)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Zmm, Zmm, Zmm)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckhwd, Vpunpckhwd, Zmm, Zmm, Mem)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Xmm, Xmm, Xmm)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Xmm, Xmm, Mem)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Ymm, Ymm, Ymm)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Ymm, Ymm, Mem)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Zmm, Zmm, Zmm)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklbw, Vpunpcklbw, Zmm, Zmm, Mem)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Xmm, Xmm, Xmm)                // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Xmm, Xmm, Mem)                // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Ymm, Ymm, Ymm)                // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Ymm, Ymm, Mem)                // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Zmm, Zmm, Zmm)                //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpckldq, Vpunpckldq, Zmm, Zmm, Mem)                //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Xmm, Xmm, Xmm)              // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Xmm, Xmm, Mem)              // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Ymm, Ymm, Ymm)              // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Ymm, Ymm, Mem)              // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Zmm, Zmm, Zmm)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpcklqdq, Vpunpcklqdq, Zmm, Zmm, Mem)              //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Xmm, Xmm, Xmm)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Xmm, Xmm, Mem)                // AVX  AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Ymm, Ymm, Ymm)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Ymm, Ymm, Mem)                // AVX2 AVX512_BW{kz}-VL
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Zmm, Zmm, Zmm)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpunpcklwd, Vpunpcklwd, Zmm, Zmm, Mem)                //      AVX512_BW{kz}
  ASMJIT_INST_3x(vpxor, Vpxor, Xmm, Xmm, Xmm)                          // AVX
  ASMJIT_INST_3x(vpxor, Vpxor, Xmm, Xmm, Mem)                          // AVX
  ASMJIT_INST_3x(vpxor, Vpxor, Ymm, Ymm, Ymm)                          // AVX2
  ASMJIT_INST_3x(vpxor, Vpxor, Ymm, Ymm, Mem)                          // AVX2
  ASMJIT_INST_3x(vpxord, Vpxord, Xmm, Xmm, Xmm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpxord, Vpxord, Xmm, Xmm, Mem)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpxord, Vpxord, Ymm, Ymm, Ymm)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpxord, Vpxord, Ymm, Ymm, Mem)                        //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vpxord, Vpxord, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpxord, Vpxord, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vpxorq, Vpxorq, Xmm, Xmm, Xmm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpxorq, Vpxorq, Xmm, Xmm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpxorq, Vpxorq, Ymm, Ymm, Ymm)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpxorq, Vpxorq, Ymm, Ymm, Mem)                        //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vpxorq, Vpxorq, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vpxorq, Vpxorq, Zmm, Zmm, Mem)                        //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vrangepd, Vrangepd, Xmm, Xmm, Xmm, Imm)               //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_4i(vrangepd, Vrangepd, Xmm, Xmm, Mem, Imm)               //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_4i(vrangepd, Vrangepd, Ymm, Ymm, Ymm, Imm)               //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_4i(vrangepd, Vrangepd, Ymm, Ymm, Mem, Imm)               //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_4i(vrangepd, Vrangepd, Zmm, Zmm, Zmm, Imm)               //      AVX512_DQ{kz|sae|b64}
  ASMJIT_INST_4i(vrangepd, Vrangepd, Zmm, Zmm, Mem, Imm)               //      AVX512_DQ{kz|sae|b64}
  ASMJIT_INST_4i(vrangeps, Vrangeps, Xmm, Xmm, Xmm, Imm)               //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_4i(vrangeps, Vrangeps, Xmm, Xmm, Mem, Imm)               //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_4i(vrangeps, Vrangeps, Ymm, Ymm, Ymm, Imm)               //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_4i(vrangeps, Vrangeps, Ymm, Ymm, Mem, Imm)               //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_4i(vrangeps, Vrangeps, Zmm, Zmm, Zmm, Imm)               //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_4i(vrangeps, Vrangeps, Zmm, Zmm, Mem, Imm)               //      AVX512_DQ{kz|sae|b32}
  ASMJIT_INST_4i(vrangesd, Vrangesd, Xmm, Xmm, Xmm, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4i(vrangesd, Vrangesd, Xmm, Xmm, Mem, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4i(vrangess, Vrangess, Xmm, Xmm, Xmm, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_4i(vrangess, Vrangess, Xmm, Xmm, Mem, Imm)               //      AVX512_DQ{kz|sae}
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Xmm, Xmm)                         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Xmm, Mem)                         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Ymm, Ymm)                         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Ymm, Mem)                         //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Zmm, Zmm)                         //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrcp14pd, Vrcp14pd, Zmm, Mem)                         //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Xmm, Xmm)                         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Xmm, Mem)                         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Ymm, Ymm)                         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Ymm, Mem)                         //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Zmm, Zmm)                         //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vrcp14ps, Vrcp14ps, Zmm, Mem)                         //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vrcp14sd, Vrcp14sd, Xmm, Xmm, Xmm)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14sd, Vrcp14sd, Xmm, Xmm, Mem)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14ss, Vrcp14ss, Xmm, Xmm, Xmm)                    //      AVX512_F{kz}
  ASMJIT_INST_3x(vrcp14ss, Vrcp14ss, Xmm, Xmm, Mem)                    //      AVX512_F{kz}
  ASMJIT_INST_2x(vrcp28pd, Vrcp28pd, Zmm, Zmm)                         //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vrcp28pd, Vrcp28pd, Zmm, Mem)                         //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vrcp28ps, Vrcp28ps, Zmm, Zmm)                         //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_2x(vrcp28ps, Vrcp28ps, Zmm, Mem)                         //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_3x(vrcp28sd, Vrcp28sd, Xmm, Xmm, Xmm)                    //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrcp28sd, Vrcp28sd, Xmm, Xmm, Mem)                    //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrcp28ss, Vrcp28ss, Xmm, Xmm, Xmm)                    //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrcp28ss, Vrcp28ss, Xmm, Xmm, Mem)                    //      AVX512_ER{kz|sae}
  ASMJIT_INST_2x(vrcpps, Vrcpps, Xmm, Xmm)                             // AVX
  ASMJIT_INST_2x(vrcpps, Vrcpps, Xmm, Mem)                             // AVX
  ASMJIT_INST_2x(vrcpps, Vrcpps, Ymm, Ymm)                             // AVX
  ASMJIT_INST_2x(vrcpps, Vrcpps, Ymm, Mem)                             // AVX
  ASMJIT_INST_3x(vrcpss, Vrcpss, Xmm, Xmm, Xmm)                        // AVX
  ASMJIT_INST_3x(vrcpss, Vrcpss, Xmm, Xmm, Mem)                        // AVX
  ASMJIT_INST_3i(vreducepd, Vreducepd, Xmm, Xmm, Imm)                  //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3i(vreducepd, Vreducepd, Xmm, Mem, Imm)                  //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3i(vreducepd, Vreducepd, Ymm, Ymm, Imm)                  //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3i(vreducepd, Vreducepd, Ymm, Mem, Imm)                  //      AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3i(vreducepd, Vreducepd, Zmm, Zmm, Imm)                  //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3i(vreducepd, Vreducepd, Zmm, Mem, Imm)                  //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3i(vreduceps, Vreduceps, Xmm, Xmm, Imm)                  //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3i(vreduceps, Vreduceps, Xmm, Mem, Imm)                  //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3i(vreduceps, Vreduceps, Ymm, Ymm, Imm)                  //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3i(vreduceps, Vreduceps, Ymm, Mem, Imm)                  //      AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3i(vreduceps, Vreduceps, Zmm, Zmm, Imm)                  //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3i(vreduceps, Vreduceps, Zmm, Mem, Imm)                  //      AVX512_DQ{kz|b32}
  ASMJIT_INST_4i(vreducesd, Vreducesd, Xmm, Xmm, Xmm, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vreducesd, Vreducesd, Xmm, Xmm, Mem, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vreducess, Vreducess, Xmm, Xmm, Xmm, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_4i(vreducess, Vreducess, Xmm, Xmm, Mem, Imm)             //      AVX512_DQ{kz}
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Xmm, Xmm, Imm)              //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Xmm, Mem, Imm)              //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Ymm, Ymm, Imm)              //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Ymm, Mem, Imm)              //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Zmm, Zmm, Imm)              //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3i(vrndscalepd, Vrndscalepd, Zmm, Mem, Imm)              //      AVX512_F{kz|sae|b64}
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Xmm, Xmm, Imm)              //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Xmm, Mem, Imm)              //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Ymm, Ymm, Imm)              //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Ymm, Mem, Imm)              //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Zmm, Zmm, Imm)              //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_3i(vrndscaleps, Vrndscaleps, Zmm, Mem, Imm)              //      AVX512_F{kz|sae|b32}
  ASMJIT_INST_4i(vrndscalesd, Vrndscalesd, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vrndscalesd, Vrndscalesd, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vrndscaless, Vrndscaless, Xmm, Xmm, Xmm, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_4i(vrndscaless, Vrndscaless, Xmm, Xmm, Mem, Imm)         //      AVX512_F{kz|sae}
  ASMJIT_INST_3i(vroundpd, Vroundpd, Xmm, Xmm, Imm)                    // AVX
  ASMJIT_INST_3i(vroundpd, Vroundpd, Xmm, Mem, Imm)                    // AVX
  ASMJIT_INST_3i(vroundpd, Vroundpd, Ymm, Ymm, Imm)                    // AVX
  ASMJIT_INST_3i(vroundpd, Vroundpd, Ymm, Mem, Imm)                    // AVX
  ASMJIT_INST_3i(vroundps, Vroundps, Xmm, Xmm, Imm)                    // AVX
  ASMJIT_INST_3i(vroundps, Vroundps, Xmm, Mem, Imm)                    // AVX
  ASMJIT_INST_3i(vroundps, Vroundps, Ymm, Ymm, Imm)                    // AVX
  ASMJIT_INST_3i(vroundps, Vroundps, Ymm, Mem, Imm)                    // AVX
  ASMJIT_INST_4i(vroundsd, Vroundsd, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vroundsd, Vroundsd, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_4i(vroundss, Vroundss, Xmm, Xmm, Xmm, Imm)               // AVX
  ASMJIT_INST_4i(vroundss, Vroundss, Xmm, Xmm, Mem, Imm)               // AVX
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Xmm, Xmm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Xmm, Mem)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Ymm, Ymm)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Ymm, Mem)                     //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Zmm, Zmm)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrsqrt14pd, Vrsqrt14pd, Zmm, Mem)                     //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Xmm, Xmm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Xmm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Ymm, Ymm)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Ymm, Mem)                     //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Zmm, Zmm)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_2x(vrsqrt14ps, Vrsqrt14ps, Zmm, Mem)                     //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vrsqrt14sd, Vrsqrt14sd, Xmm, Xmm, Xmm)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14sd, Vrsqrt14sd, Xmm, Xmm, Mem)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14ss, Vrsqrt14ss, Xmm, Xmm, Xmm)                //      AVX512_F{kz}
  ASMJIT_INST_3x(vrsqrt14ss, Vrsqrt14ss, Xmm, Xmm, Mem)                //      AVX512_F{kz}
  ASMJIT_INST_2x(vrsqrt28pd, Vrsqrt28pd, Zmm, Zmm)                     //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vrsqrt28pd, Vrsqrt28pd, Zmm, Mem)                     //      AVX512_ER{kz|sae|b64}
  ASMJIT_INST_2x(vrsqrt28ps, Vrsqrt28ps, Zmm, Zmm)                     //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_2x(vrsqrt28ps, Vrsqrt28ps, Zmm, Mem)                     //      AVX512_ER{kz|sae|b32}
  ASMJIT_INST_3x(vrsqrt28sd, Vrsqrt28sd, Xmm, Xmm, Xmm)                //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrsqrt28sd, Vrsqrt28sd, Xmm, Xmm, Mem)                //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrsqrt28ss, Vrsqrt28ss, Xmm, Xmm, Xmm)                //      AVX512_ER{kz|sae}
  ASMJIT_INST_3x(vrsqrt28ss, Vrsqrt28ss, Xmm, Xmm, Mem)                //      AVX512_ER{kz|sae}
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Xmm, Xmm)                         // AVX
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Xmm, Mem)                         // AVX
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Ymm, Ymm)                         // AVX
  ASMJIT_INST_2x(vrsqrtps, Vrsqrtps, Ymm, Mem)                         // AVX
  ASMJIT_INST_3x(vrsqrtss, Vrsqrtss, Xmm, Xmm, Xmm)                    // AVX
  ASMJIT_INST_3x(vrsqrtss, Vrsqrtss, Xmm, Xmm, Mem)                    // AVX
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vscalefpd, Vscalefpd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vscalefps, Vscalefps, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vscalefps, Vscalefps, Xmm, Xmm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vscalefps, Vscalefps, Ymm, Ymm, Ymm)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vscalefps, Vscalefps, Ymm, Ymm, Mem)                  //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vscalefps, Vscalefps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vscalefps, Vscalefps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vscalefsd, Vscalefsd, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefsd, Vscalefsd, Xmm, Xmm, Mem)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefss, Vscalefss, Xmm, Xmm, Xmm)                  //      AVX512_F{kz|er}
  ASMJIT_INST_3x(vscalefss, Vscalefss, Xmm, Xmm, Mem)                  //      AVX512_F{kz|er}
  ASMJIT_INST_2x(vscatterdpd, Vscatterdpd, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterdpd, Vscatterdpd, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterdpd, Vscatterdpd, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vscatterdps, Vscatterdps, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterdps, Vscatterdps, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterdps, Vscatterdps, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_1x(vscatterpf0dpd, Vscatterpf0dpd, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf0dps, Vscatterpf0dps, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf0qpd, Vscatterpf0qpd, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf0qps, Vscatterpf0qps, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf1dpd, Vscatterpf1dpd, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf1dps, Vscatterpf1dps, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf1qpd, Vscatterpf1qpd, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_1x(vscatterpf1qps, Vscatterpf1qps, Mem)                  //      AVX512_PF{k}
  ASMJIT_INST_2x(vscatterqpd, Vscatterqpd, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterqpd, Vscatterqpd, Mem, Ymm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterqpd, Vscatterqpd, Mem, Zmm)                   //      AVX512_F{k}
  ASMJIT_INST_2x(vscatterqps, Vscatterqps, Mem, Xmm)                   //      AVX512_F{k}-VL
  ASMJIT_INST_2x(vscatterqps, Vscatterqps, Mem, Ymm)                   //      AVX512_F{k}
  ASMJIT_INST_4i(vshuff32x4, Vshuff32x4, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshuff32x4, Vshuff32x4, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshuff32x4, Vshuff32x4, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshuff32x4, Vshuff32x4, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshuff64x2, Vshuff64x2, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshuff64x2, Vshuff64x2, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshuff64x2, Vshuff64x2, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vshuff64x2, Vshuff64x2, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vshufi32x4, Vshufi32x4, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufi32x4, Vshufi32x4, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufi32x4, Vshufi32x4, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshufi32x4, Vshufi32x4, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshufi64x2, Vshufi64x2, Ymm, Ymm, Ymm, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufi64x2, Vshufi64x2, Ymm, Ymm, Mem, Imm)           //      AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufi64x2, Vshufi64x2, Zmm, Zmm, Zmm, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vshufi64x2, Vshufi64x2, Zmm, Zmm, Mem, Imm)           //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vshufpd, Vshufpd, Xmm, Xmm, Xmm, Imm)                 // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufpd, Vshufpd, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufpd, Vshufpd, Ymm, Ymm, Ymm, Imm)                 // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufpd, Vshufpd, Ymm, Ymm, Mem, Imm)                 // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_4i(vshufpd, Vshufpd, Zmm, Zmm, Zmm, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshufpd, Vshufpd, Zmm, Zmm, Mem, Imm)                 //      AVX512_F{kz|b32}
  ASMJIT_INST_4i(vshufps, Vshufps, Xmm, Xmm, Xmm, Imm)                 // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufps, Vshufps, Xmm, Xmm, Mem, Imm)                 // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufps, Vshufps, Ymm, Ymm, Ymm, Imm)                 // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufps, Vshufps, Ymm, Ymm, Mem, Imm)                 // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_4i(vshufps, Vshufps, Zmm, Zmm, Zmm, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_4i(vshufps, Vshufps, Zmm, Zmm, Mem, Imm)                 //      AVX512_F{kz|b64}
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Xmm, Xmm)                           // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Xmm, Mem)                           // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Ymm, Ymm)                           // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Ymm, Mem)                           // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Zmm, Zmm)                           //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vsqrtpd, Vsqrtpd, Zmm, Mem)                           //      AVX512_F{kz|er|b64}
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Xmm, Xmm)                           // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Xmm, Mem)                           // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Ymm, Ymm)                           // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Ymm, Mem)                           // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Zmm, Zmm)                           //      AVX512_F{kz|er|b32}
  ASMJIT_INST_2x(vsqrtps, Vsqrtps, Zmm, Mem)                           //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vsqrtsd, Vsqrtsd, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtsd, Vsqrtsd, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtss, Vsqrtss, Xmm, Xmm, Xmm)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsqrtss, Vsqrtss, Xmm, Xmm, Mem)                      // AVX  AVX512_F{kz|er}
  ASMJIT_INST_1x(vstmxcsr, Vstmxcsr, Mem)                              // AVX
  ASMJIT_INST_3x(vsubpd, Vsubpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vsubpd, Vsubpd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vsubpd, Vsubpd, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vsubpd, Vsubpd, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vsubpd, Vsubpd, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vsubpd, Vsubpd, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b64}
  ASMJIT_INST_3x(vsubps, Vsubps, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vsubps, Vsubps, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vsubps, Vsubps, Ymm, Ymm, Ymm)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vsubps, Vsubps, Ymm, Ymm, Mem)                        // AVX2 AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vsubps, Vsubps, Zmm, Zmm, Zmm)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vsubps, Vsubps, Zmm, Zmm, Mem)                        //      AVX512_F{kz|er|b32}
  ASMJIT_INST_3x(vsubsd, Vsubsd, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubsd, Vsubsd, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubss, Vsubss, Xmm, Xmm, Xmm)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_3x(vsubss, Vsubss, Xmm, Xmm, Mem)                        // AVX  AVX512_F{kz|er}
  ASMJIT_INST_2x(vtestpd, Vtestpd, Xmm, Xmm)                           // AVX
  ASMJIT_INST_2x(vtestpd, Vtestpd, Xmm, Mem)                           // AVX
  ASMJIT_INST_2x(vtestpd, Vtestpd, Ymm, Ymm)                           // AVX
  ASMJIT_INST_2x(vtestpd, Vtestpd, Ymm, Mem)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Xmm, Xmm)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Xmm, Mem)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Ymm, Ymm)                           // AVX
  ASMJIT_INST_2x(vtestps, Vtestps, Ymm, Mem)                           // AVX
  ASMJIT_INST_2x(vucomisd, Vucomisd, Xmm, Xmm)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomisd, Vucomisd, Xmm, Mem)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomiss, Vucomiss, Xmm, Xmm)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_2x(vucomiss, Vucomiss, Xmm, Mem)                         // AVX  AVX512_F{sae}
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpckhpd, Vunpckhpd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpckhps, Vunpckhps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b64}-VL
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpcklpd, Vunpcklpd, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b64}
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Xmm, Xmm, Xmm)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Xmm, Xmm, Mem)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Ymm, Ymm, Ymm)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Ymm, Ymm, Mem)                  // AVX  AVX512_F{kz|b32}-VL
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Zmm, Zmm, Zmm)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vunpcklps, Vunpcklps, Zmm, Zmm, Mem)                  //      AVX512_F{kz|b32}
  ASMJIT_INST_3x(vxorpd, Vxorpd, Xmm, Xmm, Xmm)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vxorpd, Vxorpd, Xmm, Xmm, Mem)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vxorpd, Vxorpd, Ymm, Ymm, Ymm)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vxorpd, Vxorpd, Ymm, Ymm, Mem)                        // AVX  AVX512_DQ{kz|b64}-VL
  ASMJIT_INST_3x(vxorpd, Vxorpd, Zmm, Zmm, Zmm)                        //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vxorpd, Vxorpd, Zmm, Zmm, Mem)                        //      AVX512_DQ{kz|b64}
  ASMJIT_INST_3x(vxorps, Vxorps, Xmm, Xmm, Xmm)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vxorps, Vxorps, Xmm, Xmm, Mem)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vxorps, Vxorps, Ymm, Ymm, Ymm)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vxorps, Vxorps, Ymm, Ymm, Mem)                        // AVX  AVX512_DQ{kz|b32}-VL
  ASMJIT_INST_3x(vxorps, Vxorps, Zmm, Zmm, Zmm)                        //      AVX512_DQ{kz|b32}
  ASMJIT_INST_3x(vxorps, Vxorps, Zmm, Zmm, Mem)                        //      AVX512_DQ{kz|b32}
  ASMJIT_INST_0x(vzeroall, Vzeroall)                                   // AVX
  ASMJIT_INST_0x(vzeroupper, Vzeroupper)                               // AVX

  //! \}

  //! \name FMA4 Instructions
  //! \{

  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Ymm, Ymm, Ymm, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Ymm, Ymm, Mem, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmaddpd, Vfmaddpd, Ymm, Ymm, Ymm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Ymm, Ymm, Ymm, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Ymm, Ymm, Mem, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmaddps, Vfmaddps, Ymm, Ymm, Ymm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddsd, Vfmaddsd, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmaddss, Vfmaddss, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Xmm, Xmm, Xmm, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Xmm, Xmm, Mem, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Xmm, Xmm, Xmm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Ymm, Ymm, Ymm, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Ymm, Ymm, Mem, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubpd, Vfmaddsubpd, Ymm, Ymm, Ymm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Xmm, Xmm, Xmm, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Xmm, Xmm, Mem, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Xmm, Xmm, Xmm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Ymm, Ymm, Ymm, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Ymm, Ymm, Mem, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmaddsubps, Vfmaddsubps, Ymm, Ymm, Ymm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Xmm, Xmm, Xmm, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Xmm, Xmm, Mem, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Xmm, Xmm, Xmm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Ymm, Ymm, Ymm, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Ymm, Ymm, Mem, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddpd, Vfmsubaddpd, Ymm, Ymm, Ymm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Xmm, Xmm, Xmm, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Xmm, Xmm, Mem, Xmm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Xmm, Xmm, Xmm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Ymm, Ymm, Ymm, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Ymm, Ymm, Mem, Ymm)         // FMA4
  ASMJIT_INST_4x(vfmsubaddps, Vfmsubaddps, Ymm, Ymm, Ymm, Mem)         // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Ymm, Ymm, Ymm, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Ymm, Ymm, Mem, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmsubpd, Vfmsubpd, Ymm, Ymm, Ymm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Ymm, Ymm, Ymm, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Ymm, Ymm, Mem, Ymm)               // FMA4
  ASMJIT_INST_4x(vfmsubps, Vfmsubps, Ymm, Ymm, Ymm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubsd, Vfmsubsd, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Xmm, Xmm, Xmm, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Xmm, Xmm, Mem, Xmm)               // FMA4
  ASMJIT_INST_4x(vfmsubss, Vfmsubss, Xmm, Xmm, Xmm, Mem)               // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Ymm, Ymm, Ymm, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Ymm, Ymm, Mem, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmaddpd, Vfnmaddpd, Ymm, Ymm, Ymm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Ymm, Ymm, Ymm, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Ymm, Ymm, Mem, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmaddps, Vfnmaddps, Ymm, Ymm, Ymm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddsd, Vfnmaddsd, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmaddss, Vfnmaddss, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Ymm, Ymm, Ymm, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Ymm, Ymm, Mem, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmsubpd, Vfnmsubpd, Ymm, Ymm, Ymm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Ymm, Ymm, Ymm, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Ymm, Ymm, Mem, Ymm)             // FMA4
  ASMJIT_INST_4x(vfnmsubps, Vfnmsubps, Ymm, Ymm, Ymm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubsd, Vfnmsubsd, Xmm, Xmm, Xmm, Mem)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Xmm, Xmm, Xmm, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Xmm, Xmm, Mem, Xmm)             // FMA4
  ASMJIT_INST_4x(vfnmsubss, Vfnmsubss, Xmm, Xmm, Xmm, Mem)             // FMA4

  //! \}

  //! \name XOP Instructions (Deprecated)
  //! \{

  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Xmm, Xmm)                           // XOP
  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Xmm, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Ymm, Ymm)                           // XOP
  ASMJIT_INST_2x(vfrczpd, Vfrczpd, Ymm, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Xmm, Xmm)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Xmm, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Ymm, Ymm)                           // XOP
  ASMJIT_INST_2x(vfrczps, Vfrczps, Ymm, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczsd, Vfrczsd, Xmm, Xmm)                           // XOP
  ASMJIT_INST_2x(vfrczsd, Vfrczsd, Xmm, Mem)                           // XOP
  ASMJIT_INST_2x(vfrczss, Vfrczss, Xmm, Xmm)                           // XOP
  ASMJIT_INST_2x(vfrczss, Vfrczss, Xmm, Mem)                           // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Xmm, Xmm, Xmm, Xmm)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Xmm, Xmm, Mem, Xmm)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Xmm, Xmm, Xmm, Mem)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Ymm, Ymm, Ymm, Ymm)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Ymm, Ymm, Mem, Ymm)                   // XOP
  ASMJIT_INST_4x(vpcmov, Vpcmov, Ymm, Ymm, Ymm, Mem)                   // XOP
  ASMJIT_INST_4i(vpcomb, Vpcomb, Xmm, Xmm, Xmm, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomb, Vpcomb, Xmm, Xmm, Mem, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomd, Vpcomd, Xmm, Xmm, Xmm, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomd, Vpcomd, Xmm, Xmm, Mem, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomq, Vpcomq, Xmm, Xmm, Xmm, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomq, Vpcomq, Xmm, Xmm, Mem, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomw, Vpcomw, Xmm, Xmm, Xmm, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomw, Vpcomw, Xmm, Xmm, Mem, Imm)                   // XOP
  ASMJIT_INST_4i(vpcomub, Vpcomub, Xmm, Xmm, Xmm, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomub, Vpcomub, Xmm, Xmm, Mem, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomud, Vpcomud, Xmm, Xmm, Xmm, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomud, Vpcomud, Xmm, Xmm, Mem, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomuq, Vpcomuq, Xmm, Xmm, Xmm, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomuq, Vpcomuq, Xmm, Xmm, Mem, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomuw, Vpcomuw, Xmm, Xmm, Xmm, Imm)                 // XOP
  ASMJIT_INST_4i(vpcomuw, Vpcomuw, Xmm, Xmm, Mem, Imm)                 // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Xmm, Xmm, Xmm, Xmm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Xmm, Xmm, Mem, Xmm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Xmm, Xmm, Xmm, Mem, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Ymm, Ymm, Ymm, Ymm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Ymm, Ymm, Mem, Ymm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2pd, Vpermil2pd, Ymm, Ymm, Ymm, Mem, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Xmm, Xmm, Xmm, Xmm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Xmm, Xmm, Mem, Xmm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Xmm, Xmm, Xmm, Mem, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Ymm, Ymm, Ymm, Ymm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Ymm, Ymm, Mem, Ymm, Imm)      // XOP
  ASMJIT_INST_5i(vpermil2ps, Vpermil2ps, Ymm, Ymm, Ymm, Mem, Imm)      // XOP
  ASMJIT_INST_2x(vphaddbd, Vphaddbd, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphaddbd, Vphaddbd, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddbq, Vphaddbq, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphaddbq, Vphaddbq, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddbw, Vphaddbw, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphaddbw, Vphaddbw, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphadddq, Vphadddq, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphadddq, Vphadddq, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddwd, Vphaddwd, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphaddwd, Vphaddwd, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddwq, Vphaddwq, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphaddwq, Vphaddwq, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphaddubd, Vphaddubd, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphaddubd, Vphaddubd, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddubq, Vphaddubq, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphaddubq, Vphaddubq, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddubw, Vphaddubw, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphaddubw, Vphaddubw, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphaddudq, Vphaddudq, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphaddudq, Vphaddudq, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphadduwd, Vphadduwd, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphadduwd, Vphadduwd, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphadduwq, Vphadduwq, Xmm, Xmm)                       // XOP
  ASMJIT_INST_2x(vphadduwq, Vphadduwq, Xmm, Mem)                       // XOP
  ASMJIT_INST_2x(vphsubbw, Vphsubbw, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphsubbw, Vphsubbw, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphsubdq, Vphsubdq, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphsubdq, Vphsubdq, Xmm, Mem)                         // XOP
  ASMJIT_INST_2x(vphsubwd, Vphsubwd, Xmm, Xmm)                         // XOP
  ASMJIT_INST_2x(vphsubwd, Vphsubwd, Xmm, Mem)                         // XOP
  ASMJIT_INST_4x(vpmacsdd, Vpmacsdd, Xmm, Xmm, Xmm, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacsdd, Vpmacsdd, Xmm, Xmm, Mem, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacsdqh, Vpmacsdqh, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacsdqh, Vpmacsdqh, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacsdql, Vpmacsdql, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacsdql, Vpmacsdql, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacswd, Vpmacswd, Xmm, Xmm, Xmm, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacswd, Vpmacswd, Xmm, Xmm, Mem, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacsww, Vpmacsww, Xmm, Xmm, Xmm, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacsww, Vpmacsww, Xmm, Xmm, Mem, Xmm)               // XOP
  ASMJIT_INST_4x(vpmacssdd, Vpmacssdd, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacssdd, Vpmacssdd, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacssdqh, Vpmacssdqh, Xmm, Xmm, Xmm, Xmm)           // XOP
  ASMJIT_INST_4x(vpmacssdqh, Vpmacssdqh, Xmm, Xmm, Mem, Xmm)           // XOP
  ASMJIT_INST_4x(vpmacssdql, Vpmacssdql, Xmm, Xmm, Xmm, Xmm)           // XOP
  ASMJIT_INST_4x(vpmacssdql, Vpmacssdql, Xmm, Xmm, Mem, Xmm)           // XOP
  ASMJIT_INST_4x(vpmacsswd, Vpmacsswd, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacsswd, Vpmacsswd, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacssww, Vpmacssww, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmacssww, Vpmacssww, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpmadcsswd, Vpmadcsswd, Xmm, Xmm, Xmm, Xmm)           // XOP
  ASMJIT_INST_4x(vpmadcsswd, Vpmadcsswd, Xmm, Xmm, Mem, Xmm)           // XOP
  ASMJIT_INST_4x(vpmadcswd, Vpmadcswd, Xmm, Xmm, Xmm, Xmm)             // XOP
  ASMJIT_INST_4x(vpmadcswd, Vpmadcswd, Xmm, Xmm, Mem, Xmm)             // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Xmm, Xmm, Xmm, Xmm)                   // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Xmm, Xmm, Mem, Xmm)                   // XOP
  ASMJIT_INST_4x(vpperm, Vpperm, Xmm, Xmm, Xmm, Mem)                   // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotb, Vprotb, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3i(vprotb, Vprotb, Xmm, Xmm, Imm)                        // XOP
  ASMJIT_INST_3i(vprotb, Vprotb, Xmm, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotd, Vprotd, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3i(vprotd, Vprotd, Xmm, Xmm, Imm)                        // XOP
  ASMJIT_INST_3i(vprotd, Vprotd, Xmm, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotq, Vprotq, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3i(vprotq, Vprotq, Xmm, Xmm, Imm)                        // XOP
  ASMJIT_INST_3i(vprotq, Vprotq, Xmm, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vprotw, Vprotw, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3i(vprotw, Vprotw, Xmm, Xmm, Imm)                        // XOP
  ASMJIT_INST_3i(vprotw, Vprotw, Xmm, Mem, Imm)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshab, Vpshab, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshad, Vpshad, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshaq, Vpshaq, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshaw, Vpshaw, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlb, Vpshlb, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshld, Vpshld, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlq, Vpshlq, Xmm, Xmm, Mem)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Xmm, Xmm, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Xmm, Mem, Xmm)                        // XOP
  ASMJIT_INST_3x(vpshlw, Vpshlw, Xmm, Xmm, Mem)                        // XOP

  //! \}
};

// ============================================================================
// [asmjit::x86::EmitterImplicitT]
// ============================================================================

template<typename This>
struct EmitterImplicitT : public EmitterExplicitT<This> {
  //! \name Prefix Options
  //! \{

  //! Use REP/REPE prefix.
  inline This& rep() noexcept { return EmitterExplicitT<This>::_addInstOptions(Inst::kOptionRep); }
  //! Use REP/REPE prefix.
  inline This& repe() noexcept { return rep(); }
  //! Use REP/REPE prefix.
  inline This& repz() noexcept { return rep(); }

  //! Use REPNE prefix.
  inline This& repne() noexcept { return EmitterExplicitT<This>::_addInstOptions(Inst::kOptionRepne); }
  //! Use REPNE prefix.
  inline This& repnz() noexcept { return repne(); }

  //! \}

  //! \name Base Instructions & GP Extensions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::_emitter;

  // TODO: xrstor and xsave don't have explicit variants yet.
  using EmitterExplicitT<This>::cbw;
  using EmitterExplicitT<This>::cdq;
  using EmitterExplicitT<This>::cdqe;
  using EmitterExplicitT<This>::clzero;
  using EmitterExplicitT<This>::cqo;
  using EmitterExplicitT<This>::cwd;
  using EmitterExplicitT<This>::cwde;
  using EmitterExplicitT<This>::cmpsd;
  using EmitterExplicitT<This>::cmpxchg;
  using EmitterExplicitT<This>::cmpxchg8b;
  using EmitterExplicitT<This>::cmpxchg16b;
  using EmitterExplicitT<This>::cpuid;
  using EmitterExplicitT<This>::div;
  using EmitterExplicitT<This>::idiv;
  using EmitterExplicitT<This>::imul;
  using EmitterExplicitT<This>::jecxz;
  using EmitterExplicitT<This>::lahf;
  using EmitterExplicitT<This>::mulx;
  using EmitterExplicitT<This>::movsd;
  using EmitterExplicitT<This>::mul;
  using EmitterExplicitT<This>::rdmsr;
  using EmitterExplicitT<This>::rdpmc;
  using EmitterExplicitT<This>::rdtsc;
  using EmitterExplicitT<This>::rdtscp;
  using EmitterExplicitT<This>::sahf;
  using EmitterExplicitT<This>::wrmsr;
  using EmitterExplicitT<This>::xgetbv;
  using EmitterExplicitT<This>::xsetbv;
  //! \endcond

  ASMJIT_INST_0x(cbw, Cbw)                                             // ANY       [IMPLICIT] AX      <- Sign Extend AL
  ASMJIT_INST_0x(cdq, Cdq)                                             // ANY       [IMPLICIT] EDX:EAX <- Sign Extend EAX
  ASMJIT_INST_0x(cdqe, Cdqe)                                           // X64       [IMPLICIT] RAX     <- Sign Extend EAX
  ASMJIT_INST_2x(cmpxchg, Cmpxchg, Gp, Gp)                             // I486      [IMPLICIT]
  ASMJIT_INST_2x(cmpxchg, Cmpxchg, Mem, Gp)                            // I486      [IMPLICIT]
  ASMJIT_INST_1x(cmpxchg16b, Cmpxchg16b, Mem)                          // CMPXCHG8B [IMPLICIT] m == RDX:RAX ? m <- RCX:RBX
  ASMJIT_INST_1x(cmpxchg8b, Cmpxchg8b, Mem)                            // CMPXCHG16B[IMPLICIT] m == EDX:EAX ? m <- ECX:EBX
  ASMJIT_INST_0x(cpuid, Cpuid)                                         // I486      [IMPLICIT] EAX:EBX:ECX:EDX  <- CPUID[EAX:ECX]
  ASMJIT_INST_0x(cqo, Cqo)                                             // X64       [IMPLICIT] RDX:RAX <- Sign Extend RAX
  ASMJIT_INST_0x(cwd, Cwd)                                             // ANY       [IMPLICIT] DX:AX   <- Sign Extend AX
  ASMJIT_INST_0x(cwde, Cwde)                                           // ANY       [IMPLICIT] EAX     <- Sign Extend AX
  ASMJIT_INST_0x(daa, Daa)
  ASMJIT_INST_0x(das, Das)
  ASMJIT_INST_1x(div, Div, Gp)                                         // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / r8} {xDX[Rem]:xAX[Quot] <- DX:AX / r16|r32|r64}
  ASMJIT_INST_1x(div, Div, Mem)                                        // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / m8} {xDX[Rem]:xAX[Quot] <- DX:AX / m16|m32|m64}
  ASMJIT_INST_1x(idiv, Idiv, Gp)                                       // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / r8} {xDX[Rem]:xAX[Quot] <- DX:AX / r16|r32|r64}
  ASMJIT_INST_1x(idiv, Idiv, Mem)                                      // ANY       [IMPLICIT] {AH[Rem]: AL[Quot] <- AX / m8} {xDX[Rem]:xAX[Quot] <- DX:AX / m16|m32|m64}
  ASMJIT_INST_1x(imul, Imul, Gp)                                       // ANY       [IMPLICIT] {AX <- AL * r8} {xAX:xDX <- xAX * r16|r32|r64}
  ASMJIT_INST_1x(imul, Imul, Mem)                                      // ANY       [IMPLICIT] {AX <- AL * m8} {xAX:xDX <- xAX * m16|m32|m64}
  ASMJIT_INST_0x(iret, Iret)                                           // ANY       [IMPLICIT]
  ASMJIT_INST_0x(iretd, Iretd)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(iretq, Iretq)                                         // X64       [IMPLICIT]
  ASMJIT_INST_0x(iretw, Iretw)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_1x(jecxz, Jecxz, Label)                                  // ANY       [IMPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(jecxz, Jecxz, Imm)                                    // ANY       [IMPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_1x(jecxz, Jecxz, uint64_t)                               // ANY       [IMPLICIT] Short jump if CX/ECX/RCX is zero.
  ASMJIT_INST_0x(lahf, Lahf)                                           // LAHFSAHF  [IMPLICIT] AH <- EFL
  ASMJIT_INST_1x(loop, Loop, Label)                                    // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_1x(loop, Loop, Imm)                                      // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_1x(loop, Loop, uint64_t)                                 // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0.
  ASMJIT_INST_1x(loope, Loope, Label)                                  // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_1x(loope, Loope, Imm)                                    // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_1x(loope, Loope, uint64_t)                               // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 1.
  ASMJIT_INST_1x(loopne, Loopne, Label)                                // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_1x(loopne, Loopne, Imm)                                  // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_1x(loopne, Loopne, uint64_t)                             // ANY       [IMPLICIT] Decrement xCX; short jump if xCX != 0 && ZF == 0.
  ASMJIT_INST_1x(mul, Mul, Gp)                                         // ANY       [IMPLICIT] {AX <- AL * r8} {xDX:xAX <- xAX * r16|r32|r64}
  ASMJIT_INST_1x(mul, Mul, Mem)                                        // ANY       [IMPLICIT] {AX <- AL * m8} {xDX:xAX <- xAX * m16|m32|m64}
  ASMJIT_INST_0x(rdmsr, Rdmsr)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(rdpmc, Rdpmc)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(rdtsc, Rdtsc)                                         // RDTSC     [IMPLICIT] EDX:EAX <- CNT
  ASMJIT_INST_0x(rdtscp, Rdtscp)                                       // RDTSCP    [IMPLICIT] EDX:EAX:EXC <- CNT
  ASMJIT_INST_0x(ret, Ret)
  ASMJIT_INST_1i(ret, Ret, Imm)
  ASMJIT_INST_0x(sahf, Sahf)                                           // LAHFSAHF  [IMPLICIT] EFL <- AH
  ASMJIT_INST_0x(syscall, Syscall)                                     // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysenter, Sysenter)                                   // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysexit, Sysexit)                                     // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysexit64, Sysexit64)                                 // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysret, Sysret)                                       // X64       [IMPLICIT]
  ASMJIT_INST_0x(sysret64, Sysret64)                                   // X64       [IMPLICIT]
  ASMJIT_INST_0x(wrmsr, Wrmsr)                                         // ANY       [IMPLICIT]
  ASMJIT_INST_0x(xlatb, Xlatb)                                         // ANY       [IMPLICIT]

  //! \}

  //! \name String Instruction Aliases
  //! \{

  inline Error cmpsb() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 1), EmitterExplicitT<This>::ptr_zdi(0, 1)); }
  inline Error cmpsd() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 4), EmitterExplicitT<This>::ptr_zdi(0, 4)); }
  inline Error cmpsq() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 8), EmitterExplicitT<This>::ptr_zdi(0, 8)); }
  inline Error cmpsw() { return _emitter()->emit(Inst::kIdCmps, EmitterExplicitT<This>::ptr_zsi(0, 2), EmitterExplicitT<This>::ptr_zdi(0, 2)); }

  inline Error lodsb() { return _emitter()->emit(Inst::kIdLods, al , EmitterExplicitT<This>::ptr_zdi(0, 1)); }
  inline Error lodsd() { return _emitter()->emit(Inst::kIdLods, eax, EmitterExplicitT<This>::ptr_zdi(0, 4)); }
  inline Error lodsq() { return _emitter()->emit(Inst::kIdLods, rax, EmitterExplicitT<This>::ptr_zdi(0, 8)); }
  inline Error lodsw() { return _emitter()->emit(Inst::kIdLods, ax , EmitterExplicitT<This>::ptr_zdi(0, 2)); }

  inline Error movsb() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 1), EmitterExplicitT<This>::ptr_zsi(0, 1)); }
  inline Error movsd() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 4), EmitterExplicitT<This>::ptr_zsi(0, 4)); }
  inline Error movsq() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 8), EmitterExplicitT<This>::ptr_zsi(0, 8)); }
  inline Error movsw() { return _emitter()->emit(Inst::kIdMovs, EmitterExplicitT<This>::ptr_zdi(0, 2), EmitterExplicitT<This>::ptr_zsi(0, 2)); }

  inline Error scasb() { return _emitter()->emit(Inst::kIdScas, al , EmitterExplicitT<This>::ptr_zdi(0, 1)); }
  inline Error scasd() { return _emitter()->emit(Inst::kIdScas, eax, EmitterExplicitT<This>::ptr_zdi(0, 4)); }
  inline Error scasq() { return _emitter()->emit(Inst::kIdScas, rax, EmitterExplicitT<This>::ptr_zdi(0, 8)); }
  inline Error scasw() { return _emitter()->emit(Inst::kIdScas, ax , EmitterExplicitT<This>::ptr_zdi(0, 2)); }

  inline Error stosb() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 1), al ); }
  inline Error stosd() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 4), eax); }
  inline Error stosq() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 8), rax); }
  inline Error stosw() { return _emitter()->emit(Inst::kIdStos, EmitterExplicitT<This>::ptr_zdi(0, 2), ax ); }

  //! \}

  //! \name CL Instructions
  //! \{

  ASMJIT_INST_0x(clzero, Clzero)                                       // CLZERO    [IMPLICIT]

  //! \}

  //! \name BMI2 Instructions
  //! \{

  ASMJIT_INST_3x(mulx, Mulx, Gp, Gp, Gp)                               // BMI2      [IMPLICIT]
  ASMJIT_INST_3x(mulx, Mulx, Gp, Gp, Mem)                              // BMI2      [IMPLICIT]

  //! \}

  //! \name FXSR & XSAVE Instructions
  //! \{

  ASMJIT_INST_0x(xgetbv, Xgetbv)                                       // XSAVE     [IMPLICIT] EDX:EAX <- XCR[ECX]
  ASMJIT_INST_1x(xrstor, Xrstor, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xrstor64, Xrstor64, Mem)                              // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xrstors, Xrstors, Mem)                                // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xrstors64, Xrstors64, Mem)                            // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsave, Xsave, Mem)                                    // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsave64, Xsave64, Mem)                                // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsavec, Xsavec, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsavec64, Xsavec64, Mem)                              // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsaveopt, Xsaveopt, Mem)                              // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsaveopt64, Xsaveopt64, Mem)                          // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_1x(xsaves, Xsaves, Mem)                                  // XSAVE     [IMPLICIT]
  ASMJIT_INST_1x(xsaves64, Xsaves64, Mem)                              // XSAVE+X64 [IMPLICIT]
  ASMJIT_INST_0x(xsetbv, Xsetbv)                                       // XSAVE     [IMPLICIT] XCR[ECX] <- EDX:EAX

  //! \}

  //! \name Monitor & MWait Instructions
  //! \{

  ASMJIT_INST_0x(monitor, Monitor)
  ASMJIT_INST_0x(monitorx, Monitorx)
  ASMJIT_INST_0x(mwait, Mwait)
  ASMJIT_INST_0x(mwaitx, Mwaitx)

  //! \}

  //! \name MMX & SSE Instructions
  //! \{

  //! \cond
  using EmitterExplicitT<This>::blendvpd;
  using EmitterExplicitT<This>::blendvps;
  using EmitterExplicitT<This>::maskmovq;
  using EmitterExplicitT<This>::maskmovdqu;
  using EmitterExplicitT<This>::pblendvb;
  using EmitterExplicitT<This>::pcmpestri;
  using EmitterExplicitT<This>::pcmpestrm;
  using EmitterExplicitT<This>::pcmpistri;
  using EmitterExplicitT<This>::pcmpistrm;
  //! \endcond

  ASMJIT_INST_2x(blendvpd, Blendvpd, Xmm, Xmm)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvpd, Blendvpd, Xmm, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvps, Blendvps, Xmm, Xmm)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(blendvps, Blendvps, Xmm, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(pblendvb, Pblendvb, Xmm, Xmm)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(pblendvb, Pblendvb, Xmm, Mem)                         // SSE4_1 [IMPLICIT]
  ASMJIT_INST_2x(maskmovq, Maskmovq, Mm, Mm)                           // SSE    [IMPLICIT]
  ASMJIT_INST_2x(maskmovdqu, Maskmovdqu, Xmm, Xmm)                     // SSE2   [IMPLICIT]
  ASMJIT_INST_3i(pcmpestri, Pcmpestri, Xmm, Xmm, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpestri, Pcmpestri, Xmm, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpestrm, Pcmpestrm, Xmm, Xmm, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpestrm, Pcmpestrm, Xmm, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpistri, Pcmpistri, Xmm, Xmm, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpistri, Pcmpistri, Xmm, Mem, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpistrm, Pcmpistrm, Xmm, Xmm, Imm)                  // SSE4_1 [IMPLICIT]
  ASMJIT_INST_3i(pcmpistrm, Pcmpistrm, Xmm, Mem, Imm)                  // SSE4_1 [IMPLICIT]

  //! \}

  //! \name SHA Instructions
  //! \{

  using EmitterExplicitT<This>::sha256rnds2;

  ASMJIT_INST_2x(sha256rnds2, Sha256rnds2, Xmm, Xmm)                   // SHA [IMPLICIT]
  ASMJIT_INST_2x(sha256rnds2, Sha256rnds2, Xmm, Mem)                   // SHA [IMPLICIT]

  //! \}

  //! \name AVX, FMA, and AVX512 Instructions
  //! \{

  using EmitterExplicitT<This>::vmaskmovdqu;
  using EmitterExplicitT<This>::vpcmpestri;
  using EmitterExplicitT<This>::vpcmpestrm;
  using EmitterExplicitT<This>::vpcmpistri;
  using EmitterExplicitT<This>::vpcmpistrm;

  ASMJIT_INST_2x(vmaskmovdqu, Vmaskmovdqu, Xmm, Xmm)                   // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpestri, Vpcmpestri, Xmm, Xmm, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpestri, Vpcmpestri, Xmm, Mem, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpestrm, Vpcmpestrm, Xmm, Xmm, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpestrm, Vpcmpestrm, Xmm, Mem, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpistri, Vpcmpistri, Xmm, Xmm, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpistri, Vpcmpistri, Xmm, Mem, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpistrm, Vpcmpistrm, Xmm, Xmm, Imm)                // AVX  [IMPLICIT]
  ASMJIT_INST_3i(vpcmpistrm, Vpcmpistrm, Xmm, Mem, Imm)                // AVX  [IMPLICIT]

  //! \}
};

// ============================================================================
// [asmjit::x86::Emitter]
// ============================================================================

//! Emitter (X86).
//!
//! \note This class cannot be instantiated, you can only cast to it and use
//! it as emitter that emits to either `x86::Assembler`, `x86::Builder`, or
//! `x86::Compiler` (use with caution with `x86::Compiler` as it requires virtual
//! registers).
class Emitter : public BaseEmitter, public EmitterImplicitT<Emitter> {
  ASMJIT_NONCONSTRUCTIBLE(Emitter)
};

//! \}

#undef ASMJIT_INST_0x
#undef ASMJIT_INST_1x
#undef ASMJIT_INST_1i
#undef ASMJIT_INST_1c
#undef ASMJIT_INST_2x
#undef ASMJIT_INST_2i
#undef ASMJIT_INST_2c
#undef ASMJIT_INST_3x
#undef ASMJIT_INST_3i
#undef ASMJIT_INST_3ii
#undef ASMJIT_INST_4x
#undef ASMJIT_INST_4i
#undef ASMJIT_INST_4ii
#undef ASMJIT_INST_5x
#undef ASMJIT_INST_5i
#undef ASMJIT_INST_6x

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86EMITTER_H_INCLUDED
