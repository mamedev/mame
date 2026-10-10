/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2026 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef STATE_H
#define STATE_H

#include "EnvelopeGenerator.h"
#include "resample/TwoPassSincResampler.h"
#include "residfp/residfp_defs.h"

#include <cstdint>

namespace reSIDfp
{

class SID;

struct Params
{
    SamplingMethod method;
    double samplingFrequency;
    double filterCurve6581;
    double filterRange6581;
    double filterCurve8580;
    bool   old6581caps;
};

struct State
{
    /**
     * Save current state.
     */
    static int saveState(SID &s, char* buffer, int size);

    /**
     * Restore saved state.
     */
    static void restoreState(SID &s, char* buffer, int size);

    /**
     * Return size of save state.
     */
    static int size(SID &s);

    // SID
    int bus_value_ttl;
    unsigned int nextVoiceSync;
    unsigned int offset_6581;
    ChipModel model;
    CombinedWaveforms cws;
    double dacLeakage;
    uint8_t bus_value;
    uint8_t paddle_x;
    uint8_t paddle_y;

    // Waveform
    uint32_t pw[3];
    uint32_t shift_register[3];
    uint32_t shift_latch[3];
    uint32_t ring_msb_mask[3];
    uint32_t no_noise[3];
    uint32_t noise_output[3];
    uint32_t no_noise_or_noise_output[3];
    uint32_t no_pulse[3];
    uint32_t pulse_output[3];
    uint32_t waveform_output[3];
    uint32_t accumulator[3];
    uint32_t freq[3];
    uint32_t tri_saw_pipeline[3];
    uint32_t osc3[3];
    int shift_pipeline[3];
    unsigned int shift_register_reset[3];
    unsigned int floating_output_ttl[3];
    bool test[3];
    bool sync[3];
    bool test_or_reset[3];
    bool msb_rising[3];
    uint8_t waveform[3];

    // Envelope
    uint16_t lfsr[3];
    uint16_t rate[3];
    unsigned int exponential_counter[3];
    unsigned int exponential_counter_period[3];
    unsigned int new_exponential_counter_period[3];
    unsigned int state_pipeline[3];
    unsigned int envelope_pipeline[3];
    unsigned int exponential_pipeline[3];
    EnvelopeGenerator::State env_state[3];
    EnvelopeGenerator::State next_state[3];
    bool counter_enabled[3];
    bool gate[3];
    bool resetLfsr[3];
    uint8_t envelope_counter[3];
    uint8_t attack[3];
    uint8_t decay[3];
    uint8_t sustain[3];
    uint8_t release[3];
    uint8_t env3[3];

    // Filter
    int32_t Vhp[2];
    int32_t Vbp[2];
    int32_t Vlp[2];
    float extin[2];
    uint8_t fc[2];
    uint8_t vol[2];
    uint8_t filt[2];
    bool filt1[2];
    bool filt2[2];
    bool filt3[2];
    bool filtE[2];
    bool voice3off[2];
    bool hp[2];
    bool bp[2];
    bool lp[2];
    bool enabled[2];

    double filterCurve6581;
    double filterRange6581;
    double filterCurve8580;
    bool   old6581caps;

    // Integrators
    int32_t vx[2][2];
    int32_t vc[2][2];
    uint32_t nVddt_Vw_2[2];
    uint16_t nVgt[2];
    uint16_t n_dac[2];

    /// External filter
    int32_t exVlp;
    int32_t exVhp;
    double ext_res;

    // Resampler
    double clockFrequency;
    double samplingFrequency;
    SamplingMethod method;
    // ZeroOrder
    int32_t zor_cachedSample;
    int32_t zor_outputValue;
    int zor_sampleOffset;
    //  PassThrough
    int pt_outputValue;
    // TwoPassSinc
    int tp_sampleIndex[2];
    int tp_sampleOffset[2];
    int32_t tp_outputValue[2];
};


} // namespace reSIDfp

#endif
