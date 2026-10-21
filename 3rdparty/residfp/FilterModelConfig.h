/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2024 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef FILTERMODELCONFIG_H
#define FILTERMODELCONFIG_H

#include <algorithm>
#include <random>
#include <cassert>
#include <climits>

#include "OpAmp.h"
#include "Spline.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

class FilterModelConfig
{
public:
    // The highpass summer has 2 - 6 inputs (bandpass, lowpass, and 0 - 4 voices).
    template<int i>
    struct summer_offset
    {
        static constexpr int value = summer_offset<i - 1>::value + ((2 + i - 1) << 16);
    };

    // The mixer has 0 - 7 inputs (0 - 4 voices and 0 - 3 filter outputs).
    template<int i>
    struct mixer_offset
    {
        static constexpr int value = mixer_offset<i - 1>::value + ((i - 1) << 16);
    };

protected:
    static inline uint16_t to_uint16_dither(double x, double d_noise)
    {
        const int tmp = static_cast<int>(x + d_noise);
        assert((tmp >= 0) && (tmp <= USHRT_MAX));
        return static_cast<uint16_t>(tmp);
    }

    static inline uint16_t to_uint16(double x)
    {
        return to_uint16_dither(x, 0.5);
    }

private:
    /*
     * Hack to add quick dither when converting values from float to int
     * and avoid quantization noise.
     * Hopefully this can be removed the day we move all the analog part
     * processing to floats.
     *
     * Not sure about the effect of using such small buffer of numbers
     * since the random sequence repeats every 1024 values but for
     * now it seems to do the job.
     */
    class Randomnoise
    {
    private:
        double buffer[1024];
        mutable int index = 0;
    public:
        Randomnoise()
        {
            std::uniform_real_distribution<double> unif(0., 1.);
            std::default_random_engine re;
            for (int i=0; i<1024; i++)
                buffer[i] = unif(re);
        }
        double getNoise() const { index = (index + 1) & 0x3ff; return buffer[index]; }
    };

protected:
    /// Capacitor value.
    const double C;

    /// Transistor parameters.
    //@{
    /// Thermal voltage: Ut = kT/q = 8.61734315e-5*T ~ 26mV
    static constexpr double k = 1.380649e-23;    // Boltzmann Constant
    static constexpr double q = 1.602176634e-19; // charge of an electron
    static constexpr double temp = 27;           // temperature in °C
    static constexpr double Ut = (k * (temp + 273.15)) / q;

    const double Vdd;           ///< Positive supply voltage
    const double Vth;           ///< Threshold voltage
    const double Vddt;          ///< Vdd - Vth
    double uCox;                ///< Transconductance coefficient: u*Cox
    //@}

    // Derived stuff
    const double vmin, vmax;
    const double denorm, norm;

    /// Fixed point scaling for 16 bit op-amp output.
    const double N16;

    const double voice_voltage_range;

    /// Current factor coefficient for op-amp integrators.
    double currFactorCoeff;

    /// Lookup tables for gain and summer op-amps in output stage / filter.
    //@{
    uint16_t* mixer;          //-V730_NOINIT this is initialized in the derived class constructor
    uint16_t* summer;         //-V730_NOINIT this is initialized in the derived class constructor
    uint16_t* volume;         //-V730_NOINIT this is initialized in the derived class constructor
    uint16_t* resonance;      //-V730_NOINIT this is initialized in the derived class constructor
    //@}

    /// Reverse op-amp transfer function.
    uint16_t opamp_rev[1 << 16]; //-V730_NOINIT this is initialized in the derived class constructor

private:
    Randomnoise rnd;

private:
    FilterModelConfig(const FilterModelConfig&) = delete;
    FilterModelConfig& operator= (const FilterModelConfig&) = delete;

    inline double getVoiceVoltage(float value, uint8_t env) const
    {
        return value * voice_voltage_range + getVoiceDC(env);
    }

protected:
    /**
     * @param vvr voice voltage range
     * @param c   capacitor value
     * @param vdd Vdd supply voltage
     * @param vth threshold voltage
     * @param ucox u*Cox
     * @param opamp_voltage opamp voltage array
     * @param opamp_size opamp voltage array size
     */
    FilterModelConfig(
        double vvr,
        double c,
        double vdd,
        double vth,
        double ucox,
        const Spline::Point *opamp_voltage,
        int opamp_size
    );

    ~FilterModelConfig();

    void calcCurrFactorCoeff();

    virtual double getVoiceDC(uint8_t env) const = 0;

