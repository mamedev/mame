// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/ujit.h>

#if defined(ASMJIT_UJIT_X86)

#if defined(_MSC_VER)
  #include <intrin.h>
#else
  #include <immintrin.h>
#endif

namespace UniCompilerTests {

// A reference implementation of MUL+ADD with the use of FMA. This has to be provided otherwise the
// compiler may use FPU registers in 32-bit x86 case, which would make the result different than when
// compiled by JIT compiler that would use XMM registers (32/64-bit SSE/AVX operations).

float fmadd_fma_ref(float a, float b, float c) noexcept {
  __m128 av = _mm_set1_ps(a);
  __m128 bv = _mm_set1_ps(b);
  __m128 cv = _mm_set1_ps(c);

  return _mm_cvtss_f32(_mm_fmadd_ss(av, bv, cv));
}

double fmadd_fma_ref(double a, double b, double c) noexcept {
  __m128d av = _mm_set1_pd(a);
  __m128d bv = _mm_set1_pd(b);
  __m128d cv = _mm_set1_pd(c);

  return _mm_cvtsd_f64(_mm_fmadd_sd(av, bv, cv));
}

void madd_fma_check_valgrind_bug(const float a[4], const float b[4], const float c[4], float dst[4]) noexcept {
  __m128 av = _mm_loadu_ps(a);
  __m128 bv = _mm_loadu_ps(b);
  __m128 cv = _mm_loadu_ps(c);

  __m128 dv = _mm_fmadd_ss(av, bv, cv);
  _mm_storeu_ps(dst, dv);
}

} // {UniCompilerTests}

#endif // ASMJIT_UJIT_X86
