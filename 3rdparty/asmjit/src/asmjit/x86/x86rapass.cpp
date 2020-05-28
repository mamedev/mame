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
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_NO_COMPILER)

#include "../core/cpuinfo.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86instapi_p.h"
#include "../x86/x86instdb_p.h"
#include "../x86/x86internal_p.h"
#include "../x86/x86rapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::X86RAPass - Helpers]
// ============================================================================

static ASMJIT_INLINE uint64_t raImmMaskFromSize(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static const uint64_t masks[] = {
    0x00000000000000FFu, //   1
    0x000000000000FFFFu, //   2
    0x00000000FFFFFFFFu, //   4
    0xFFFFFFFFFFFFFFFFu, //   8
    0x0000000000000000u, //  16
    0x0000000000000000u, //  32
    0x0000000000000000u, //  64
    0x0000000000000000u, // 128
    0x0000000000000000u  // 256
  };
  return masks[Support::ctz(size)];
}

static ASMJIT_INLINE uint32_t raUseOutFlagsFromRWFlags(uint32_t rwFlags) noexcept {
  static const uint32_t map[] = {
    0,
    RATiedReg::kRead  | RATiedReg::kUse,                     // kRead
    RATiedReg::kWrite | RATiedReg::kOut,                     // kWrite
    RATiedReg::kRW    | RATiedReg::kUse,                     // kRW
    0,
    RATiedReg::kRead  | RATiedReg::kUse | RATiedReg::kUseRM, // kRead  | kRegMem
    RATiedReg::kWrite | RATiedReg::kOut | RATiedReg::kOutRM, // kWrite | kRegMem
    RATiedReg::kRW    | RATiedReg::kUse | RATiedReg::kUseRM  // kRW    | kRegMem
  };

  return map[rwFlags & (OpRWInfo::kRW | OpRWInfo::kRegMem)];
}

static ASMJIT_INLINE uint32_t raRegRwFlags(uint32_t flags) noexcept {
  return raUseOutFlagsFromRWFlags(flags);
}

static ASMJIT_INLINE uint32_t raMemBaseRwFlags(uint32_t flags) noexcept {
  constexpr uint32_t shift = Support::constCtz(OpRWInfo::kMemBaseRW);
  return raUseOutFlagsFromRWFlags((flags >> shift) & OpRWInfo::kRW);
}

static ASMJIT_INLINE uint32_t raMemIndexRwFlags(uint32_t flags) noexcept {
  constexpr uint32_t shift = Support::constCtz(OpRWInfo::kMemIndexRW);
  return raUseOutFlagsFromRWFlags((flags >> shift) & OpRWInfo::kRW);
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder]
// ============================================================================

class X86RACFGBuilder : public RACFGBuilder<X86RACFGBuilder> {
public:
  uint32_t _archId;
  bool _is64Bit;
  bool _avxEnabled;

  inline X86RACFGBuilder(X86RAPass* pass) noexcept
    : RACFGBuilder<X86RACFGBuilder>(pass),
      _archId(pass->cc()->archId()),
      _is64Bit(pass->gpSize() == 8),
      _avxEnabled(pass->_avxEnabled) {
  }

  inline Compiler* cc() const noexcept { return static_cast<Compiler*>(_cc); }

  inline uint32_t choose(uint32_t sseInst, uint32_t avxInst) const noexcept {
    return _avxEnabled ? avxInst : sseInst;
  }

  Error onInst(InstNode* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept;

  Error onBeforeCall(FuncCallNode* call) noexcept;
  Error onCall(FuncCallNode* call, RAInstBuilder& ib) noexcept;

  Error moveImmToRegArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_, BaseReg* out) noexcept;
  Error moveImmToStackArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_) noexcept;
  Error moveRegToStackArg(FuncCallNode* call, const FuncValue& arg, const BaseReg& reg) noexcept;

  Error onBeforeRet(FuncRetNode* funcRet) noexcept;
  Error onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept;
};

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnInst]
// ============================================================================

