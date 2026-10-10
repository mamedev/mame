/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2014 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "Integrator6581.h"

#include <cstdint>

#ifdef SLOPE_FACTOR
#  include <cmath>
#endif

namespace reSIDfp
{

int32_t Integrator6581::solve(int32_t vi) const
{
    // Make sure Vgst>0 so we're not in subthreshold mode
    assert(vx < nVddt);

    // Check that transistor is actually in triode mode
    // Vds < Vgs - Vth
    assert(vi < nVddt);

    // "Snake" voltages for triode mode calculation.
    const uint32_t Vgst = nVddt - vx;
    const uint32_t Vgdt = nVddt - vi;

    const uint32_t Vgst_2 = Vgst * Vgst;
    const uint32_t Vgdt_2 = Vgdt * Vgdt;

    // "Snake" current, scaled by (1/m)*2^13*m*2^16*m*2^16*2^-15 = m*2^30
    const int32_t n_I_snake = fmc.getNormalizedCurrentFactor<13>(wlSnake) * (static_cast<int32_t>(Vgst_2 - Vgdt_2) >> 15);

    // VCR gate voltage.       // Scaled by m*2^16
    // Vg = Vddt - sqrt(((Vddt - Vw)^2 + Vgdt^2)/2)
    const int32_t nVg = static_cast<int32_t>(fmc.getVcr_nVg((nVddt_Vw_2 + (Vgdt_2 >> 1)) >> 16));
#ifdef SLOPE_FACTOR
    const double nVp = static_cast<double>(nVg - nVt) / n; // Pinch-off voltage
    const int32_t kVgt = static_cast<int32_t>(nVp + 0.5) - nVmin;
#else
    const int32_t kVgt = (nVg - nVt) - nVmin;
#endif

    // VCR voltages for EKV model table lookup.
    const int32_t kVgt_Vs = (kVgt - vx) - INT16_MIN;
    assert((kVgt_Vs >= 0) && (kVgt_Vs <= UINT16_MAX));
    const int32_t kVgt_Vd = (kVgt - vi) - INT16_MIN;
    assert((kVgt_Vd >= 0) && (kVgt_Vd <= UINT16_MAX));

    // VCR current, scaled by m*2^15*2^15 = m*2^30
    const uint32_t If = static_cast<uint32_t>(fmc.getVcr_n_Ids_term(kVgt_Vs)) << 15;
    const uint32_t Ir = static_cast<uint32_t>(fmc.getVcr_n_Ids_term(kVgt_Vd)) << 15;
#ifdef SLOPE_FACTOR
    const double iVcr = static_cast<double>(If - Ir);
    const int32_t n_I_vcr = static_cast<int32_t>(iVcr * n);
#else
    const int32_t n_I_vcr = If - Ir;
#endif

#ifdef SLOPE_FACTOR
    // estimate new slope factor based on gate voltage
    constexpr double gamma = 1.0;   // body effect factor
    constexpr double phi = 0.8;     // bulk Fermi potential
    const double Vp = nVp / fmc.getN16();
    n = 1. + (gamma / (2. * std::sqrt(Vp + phi + 4. * fmc.getUt())));
    assert((n > 1.2) && (n < 1.8));
#endif

    // Change in capacitor charge.
    vc += n_I_snake + n_I_vcr;

    // vx = g(vc)
    const int32_t tmp = (vc >> 15) - INT16_MIN;
    assert(tmp <= UINT16_MAX);
    vx = fmc.getOpampRev(tmp);

    // Return vo.
    return vx - (vc >> 14);
}

} // namespace reSIDfp
