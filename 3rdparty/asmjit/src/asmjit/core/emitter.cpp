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
#include "../core/logging.h"
#include "../core/support.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86internal_p.h"
  #include "../x86/x86instdb_p.h"
#endif // ASMJIT_BUILD_X86

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminternal_p.h"
  #include "../arm/arminstdb.h"
#endif // ASMJIT_BUILD_ARM

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::BaseEmitter - Construction / Destruction]
// ============================================================================

BaseEmitter::BaseEmitter(uint32_t type) noexcept
  : _type(uint8_t(type)),
    _reserved(0),
    _flags(0),
    _emitterOptions(0),
    _code(nullptr),
    _errorHandler(nullptr),
    _codeInfo(),
    _gpRegInfo(),
    _privateData(0),
    _instOptions(0),
    _globalInstOptions(BaseInst::kOptionReserved),
    _extraReg(),
    _inlineComment(nullptr) {}

BaseEmitter::~BaseEmitter() noexcept {
  if (_code) {
    _addFlags(kFlagDestroyed);
    _code->detach(this);
  }
}

// ============================================================================
// [asmjit::BaseEmitter - Code-Generation]
// ============================================================================

Error BaseEmitter::_emitOpArray(uint32_t instId, const Operand_* operands, size_t count) {
  const Operand_* op = operands;
  const Operand& none_ = Globals::none;

  switch (count) {
    case  0: return _emit(instId, none_, none_, none_, none_);
    case  1: return _emit(instId, op[0], none_, none_, none_);
    case  2: return _emit(instId, op[0], op[1], none_, none_);
    case  3: return _emit(instId, op[0], op[1], op[2], none_);
    case  4: return _emit(instId, op[0], op[1], op[2], op[3]);
    case  5: return _emit(instId, op[0], op[1], op[2], op[3], op[4], none_);
    case  6: return _emit(instId, op[0], op[1], op[2], op[3], op[4], op[5]);
    default: return DebugUtils::errored(kErrorInvalidArgument);
  }
}

// ============================================================================
// [asmjit::BaseEmitter - Finalize]
// ============================================================================

Label BaseEmitter::labelByName(const char* name, size_t nameSize, uint32_t parentId) noexcept {
  return Label(_code ? _code->labelIdByName(name, nameSize, parentId) : uint32_t(Globals::kInvalidId));
}

// ============================================================================
// [asmjit::BaseEmitter - Finalize]
// ============================================================================

Error BaseEmitter::finalize() {
  // Does nothing by default, overridden by `BaseBuilder` and `BaseCompiler`.
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseEmitter - Error Handling]
// ============================================================================

Error BaseEmitter::reportError(Error err, const char* message) {
  ErrorHandler* handler = errorHandler();
  if (!handler) {
    if (code())
      handler = code()->errorHandler();
  }

  if (handler) {
    if (!message)
      message = DebugUtils::errorAsString(err);
    handler->handleError(err, message, this);
  }

  return err;
}

// ============================================================================
// [asmjit::BaseEmitter - Label Management]
// ============================================================================

bool BaseEmitter::isLabelValid(uint32_t labelId) const noexcept {
  return _code && labelId < _code->labelCount();
}

// ============================================================================
// [asmjit::BaseEmitter - Emit (High-Level)]
// ============================================================================

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitProlog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

#ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitProlog(as<x86::Emitter>(), frame);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitProlog(as<arm::Emitter>(), frame);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitEpilog(const FuncFrame& frame) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

#ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitEpilog(as<x86::Emitter>(), frame);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitEpilog(as<arm::Emitter>(), frame);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_FAVOR_SIZE Error BaseEmitter::emitArgsAssignment(const FuncFrame& frame, const FuncArgsAssignment& args) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

#ifdef ASMJIT_BUILD_X86
  if (archInfo().isX86Family())
    return x86::X86Internal::emitArgsAssignment(as<x86::Emitter>(), frame, args);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (archInfo().isArmFamily())
    return arm::ArmInternal::emitArgsAssignment(as<arm::Emitter>(), frame, args);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}

// ============================================================================
// [asmjit::BaseEmitter - Comment]
// ============================================================================

Error BaseEmitter::commentf(const char* fmt, ...) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

#ifndef ASMJIT_NO_LOGGING
  StringTmp<1024> sb;

  va_list ap;
  va_start(ap, fmt);
  Error err = sb.appendVFormat(fmt, ap);
  va_end(ap);

  if (ASMJIT_UNLIKELY(err))
    return err;

  return comment(sb.data(), sb.size());
#else
  DebugUtils::unused(fmt);
  return kErrorOk;
#endif
}

Error BaseEmitter::commentv(const char* fmt, va_list ap) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

#ifndef ASMJIT_NO_LOGGING
  StringTmp<1024> sb;

  Error err = sb.appendVFormat(fmt, ap);
  if (ASMJIT_UNLIKELY(err))
    return err;

  return comment(sb.data(), sb.size());
#else
  DebugUtils::unused(fmt, ap);
  return kErrorOk;
#endif
}

// ============================================================================
// [asmjit::BaseEmitter - Events]
// ============================================================================

Error BaseEmitter::onAttach(CodeHolder* code) noexcept {
  _code = code;
  _codeInfo = code->codeInfo();
  _emitterOptions = code->emitterOptions();

  onUpdateGlobalInstOptions();
  return kErrorOk;
}

Error BaseEmitter::onDetach(CodeHolder* code) noexcept {
  DebugUtils::unused(code);

  _flags = 0;
  _emitterOptions = 0;
  _errorHandler = nullptr;

  _codeInfo.reset();
  _gpRegInfo.reset();
  _privateData = 0;

  _instOptions = 0;
  _globalInstOptions = BaseInst::kOptionReserved;
  _extraReg.reset();
  _inlineComment = nullptr;

  return kErrorOk;
}

void BaseEmitter::onUpdateGlobalInstOptions() noexcept {
  constexpr uint32_t kCriticalEmitterOptions =
    kOptionLoggingEnabled   |
    kOptionStrictValidation ;

  _globalInstOptions &= ~BaseInst::kOptionReserved;
  if ((_emitterOptions & kCriticalEmitterOptions) != 0)
    _globalInstOptions |= BaseInst::kOptionReserved;
}

ASMJIT_END_NAMESPACE
