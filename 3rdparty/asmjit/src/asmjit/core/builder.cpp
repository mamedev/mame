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
#ifndef ASMJIT_NO_BUILDER

#include "../core/builder.h"
#include "../core/logging.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::PostponedErrorHandler (Internal)]
// ============================================================================

//! Postponed error handler that never throws. Used as a temporal error handler
//! to run passes. If error occurs, the caller is notified and will call the
//! real error handler, that can throw.
class PostponedErrorHandler : public ErrorHandler {
public:
  void handleError(Error err, const char* message, BaseEmitter* origin) override {
    DebugUtils::unused(err, origin);
    _message.assignString(message);
  }

  StringTmp<128> _message;
};

// ============================================================================
// [asmjit::BaseBuilder - Construction / Destruction]
// ============================================================================

BaseBuilder::BaseBuilder() noexcept
  : BaseEmitter(kTypeBuilder),
    _codeZone(32768 - Zone::kBlockOverhead),
    _dataZone(16384 - Zone::kBlockOverhead),
    _passZone(65536 - Zone::kBlockOverhead),
    _allocator(&_codeZone),
    _passes(),
    _labelNodes(),
    _cursor(nullptr),
    _firstNode(nullptr),
    _lastNode(nullptr),
    _nodeFlags(0) {}
BaseBuilder::~BaseBuilder() noexcept {}

// ============================================================================
// [asmjit::BaseBuilder - Node Management]
// ============================================================================

LabelNode* BaseBuilder::newLabelNode() noexcept {
  LabelNode* node = newNodeT<LabelNode>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

AlignNode* BaseBuilder::newAlignNode(uint32_t alignMode, uint32_t alignment) noexcept {
  return newNodeT<AlignNode>(alignMode, alignment);
}

EmbedDataNode* BaseBuilder::newEmbedDataNode(const void* data, uint32_t size) noexcept {
  if (size > EmbedDataNode::kInlineBufferSize) {
    void* cloned = _dataZone.alloc(size);
    if (ASMJIT_UNLIKELY(!cloned))
      return nullptr;

    if (data)
      memcpy(cloned, data, size);
    data = cloned;
  }

  return newNodeT<EmbedDataNode>(const_cast<void*>(data), size);
}

ConstPoolNode* BaseBuilder::newConstPoolNode() noexcept {
  ConstPoolNode* node = newNodeT<ConstPoolNode>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

CommentNode* BaseBuilder::newCommentNode(const char* data, size_t size) noexcept {
  if (data) {
    if (size == SIZE_MAX)
      size = strlen(data);

    if (size > 0) {
      data = static_cast<char*>(_dataZone.dup(data, size, true));
      if (!data) return nullptr;
    }
  }

  return newNodeT<CommentNode>(data);
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0) noexcept {
  uint32_t opCount = 1;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->resetOps(opCount, opCapacity);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1) noexcept {
  uint32_t opCount = 2;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->resetOps(opCount, opCapacity);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  uint32_t opCount = 3;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->resetOps(opCount, opCapacity);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  uint32_t opCount = 4;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  node->resetOps(opCount, opCapacity);
  return node;
}

InstNode* BaseBuilder::newInstNodeRaw(uint32_t instId, uint32_t instOptions, uint32_t opCount) noexcept {
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;
  return new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
}

BaseNode* BaseBuilder::addNode(BaseNode* node) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->isActive());

  if (!_cursor) {
    if (!_firstNode) {
      _firstNode = node;
      _lastNode = node;
    }
    else {
      node->_next = _firstNode;
      _firstNode->_prev = node;
      _firstNode = node;
    }
  }
  else {
    BaseNode* prev = _cursor;
    BaseNode* next = _cursor->next();

    node->_prev = prev;
    node->_next = next;

    prev->_next = node;
    if (next)
      next->_prev = node;
    else
      _lastNode = node;
  }

  node->addFlags(BaseNode::kFlagIsActive);
  if (node->isSection())
    _dirtySectionLinks = true;

  _cursor = node;
  return node;
}

BaseNode* BaseBuilder::addAfter(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(ref);

  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);

  BaseNode* prev = ref;
  BaseNode* next = ref->next();

  node->_prev = prev;
  node->_next = next;

  node->addFlags(BaseNode::kFlagIsActive);
  if (node->isSection())
    _dirtySectionLinks = true;

  prev->_next = node;
  if (next)
    next->_prev = node;
  else
    _lastNode = node;

  return node;
}

