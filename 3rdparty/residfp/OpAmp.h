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

#ifndef OPAMP_H
#define OPAMP_H

#include <vector>

#include "Spline.h"

#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * Find output voltage in inverting gain and inverting summer SID op-amp
 * circuits, using a combination of Newton-Raphson and bisection.
 *
 *               +---R2--+
 *               |       |
 *     vi ---R1--o--[A>--o-- vo
 *               vx
 *
 * From Kirchoff's current law it follows that
 *
 *     IR1f + IR2r = 0
 *
 * Substituting the triode mode transistor model K*W/L*(Vgst^2 - Vgdt^2)
 * for the currents, we get:
 *
 *     n*((Vddt - vx)^2 - (Vddt - vi)^2) + (Vddt - vx)^2 - (Vddt - vo)^2 = 0
 * 
 * where n is the ratio between R1 and R2.
 *
 * Our root function f can thus be written as:
 *
 *     f = (n + 1)*(Vddt - vx)^2 - n*(Vddt - vi)^2 - (Vddt - vo)^2 = 0
 *
 * Using substitution constants
 *
 *     a = n + 1
 *     b = Vddt
 *     c = n*(Vddt - vi)^2
 *
 * the equations for the root function and its derivative can be written as:
 *
 *     f = a*(b - vx)^2 - c - (b - vo)^2
 *     df = 2*((b - vo)*dvo - a*(b - vx))
 */
class OpAmp
{
private:
    /// Current root position (cached as guess to speed up next iteration)
    mutable double x = 0.;

    const double Vddt;
    const double vmin;
    const double vmax;

    Spline opamp;

public:
    /**
     * Opamp input -> output voltage conversion
     *
     * @param opamp opamp mapping table as pairs of points (in -> out)
     * @param Vddt transistor dt parameter (in volts)
     * @param vmin
     * @param vmax
     */
    OpAmp(const std::vector<Spline::Point> &opamp_voltages, double new_Vddt,
            double new_vmin, double new_vmax
    ) :
        Vddt(new_Vddt),
        vmin(new_vmin),
        vmax(new_vmax),
        opamp(opamp_voltages) {}

    /**
     * Reset root position
     */
    void reset() const
    {
        x = vmin;
    }

    /**
     * Solve the opamp equation for input vi in loading context n
     *
     * @param n the ratio of input/output loading
     * @param vi input voltage
     * @return vo output voltage
     */
    double solve(double n, double vi) const;
};

} // namespace reSIDfp

#endif
