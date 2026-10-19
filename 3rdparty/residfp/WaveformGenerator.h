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

#ifndef WAVEFORMGENERATOR_H
#define WAVEFORMGENERATOR_H

#include "siddefs-fp.h"
#include "array.h"

// print SR debugging info
//#define TRACE 1

#ifdef TRACE
#  include <iostream>
#endif

namespace reSIDfp
{

/**
 * A 24 bit accumulator is the basis for waveform generation.
 * FREQ is added to the lower 16 bits of the accumulator each cycle.
 * The accumulator is set to zero when TEST is set, and starts counting
 * when TEST is cleared.
 *
 * Waveforms are generated as follows:
 *
 * - No waveform:
 * When no waveform is selected, the DAC input is floating.
 *
 *
 * - Triangle:
 * The upper 12 bits of the accumulator are used.
 * The MSB is used to create the falling edge of the triangle by inverting
 * the lower 11 bits. The MSB is thrown away and the lower 11 bits are
 * left-shifted (half the resolution, full amplitude).
 * Ring modulation substitutes the MSB with MSB EOR NOT sync_source MSB.
 *
 *
 * - Sawtooth:
 * The output is identical to the upper 12 bits of the accumulator.
 *
 *
 * - Pulse:
 * The upper 12 bits of the accumulator are used.
 * These bits are compared to the pulse width register by a 12 bit digital
 * comparator; output is either all one or all zero bits.
 * The pulse setting is delayed one cycle after the compare.
 * The test bit, when set to one, holds the pulse waveform output at 0xfff
 * regardless of the pulse width setting.
 *
 *
 * - Noise:
 * The noise output is taken from intermediate bits of a 23-bit shift register
 * which is clocked by bit 19 of the accumulator.
 * The shift is delayed 2 cycles after bit 19 is set high.
 *
 * Operation: Calculate EOR result, shift register, set bit 0 = result.
 *
 *                    reset  +--------------------------------------------+
 *                      |    |                                            |
 *               test--OR-->EOR<--+                                       |
 *                      |         |                                       |
 *                      2 2 2 1 1 1 1 1 1 1 1 1 1                         |
 *     Register bits:   2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 <---+
 *                          |   |       |     |   |       |     |   |
 *     Waveform bits:       1   1       9     8   7       6     5   4
 *                          1   0
 *
 * The low 4 waveform bits are zero (grounded).
 */
class WaveformGenerator
{
    friend class State;

private:
    rc_matrix_t model_wave;
    rc_matrix_t model_pulldown;

    int16_t* wave = nullptr;
    int16_t* pulldown = nullptr;

    // PWout = (PWn/40.95)%
    uint32_t pw = 0;

    uint32_t shift_register = 0;

    /// Shift register is latched when transitioning to shift phase 1.
    uint32_t shift_latch = 0;

    /// Emulation of pipeline causing bit 19 to clock the shift register.
    int shift_pipeline = 0;

    uint32_t ring_msb_mask = 0;
    uint32_t no_noise = 0;
    uint32_t noise_output = 0;
    uint32_t no_noise_or_noise_output = 0;
    uint32_t no_pulse = 0;
    uint32_t pulse_output = 0;

    uint32_t waveform_output = 0;

    /// Current accumulator value.
    uint32_t accumulator = 0x555555; // Accumulator's even bits are high on powerup

    // Fout = (Fn*Fclk/16777216)Hz
    uint32_t freq = 0;

    /// 8580 tri/saw pipeline
    uint32_t tri_saw_pipeline = 0x555;

    /// The OSC3 value
    uint32_t osc3 = 0;

    /// Remaining time to fully reset shift register.
    unsigned int shift_register_reset = 0;

    // The wave signal TTL when no waveform is selected.
    unsigned int floating_output_ttl = 0;

    /// The control register right-shifted 4 bits; used for output function table lookup.
    uint8_t waveform = 0;

    /// The control register bits. Gate is handled by EnvelopeGenerator.
    //@{
    bool test = false;
    bool sync = false;
    //@}

    /// Test bit is latched at phi2 for the noise XOR.
    bool test_or_reset;