Error X86RACFGBuilder::onInst(InstNode* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept {
  InstRWInfo rwInfo;

  uint32_t instId = inst->id();
  if (Inst::isDefinedId(instId)) {
    uint32_t opCount = inst->opCount();
    const Operand* opArray = inst->operands();
    ASMJIT_PROPAGATE(InstInternal::queryRWInfo(_archId, inst->baseInst(), opArray, opCount, rwInfo));

    const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
    bool hasGpbHiConstraint = false;
    uint32_t singleRegOps = 0;

    if (opCount) {
      for (uint32_t i = 0; i < opCount; i++) {
        const Operand& op = opArray[i];
        const OpRWInfo& opRwInfo = rwInfo.operand(i);

        if (op.isReg()) {
          // Register Operand
          // ----------------
          const Reg& reg = op.as<Reg>();

          uint32_t flags = raRegRwFlags(opRwInfo.opFlags());
          uint32_t allowedRegs = 0xFFFFFFFFu;

          // X86-specific constraints related to LO|HI general purpose registers.
          // This is only required when the register is part of the encoding. If
          // the register is fixed we won't restrict anything as it doesn't restrict
          // encoding of other registers.
          if (reg.isGpb() && !(opRwInfo.opFlags() & OpRWInfo::kRegPhysId)) {
            flags |= RATiedReg::kX86Gpb;
            if (!_is64Bit) {
              // Restrict to first four - AL|AH|BL|BH|CL|CH|DL|DH. In 32-bit mode
              // it's not possible to access SIL|DIL, etc, so this is just enough.
              allowedRegs = 0x0Fu;
            }
            else {
              // If we encountered GPB-HI register the situation is much more
              // complicated than in 32-bit mode. We need to patch all registers
              // to not use ID higher than 7 and all GPB-LO registers to not use
              // index higher than 3. Instead of doing the patching here we just
              // set a flag and will do it later, to not complicate this loop.
              if (reg.isGpbHi()) {
                hasGpbHiConstraint = true;
                allowedRegs = 0x0Fu;
              }
            }
          }

          uint32_t vIndex = Operand::virtIdToIndex(reg.id());
          if (vIndex < Operand::kVirtIdCount) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

            // Use RW instead of Write in case that not the whole register is
            // overwritten. This is important for liveness as we cannot kill a
            // register that will be used. For example `mov al, 0xFF` is not a
            // write-only operation if user allocated the whole `rax` register.
            if ((flags & RATiedReg::kRW) == RATiedReg::kWrite) {
              if (workReg->regByteMask() & ~(opRwInfo.writeByteMask() | opRwInfo.extendByteMask())) {
                // Not write-only operation.
                flags = (flags & ~RATiedReg::kOut) | (RATiedReg::kRead | RATiedReg::kUse);
              }
            }

            // Do not use RegMem flag if changing Reg to Mem requires additional
            // CPU feature that may not be enabled.
            if (rwInfo.rmFeature() && (flags & (RATiedReg::kUseRM | RATiedReg::kOutRM))) {
              flags &= ~(RATiedReg::kUseRM | RATiedReg::kOutRM);
            }

            uint32_t group = workReg->group();
            uint32_t allocable = _pass->_availableRegs[group] & allowedRegs;

            uint32_t useId = BaseReg::kIdBad;
            uint32_t outId = BaseReg::kIdBad;

            uint32_t useRewriteMask = 0;
            uint32_t outRewriteMask = 0;

            if (flags & RATiedReg::kUse) {
              useRewriteMask = Support::bitMask(inst->getRewriteIndex(&reg._baseId));
              if (opRwInfo.opFlags() & OpRWInfo::kRegPhysId) {
                useId = opRwInfo.physId();
                flags |= RATiedReg::kUseFixed;
              }
            }
            else {
              outRewriteMask = Support::bitMask(inst->getRewriteIndex(&reg._baseId));
              if (opRwInfo.opFlags() & OpRWInfo::kRegPhysId) {
                outId = opRwInfo.physId();
                flags |= RATiedReg::kOutFixed;
              }
            }

            ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask, opRwInfo.rmSize()));
            if (singleRegOps == i)
              singleRegOps++;
          }
        }
        else if (op.isMem()) {
          // Memory Operand
          // --------------
          const Mem& mem = op.as<Mem>();
          ib.addForbiddenFlags(RATiedReg::kUseRM | RATiedReg::kOutRM);

          if (mem.isRegHome()) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(mem.baseId()), &workReg));
            _pass->getOrCreateStackSlot(workReg);
          }
          else if (mem.hasBaseReg()) {
            uint32_t vIndex = Operand::virtIdToIndex(mem.baseId());
            if (vIndex < Operand::kVirtIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t flags = raMemBaseRwFlags(opRwInfo.opFlags());
              uint32_t group = workReg->group();
              uint32_t allocable = _pass->_availableRegs[group];

              uint32_t useId = BaseReg::kIdBad;
              uint32_t outId = BaseReg::kIdBad;

              uint32_t useRewriteMask = 0;
              uint32_t outRewriteMask = 0;

              if (flags & RATiedReg::kUse) {
                useRewriteMask = Support::bitMask(inst->getRewriteIndex(&mem._baseId));
                if (opRwInfo.opFlags() & OpRWInfo::kMemPhysId) {
                  useId = opRwInfo.physId();
                  flags |= RATiedReg::kUseFixed;
                }
              }
              else {
                outRewriteMask = Support::bitMask(inst->getRewriteIndex(&mem._baseId));
                if (opRwInfo.opFlags() & OpRWInfo::kMemPhysId) {
                  outId = opRwInfo.physId();
                  flags |= RATiedReg::kOutFixed;
                }
              }

              ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));
            }
          }

          if (mem.hasIndexReg()) {
            uint32_t vIndex = Operand::virtIdToIndex(mem.indexId());
            if (vIndex < Operand::kVirtIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t flags = raMemIndexRwFlags(opRwInfo.opFlags());
              uint32_t group = workReg->group();
              uint32_t allocable = _pass->_availableRegs[group];

              // Index registers have never fixed id on X86/x64.
              const uint32_t useId = BaseReg::kIdBad;
              const uint32_t outId = BaseReg::kIdBad;

              uint32_t useRewriteMask = 0;
              uint32_t outRewriteMask = 0;

              if (flags & RATiedReg::kUse)
                useRewriteMask = Support::bitMask(inst->getRewriteIndex(&mem._data[Operand::kDataMemIndexId]));
              else
                outRewriteMask = Support::bitMask(inst->getRewriteIndex(&mem._data[Operand::kDataMemIndexId]));

              ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, useId, useRewriteMask, outId, outRewriteMask));
            }
          }
        }
      }
    }

    // Handle extra operand (either REP {cx|ecx|rcx} or AVX-512 {k} selector).
    if (inst->hasExtraReg()) {
      uint32_t vIndex = Operand::virtIdToIndex(inst->extraReg().id());
      if (vIndex < Operand::kVirtIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->group();
        uint32_t rewriteMask = Support::bitMask(inst->getRewriteIndex(&inst->extraReg()._id));

        if (group == Gp::kGroupKReg) {
          // AVX-512 mask selector {k} register - read-only, allocable to any register except {k0}.
          uint32_t allocableRegs= _pass->_availableRegs[group] & ~Support::bitMask(0);
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocableRegs, BaseReg::kIdBad, rewriteMask, BaseReg::kIdBad, 0));
          singleRegOps = 0;
        }
        else {
          // REP {cx|ecx|rcx} register - read & write, allocable to {cx|ecx|rcx} only.
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRW, 0, Gp::kIdCx, rewriteMask, Gp::kIdBad, 0));
        }
      }
      else {
        uint32_t group = inst->extraReg().group();
        if (group == Gp::kGroupKReg && inst->extraReg().id() != 0)
          singleRegOps = 0;
      }
    }

    // Handle X86 constraints.
    if (hasGpbHiConstraint) {
      for (RATiedReg& tiedReg : ib) {
        tiedReg._allocableRegs &= tiedReg.hasFlag(RATiedReg::kX86Gpb) ? 0x0Fu : 0xFFu;
      }
    }

    if (ib.tiedRegCount() == 1) {
      // Handle special cases of some instructions where all operands share the same
      // register. In such case the single operand becomes read-only or write-only.
      uint32_t singleRegCase = InstDB::kSingleRegNone;
      if (singleRegOps == opCount) {
        singleRegCase = instInfo.singleRegCase();
      }
      else if (opCount == 2 && inst->opType(1).isImm()) {
        // Handle some tricks used by X86 asm.
        const BaseReg& reg = inst->opType(0).as<BaseReg>();
        const Imm& imm = inst->opType(1).as<Imm>();

        const RAWorkReg* workReg = _pass->workRegById(ib[0]->workId());
        uint32_t workRegSize = workReg->info().size();

        switch (inst->id()) {
          case Inst::kIdOr: {
            // Sets the value of the destination register to -1, previous content unused.
            if (reg.size() >= 4 || reg.size() >= workRegSize) {
              if (imm.i64() == -1 || imm.u64() == raImmMaskFromSize(reg.size()))
                singleRegCase = InstDB::kSingleRegWO;
            }
            ASMJIT_FALLTHROUGH;
          }

          case Inst::kIdAdd:
          case Inst::kIdAnd:
          case Inst::kIdRol:
          case Inst::kIdRor:
          case Inst::kIdSar:
          case Inst::kIdShl:
          case Inst::kIdShr:
          case Inst::kIdSub:
          case Inst::kIdXor: {
            // Updates [E|R]FLAGS without changing the content.
            if (reg.size() != 4 || reg.size() >= workRegSize) {
              if (imm.u64() == 0)
                singleRegCase = InstDB::kSingleRegRO;
            }
            break;
          }
        }
      }

      switch (singleRegCase) {
        case InstDB::kSingleRegNone:
          break;
        case InstDB::kSingleRegRO:
          ib[0]->makeReadOnly();
          break;
        case InstDB::kSingleRegWO:
          ib[0]->makeWriteOnly();
          break;
      }
    }

    controlType = instInfo.controlType();
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnCall]
// ============================================================================

