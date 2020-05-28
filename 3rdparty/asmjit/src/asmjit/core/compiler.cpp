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
#ifndef ASMJIT_NO_COMPILER

#include "../core/assembler.h"
#include "../core/compiler.h"
#include "../core/cpuinfo.h"
#include "../core/logging.h"
#include "../core/rapass_p.h"
#include "../core/rastack_p.h"
#include "../core/support.h"
#include "../core/type.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::GlobalConstPoolPass]
// ============================================================================

class GlobalConstPoolPass : public Pass {
  ASMJIT_NONCOPYABLE(GlobalConstPoolPass)
  typedef Pass Base;

  GlobalConstPoolPass() noexcept : Pass("GlobalConstPoolPass") {}

  Error run(Zone* zone, Logger* logger) noexcept override {
    DebugUtils::unused(zone, logger);

    // Flush the global constant pool.
    BaseCompiler* compiler = static_cast<BaseCompiler*>(_cb);
    if (compiler->_globalConstPool) {
      compiler->addAfter(compiler->_globalConstPool, compiler->lastNode());
      compiler->_globalConstPool = nullptr;
    }
    return kErrorOk;
  }
};

// ============================================================================
// [asmjit::FuncCallNode - Arg / Ret]
// ============================================================================

bool FuncCallNode::_setArg(uint32_t i, const Operand_& op) noexcept {
  if ((i & ~kFuncArgHi) >= _funcDetail.argCount())
    return false;

  _args[i] = op;
  return true;
}

bool FuncCallNode::_setRet(uint32_t i, const Operand_& op) noexcept {
  if (i >= 2)
    return false;

  _rets[i] = op;
  return true;
}

// ============================================================================
// [asmjit::BaseCompiler - Construction / Destruction]
// ============================================================================

BaseCompiler::BaseCompiler() noexcept
  : BaseBuilder(),
    _func(nullptr),
    _vRegZone(4096 - Zone::kBlockOverhead),
    _vRegArray(),
    _localConstPool(nullptr),
    _globalConstPool(nullptr) {

  _type = kTypeCompiler;
}
BaseCompiler::~BaseCompiler() noexcept {}

// ============================================================================
// [asmjit::BaseCompiler - Function API]
// ============================================================================

FuncNode* BaseCompiler::newFunc(const FuncSignature& sign) noexcept {
  Error err;

  FuncNode* func = newNodeT<FuncNode>();
  if (ASMJIT_UNLIKELY(!func)) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  err = registerLabelNode(func);
  if (ASMJIT_UNLIKELY(err)) {
    // TODO: Calls reportError, maybe rethink noexcept?
    reportError(err);
    return nullptr;
  }

  // Create helper nodes.
  func->_exitNode = newLabelNode();
  func->_end = newNodeT<SentinelNode>(SentinelNode::kSentinelFuncEnd);

  if (ASMJIT_UNLIKELY(!func->_exitNode || !func->_end)) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  // Initialize the function info.
  err = func->detail().init(sign);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // If the Target guarantees greater stack alignment than required by the
  // calling convention then override it as we can prevent having to perform
  // dynamic stack alignment
  if (func->_funcDetail._callConv.naturalStackAlignment() < _codeInfo.stackAlignment())
    func->_funcDetail._callConv.setNaturalStackAlignment(_codeInfo.stackAlignment());

  // Initialize the function frame.
  err = func->_frame.init(func->_funcDetail);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // Allocate space for function arguments.
  func->_args = nullptr;
  if (func->argCount() != 0) {
    func->_args = _allocator.allocT<VirtReg*>(func->argCount() * sizeof(VirtReg*));
    if (ASMJIT_UNLIKELY(!func->_args)) {
      reportError(DebugUtils::errored(kErrorOutOfMemory));
      return nullptr;
    }

    memset(func->_args, 0, func->argCount() * sizeof(VirtReg*));
  }

  return func;
}

FuncNode* BaseCompiler::addFunc(FuncNode* func) {
  ASMJIT_ASSERT(_func == nullptr);
  _func = func;

  addNode(func);                 // Function node.
  BaseNode* prev = cursor();     // {CURSOR}.
  addNode(func->exitNode());     // Function exit label.
  addNode(func->endNode());      // Function end marker.

  _setCursor(prev);
  return func;
}

FuncNode* BaseCompiler::addFunc(const FuncSignature& sign) {
  FuncNode* func = newFunc(sign);

  if (!func) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  return addFunc(func);
}

Error BaseCompiler::endFunc() {
  FuncNode* func = _func;
  if (ASMJIT_UNLIKELY(!func))
    return reportError(DebugUtils::errored(kErrorInvalidState));

  // Add the local constant pool at the end of the function (if exists).
  if (_localConstPool) {
    setCursor(func->endNode()->prev());
    addNode(_localConstPool);
    _localConstPool = nullptr;
  }

  // Mark as finished.
  _func = nullptr;

  SentinelNode* end = func->endNode();
  setCursor(end);
  return kErrorOk;
}

