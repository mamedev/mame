/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2015 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef TWOPASSSINCRESAMPLER_H
#define TWOPASSSINCRESAMPLER_H

#include <cmath>

#include <memory>

#include "Resampler.h"
#include "SincResampler.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * Compose a more efficient SINC from chaining two other SINCs.
 */
class TwoPassSincResampler final : public Resampler
{
    friend class State;

private:
    std::unique_ptr<SincResampler> const s1;
    std::unique_ptr<SincResampler> const s2;

private:
    TwoPassSincResampler(double clockFrequency, double samplingFrequency, double highestAccurateFrequency, double intermediateFrequency) :
        s1(new SincResampler(clockFrequency, intermediateFrequency, highestAccurateFrequency)),
        s2(new SincResampler(intermediateFrequency, samplingFrequency, highestAccurateFrequency))
    {}

public:
    // Named constructor
    static TwoPassSincResampler* create(double clockFrequency, double samplingFrequency)
    {
        // Set the passband frequency slightly below half sampling frequency
        //   pass_freq <= 0.9*sample_freq/2
        //
        // This constraint ensures that the FIR table is not overfilled.
        // For higher sampling frequencies we're fine with 20KHz
        const double halfFreq = (samplingFrequency > 44000.)
            ? 20000. : samplingFrequency * 0.45;

        // Calculation according to Laurent Ganier.
        // It evaluates to about 120 kHz at typical settings.
        // Some testing around the chosen value seems to confirm that this does work.
        double const intermediateFrequency = 2. * halfFreq
            + std::sqrt(2. * halfFreq * clockFrequency
                * (samplingFrequency - 2. * halfFreq) / samplingFrequency);

        return new TwoPassSincResampler(
            clockFrequency, samplingFrequency, halfFreq, intermediateFrequency);
    }

    bool input(int32_t sample) override
    {
        return s1->input(sample) && s2->input(s1->output());
    }

    int32_t output() const override
    {
        return s2->output();
    }

    void reset() override
    {
        s1->reset();
        s2->reset();
    }
};

} // namespace reSIDfp

#endif
