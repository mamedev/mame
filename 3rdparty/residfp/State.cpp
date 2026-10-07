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

#include "State.h"

#include "SID.h"
#include "Filter6581.h"
#include "Filter8580.h"
#include "Filter8580.h"
#include "resample/ZeroOrderResampler.h"
#include "resample/PassThrough.h"

#include <cstring>

namespace reSIDfp
{

int State::saveState(SID &s, char* buffer, int size)
{
    State state;

    for (int i = 0; i < 3; i++)
    {
        WaveformGenerator* const wave = s.voice[i].wave();
        state.pw[i] = wave->pw;
        state.shift_register[i] = wave->shift_register;
        state.shift_latch[i] = wave->shift_latch;
        state.ring_msb_mask[i] = wave->ring_msb_mask;
        state.no_noise[i] = wave->no_noise;
        state.noise_output[i] = wave->noise_output;
        state.no_noise_or_noise_output[i] = wave->no_noise_or_noise_output;
        state.no_pulse[i] = wave->no_pulse;
        state.pulse_output[i] = wave->pulse_output;
        state.waveform_output[i] = wave->waveform_output;
        state.accumulator[i] = wave->accumulator;
        state.freq[i] = wave->freq;
        state.tri_saw_pipeline[i] = wave->tri_saw_pipeline;
        state.osc3[i] = wave->osc3;
        state.shift_pipeline[i] = wave->shift_pipeline;
        state.shift_register_reset[i] = wave->shift_register_reset;
        state.floating_output_ttl[i] = wave->floating_output_ttl;
        state.waveform[i] = wave->waveform;
        state.test[i] = wave->test;
        state.sync[i] = wave->sync;
        state.test_or_reset[i] = wave->test_or_reset;
        state.msb_rising[i] = wave->msb_rising;

        EnvelopeGenerator* const envelope = s.voice[i].envelope();
        state.lfsr[i] = envelope->lfsr;
        state.rate[i] = envelope->rate;
        state.exponential_counter[i] = envelope->exponential_counter;
        state.exponential_counter_period[i] = envelope->exponential_counter_period;
        state.new_exponential_counter_period[i] = envelope->new_exponential_counter_period;
        state.state_pipeline[i] = envelope->state_pipeline;
        state.envelope_pipeline[i] = envelope->envelope_pipeline;
        state.exponential_pipeline[i] = envelope->exponential_pipeline;
        state.env_state[i] = envelope->state;
        state.next_state[i] = envelope->next_state;
        state.counter_enabled[i] = envelope->counter_enabled;
        state.gate[i] = envelope->gate;
        state.resetLfsr[i] = envelope->resetLfsr;
        state.envelope_counter[i] = envelope->envelope_counter;
        state.attack[i] = envelope->attack;
        state.decay[i] = envelope->decay;
        state.sustain[i] = envelope->sustain;
        state.release[i] = envelope->release;
        state.env3[i] = envelope->env3;
    }

    state.dacLeakage = s.dacLeakage;
    state.bus_value = s.busValue;
    state.bus_value_ttl = s.busValueTtl;
    state.nextVoiceSync = s.nextVoiceSync;
    state.offset_6581 = s.offset_6581;
    state.paddle_x = s.paddleX;
    state.paddle_y = s.paddleY;
    state.model = s.model;
    state.cws = s.cws;

    for (int i = 0; i < 2; i++)
    {
        Filter* f = (i == 0) ? static_cast<Filter*>(s.filter6581) : static_cast<Filter*>(s.filter8580);
        state.Vhp[i] = f->Vhp;
        state.Vbp[i] = f->Vbp;
        state.Vlp[i] = f->Vlp;
        state.extin[i] = f->extin;
        state.fc[i] = f->fc;
        state.filt1[i] = f->filt1;
        state.filt2[i] = f->filt2;
        state.filt3[i] = f->filt3;
        state.filtE[i] = f->filtE;
        state.voice3off[i] = f->voice3off;
        state.hp[i] = f->hp;
        state.bp[i] = f->bp;
        state.lp[i] = f->lp;
        state.vol[i] = f->vol;
        state.enabled[i] = f->enabled;
        state.filt[i] = f->filt;
    }
    state.filterCurve6581 = s.p->filterCurve6581;
    state.filterRange6581 = s.p->filterRange6581;
    state.filterCurve8580 = s.p->filterCurve8580;
    state.old6581caps = s.p->old6581caps;

    state.vx[0][0] = s.filter6581->hpIntegrator.vx;
    state.vx[0][1] = s.filter6581->bpIntegrator.vx;
    state.vx[1][0] = s.filter8580->hpIntegrator.vx;
    state.vx[1][1] = s.filter8580->bpIntegrator.vx;
    state.vc[0][0] = s.filter6581->hpIntegrator.vc;
    state.vc[0][1] = s.filter6581->bpIntegrator.vc;
    state.vc[1][0] = s.filter8580->hpIntegrator.vc;
    state.vc[1][1] = s.filter8580->bpIntegrator.vc;
    state.nVddt_Vw_2[0] = s.filter6581->hpIntegrator.nVddt_Vw_2;
    state.nVddt_Vw_2[1] = s.filter6581->bpIntegrator.nVddt_Vw_2;
    state.nVgt[0] = s.filter8580->hpIntegrator.nVgt;
    state.nVgt[1] = s.filter8580->bpIntegrator.nVgt;
    state.n_dac[0] = s.filter8580->hpIntegrator.n_dac;
    state.n_dac[1] = s.filter8580->bpIntegrator.n_dac;

    state.exVlp = s.externalFilter.Vlp;
    state.exVhp = s.externalFilter.Vhp;
    state.ext_res = s.externalFilter.m_ext_res;
    state.clockFrequency = s.externalFilter.m_frequency;

    state.method = s.p->method;
    state.samplingFrequency = s.p->samplingFrequency;

    switch (s.p->method)
    {
    case DECIMATE: {
        ZeroOrderResampler *zor = static_cast<ZeroOrderResampler*>(s.resampler.get());
        state.zor_cachedSample = zor->cachedSample;
        state.zor_sampleOffset = zor->sampleOffset;
        state.zor_outputValue = zor->outputValue;
        } break;
    case RESAMPLE: {
        TwoPassSincResampler *tp = static_cast<TwoPassSincResampler*>(s.resampler.get());
        for (int i=0; i<2; i++)
        {
            SincResampler *sr = (i == 0) ? tp->s1.get(): tp->s2.get();
            state.tp_sampleIndex[i] = sr->sampleIndex;
            state.tp_sampleOffset[i] = sr->sampleOffset;
            state.tp_outputValue[i] = sr->outputValue;
        }
        } break;
    case NONE: {
        PassThrough *pt = static_cast<PassThrough*>(s.resampler.get());
        state.pt_outputValue = pt->outputValue;
        } break;
    }

    int cnt = sizeof(reSIDfp::State);
    if (size < cnt)
        return 0;
    std::memcpy(buffer, &state, cnt);

    if (state.method == RESAMPLE)
    {
        TwoPassSincResampler *tp = static_cast<TwoPassSincResampler*>(s.resampler.get());
        for (int i=0; i<2; i++)
        {
            SincResampler *sr = (i == 0) ? tp->s1.get(): tp->s2.get();
            int spl = sizeof(sr->sample);
            if (size < cnt+spl)
                return 0;
            std::memcpy(buffer+cnt, sr->sample, spl);
            cnt += spl;
        }
    }

    return cnt;
}


void State::restoreState(SID &s, char* buffer, int size)
{
    int cnt = sizeof(reSIDfp::State);
    if (size < cnt)
        return;
    State state;
    std::memcpy(&state, buffer, cnt);

    s.dacLeakage = state.dacLeakage;
    s.busValue = state.bus_value;
    s.busValueTtl = state.bus_value_ttl;
    s.nextVoiceSync = state.nextVoiceSync;
    s.offset_6581 = state.offset_6581;
    s.paddleX = state.paddle_x;
    s.paddleY = state.paddle_y;
    s.model = state.model;
    s.setChipModel(s.model);
    s.cws = state.cws;
    s.setCombinedWaveforms(s.cws);

    for (int i = 0; i < 2; i++)
    {
        Filter* f = (i == 0) ? static_cast<Filter*>(s.filter6581) : static_cast<Filter*>(s.filter8580);
        f->Vhp = state.Vhp[i];
        f->Vbp = state.Vbp[i];
        f->Vlp = state.Vlp[i];
        f->extin = state.extin[i];
        f->fc = state.fc[i];
        f->filt1 = state.filt1[i];
        f->filt2 = state.filt2[i];
        f->filt3 = state.filt3[i];
        f->filtE = state.filtE[i];
        f->voice3off = state.voice3off[i];
        f->hp = state.hp[i];
        f->bp = state.bp[i];
        f->lp = state.lp[i];
        f->vol = state.vol[i];
        f->enabled = state.enabled[i];
        f->filt = state.filt[i];
        f->updateResonance((f->filt >> 4) & 0x0f);
        f->updateMixing();
    }
    s.setFilter6581Curve(state.filterCurve6581);
    s.setFilter6581Range(state.filterRange6581);
    s.setFilter8580Curve(state.filterCurve8580);
    s.enableOld6581caps(state.old6581caps);

    s.filter6581->hpIntegrator.vx = state.vx[0][0];
    s.filter6581->bpIntegrator.vx = state.vx[0][1];
    s.filter8580->hpIntegrator.vx = state.vx[1][0];
    s.filter8580->bpIntegrator.vx = state.vx[1][1];
    s.filter6581->hpIntegrator.vc = state.vc[0][0];
    s.filter6581->bpIntegrator.vc = state.vc[0][1];
    s.filter8580->hpIntegrator.vc = state.vc[1][0];
    s.filter8580->bpIntegrator.vc = state.vc[1][1];
    s.filter6581->hpIntegrator.nVddt_Vw_2 = state.nVddt_Vw_2[0];
    s.filter6581->bpIntegrator.nVddt_Vw_2 = state.nVddt_Vw_2[1];
    s.filter8580->hpIntegrator.nVgt = state.nVgt[0];
    s.filter8580->bpIntegrator.nVgt = state.nVgt[1];
    s.filter8580->hpIntegrator.n_dac = state.n_dac[0];
    s.filter8580->bpIntegrator.n_dac = state.n_dac[1];

    s.externalFilter.Vlp = state.exVlp;
    s.externalFilter.Vhp = state.exVhp;
    s.externalFilter.m_ext_res = state.ext_res;

    s.setSamplingParameters(state.clockFrequency, state.method, state.samplingFrequency);
    s.externalFilter.recalcParams();

    for (int i = 0; i < 3; i++)
    {
        WaveformGenerator* const wave = s.voice[i].wave();
        wave->pw = state.pw[i];
        wave->shift_register = state.shift_register[i];
        wave->shift_latch = state.shift_latch[i];
        wave->ring_msb_mask = state.ring_msb_mask[i];
        wave->no_noise = state.no_noise[i];
        wave->noise_output = state.noise_output[i];
        wave->no_noise_or_noise_output = state.no_noise_or_noise_output[i];
        wave->no_pulse = state.no_pulse[i];
        wave->pulse_output = state.pulse_output[i];
        wave->waveform_output = state.waveform_output[i];
        wave->accumulator = state.accumulator[i];
        wave->freq = state.freq[i];
        wave->tri_saw_pipeline = state.tri_saw_pipeline[i];
        wave->osc3 = state.osc3[i];
        wave->shift_pipeline = state.shift_pipeline[i];
        wave->shift_register_reset = state.shift_register_reset[i];
        wave->floating_output_ttl = state.floating_output_ttl[i];
        wave->waveform = state.waveform[i];
        wave->test = state.test[i];
        wave->sync = state.sync[i];
        wave->test_or_reset = state.test_or_reset[i];
        wave->msb_rising = state.msb_rising[i];

        wave->setWave();
        wave->setPulldown();

        EnvelopeGenerator* const envelope = s.voice[i].envelope();
        envelope->lfsr = state.lfsr[i];
        envelope->rate = state.rate[i];
        envelope->exponential_counter = state.exponential_counter[i];
        envelope->exponential_counter_period = state.exponential_counter_period[i];
        envelope->new_exponential_counter_period = state.new_exponential_counter_period[i];
        envelope->state_pipeline = state.state_pipeline[i];
        envelope->envelope_pipeline = state.envelope_pipeline[i];
        envelope->exponential_pipeline = state.exponential_pipeline[i];
        envelope->state = state.env_state[i];
        envelope->next_state = state.next_state[i];
        envelope->counter_enabled = state.counter_enabled[i];
        envelope->gate = state.gate[i];
        envelope->resetLfsr = state.resetLfsr[i];
        envelope->envelope_counter = state.envelope_counter[i];
        envelope->attack = state.attack[i];
        envelope->decay = state.decay[i];
        envelope->sustain = state.sustain[i];
        envelope->release = state.release[i];
        envelope->env3 = state.env3[i];
    }

    switch (s.p->method)
    {
    case DECIMATE: {
        ZeroOrderResampler *zor = static_cast<ZeroOrderResampler*>(s.resampler.get());
        zor->cachedSample = state.zor_cachedSample;
        zor->sampleOffset = state.zor_sampleOffset;
        zor->outputValue = state.zor_outputValue;
        } break;
    case RESAMPLE: {
        TwoPassSincResampler *tp = static_cast<TwoPassSincResampler*>(s.resampler.get());
        for (int i=0; i<2; i++)
        {
            SincResampler *sr = (i == 0) ? tp->s1.get(): tp->s2.get();
            sr->sampleIndex = state.tp_sampleIndex[i];
            sr->sampleOffset = state.tp_sampleOffset[i];
            sr->outputValue = state.tp_outputValue[i];
        }
        } break;
    case NONE: {
        PassThrough *pt = static_cast<PassThrough*>(s.resampler.get());
        pt->outputValue = state.pt_outputValue;
        } break;
    }

    if (state.method == RESAMPLE)
    {
        TwoPassSincResampler *tp = static_cast<TwoPassSincResampler*>(s.resampler.get());
        for (int i=0; i<2; i++)
        {
            SincResampler *sr = (i == 0) ? tp->s1.get(): tp->s2.get();
            int spl = sizeof(sr->sample);
            if (size < cnt+spl)
                return;
            std::memcpy(sr->sample, buffer+cnt, spl);
            cnt += spl;
        }
    }
}

int State::size(SID &s)
{
    int cnt = sizeof(reSIDfp::State);
    if (s.p->method == RESAMPLE)
    {
        cnt += sizeof(SincResampler::sample) * 2;
    }
    return cnt;
}

} // namespace reSIDfp