Error BaseCompiler::setArg(uint32_t argIndex, const BaseReg& r) {
  FuncNode* func = _func;

  if (ASMJIT_UNLIKELY(!func))
    return reportError(DebugUtils::errored(kErrorInvalidState));

  if (ASMJIT_UNLIKELY(!isVirtRegValid(r)))
    return reportError(DebugUtils::errored(kErrorInvalidVirtId));

  VirtReg* vReg = virtRegByReg(r);
  func->setArg(argIndex, vReg);

  return kErrorOk;
}

FuncRetNode* BaseCompiler::newRet(const Operand_& o0, const Operand_& o1) noexcept {
  FuncRetNode* node = newNodeT<FuncRetNode>();
  if (!node) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOpCount(!o1.isNone() ? 2u : !o0.isNone() ? 1u : 0u);

  return node;
}

FuncRetNode* BaseCompiler::addRet(const Operand_& o0, const Operand_& o1) noexcept {
  FuncRetNode* node = newRet(o0, o1);
  if (!node) return nullptr;
  return addNode(node)->as<FuncRetNode>();
}

// ============================================================================
// [asmjit::BaseCompiler - Call]
// ============================================================================

FuncCallNode* BaseCompiler::newCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept {
  FuncCallNode* node = newNodeT<FuncCallNode>(instId, 0u);
  if (ASMJIT_UNLIKELY(!node)) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  node->setOpCount(1);
  node->setOp(0, o0);
  node->resetOp(1);
  node->resetOp(2);
  node->resetOp(3);

  Error err = node->detail().init(sign);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // If there are no arguments skip the allocation.
  uint32_t nArgs = sign.argCount();
  if (!nArgs) return node;

  node->_args = static_cast<Operand*>(_allocator.alloc(nArgs * sizeof(Operand)));
  if (!node->_args) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  memset(node->_args, 0, nArgs * sizeof(Operand));
  return node;
}

FuncCallNode* BaseCompiler::addCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept {
  FuncCallNode* node = newCall(instId, o0, sign);
  if (!node) return nullptr;
  return addNode(node)->as<FuncCallNode>();
}

// ============================================================================
// [asmjit::BaseCompiler - Vars]
// ============================================================================

static void BaseCompiler_assignGenericName(BaseCompiler* self, VirtReg* vReg) {
  uint32_t index = unsigned(Operand::virtIdToIndex(vReg->_id));

  char buf[64];
  int size = snprintf(buf, ASMJIT_ARRAY_SIZE(buf), "%%%u", unsigned(index));

  ASMJIT_ASSERT(size > 0 && size < int(ASMJIT_ARRAY_SIZE(buf)));
  vReg->_name.setData(&self->_dataZone, buf, unsigned(size));
}

VirtReg* BaseCompiler::newVirtReg(uint32_t typeId, uint32_t signature, const char* name) noexcept {
  uint32_t index = _vRegArray.size();
  if (ASMJIT_UNLIKELY(index >= uint32_t(Operand::kVirtIdCount)))
    return nullptr;

  if (_vRegArray.willGrow(&_allocator) != kErrorOk)
    return nullptr;

  VirtReg* vReg = _vRegZone.allocZeroedT<VirtReg>();
  if (ASMJIT_UNLIKELY(!vReg)) return nullptr;

  uint32_t size = Type::sizeOf(typeId);
  uint32_t alignment = Support::min<uint32_t>(size, 64);

  vReg = new(vReg) VirtReg(Operand::indexToVirtId(index), signature, size, alignment, typeId);

#ifndef ASMJIT_NO_LOGGING
  if (name && name[0] != '\0')
    vReg->_name.setData(&_dataZone, name, SIZE_MAX);
  else
    BaseCompiler_assignGenericName(this, vReg);
#else
  DebugUtils::unused(name);
#endif

  _vRegArray.appendUnsafe(vReg);
  return vReg;
}