BaseNode* BaseBuilder::addBefore(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(node != nullptr);
  ASMJIT_ASSERT(!node->_prev);
  ASMJIT_ASSERT(!node->_next);
  ASMJIT_ASSERT(!node->isActive());
  ASMJIT_ASSERT(ref != nullptr);
  ASMJIT_ASSERT(ref->isActive());

  BaseNode* prev = ref->prev();
  BaseNode* next = ref;

  node->_prev = prev;
  node->_next = next;

  node->addFlags(BaseNode::kFlagIsActive);
  if (node->isSection())
    _dirtySectionLinks = true;

  next->_prev = node;
  if (prev)
    prev->_next = node;
  else
    _firstNode = node;

  return node;
}

BaseNode* BaseBuilder::removeNode(BaseNode* node) noexcept {
  if (!node->isActive())
    return node;

  BaseNode* prev = node->prev();
  BaseNode* next = node->next();

  if (_firstNode == node)
    _firstNode = next;
  else
    prev->_next = next;

  if (_lastNode == node)
    _lastNode  = prev;
  else
    next->_prev = prev;

  node->_prev = nullptr;
  node->_next = nullptr;
  node->clearFlags(BaseNode::kFlagIsActive);
  if (node->isSection())
    _dirtySectionLinks = true;

  if (_cursor == node)
    _cursor = prev;

  return node;
}

void BaseBuilder::removeNodes(BaseNode* first, BaseNode* last) noexcept {
  if (first == last) {
    removeNode(first);
    return;
  }

  if (!first->isActive())
    return;

  BaseNode* prev = first->prev();
  BaseNode* next = last->next();

  if (_firstNode == first)
    _firstNode = next;
  else
    prev->_next = next;

  if (_lastNode == last)
    _lastNode  = prev;
  else
    next->_prev = prev;

  BaseNode* node = first;
  uint32_t didRemoveSection = false;

  for (;;) {
    next = node->next();
    ASMJIT_ASSERT(next != nullptr);

    node->_prev = nullptr;
    node->_next = nullptr;
    node->clearFlags(BaseNode::kFlagIsActive);
    didRemoveSection |= uint32_t(node->isSection());

    if (_cursor == node)
      _cursor = prev;

    if (node == last)
      break;
    node = next;
  }

  if (didRemoveSection)
    _dirtySectionLinks = true;
}

BaseNode* BaseBuilder::setCursor(BaseNode* node) noexcept {
  BaseNode* old = _cursor;
  _cursor = node;
  return old;
}

// ============================================================================
// [asmjit::BaseBuilder - Section]
// ============================================================================

Error BaseBuilder::sectionNodeOf(SectionNode** pOut, uint32_t sectionId) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(!_code->isSectionValid(sectionId)))
    return DebugUtils::errored(kErrorInvalidSection);

  if (sectionId >= _sectionNodes.size())
    ASMJIT_PROPAGATE(_sectionNodes.resize(&_allocator, sectionId + 1));

  SectionNode* node = _sectionNodes[sectionId];
  if (!node) {
    node = newNodeT<SectionNode>(sectionId);
    if (ASMJIT_UNLIKELY(!node))
      return DebugUtils::errored(kErrorOutOfMemory);
    _sectionNodes[sectionId] = node;
  }

  *pOut = node;
  return kErrorOk;
}

Error BaseBuilder::section(Section* section) {
  SectionNode* node;
  Error err = sectionNodeOf(&node, section->id());

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  if (!node->isActive()) {
    // Insert the section at the end if it was not part of the code.
    addAfter(node, lastNode());
    _cursor = node;
  }
  else {
    // This is a bit tricky. We cache section links to make sure that
    // switching sections doesn't involve traversal in linked-list unless
    // the position of the section has changed.
    if (hasDirtySectionLinks())
      updateSectionLinks();

    if (node->_nextSection)
      _cursor = node->_nextSection->_prev;
    else
      _cursor = _lastNode;
  }

  return kErrorOk;
}

void BaseBuilder::updateSectionLinks() noexcept {
  if (!_dirtySectionLinks)
    return;

  BaseNode* node_ = _firstNode;
  SectionNode* currentSection = nullptr;

  while (node_) {
    if (node_->isSection()) {
      if (currentSection)
        currentSection->_nextSection = node_->as<SectionNode>();
      currentSection = node_->as<SectionNode>();
    }
    node_ = node_->next();
  }

  if (currentSection)
    currentSection->_nextSection = nullptr;

  _dirtySectionLinks = false;
}

