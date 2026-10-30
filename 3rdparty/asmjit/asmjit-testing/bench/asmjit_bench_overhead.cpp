#include <asmjit/host.h>

#include <asmjit-testing/commons/asmjitutils.h>
#include <asmjit-testing/commons/cmdline.h>
#include <asmjit-testing/commons/performancetimer.h>

#include <stdint.h>

using namespace asmjit;

static void print_app_info(size_t n) noexcept {
  printf("AsmJit Benchmark Overhead v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("This benchmark was designed to benchmark the cost of initialization and\n"
         "reset (or reinitialization) of CodeHolder and Emitters; and the cost of\n"
         "moving a minimal assembled function to executable memory. Each output line\n"
         "provides the following columns:\n"
         "\n"
         "  - <Test>     - test case name - either 'CodeHolder' only or an emitter\n"
         "  - Strategy   - reusability strategy - whether init/reset or reinit is used\n"
         "  - Reuse Only - no code generation, no use of emitter except for init/reuse\n"
         "  - Func       - function was assembled\n"
         "  - RA         - function was compiled (registers allocated) (Compiler)\n"
         "  - Asm        - function was finalized & serialized (Builder/Compiler)\n"
         "  - RT         - function was added to JitRuntime and then removed from it\n"
         "\n"
         "Essentially the output provides an insight into the cost of reusing\n"
         "CodeHolder and other emitters, and the cost of assembling, finalizing,\n"
         "and moving the assembled code into executable memory by separating each\n"
         "phase.\n\n"
  );

  printf("The number of iterations benchmarked: %zu (override by --count=n)\n", n);
  printf("\n");
}

enum class AssemblerOp {
  kNone,
  kRT
};

enum class BuilderOp {
  kNone,
  kFinalize,
  kFinalize_RT
};

enum class CompilerOp {
  kNone,
  kCompile,
  kFinalize,
  kFinalize_RT
};

#if !defined(ASMJIT_NO_JIT)

class MyErrorHandler : public ErrorHandler {
public:
  void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    Support::maybe_unused(err, origin);
    fprintf(stderr, "AsmJit error: %s\n", message);
  }
};

enum class InitStrategy : uint32_t {
  kInitReset,
  kReinit
};

static inline void bench_codeholder(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    for (size_t i = 0; i < count; i++) {
      code.reinit();
    }
  }
}

#if ASMJIT_ARCH_X86 != 0 && !defined(ASMJIT_NO_X86)
template<typename EmitterT>
static ASMJIT_INLINE void emit_raw_func(EmitterT& emitter) {
  emitter.mov(x86::eax, 0);
  emitter.ret();
}

template<typename CompilerT>
static ASMJIT_INLINE void compile_raw_func(CompilerT& cc) {
  x86::Gp r = cc.new_gp32();
  cc.mov(r, 0);
  cc.ret(r);
}
#endif

#if ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)
template<typename EmitterT>
static ASMJIT_INLINE void emit_raw_func(EmitterT& emitter) {
  emitter.mov(a64::w0, 0);
  emitter.ret(a64::x30);
}

template<typename CompilerT>
static ASMJIT_INLINE void compile_raw_func(CompilerT& cc) {
  a64::Gp gp = cc.new_gp32();
  cc.mov(gp, 0);
  cc.ret(gp);
}
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND)
template<typename AssemblerT>
static inline void bench_assembler(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  AssemblerT a;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&a);
      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&a);

    for (size_t i = 0; i < count; i++) {
      code.reinit();
    }
  }
}

template<typename AssemblerT, AssemblerOp op>
static inline void bench_assembler_func(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  AssemblerT a;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&a);
      emit_raw_func(a);

      if constexpr (op == AssemblerOp::kRT) {
        using Func = uint32_t(*)(void);
        Func fn;
        rt.add(&fn, &code);
        rt.release(fn);
      }

      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&a);

    for (size_t i = 0; i < count; i++) {
      code.reinit();
      emit_raw_func(a);

      if constexpr (op == AssemblerOp::kRT) {
        using Func = uint32_t(*)(void);
        Func fn;
        rt.add(&fn, &code);
        rt.release(fn);
      }
    }
  }
}
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_BUILDER)
template<typename BuilderT>
static inline void bench_builder(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  BuilderT b;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&b);
      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&b);

    for (size_t i = 0; i < count; i++) {
      code.reinit();
    }
  }
}

template<typename BuilderT, BuilderOp op>
static inline void bench_builder_func(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  BuilderT b;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&b);
      emit_raw_func(b);

      if constexpr (op >= BuilderOp::kFinalize) {
        b.finalize();
        if constexpr (op == BuilderOp::kFinalize_RT) {
          using Func = uint32_t(*)(void);
          Func fn;
          rt.add(&fn, &code);
          rt.release(fn);
        }
      }

      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&b);

    for (size_t i = 0; i < count; i++) {
      code.reinit();
      emit_raw_func(b);

      if constexpr (op >= BuilderOp::kFinalize) {
        b.finalize();
        if constexpr (op == BuilderOp::kFinalize_RT) {
          using Func = uint32_t(*)(void);
          Func fn;
          rt.add(&fn, &code);
          rt.release(fn);
        }
      }
    }
  }
}
#endif // ASMJIT_HAS_HOST_BACKEND && !ASMJIT_NO_BUILDER

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_COMPILER)
template<typename CompilerT>
static inline void bench_compiler(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  CompilerT cc;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&cc);
      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&cc);

    for (size_t i = 0; i < count; i++) {
      code.reinit();
    }
  }
}

