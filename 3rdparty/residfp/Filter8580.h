/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef FILTER8580_H
#define FILTER8580_H

#include "Filter.h"
#include "FilterModelConfig8580.h"
#include "Integrator8580.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

class Integrator8580;

/**
 * Filter for 8580 chip
 * --------------------
 * The 8580 filter stage had been redesigned to be more linear and robust
 * against temperature change. It also features real op-amps and a
 * revisited resonance model.
 * The filter schematics below are reverse engineered from re-vectorized
 * and annotated die photographs. Credits to Michael Huth for the microscope
 * photographs of the die, Tommi Lempinen for re-vectorizating and annotating
 * the images and ttlworks from forum.6502.org for the circuit analysis.
 *
 * ~~~
 *
 *               +---------------------------------------------------+
 *               |    $17      +----Rf-+                             |
 *               |             |       |                             |
 *               |      D4&!D5 o- \-R3-o                             |
 *               |             |       |                    $17      |
 *               |     !D4&!D5 o- \-R2-o                             |
 *               |             |       |  +---R8-- \--+  !D6&D7      |
 *               |      D4&!D5 o- \-R1-o  |           |              |
 *               |             |       |  o---RC-- \--o   D6&D7      |
 *               |   +---------o--<A]--o--o           |              |
 *               |   |                    o---R4-- \--o  D6&!D7      |
 *               |   |                    |           |              |
 *               |   |                    +---Ri-- \--o !D6&!D7      |
 *               |   |                                |              |
 * $17           |   |                    (CAP2B)     |  (CAP1B)     |
 * 0=to mixer    |   +--R7--+  +---R7--+      +---C---o      +---C---o
 * 1=to filter   |          |  |       |      |       |      |       |
 *               +------R7--o--o--[A>--o--Rfc-o--[A>--o--Rfc-o--[A>--o
 *     ve (EXT IN)          |          |              |              |
 * D3  \ --------------R12--o          |              | (CAP2A)      | (CAP1A)
 *     |   v3               |          | vhp          | vbp          | vlp
 * D2  |   \ -----------R7--o    +-----+              |              |
 *     |   |   v2           |    |                    |              |
 * D1  |   |   \ -------R7--o    |   +----------------+              |
 *     |   |   |   v1       |    |   |                               |
 * D0  |   |   |   \ ---R7--+    |   |   +---------------------------+
 *     |   |   |   |             |   |   |
 *     R9  R5  R5  R5            R5  R5  R5
 *     |   |   |   | $18         |   |   |  $18
 *     |    \  |   | D7: 1=open   \   \   \ D6 - D4: 0=open
 *     |   |   |   |             |   |   |
 *     +---o---o---o-------------o---o---+
 *                 |
 *                 |               D3 +--/ --1R4--+
 *                 |   +---R8--+      |           |  +---R2--+
 *                 |   |       |   D2 o--/ --2R4--o  |       |
 *                 +---o--[A>--o------o           o--o--[A>--o-- vo (AUDIO OUT)
 *                                 D1 o--/ --4R4--o
 *                        $18         |           |
 *                        0=open   D0 +--/ --8R4--+
 *
 *
 *
 * Resonance
 * ---------
 * For resonance, we have two tiny DACs that controls both the input
 * and feedback resistances.
 *
 * The "resistors" are switched in as follows by bits in register $17:
 *
 * feedback:
 * R1: bit4&!bit5
 * R2: !bit4&bit5
 * R3: bit4&bit5
 * Rf: always on
 *
 * input:
 * R4: bit6&!bit7
 * R8: !bit6&bit7
 * RC: bit6&bit7
 * Ri: !(R4|R8|RC) = !(bit6|bit7) = !bit6&!bit7
 *
 *
 * The relative "resistor" values are approximately (using channel length):
 *
 * R1 = 15.3*Ri
 * R2 =  7.3*Ri
 * R3 =  4.7*Ri
 * Rf =  1.4*Ri
 * R4 =  1.4*Ri
 * R8 =  2.0*Ri
 * RC =  2.8*Ri
 *
 *
 * Approximate values for 1/Q can now be found as follows (assuming an
 * ideal op-amp):
 *
 * res  feedback  input  -gain (1/Q)
 * ---  --------  -----  ----------
 *  0   Rf        Ri     Rf/Ri      = 1/(Ri*(1/Rf))      = 1/0.71
 *  1   Rf|R1     Ri     (Rf|R1)/Ri = 1/(Ri*(1/Rf+1/R1)) = 1/0.78
 *  2   Rf|R2     Ri     (Rf|R2)/Ri = 1/(Ri*(1/Rf+1/R2)) = 1/0.85
 *  3   Rf|R3     Ri     (Rf|R3)/Ri = 1/(Ri*(1/Rf+1/R3)) = 1/0.92
 *  4   Rf        R4     Rf/R4      = 1/(R4*(1/Rf))      = 1/1.00
 *  5   Rf|R1     R4     (Rf|R1)/R4 = 1/(R4*(1/Rf+1/R1)) = 1/1.10
 *  6   Rf|R2     R4     (Rf|R2)/R4 = 1/(R4*(1/Rf+1/R2)) = 1/1.20
 *  7   Rf|R3     R4     (Rf|R3)/R4 = 1/(R4*(1/Rf+1/R3)) = 1/1.30
 *  8   Rf        R8     Rf/R8      = 1/(R8*(1/Rf))      = 1/1.43
 *  9   Rf|R1     R8     (Rf|R1)/R8 = 1/(R8*(1/Rf+1/R1)) = 1/1.56
 *  A   Rf|R2     R8     (Rf|R2)/R8 = 1/(R8*(1/Rf+1/R2)) = 1/1.70
 *  B   Rf|R3     R8     (Rf|R3)/R8 = 1/(R8*(1/Rf+1/R3)) = 1/1.86
 *  C   Rf        RC     Rf/RC      = 1/(RC*(1/Rf))      = 1/2.00
 *  D   Rf|R1     RC     (Rf|R1)/RC = 1/(RC*(1/Rf+1/R1)) = 1/2.18
 *  E   Rf|R2     RC     (Rf|R2)/RC = 1/(RC*(1/Rf+1/R2)) = 1/2.38
 *  F   Rf|R3     RC     (Rf|R3)/RC = 1/(RC*(1/Rf+1/R3)) = 1/2.60
 *
 *
 * These data indicate that the following function for 1/Q has been
 * modeled in the MOS 8580:
 *
 *    1/Q = 2^(1/2)*2^(-x/8) = 2^(1/2 - x/8) = 2^((4 - x)/8)
 *
 *
 *
 * Op-amps
 * -------
 * Unlike the 6581, the 8580 has real OpAmps.
 *
 * Temperature compensated differential amplifier:
 *
 *                9V
 *
 *                |
 *      +-------o-o-o-------+
 *      |       |   |       |
 *      |       R   R       |
 *      +--||   |   |   ||--+
 *         ||---o   o---||
 *      +--||   |   |   ||--+
 *      |       |   |       |
 *      o-----+ |   |       o--- Va
 *      |     | |   |       |
 *      +--|| | |   |   ||--+
 *         ||-o-+---+---||
 *      +--||   |   |   ||--+
 *      |       |   |       |
 *              |   |
 *     GND      |   |      GND
 *          ||--+   +--||
 * in- -----||         ||------ in+
 *          ||----o----||
 *                |
 *                8 Current sink
 *                |
 *
 *               GND
 *
 * Inverter + non-inverting output amplifier:
 *
 * Va ---o---||-------------------o--------------------+
 *       |                        |               9V   |
 *       |             +----------+----------+     |   |
 *       |        9V   |          |     9V   | ||--+   |
 *       |         |   |      9V  |      |   +-||      |
 *       |         R   |       |  |  ||--+     ||--+   |
 *       |         |   |   ||--+  +--||            o---o--- Vout
 *       |         o---o---||        ||--+     ||--+
 *       |         |       ||--+         o-----||
 *       |     ||--+           |     ||--+     ||--+
 *       +-----||              o-----||            |
 *             ||--+           |     ||--+
 *                 |           R         |        GND
 *                             |
 *                GND                   GND
 *                            GND
 *
 *
 *
 * Virtual ground
 * --------------
 * A PolySi resitive voltage divider provides the voltage
 * for the positive input of the filter op-amps.
 *
 *     5V
 *          +----------+
 *      |   |   |\     |
 *      R1  +---|-\    |
 * 5V   |       |A >---o--- Vref
 *      o-------|+/
 *  |   |       |/
 * R10  R4
 *  |   |
 *  o---+
 *  |
 * R10
 *  |
 *
 * GND
 *
 * Rn = n*R1
 *
 *
 *
 * Rfc - freq control DAC resistance ladder
 * ----------------------------------------
 * The 8580 has 11 bits for frequency control, but 12 bit DACs.
 * If those 11 bits would be '0', the impedance of the DACs would be "infinitely high".
 * To get around this, there is an 11 input NOR gate below the DACs sensing those 11 bits.
 * If all are 0, the NOR gate gives the gate control voltage to the 12 bit DAC LSB.
 *
 *     ----o---o--...--o---o---o---
 *         |   |       |   |   |
 *       Rb10 Rb9 ... Rb1 Rb0  R0
 *         |   |       |   |   |
 *     ----o---o--...--o---o---o---
 *
 *
 *
 * Crystal stabilized precision switched capacitor voltage divider
 * ---------------------------------------------------------------
 * There is a FET working as a temperature sensor close to the DACs which changes the gate voltage
 * of the frequency control DACs according to the temperature of the DACs,
 * to reduce the effects of temperature on the filter curve.
 * An asynchronous 3 bit binary counter, running at the speed of PHI2, drives two big capacitors
 * whose AC resistance is then used as a voltage divider.
 * This implicates that frequency difference between PAL and NTSC might shift the filter curve by 4% or such.
 *
 *                                |\  OpAmp has a smaller capacitor than the other OPs
 *                        Vref ---|+\
 *                                |A >---o--- Vdac
 *                        +-------|-/    |
 *                        |       |/     |
 *                        |              |
 *       C1               |     C2       |
 *   +---||---o---+   +---o-----||-------o
 *   |        |   |   |   |              |
 *   o----+   |   -----   |              |
 *   |    |   |   -----   +----+   +-----o
 *   |    -----     |          |   |     |
 *   |    -----     |          -----     |
 *   |      |       |          -----     |
 *   |    +-----------+          |       |
 *        | /Q      Q |          +-------+
 *  GND   +-----------+      FET close to DAC
 *        |   clk/8   |      working as temperature sensor
 *        +-----------+
 */
class Filter8580 final : public Filter
{
    friend class State;

private:
    /// VCR + associated capacitor connected to highpass output.
    Integrator8580 hpIntegrator;

    /// VCR + associated capacitor connected to bandpass output.
    Integrator8580 bpIntegrator;

    double cp;

protected:
    /**
     * Set filter cutoff frequency.
     */
    void updateCenterFrequency() override;

    void restartIntegrators() override { hpIntegrator.restart(); bpIntegrator.restart(); }

    int32_t getNormalizedMixerVoice(float v, uint8_t env) const override
    {
        return getNormalizedVoice(v, env);
    }

public:
    Filter8580() :
        Filter(*FilterModelConfig8580::getInstance(), hpIntegrator, bpIntegrator),
        hpIntegrator(*FilterModelConfig8580::getInstance()),
        bpIntegrator(*FilterModelConfig8580::getInstance())
    {
        setFilterCurve(0.5);
    }

    ~Filter8580() override;

    /**
     * Set filter curve type based on single parameter.
     *
     * @param curvePosition 0 .. 1, where 0 sets center frequency low ("dark") and 1 sets it high ("bright").
     *                      Default is 0.5
     */
    void setFilterCurve(double curvePosition);
};

} // namespace reSIDfp

#endif
