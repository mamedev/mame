/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2025 Leandro Nini <drfiemost@users.sourceforge.net>
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

#define WAVEFORMGENERATOR_CPP

#include "WaveformGenerator.h"

namespace reSIDfp
{

/**
 * Number of cycles after which the waveform output fades to 0 when setting
 * the waveform register to 0.
 * Values measured on warm chips (6581R3/R4 and 8580R5)
 * checking OSC3.
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represent
 * only the big difference between the old and new models.
 *
 * See [VICE Bug #290](http://sourceforge.net/p/vice-emu/bugs/290/)
 * and [VICE Bug #1128](http://sourceforge.net/p/vice-emu/bugs/1128/)
 */
// ~95ms
constexpr unsigned int FLOATING_OUTPUT_TTL_6581R3  =   54000;
constexpr unsigned int FLOATING_OUTPUT_FADE_6581R3 =    1400;
// ~1s
constexpr unsigned int FLOATING_OUTPUT_TTL_6581R4  = 1000000;
// ~1s
constexpr unsigned int FLOATING_OUTPUT_TTL_8580R5  =  800000;
constexpr unsigned int FLOATING_OUTPUT_FADE_8580R5 =   50000;

/**
 * Number of cycles after which the shift register is reset
 * when the test bit is set.
 * Values measured on warm chips (6581R3/R4 and 8580R5)
 * checking OSC3.
 * Times vary wildly with temperature and may differ
 * from chip to chip so the numbers here represent
 * only the big difference between the old and new models.
 */
// ~210ms
constexpr unsigned int SHIFT_REGISTER_RESET_6581R3 =   50000;
constexpr unsigned int SHIFT_REGISTER_FADE_6581R3  =   15000;
// ~2.15s
constexpr unsigned int SHIFT_REGISTER_RESET_6581R4 = 2150000;
// ~2.8s
constexpr unsigned int SHIFT_REGISTER_RESET_8580R5 =  986000;
constexpr unsigned int SHIFT_REGISTER_FADE_8580R5  =  314300;

constexpr unsigned int shift_mask =
    ~(
        (1u <<  2) |  // Bit 20
        (1u <<  4) |  // Bit 18
        (1u <<  8) |  // Bit 14
        (1u << 11) |  // Bit 11
        (1u << 13) |  // Bit  9
        (1u << 17) |  // Bit  5
        (1u << 20) |  // Bit  2
        (1u << 22)    // Bit  0
    );

/*
 * This is what happens when the lfsr is clocked:
 *
 * cycle 0: bit 19 of the accumulator goes from low to high, the noise register acts normally,
 *          the output may pulldown a bit;
 *
 * cycle 1: first phase of the shift, the bits are interconnected and the output of each bit
 *          is latched into the following. The output may overwrite the latched value.
 *
 * cycle 2: second phase of the shift, the latched value becomes active in the first
 *          half of the clock and from the second half the register returns to normal operation.
 *
 * When the test or reset lines are active the first phase is executed at every cyle
 * until the signal is released triggering the second phase.
 *
 *      |       |    bit n     |   bit n+1
 *      | bit19 | latch output | latch output
 * -----+-------+--------------+--------------
 * phi1 |   0   |   A <-> A    |   B <-> B
 * phi2 |   0   |   A <-> A    |   B <-> B
 * -----+-------+--------------+--------------
 * phi1 |   1   |   A <-> A    |   B <-> B      <- bit19 raises
 * phi2 |   1   |   A <-> A    |   B <-> B
 * -----+-------+--------------+--------------
 * phi1 |   1   |   X     A  --|-> A     B      <- shift phase 1
 * phi2 |   1   |   X     A  --|-> A     B
 * -----+-------+--------------+--------------
 * phi1 |   1   |   X --> X    |   A --> A      <- shift phase 2
 * phi2 |   1   |   X <-> X    |   A <-> A
 *
 *
 * Normal cycles
 * -------------
 * Normally, when noise is selected along with another waveform,
 * c1 and c2 are closed and the output bits pull down the corresponding
 * shift register bits.
 * 
 *        noi_out_x             noi_out_x+1
 *          ^                     ^
 *          |                     |
 *          +-------------+       +-------------+
 *          |             |       |             |
 *          +---o<|---+   |       +---o<|---+   |
 *          |         |   |       |         |   |
 *       c2 |      c1 |   |    c2 |      c1 |   |
 *          |         |   |       |         |   |
 *  >---/---+---|>o---+   +---/---+---|>o---+   +---/--->
 *      LC                    LC                    LC
 *
 *
 * Shift phase 1
 * -------------
 * During shift phase 1 c1 and c2 are open, the SR bits are floating
 * and will be driven by the output of combined waveforms,
 * or slowly turn high.
 *
 *        noi_out_x             noi_out_x+1
 *          ^                     ^
 *          |                     |
 *          +-------------+       +-------------+
 *          |             |       |             |
 *          +---o<|---+   |       +---o<|---+   |
 *          |         |   |       |         |   |
 *       c2 /      c1 /   |    c2 /      c1 /   |
 *          |         |   |       |         |   |
 *  >-------+---|>o---+   +-------+---|>o---+   +------->
 *      LC                    LC                    LC
 *
 *
 * Shift phase 2 (phi1)
 * --------------------
 * During the first half cycle of shift phase 2 c1 is closed
 * so the value from of noi_out_x-1 enters the bit.
 *
 *        noi_out_x             noi_out_x+1
 *          ^                     ^
 *          |                     |
 *          +-------------+       +-------------+
 *          |             |       |             |
 *          +---o<|---+   |       +---o<|---+   |
 *          |         |   |       |         |   |
 *       c2 /      c1 |   |    c2 /      c1 |   |
 *          |         |   |       |         |   |
 *  >---/---+---|>o---+   +---/---+---|>o---+   +---/--->
 *      LC                    LC                    LC
 *
 *
 * Shift phase 2 (phi2)
 * --------------------
 * On the second half of shift phase 2 c2 closes and
 * we're back to normal cycles.
 */

inline bool do_writeback(uint8_t waveform_old, uint8_t waveform_new, bool is6581)
{
    // no writeback without combined waveforms

    if (waveform_old <= 8)
        // fixes SID/noisewriteback/noise_writeback_test2-{old,new}
        return false;

    if (waveform_new < 8)
        return false;

    if ((waveform_new == 8)
        // breaks noise_writeback_check_F_to_8_old
        // but fixes simple and scan
        && (waveform_old != 0xf))
    {
        // fixes
        // noise_writeback_check_9_to_8_old
        // noise_writeback_check_A_to_8_old
        // noise_writeback_check_B_to_8_old
        // noise_writeback_check_D_to_8_old
        // noise_writeback_check_E_to_8_old
        // noise_writeback_check_F_to_8_old
        // noise_writeback_check_9_to_8_new
        // noise_writeback_check_A_to_8_new
        // noise_writeback_check_D_to_8_new
        // noise_writeback_check_E_to_8_new
        // noise_writeback_test1-{old,new}
        return false;
    }

    // What's happening here?
    if (is6581 &&
            ((((waveform_old & 0x3) == 0x1) && ((waveform_new & 0x3) == 0x2))
            || (((waveform_old & 0x3) == 0x2) && ((waveform_new & 0x3) == 0x1))))
    {
        // fixes
        // noise_writeback_check_9_to_A_old
        // noise_writeback_check_9_to_E_old
        // noise_writeback_check_A_to_9_old
        // noise_writeback_check_A_to_D_old
        // noise_writeback_check_D_to_A_old
        // noise_writeback_check_E_to_9_old
        return false;
    }
    if (waveform_old == 0xc)
    {
        // fixes
        // noise_writeback_check_C_to_A_new
        return false;
    }
    if (waveform_new == 0xc)
    {
        // fixes
        // noise_writeback_check_9_to_C_old
        // noise_writeback_check_A_to_C_old
        return false;
    }

    // ok do the writeback
    return true;
}

inline unsigned int get_noise_writeback(unsigned int waveform_output)
{
  return
    ((waveform_output & (1u << 11)) >>  9) |  // Bit 11 -> bit 20
    ((waveform_output & (1u << 10)) >>  6) |  // Bit 10 -> bit 18
    ((waveform_output & (1u <<  9)) >>  1) |  // Bit  9 -> bit 14
    ((waveform_output & (1u <<  8)) <<  3) |  // Bit  8 -> bit 11
    ((waveform_output & (1u <<  7)) <<  6) |  // Bit  7 -> bit  9
    ((waveform_output & (1u <<  6)) << 11) |  // Bit  6 -> bit  5
    ((waveform_output & (1u <<  5)) << 15) |  // Bit  5 -> bit  2
    ((waveform_output & (1u <<  4)) << 18);   // Bit  4 -> bit  0
}

/*
 * Perform the actual shifting, moving the latched value into following bits.
 * The XORing for bit0 is done in this cycle using the test bit latched during
 * the previous phi2 cycle.
 */
void WaveformGenerator::shift_phase2(uint8_t waveform_old, uint8_t waveform_new)
{
    if (do_writeback(waveform_old, waveform_new, is6581))
    {
        // if noise is combined with another waveform the output drives the SR bits
        shift_latch = (shift_register & shift_mask) | get_noise_writeback(waveform_output);
    }

    // bit0 = (bit22 | test | reset) ^ bit17 = 1 ^ bit17 = ~bit17
    const unsigned int bit22 = ((test_or_reset ? 1 : 0) | shift_latch) << 22;
    const unsigned int bit0 = (bit22 ^ (shift_latch << 17)) & (1 << 22);

    shift_register = (shift_latch >> 1) | bit0;
#ifdef TRACE
    std::cout << std::hex << shift_latch << " -> " << shift_register << std::endl;
#endif
    set_noise_output();
}

void WaveformGenerator::write_shift_register()
{
    if (unlikely(waveform > 0x8))
    {
#if 0
        // FIXME this breaks SID/wf12nsr/wf12nsr
        if (waveform == 0xc)
            // fixes
            // noise_writeback_check_8_to_C_old
            // noise_writeback_check_9_to_C_old
            // noise_writeback_check_A_to_C_old
            // noise_writeback_check_C_to_C_old
            return;
#endif

        // Write changes to the shift register output caused by combined waveforms
        // back into the shift register.
        if (likely(shift_pipeline != 1) && !test)
        {
#ifdef TRACE
            std::cout << "write shift_register" << std::endl;
#endif
            // the output pulls down the SR bits
            shift_register = shift_register & (shift_mask | get_noise_writeback(waveform_output));
            noise_output &= waveform_output;
        }
        else
        {
#ifdef TRACE
            std::cout << "write shift_latch" << std::endl;
#endif
            // shift phase 1: the output drives the SR bits
            noise_output = waveform_output;
        }

        set_no_noise_or_noise_output();
    }
}

void WaveformGenerator::set_noise_output()
{
    noise_output =
        ((shift_register & (1u <<  2)) <<  9) |  // Bit 20 -> bit 11
        ((shift_register & (1u <<  4)) <<  6) |  // Bit 18 -> bit 10
        ((shift_register & (1u <<  8)) <<  1) |  // Bit 14 -> bit  9
        ((shift_register & (1u << 11)) >>  3) |  // Bit 11 -> bit  8
        ((shift_register & (1u << 13)) >>  6) |  // Bit  9 -> bit  7
        ((shift_register & (1u << 17)) >> 11) |  // Bit  5 -> bit  6
        ((shift_register & (1u << 20)) >> 15) |  // Bit  2 -> bit  5
        ((shift_register & (1u << 22)) >> 18);   // Bit  0 -> bit  4

    set_no_noise_or_noise_output();
}

void WaveformGenerator::setWaveformModels(rc_matrix_t models)
{
    model_wave = models;
}

void WaveformGenerator::setPulldownModels(rc_matrix_t models)
{
    model_pulldown = models;
}

void WaveformGenerator::synchronize() const
{
    // A special case occurs when a sync source is synced itself on the same
    // cycle as when its MSB is set high. In this case the destination will
    // not be synced. This has been verified by sampling OSC3.
    if (unlikely(msb_rising) && nextVoice->sync && !(sync && prevVoice->msb_rising))
    {
        nextVoice->accumulator = 0;
    }
}

void WaveformGenerator::set_no_noise_or_noise_output()
{
    no_noise_or_noise_output = no_noise | noise_output;
}

void WaveformGenerator::writeCONTROL_REG(uint8_t control)
{
    const uint8_t waveform_prev = waveform;
    const bool test_prev = test;

    waveform = (control >> 4) & 0x0f;
    test = (control & 0x08) != 0;
    sync = (control & 0x02) != 0;

    // Substitution of accumulator MSB when sawtooth = 0, ring_mod = 1.
    ring_msb_mask = ((~control >> 5) & (control >> 2) & 0x1) << 23;

    if (waveform != waveform_prev)
    {
        // Set up waveform tables
        setWave();
        setPulldown();

        // no_noise and no_pulse are used in set_waveform_output() as bitmasks to
        // only let the noise or pulse influence the output when the noise or pulse
        // waveforms are selected.
        no_noise = (waveform & 0x8) != 0 ? 0x000 : 0xfff;
        set_no_noise_or_noise_output();
        no_pulse = (waveform & 0x4) != 0 ? 0x000 : 0xfff;

        if (waveform == 0)
        {
            // Change to floating DAC input.
            // Reset fading time for floating DAC input.
            floating_output_ttl = is6581 ? FLOATING_OUTPUT_TTL_6581R3 : FLOATING_OUTPUT_TTL_8580R5;
        }
    }

    if (test != test_prev)
    {
        if (test)
        {
            // Reset accumulator.
            accumulator = 0;

            // Flush shift pipeline.
            shift_pipeline = 0;

            // Latch the shift register value.
            shift_latch = shift_register;
#ifdef TRACE
            std::cout << "shift phase 1 (test)" << std::endl;
#endif

            // Set reset time for shift register.
            shift_register_reset = is6581 ? SHIFT_REGISTER_RESET_6581R3 : SHIFT_REGISTER_RESET_8580R5;
        }
        else
        {
            // When the test bit is falling, the second phase of the shift is
            // completed by enabling SRAM write.
            shift_phase2(waveform_prev, waveform);
        }
    }
}

void WaveformGenerator::waveBitfade()
{
    waveform_output &= waveform_output >> 1;
    osc3 = waveform_output;
    if (waveform_output != 0)
        floating_output_ttl = is6581 ? FLOATING_OUTPUT_FADE_6581R3 : FLOATING_OUTPUT_FADE_8580R5;
}

void WaveformGenerator::shiftregBitfade()
{
    shift_register |= shift_register >> 1;
    shift_register |= 0x400000;
    if (shift_register != 0x7fffff)
        shift_register_reset = is6581 ? SHIFT_REGISTER_FADE_6581R3 : SHIFT_REGISTER_FADE_8580R5;
}

void WaveformGenerator::reset()
{
    // accumulator is not changed on reset
    freq = 0;
    pw = 0;

    msb_rising = false;

    waveform = 0;
    osc3 = 0;

    test = false;
    sync = false;

    wave = model_wave ? (*model_wave)[0] : nullptr;
    pulldown = nullptr;

    ring_msb_mask = 0;
    no_noise = 0xfff;
    no_pulse = 0xfff;
    pulse_output = 0xfff;

    shift_register_reset = 0;
    shift_register = 0x7fffff;
    // when reset is released the shift register is clocked once
    // so the lower bit is zeroed out
    // bit0 = (bit22 | test) ^ bit17 = 1 ^ 1 = 0
    test_or_reset = true;
    shift_latch = shift_register;
    shift_phase2(0, 0);

    shift_pipeline = 0;

    waveform_output = 0;
    floating_output_ttl = 0;
}

} // namespace reSIDfp
