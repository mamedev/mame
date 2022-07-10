// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_COMPILER_H_INCLUDED
#define ASMJIT_TEST_COMPILER_H_INCLUDED

#include <asmjit/core.h>

#include <memory>
#include <vector>

class SimpleErrorHandler : public asmjit::ErrorHandler {
public:
  SimpleErrorHandler()
    : _err(asmjit::kErrorOk) {}

  virtual void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) {
    asmjit::DebugUtils::unused(origin);
    _err = err;
    _message.assign(message);
  }

  asmjit::Error _err;
  asmjit::String _message;
};

//! A test case interface for testing AsmJit's Compiler.
class TestCase {
public:
  TestCase(const char* name = nullptr) {
    if (name)
      _name.assign(name);
  }

  virtual ~TestCase() {}

  inline const char* name() const { return _name.data(); }

  virtual void compile(asmjit::BaseCompiler& cc) = 0;
  virtual bool run(void* func, asmjit::String& result, asmjit::String& expect) = 0;

  asmjit::String _name;
};

class TestApp {
public:
  std::vector<std::unique_ptr<TestCase>> _tests;

  unsigned _nFailed = 0;
  size_t _outputSize = 0;

  bool _verbose = false;
  bool _dumpAsm = false;
  bool _dumpHex = false;

  TestApp() noexcept {}
  ~TestApp() noexcept {}

  void add(TestCase* test) noexcept {
    _tests.push_back(std::unique_ptr<TestCase>(test));
  }

  template<class T>
  inline void addT() { T::add(*this); }

  int handleArgs(int argc, const char* const* argv);
  void showInfo();
  int run();
};

#endif // ASMJIT_TEST_COMPILER_H_INCLUDED
