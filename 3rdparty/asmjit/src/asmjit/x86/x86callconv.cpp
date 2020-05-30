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
#ifdef ASMJIT_BUILD_X86

#include "../x86/x86callconv_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::CallConvInternal - Init]
// ============================================================================

static inline void CallConv_initX86Common(CallConv& cc) noexcept {
  cc.setNaturalStackAlignment(4);
  cc.setArchType(ArchInfo::kIdX86);
  cc.setPreservedRegs(Reg::kGroupGp, Support::bitMask(Gp::kIdBx, Gp::kIdSp, Gp::kIdBp, Gp::kIdSi, Gp::kIdDi));
}

ASMJIT_FAVOR_SIZE Error CallConvInternal::init(CallConv& cc, uint32_t ccId) noexcept {
  constexpr uint32_t kGroupGp   = Reg::kGroupGp;
  constexpr uint32_t kGroupVec  = Reg::kGroupVec;
  constexpr uint32_t kGroupMm   = Reg::kGroupMm;
  constexpr uint32_t kGroupKReg = Reg::kGroupKReg;

  constexpr uint32_t kZax = Gp::kIdAx;
  constexpr uint32_t kZbx = Gp::kIdBx;
  constexpr uint32_t kZcx = Gp::kIdCx;
  constexpr uint32_t kZdx = Gp::kIdDx;
  constexpr uint32_t kZsp = Gp::kIdSp;
  constexpr uint32_t kZbp = Gp::kIdBp;
  constexpr uint32_t kZsi = Gp::kIdSi;
  constexpr uint32_t kZdi = Gp::kIdDi;

  switch (ccId) {
    case CallConv::kIdX86StdCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86MsThisCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86MsFastCall:
    case CallConv::kIdX86GccFastCall:
      cc.setFlags(CallConv::kFlagCalleePopsStack);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86GccRegParm1:
      cc.setPassedOrder(kGroupGp, kZax);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86GccRegParm2:
      cc.setPassedOrder(kGroupGp, kZax, kZdx);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86GccRegParm3:
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx);
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86CDecl:
      CallConv_initX86Common(cc);
      break;

    case CallConv::kIdX86Win64:
      cc.setArchType(ArchInfo::kIdX64);
      cc.setStrategy(CallConv::kStrategyWin64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec | CallConv::kFlagIndirectVecArgs);
      cc.setNaturalStackAlignment(16);
      cc.setSpillZoneSize(32);
      cc.setPassedOrder(kGroupGp, kZcx, kZdx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3);
      cc.setPreservedRegs(kGroupGp, Support::bitMask(kZbx, kZsp, kZbp, kZsi, kZdi, 12, 13, 14, 15));
      cc.setPreservedRegs(kGroupVec, Support::bitMask(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86SysV64:
      cc.setArchType(ArchInfo::kIdX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setRedZoneSize(128);
      cc.setPassedOrder(kGroupGp, kZdi, kZsi, kZdx, kZcx, 8, 9);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPreservedRegs(kGroupGp, Support::bitMask(kZbx, kZsp, kZbp, 12, 13, 14, 15));
      break;

    case CallConv::kIdX86LightCall2:
    case CallConv::kIdX86LightCall3:
    case CallConv::kIdX86LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX86LightCall2) + 2;

      cc.setArchType(ArchInfo::kIdX86);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupKReg, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp  , Support::lsbMask<uint32_t>(8));
      cc.setPreservedRegs(kGroupVec , Support::lsbMask<uint32_t>(8) & ~Support::lsbMask<uint32_t>(n));
      break;
    }

    case CallConv::kIdX64LightCall2:
    case CallConv::kIdX64LightCall3:
    case CallConv::kIdX64LightCall4: {
      uint32_t n = (ccId - CallConv::kIdX64LightCall2) + 2;

      cc.setArchType(ArchInfo::kIdX64);
      cc.setFlags(CallConv::kFlagPassFloatsByVec);
      cc.setNaturalStackAlignment(16);
      cc.setPassedOrder(kGroupGp, kZax, kZdx, kZcx, kZsi, kZdi);
      cc.setPassedOrder(kGroupMm, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupVec, 0, 1, 2, 3, 4, 5, 6, 7);
      cc.setPassedOrder(kGroupKReg, 0, 1, 2, 3, 4, 5, 6, 7);

      cc.setPreservedRegs(kGroupGp  , Support::lsbMask<uint32_t>(16));
      cc.setPreservedRegs(kGroupVec ,~Support::lsbMask<uint32_t>(n));
      break;
    }

    default:
      return DebugUtils::errored(kErrorInvalidArgument);
  }

  cc.setId(ccId);
  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_BUILD_X86