    /// Tell whether the accumulator MSB was set high on this cycle.
    bool msb_rising = false;

    bool is6581; //-V730_NOINIT this is initialized in the SID constructor

    /// The other two waveform generators, for syncing and ring-mod.
    //@{
    const WaveformGenerator* prevVoice;
    WaveformGenerator* nextVoice;
    //@}

private:
    void shift_phase2(uint8_t waveform_old, uint8_t waveform_new);

    void write_shift_register();

    void set_noise_output();

    void set_no_noise_or_noise_output();

    void waveBitfade();

    void shiftregBitfade();

    inline void setWave() { wave = (*model_wave)[waveform & 0x3]; }

    inline void setPulldown()
    {
        // We assume tha combinations including noise
        // behave the same as without
        switch (waveform & 0x7)
        {
        case 3:
            pulldown = (*model_pulldown)[0];
            break;
        case 4:
            pulldown = (waveform & 0x8) ? (*model_pulldown)[4] : nullptr;
            break;
        case 5:
            pulldown = (*model_pulldown)[1];
            break;
        case 6:
            pulldown = (*model_pulldown)[2];
            break;
        case 7:
            pulldown = (*model_pulldown)[3];
            break;
        default:
            pulldown = nullptr;
        }
    }

public:
    void setWaveformModels(rc_matrix_t models);
    void setPulldownModels(rc_matrix_t models);

    void setOtherWaveforms(const WaveformGenerator* prev, WaveformGenerator* next)
    {
        prevVoice = prev;
        nextVoice = next;
    }

    /**
     * Set the chip model.
     * Must be called before any operation.
     *
     * @param is6581 true if MOS6581, false if CSG8580
     */
    void setModel(bool new_is6581) { is6581 = new_is6581; }

    /**
     * SID clocking.
     */
    void clock();

    /**
     * Synchronize oscillators.
     * This must be done after all the oscillators have been clock()'ed,
     * so that they are in the same state.
     */
    void synchronize() const;

    /**
     * Write FREQ LO register.
     *
     * @param freq_lo low 8 bits of frequency
     */
    void writeFREQ_LO(uint8_t freq_lo) { freq = (freq & 0xff00) | (freq_lo & 0xff); }

    /**
     * Write FREQ HI register.
     *
     * @param freq_hi high 8 bits of frequency
     */
    void writeFREQ_HI(uint8_t freq_hi) { freq = (freq_hi << 8 & 0xff00) | (freq & 0xff); }

    /**
     * Write PW LO register.
     *
     * @param pw_lo low 8 bits of pulse width
     */
    void writePW_LO(uint8_t pw_lo) { pw = (pw & 0xf00) | (pw_lo & 0x0ff); }

    /**
     * Write PW HI register.
     *
     * @param pw_hi high 8 bits of pulse width
     */
    void writePW_HI(uint8_t pw_hi) { pw = (pw_hi << 8 & 0xf00) | (pw & 0x0ff); }

    /**
     * Write CONTROL REGISTER register.
     *
     * @param control control register value
     */
    void writeCONTROL_REG(uint8_t control);

    /**
     * SID reset.
     */
    void reset();

    /**
     * 12-bit waveform output.
     *
     * @return the waveform generator digital output
     */
    uint32_t output();

    /**
     * Read OSC3 value.
     */
    uint8_t readOSC() const { return static_cast<uint8_t>(osc3 >> 4); }

    /**
     * Read accumulator value.
     */
    uint32_t readAccumulator() const { return accumulator; }

    /**
     * Read freq value.
     */
    uint32_t readFreq() const { return freq; }

    /**
     * Read test value.
     */
    bool readTest() const { return test; }

    /**
     * Read sync value from following voice.
     */
    bool readFollowingVoiceSync() const { return nextVoice->sync; }
};

} // namespace reSIDfp

#if RESIDFP_INLINING || defined(WAVEFORMGENERATOR_CPP)