Error BaseCompiler::_newReg(BaseReg& out, uint32_t typeId, const char* name) {
  RegInfo regInfo;

  Error err = ArchUtils::typeIdToRegInfo(archId(), typeId, regInfo);
  if (ASMJIT_UNLIKELY(err)) return reportError(err);

  VirtReg* vReg = newVirtReg(typeId, regInfo.signature(), name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  out._initReg(regInfo.signature(), vReg->id());
  return kErrorOk;
}

Error BaseCompiler::_newRegFmt(BaseReg& out, uint32_t typeId, const char* fmt, ...) {
  va_list ap;
  StringTmp<256> sb;

  va_start(ap, fmt);
  sb.appendVFormat(fmt, ap);
  va_end(ap);

  return _newReg(out, typeId, sb.data());
}

Error BaseCompiler::_newReg(BaseReg& out, const BaseReg& ref, const char* name) {
  RegInfo regInfo;
  uint32_t typeId;

  if (isVirtRegValid(ref)) {
    VirtReg* vRef = virtRegByReg(ref);
    typeId = vRef->typeId();

    // NOTE: It's possible to cast one register type to another if it's the
    // same register group. However, VirtReg always contains the TypeId that
    // was used to create the register. This means that in some cases we may
    // end up having different size of `ref` and `vRef`. In such case we
    // adjust the TypeId to match the `ref` register type instead of the
    // original register type, which should be the expected behavior.
    uint32_t typeSize = Type::sizeOf(typeId);
    uint32_t refSize = ref.size();

    if (typeSize != refSize) {
      if (Type::isInt(typeId)) {
        // GP register - change TypeId to match `ref`, but keep sign of `vRef`.
        switch (refSize) {
          case  1: typeId = Type::kIdI8  | (typeId & 1); break;
          case  2: typeId = Type::kIdI16 | (typeId & 1); break;
          case  4: typeId = Type::kIdI32 | (typeId & 1); break;
          case  8: typeId = Type::kIdI64 | (typeId & 1); break;
          default: typeId = Type::kIdVoid; break;
        }
      }
      else if (Type::isMmx(typeId)) {
        // MMX register - always use 64-bit.
        typeId = Type::kIdMmx64;
      }
      else if (Type::isMask(typeId)) {
        // Mask register - change TypeId to match `ref` size.
        switch (refSize) {
          case  1: typeId = Type::kIdMask8; break;
          case  2: typeId = Type::kIdMask16; break;
          case  4: typeId = Type::kIdMask32; break;
          case  8: typeId = Type::kIdMask64; break;
          default: typeId = Type::kIdVoid; break;
        }
      }
      else {
        // VEC register - change TypeId to match `ref` size, keep vector metadata.
        uint32_t elementTypeId = Type::baseOf(typeId);

        switch (refSize) {
          case 16: typeId = Type::_kIdVec128Start + (elementTypeId - Type::kIdI8); break;
          case 32: typeId = Type::_kIdVec256Start + (elementTypeId - Type::kIdI8); break;
          case 64: typeId = Type::_kIdVec512Start + (elementTypeId - Type::kIdI8); break;
          default: typeId = Type::kIdVoid; break;
        }
      }

      if (typeId == Type::kIdVoid)
        return reportError(DebugUtils::errored(kErrorInvalidState));
    }
  }
  else {
    typeId = ref.type();
  }

  Error err = ArchUtils::typeIdToRegInfo(archId(), typeId, regInfo);
  if (ASMJIT_UNLIKELY(err)) return reportError(err);

  VirtReg* vReg = newVirtReg(typeId, regInfo.signature(), name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  out._initReg(regInfo.signature(), vReg->id());
  return kErrorOk;
}

Error BaseCompiler::_newRegFmt(BaseReg& out, const BaseReg& ref, const char* fmt, ...) {
  va_list ap;
  StringTmp<256> sb;

  va_start(ap, fmt);
  sb.appendVFormat(fmt, ap);
  va_end(ap);

  return _newReg(out, ref, sb.data());
}

Error BaseCompiler::_newStack(BaseMem& out, uint32_t size, uint32_t alignment, const char* name) {
  if (size == 0)
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment == 0)
    alignment = 1;

  if (!Support::isPowerOf2(alignment))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment > 64)
    alignment = 64;

  VirtReg* vReg = newVirtReg(0, 0, name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  vReg->_virtSize = size;
  vReg->_isStack = true;
  vReg->_alignment = uint8_t(alignment);

  // Set the memory operand to GPD/GPQ and its id to VirtReg.
  out = BaseMem(BaseMem::Decomposed { _gpRegInfo.type(), vReg->id(), BaseReg::kTypeNone, 0, 0, 0, BaseMem::kSignatureMemRegHomeFlag });
  return kErrorOk;
}

Error BaseCompiler::setStackSize(uint32_t virtId, uint32_t newSize, uint32_t newAlignment) noexcept {
  if (!isVirtIdValid(virtId))
    return DebugUtils::errored(kErrorInvalidVirtId);

  if (newAlignment && !Support::isPowerOf2(newAlignment))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (newAlignment > 64)
    newAlignment = 64;

  VirtReg* vReg = virtRegById(virtId);
  if (newSize)
    vReg->_virtSize = newSize;

  if (newAlignment)
    vReg->_alignment = uint8_t(newAlignment);

  // This is required if the RAPass is already running. There is a chance that
  // a stack-slot has been already allocated and in that case it has to be
  // updated as well, otherwise we would allocate wrong amount of memory.
  RAWorkReg* workReg = vReg->_workReg;
  if (workReg && workReg->_stackSlot) {
    workReg->_stackSlot->_size = vReg->_virtSize;
    workReg->_stackSlot->_alignment = vReg->_alignment;
  }

  return kErrorOk;
}

Error BaseCompiler::_newConst(BaseMem& out, uint32_t scope, const void* data, size_t size) {
  ConstPoolNode** pPool;
  if (scope == ConstPool::kScopeLocal)
    pPool = &_localConstPool;
  else if (scope == ConstPool::kScopeGlobal)
    pPool = &_globalConstPool;
  else
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  ConstPoolNode* pool = *pPool;
  if (!pool) {
    pool = newConstPoolNode();
    if (ASMJIT_UNLIKELY(!pool))
      return reportError(DebugUtils::errored(kErrorOutOfMemory));
    *pPool = pool;
  }

  size_t off;
  Error err = pool->add(data, size, off);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  out = BaseMem(BaseMem::Decomposed {
    Label::kLabelTag,      // Base type.
    pool->id(),            // Base id.
    0,                     // Index type.
    0,                     // Index id.
    int32_t(off),          // Offset.
    uint32_t(size),        // Size.
    0                      // Flags.
  });
  return kErrorOk;
}

void BaseCompiler::rename(const BaseReg& reg, const char* fmt, ...) {
  if (!reg.isVirtReg()) return;

  VirtReg* vReg = virtRegById(reg.id());
  if (!vReg) return;

  if (fmt && fmt[0] != '\0') {
    char buf[128];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    va_end(ap);

    vReg->_name.setData(&_dataZone, buf, SIZE_MAX);
  }
  else {
    BaseCompiler_assignGenericName(this, vReg);
  }
}

// ============================================================================
// [asmjit::BaseCompiler - Jump Annotations]
// ============================================================================

JumpNode* BaseCompiler::newJumpNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, JumpAnnotation* annotation) noexcept {
  uint32_t opCount = 1;
  JumpNode* node = _allocator.allocT<JumpNode>();
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) JumpNode(this, instId, instOptions, opCount, annotation);
  node->setOp(0, o0);
  node->resetOps(opCount, JumpNode::kBaseOpCapacity);
  return node;
}