Error X86RACFGBuilder::onBeforeCall(FuncCallNode* call) noexcept {
  uint32_t argCount = call->argCount();
  uint32_t retCount = call->retCount();
  const FuncDetail& fd = call->detail();

  cc()->_setCursor(call->prev());

  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
      if (!fd.hasArg(argIndex + argHi))
        continue;

      const FuncValue& arg = fd.arg(argIndex + argHi);
      const Operand& op = call->arg(argIndex + argHi);

      if (op.isNone())
        continue;

      if (op.isReg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

        if (arg.isReg()) {
          uint32_t regGroup = workReg->group();
          uint32_t argGroup = Reg::groupOf(arg.regType());

          if (regGroup != argGroup) {
            // TODO:
            ASMJIT_ASSERT(false);
          }
        }
        else {
          ASMJIT_PROPAGATE(moveRegToStackArg(call, arg, op.as<BaseReg>()));
        }
      }
      else if (op.isImm()) {
        if (arg.isReg()) {
          BaseReg reg;
          ASMJIT_PROPAGATE(moveImmToRegArg(call, arg, op.as<Imm>(), &reg));
          call->_args[argIndex + argHi] = reg;
        }
        else {
          ASMJIT_PROPAGATE(moveImmToStackArg(call, arg, op.as<Imm>()));
        }
      }
    }
  }

  cc()->_setCursor(call);
  if (fd.hasFlag(CallConv::kFlagCalleePopsStack))
    ASMJIT_PROPAGATE(cc()->sub(cc()->zsp(), fd.argStackSize()));

  for (uint32_t retIndex = 0; retIndex < retCount; retIndex++) {
    const FuncValue& ret = fd.ret(retIndex);
    const Operand& op = call->ret(retIndex);

    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      if (ret.isReg()) {
        if (ret.regType() == Reg::kTypeSt) {
          if (workReg->group() != Reg::kGroupVec)
            return DebugUtils::errored(kErrorInvalidAssignment);

          Reg dst = Reg(workReg->signature(), workReg->virtId());
          Mem mem;

          uint32_t typeId = Type::baseOf(workReg->typeId());
          if (ret.hasTypeId())
            typeId = ret.typeId();

          switch (typeId) {
            case Type::kIdF32:
              ASMJIT_PROPAGATE(_pass->useTemporaryMem(mem, 4, 4));
              mem.setSize(4);
              ASMJIT_PROPAGATE(cc()->fstp(mem));
              ASMJIT_PROPAGATE(cc()->emit(choose(Inst::kIdMovss, Inst::kIdVmovss), dst.as<Xmm>(), mem));
              break;

            case Type::kIdF64:
              ASMJIT_PROPAGATE(_pass->useTemporaryMem(mem, 8, 4));
              mem.setSize(8);
              ASMJIT_PROPAGATE(cc()->fstp(mem));
              ASMJIT_PROPAGATE(cc()->emit(choose(Inst::kIdMovsd, Inst::kIdVmovsd), dst.as<Xmm>(), mem));
              break;

            default:
              return DebugUtils::errored(kErrorInvalidAssignment);
          }
        }
        else {
          uint32_t regGroup = workReg->group();
          uint32_t retGroup = Reg::groupOf(ret.regType());

          if (regGroup != retGroup) {
            // TODO:
            ASMJIT_ASSERT(false);
          }
        }
      }
    }
  }

  // This block has function call(s).
  _curBlock->addFlags(RABlock::kFlagHasFuncCalls);
  _pass->func()->frame().addAttributes(FuncFrame::kAttrHasFuncCalls);
  _pass->func()->frame().updateCallStackSize(fd.argStackSize());

  return kErrorOk;
}