namespace reSIDfp
{

RESIDFP_INLINE
void WaveformGenerator::clock()
{
    if (unlikely(test))
    {
        if (unlikely(shift_register_reset != 0) && unlikely(--shift_register_reset == 0))
        {
#ifdef TRACE
            std::cout << "shiftregBitfade" << std::endl;
#endif
            shiftregBitfade();
            shift_latch = shift_register;

            // New noise waveform output.
            set_noise_output();
        }

        // Latch the test bit value for shift phase 2.
        test_or_reset = true;

        // The test bit sets pulse high.
        pulse_output = 0xfff;
    }
    else
    {
        // Calculate new accumulator value;
        const uint32_t accumulator_old = accumulator;
        accumulator = (accumulator + freq) & 0xffffff;

        // Check which bit have changed from low to high
        const uint32_t accumulator_bits_set = ~accumulator_old & accumulator;

        // Check whether the MSB is set high. This is used for synchronization.
        msb_rising = (accumulator_bits_set & 0x800000) != 0;

        // Shift noise register once for each time accumulator bit 19 is set high.
        // The shift is delayed 2 cycles.
        if (unlikely((accumulator_bits_set & 0x080000) != 0))
        {
            // Pipeline: Detect rising bit, shift phase 1, shift phase 2.
            shift_pipeline = 2;
        }
        else if (unlikely(shift_pipeline != 0))
        {
            switch (--shift_pipeline)
            {
            case 0:
#ifdef TRACE
                std::cout << "shift phase 2" << std::endl;
#endif
                shift_phase2(waveform, waveform);
                break;
            case 1:
#ifdef TRACE
                std::cout << "shift phase 1" << std::endl;
#endif
                // Start shift phase 1.
                test_or_reset = false;
                shift_latch = shift_register;
                break;
            }
        }
    }
}

RESIDFP_INLINE
uint32_t WaveformGenerator::output()
{
    // Set output value.
    if (likely(waveform != 0))
    {
        const uint32_t ix = (accumulator ^ (~prevVoice->accumulator & ring_msb_mask)) >> 12;

        // The bit masks no_pulse and no_noise are used to achieve branch-free
        // calculation of the output value.
        waveform_output = wave[ix] & (no_pulse | pulse_output) & no_noise_or_noise_output;
        if (pulldown != nullptr)
            waveform_output = pulldown[waveform_output];

        // Triangle/Sawtooth output is delayed half cycle on 8580.
        // This will appear as a one cycle delay on OSC3 as it is latched
        // in the first phase of the clock.
        if (!is6581 && (waveform & 3))
        {
            osc3 = tri_saw_pipeline & (no_pulse | pulse_output) & no_noise_or_noise_output;
            if (pulldown != nullptr)
                osc3 = pulldown[osc3];
            tri_saw_pipeline = wave[ix];
        }
        else
        {
            osc3 = waveform_output;
        }

        // In the 6581 the top bit of the accumulator may be driven low by combined waveforms
        // when the sawtooth is selected
        if (is6581 && (waveform & 0x2) && ((waveform_output & 0x800) == 0))
        {
            // On some chips it doesn't happen with P+S
            msb_rising = false;
            accumulator &= 0x7fffff;
        }

        write_shift_register();
    }
    else
    {
        // Age floating DAC input.
        if (likely(floating_output_ttl != 0) && unlikely(--floating_output_ttl == 0))
        {
            waveBitfade();
        }
    }

    // The pulse level is defined as (accumulator >> 12) >= pw ? 0xfff : 0x000.
    // The expression -((accumulator >> 12) >= pw) & 0xfff yields the same
    // results without any branching (and thus without any pipeline stalls).
    // NB! This expression relies on that the result of a boolean expression
    // is either 0 or 1, and furthermore requires two's complement integer.
    // A few more cycles may be saved by storing the pulse width left shifted
    // 12 bits, and dropping the and with 0xfff (this is valid since pulse is
    // used as a bit mask on 12 bit values), yielding the expression
    // -(accumulator >= pw24). However this only results in negligible savings.

    // The result of the pulse width compare is delayed one cycle.
    // Push next pulse level into pulse level pipeline.
    pulse_output = ((accumulator >> 12) >= pw) ? 0xfff : 0x000;

    return waveform_output;
}

} // namespace reSIDfp

#endif

#endif
