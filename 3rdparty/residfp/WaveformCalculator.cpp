/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
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

#include "WaveformCalculator.h"

#include "siddefs-fp.h"

#include <map>
#include <mutex>
#include <cmath>

namespace reSIDfp
{

/**
 * Combined waveform model parameters.
 */
using distance_t = float (*)(float, int);

using CombinedWaveformConfig = struct
{
    distance_t distFunc;
    float threshold;
    float topbit;
    float pulsestrength;
    float distance1;
    float distance2;
};

using cw_cache_t = std::map<const CombinedWaveformConfig*, rc_matrix_t>;

cw_cache_t PULLDOWN_CACHE;

std::mutex PULLDOWN_CACHE_Lock;

WaveformCalculator* WaveformCalculator::getInstance()
{
    static WaveformCalculator instance;
    return &instance;
}

// Distance functions
static float exponentialDistance(float distance, int i)
{
    return std::pow(distance, -i);
}

static float linearDistance(float distance, int i)
{
    return 1.f / (1.f + i * distance);
}

static float quadraticDistance(float distance, int i)
{
    return 1.f / (1.f + (i*i) * distance);
}

/**
 * Parameters derived with the Monte Carlo method based on
 * samplings from real machines.
 * Code and data available in the project repository [1].
 * Sampling program made by Dag Lem [2].
 *
 * The score here reported is the acoustic error
 * calculated XORing the estimated and the sampled values.
 * For the combinations including saw on 6581 only
 * the first half of the wave is considered.
 * In parentheses the number of mispredicted bits.
 *
 * [1] https://github.com/libsidplayfp/combined-waveforms
 * [2] https://github.com/daglem/reDIP-SID/blob/master/research/combsample.d64
 */
const CombinedWaveformConfig configAverage[2][5] =
{
    { /* 6581 R3 0486S sampled by Trurl */
        // TS  error   406  (764/32768) [RMS: -13.55]
        { exponentialDistance,  0.79111582f, 1.06053483f, 0.f, 1.97922957f, 2.67848182f },
        // PT  error  4590  (124/32768) [RMS: -11.40dB]
        { linearDistance, 0.941692829f, 1.f, 1.80072665f, 0.033124879f, 0.232303441f },
        // PS  error   211 (1030/32768) [RMS: -10.31]
        { linearDistance, 1.09394681f, 1.42332006f, 3.44251633f, 0.0797301158f, 0.102444135f },
        // PTS error    57  (278/32768) [RMS: -18.51]
        { linearDistance, 1.53609717f, 0.0391017497f, 1.67601228f, 1.44580793f, 1.42448807f },
        // NP  guessed
        { exponentialDistance, 0.96f, 1.f, 2.5f, 1.1f, 1.2f },
    },
    { /* 8580 R5 1088 sampled by reFX-Mike */
        // TS  error 10660 (353/32768) [RMS: -12.85dB]
        { exponentialDistance, 0.853578329f, 1.09615636f, 0.f, 1.8819375f, 6.80794907f },
        // PT  error 10635 (289/32768) [RMS: -7.43dB]
        { exponentialDistance, 0.929835618f, 1.f, 1.12836814f, 1.10453653f, 1.48065746f },
        // PS  error 12255 (554/32768) [RMS: -7.97dB]
        { quadraticDistance, 0.911938608f, 0.996440411f, 1.2278074f, 0.000117214302f, 0.18948476f },
        // PTS error  6913 (127/32768) [RMS: -13.23dB]
        { exponentialDistance, 0.938004673f, 1.04827631f, 1.21178246f, 0.915959001f, 1.42698038f },
        // NP  guessed
        { exponentialDistance, 0.95f, 1.f, 1.15f, 1.f, 1.45f },
    },
};

const CombinedWaveformConfig configWeak[2][5] =
{
    { /* 6581 R2 4383 sampled by ltx128 */
        // TS  error  169 (843/32768) [RMS: -14.61]
        { exponentialDistance, 0.946056068f, 26.9527836f, 0.f, 6.38644743f, 3.61852479f },
        // PT  error  612  (102/32768) [RMS: -15.35dB]
        { linearDistance, 1.01262534f, 1.f, 2.46070528f, 0.0537485816f, 0.0986242667f },
        // PS  error    5 (1535/32768) [RMS: -15.71]
        { linearDistance, 0.760607481f, 0.0588437207f, 0.407531887f, 0.0994859114f, 0.000200334835f },
        // PTS error    0  (138/32768) [RMS: -25.46]
        { linearDistance, 1.10582423f, 0.578988612f, 1.94850934f, 0.0783150643f, 0.300926387f },
        // NP  guessed
        { exponentialDistance, 0.96f, 1.f, 2.5f, 1.1f, 1.2f },
    },
    { /* 8580 R5 4887 sampled by reFX-Mike */
        // TS  error  741 (76/32768) [RMS: -13.56dB]
        { exponentialDistance, 0.812351167f, 1.1727736f, 0.f, 1.87459648f, 2.31578159f },
        // PT  error 7199 (192/32768) [RMS: -9.23]
        { exponentialDistance, 0.917997837f, 1.f, 1.01248944f, 1.05761552f, 1.37529826f },
        // PS  error 9849 (333/32768) [RMS: -9.45]
        { quadraticDistance, 0.969898582f, 1.00785899f, 1.30233467f, 0.00962228701f, 0.146903187f },
        // PTS error 4809 (60/32768) [RMS: -15.03dB]
        { exponentialDistance, 0.941834152f, 1.06401193f, 0.991132736f, 0.995310068f, 1.41105855f },
        // NP  guessed
        { exponentialDistance, 0.95f, 1.f, 1.15f, 1.f, 1.45f },
    },
};

const CombinedWaveformConfig configStrong[2][5] =
{
    { /* 6581 R2 0384 sampled by Trurl */
        // TS  error   754 (2056/32768) [RMS: -18.87]
        { exponentialDistance, 0.714277208f, 0.00729158986f, 0.f, 2.12244034f, 1.66707671f },
        // PT  error  5190  (238/32768) [RMS: -9.73dB]
        { linearDistance, 0.924780309f, 1.f, 1.96809769f, 0.0888123438f, 0.234606609f },
        // PS  error   860 (1288/32768) [RMS: -8.28]
        { linearDistance, 1.02248156f, 1.36571658f, 3.66920304f, 0.00203792308f, 0.0826661736f },
        // PTS error    60  (411/32768) [RMS: -17.97]
        { linearDistance, 0.891591191f, 1.88450027f, 1.33901846f, 0.134212971f, 0.293466389f },
        // NP  guessed
        { exponentialDistance, 0.96f, 1.f, 2.5f, 1.1f, 1.2f },
    },
    { /* 8580 R5 1489 sampled by reFX-Mike */
        // TS  error  4837 (388/32768) [RMS: -10.54dB]
        { exponentialDistance, 0.89762634f, 56.7594185f, 0.f, 7.68995237f, 12.0754194f },
        // PT  error  9242 (504/32768) [RMS: -6.03]
        { exponentialDistance,  0.871706188f, 1.f, 1.44852948f, 1.05926013f, 1.43830109f },
        // PS  error 13146 (713/32768) [RMS: -6.34]
        { quadraticDistance, 0.892224431f, 1.22416508f, 1.74952936f, 0.0251259189f, 0.13089405f },
        // PTS error  6702 (300/32768) [RMS: -11.14dB]
        { linearDistance, 0.91124934f, 0.963609755f, 0.909965038f, 1.07445884f, 1.82399702f },
        // NP  guessed
        { exponentialDistance, 0.95f, 1.f, 1.15f, 1.f, 1.45f },
    },
};

/// Calculate triangle waveform
inline unsigned int triXor(unsigned int val)
{
    return (((val & 0x800) == 0) ? val : (val ^ 0xfff)) << 1;
}

/**
 * Generate bitstate based on emulation of combined waves pulldown.
 *
 * @param distancetable
 * @param pulsestrength
 * @param threshold
 * @param wave the waveform bits
 */
int16_t calculatePulldown(float distancetable[], float topbit, float pulsestrength, float threshold, unsigned int wave)
{
    float bit[12];

    for (unsigned int i = 0; i < 12; i++)
    {
        bit[i] = (wave & (1u << i)) != 0 ? 1.f : 0.f;
    }

    bit[11] *= topbit;

    float pulldown[12];

    for (int sb = 0; sb < 12; sb++)
    {
        float avg = 0.f;
        float n = 0.f;

        for (int cb = 0; cb < 12; cb++)
        {
            if (cb == sb)
                continue;
            const float weight = distancetable[sb - cb + 12];
            avg += (1.f - bit[cb]) * weight;
            n += weight;
        }

        avg -= pulsestrength;

        pulldown[sb] = avg / n;
    }

    // Get the predicted value
    int16_t value = 0;

    for (unsigned int i = 0; i < 12; i++)
    {
        const float bitValue = bit[i] > 0.f ? 1.f - pulldown[i] : 0.f;
        if (bitValue > threshold)
        {
            value |= 1u << i;
        }
    }

    return value;
}

WaveformCalculator::WaveformCalculator() :
    wftable(std::make_shared<matrix_t>(4, 4096))
{
    // Build waveform table.
    for (unsigned int idx = 0; idx < 4096; idx++)
    {
        const int16_t saw = static_cast<int16_t>(idx);
        const int16_t tri = static_cast<int16_t>(triXor(idx));

        (*wftable)[0][idx] = 0xfff;
        (*wftable)[1][idx] = tri;
        (*wftable)[2][idx] = saw;
        (*wftable)[3][idx] = saw & (saw << 1);
    }
}

rc_matrix_t WaveformCalculator::buildPulldownTable(ChipModel model, CombinedWaveforms cws)
{
    std::lock_guard<std::mutex> lock(PULLDOWN_CACHE_Lock);

    const int modelIdx = model == MOS6581 ? 0 : 1;
    const CombinedWaveformConfig* cfgArray;

    switch (cws)
    {
    default:
    case AVERAGE:
        cfgArray = configAverage[modelIdx];
        break;
    case WEAK:
        cfgArray = configWeak[modelIdx];
        break;
    case STRONG:
        cfgArray = configStrong[modelIdx];
        break;
    }

    cw_cache_t::iterator lb = PULLDOWN_CACHE.lower_bound(cfgArray);

    if (lb != PULLDOWN_CACHE.end() && !(PULLDOWN_CACHE.key_comp()(cfgArray, lb->first)))
    {
        return lb->second;
    }

    rc_matrix_t pdTable = std::make_shared<matrix_t>(5, 4096);

    for (int wav = 0; wav < 5; wav++)
    {
        const CombinedWaveformConfig& cfg = cfgArray[wav];

        const distance_t distFunc = cfg.distFunc;

        float distancetable[12 * 2 + 1];
        distancetable[12] = 1.f;
        for (int i = 12; i > 0; i--)
        {
            distancetable[12-i] = distFunc(cfg.distance1, i);
            distancetable[12+i] = distFunc(cfg.distance2, i);
        }

        for (unsigned int idx = 0; idx < 4096; idx++)
        {
            (*pdTable)[wav][idx] = calculatePulldown(distancetable, cfg.topbit, cfg.pulsestrength, cfg.threshold, idx);
        }
    }

    return PULLDOWN_CACHE.emplace_hint(lb, cw_cache_t::value_type(cfgArray, pdTable))->second;
}

} // namespace reSIDfp
