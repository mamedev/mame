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

#ifndef PASSTHROUGH_H
#define PASSTHROUGH_H

#include "Resampler.h"

namespace reSIDfp
{

/**
 * Pass through with no resampling.
 */
class PassThrough final : public Resampler
{
    friend class State;

private:
    /// Last sample
    int32_t outputValue = 0;

public:
    bool input(int32_t sample) override
    {
        outputValue = sample;

        return true;
    }

    int32_t output() const override { return outputValue; }

    void reset() override
    {
        outputValue = 0;
    }
};

} // namespace reSIDfp

#endif