// ============================================================================
// [asmjit::BaseBuilder - Labels]
// ============================================================================

Error BaseBuilder::labelNodeOf(LabelNode** pOut, uint32_t labelId) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  uint32_t index = labelId;
  if (ASMJIT_UNLIKELY(index >= _code->labelCount()))
    return DebugUtils::errored(kErrorInvalidLabel);

  if (index >= _labelNodes.size())
    ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));

  LabelNode* node = _labelNodes[index];
  if (!node) {
    node = newNodeT<LabelNode>(labelId);
    if (ASMJIT_UNLIKELY(!node))
      return DebugUtils::errored(kErrorOutOfMemory);
    _labelNodes[index] = node;
  }

  *pOut = node;
  return kErrorOk;
}

Error BaseBuilder::registerLabelNode(LabelNode* node) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  // Don't call `reportError()` from here, we are noexcept and we are called
  // by `newLabelNode()` and `newFuncNode()`, which are noexcept as well.
  LabelEntry* le;
  ASMJIT_PROPAGATE(_code->newLabelEntry(&le));
  uint32_t labelId = le->id();

  // We just added one label so it must be true.
  ASMJIT_ASSERT(_labelNodes.size() < labelId + 1);
  ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, labelId + 1));

  _labelNodes[labelId] = node;
  node->_id = labelId;

  return kErrorOk;
}

static Error BaseBuilder_newLabelInternal(BaseBuilder* self, uint32_t labelId) noexcept {
  ASMJIT_ASSERT(self->_labelNodes.size() < labelId + 1);
  LabelNode* node = self->newNodeT<LabelNode>(labelId);

  if (ASMJIT_UNLIKELY(!node))
    return DebugUtils::errored(kErrorOutOfMemory);

  ASMJIT_PROPAGATE(self->_labelNodes.resize(&self->_allocator, labelId + 1));
  self->_labelNodes[labelId] = node;
  node->_id = labelId;
  return kErrorOk;
}

Label BaseBuilder::newLabel() {
  uint32_t labelId = Globals::kInvalidId;
  if (_code) {
    LabelEntry* le;
    Error err = _code->newLabelEntry(&le);
    if (ASMJIT_UNLIKELY(err)) {
      reportError(err);
    }
    else {
      err = BaseBuilder_newLabelInternal(this, le->id());
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        labelId = le->id();
    }
  }
  return Label(labelId);
}

Label BaseBuilder::newNamedLabel(const char* name, size_t nameSize, uint32_t type, uint32_t parentId) {
  uint32_t labelId = Globals::kInvalidId;
  if (_code) {
    LabelEntry* le;
    Error err = _code->newNamedLabelEntry(&le, name, nameSize, type, parentId);
    if (ASMJIT_UNLIKELY(err)) {
      reportError(err);
    }
    else {
      err = BaseBuilder_newLabelInternal(this, le->id());
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        labelId = le->id();
    }
  }
  return Label(labelId);
}

Error BaseBuilder::bind(const Label& label) {
  LabelNode* node;
  Error err = labelNodeOf(&node, label);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Passes]
// ============================================================================

ASMJIT_FAVOR_SIZE Pass* BaseBuilder::passByName(const char* name) const noexcept {
  for (Pass* pass : _passes)
    if (strcmp(pass->name(), name) == 0)
      return pass;
  return nullptr;
}

ASMJIT_FAVOR_SIZE Error BaseBuilder::addPass(Pass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(pass == nullptr)) {
    // Since this is directly called by `addPassT()` we treat `null` argument
    // as out-of-memory condition. Otherwise it would be API misuse.
    return DebugUtils::errored(kErrorOutOfMemory);
  }
  else if (ASMJIT_UNLIKELY(pass->_cb)) {
    // Kinda weird, but okay...
    if (pass->_cb == this)
      return kErrorOk;
    return DebugUtils::errored(kErrorInvalidState);
  }

  ASMJIT_PROPAGATE(_passes.append(&_allocator, pass));
  pass->_cb = this;
  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error BaseBuilder::deletePass(Pass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(pass == nullptr))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (pass->_cb != nullptr) {
    if (pass->_cb != this)
      return DebugUtils::errored(kErrorInvalidState);

    uint32_t index = _passes.indexOf(pass);
    ASMJIT_ASSERT(index != Globals::kNotFound);

    pass->_cb = nullptr;
    _passes.removeAt(index);
  }

  pass->~Pass();
  return kErrorOk;
}

