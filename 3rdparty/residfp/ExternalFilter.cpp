/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2020 Leandro Nini <drfiemost@users.sourceforge.net>
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

#define EXTERNALFILTER_CPP

#include "ExternalFilter.h"

namespace reSIDfp
{

/**
 * Get the 3 dB attenuation point.
 *
 * @param res the resistance value in Ohms
 * @param cap the capacitance value in Farads
 */
constexpr inline double getRC(double res, double cap)
{
    return res * cap;
}

ExternalFilter::ExternalFilter()
{
    reset();
}

void ExternalFilter::setClockFrequency(double frequency)
{
    m_frequency = frequency;
    recalcParams();
}

void ExternalFilter::setExtResistance(double res)
{
    m_ext_res = 1e3 + ((1. - res) * 9e3);
    recalcParams();
}

void ExternalFilter::recalcParams()
{
    const double dt = 1. / m_frequency;

    // Low-pass:  R = 10kOhm, C = 1000pF; w0l = dt/(dt+RC) = 1e-6/(1e-6+1e4*1e-9) = 0.091
    // Cutoff 1/(2*PI*RC) = 1/(2*PI*1e4*1e-9) = 15915.5 Hz
    w0lp_1_s7 = static_cast<int32_t>((dt / (dt + getRC(10e3, 1000e-12))) * (1 << 7) + 0.5);

    // High-pass: R = 1-10kOhm, C = 10uF;  w0h = dt/(dt+RC)
    // Cutoff 1/(2*PI*RC) ∈ [1.59155,15.9155] Hz
    w0hp_1_s17 = static_cast<int32_t>((dt / (dt + getRC(m_ext_res, 10e-6))) * (1 << 17) + 0.5);
}

void ExternalFilter::reset()
{
    // State of filter.
    Vlp = 0; //1 << (15 + 11);
    Vhp = 0;
}

} // namespace reSIDfp
