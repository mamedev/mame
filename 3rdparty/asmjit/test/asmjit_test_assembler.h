// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_ASSEMBLER_H_INCLUDED
#define ASMJIT_TEST_ASSEMBLER_H_INCLUDED

#include <asmjit/core.h>
#include <stdio.h>

struct TestSettings {
  bool quiet;
  bool validate;
};

template<typename AssemblerType>
class AssemblerTester {
public:
  asmjit::Environment env {};
  asmjit::CodeHolder code {};
  AssemblerType assembler {};
  asmjit::Label L0 {};
  const TestSettings& settings;

  size_t passed {};
  size_t count {};

  AssemblerTester(asmjit::Arch arch, const TestSettings& settings) noexcept
    : env(arch),
      settings(settings) {
    prepare();
  }

  void printHeader(const char* archName) noexcept {
    printf("%s assembler tests:\n", archName);
  }

  void printSummary() noexcept {
    printf("  Passed: %zu / %zu tests\n\n", passed, count);
  }

  bool didPass() const noexcept { return passed == count; }

  void prepare() noexcept {
    code.reset();
    code.init(env, 0);
    code.attach(&assembler);
    L0 = assembler.newLabel();

    if (settings.validate)
      assembler.addDiagnosticOptions(asmjit::DiagnosticOptions::kValidateAssembler);
  }

  ASMJIT_NOINLINE bool testInstruction(const char* expectedOpcode, const char* s, uint32_t err) noexcept {
    count++;

    if (err) {
      printf("  !! %s\n"
             "    <%s>\n", s, asmjit::DebugUtils::errorAsString(err));
      prepare();
      return false;
    }

    asmjit::String encodedOpcode;
    asmjit::Section* text = code.textSection();

    encodedOpcode.appendHex(text->data(), text->bufferSize());
    if (encodedOpcode != expectedOpcode) {
      printf("  !! [%s] <- %s\n"
             "     [%s] (Expected)\n", encodedOpcode.data(), s, expectedOpcode);
      prepare();
      return false;
    }

    if (!settings.quiet)
      printf("  OK [%s] <- %s\n", encodedOpcode.data(), s);

    passed++;
    prepare();
    return true;
  }
};

#endif // ASMJIT_TEST_ASSEMBLER_H_INCLUDED
