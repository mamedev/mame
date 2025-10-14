// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
#endif

#include <stdio.h>

#include "../commons/asmjitutils.h"

using namespace asmjit;

static void print_app_info() noexcept {
  printf("AsmJit Instruction Info Test Suite v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

namespace {

#if !defined(ASMJIT_NO_X86)
static char access_letter(bool r, bool w) noexcept {
  return r && w ? 'X' : r ? 'R' : w ? 'W' : '_';
}

static void print_info(Arch arch, const BaseInst& inst, const Operand_* operands, size_t op_count) {
  StringTmp<512> sb;

  // Read & Write Information
  // ------------------------

  InstRWInfo rw;
  InstAPI::query_rw_info(arch, inst, operands, op_count, &rw);

#ifndef ASMJIT_NO_LOGGING
  Formatter::format_instruction(sb, FormatFlags::kNone, nullptr, arch, inst, Span(operands, op_count));
#else
  sb.append("<Logging-Not-Available>");
#endif
  sb.append("\n");

  sb.append("  Operands:\n");
  for (uint32_t i = 0; i < rw.op_count(); i++) {
    const OpRWInfo& op = rw.operand(i);

    sb.append_format("    [%u] Op=%c Read=%016llX Write=%016llX Extend=%016llX",
                    i,
                    access_letter(op.is_read(), op.is_write()),
                    op.read_byte_mask(),
                    op.write_byte_mask(),
                    op.extend_byte_mask());

    if (op.is_mem_base_used()) {
      sb.append_format(" Base=%c", access_letter(op.is_mem_base_read(), op.is_mem_base_write()));
      if (op.is_mem_base_pre_modify())
        sb.append_format(" <PRE>");
      if (op.is_mem_base_post_modify())
        sb.append_format(" <POST>");
    }

    if (op.is_mem_index_used()) {
      sb.append_format(" Index=%c", access_letter(op.is_mem_index_read(), op.is_mem_index_write()));
    }

    sb.append("\n");
  }

  // CPU Flags (Read/Write)
  // ----------------------

  if ((rw.read_flags() | rw.write_flags()) != CpuRWFlags::kNone) {
    sb.append("  Flags: \n");

    struct FlagMap {
      CpuRWFlags flag;
      char name[4];
    };

    static const FlagMap flag_map_table[] = {
      { CpuRWFlags::kX86_CF, "CF" },
      { CpuRWFlags::kX86_OF, "OF" },
      { CpuRWFlags::kX86_SF, "SF" },
      { CpuRWFlags::kX86_ZF, "ZF" },
      { CpuRWFlags::kX86_AF, "AF" },
      { CpuRWFlags::kX86_PF, "PF" },
      { CpuRWFlags::kX86_DF, "DF" },
      { CpuRWFlags::kX86_IF, "IF" },
      { CpuRWFlags::kX86_AC, "AC" },
      { CpuRWFlags::kX86_C0, "C0" },
      { CpuRWFlags::kX86_C1, "C1" },
      { CpuRWFlags::kX86_C2, "C2" },
      { CpuRWFlags::kX86_C3, "C3" }
    };

    sb.append("    ");
    for (uint32_t f = 0; f < 13; f++) {
      char c = access_letter((rw.read_flags() & flag_map_table[f].flag) != CpuRWFlags::kNone,
                            (rw.write_flags() & flag_map_table[f].flag) != CpuRWFlags::kNone);
      if (c != '_')
        sb.append_format("%s=%c ", flag_map_table[f].name, c);
    }

    sb.append("\n");
  }

  // CPU Features
  // ------------

  CpuFeatures features;
  InstAPI::query_features(arch, inst, operands, op_count, &features);

#ifndef ASMJIT_NO_LOGGING
  if (!features.is_empty()) {
    sb.append("  Features:\n");
    sb.append("    ");

    bool first = true;
    CpuFeatures::Iterator it(features.iterator());
    while (it.has_next()) {
      uint32_t feature_id = uint32_t(it.next());
      if (!first)
        sb.append(" & ");
      Formatter::format_feature(sb, arch, feature_id);
      first = false;
    }
    sb.append("\n");
  }
#endif

  printf("%s\n", sb.data());
}

template<typename... Args>
static void print_info_simple(Arch arch,InstId inst_id, InstOptions options, Args&&... args) {
  BaseInst inst(inst_id);
  inst.add_options(options);
  Operand_ op_array[] = { std::forward<Args>(args)... };
  print_info(arch, inst, op_array, sizeof...(args));
}

template<typename... Args>
static void print_info_extra(Arch arch, InstId inst_id, InstOptions options, const Reg& extra_reg, Args&&... args) {
  BaseInst inst(inst_id);
  inst.add_options(options);
  inst.set_extra_reg(extra_reg);
  Operand_ op_array[] = { std::forward<Args>(args)... };
  print_info(arch, inst, op_array, sizeof...(args));
}
#endif // !ASMJIT_NO_X86

static void test_x86_arch() {
#if !defined(ASMJIT_NO_X86)
  using namespace x86;
  Arch arch = Arch::kX64;

  print_info_simple(arch, Inst::kIdAdd, InstOptions::kNone, eax, ebx);
  print_info_simple(arch, Inst::kIdXor, InstOptions::kNone, eax, eax);
  print_info_simple(arch, Inst::kIdLods, InstOptions::kNone, eax, dword_ptr(rsi));

  print_info_simple(arch, Inst::kIdPshufd, InstOptions::kNone, xmm0, xmm1, imm(0));
  print_info_simple(arch, Inst::kIdPabsb, InstOptions::kNone, mm1, mm2);
  print_info_simple(arch, Inst::kIdPabsb, InstOptions::kNone, xmm1, xmm2);
  print_info_simple(arch, Inst::kIdPextrw, InstOptions::kNone, eax, mm1, imm(0));
  print_info_simple(arch, Inst::kIdPextrw, InstOptions::kNone, eax, xmm1, imm(0));
  print_info_simple(arch, Inst::kIdPextrw, InstOptions::kNone, ptr(rax), xmm1, imm(0));

  print_info_simple(arch, Inst::kIdVpdpbusd, InstOptions::kNone, xmm0, xmm1, xmm2);
  print_info_simple(arch, Inst::kIdVpdpbusd, InstOptions::kX86_Vex, xmm0, xmm1, xmm2);

  print_info_simple(arch, Inst::kIdVaddpd, InstOptions::kNone, ymm0, ymm1, ymm2);
  print_info_simple(arch, Inst::kIdVaddpd, InstOptions::kNone, ymm0, ymm30, ymm31);
  print_info_simple(arch, Inst::kIdVaddpd, InstOptions::kNone, zmm0, zmm1, zmm2);

  print_info_simple(arch, Inst::kIdVpternlogd, InstOptions::kNone, zmm0, zmm0, zmm0, imm(0xFF));
  print_info_simple(arch, Inst::kIdVpternlogq, InstOptions::kNone, zmm0, zmm1, zmm2, imm(0x33));

  print_info_extra(arch, Inst::kIdVaddpd, InstOptions::kNone, k1, zmm0, zmm1, zmm2);
  print_info_extra(arch, Inst::kIdVaddpd, InstOptions::kX86_ZMask, k1, zmm0, zmm1, zmm2);

  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, xmm0, xmm1);
  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, ymm0, xmm1);
  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, zmm0, ymm1);

  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, xmm0, ptr(rsi));
  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, ymm0, ptr(rsi));
  print_info_simple(arch, Inst::kIdVcvtdq2pd, InstOptions::kNone, zmm0, ptr(rsi));
#endif // !ASMJIT_NO_X86
}

} // {anonymous}

int main() {
  print_app_info();
  test_x86_arch();

  return 0;
}