    /**
     * The filter summer operates at n ~ 1, and has 5 fundamentally different
     * input configurations (2 - 6 input "resistors").
     *
     * Note that all "on" transistors are modeled as one. This is not
     * entirely accurate, since the input for each transistor is different,
     * and transistors are not linear components. However modeling all
     * transistors separately would be extremely costly.
     */
    inline void buildSummerTable(const OpAmp& opampModel)
    {
        const double r_N16 = 1. / N16;

        int idx = 0;
        for (int i = 0; i < 5; i++)
        {
            const int idiv = 2 + i;        // 2 - 6 input "resistors".
            const int size = idiv << 16;
            const double n = idiv;
            const double r_idiv = 1. / idiv;
            opampModel.reset();

            for (int vi = 0; vi < size; vi++)
            {
                const double vin = vmin + vi * r_N16 * r_idiv; /* vmin .. vmax */
                summer[idx++] = getNormalizedValue(opampModel.solve(n, vin));
            }
        }
    }

    /**
     * The audio mixer operates at n ~ 8/6 (6581) or 8/5 (8580),
     * and has 8 fundamentally different input configurations
     * (0 - 7 input "resistors").
     *
     * All "on", transistors are modeled as one - see comments above for
     * the filter summer.
     */
    inline void buildMixerTable(const OpAmp& opampModel, double nRatio)
    {
        const double r_N16 = 1. / N16;

        int idx = 0;
        for (int i = 0; i < 8; i++)
        {
            const int idiv = (i == 0) ? 1 : i;
            const int size = (i == 0) ? 1 : i << 16;
            const double n = i * nRatio;
            const double r_idiv = 1. / idiv;
            opampModel.reset();

            for (int vi = 0; vi < size; vi++)
            {
                const double vin = vmin + vi * r_N16 * r_idiv; /* vmin .. vmax */
                mixer[idx++] = getNormalizedValue(opampModel.solve(n, vin));
            }
        }
    }

    /**
     * 4 bit "resistor" ladders in the audio output gain
     * necessitate 16 gain tables.
     * From die photographs of the volume "resistor" ladders
     * it follows that gain ~ vol/12 (6581) or vol/16 (8580)
     * (assuming ideal op-amps and ideal "resistors").
     */
    inline void buildVolumeTable(const OpAmp& opampModel, double nDivisor)
    {
        const double r_N16 = 1. / N16;

        int idx = 0;
        for (int n8 = 0; n8 < 16; n8++)
        {
            constexpr int size = 1 << 16;
            const double n = n8 / nDivisor;
            opampModel.reset();

            for (int vi = 0; vi < size; vi++)
            {
                const double vin = vmin + vi * r_N16; /* vmin .. vmax */
                volume[idx++] = getNormalizedValue(opampModel.solve(n, vin));
            }
        }
    }

    /**
     * 4 bit "resistor" ladders in the bandpass resonance gain
     * necessitate 16 gain tables.
     * From die photographs of the bandpass "resistor" ladders
     * it follows that 1/Q ~ ~res/8 (6581) or 2^((4 - res)/8) (8580)
     * (assuming ideal op-amps and ideal "resistors").
     */
    inline void buildResonanceTable(const OpAmp& opampModel, const double resonance_n[16])
    {
        const double r_N16 = 1. / N16;

        int idx = 0;
        for (int n8 = 0; n8 < 16; n8++)
        {
            constexpr int size = 1 << 16;
            opampModel.reset();

            for (int vi = 0; vi < size; vi++)
            {
                const double vin = vmin + vi * r_N16; /* vmin .. vmax */
                resonance[idx++] = getNormalizedValue(opampModel.solve(resonance_n[n8], vin));
            }
        }
    }

public:
    uint16_t* getVolume() const { return volume; }
    uint16_t* getResonance() const { return resonance; }
    uint16_t* getSummer() const { return summer; }
    uint16_t* getMixer() const { return mixer; }

    inline uint16_t getOpampRev(int i) const { return opamp_rev[i]; }
    inline double getVddt() const { return Vddt; }
    inline double getVth() const { return Vth; }

    // helper functions

    inline uint16_t getNormalizedValue(double value) const
    {
        return to_uint16_dither(N16 * (value - vmin), rnd.getNoise());
    }

    template<int N>
    inline uint16_t getNormalizedCurrentFactor(double wl) const
    {
        return to_uint16((1 << N) * currFactorCoeff * wl);
    }

    inline uint16_t getNVmin() const
    {
        return to_uint16(N16 * vmin);
    }

    inline int32_t getNormalizedVoice(float value, uint8_t env) const
    {
        return static_cast<int32_t>(getNormalizedValue(getVoiceVoltage(value, env)));
    }
};

template<>
struct FilterModelConfig::summer_offset<0>
{
    static constexpr int value = 0;
};

template<>
struct FilterModelConfig::mixer_offset<1>
{
    static constexpr int value = 1;
};

template<>
struct FilterModelConfig::mixer_offset<0>
{
    static constexpr int value = 0;
};

} // namespace reSIDfp

#endif
