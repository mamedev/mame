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

#include <asmjit/x86.h>
#include <stdio.h>

using namespace asmjit;

static void printInfo(uint32_t arch, const BaseInst& inst, const Operand_* operands, size_t opCount) {
  StringTmp<512> sb;

  // Read & Write Information
  // ------------------------

  InstRWInfo rw;
  InstAPI::queryRWInfo(arch, inst, operands, opCount, &rw);

  sb.append("Instruction:\n");
  sb.append("  ");
#ifndef ASMJIT_NO_LOGGING
  Formatter::formatInstruction(sb, 0, nullptr, arch, inst, operands, opCount);
#else
  sb.append("<Logging-Not-Available>");
#endif
  sb.append("\n");

  sb.append("Operands:\n");
  for (uint32_t i = 0; i < rw.opCount(); i++) {
    const OpRWInfo& op = rw.operand(i);
    const char* access = op.isReadOnly() ? "R" :
                         op.isWriteOnly() ? "W" :
                         op.isReadWrite() ? "X" : "_";

    sb.appendFormat("  [%u] RW=%s Read=%016llX Write=%016llX Extend=%016llX",
                    i, access, op.readByteMask(), op.writeByteMask(), op.extendByteMask());
    sb.append("\n");
  }

  // CPU Features
  // ------------

  BaseFeatures features;
  InstAPI::queryFeatures(arch, inst, operands, opCount, &features);

#ifndef ASMJIT_NO_LOGGING
  if (!features.empty()) {
    sb.append("Features:\n");
    sb.append("  ");

    bool first = true;
    BaseFeatures::Iterator it(features.iterator());
    while (it.hasNext()) {
      uint32_t featureId = uint32_t(it.next());
      if (!first)
        sb.append(" & ");
      Formatter::formatFeature(sb, arch, featureId);
      first = false;
    }
    sb.append("\n");
  }
#endif

  printf("%s\n", sb.data());
}

template<typename... Args>
static void printInfoSimple(uint32_t arch, uint32_t instId, Args&&... args) {
  BaseInst inst(instId);
  Operand_ opArray[] = { std::forward<Args>(args)... };
  printInfo(arch, inst, opArray, sizeof...(args));
}

int main() {
  using namespace x86;
  uint32_t arch = Environment::kArchX64;

  printf("AsmJit X86 Instruction Information Test\n\n");

  printInfoSimple(arch,
                  x86::Inst::kIdAdd,
                  x86::eax, x86::ebx);

  printInfoSimple(arch,
                  x86::Inst::kIdPshufd,
                  x86::xmm0, x86::xmm1, imm(0));

  printInfoSimple(arch,
                  x86::Inst::kIdPextrw,
                  x86::eax, x86::xmm1, imm(0));

  printInfoSimple(arch,
                  x86::Inst::kIdPextrw,
                  x86::ptr(rax), x86::xmm1, imm(0));

  printInfoSimple(arch,
                  x86::Inst::kIdVaddpd,
                  x86::ymm0, x86::ymm1, x86::ymm2);

  printInfoSimple(arch,
                  x86::Inst::kIdVaddpd,
                  x86::ymm0, x86::ymm30, x86::ymm31);

  printInfoSimple(arch,
                  x86::Inst::kIdVaddpd,
                  x86::zmm0, x86::zmm1, x86::zmm2);

  return 0;
}
