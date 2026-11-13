/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2020 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004,2010 Dag Lem
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

#ifndef FILTERMODELCONFIG8580_H
#define FILTERMODELCONFIG8580_H

#include "FilterModelConfig.h"

#include <memory>

#include "siddefs-fp.h"

namespace reSIDfp
{

class Integrator8580;

/**
 * Calculate parameters for 8580 filter emulation.
 */
class FilterModelConfig8580 final : public FilterModelConfig
{
private:
    static std::unique_ptr<FilterModelConfig8580> instance;
    // This allows access to the private constructor
    friend std::unique_ptr<FilterModelConfig8580>::deleter_type;

private:
    /**
     * Reference voltage generated from Vcc by a voltage divider
     */
    static constexpr double Vref = 4.75;

    /**
     * Power bricks generate voltages slightly out of spec
     */
    static constexpr double VOLTAGE_SKEW = 1.01;

private:
    FilterModelConfig8580();
    ~FilterModelConfig8580() = default;

protected:
    inline double getVoiceDC(uint8_t) const override { return getVref(); }

public:
    static FilterModelConfig8580* getInstance();

    static inline constexpr double getVref() { return Vref * VOLTAGE_SKEW; }
};

} // namespace reSIDfp

#endif
