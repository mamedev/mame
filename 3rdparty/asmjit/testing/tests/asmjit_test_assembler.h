// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_ASSEMBLER_H_INCLUDED
#define ASMJIT_TEST_ASSEMBLER_H_INCLUDED

#include <asmjit/core.h>
#include <stdio.h>

struct TestSettings {
  bool verbose;
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

  void print_header(const char* arch_name) noexcept {
    printf("%s assembler tests:\n", arch_name);
  }

  void print_summary() noexcept {
    printf("  Passed: %zu / %zu tests\n\n", passed, count);
  }

  bool did_pass() const noexcept { return passed == count; }

  void prepare() noexcept {
    code.reset();
    code.init(env, 0);
    code.attach(&assembler);
    L0 = assembler.new_label();

    if (settings.validate)
      assembler.add_diagnostic_options(asmjit::DiagnosticOptions::kValidateAssembler);
  }

  ASMJIT_NOINLINE bool test_valid_instruction(const char* s, const char* expected_opcode, asmjit::Error err = asmjit::Error::kOk) noexcept {
    count++;

    if (err != asmjit::Error::kOk) {
      printf("  !! %s\n"
             "    <%s>\n", s, asmjit::DebugUtils::error_as_string(err));
      prepare();
      return false;
    }

    asmjit::String encoded_opcode;
    asmjit::Section* text = code.text_section();

    encoded_opcode.append_hex(text->data(), text->buffer_size());
    if (encoded_opcode != expected_opcode) {
      printf("  !! [%s] <- %s\n"
             "     [%s] (Expected)\n", encoded_opcode.data(), s, expected_opcode);
      prepare();
      return false;
    }

    if (settings.verbose)
      printf("  OK [%s] <- %s\n", encoded_opcode.data(), s);

    passed++;
    prepare();
    return true;
  }

  ASMJIT_NOINLINE bool test_invalid_instruction(const char* s, asmjit::Error expected_error, asmjit::Error err) noexcept {
    count++;

    if (err == asmjit::Error::kOk) {
      printf("  !! %s passed, but should have failed with <%s> error\n", s, asmjit::DebugUtils::error_as_string(expected_error));
      prepare();
      return false;
    }

    if (err != asmjit::Error::kOk) {
      printf("  !! %s failed with <%s>, but should have failed with <%s>\n", s, asmjit::DebugUtils::error_as_string(err), asmjit::DebugUtils::error_as_string(expected_error));
      prepare();
      return false;
    }

    if (settings.verbose)
      printf("  OK [%s] <- %s\n", asmjit::DebugUtils::error_as_string(err), s);

    passed++;
    prepare();
    return true;
  }
};

#endif // ASMJIT_TEST_ASSEMBLER_H_INCLUDED
