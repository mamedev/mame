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
#ifndef ASMJIT_NO_LOGGING

#include "../core/builder.h"
#include "../core/codeholder.h"
#include "../core/compiler.h"
#include "../core/emitter.h"
#include "../core/logging.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/type.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86logging_p.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/armlogging_p.h"
#endif

ASMJIT_BEGIN_NAMESPACE

#if defined(ASMJIT_NO_COMPILER)
class VirtReg;
#endif

// ============================================================================
// [asmjit::Logger - Construction / Destruction]
// ============================================================================

Logger::Logger() noexcept
  : _options() {}
Logger::~Logger() noexcept {}

// ============================================================================
// [asmjit::Logger - Logging]
// ============================================================================

Error Logger::logf(const char* fmt, ...) noexcept {
  Error err;
  va_list ap;

  va_start(ap, fmt);
  err = logv(fmt, ap);
  va_end(ap);

  return err;
}

Error Logger::logv(const char* fmt, va_list ap) noexcept {
  StringTmp<2048> sb;
  ASMJIT_PROPAGATE(sb.appendVFormat(fmt, ap));
  return log(sb);
}

Error Logger::logBinary(const void* data, size_t size) noexcept {
  static const char prefix[] = "db ";

  StringTmp<256> sb;
  sb.appendString(prefix, ASMJIT_ARRAY_SIZE(prefix) - 1);

  size_t i = size;
  const uint8_t* s = static_cast<const uint8_t*>(data);

  while (i) {
    uint32_t n = uint32_t(Support::min<size_t>(i, 16));
    sb.truncate(ASMJIT_ARRAY_SIZE(prefix) - 1);
    sb.appendHex(s, n);
    sb.appendChar('\n');
    ASMJIT_PROPAGATE(log(sb));
    s += n;
    i -= n;
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::FileLogger - Construction / Destruction]
// ============================================================================

FileLogger::FileLogger(FILE* file) noexcept
  : _file(nullptr) { setFile(file); }
FileLogger::~FileLogger() noexcept {}

// ============================================================================
// [asmjit::FileLogger - Logging]
// ============================================================================

Error FileLogger::_log(const char* data, size_t size) noexcept {
  if (!_file)
    return kErrorOk;

  if (size == SIZE_MAX)
    size = strlen(data);

  fwrite(data, 1, size, _file);
  return kErrorOk;
}

// ============================================================================
// [asmjit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger() noexcept {}
StringLogger::~StringLogger() noexcept {}

// ============================================================================
// [asmjit::StringLogger - Logging]
// ============================================================================

Error StringLogger::_log(const char* data, size_t size) noexcept {
  return _content.appendString(data, size);
}

// ============================================================================
// [asmjit::Logging]
// ============================================================================

Error Logging::formatLabel(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  uint32_t labelId) noexcept {

  DebugUtils::unused(flags);

  const LabelEntry* le = emitter->code()->labelEntry(labelId);
  if (ASMJIT_UNLIKELY(!le))
    return sb.appendFormat("InvalidLabel[Id=%u]", labelId);

  if (le->hasName()) {
    if (le->hasParent()) {
      uint32_t parentId = le->parentId();
      const LabelEntry* pe = emitter->code()->labelEntry(parentId);

      if (ASMJIT_UNLIKELY(!pe))
        ASMJIT_PROPAGATE(sb.appendFormat("InvalidLabel[Id=%u]", labelId));
      else if (ASMJIT_UNLIKELY(!pe->hasName()))
        ASMJIT_PROPAGATE(sb.appendFormat("L%u", parentId));
      else
        ASMJIT_PROPAGATE(sb.appendString(pe->name()));

      ASMJIT_PROPAGATE(sb.appendChar('.'));
    }
    return sb.appendString(le->name());
  }
  else {
    return sb.appendFormat("L%u", labelId);
  }
}

Error Logging::formatRegister(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  uint32_t archId,
  uint32_t regType,
  uint32_t regId) noexcept {

#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::LoggingInternal::formatRegister(sb, flags, emitter, archId, regType, regId);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::LoggingInternal::formatRegister(sb, flags, emitter, archId, regType, regId);
#endif

  return kErrorInvalidArch;
}

Error Logging::formatOperand(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  uint32_t archId,
  const Operand_& op) noexcept {

#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::LoggingInternal::formatOperand(sb, flags, emitter, archId, op);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::LoggingInternal::formatOperand(sb, flags, emitter, archId, op);
#endif

  return kErrorInvalidArch;
}

Error Logging::formatInstruction(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  uint32_t archId,
  const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept {

#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::LoggingInternal::formatInstruction(sb, flags, emitter, archId, inst, operands, opCount);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::LoggingInternal::formatInstruction(sb, flags, emitter, archId, inst, operands, opCount);
#endif

  return kErrorInvalidArch;
}

Error Logging::formatTypeId(String& sb, uint32_t typeId) noexcept {
  if (typeId == Type::kIdVoid)
    return sb.appendString("void");

  if (!Type::isValid(typeId))
    return sb.appendString("unknown");

  const char* typeName = "unknown";
  uint32_t typeSize = Type::sizeOf(typeId);

  uint32_t baseId = Type::baseOf(typeId);
  switch (baseId) {
    case Type::kIdIntPtr : typeName = "iptr"  ; break;
    case Type::kIdUIntPtr: typeName = "uptr"  ; break;
    case Type::kIdI8     : typeName = "i8"    ; break;
    case Type::kIdU8     : typeName = "u8"    ; break;
    case Type::kIdI16    : typeName = "i16"   ; break;
    case Type::kIdU16    : typeName = "u16"   ; break;
    case Type::kIdI32    : typeName = "i32"   ; break;
    case Type::kIdU32    : typeName = "u32"   ; break;
    case Type::kIdI64    : typeName = "i64"   ; break;
    case Type::kIdU64    : typeName = "u64"   ; break;
    case Type::kIdF32    : typeName = "f32"   ; break;
    case Type::kIdF64    : typeName = "f64"   ; break;
    case Type::kIdF80    : typeName = "f80"   ; break;
    case Type::kIdMask8  : typeName = "mask8" ; break;
    case Type::kIdMask16 : typeName = "mask16"; break;
    case Type::kIdMask32 : typeName = "mask32"; break;
    case Type::kIdMask64 : typeName = "mask64"; break;
    case Type::kIdMmx32  : typeName = "mmx32" ; break;
    case Type::kIdMmx64  : typeName = "mmx64" ; break;
  }

  uint32_t baseSize = Type::sizeOf(baseId);
  if (typeSize > baseSize) {
    uint32_t count = typeSize / baseSize;
    return sb.appendFormat("%sx%u", typeName, unsigned(count));
  }
  else {
    return sb.appendString(typeName);
  }

}

#ifndef ASMJIT_NO_BUILDER
static Error formatFuncValue(String& sb, uint32_t flags, const BaseEmitter* emitter, FuncValue value) noexcept {
  uint32_t typeId = value.typeId();
  ASMJIT_PROPAGATE(Logging::formatTypeId(sb, typeId));

  if (value.isReg()) {
    ASMJIT_PROPAGATE(sb.appendChar('@'));
    ASMJIT_PROPAGATE(Logging::formatRegister(sb, flags, emitter, emitter->archId(), value.regType(), value.regId()));
  }

  if (value.isStack()) {
    ASMJIT_PROPAGATE(sb.appendFormat("@[%d]", int(value.stackOffset())));
  }

  return kErrorOk;
}

static Error formatFuncRets(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  const FuncDetail& fd,
  VirtReg* const* vRegs) noexcept {

  if (!fd.hasRet())
    return sb.appendString("void");

  for (uint32_t i = 0; i < fd.retCount(); i++) {
    if (i) ASMJIT_PROPAGATE(sb.appendString(", "));
    ASMJIT_PROPAGATE(formatFuncValue(sb, flags, emitter, fd.ret(i)));

#ifndef ASMJIT_NO_COMPILER
    if (vRegs) {
      static const char nullRet[] = "<none>";
      ASMJIT_PROPAGATE(sb.appendFormat(" %s", vRegs[i] ? vRegs[i]->name() : nullRet));
    }
#else
    DebugUtils::unused(vRegs);
#endif
  }

  return kErrorOk;
}

static Error formatFuncArgs(
  String& sb,
  uint32_t flags,
  const BaseEmitter* emitter,
  const FuncDetail& fd,
  VirtReg* const* vRegs) noexcept {

  uint32_t count = fd.argCount();
  if (!count)
    return sb.appendString("void");

  for (uint32_t i = 0; i < count; i++) {
    if (i) ASMJIT_PROPAGATE(sb.appendString(", "));
    ASMJIT_PROPAGATE(formatFuncValue(sb, flags, emitter, fd.arg(i)));

#ifndef ASMJIT_NO_COMPILER
    if (vRegs) {
      static const char nullArg[] = "<none>";
      ASMJIT_PROPAGATE(sb.appendFormat(" %s", vRegs[i] ? vRegs[i]->name() : nullArg));
    }
#else
    DebugUtils::unused(vRegs);
#endif
  }

  return kErrorOk;
}

Error Logging::formatNode(
  String& sb,
  uint32_t flags,
  const BaseBuilder* cb,
  const BaseNode* node_) noexcept {

  if (node_->hasPosition() && (flags & FormatOptions::kFlagPositions) != 0)
    ASMJIT_PROPAGATE(sb.appendFormat("<%05u> ", node_->position()));

  switch (node_->type()) {
    case BaseNode::kNodeInst:
    case BaseNode::kNodeJump: {
      const InstNode* node = node_->as<InstNode>();
      ASMJIT_PROPAGATE(
        Logging::formatInstruction(sb, flags, cb,
          cb->archId(),
          node->baseInst(), node->operands(), node->opCount()));
      break;
    }

    case BaseNode::kNodeSection: {
      const SectionNode* node = node_->as<SectionNode>();
      if (cb->_code->isSectionValid(node->id())) {
        const Section* section = cb->_code->sectionById(node->id());
        ASMJIT_PROPAGATE(sb.appendFormat(".section %s", section->name()));
      }
      break;
    }

    case BaseNode::kNodeLabel: {
      const LabelNode* node = node_->as<LabelNode>();
      ASMJIT_PROPAGATE(formatLabel(sb, flags, cb, node->id()));
      ASMJIT_PROPAGATE(sb.appendString(":"));
      break;
    }

    case BaseNode::kNodeAlign: {
      const AlignNode* node = node_->as<AlignNode>();
      ASMJIT_PROPAGATE(
        sb.appendFormat(".align %u (%s)",
          node->alignment(),
          node->alignMode() == kAlignCode ? "code" : "data"));
      break;
    }

    case BaseNode::kNodeEmbedData: {
      const EmbedDataNode* node = node_->as<EmbedDataNode>();
      ASMJIT_PROPAGATE(sb.appendFormat(".embed (%u bytes)", node->size()));
      break;
    }

    case BaseNode::kNodeEmbedLabel: {
      const EmbedLabelNode* node = node_->as<EmbedLabelNode>();
      ASMJIT_PROPAGATE(sb.appendString(".label "));
      ASMJIT_PROPAGATE(formatLabel(sb, flags, cb, node->id()));
      break;
    }

    case BaseNode::kNodeEmbedLabelDelta: {
      const EmbedLabelDeltaNode* node = node_->as<EmbedLabelDeltaNode>();
      ASMJIT_PROPAGATE(sb.appendString(".label ("));
      ASMJIT_PROPAGATE(formatLabel(sb, flags, cb, node->id()));
      ASMJIT_PROPAGATE(sb.appendString(" - "));
      ASMJIT_PROPAGATE(formatLabel(sb, flags, cb, node->baseId()));
      ASMJIT_PROPAGATE(sb.appendString(")"));
      break;
    }

    case BaseNode::kNodeComment: {
      const CommentNode* node = node_->as<CommentNode>();
      ASMJIT_PROPAGATE(sb.appendFormat("; %s", node->inlineComment()));
      break;
    }

    case BaseNode::kNodeSentinel: {
      const SentinelNode* node = node_->as<SentinelNode>();
      const char* sentinelName = nullptr;

      switch (node->sentinelType()) {
        case SentinelNode::kSentinelFuncEnd:
          sentinelName = "[FuncEnd]";
          break;

        default:
          sentinelName = "[Sentinel]";
          break;
      }

      ASMJIT_PROPAGATE(sb.appendString(sentinelName));
      break;
    }

#ifndef ASMJIT_NO_COMPILER
    case BaseNode::kNodeFunc: {
      const FuncNode* node = node_->as<FuncNode>();

      ASMJIT_PROPAGATE(formatLabel(sb, flags, cb, node->id()));
      ASMJIT_PROPAGATE(sb.appendString(": "));

      ASMJIT_PROPAGATE(formatFuncRets(sb, flags, cb, node->detail(), nullptr));
      ASMJIT_PROPAGATE(sb.appendString(" Func("));
      ASMJIT_PROPAGATE(formatFuncArgs(sb, flags, cb, node->detail(), node->args()));
      ASMJIT_PROPAGATE(sb.appendString(")"));
      break;
    }

    case BaseNode::kNodeFuncRet: {
      const FuncRetNode* node = node_->as<FuncRetNode>();
      ASMJIT_PROPAGATE(sb.appendString("[FuncRet]"));

      for (uint32_t i = 0; i < 2; i++) {
        const Operand_& op = node->_opArray[i];
        if (!op.isNone()) {
          ASMJIT_PROPAGATE(sb.appendString(i == 0 ? " " : ", "));
          ASMJIT_PROPAGATE(formatOperand(sb, flags, cb, cb->archId(), op));
        }
      }
      break;
    }

    case BaseNode::kNodeFuncCall: {
      const FuncCallNode* node = node_->as<FuncCallNode>();
      ASMJIT_PROPAGATE(
        Logging::formatInstruction(sb, flags, cb,
          cb->archId(),
          node->baseInst(), node->operands(), node->opCount()));
      break;
    }
#endif

    default: {
      ASMJIT_PROPAGATE(sb.appendFormat("[User:%u]", node_->type()));
      break;
    }
  }

  return kErrorOk;
}
#endif

Error Logging::formatLine(String& sb, const uint8_t* binData, size_t binSize, size_t dispSize, size_t immSize, const char* comment) noexcept {
  size_t currentSize = sb.size();
  size_t commentSize = comment ? Support::strLen(comment, Globals::kMaxCommentSize) : 0;

  ASMJIT_ASSERT(binSize >= dispSize);
  const size_t kNoBinSize = std::numeric_limits<size_t>::max();

  if ((binSize != 0 && binSize != kNoBinSize) || commentSize) {
    size_t align = kMaxInstLineSize;
    char sep = ';';

    for (size_t i = (binSize == kNoBinSize); i < 2; i++) {
      size_t begin = sb.size();
      ASMJIT_PROPAGATE(sb.padEnd(align));

      if (sep) {
        ASMJIT_PROPAGATE(sb.appendChar(sep));
        ASMJIT_PROPAGATE(sb.appendChar(' '));
      }

      // Append binary data or comment.
      if (i == 0) {
        ASMJIT_PROPAGATE(sb.appendHex(binData, binSize - dispSize - immSize));
        ASMJIT_PROPAGATE(sb.appendChars('.', dispSize * 2));
        ASMJIT_PROPAGATE(sb.appendHex(binData + binSize - immSize, immSize));
        if (commentSize == 0) break;
      }
      else {
        ASMJIT_PROPAGATE(sb.appendString(comment, commentSize));
      }

      currentSize += sb.size() - begin;
      align += kMaxBinarySize;
      sep = '|';
    }
  }

  return sb.appendChar('\n');
}

ASMJIT_END_NAMESPACE

#endif
