/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "FilterModelConfig6581.h"

#include "Integrator6581.h"
#include "OpAmp.h"

#include <algorithm>
#include <mutex>
#include <thread>
#include <cmath>
#include <cstdint>

#if defined(HAVE_CXX20) && defined(__cpp_lib_jthread)
#  define HAVE_JTHREADS
#endif

namespace reSIDfp
{

constexpr unsigned int OPAMP_SIZE = 33;

/**
 * This is the SID 6581 op-amp voltage transfer function, measured on
 * CAP1B/CAP1A on a chip marked MOS 6581R4AR 0687 14.
 * All measured chips have op-amps with output voltages (and thus input
 * voltages) within the range of 0.81V - 10.31V.
 */
constexpr Spline::Point opamp_voltage[OPAMP_SIZE] =
{
  {  0.81, 10.31 },  // Approximate start of actual range
  {  2.40, 10.31 },
  {  2.60, 10.30 },
  {  2.70, 10.29 },
  {  2.80, 10.26 },
  {  2.90, 10.17 },
  {  3.00, 10.04 },
  {  3.10,  9.83 },
  {  3.20,  9.58 },
  {  3.30,  9.32 },
  {  3.50,  8.69 },
  {  3.70,  8.00 },
  {  4.00,  6.89 },
  {  4.40,  5.21 },
  {  4.54,  4.54 },  // Working point (vi = vo)
  {  4.60,  4.19 },
  {  4.80,  3.00 },
  {  4.90,  2.30 },  // Change of curvature
  {  4.95,  2.03 },
  {  5.00,  1.88 },
  {  5.05,  1.77 },
  {  5.10,  1.69 },
  {  5.20,  1.58 },
  {  5.40,  1.44 },
  {  5.60,  1.33 },
  {  5.80,  1.26 },
  {  6.00,  1.21 },
  {  6.40,  1.12 },
  {  7.00,  1.02 },
  {  7.50,  0.97 },
  {  8.50,  0.89 },
  { 10.00,  0.81 },
  { 10.31,  0.81 },  // Approximate end of actual range
};

constexpr double CAPS_OLD = 2200e-12; // ASSY 326298 uses 2200pF caps
constexpr double CAPS_NEW =  470e-12; // Standard 470pF caps used on other ASSY

std::unique_ptr<FilterModelConfig6581> FilterModelConfig6581::instance(nullptr);

std::once_flag flag6581;

FilterModelConfig6581* FilterModelConfig6581::getInstance()
{
    std::call_once(flag6581, [](){ instance.reset(new FilterModelConfig6581()); });

    return instance.get();
}

void FilterModelConfig6581::setFilterRange(double adjustment)
{
    // clamp into allowed range
#ifdef HAVE_CXX17
     adjustment = std::clamp(adjustment, 0.0, 1.0);
#else
     adjustment = std::max(std::min(adjustment, 1.0), 0.);
#endif

     // Get the new uCox value, in the range [1,40]
     const double new_uCox = (1. + 39. * adjustment) * 1e-6;

    // Ignore small changes
    if (std::abs(uCox - new_uCox) < 1e-12)
        return;

    uCox = new_uCox;
    updateParams();
}

void FilterModelConfig6581::enableOldCaps(bool enable)
{
    if (m_oldCaps == enable)
        return;

    m_oldCaps = enable;
    updateParams();
}

void FilterModelConfig6581::updateParams()
{
    calcCurrFactorCoeff();
    vcr_mult = uCox;

    if (m_oldCaps)
    {
        constexpr double caps_mult = CAPS_NEW/CAPS_OLD;
        currFactorCoeff *= caps_mult;
        vcr_mult *= caps_mult;
    }
}

FilterModelConfig6581::FilterModelConfig6581() :
    FilterModelConfig(
        1.5,                    // voice voltage range FIXME should theoretically be ~3,571V
        CAPS_NEW,               // capacitor value
        12. * VOLTAGE_SKEW,     // Vdd
        1.31,                   // Vth
        20e-6,                  // uCox
        opamp_voltage,
        OPAMP_SIZE
    ),
    WL_vcr(9.0 / 1.0),
    WL_snake(1.0 / 115.0),
    dac_zero(7.15),
    dac_scale(2.63),
    dac(DAC_BITS),
    m_oldCaps(false)
{
    updateParams();

    dac.kinkedDac(MOS6581);

    {
        Dac envDac(8);
        envDac.kinkedDac(MOS6581);
        for(int i=0; i<256; i++)
        {
            const double envI = envDac.getOutput(i);
            voiceDC[i] = 5. * VOLTAGE_SKEW + (0.2143 * envI);
        }
    }

    // Create lookup tables for gains / summers.

    //
    // We spawn six threads to calculate these tables in parallel
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

        buildMixerTable(opampModel, 8.0 / (6.0 * VF_TR_RATIO));
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

        buildVolumeTable(opampModel, 12.0);
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

        // build temp n table
        double resonance_n[16];
        for (int n8 = 0; n8 < 16; n8++)
        {
            resonance_n[n8] = (~n8 & 0xf) / 8.0;
        }

        buildResonanceTable(opampModel, resonance_n);
    };