template<typename CompilerT, CompilerOp op>
static inline void bench_compiler_func(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  CompilerT cc;
  MyErrorHandler eh;

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&cc);

      (void)cc.add_func(FuncSignature::build<uint32_t>());
      compile_raw_func(cc);
      cc.end_func();

      if constexpr (op == CompilerOp::kCompile) {
        cc.run_passes();
      }
      else if constexpr (op >= CompilerOp::kFinalize) {
        cc.finalize();
        if constexpr (op == CompilerOp::kFinalize_RT) {
          using Func = uint32_t(*)(void);
          Func fn;
          rt.add(&fn, &code);
          rt.release(fn);
        }
      }

      code.reset();
    }
  }
  else {
    code.init(rt.environment());
    code.set_error_handler(&eh);
    code.attach(&cc);

    for (size_t i = 0; i < count; i++) {
      code.reinit();

      (void)cc.add_func(FuncSignature::build<uint32_t>());
      compile_raw_func(cc);
      cc.end_func();

      if constexpr (op == CompilerOp::kCompile) {
        cc.run_passes();
      }
      else if constexpr (op >= CompilerOp::kFinalize) {
        cc.finalize();
        if constexpr (op == CompilerOp::kFinalize_RT) {
          using Func = uint32_t(*)(void);
          Func fn;
          rt.add(&fn, &code);
          rt.release(fn);
        }
      }
    }
  }
}
#endif // ASMJIT_HAS_HOST_BACKEND && !ASMJIT_NO_COMPILER

template<typename Lambda>
static inline void test_perf(const char* group, const char* params, InitStrategy strategy, size_t n, Lambda&& fn) {
  PerformanceTimer timer;
  const char* strategy_name = strategy == InitStrategy::kInitReset ? "init/reset" : "reinit";

  timer.start();
  fn(strategy, n);
  timer.stop();

  printf("| %-10s | %-10s | %-23s| %8.1f [ms] |\n", group, strategy_name, params, timer.duration());
}

static inline void test_perf_all(InitStrategy strategy, size_t n) {
  using IS = InitStrategy;

  const char frame[]  = "+------------+------------+------------------------+---------------+\n";
  const char header[] = "| Group      | Strategy   | Features Used          |     Time [ms] |\n";

  printf(frame);
  printf(header);
  printf(frame);

  test_perf("CodeHolder", "Reuse Only"          , strategy, n, [](IS s, size_t n) { bench_codeholder(s, n); });

#if defined(ASMJIT_HAS_HOST_BACKEND)
  test_perf("Assembler ", "Reuse Only"          , strategy, n, [](IS s, size_t n) { bench_assembler<host::Assembler>(s, n); });
  test_perf("Assembler ", "Func"                , strategy, n, [](IS s, size_t n) { bench_assembler_func<host::Assembler, AssemblerOp::kNone>(s, n); });
  test_perf("Assembler ", "Func + RT"           , strategy, n, [](IS s, size_t n) { bench_assembler_func<host::Assembler, AssemblerOp::kRT>(s, n); });
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_BUILDER)
  test_perf("Builder   ", "Reuse Only"          , strategy, n, [](IS s, size_t n) { bench_builder<host::Builder>(s, n); });
  test_perf("Builder   ", "Func"                , strategy, n, [](IS s, size_t n) { bench_builder_func<host::Builder, BuilderOp::kNone>(s, n); });
  test_perf("Builder   ", "Func + Asm"          , strategy, n, [](IS s, size_t n) { bench_builder_func<host::Builder, BuilderOp::kFinalize>(s, n); });
  test_perf("Builder   ", "Func + Asm + RT"     , strategy, n, [](IS s, size_t n) { bench_builder_func<host::Builder, BuilderOp::kFinalize_RT>(s, n); });
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_COMPILER)

  test_perf("Compiler  ", "Reuse Only"          , strategy, n, [](IS s, size_t n) { bench_compiler<host::Compiler>(s, n); });
  test_perf("Compiler  ", "Func"                , strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler, CompilerOp::kNone>(s, n); });
  test_perf("Compiler  ", "Func + RA"           , strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler, CompilerOp::kCompile>(s, n); });
  test_perf("Compiler  ", "Func + RA + Asm"     , strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler, CompilerOp::kFinalize>(s, n); });
  test_perf("Compiler  ", "Func + RA + Asm + RT", strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler, CompilerOp::kFinalize_RT>(s, n); });
#endif

  printf(frame);
}

int main(int argc, char* argv[]) {
  CmdLine cmd_line(argc, argv);
  size_t n = cmd_line.value_as_uint("--count", 1000000);

  print_app_info(n);

  test_perf_all(InitStrategy::kInitReset, n);
  printf("\n");
  test_perf_all(InitStrategy::kReinit, n);

  return 0;
}

#else

int main() {
  print_app_info(0);
  printf("!!AsmJit Benchmark Reuse is currently disabled: <ASMJIT_NO_JIT> or unsuitable target architecture !!\n");
  return 0;
}

#endif
