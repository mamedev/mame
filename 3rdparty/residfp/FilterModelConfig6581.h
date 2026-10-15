/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef FILTERMODELCONFIG6581_H
#define FILTERMODELCONFIG6581_H

#include "FilterModelConfig.h"

#include <memory>

#include "Dac.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

class Integrator6581;

/**
 * Calculate parameters for 6581 filter emulation.
 */
class FilterModelConfig6581 final : public FilterModelConfig
{
private:
    static std::unique_ptr<FilterModelConfig6581> instance;
    // This allows access to the private constructor
    friend std::unique_ptr<FilterModelConfig6581>::deleter_type;

public:
    /*
     * The ratio of the resistors of the mixer input for filter signals
     * compared to the voice ones.
     * See the 6581 filter schematic.
     */
    static constexpr double VF_TR_RATIO = 1.07;

private:
    static constexpr unsigned int DAC_BITS = 11;

    /**
     * Power bricks generate voltages slightly out of spec
     */
    static constexpr double VOLTAGE_SKEW = 1.015;

    /// Transistor parameters.
    //@{
    const double WL_vcr;        ///< W/L for VCR
    const double WL_snake;      ///< W/L for "snake"
    //@}

    /// DAC parameters.
    //@{
    const double dac_zero;
    const double dac_scale;
    //@}

    /// DAC lookup table
    Dac dac;

    /// Voltage Controlled Resistors
    //@{
    uint16_t vcr_nVg[1 << 16];
    double vcr_n_Ids_term[1 << 16];
    //@}

    double vcr_mult;
    bool m_oldCaps;

    // Voice DC offset LUT
    double voiceDC[256];

private:
    double getDacZero(double adjustment) const { return dac_zero + 2. * adjustment - 1.; }

    void updateParams();

    FilterModelConfig6581();
    ~FilterModelConfig6581() = default;

protected:
    /**
     * On 6581 the DC offset varies between ~5.0V and ~5.214V depending on
     * the envelope value.
     */
    inline double getVoiceDC(uint8_t env) const override
    {
        return voiceDC[env];
    }

public:
    static FilterModelConfig6581* getInstance();

    void setFilterRange(double adjustment);

    void enableOldCaps(bool enable);

    /**
     * Construct an 11 bit cutoff frequency DAC output voltage table.
     * Ownership is transferred to the requester which becomes responsible
     * of freeing the object when done.
     *
     * @param adjustment
     * @return the DAC table
     */
    uint16_t* getDAC(double adjustment) const;

    inline double getWL_snake() const { return WL_snake; }

    inline uint16_t getVcr_nVg(int i) const { return vcr_nVg[i]; }
    inline uint16_t getVcr_n_Ids_term(int i) const
    {
        return to_uint16(vcr_n_Ids_term[i] * vcr_mult);
    }
    // only used if SLOPE_FACTOR is defined
    static inline constexpr double getUt() { return Ut; }
    inline double getN16() const { return N16; }
};

} // namespace reSIDfp

#endif
