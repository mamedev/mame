#include <stdint.h>
#include <asmjit/host.h>

#include "../commons/asmjitutils.h"
#include "../commons/cmdline.h"
#include "../commons/performancetimer.h"

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
         "uses \"<Test> [Func] [Finalize] [RT]\" format, with the following meaning:\n"
         "\n"
         "  - <Test>     - test case name - either 'CodeHolder' only or an emitter\n"
         "  - [Func]     - function was assembled\n"
         "  - [Finalize] - function was finalized (Builder/Compiler)\n"
         "  - [RT]       - function was added to JitRuntime and then removed from it\n"
         "\n"
         "Essentially the output provides an insight into the cost of reusing\n"
         "CodeHolder and other emitters, and the cost of assembling, finalizing,\n"
         "and moving the assembled code into executable memory by separating each\n"
         "phase.\n\n"
  );

  printf("The number of iterations benchmarked: %zu (override by --count=n)\n", n);
  printf("\n");
}

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

template<typename AssemblerT>
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
    }
  }
}

template<typename AssemblerT>
static inline void bench_assembler_func_rt(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  AssemblerT a;
  MyErrorHandler eh;

  using Func = uint32_t(*)(void);

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&a);
      emit_raw_func(a);

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);

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

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);
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

template<typename BuilderT>
static inline void bench_builder_func(InitStrategy strategy, size_t count, bool finalize) {
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

      if (finalize) {
        b.finalize();
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

      if (finalize) {
        b.finalize();
      }
    }
  }
}

template<typename BuilderT>
static inline void bench_builder_func_finalize_rt(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  BuilderT b;
  MyErrorHandler eh;

  using Func = uint32_t(*)(void);

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&b);
      emit_raw_func(b);
      b.finalize();

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);

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
      b.finalize();

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);
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

template<typename CompilerT>
static inline void bench_compiler_func(InitStrategy strategy, size_t count, bool finalize) {
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

      if (finalize) {
        cc.finalize();
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

      if (finalize) {
        cc.finalize();
      }
    }
  }
}

template<typename CompilerT>
static inline void bench_compiler_func_rt(InitStrategy strategy, size_t count) {
  JitRuntime rt;
  CodeHolder code;
  CompilerT cc;
  MyErrorHandler eh;

  using Func = uint32_t(*)(void);

  if (strategy == InitStrategy::kInitReset) {
    for (size_t i = 0; i < count; i++) {
      code.init(rt.environment());
      code.set_error_handler(&eh);
      code.attach(&cc);

      (void)cc.add_func(FuncSignature::build<uint32_t>());
      compile_raw_func(cc);
      cc.end_func();
      cc.finalize();

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);

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
      cc.finalize();

      Func fn;
      rt.add(&fn, &code);
      rt.release(fn);
    }
  }
}
#endif // ASMJIT_HAS_HOST_BACKEND && !ASMJIT_NO_COMPILER

template<typename Lambda>
static inline void test_perf(const char* bench_name, InitStrategy strategy, size_t n, Lambda&& fn) {
  PerformanceTimer timer;
  const char* strategy_name = strategy == InitStrategy::kInitReset ? "init/reset" : "reinit    ";

  timer.start();
  fn(strategy, n);
  timer.stop();

  printf("%-31s [%s]: %8.3f [ms]\n", bench_name, strategy_name, timer.duration());
}

static inline void test_perf_all(InitStrategy strategy, size_t n) {
  using IS = InitStrategy;

  test_perf("CodeHolder (Only)"              , strategy, n, [](IS s, size_t n) { bench_codeholder(s, n); });

#if defined(ASMJIT_HAS_HOST_BACKEND)
  test_perf("Assembler"                      , strategy, n, [](IS s, size_t n) { bench_assembler<host::Assembler>(s, n); });
  test_perf("Assembler + Func"               , strategy, n, [](IS s, size_t n) { bench_assembler_func<host::Assembler>(s, n); });
  test_perf("Assembler + Func + RT"          , strategy, n, [](IS s, size_t n) { bench_assembler_func_rt<host::Assembler>(s, n); });
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_BUILDER)
  test_perf("Builder"                        , strategy, n, [](IS s, size_t n) { bench_builder<host::Builder>(s, n); });
  test_perf("Builder + Func"                 , strategy, n, [](IS s, size_t n) { bench_builder_func<host::Builder>(s, n, false); });
  test_perf("Builder + Func + Finalize"      , strategy, n, [](IS s, size_t n) { bench_builder_func<host::Builder>(s, n, true); });
  test_perf("Builder + Func + Finalize + RT" , strategy, n, [](IS s, size_t n) { bench_builder_func_finalize_rt<host::Builder>(s, n); });
#endif

#if defined(ASMJIT_HAS_HOST_BACKEND) && !defined(ASMJIT_NO_COMPILER)

  test_perf("Compiler"                       , strategy, n, [](IS s, size_t n) { bench_compiler<host::Compiler>(s, n); });
  test_perf("Compiler + Func"                , strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler>(s, n, false); });
  test_perf("Compiler + Func + Finalize"     , strategy, n, [](IS s, size_t n) { bench_compiler_func<host::Compiler>(s, n, true); });

  test_perf("Compiler + Func + Finalize + RT", strategy, n, [](IS s, size_t n) { bench_compiler_func_rt<host::Compiler>(s, n); });
#endif
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