Error BaseBuilder::runPasses() {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (_passes.empty())
    return kErrorOk;

  Logger* logger = code()->logger();
  ErrorHandler* prev = errorHandler();
  PostponedErrorHandler postponed;

  Error err = kErrorOk;
  setErrorHandler(&postponed);

  for (Pass* pass : _passes) {
    _passZone.reset();
    err = pass->run(&_passZone, logger);
    if (err) break;
  }
  _passZone.reset();
  setErrorHandler(prev);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err, !postponed._message.empty() ? postponed._message.data() : nullptr);

  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Emit]
// ============================================================================

Error BaseBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
  uint32_t opCount = 4;

  if (o3.isNone()) {
    opCount = 3;
    if (o2.isNone()) {
      opCount = 2;
      if (o1.isNone()) {
        opCount = 1;
        if (o0.isNone())
          opCount = 0;
      }
    }
  }

  uint32_t options = instOptions() | globalInstOptions();
  if (options & BaseInst::kOptionReserved) {
    if (ASMJIT_UNLIKELY(!_code))
      return DebugUtils::errored(kErrorNotInitialized);

#ifndef ASMJIT_NO_VALIDATION
    // Strict validation.
    if (hasEmitterOption(kOptionStrictValidation)) {
      Operand_ opArray[4];
      opArray[0].copyFrom(o0);
      opArray[1].copyFrom(o1);
      opArray[2].copyFrom(o2);
      opArray[3].copyFrom(o3);

      Error err = InstAPI::validate(archId(), BaseInst(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
#endif

    // Clear options that should never be part of `InstNode`.
    options &= ~BaseInst::kOptionReserved;
  }

  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  const char* comment = inlineComment();

  resetInstOptions();
  resetInlineComment();

  if (ASMJIT_UNLIKELY(!node)) {
    resetExtraReg();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  node = new(node) InstNode(this, instId, options, opCount, opCapacity);
  node->setExtraReg(extraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  node->resetOps(4, opCapacity);

  if (comment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(comment, strlen(comment), true)));

  addNode(node);
  resetExtraReg();
  return kErrorOk;
}

Error BaseBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  uint32_t opCount = Globals::kMaxOpCount;
  if (o5.isNone()) {
    opCount = 5;
    if (o4.isNone())
      return _emit(instId, o0, o1, o2, o3);
  }

  uint32_t options = instOptions() | globalInstOptions();
  if (ASMJIT_UNLIKELY(options & BaseInst::kOptionReserved)) {
    if (ASMJIT_UNLIKELY(!_code))
      return DebugUtils::errored(kErrorNotInitialized);

#ifndef ASMJIT_NO_VALIDATION
    // Strict validation.
    if (hasEmitterOption(kOptionStrictValidation)) {
      Operand_ opArray[Globals::kMaxOpCount];
      opArray[0].copyFrom(o0);
      opArray[1].copyFrom(o1);
      opArray[2].copyFrom(o2);
      opArray[3].copyFrom(o3);
      opArray[4].copyFrom(o4);
      opArray[5].copyFrom(o5);

      Error err = InstAPI::validate(archId(), BaseInst(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
#endif

    // Clear options that should never be part of `InstNode`.
    options &= ~BaseInst::kOptionReserved;
  }

  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= opCount);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  const char* comment = inlineComment();

  resetInstOptions();
  resetInlineComment();

  if (ASMJIT_UNLIKELY(!node)) {
    resetExtraReg();
    return reportError(DebugUtils::errored(kErrorOutOfMemory));
  }

  node = new(node) InstNode(this, instId, options, opCount, opCapacity);
  node->setExtraReg(extraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  node->setOp(4, o4);

  if (opCapacity > 5)
    node->setOp(5, o5);

  if (comment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(comment, strlen(comment), true)));

  addNode(node);
  resetExtraReg();
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Align]
// ============================================================================

Error BaseBuilder::align(uint32_t alignMode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  AlignNode* node = newAlignNode(alignMode, alignment);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Embed]
// ============================================================================

Error BaseBuilder::embed(const void* data, uint32_t dataSize) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  EmbedDataNode* node = newEmbedDataNode(data, dataSize);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedLabel(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  EmbedLabelNode* node = newNodeT<EmbedLabelNode>(label.id());
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedLabelDelta(const Label& label, const Label& base, uint32_t dataSize) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  EmbedLabelDeltaNode* node = newNodeT<EmbedLabelDeltaNode>(label.id(), base.id(), dataSize);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (!isLabelValid(label))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  ASMJIT_PROPAGATE(align(kAlignData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  EmbedDataNode* node = newEmbedDataNode(nullptr, uint32_t(pool.size()));
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  pool.fill(node->data());
  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Comment]
// ============================================================================

Error BaseBuilder::comment(const char* data, size_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CommentNode* node = newCommentNode(data, size);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorOutOfMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Serialize]
// ============================================================================

Error BaseBuilder::serialize(BaseEmitter* dst) {
  Error err = kErrorOk;
  BaseNode* node_ = _firstNode;

  do {
    dst->setInlineComment(node_->inlineComment());

    if (node_->isInst()) {
      InstNode* node = node_->as<InstNode>();
      err = dst->emitInst(node->baseInst(), node->operands(), node->opCount());
    }
    else if (node_->isLabel()) {
      if (node_->isConstPool()) {
        ConstPoolNode* node = node_->as<ConstPoolNode>();
        err = dst->embedConstPool(node->label(), node->constPool());
      }
      else {
        LabelNode* node = node_->as<LabelNode>();
        err = dst->bind(node->label());
      }
    }
    else if (node_->isAlign()) {
      AlignNode* node = node_->as<AlignNode>();
      err = dst->align(node->alignMode(), node->alignment());
    }
    else if (node_->isEmbedData()) {
      EmbedDataNode* node = node_->as<EmbedDataNode>();
      err = dst->embed(node->data(), node->size());
    }
    else if (node_->isEmbedLabel()) {
      EmbedLabelNode* node = node_->as<EmbedLabelNode>();
      err = dst->embedLabel(node->label());
    }
    else if (node_->isEmbedLabelDelta()) {
      EmbedLabelDeltaNode* node = node_->as<EmbedLabelDeltaNode>();
      err = dst->embedLabelDelta(node->label(), node->baseLabel(), node->dataSize());
    }
    else if (node_->isSection()) {
      SectionNode* node = node_->as<SectionNode>();
      err = dst->section(_code->sectionById(node->id()));
    }
    else if (node_->isComment()) {
      CommentNode* node = node_->as<CommentNode>();
      err = dst->comment(node->inlineComment());
    }

    if (err) break;
    node_ = node_->next();
  } while (node_);

  return err;
}

// ============================================================================
// [asmjit::BaseBuilder - Logging]
// ============================================================================

#ifndef ASMJIT_NO_LOGGING
Error BaseBuilder::dump(String& sb, uint32_t flags) const noexcept {
  BaseNode* node = _firstNode;
  while (node) {
    ASMJIT_PROPAGATE(Logging::formatNode(sb, flags, this, node));
    sb.appendChar('\n');
    node = node->next();
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::BaseBuilder - Events]
// ============================================================================

Error BaseBuilder::onAttach(CodeHolder* code) noexcept {
  ASMJIT_PROPAGATE(Base::onAttach(code));

  SectionNode* initialSection;
  Error err = sectionNodeOf(&initialSection, 0);

  if (!err)
    err = _passes.willGrow(&_allocator, 8);

  if (ASMJIT_UNLIKELY(err)) {
    onDetach(code);
    return err;
  }


  _cursor = initialSection;
  _firstNode = initialSection;
  _lastNode = initialSection;
  initialSection->setFlags(BaseNode::kFlagIsActive);

  return kErrorOk;
}

Error BaseBuilder::onDetach(CodeHolder* code) noexcept {
  _passes.reset();
  _sectionNodes.reset();
  _labelNodes.reset();

  _allocator.reset(&_codeZone);
  _codeZone.reset();
  _dataZone.reset();
  _passZone.reset();

  _nodeFlags = 0;

  _cursor = nullptr;
  _firstNode = nullptr;
  _lastNode = nullptr;

  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::Pass - Construction / Destruction]
// ============================================================================

Pass::Pass(const char* name) noexcept
  : _cb(nullptr),
    _name(name) {}
Pass::~Pass() noexcept {}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_BUILDER
