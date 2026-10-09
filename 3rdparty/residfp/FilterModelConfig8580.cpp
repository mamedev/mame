/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2023 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2010 Dag Lem
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

#include "FilterModelConfig8580.h"

#include "Integrator8580.h"
#include "OpAmp.h"

#include <mutex>
#include <thread>

#if defined(HAVE_CXX20) && defined(__cpp_lib_jthread)
#  define HAVE_JTHREADS
#endif

namespace reSIDfp
{

/*
 * R1 = 15.3*Ri
 * R2 =  7.3*Ri
 * R3 =  4.7*Ri
 * Rf =  1.4*Ri
 * R4 =  1.4*Ri
 * R8 =  2.0*Ri
 * RC =  2.8*Ri
 *
 * res  feedback  input
 * ---  --------  -----
 *  0   Rf        Ri
 *  1   Rf|R1     Ri
 *  2   Rf|R2     Ri
 *  3   Rf|R3     Ri
 *  4   Rf        R4
 *  5   Rf|R1     R4
 *  6   Rf|R2     R4
 *  7   Rf|R3     R4
 *  8   Rf        R8
 *  9   Rf|R1     R8
 *  A   Rf|R2     R8
 *  B   Rf|R3     R8
 *  C   Rf        RC
 *  D   Rf|R1     RC
 *  E   Rf|R2     RC
 *  F   Rf|R3     RC
 */
constexpr double resGain[16] =
{
                        1.4/1.0, //      Rf/Ri   1.4
    ((1.4*15.3)/(1.4+15.3))/1.0, // (Rf|R1)/Ri   1.28263
      ((1.4*7.3)/(1.4+7.3))/1.0, // (Rf|R2)/Ri   1.17471
      ((1.4*4.7)/(1.4+4.7))/1.0, // (Rf|R3)/Ri   1.07869
                        1.4/1.4, //      Rf/R4   1
    ((1.4*15.3)/(1.4+15.3))/1.4, // (Rf|R1)/R4   0.916168
      ((1.4*7.3)/(1.4+7.3))/1.4, // (Rf|R2)/R4   0.83908
      ((1.4*4.7)/(1.4+4.7))/1.4, // (Rf|R3)/R4   0.770492
                        1.4/2.0, //      Rf/R8   0.7
    ((1.4*15.3)/(1.4+15.3))/2.0, // (Rf|R1)/R8   0.641317
      ((1.4*7.3)/(1.4+7.3))/2.0, // (Rf|R2)/R8   0.587356
      ((1.4*4.7)/(1.4+4.7))/2.0, // (Rf|R3)/R8   0.539344
                        1.4/2.8, //      Rf/RC   0.5
    ((1.4*15.3)/(1.4+15.3))/2.8, // (Rf|R1)/RC   0.458084
      ((1.4*7.3)/(1.4+7.3))/2.8, // (Rf|R2)/RC   0.41954
      ((1.4*4.7)/(1.4+4.7))/2.8, // (Rf|R3)/RC   0.385246
};

constexpr unsigned int OPAMP_SIZE = 21;

/**
 * This is the SID 8580 op-amp voltage transfer function, measured on
 * CAP1B/CAP1A on a chip marked CSG 8580R5 1690 25.
 */
constexpr Spline::Point opamp_voltage[OPAMP_SIZE] =
{
    {  1.30,  8.91 },  // Approximate start of actual range
    {  4.76,  8.91 },
    {  4.77,  8.90 },
    {  4.78,  8.88 },
    {  4.785, 8.86 },
    {  4.79,  8.80 },
    {  4.795, 8.60 },
    {  4.80,  8.25 },
    {  4.805, 7.50 },
    {  4.81,  6.10 },
    {  4.815, 4.05 },  // Change of curvature
    {  4.82,  2.27 },
    {  4.825, 1.65 },
    {  4.83,  1.55 },
    {  4.84,  1.47 },
    {  4.85,  1.43 },
    {  4.87,  1.37 },
    {  4.90,  1.34 },
    {  5.00,  1.30 },
    {  5.10,  1.30 },
    {  8.91,  1.30 },  // Approximate end of actual range
};

std::unique_ptr<FilterModelConfig8580> FilterModelConfig8580::instance(nullptr);

std::once_flag flag8580;

FilterModelConfig8580* FilterModelConfig8580::getInstance()
{
    std::call_once(flag8580, [](){ instance.reset(new FilterModelConfig8580()); });

    return instance.get();
}

FilterModelConfig8580::FilterModelConfig8580() :
    FilterModelConfig(
        0.24,               // voice voltage range FIXME should theoretically be ~0,474V
        22e-9,              // capacitor value
        9. * VOLTAGE_SKEW,  // Vdd
        0.80,               // Vth
        100e-6,             // uCox
        opamp_voltage,
        OPAMP_SIZE
    )
{
    // Create lookup tables for gains / summers.

    //
    // We spawn four threads to calculate these tables in parallel
    //
    auto filterSummer = [this]
    {
        OpAmp opampModel(
            std::vector<Spline::Point>(
                std::begin(opamp_voltage),
                std::end(opamp_voltage)),
            Vddt,
            vmin,
            vmax);

        buildSummerTable(opampModel);
    };

    auto filterMixer = [this]
    {
        OpAmp opampModel(
            std::vector<Spline::Point>(
                std::begin(opamp_voltage),
                std::end(opamp_voltage)),
            Vddt,
            vmin,
            vmax);

        buildMixerTable(opampModel, 8.0 / 5.0);
    };

    auto filterGain = [this]
    {
        OpAmp opampModel(
            std::vector<Spline::Point>(
                std::begin(opamp_voltage),
                std::end(opamp_voltage)),
            Vddt,
            vmin,
            vmax);

        buildVolumeTable(opampModel, 16.0);
    };

    auto filterResonance = [this]
    {
        OpAmp opampModel(
            std::vector<Spline::Point>(
                std::begin(opamp_voltage),
                std::end(opamp_voltage)),
            Vddt,
            vmin,
            vmax);

        buildResonanceTable(opampModel, resGain);
    };

#ifdef HAVE_JTHREADS
    using sidThread = std::jthread;
#else
    using sidThread = std::thread;
#endif

    sidThread thdSummer(filterSummer);
    sidThread thdMixer(filterMixer);
    sidThread thdGain(filterGain);
    sidThread thdResonance(filterResonance);

#ifndef HAVE_JTHREADS
    thdSummer.join();
    thdMixer.join();
    thdGain.join();
    thdResonance.join();
#endif
}

} // namespace reSIDfp
