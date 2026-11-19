/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2024 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem
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

#include "FilterModelConfig.h"

#include <vector>
#include <cstdint>

namespace reSIDfp
{

FilterModelConfig::FilterModelConfig(
    double vvr,
    double c,
    double vdd,
    double vth,
    double ucox,
    const Spline::Point *opamp_voltage,
    int opamp_size
) :
    C(c),
    Vdd(vdd),
    Vth(vth),
    Vddt(Vdd - Vth),
    uCox(ucox),
    vmin(opamp_voltage[0].x),
    vmax(std::max(Vddt, opamp_voltage[0].y)),
    denorm(vmax - vmin),
    norm(1.0 / denorm),
    N16(norm * UINT16_MAX),
    voice_voltage_range(vvr),
    mixer(new uint16_t[mixer_offset<8>::value]),
    summer(new uint16_t[summer_offset<5>::value]),
    volume(new uint16_t[16 * (1 << 16)]),
    resonance(new uint16_t[16 * (1 << 16)])
{
    calcCurrFactorCoeff();

    // Convert op-amp voltage transfer to 16 bit values.

    std::vector<Spline::Point> scaled_voltage(opamp_size);

    for (int i = 0; i < opamp_size; i++)
    {
        scaled_voltage[i].x = N16 * (opamp_voltage[i].x - opamp_voltage[i].y) / 2.;
        // We add 32768 to get a positive number in the range [0-65535]
        scaled_voltage[i].x += static_cast<double>(1u << 15);

        scaled_voltage[i].y = N16 * (opamp_voltage[i].x - vmin);
    }

    // Create lookup table mapping capacitor voltage to op-amp input voltage:

    Spline s(scaled_voltage);

    for (int x = 0; x < (1 << 16); x++)
    {
        const Spline::Point out = s.evaluate(x);
        // When interpolating outside range the first elements may be negative
        opamp_rev[x] = out.x > 0. ? to_uint16(out.x) : 0;
    }
}

FilterModelConfig::~FilterModelConfig()
{
    delete [] mixer;
    delete [] summer;
    delete [] volume;
    delete [] resonance;
}

void FilterModelConfig::calcCurrFactorCoeff()
{
    currFactorCoeff = denorm * (uCox / 2. * 1.0e-6 / C);
}

} // namespace reSIDfp
