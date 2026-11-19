/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2014-2016 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "Integrator8580.h"

#include <cstdint>

namespace reSIDfp
{

int32_t Integrator8580::solve(int32_t vi) const
{
    // Make sure we're not in subthreshold mode
    assert(vx < nVgt);

    // DAC voltages
    const uint32_t Vgst = nVgt - vx;
    const uint32_t Vgdt = (vi < nVgt) ? nVgt - vi : 0;  // triode/saturation mode

    const uint32_t Vgst_2 = Vgst * Vgst;
    const uint32_t Vgdt_2 = Vgdt * Vgdt;

    // DAC current, scaled by (1/m)*2^13*m*2^16*m*2^16*2^-15 = m*2^30
    const int32_t n_I_dac = (n_dac * (static_cast<int32_t>(Vgst_2 - Vgdt_2) >> 15)) >> 4;

    // Change in capacitor charge.
    vc += n_I_dac;

    // vx = g(vc)
    const int32_t tmp = (vc >> 15) - INT16_MIN;
    assert(tmp <= UINT16_MAX);
    vx = fmc.getOpampRev(tmp);

    // Return vo.
    return vx - (vc >> 14);
}

} // namespace reSIDfp
