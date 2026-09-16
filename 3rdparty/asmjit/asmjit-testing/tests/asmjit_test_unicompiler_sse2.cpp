// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/ujit.h>

#if defined(ASMJIT_UJIT_X86)

#if defined(_MSC_VER)
  #include <intrin.h>
#else
  #include <emmintrin.h>
#endif

namespace UniCompilerTests {

// A reference implementation of MUL+ADD without the use of FMA. This has to be provided otherwise the
// compiler may use FPU registers in 32-bit x86 case, which would make the result different than when
// compiled by JIT compiler that would use XMM registers (32/64-bit SSE/AVX operations).

float fadd(float a, float b) noexcept {
  return _mm_cvtss_f32(_mm_add_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

float fsub(float a, float b) noexcept {
  return _mm_cvtss_f32(_mm_sub_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

float fmul(float a, float b) noexcept {
  return _mm_cvtss_f32(_mm_mul_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

float fdiv(float a, float b) noexcept {
  return _mm_cvtss_f32(_mm_div_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

float fsqrt(float a) noexcept {
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(a)));
}

float fmadd_nofma_ref(float a, float b, float c) noexcept {
  return _mm_cvtss_f32(_mm_add_ss(_mm_mul_ss(_mm_set1_ps(a), _mm_set1_ps(b)), _mm_set1_ps(c)));
}

double fadd(double a, double b) noexcept {
  return _mm_cvtsd_f64(_mm_add_sd(_mm_set1_pd(a), _mm_set1_pd(b)));
}

double fsub(double a, double b) noexcept {
  return _mm_cvtsd_f64(_mm_sub_sd(_mm_set1_pd(a), _mm_set1_pd(b)));
}

double fmul(double a, double b) noexcept {
  return _mm_cvtsd_f64(_mm_mul_sd(_mm_set1_pd(a), _mm_set1_pd(b)));
}

double fdiv(double a, double b) noexcept {
  return _mm_cvtsd_f64(_mm_div_sd(_mm_set1_pd(a), _mm_set1_pd(b)));
}

double fsqrt(double a) noexcept {
  return _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set1_pd(a)));
}

double fmadd_nofma_ref(double a, double b, double c) noexcept {
  return _mm_cvtsd_f64(_mm_add_sd(_mm_mul_sd(_mm_set1_pd(a), _mm_set1_pd(b)), _mm_set1_pd(c)));
}

} // {UniCompilerTests}

#endif // ASMJIT_UJIT_X86
