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

#ifndef EXTERNALFILTER_H
#define EXTERNALFILTER_H

#include "siddefs-fp.h"

#include <cstdint>

namespace reSIDfp
{

/**
 * The audio output stage in a Commodore 64 consists of two STC networks, a
 * low-pass RC filter with 3 dB frequency 16kHz followed by a DC-blocker which
 * acts as a high-pass filter with a cutoff dependent on the attached audio
 * equipment impedance. Here we suppose an impedance of 10kOhm resulting
 * in a 3 dB attenuation at 1.6Hz.
 * The STC networks are connected with a [BJT] based [common collector]
 * used as a voltage follower (featuring a 2SC1815 NPN transistor).
 *
 * To operate properly the 6581 audio output needs a pull-down resistor,
 * 1KOhm is the recommended value.
 *
 * ~~~
 *                                 9/12V
 * -----+
 * audio|       10k                  |
 *      +---o----R---o--------o-----(K)          +-----
 *  out |   |        |        |      |           |audio
 * -----+   R 1k     C 1000   |      |    10 uF  |
 *          |        |  pF    +-C----o-----C-----+ ~10k
 *                             470   |           |
 *         GND      GND         pF   R 1K        | amp
 *          *                   **   |           +-----
 *
 *                                  GND
 * ~~~
 *
 * * Pulldown resistor, only for the 6581.
 * ** The C64c board additionally includes a [bootstrap] capacitor to increase
 *    the input impedance of the common collector.
 *
 * The C128 boards include an additional capacitor and a resistor
 * to the ouptput stage.
 *
 * ~~~
 *                                 9/12V
 * -----+
 * audio|       10k                  |
 *      +---o----R---o--------o-----(K)                    +-----
 *  out |   |        |        |      |                     |audio
 * -----+   R 1k     C 1000   |      |        10 uF        |
 *          |        |  pF    +-C----o-----o----C---o------+ ~10k
 *                             470   |     |        |      |
 *         GND      GND         pF   R 1K  C 220pF  R 47K  | amp
 *          *                        |     |        |      +-----
 *
 *                                  GND   GND      GND
 * ~~~
 *
 * [BJT]: https://en.wikipedia.org/wiki/Bipolar_junction_transistor
 * [common collector]: https://en.wikipedia.org/wiki/Common_collector
 * [bootstrap]: https://en.wikipedia.org/wiki/Bootstrapping_(electronics)
 */
class ExternalFilter
{
    friend class State;

private:
    /// Lowpass filter voltage
    int32_t Vlp;

    /// Highpass filter voltage
    int32_t Vhp;

    int32_t w0lp_1_s7 = 0;

    int32_t w0hp_1_s17 = 0;

    double m_frequency;

    double m_ext_res = 10e3;

private:
    void recalcParams();

public:
    /**
     * SID clocking.
     *
     * @param input input sample, signed 16 bit
     * @return filtered sample, signed 16 bit
     */
    int32_t clock(int32_t input);

    /**
     * Constructor.
     */
    ExternalFilter();

    /**
     * Setup of the external filter sampling parameters.
     *
     * @param frequency the main system clock frequency in Hertz
     */
    void setClockFrequency(double frequency);

    /**
     * Setup of the external filter sampling parameters.
     *
     * @param res the resistance value in Ohms
     */
    void setExtResistance(double res);

    /**
     * SID reset.
     */
    void reset();
};

} // namespace reSIDfp

#if RESIDFP_INLINING || defined(EXTERNALFILTER_CPP)

namespace reSIDfp
{

RESIDFP_INLINE
int32_t ExternalFilter::clock(int32_t input)
{
#if __cplusplus >= 202002L
    const int32_t Vi = input << 11;
#else
    // Left shift of negative values is undefined
    const int32_t Vi = input * (1 << 11);
#endif
    const int32_t dVlp = (w0lp_1_s7 * (Vi - Vlp)) >> 7;
    const int32_t dVhp = (w0hp_1_s17 * (Vlp - Vhp)) >> 17;
    Vlp += dVlp;
    Vhp += dVhp;
    return (Vlp - Vhp) >> 11;
}

} // namespace reSIDfp

#endif

#endif
