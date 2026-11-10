/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2023 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef WAVEFORMCALCULATOR_h
#define WAVEFORMCALCULATOR_h

#include "array.h"

#include "residfp/residfp_defs.h"


namespace reSIDfp
{

/**
 * Combined waveform calculator for WaveformGenerator.
 * By combining waveforms, the bits of each waveform are effectively short
 * circuited, a zero bit in one waveform will result in a zero output bit,
 * thus the claim that the waveforms are AND'ed.
 * However, a zero bit in one waveform may also affect the neighboring bits
 * in the output.
 *
 * Example:
 *
 *                 1 1
 *     Bit #       1 0 9 8 7 6 5 4 3 2 1 0
 *                 -----------------------
 *     Sawtooth    0 0 0 1 1 1 1 1 1 0 0 0
 *
 *     Triangle    0 0 1 1 1 1 1 1 0 0 0 0
 *
 *     AND         0 0 0 1 1 1 1 1 0 0 0 0
 *
 *     Output      0 0 0 0 1 1 1 0 0 0 0 0
 *
 *
 * Re-vectorized die photographs reveal the mechanism behind this behavior.
 * Each waveform selector bit acts as a switch, which directly connects
 * internal outputs into the waveform DAC inputs as follows:
 *
 * - Noise outputs the shift register bits to DAC inputs as described above.
 *   Each output is also used as input to the next bit when the shift register
 *   is shifted. Lower four bits are grounded.
 * - Pulse connects a single line to all DAC inputs. The line is connected to
 *   either 5V (pulse on) or 0V (pulse off) at bit 11, and ends at bit 0.
 * - Triangle connects the upper 11 bits of the (MSB EOR'ed) accumulator to the
 *   DAC inputs, so that DAC bit 0 = 0, DAC bit n = accumulator bit n - 1.
 * - Sawtooth connects the upper 12 bits of the accumulator to the DAC inputs,
 *   so that DAC bit n = accumulator bit n. Sawtooth blocks out the MSB from
 *   the EOR used to generate the triangle waveform.
 *
 * We can thus draw the following conclusions:
 *
 * - The shift register may be written to by combined waveforms.
 * - The pulse waveform interconnects all bits in combined waveforms via the
 *   pulse line.
 * - The combination of triangle and sawtooth interconnects neighboring bits
 *   of the sawtooth waveform.
 *
 * Also in the 6581 the MSB of the oscillator, used as input for the
 * triangle xor logic and the pulse adder's last bit, is connected directly
 * to the waveform selector, while in the 8580 it is latched at sid_clk2
 * before being forwarded to the selector. Thus in the 6581 if the sawtooth MSB
 * is pulled down it might affect the oscillator's adder
 * driving the top bit low.
 *
 */
class WaveformCalculator
{
private:
    rc_matrix_t wftable;

private:
    WaveformCalculator();

public:
    /**
     * Get the singleton instance.
     */
    static WaveformCalculator* getInstance();

    /**
     * Get the waveform table for use by WaveformGenerator.
     *
     * @return Waveform table
     */
    rc_matrix_t getWaveTable() { return wftable; }

    /**
     * Build pulldown table for use by WaveformGenerator.
     *
     * @param model Chip model to use
     * @param cws strength of combined waveforms
     * @return Pulldown table
     */
    static rc_matrix_t buildPulldownTable(ChipModel model, CombinedWaveforms cws);
};

} // namespace reSIDfp

#endif