    auto filterVcrVg = [this]
    {
        const double nVddt = N16 * (Vddt - vmin);

        for (unsigned int i = 0; i < (1 << 16); i++)
        {
            // The table index is right-shifted 16 times in order to fit in
            // 16 bits; the argument to sqrt is thus multiplied by (1 << 16).
            vcr_nVg[i] = to_uint16(nVddt - std::sqrt(static_cast<double>(i << 16)));
        }
    };

    auto filterVcrIds = [this]
    {
        //  EKV model:
        //
        //  Ids = Is * (if - ir)
        //  Is = (2 * u*Cox * Ut^2)/k * W/L
        //  if = ln^2(1 + e^((k*(Vg - Vt) - Vs)/(2*Ut))
        //  ir = ln^2(1 + e^((k*(Vg - Vt) - Vd)/(2*Ut))

        // moderate inversion characteristic current
        // will be multiplied by uCox later
        const double Is = (2. * Ut * Ut) * WL_vcr;

        // Normalized current factor for 1 cycle at 1MHz.
        const double N15 = norm * INT16_MAX;
        const double n_Is = N15 * 1.0e-6 / C * Is;

        // kVgt_Vx = k*(Vg - Vt) - Vx
        // I.e. if k != 1.0, Vg must be scaled accordingly.
        const double  r_N16_2Ut = 1.0 / (N16 * 2.0 * Ut);
        for (int i = 0; i < (1 << 16); i++)
        {
            const int kVgt_Vx = i + INT16_MIN;
            const double log_term = std::log1p(std::exp(kVgt_Vx * r_N16_2Ut));
            // Scaled by m*2^15
            vcr_n_Ids_term[i] = n_Is * log_term * log_term;
        }
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
    sidThread thdVcrVg(filterVcrVg);
    sidThread thdVcrIds(filterVcrIds);

#ifndef HAVE_JTHREADS
    thdSummer.join();
    thdMixer.join();
    thdGain.join();
    thdResonance.join();
    thdVcrVg.join();
    thdVcrIds.join();
#endif
}

uint16_t* FilterModelConfig6581::getDAC(double adjustment) const
{
    const double new_dac_zero = getDacZero(adjustment);

    uint16_t* f0_dac = new uint16_t[1 << DAC_BITS];

    for (unsigned int i = 0; i < (1 << DAC_BITS); i++)
    {
        const double fcd = dac.getOutput(i);
        f0_dac[i] = getNormalizedValue(new_dac_zero + fcd * dac_scale);
    }

    return f0_dac;
}

} // namespace reSIDfp
