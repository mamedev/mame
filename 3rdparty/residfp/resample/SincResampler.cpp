/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "SincResampler.h"

#include <algorithm>
#include <iterator>
#include <numeric>
#ifdef __has_include
#  if __has_include(<version>)
#    include <version>
#  endif
#endif
#include <cassert>
#include <cstring>
#include <cmath>
#include <cstdint>

#include "siddefs-fp.h"

#ifdef __cpp_lib_math_constants
#  include <numbers>
#endif

#if __cplusplus >= 201402L
#  define CONSTEXPR_FUNC  constexpr
#  define CONSTEXPR_VAR   constexpr
#else
#  define CONSTEXPR_FUNC
#  define CONSTEXPR_VAR   const
#endif

namespace reSIDfp
{

/// Maximum error acceptable in I0 is 1e-6, or ~96 dB.
constexpr double I0E = 1e-6;

constexpr int BITS = 16;

/**
 * Compute the 0th order modified Bessel function of the first kind.
 * This function is originally from resample-1.5/filterkit.c by J. O. Smith.
 * It is used to build the Kaiser window for resampling.
 *
 * @param x evaluate I0 at x
 * @return value of I0 at x.
 */
CONSTEXPR_FUNC
double I0(double x)
{
    double sum = 1.;
    double u = 1.;
    double n = 1.;
    const double halfx = x / 2.;

    do
    {
        const double temp = halfx / n;
        u *= temp * temp;
        sum += u;
        n += 1.;
    }
    while (u >= I0E * sum);

    return sum;
}

#ifdef RUNTIME_DISPATCH

// https://godbolt.org/z/hz51cTT8s

#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute( assume )
#    define CONVOLVE_SIMD(simd, name) \
        __attribute__ ((__target__ (#simd))) \
        int32_t convolve_ ## name(const int32_t* a, const int16_t* b, int bLength)  \
        { \
            [[assume( bLength > 0 )]]; \
            int32_t out = std::inner_product(a, a+bLength, b, out); \
            return (out + (1 << 14)) >> 15; \
        }
#  endif
#endif

#ifndef CONVOLVE_SIMD
#  define CONVOLVE_SIMD(simd, name) \
        __attribute__ ((__target__ (#simd))) \
        int32_t convolve_ ## name(const int32_t* a, const int16_t* b, int bLength)  \
        { \
            int32_t out = std::inner_product(a, a+bLength, b, out); \
            return (out + (1 << 14)) >> 15; \
        }
#endif

CONVOLVE_SIMD(mmx, mmx)
CONVOLVE_SIMD(sse2, sse2)
CONVOLVE_SIMD(sse4.1, sse4)
CONVOLVE_SIMD(avx2, avx2)
CONVOLVE_SIMD(avx512f, avx512f)

#endif

/**
 * Calculate convolution with sample and sinc.
 *
 * @param a sample buffer input
 * @param b sinc buffer
 * @param bLength length of the sinc buffer
 * @return convolved result
 */
int32_t convolve(const int32_t* a, const int16_t* b, int bLength)
{
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute( assume )
    [[assume( bLength > 0 )]];
#  endif
#endif
    int32_t out = 0;
#ifndef __clang__
    out = std::inner_product(a, a+bLength, b, out);
#else
    // Apparently clang is unable to fully optimize the above
    // feed it some plain ol' C code
    for (int i=0; i<bLength; i++)
    {
        out += a[i] * static_cast<int32_t>(b[i]);
    }
#endif

    return (out + (1 << 14)) >> 15;
}

int32_t SincResampler::fir(int subcycle)
{
    // Find the first of the nearest fir tables close to the phase
    int firTableFirst = (subcycle * firRES >> 10);
    const int firTableOffset = (subcycle * firRES) & 0x3ff;

    // Find firN most recent samples, plus one extra in case the FIR wraps.
    int sampleStart = sampleIndex - firN + RINGSIZE - 1;

#ifdef RUNTIME_DISPATCH
    const int32_t v1 = simd_convolve(sample + sampleStart, (*firTable)[firTableFirst], firN);
#else
    const int32_t v1 = convolve(sample + sampleStart, (*firTable)[firTableFirst], firN);
#endif

    // Use next FIR table, wrap around to first FIR table using
    // previous sample.
    if (unlikely(++firTableFirst == firRES))
    {
        firTableFirst = 0;
        ++sampleStart;
    }

#ifdef RUNTIME_DISPATCH
    const int32_t v2 = simd_convolve(sample + sampleStart, (*firTable)[firTableFirst], firN);
#else
    const int32_t v2 = convolve(sample + sampleStart, (*firTable)[firTableFirst], firN);
#endif

    // Linear interpolation between the sinc tables yields good
    // approximation for the exact value.
    return v1 + (firTableOffset * (v2 - v1) >> 10);
}

SincResampler::SincResampler(
        double clockFrequency,
        double samplingFrequency,
        double highestAccurateFrequency) :
    cyclesPerSample(static_cast<int>(clockFrequency / samplingFrequency * 1024.))
{
#ifdef __cpp_lib_math_constants
    constexpr double PI = std::numbers::pi;
#else
#  ifdef M_PI
        constexpr double PI = M_PI;
#else
        constexpr double PI = 3.14159265358979323846;
#  endif
#endif

    // A fraction of the bandwidth is allocated to the transition band, which we double
    // because we design the filter to transition halfway at nyquist.
    const double dw = (1. - 2.*highestAccurateFrequency / samplingFrequency) * PI * 2.;

    // 16 bits -> -96dB stopband attenuation.
#if __cplusplus >= 202306L
    constexpr double A = -20. * std::log10(1.0 / (1 << BITS));
#else
    constexpr double A = 96.3295986125;
#endif
    // For calculation of beta and N see the reference for the kaiserord
    // function in the MATLAB Signal Processing Toolbox:
    // http://www.mathworks.com/help/signal/ref/kaiserord.html
    constexpr double beta = 0.1102 * (A - 8.7);
    CONSTEXPR_VAR double I0beta = I0(beta);
    const double cyclesPerSampleD = clockFrequency / samplingFrequency;
    const double inv_cyclesPerSampleD = samplingFrequency / clockFrequency;

    {
        // The filter order will maximally be 124 with the current constraints.
        // N >= (96.33 - 7.95)/(2 * pi * 2.285 * (maxfreq - passbandfreq) >= 123
        // The filter order is equal to the number of zero crossings, i.e.
        // it should be an even number (sinc is symmetric with respect to x = 0).
        int N = static_cast<int>((A - 7.95) / (2.285 * dw) + 0.5);
        N += N & 1;

        // The filter length is equal to the filter order + 1.
        // The filter length must be an odd number (sinc is symmetric with respect to
        // x = 0).
        firN = static_cast<int>(N * cyclesPerSampleD) + 1;
        firN |= 1;

        // Check whether the sample ring buffer would overflow.
        assert(firN < RINGSIZE);

        // Error is bounded by err < 1.234 / L^2, so L = sqrt(1.234 / (2^-16)) = sqrt(1.234 * 2^16).
        firRES = static_cast<int>(std::ceil(std::sqrt(1.234 * (1 << BITS)) * inv_cyclesPerSampleD));

        // firN*firRES represent the total resolution of the sinc sampling. JOS
        // recommends a length of 2^BITS, but we don't quite use that good a filter.
        // The filter test program indicates that the filter performs well, though.
    }

    {
        // Allocate memory for FIR tables.
        firTable = new matrix_t(firRES, firN);

        // The cutoff frequency is midway through the transition band, in effect the same as nyquist.
        constexpr double wc = PI;

        // Calculate the sinc tables.
        const double scale = 32768.0 * wc * inv_cyclesPerSampleD / PI;

        // we're not interested in the fractional part
        // so use int division before converting to double
        const int tmp = firN / 2;
        const double firN_2 = static_cast<double>(tmp);

        for (int i = 0; i < firRES; i++)
        {
            const double jPhase = static_cast<double>(i) / static_cast<double>(firRES) + firN_2;

            for (int j = 0; j < firN; j++)
            {
                const double x = j - jPhase;

                const double xt = x / firN_2;
                const double kaiserXt = std::fabs(xt) < 1. ? I0(beta * std::sqrt(1. - xt * xt)) / I0beta : 0.;

                const double wt = wc * x * inv_cyclesPerSampleD;
                const double sincWt = std::fabs(wt) >= 1e-8 ? std::sin(wt) / wt : 1.;

                (*firTable)[i][j] = static_cast<int16_t>(scale * sincWt * kaiserXt);
            }
        }
    }

#ifdef RUNTIME_DISPATCH

#define DISPATCH(simd, name) \
    (__builtin_cpu_supports (#simd)) \
        simd_convolve = convolve_ ## name;

    if DISPATCH(avx512f, avx512f)
    else
    if DISPATCH(avx2, avx2)
    else
    if DISPATCH(sse4.1, sse4)
    else
    if DISPATCH(sse2, sse2)
    else
    if DISPATCH(mmx, mmx)
    else
        simd_convolve = convolve;
#endif
}

SincResampler::~SincResampler()
{
    delete firTable;
}

bool SincResampler::input(int32_t input)
{
    bool ready = false;

    sample[sampleIndex] = sample[sampleIndex + RINGSIZE] = input;
    sampleIndex = (sampleIndex + 1) & (RINGSIZE - 1);

    if (unlikely(sampleOffset < 1024))
    {
        outputValue = fir(sampleOffset);
        ready = true;
        sampleOffset += cyclesPerSample;
    }

    sampleOffset -= 1024;

    return ready;
}

void SincResampler::reset()
{
    std::fill(std::begin(sample), std::end(sample), 0);
    sampleOffset = 0;
    outputValue = 0;
}

} // namespace reSIDfp
