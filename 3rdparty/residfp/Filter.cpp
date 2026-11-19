/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2024 Leandro Nini <drfiemost@users.sourceforge.net>
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

#define FILTER_CPP

#include "Filter.h"

namespace reSIDfp
{

static constexpr int summerIdx[5] =
{
    FilterModelConfig::summer_offset<0>::value,
    FilterModelConfig::summer_offset<1>::value,
    FilterModelConfig::summer_offset<2>::value,
    FilterModelConfig::summer_offset<3>::value,
    FilterModelConfig::summer_offset<4>::value
};

static constexpr int mixerIdx[8] =
{
    FilterModelConfig::mixer_offset<0>::value,
    FilterModelConfig::mixer_offset<1>::value,
    FilterModelConfig::mixer_offset<2>::value,
    FilterModelConfig::mixer_offset<3>::value,
    FilterModelConfig::mixer_offset<4>::value,
    FilterModelConfig::mixer_offset<5>::value,
    FilterModelConfig::mixer_offset<6>::value,
    FilterModelConfig::mixer_offset<7>::value
};

void Filter::updateMixing()
{
    currentVolume = volume + (vol * (1<<16));

    int Nsum = 0;
    int Nmix = 0;

    (filt1 ? Nsum : Nmix)++;
    (filt2 ? Nsum : Nmix)++;

    if (filt3) Nsum++;
    else if (!voice3off) Nmix++;

    (filtE ? Nsum : Nmix)++;

    currentSummer = summer + summerIdx[Nsum];

    if (lp) Nmix++;
    if (bp) Nmix++;
    if (hp) Nmix++;

    currentMixer = mixer + mixerIdx[Nmix];
}

void Filter::writeFC_LO(uint8_t fc_lo)
{
    fc = (fc & 0x7f8) | (fc_lo & 0x007);
    updateCenterFrequency();
}

void Filter::writeFC_HI(uint8_t fc_hi)
{
    fc = (fc_hi << 3 & 0x7f8) | (fc & 0x007);
    updateCenterFrequency();
}

void Filter::writeRES_FILT(uint8_t res_filt)
{
    filt = res_filt;

    updateResonance((res_filt >> 4) & 0x0f);

    if (enabled)
    {
        filt1 = (filt & 0x01) != 0;
        filt2 = (filt & 0x02) != 0;
        filt3 = (filt & 0x04) != 0;
        filtE = (filt & 0x08) != 0;
    }

    updateMixing();
}

void Filter::writeMODE_VOL(uint8_t mode_vol)
{
    vol = mode_vol & 0x0f;
    lp = (mode_vol & 0x10) != 0;
    bp = (mode_vol & 0x20) != 0;
    hp = (mode_vol & 0x40) != 0;
    voice3off = (mode_vol & 0x80) != 0;

    updateMixing();
}

Filter::Filter(const FilterModelConfig& fmc,
               const Integrator& hpIntegrator,
               const Integrator& bpIntegrator) :
    mixer(fmc.getMixer()),
    summer(fmc.getSummer()),
    resonance(fmc.getResonance()),
    volume(fmc.getVolume()),
    m_fmc(fmc),
    m_hpIntegrator(hpIntegrator),
    m_bpIntegrator(bpIntegrator)
{
    input(0);
}

void Filter::enable(bool enable)
{
    enabled = enable;

    if (enabled)
    {
        writeRES_FILT(filt);
    }
    else
    {
        filt1 = filt2 = filt3 = filtE = false;
    }
}

void Filter::reset()
{
    writeFC_LO(0);
    writeFC_HI(0);
    writeMODE_VOL(0);
    writeRES_FILT(0);
}

} // namespace reSIDfp
