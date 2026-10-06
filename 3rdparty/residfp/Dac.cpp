/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2025 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem <resid@nimrod.no>
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

#include "Dac.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

constexpr double MOSFET_LEAKAGE_6581 = 0.0075;
constexpr double MOSFET_LEAKAGE_8580 = 0.0035;

Dac::Dac(unsigned int bits) :
    dac(new double[bits]),
    dacLength(bits)
{}

Dac::~Dac()
{
    delete [] dac;
}

double Dac::getOutput(unsigned int input, bool saturate) const
{
    double dacValue = 0.;

    for (unsigned int i = 0; i < dacLength; i++)
    {
        const bool transistor_on = (input & (1 << i)) != 0;
        dacValue += transistor_on ? dac[i] : dac[i] * leakage;
    }

    /*
     * Rough attempt at modeling the MDAC saturation for the 6581.
     * Things are actually more complex, the saturation is likely
     * caused by the two NMOS source followers, one at the output
     * of the waveform DAC and the second at the output of the MDAC.
     * The buffers are also supposed to introduce a DC offset.
     * As a first step we use a cubic model for saturation and
     * apply it only at the waveform output, providing a decent
     * result without any runtime overhead.
     */
    if (saturate)
    {
        static constexpr double GAIN = 1.1;
        static constexpr double SAT = 1.1;
        dacValue = GAIN*dacValue + (1. - GAIN)*SAT*dacValue*dacValue*dacValue;
    }
    return dacValue;
}

void Dac::kinkedDac(ChipModel chipModel, double leakLevel)
{
    constexpr double R_INFINITY = 1e6;

    // Non-linearity parameter, 8580 DACs are perfectly linear
    const double _2R_div_R = chipModel == MOS6581 ? 2.20 : 2.00;

    // 6581 DACs are not terminated by a 2R resistor
    const bool term = chipModel == CSG8580;

    leakage = leakLevel * (chipModel == MOS6581 ? MOSFET_LEAKAGE_6581 : MOSFET_LEAKAGE_8580);

    double Vsum = 0.;

    // Calculate voltage contribution by each individual bit in the R-2R ladder.
    for (unsigned int set_bit = 0; set_bit < dacLength; set_bit++)
    {
        double Vn = 1.;                   // Normalized bit voltage.
        double R = 1.;                    // Normalized R
        const double _2R = _2R_div_R * R; // 2R
        double Rn = term ?                // Rn = 2R for correct termination,
                    _2R : R_INFINITY;     // INFINITY for missing termination.

        unsigned int bit;

        // Calculate DAC "tail" resistance by repeated parallel substitution.
        for (bit = 0; bit < set_bit; bit++)
        {
            Rn = (Rn == R_INFINITY) ?
                 R + _2R :
                 R + (_2R * Rn) / (_2R + Rn); // R + 2R || Rn
        }

        // Source transformation for bit voltage.
        if (Rn == R_INFINITY)
        {
            Rn = _2R;
        }
        else
        {
            Rn = (_2R * Rn) / (_2R + Rn); // 2R || Rn
            Vn = Vn * Rn / _2R;
        }

        // Calculate DAC output voltage by repeated source transformation from
        // the "tail".

        for (++bit; bit < dacLength; bit++)
        {
            Rn += R;
            const double I = Vn / Rn;
            Rn = (_2R * Rn) / (_2R + Rn); // 2R || Rn
            Vn = Rn * I;
        }

        dac[set_bit] = Vn;
        Vsum += Vn;
    }

    // Normalize to integerish behavior
    for (unsigned int i = 0; i < dacLength; i++)
    {
        dac[i] /= Vsum;
    }
}

} // namespace reSIDfp
