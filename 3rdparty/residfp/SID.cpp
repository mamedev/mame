/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
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

#define SID_CPP

#include "SID.h"

#include <algorithm>
#include <limits>

#include "array.h"
#include "Dac.h"
#include "Filter6581.h"
#include "Filter8580.h"
#include "WaveformCalculator.h"
#include "resample/TwoPassSincResampler.h"
#include "resample/ZeroOrderResampler.h"
#include "resample/PassThrough.h"

namespace reSIDfp
{

constexpr unsigned int ENV_DAC_BITS = 8;
constexpr unsigned int OSC_DAC_BITS = 12;

/**
 * The waveform D/A converter introduces a DC offset in the signal
 * to the envelope multiplying D/A converter. The "zero" level of
 * the waveform D/A converter can be found as follows:
 *
 * Measure the "zero" voltage of voice 3 on the SID audio output
 * pin, routing only voice 3 to the mixer ($d417 = $0b, $d418 =
 * $0f, all other registers zeroed).
 *
 * Then set the sustain level for voice 3 to maximum and search for
 * the waveform output value yielding the same voltage as found
 * above. This is done by trying out different waveform output
 * values until the correct value is found, e.g. with the following
 * program:
 *
 *        lda #$08
 *        sta $d412
 *        lda #$0b
 *        sta $d417
 *        lda #$0f
 *        sta $d418
 *        lda #$f0
 *        sta $d414
 *        lda #$21
 *        sta $d412
 *        lda #$01
 *        sta $d40e
 *
 *        ldx #$00
 *        lda #$38        ; Tweak this to find the "zero" level
 * l      cmp $d41b
 *        bne l
 *        stx $d40e        ; Stop frequency counter - freeze waveform output
 *        brk
 *
 * The waveform output range is 0x000 to 0xfff, so the "zero"
 * level should ideally have been 0x800. In the measured chip, the
 * waveform output "zero" level was found to be 0x380 (i.e. $d41b
 * = 0x38) at an audio output voltage of 5.94V.
 *
 * With knowledge of the mixer op-amp characteristics, further estimates
 * of waveform voltages can be obtained by sampling the EXT IN pin.
 * From EXT IN samples, the corresponding waveform output can be found by
 * using the model for the mixer.
 *
 * Such measurements have been done on a chip marked MOS 6581R4AR
 * 0687 14, and the following results have been obtained:
 * * The full range of one voice is approximately 1.5V.
 * * The "zero" level rides at approximately 5.0V.
 *
 *
 * zero-x did the measuring on the 8580 (https://sourceforge.net/p/vice-emu/bugs/1036/#c5b3):
 * When it sits on basic from powerup it's at 4.72
 * Run 1.prg and check the output pin level.
 * Then run 2.prg and adjust it until the output level is the same...
 * 0x94-0xA8 gives me the same 4.72 1.prg shows.
 * On another 8580 it's 0x90-0x9C
 * Third chip 0x94-0xA8
 * Fourth chip 0x90-0xA4
 * On the 8580 that plays digis the output is 4.66 and 0x93 is the only value to reach that.
 * To me that seems as regular 8580s have somewhat wide 0-level range,
 * whereas that digi-compatible 8580 has it very narrow.
 * On my 6581R4AR has 0x3A as the only value giving the same output level as 1.prg
 *
 * NOTE: this needs more in-depth analysis.
 * There is no evidence in the schematics for a DC offset in the 8580
 * and it would make digis too loud.
 */
//@{
constexpr unsigned int OFFSET_6581 = 0x380;
constexpr unsigned int OFFSET_8580 = 0x9c0;
//@}


/**
 * Bus value stays alive for some time after each operation.
 * Values differs between chip models, the timings used here
 * are taken from VICE [1].
 * See also the discussion "How do I reliably detect 6581/8580 sid?" on CSDb [2].
 *
 *   Results from real C64 (testprogs/SID/bitfade/delayfrq0.prg):
 *
 *   (new SID) (250469/8580R5) (250469/8580R5)
 *   delayfrq0    ~7a000        ~108000
 *
 *   (old SID) (250407/6581)
 *   delayfrq0    ~01d00
 *
 * [1]: http://sourceforge.net/p/vice-emu/patches/99/
 * [2]: http://noname.c64.org/csdb/forums/?roomid=11&topicid=29025&showallposts=1
 */
//@{
constexpr int BUS_TTL_6581 = 0x01d00;
constexpr int BUS_TTL_8580 = 0xa2000;
//@}

// Clamp parameter in [0,1] range
double clamp(double param)
{
#ifdef HAVE_CXX17
     return std::clamp(param, 0.0, 1.0);
#else
     return std::max(std::min(param, 1.0), 0.0);
#endif
}

SID::SID() :
    filter6581(new Filter6581()),
    filter8580(new Filter8580()),
    resampler(nullptr),
    offset_6581(OFFSET_6581),
    cws(AVERAGE),
    dacLeakage(1.0),
    p(new Params)
{
    voice[0].setOtherVoices(voice[2], voice[1]);
    voice[1].setOtherVoices(voice[0], voice[2]);
    voice[2].setOtherVoices(voice[1], voice[0]);

    setChipModel(CSG8580);
    reset();
}

SID::~SID()
{
    delete filter6581;
    delete filter8580;
    delete p;
}

void SID::setFilter6581Curve(double filterCurve)
{
    p->filterCurve6581 = clamp(filterCurve);
    filter6581->setFilterCurve(filterCurve);
}

void SID::setFilter6581Range(double adjustment)
{
    p->filterRange6581 = clamp(adjustment);
    filter6581->setFilterRange(adjustment);
}

void SID::setFilter8580Curve(double filterCurve)
{
    p->filterCurve8580 = clamp(filterCurve);
    filter8580->setFilterCurve(filterCurve);
}

void SID::enableFilter(bool enable)
{
    filter6581->enable(enable);
    filter8580->enable(enable);
}

void SID::enableOld6581caps(bool enable)
{
    p->old6581caps = enable;
    filter6581->enableOldCaps(enable);
}

void SID::voiceSync(bool sync)
{
    if (sync)
    {
        // Synchronize the 3 waveform generators.
        for (int i = 0; i < 3; i++)
        {
            voice[i].wave()->synchronize();
        }
    }

    // Calculate the time to next voice sync
    nextVoiceSync = std::numeric_limits<int>::max();

    for (int i = 0; i < 3; i++)
    {
        WaveformGenerator* const wave = voice[i].wave();
        const unsigned int freq = wave->readFreq();

        if (wave->readTest() || freq == 0 || !voice[i].wave()->readFollowingVoiceSync())
        {
            continue;
        }

        const unsigned int accumulator = wave->readAccumulator();
        const unsigned int thisVoiceSync = ((0x7fffff - accumulator) & 0xffffff) / freq + 1;

        if (thisVoiceSync < nextVoiceSync)
        {
            nextVoiceSync = thisVoiceSync;
        }
    }
}

void SID::setChipModel(ChipModel new_model)
{
    switch (new_model)
    {
    case MOS6581:
        filter = filter6581;
        scaleFactor = 3;
        modelTTL = BUS_TTL_6581;
        break;

    case CSG8580:
        filter = filter8580;
        scaleFactor = 5;
        modelTTL = BUS_TTL_8580;
        break;

    default:
        throw SIDError("Unknown chip type");
    }

    model = new_model;

    // calculate waveform-related tables
    rc_matrix_t wavetables = WaveformCalculator::getInstance()->getWaveTable();
    rc_matrix_t pulldowntables = WaveformCalculator::getInstance()->buildPulldownTable(model, cws);

    // calculate envelope DAC table
    {
        Dac dacBuilder(ENV_DAC_BITS);
        dacBuilder.kinkedDac(model, dacLeakage);

        for (unsigned int i = 0; i < (1 << ENV_DAC_BITS); i++)
        {
            envDAC[i] = static_cast<float>(dacBuilder.getOutput(i));
        }
    }

    // calculate oscillator DAC table
    const bool is6581 = model == MOS6581;

    {
        Dac dacBuilder(OSC_DAC_BITS);
        dacBuilder.kinkedDac(model, dacLeakage);

        const double offset = dacBuilder.getOutput(is6581 ? offset_6581 : 0x800, is6581);

        for (unsigned int i = 0; i < (1 << OSC_DAC_BITS); i++)
        {
            const double dacValue = dacBuilder.getOutput(i, is6581);
            oscDAC[i] = static_cast<float>(dacValue - offset);
        }
    }

    // set voice tables
    for (int i = 0; i < 3; i++)
    {
        voice[i].setEnvDAC(envDAC);
        voice[i].setWavDAC(oscDAC);
        voice[i].wave()->setModel(is6581);
        voice[i].wave()->setWaveformModels(wavetables);
        voice[i].wave()->setPulldownModels(pulldowntables);
    }

    filter->restart();
}

void SID::setCombinedWaveforms(CombinedWaveforms new_cws)
{
    switch (new_cws)
    {
    case AVERAGE:
    case WEAK:
    case STRONG:
        break;

    default:
        throw SIDError("Unknown combined waveforms type");
    }

    cws = new_cws;

    // rebuild waveform-related tables
    rc_matrix_t pulldowntables = WaveformCalculator::getInstance()->buildPulldownTable(model, cws);

    for (int i = 0; i < 3; i++)
    {
        voice[i].wave()->setPulldownModels(pulldowntables);
    }
}

void SID::reset()
{
    for (int i = 0; i < 3; i++)
    {
        voice[i].reset();
    }

    filter6581->reset();
    filter8580->reset();
    externalFilter.reset();

    if (resampler.get())
    {
        resampler->reset();
    }

    busValue = 0;
    busValueTtl = 0;
    voiceSync(false);
}

void SID::input(int value)
{
    filter6581->input(value);
    filter8580->input(value);
}

uint8_t SID::peek(int offset) const
{
    switch (offset)
    {
    case 0x19: // X value of paddle
        return paddleX;

    case 0x1a: // Y value of paddle
        return paddleY;

    case 0x1b: // Voice #3 waveform output
        return voice[2].wave()->readOSC();

    case 0x1c: // Voice #3 ADSR output
        return voice[2].envelope()->readENV();

    default:
        return busValue;
    }
}

uint8_t SID::read(int offset)
{
    switch (offset)
    {
    case 0x19: // X value of paddle
        busValue = paddleX;
        busValueTtl = modelTTL;
        break;

    case 0x1a: // Y value of paddle
        busValue = paddleY;
        busValueTtl = modelTTL;
        break;

    case 0x1b: // Voice #3 waveform output
        busValue = voice[2].wave()->readOSC();
        busValueTtl = modelTTL;
        break;

    case 0x1c: // Voice #3 ADSR output
        busValue = voice[2].envelope()->readENV();
        busValueTtl = modelTTL;
        break;

    default:
        // Reading from a write-only or non-existing register
        // makes the bus discharge faster.
        // Emulate this by halving the residual TTL.
        busValueTtl /= 2;
        break;
    }

    return busValue;
}

void SID::write(int offset, uint8_t value)
{
    busValue = value;
    busValueTtl = modelTTL;

    switch (offset)
    {
    case 0x00: // Voice #1 frequency (Low-byte)
        voice[0].wave()->writeFREQ_LO(value);
        break;

    case 0x01: // Voice #1 frequency (High-byte)
        voice[0].wave()->writeFREQ_HI(value);
        break;

    case 0x02: // Voice #1 pulse width (Low-byte)
        voice[0].wave()->writePW_LO(value);
        break;

    case 0x03: // Voice #1 pulse width (bits #8-#15)
        voice[0].wave()->writePW_HI(value);
        break;

    case 0x04: // Voice #1 control register
        voice[0].writeCONTROL_REG(value);
        break;

    case 0x05: // Voice #1 Attack and Decay length
        voice[0].envelope()->writeATTACK_DECAY(value);
        break;

    case 0x06: // Voice #1 Sustain volume and Release length
        voice[0].envelope()->writeSUSTAIN_RELEASE(value);
        break;

    case 0x07: // Voice #2 frequency (Low-byte)
        voice[1].wave()->writeFREQ_LO(value);
        break;

    case 0x08: // Voice #2 frequency (High-byte)
        voice[1].wave()->writeFREQ_HI(value);
        break;

    case 0x09: // Voice #2 pulse width (Low-byte)
        voice[1].wave()->writePW_LO(value);
        break;

    case 0x0a: // Voice #2 pulse width (bits #8-#15)
        voice[1].wave()->writePW_HI(value);
        break;

    case 0x0b: // Voice #2 control register
        voice[1].writeCONTROL_REG(value);
        break;

    case 0x0c: // Voice #2 Attack and Decay length
        voice[1].envelope()->writeATTACK_DECAY(value);
        break;

    case 0x0d: // Voice #2 Sustain volume and Release length
        voice[1].envelope()->writeSUSTAIN_RELEASE(value);
        break;

    case 0x0e: // Voice #3 frequency (Low-byte)
        voice[2].wave()->writeFREQ_LO(value);
        break;

    case 0x0f: // Voice #3 frequency (High-byte)
        voice[2].wave()->writeFREQ_HI(value);
        break;

    case 0x10: // Voice #3 pulse width (Low-byte)
        voice[2].wave()->writePW_LO(value);
        break;

    case 0x11: // Voice #3 pulse width (bits #8-#15)
        voice[2].wave()->writePW_HI(value);
        break;

    case 0x12: // Voice #3 control register
        voice[2].writeCONTROL_REG(value);
        break;

    case 0x13: // Voice #3 Attack and Decay length
        voice[2].envelope()->writeATTACK_DECAY(value);
        break;

    case 0x14: // Voice #3 Sustain volume and Release length
        voice[2].envelope()->writeSUSTAIN_RELEASE(value);
        break;

    case 0x15: // Filter cut off frequency (bits #0-#2)
        filter6581->writeFC_LO(value);
        filter8580->writeFC_LO(value);
        break;

    case 0x16: // Filter cut off frequency (bits #3-#10)
        filter6581->writeFC_HI(value);
        filter8580->writeFC_HI(value);
        break;

    case 0x17: // Filter control
        filter6581->writeRES_FILT(value);
        filter8580->writeRES_FILT(value);
        break;

    case 0x18: // Volume and filter modes
        filter6581->writeMODE_VOL(value);
        filter8580->writeMODE_VOL(value);
        break;

    default:
        break;
    }

    // Update voicesync just in case.
    voiceSync(false);
}

void SID::setSamplingParameters(double clockFrequency, SamplingMethod method, double samplingFrequency)
{
    externalFilter.setClockFrequency(clockFrequency);

    switch (method)
    {
    case DECIMATE:
        resampler.reset(new ZeroOrderResampler(clockFrequency, samplingFrequency));
        break;

    case RESAMPLE:
        resampler.reset(TwoPassSincResampler::create(clockFrequency, samplingFrequency));
        break;

    case NONE:
        resampler.reset(new PassThrough());
        break;

    default:
        throw SIDError("Unknown sampling method");
    }

    p->method = method;
    p->samplingFrequency = samplingFrequency;
}

void SID::clockDigital(unsigned int cycles)
{
    ageBusValue(cycles);

    while (cycles != 0)
    {
        int delta_t = std::min(nextVoiceSync, cycles);

        if (delta_t > 0)
        {
            for (int i = 0; i < delta_t; i++)
            {
                clockWaveGen();
                clockEnvGen();

                voice[0].wave()->output();
                voice[1].wave()->output();
                voice[2].wave()->output();
            }

            cycles -= delta_t;
            nextVoiceSync -= delta_t;
        }

        if (nextVoiceSync == 0)
        {
            voiceSync(true);
        }
    }
}

void SID::clockSilent(unsigned int cycles)
{
    ageBusValue(cycles);

    while (cycles != 0)
    {
        int delta_t = std::min(nextVoiceSync, cycles);

        if (delta_t > 0)
        {
            for (int i = 0; i < delta_t; i++)
            {
                // clock waveform generators (can affect OSC3)
                clockWaveGen();

                voice[0].wave()->output();
                voice[1].wave()->output();
                voice[2].wave()->output();

                // clock ENV3 only
                voice[2].envelope()->clock();
            }

            cycles -= delta_t;
            nextVoiceSync -= delta_t;
        }

        if (nextVoiceSync == 0)
        {
            voiceSync(true);
        }
    }
}

void SID::setPaddle(uint8_t x, uint8_t y)
{
    paddleX = x;
    paddleY = y;
}

void SID::setDacLeakage(double level)
{
    dacLeakage = clamp(level);
    setChipModel(model);
}

void SID::setOffset6581(double offset)
{
    // TODO determine a reasonable range
    offset_6581 = 0x380 + static_cast<unsigned int>((1. - clamp(offset)) * 0x200);
    setChipModel(model);
}

void SID::setDCBRes(double res)
{
    externalFilter.setExtResistance(clamp(res));
}

} // namespace reSIDfp
