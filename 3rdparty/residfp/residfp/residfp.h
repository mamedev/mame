/*
 * This file is part of residfp, a SID player engine.
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

#ifndef RESIDFP_H
#define RESIDFP_H

#include "residfp/residfp_defs.h"
#include "residfp/sidversion.h"

/**
 * \file residfp.h
 *
 * Public residfp API.
 */

namespace reSIDfp
{

class SID;

/**
 * Cycle exact SID emulation
 */
class RESIDFP_EXTERN residfp
{
private:
    SID &sid;

public:
    residfp();
    ~residfp();

    /**
     * Set chip model.
     *
     * @param model chip model to use
     * @return false on unrecognized model
     */
    bool setChipModel(ChipModel model);

    /**
     * Get currently emulated chip model.
     */
    ChipModel getChipModel() const;

    /**
     * Set combined waveforms strength.
     *
     * @param cws strength of combined waveforms
     * @return false on unrecognized strength value
     */
    bool setCombinedWaveforms(CombinedWaveforms cws);

    /**
     * SID reset.
     */
    void reset();

    /**
     * 16-bit input (EXT IN). Write 16-bit sample to audio input. NB! The caller
     * is responsible for keeping the value within 16 bits. Note that to mix in
     * an external audio signal the signal should be resampled to ~1MHz first.
     *
     * @param value input level to set
     */
    void input(int value);

    /**
     * Read registers without altering state.
     *
     * @param offset SID register to read
     * @return value read from chip
     *
     * @since 1.1
     */
    unsigned char peek(int offset) const;

    /**
     * Read registers.
     *
     * Reading a write only register returns the last char written to any SID register.
     * The individual bits in this value start to fade down towards zero after a few cycles.
     * All bits reach zero within approximately $2000 - $4000 cycles on the 6581
     * and dar longer on the 8580.
     * It has been claimed that this fading happens in an orderly fashion,
     * however sampling of write only registers reveals that this is not the case.
     * NOTE: This is not correctly modeled.
     * The actual use of write only registers has largely been made
     * in the belief that all SID registers are readable.
     * To support this belief the read would have to be done immediately
     * after a write to the same register (remember that an intermediate write
     * to another register would yield that value instead).
     * With this in mind we return the last value written to any SID register
     * for a number of cycles dependent on the chip model without modeling the bit fading.
     *
     * @param offset SID register to read
     * @return value read from chip
     */
    unsigned char read(int offset);

    /**
     * Write registers.
     *
     * @param offset chip register to write
     * @param value value to write
     */
    void write(int offset, unsigned char value);

    /**
     * Setting of SID sampling parameters.
     *
     * Use a clock freqency of 985248Hz for PAL C64, 1022730Hz for NTSC C64.
     * The default end of passband frequency is pass_freq = 0.9*sample_freq/2
     * for sample frequencies up to ~ 44.1kHz, and 20kHz for higher sample frequencies.
     *
     * For resampling, the ratio between the clock frequency and the sample frequency
     * is limited as follows: 125*clock_freq/sample_freq < 16384
     * E.g. provided a clock frequency of ~ 1MHz, the sample frequency can not be set
     * lower than ~ 8kHz. A lower sample frequency would make the resampling code
     * overfill its 16k sample ring buffer.
     *
     * The end of passband frequency is also limited: pass_freq <= 0.9*sample_freq/2
     *
     * E.g. for a 44.1kHz sampling rate the end of passband frequency
     * is limited to slightly below 20kHz.
     * This constraint ensures that the FIR table is not overfilled.
     *
     * @param clockFrequency System clock frequency at Hz
     * @param method sampling method to use
     * @param samplingFrequency Desired output sampling rate
     * @return false on unrecognized sampling method
     */
    bool setSamplingParameters(
        double clockFrequency,
        SamplingMethod method,
        double samplingFrequency
    );