Error X86RACFGBuilder::onCall(FuncCallNode* call, RAInstBuilder& ib) noexcept {
  uint32_t argCount = call->argCount();
  uint32_t retCount = call->retCount();
  const FuncDetail& fd = call->detail();

  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
      if (!fd.hasArg(argIndex + argHi))
        continue;

      const FuncValue& arg = fd.arg(argIndex + argHi);
      const Operand& op = call->arg(argIndex + argHi);

      if (op.isNone())
        continue;

      if (op.isReg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

        if (arg.isReg()) {
          uint32_t regGroup = workReg->group();
          uint32_t argGroup = Reg::groupOf(arg.regType());

          if (regGroup == argGroup) {
            ASMJIT_PROPAGATE(ib.addCallArg(workReg, arg.regId()));
          }
        }
      }
    }
  }

  for (uint32_t retIndex = 0; retIndex < retCount; retIndex++) {
    const FuncValue& ret = fd.ret(retIndex);
    const Operand& op = call->ret(retIndex);

    // Not handled here...
    if (ret.regType() == Reg::kTypeSt)
      continue;

    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::virtIdToIndex(reg.id()), &workReg));

      if (ret.isReg()) {
        uint32_t regGroup = workReg->group();
        uint32_t retGroup = Reg::groupOf(ret.regType());

        if (regGroup == retGroup) {
          ASMJIT_PROPAGATE(ib.addCallRet(workReg, ret.regId()));
        }
      }
      else {
        return DebugUtils::errored(kErrorInvalidAssignment);
      }
    }
  }

  // Setup clobbered registers.
  ib._clobbered[0] = Support::lsbMask<uint32_t>(_pass->_physRegCount[0]) & ~fd.preservedRegs(0);
  ib._clobbered[1] = Support::lsbMask<uint32_t>(_pass->_physRegCount[1]) & ~fd.preservedRegs(1);
  ib._clobbered[2] = Support::lsbMask<uint32_t>(_pass->_physRegCount[2]) & ~fd.preservedRegs(2);
  ib._clobbered[3] = Support::lsbMask<uint32_t>(_pass->_physRegCount[3]) & ~fd.preservedRegs(3);

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveImmToRegArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToRegArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_, BaseReg* out) noexcept {
  DebugUtils::unused(call);
  ASMJIT_ASSERT(arg.isReg());

  Imm imm(imm_);
  uint32_t rTypeId = Type::kIdU32;

  switch (arg.typeId()) {
    case Type::kIdI8: imm.signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm.zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm.signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm.zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
MovU32:
      imm.zeroExtend32Bits();
      break;

    case Type::kIdI64:
    case Type::kIdU64:
      // Moving to GPD automatically zero extends in 64-bit mode.
      if (imm.isUInt32()) {
        imm.zeroExtend32Bits();
        break;
      }

      rTypeId = Type::kIdU64;
      break;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  ASMJIT_PROPAGATE(cc()->_newReg(*out, rTypeId, nullptr));
  cc()->virtRegById(out->id())->setWeight(RAPass::kCallArgWeight);

  return cc()->mov(out->as<x86::Gp>(), imm);
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveImmToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToStackArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_) noexcept {
  DebugUtils::unused(call);
  ASMJIT_ASSERT(arg.isStack());

  Mem mem = ptr(_pass->_sp.as<Gp>(), arg.stackOffset());
  Imm imm[2];

  mem.setSize(4);
  imm[0] = imm_;
  uint32_t nMovs = 0;

  // One stack entry has the same size as the native register size. That means
  // that if we want to move a 32-bit integer on the stack in 64-bit mode, we
  // need to extend it to a 64-bit integer first. In 32-bit mode, pushing a
  // 64-bit on stack is done in two steps by pushing low and high parts
  // separately.
  switch (arg.typeId()) {
    case Type::kIdI8: imm[0].signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm[0].zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm[0].signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm[0].zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdF32:
MovU32:
      imm[0].zeroExtend32Bits();
      nMovs = 1;
      break;

    case Type::kIdI64:
    case Type::kIdU64:
    case Type::kIdF64:
    case Type::kIdMmx32:
    case Type::kIdMmx64:
      if (_is64Bit && imm[0].isInt32()) {
        mem.setSize(8);
        nMovs = 1;
        break;
      }

      imm[1].setU32(imm[0].u32Hi());
      imm[0].zeroExtend32Bits();
      nMovs = 2;
      break;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  for (uint32_t i = 0; i < nMovs; i++) {
    ASMJIT_PROPAGATE(cc()->mov(mem, imm[i]));
    mem.addOffsetLo32(int32_t(mem.size()));
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveRegToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveRegToStackArg(FuncCallNode* call, const FuncValue& arg, const BaseReg& reg) noexcept {
  DebugUtils::unused(call);
  ASMJIT_ASSERT(arg.isStack());

  Mem mem = ptr(_pass->_sp.as<Gp>(), arg.stackOffset());
  Reg r0, r1;

  VirtReg* vr = cc()->virtRegById(reg.id());
  uint32_t gpSize = cc()->gpSize();
  uint32_t instId = 0;

  uint32_t dstTypeId = arg.typeId();
  uint32_t srcTypeId = vr->typeId();

  switch (dstTypeId) {
    case Type::kIdI64:
    case Type::kIdU64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGp8(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpbLo>(reg.id());

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI8) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGp16(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpw>(reg.id());

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI16) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (GP).
      if (Type::isGp32(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpd>(reg.id());

        instId = Inst::kIdMovsxd;
        if (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI32)
          goto ExtendMovGpXQ;
        else
          goto ZeroExtendGpDQ;
      }

      // Move QWORD (GP).
      if (Type::isGp64(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdI16:
    case Type::kIdU16:
      // DWORD <- WORD (Zero|Sign Extend).
      if (Type::isGp16(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setRegT<Reg::kTypeGpw>(reg.id());
        instId = isDstSigned && isSrcSigned ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpD;
      }

      // DWORD <- BYTE (Zero|Sign Extend).
      if (Type::isGp8(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setRegT<Reg::kTypeGpbLo>(reg.id());
        instId = isDstSigned && isSrcSigned ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto ExtendMovGpD;
      }
      ASMJIT_FALLTHROUGH;

    case Type::kIdI8:
    case Type::kIdU8:
      if (Type::isInt(srcTypeId)) goto MovGpD;
      if (Type::isMmx(srcTypeId)) goto MovMmD;
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdMmx32:
    case Type::kIdMmx64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGp8(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpbLo>(reg.id());

        instId = Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGp16(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpw>(reg.id());

        instId = Inst::kIdMovzx;
        goto ExtendMovGpXQ;
      }

      if (Type::isGp32(srcTypeId)) goto ExtendMovGpDQ;
      if (Type::isGp64(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdF32:
    case Type::kIdF32x1:
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdF64:
    case Type::kIdF64x1:
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    default:
      // TODO: Vector types by stack.
      break;
  }
  return DebugUtils::errored(kErrorInvalidState);

  // Extend+Move Gp.
ExtendMovGpD:
  mem.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(reg.id());

  ASMJIT_PROPAGATE(cc()->emit(instId, r0, r1));
  ASMJIT_PROPAGATE(cc()->emit(Inst::kIdMov, mem, r0));
  return kErrorOk;

ExtendMovGpXQ:
  if (gpSize == 8) {
    mem.setSize(8);
    r0.setRegT<Reg::kTypeGpq>(reg.id());

    ASMJIT_PROPAGATE(cc()->emit(instId, r0, r1));
    ASMJIT_PROPAGATE(cc()->emit(Inst::kIdMov, mem, r0));
  }
  else {
    mem.setSize(4);
    r0.setRegT<Reg::kTypeGpd>(reg.id());

    ASMJIT_PROPAGATE(cc()->emit(instId, r0, r1));

ExtendMovGpDQ:
    ASMJIT_PROPAGATE(cc()->emit(Inst::kIdMov, mem, r0));
    mem.addOffsetLo32(4);
    ASMJIT_PROPAGATE(cc()->emit(Inst::kIdAnd, mem, 0));
  }
  return kErrorOk;

ZeroExtendGpDQ:
  mem.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(reg.id());
  goto ExtendMovGpDQ;

MovGpD:
  mem.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(reg.id());
  return cc()->emit(Inst::kIdMov, mem, r0);

MovGpQ:
  mem.setSize(8);
  r0.setRegT<Reg::kTypeGpq>(reg.id());
  return cc()->emit(Inst::kIdMov, mem, r0);

MovMmD:
  mem.setSize(4);
  r0.setRegT<Reg::kTypeMm>(reg.id());
  return cc()->emit(choose(Inst::kIdMovd, Inst::kIdVmovd), mem, r0);

MovMmQ:
  mem.setSize(8);
  r0.setRegT<Reg::kTypeMm>(reg.id());
  return cc()->emit(choose(Inst::kIdMovq, Inst::kIdVmovq), mem, r0);

MovXmmD:
  mem.setSize(4);
  r0.setRegT<Reg::kTypeXmm>(reg.id());
  return cc()->emit(choose(Inst::kIdMovss, Inst::kIdVmovss), mem, r0);

MovXmmQ:
  mem.setSize(8);
  r0.setRegT<Reg::kTypeXmm>(reg.id());
  return cc()->emit(choose(Inst::kIdMovlps, Inst::kIdVmovlps), mem, r0);
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnReg]
// ============================================================================

Error X86RACFGBuilder::onBeforeRet(FuncRetNode* funcRet) noexcept {
  const FuncDetail& funcDetail = _pass->func()->detail();
  const Operand* opArray = funcRet->operands();
  uint32_t opCount = funcRet->opCount();

  cc()->_setCursor(funcRet->prev());

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = opArray[i];
    const FuncValue& ret = funcDetail.ret(i);

    if (!op.isReg())
      continue;

    if (ret.regType() == Reg::kTypeSt) {
      const Reg& reg = op.as<Reg>();
      uint32_t vIndex = Operand::virtIdToIndex(reg.id());

      if (vIndex < Operand::kVirtIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        if (workReg->group() != Reg::kGroupVec)
          return DebugUtils::errored(kErrorInvalidAssignment);

        Reg src = Reg(workReg->signature(), workReg->virtId());
        Mem mem;

        uint32_t typeId = Type::baseOf(workReg->typeId());
        if (ret.hasTypeId())
          typeId = ret.typeId();

        switch (typeId) {
          case Type::kIdF32:
            ASMJIT_PROPAGATE(_pass->useTemporaryMem(mem, 4, 4));
            mem.setSize(4);
            ASMJIT_PROPAGATE(cc()->emit(choose(Inst::kIdMovss, Inst::kIdVmovss), mem, src.as<Xmm>()));
            ASMJIT_PROPAGATE(cc()->fld(mem));
            break;

          case Type::kIdF64:
            ASMJIT_PROPAGATE(_pass->useTemporaryMem(mem, 8, 4));
            mem.setSize(8);
            ASMJIT_PROPAGATE(cc()->emit(choose(Inst::kIdMovsd, Inst::kIdVmovsd), mem, src.as<Xmm>()));
            ASMJIT_PROPAGATE(cc()->fld(mem));
            break;

          default:
            return DebugUtils::errored(kErrorInvalidAssignment);
        }
      }
    }
  }

  return kErrorOk;
}

Error X86RACFGBuilder::onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept {
  const FuncDetail& funcDetail = _pass->func()->detail();
  const Operand* opArray = funcRet->operands();
  uint32_t opCount = funcRet->opCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = opArray[i];
    if (op.isNone()) continue;

    const FuncValue& ret = funcDetail.ret(i);
    if (ASMJIT_UNLIKELY(!ret.isReg()))
      return DebugUtils::errored(kErrorInvalidAssignment);

    // Not handled here...
    if (ret.regType() == Reg::kTypeSt)
      continue;

    if (op.isReg()) {
      // Register return value.
      const Reg& reg = op.as<Reg>();
      uint32_t vIndex = Operand::virtIdToIndex(reg.id());

      if (vIndex < Operand::kVirtIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->group();
        uint32_t allocable = _pass->_availableRegs[group];
        ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, ret.regId(), 0, BaseReg::kIdBad, 0));
      }
    }
    else {
      return DebugUtils::errored(kErrorInvalidAssignment);
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RAPass - Construction / Destruction]
// ============================================================================

X86RAPass::X86RAPass() noexcept
  : RAPass(),
    _avxEnabled(false) {}
X86RAPass::~X86RAPass() noexcept {}

// ============================================================================
// [asmjit::x86::X86RAPass - OnInit / OnDone]
// ============================================================================

void X86RAPass::onInit() noexcept {
  uint32_t archId = cc()->archId();
  uint32_t baseRegCount = archId == ArchInfo::kIdX86 ? 8u : 16u;

  _archRegsInfo = &opData.archRegs;
  _archTraits[Reg::kGroupGp] |= RAArchTraits::kHasSwap;

  _physRegCount.set(Reg::kGroupGp  , baseRegCount);
  _physRegCount.set(Reg::kGroupVec , baseRegCount);
  _physRegCount.set(Reg::kGroupMm  , 8);
  _physRegCount.set(Reg::kGroupKReg, 8);
  _buildPhysIndex();

  _availableRegCount = _physRegCount;
  _availableRegs[Reg::kGroupGp  ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupGp  ));
  _availableRegs[Reg::kGroupVec ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupVec ));
  _availableRegs[Reg::kGroupMm  ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupMm  ));
  _availableRegs[Reg::kGroupKReg] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupKReg));

  _scratchRegIndexes[0] = uint8_t(Gp::kIdCx);
  _scratchRegIndexes[1] = uint8_t(baseRegCount - 1);

  // The architecture specific setup makes implicitly all registers available. So
  // make unavailable all registers that are special and cannot be used in general.
  bool hasFP = _func->frame().hasPreservedFP();

  makeUnavailable(Reg::kGroupGp, Gp::kIdSp);            // ESP|RSP used as a stack-pointer (SP).
  if (hasFP) makeUnavailable(Reg::kGroupGp, Gp::kIdBp); // EBP|RBP used as a frame-pointer (FP).

  _sp = cc()->zsp();
  _fp = cc()->zbp();
  _avxEnabled = _func->frame().isAvxEnabled();
}

void X86RAPass::onDone() noexcept {}

// ============================================================================
// [asmjit::x86::X86RAPass - BuildCFG]
// ============================================================================

Error X86RAPass::buildCFG() noexcept {
  return X86RACFGBuilder(this).run();
}

// ============================================================================
// [asmjit::x86::X86RAPass - OnEmit]
// ============================================================================

Error X86RAPass::onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseReg dst(wReg->info().signature(), dstPhysId);
  BaseReg src(wReg->info().signature(), srcPhysId);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.assignFormat("<MOVE> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
#endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dst, src, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  RAWorkReg* waReg = workRegById(aWorkId);
  RAWorkReg* wbReg = workRegById(bWorkId);

  bool is64Bit = Support::max(waReg->typeId(), wbReg->typeId()) >= Type::kIdI64;
  uint32_t sign = is64Bit ? uint32_t(RegTraits<Reg::kTypeGpq>::kSignature)
                          : uint32_t(RegTraits<Reg::kTypeGpd>::kSignature);

#ifndef ASMJIT_NO_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.assignFormat("<SWAP> %s, %s", waReg->name(), wbReg->name());
    cc()->setInlineComment(_tmpString.data());
  }
#endif

  return cc()->emit(Inst::kIdXchg, Reg(sign, aPhysId), Reg(sign, bPhysId));
}

Error X86RAPass::onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseReg dstReg(wReg->info().signature(), dstPhysId);
  BaseMem srcMem(workRegAsMem(wReg));

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.assignFormat("<LOAD> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
#endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dstReg, srcMem, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseMem dstMem(workRegAsMem(wReg));
  BaseReg srcReg(wReg->info().signature(), srcPhysId);

  const char* comment = nullptr;

#ifndef ASMJIT_NO_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.assignFormat("<SAVE> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
#endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dstMem, srcReg, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitJump(const Label& label) noexcept {
  return cc()->jmp(label);
}

Error X86RAPass::onEmitPreCall(FuncCallNode* call) noexcept {
  if (call->detail().hasVarArgs()) {
    uint32_t argCount = call->argCount();
    const FuncDetail& fd = call->detail();

    switch (call->detail().callConv().id()) {
      case CallConv::kIdX86SysV64: {
        // AL register contains the number of arguments passed in XMM register(s).
        uint32_t n = 0;
        for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
          for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
            if (!fd.hasArg(argIndex + argHi))
              continue;

            const FuncValue& arg = fd.arg(argIndex + argHi);
            if (arg.isReg() && Reg::groupOf(arg.regType()) == Reg::kGroupVec)
              n++;
          }
        }

        if (!n)
          ASMJIT_PROPAGATE(cc()->xor_(eax, eax));
        else
          ASMJIT_PROPAGATE(cc()->mov(eax, n));
        break;
      }

      case CallConv::kIdX86Win64: {
        // Each double-precision argument passed in XMM must be also passed in GP.
        for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
          for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
            if (!fd.hasArg(argIndex + argHi))
              continue;

            const FuncValue& arg = fd.arg(argIndex + argHi);
            if (arg.isReg() && Reg::groupOf(arg.regType()) == Reg::kGroupVec) {
              Gp dst = gpq(fd.callConv().passedOrder(Reg::kGroupGp)[argIndex]);
              Xmm src = xmm(arg.regId());
              ASMJIT_PROPAGATE(cc()->emit(choose(Inst::kIdMovq, Inst::kIdVmovq), dst, src));
            }
          }
        }
        break;
      }
    }
  }

  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86 && !ASMJIT_NO_COMPILER