Error BaseCompiler::emitAnnotatedJump(uint32_t instId, const Operand_& o0, JumpAnnotation* annotation) {
  uint32_t options = instOptions() | globalInstOptions();
  const char* comment = inlineComment();

  JumpNode* node = newJumpNode(instId, options, o0, annotation);

  resetInstOptions();
  resetInlineComment();

  if (ASMJIT_UNLIKELY(!node)) {
    resetExtraReg();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  node->setExtraReg(extraReg());
  if (comment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(comment, strlen(comment), true)));

  addNode(node);
  resetExtraReg();
  return kErrorOk;
}

JumpAnnotation* BaseCompiler::newJumpAnnotation() {
  if (_jumpAnnotations.grow(&_allocator, 1) != kErrorOk) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  uint32_t id = _jumpAnnotations.size();
  JumpAnnotation* jumpAnnotation = _allocator.newT<JumpAnnotation>(this, id);

  if (!jumpAnnotation) {
    reportError(DebugUtils::errored(kErrorOutOfMemory));
    return nullptr;
  }

  _jumpAnnotations.appendUnsafe(jumpAnnotation);
  return jumpAnnotation;
}

// ============================================================================
// [asmjit::BaseCompiler - Events]
// ============================================================================

Error BaseCompiler::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  Error err = addPassT<GlobalConstPoolPass>();
  if (ASMJIT_UNLIKELY(err)) {
    onDetach(code);
    return err;
  }

  return kErrorOk;
}

Error BaseCompiler::onDetach(CodeHolder* code) noexcept {
  _func = nullptr;
  _localConstPool = nullptr;
  _globalConstPool = nullptr;

  _vRegArray.reset();
  _vRegZone.reset();

  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::FuncPass - Construction / Destruction]
// ============================================================================

FuncPass::FuncPass(const char* name) noexcept
  : Pass(name) {}

// ============================================================================
// [asmjit::FuncPass - Run]
// ============================================================================

Error FuncPass::run(Zone* zone, Logger* logger) noexcept {
  BaseNode* node = cb()->firstNode();
  if (!node) return kErrorOk;

  do {
    if (node->type() == BaseNode::kNodeFunc) {
      FuncNode* func = node->as<FuncNode>();
      node = func->endNode();
      ASMJIT_PROPAGATE(runOnFunction(zone, logger, func));
    }

    // Find a function by skipping all nodes that are not `kNodeFunc`.
    do {
      node = node->next();
    } while (node && node->type() != BaseNode::kNodeFunc);
  } while (node);

  return kErrorOk;
}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