    /**
     * Clock SID forward producing audio
     * using chosen output resampling algorithm.
     *
     * @note
     * User must make sure to have enough space
     * for the buffer.
     * The number of samples produced can be approximated
     * with this formula:
     * std::ceil(cycles/clockFrequency*samplingFrequency)
     *
     * @param cycles c64 clocks to run
     * @param buf audio output buffer
     * @return number of 16bit samples produced
     */
    int clock(unsigned int cycles, short* buf);

    /**
     * Clock SID forward producing audio
     * using chosen output resampling algorithm.
     *
     * @param buf audio output buffer
     * @param bufSize the buffer size as number of 16bit samples
     * @return number of c64 clocks run
     */
    int clock(short* buf, int bufSize);

    /**
     * Clock SID forward with no audio production.
     * Only the digital parts are emulated,
     * the analog stage is ignored.
     *
     * @param cycles c64 clocks to clock.
     *
     * @since 1.1
     */
    void clockDigital(unsigned int cycles);

    /**
     * Clock SID forward with no audio production.
     *
     * @note
     * You can't mix this method of clocking with the audio-producing
     * clock() because components that don't affect OSC3/ENV3 are not
     * emulated.
     *
     * @param cycles c64 clocks to clock.
     */
    void clockSilent(unsigned int cycles);

    /**
     * Set filter curve parameter for 6581 model.
     *
     * @see Filter6581::setFilterCurve(double)
     */
    void setFilter6581Curve(double filterCurve);

    /**
    * Set filter range parameter for 6581 model.
    *
    * @see Filter6581::setFilterRange(double)
    */
    void setFilter6581Range(double adjustment);

    /**
     * Set filter curve parameter for 8580 model.
     *
     * @see Filter8580::setFilterCurve(double)
     */
    void setFilter8580Curve(double filterCurve);

    /**
     * Enable filter emulation.
     *
     * @param enable false to turn off filter emulation
     */
    void enableFilter(bool enable);

    /**
     * Enable/disable old caps for 6581 model.
     * When enabled the filter cutoff is lower.
     *
     * @param enable true to enable old 2200pF caps used on ASSY 326298
     *               false to use the standard 470pF caps. (default off)
     */
    void enableOld6581caps(bool enable);

    /**
     * Set paddle coordinates.
     *
     * @since 1.1
     */
    void setPaddle(unsigned char x, unsigned char y);

    /**
     * Get the save-state size in bytes.
     *
     * @note: the size may depend on configuration.
     *
     * @since 1.1
     */
    int stateSize() const;

    /**
     * Save current state.
     *
     * @note: the save state is not portable across different builds
     * and may change in future versions.
     *
     * @param buffer the buffer where state will be saved to
     * @param size size of the buffer in bytes
     *
     * @since 1.1
     */
    int saveState(char* buffer, int size) const;

    /**
     * Restore saved state.
     *
     * @param buffer the buffer containig the saved state
     * @param size size of the buffer in bytes
     *
     * @since 1.1
     */
    void restoreState(char* buffer, int size);

    /**
     * Set the DAC leakage level.
     * Affects the envelope and waveforms.
     *
     * @param level the leakage level, between 0 (no leakage) and 1 (standard leakage) (default 1.0)
     *
     * @since 1.2
     */
    void setDacLeakage(double level);

    /**
     * Set the 6581 wave offset.
     * Affects the volume of digi samples.
     *
     * @param offset the waveform offset, between 0 (average digis) and 1 (loud digis) (default 1.0)
     *
     * @since 1.2
     */
    void setOffset6581(double offset);

    /**
     * Set the DC-Blocker resistance.
     * Affects the highpass cutoff frequency.
     *
     * @param res the resistance value, between 0 (10KOhm => ~1.6Hz) and 1 (1KOhm => ~16Hz) (default 0.0)
     *
     * @since 1.2
     */
    void setDCBRes(double res);
};

}

#endif
